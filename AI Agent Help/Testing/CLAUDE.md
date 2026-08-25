# Testing

How to run this repo's two test suites, and what to expect from them. See
[Building](../Building/CLAUDE.md) first if you've changed C++/Cython code — these suites test
the *installed* package, so a stale build silently tests old code.

Two independent suites, each with its own `main.py` and `requirements.txt` — install
requirements once per suite before first use (`python3 -m pip install -r requirements.txt` from
that suite's directory).

## Unit Tester (`Testing/Unit Tester`)
Thin wrapper around Python's `unittest`, defaulting to testing the **API** system (points at
`Anime Game Remap (for all users)/api`, i.e. exactly what `main.py -d` installs into). Run from
`Testing/Unit Tester`:
```bash
py -3 main.py                          # everything
py -3 main.py SomeTestClass            # one test class
py -3 main.py SomeTestClass.test_name  # one test
```
Useful flags mirror `unittest`'s own CLI: `-v`/`-q`, `-f` (failfast), `-k PATTERN`
(substring filter), `-s {script,api}` (switch which distribution is under test — default `api`).

Test classes live in `UnitTester/Tests/`, registered in `UnitTester/Tests/__init__.py` — a new
test module needs an entry there to be picked up by name. Conventions:
- Inherit `BaseUnitTest` (`Tests/baseUnitTest.py`) for the standard `setUp`
  (`FRB.HashTools.clear()`), the `PatchService` mixin (`self.patch(...)`/`self.patchObj(...)`
  with automatic cleanup), and the `compareX` family of structural-equality assertion helpers
  (`compareDict`, `compareList`, `compareSet`, `compareFileStats`, `compareParseTree`,
  `compareParseTreeShape`, `compareParserNodeShape`, ...) — prefer these over hand-rolled
  comparisons when the target type has one.
  There are also narrower base classes for specific subsystems (`baseIniFileTest.py`,
  `baseTrieTest.py`, `baseOrderedMultiMapTest.py`, `baseIfTemplateTreeTest.py`, ...) — check for
  one matching what you're testing before subclassing `BaseUnitTest` directly.
- Tests reach the package via `import src.py.FixRaidenBoss2 as FRB` (after
  `sys.path.insert(1, Configs[ConfigKeys.SysPath])`) — this is the *installed* copy under
  `api/src/py/FixRaidenBoss2`, not a fresh temp install, so **rebuild before testing** any
  C++/Cython-side change.
- pybind11-bound classes get their own `test_CppXxx.py` files (e.g. `test_CppTrie.py`,
  `test_CppAhoCorasickDFA.py`) — follow that naming and the sibling files' structure for a new
  C++-backed feature. This naming assumes the class keeps its `Cpp` prefix permanently (the
  "wrapper" outcome in [Architecture](../Architecture/CLAUDE.md)'s "Two different outcomes for
  porting a class to C++/pybind11"); a class that's fully replacing a bare-named pure-Python one
  (e.g. `IfContentPartColouring`, `IfContentPart`) gets `test_Xxx.py` under the bare name instead,
  matching `test_OrderedMultiMap.py`-style existing precedent (`OrderedMultiMap` itself is
  pybind11-bound but was never `Cpp`-prefixed, since nothing shadowed it). **`IfContentPart` is a
  known exception to its own rule** — its tests are still `test_CppIfContentPart.py`/
  `CppIfContentPartTest`, unrenamed, because `test_IfContentPart.py` was already occupied by a
  stale pre-C++-port test file when the class itself went through this outcome (see
  [Architecture](../Architecture/CLAUDE.md)'s step 7 for the full story) — don't take this specific
  file's name as proof of the naming rule, and resolve the collision (or ask the user how to) if
  you're touching this area anyway. Pure-Python-implementation reference classes used to
  cross-check a C++ implementation (see `test_IOrderedMultiMap.py`'s `PyListOMM`) are a
  recognized pattern here, not a one-off.
- **When the class being renamed to `...Old` (step 2 of the migration checklist) already has its
  own currently-passing test file, rename that test file and its test class in lockstep with the
  source rename, rather than treating it as the same kind of ambiguous collision `IfContentPart`
  hit above.** `test_IfPredTokenizer.py`/`IfPredTokenizerTest` renamed to
  `test_IfPredTokenizerOld.py`/`IfPredTokenizerOldTest` (updating its `FRB.IfPredTokenizer()`
  construction call to `FRB.IfPredTokenizerOld()` along the way), freeing up
  `test_IfPredTokenizer.py` for a fresh, black-box test file against the new C++-backed class.
  This differs from the `IfContentPart` case in one important way that makes it *not* ambiguous:
  there, `test_IfContentPart.py` was already a stale, unrelated leftover occupying the bare name
  before the migration even started; here, the existing test file is a live, currently-passing,
  non-stale test of the exact class being renamed — the target filename only "collides" because
  you're about to vacate it, not because something else already lives there. Some of the old
  test's own methods may not port to the new file at all — e.g. `test_IfPredTokenizerOld.py` kept
  `test_addStartState_startStateAdded`'s `self._tokenizer._dfa.stateLen()` white-box assertion
  (only meaningful against the still-live pure-Python class, whose `_dfa` is a real Python-bound
  `DFA` object), while the fresh `test_IfPredTokenizer.py` had no equivalent (the C++ port's
  internal `dfa` member isn't exposed to Python at all) and relied on purely behavioral
  (black-box) coverage instead.
- `PyListOMM` (the pure-Python `IOrderedMultiMap` reference implementation) exists as **two
  separate copies** — one in `test_IOrderedMultiMap.py`, one in `test_CppIfContentPart.py`. If
  you change `IOrderedMultiMap`'s virtual method signatures, both need updating, or they'll
  silently stop being valid implementations of the interface (see
  [Architecture](../Architecture/CLAUDE.md) for exactly how "silently" plays out — changing an
  existing virtual method's arity breaks every call through the pybind11 trampoline for any
  pre-existing Python subclass, not just calls that touch the new parameter).
- When a change touches `IOrderedMultiMap`'s virtual API, test it **through the trampoline**,
  not just by calling a pure-Python subclass's method directly from Python (that never crosses
  the C++ vtable at all, so it can't catch an arity mismatch). Construct a C++ consumer backed
  by the Python implementation instead, e.g. `FRB.IfContentPart(content=somePyListOMM)`, then
  call the method through *that* object.
- **Per-class unit tests that each construct their own fresh instance can pass while the real
  shared-instance case is still broken.** A `vector<unique_ptr<T>>`-owning pybind11 class needs a
  test that reuses the *exact same* Python object across more than one construction (or one
  construction with the same value repeated, e.g. `[x] * 3`) if its disown-vs-clone semantics
  matter (see [Architecture](../Architecture/CLAUDE.md)'s section on this). This is exactly how
  the `BufElementType`/`BufDataType` disown bug slipped past an otherwise-thorough
  `test_BufDataType.py`/`test_BufElementType.py`: every test built its own throwaway `BufDataType`,
  so nothing ever reused one — the bug only surfaced through a real end-to-end script exercising
  `constants/BufElementTypes.py`'s actual `[BufDataTypes.Float32.value] * 3`-style literals, which
  route through a cached `DeferredEnum` value. Before finalizing tests for a class in this
  situation, grep its real (non-test) call sites for reuse the same way described in
  [Architecture](../Architecture/CLAUDE.md), and if you find any, add a dedicated
  shared-instance-reuse test rather than trusting per-class isolation tests alone — and still run
  an end-to-end script/the full suite against the real call sites before calling the port done,
  since a `DeferredEnum`-cached value is invisible to a plain `grep "ClassName("` over the
  constants module that actually triggers it.
- **A new test module needs registering in `Tests/__init__.py` in two separate places, not one.**
  There's an import line (`from .test_Xxx import XxxTest`) *and* a hand-maintained `__all__` list
  further down the same file that the import lines don't feed into automatically. `main.py`
  resolves test classes via `from UnitTester.Tests import *`, which only sees names present in
  `__all__` — a class that's imported but left out of `__all__` fails with
  `AttributeError: module '__main__' has no attribute 'XxxTest'` when run by name, even though the
  import itself succeeded silently. Add the class to both.
- **`IfContentPart`'s `src`/`buildFromOrder` constructor `index` values are not preserved as the
  literal stored index** — they only control cross-key insertion *ordering* (stable-sorted, then
  appended). Once inserted, `getByInd`/`getValsWithInds`/etc. return the **true positional
  index** — a renumbered, sequential position in the actual storage — which only happens to equal
  the raw index you supplied when every occurrence across every key in that part already forms a
  gapless, tie-free sequence starting at 0. A single-key part with `[(0, "1"), (2, "3")]` (a gap)
  or two keys sharing the same raw index (a tie, broken by `src` dict order) will NOT round-trip
  their raw indices — recompute the actual expected positions by hand (or print and inspect) when
  asserting `getValsWithInds`-shaped results in a test, rather than assuming the `src` literal.
- **A bare pybind11-bound class (constructed directly, not through its pure-Python wrapper
  subclass) has no `__dict__` and can't have arbitrary attributes set on it** — e.g.
  `FRB.CppPixelFilter()` raises trying to set `.transforms = [...]`, since only the Python
  `PixelFilter` subclass (`class PixelFilter(CppPixelFilter): ...`) gets a `__dict__`, for free,
  as an ordinary consequence of being a plain Python class (see
  [Architecture](../Architecture/CLAUDE.md)'s note on this — no `py::dynamic_attr()` involved).
  When a test for a "Wrapper" outcome class (see Architecture's "Two different outcomes" section)
  needs to set a Python-only attribute the C++ core doesn't know about, construct the bare `Cpp`
  name only when deliberately testing the underlying C++ class itself; construct the bare-named
  Python wrapper for everything else. Found this exact way writing `test_CppPixelFilter.py`.
- A method that internally iterates a `std::unordered_set`/`unordered_map` (e.g. anything built
  from `IOrderedMultiMap::getKeys()`, or an `IfContentPartColouring::updateColouring()`-style
  `targetKeys` param) has **non-deterministic iteration order** — any test asserting the resulting
  *insertion order* into a downstream ordered container (`keys()`/`items()` on the result) is
  flaky by construction. Compare with `set(...)`/`compareDict`/direct key lookups instead, and
  reserve insertion-order assertions for state you built yourself via direct, order-preserving
  calls (`set`/`__setitem__` in a specific sequence).
- **`compareSet(result, expected)` requires `result` to actually be (or support) a real `set` —
  it internally does `result - expected`, which raises `TypeError` (not a test failure) if
  `result` is a `list`.** This bites specifically when a method's return type changes from
  `Set[...]` to `List[...]` (see [Architecture](../Architecture/CLAUDE.md)'s note on
  `getCommonKeys` for why that happens) — existing tests calling `compareSet` directly on the
  result need `set(result)` wrapped around it, or switched to `compareList` if the order is now
  meaningful and worth locking down instead of just comparing membership.
- **`mock.patch`-ing a "private" method/attribute (e.g. `_generateStateId`) only ever worked
  because the target was a plain pure-Python class — once that class is replaced by a
  pybind11-bound one, the same patch call fails outright** (there's no such patchable attribute on
  a compiled type at all — `AttributeError` or a silent no-op depending on exactly what's being
  patched). This isn't a test bug to route around with a cleverer patch target; it's a real,
  permanent capability loss from the port itself (id generation is now real random UUIDs from the
  binding's own default generator, with nothing left to intercept). Two fixes, pick based on what
  the test actually needs:
  - If the test's real assertion never depended on the generated ids in the first place (e.g. it
    only checks a final derived value, like a sympy query) — delete the mocking outright and use a
    fresh, un-mocked instance per test. This is easy to miss for a *second* test file that
    superficially looks unrelated to the ported class but happens to construct one in its own
    `setUp` (`test_IfPredLogicGenerator.py`/`test_SympyIfPredGenerator.py` both broke this way from
    porting `BaseSLR1Parser`, discovered only via a full-suite run, not by touching either file
    directly) — after porting a class away from mockable pure-Python, grep every test file that
    constructs an instance of it, not just the test file with the same name.
  - If the test's expected data is large, hand-authored, and keyed by the *specific* ids the old
    mocked generator used to produce (e.g. a literal expected parse-tree structure) — don't try to
    reproduce the same fixed-id sequence some other way. Add a **shape**-comparison helper instead
    (`compareParseTreeShape`/`compareParserNodeShape` in `baseUnitTest.py`) that walks both trees
    in parallel and compares everything *except* the actual id values (structure, token/production
    identity, child order/count). The existing hand-authored expected-tree literals typically need
    **zero changes** for this — their own ids were always arbitrary test-author labels to begin
    with, not meaningful data, so comparing shape instead of exact equality doesn't lose any real
    coverage.
- **Before "fixing" a bug in the C++-ported tokenizer/parser layer (`BaseTokenizer`,
  `IfPredTokenizer`, `BaseSLR1Parser`, ...), grep the relevant test file for a test that already
  documents the exact same behavior as an intentionally-preserved quirk carried over from the
  pure-Python predecessor**, before assuming it's simply untested. Confirmed hitting this fixing an
  empty-string tokenizer crash: `test_BaseTokenizer.py` had a passing
  `test_emptySrc_raisesSyntaxErr` whose own comment explicitly said this matched
  `BaseTokenizerOld`'s behavior and was "an existing quirk being preserved here, not a gap
  introduced by the port" — i.e. a previous session had already found the same odd behavior,
  decided (at the time) it wasn't worth deviating from the Python original, and pinned it down with
  a test rather than leaving it as an accident. If you do conclude the quirk itself should now be
  fixed (as opposed to just working around it), that pinning test has to change in lockstep with
  the fix — rename it to reflect the new expected behavior (don't leave a stale name like
  `..._raisesSyntaxErr` on a test that no longer does) and update its assertion; it will otherwise
  fail as a false regression the moment your fix lands, even though the fix is correct. Don't
  assume the absence of a `test_Xxx_edgeCase.py`-shaped file means an edge case is genuinely
  untested — the pinning test may already exist under a name built around the *old* (bug) behavior
  rather than the input itself.
- **A `test_Xxx.py` for a not-yet-implemented `model/strategies/iniFixers/regEdits/`- or
  `graphGroupEdits/`-style stub often already exists on disk as a literal one-line
  `# TODO: Add tests for Xxx class` placeholder**, not a genuinely missing file — confirmed for
  `test_RegAdd.py`, `test_RegNewVals.py`, `test_RegRemap.py`, and `test_RegRemove.py`. `Write`
  refuses to overwrite a file you haven't `Read` first, so `Read` it (even though you already know
  it's just the TODO line) before writing the real test module over it, and don't assume the
  absence of a `find`/`Glob` hit for some other naming guess means no test file exists yet — check
  the exact `test_<ClassName>.py` path directly.
- **Always include at least one test that constructs every argument 100% inline, with no separate
  Python variable ever holding a reference to a piece of it**, for any pybind11-bound class that
  stores raw pointers into other Python-constructible objects (`IniSectionGraph({"a":
  IfTemplate([IfContentPart(...)])}, ...)`, not `parts = [IfContentPart(...)]; t =
  IfTemplate(parts); graph = IniSectionGraph({"a": t}, ...)`). This calling style is extremely
  common in this codebase's own real fixer code, and it's the *only* shape that reliably catches
  the wrapper-lifetime bug class described in [Architecture](../Architecture/CLAUDE.md) — a test
  that happens to hold a named variable for every constructed piece can pass by pure accident (the
  variable's own reference keeps the wrapper alive, masking the bug entirely). Found this way
  twice while writing this port's own test suite: a real access-violation crash from
  `IniSectionGraph(..., z3Ctx = Z3Context())`, and a silent `id(part)` collision from
  `IniSectionGraph({"a": IfTemplate([IfContentPart(...)])}, ...)` — neither reproduced with a
  named variable held for the inner objects.
- **After passing a Python-constructed `IfContentPart`/`IfPredPart` into `IfTemplate`'s
  constructor (or `.add()`/`__setitem__`), the *original* Python object is "disowned"** — ownership
  has moved into C++, and any further attribute/method access on that original object raises
  `ValueError: Missing value for wrapped C++ type ...: Python instance was disowned` (this is the
  same unique_ptr-transfer contract `IfContentPart`'s own `content` constructor parameter already
  has, see [Architecture](../Architecture/CLAUDE.md)'s `py::smart_holder` notes). Never keep a
  reference to the list you passed into `IfTemplate(...)` and reuse *those* objects afterward for
  comparison — fetch the current, live wrapper back out through the `IfTemplate` itself
  (`ifTemplate.parts[i]`/`ifTemplate[i]`/`ifTemplate.partsById[...]`) instead. Relatedly, don't
  write an identity assertion (`assertIs`) between two separate `.parts`/`__getitem__` accesses on
  a bare `IfTemplate` (not reached through an `IniSectionGraph`) — nothing currently guarantees
  the same Python wrapper object comes back twice in a row (a known, deliberately-scoped-out gap,
  see [Architecture](../Architecture/CLAUDE.md)'s wrapper-lifetime section); compare structurally
  (`.entries()`, `.src`/`.type`) instead.
- **A new C++-side global/static registry (a `ModTypeIdTools`-style all-static-method "Tools" class
  holding process-wide state, e.g. `_modTypes`/`_nameDFA`) needs its own `clear()` method before
  it's safely testable at all** — without one, every test in the whole suite that touches the
  registry shares the *same* process-lifetime state (Python's `unittest` runs the whole suite in
  one process), so an earlier test's `registerModType(...)` silently leaks into a later, unrelated
  test's assertions. This mirrors `HashTools.clear()`/`CppHashTools.clear()`'s own reason for
  existing — check whether a new static-registry class already has an equivalent `clear()` *before*
  writing tests against it; if it doesn't, add one (mirroring the existing `HashTools`/
  `CppHashTools` shape) as part of the same change, and call it from the test file's own `setUp()`.
  Found writing `test_ModTypeId.py` against the newly-added `ModTypeIdTools.getModType`/
  `registerModType`/`findByName` — none of the three had any way to reset state until `clear()` was
  added specifically to make the test suite viable.
- **A bare pybind11 class with no `__eq__` defined compares by Python object identity, not value** —
  `ModTypeIdData`/`ModType`(`CppModType`)-style plain data classes have no `.def(py::self ==
  py::self)` binding, so `self.compareDict(resultDict, {key: resultDict[key]})`-style self-
  referential comparisons fail even when the "expected" value was *fetched from the exact same
  dict* the result came from — each `dict[key]` access on the Python side returns/`py::cast`s a
  fresh wrapper object for the same underlying C++ value, and `!=` sees two different objects.
  `compareDict`/`compareList`'s default `!=`-based comparison silently assumes value equality is
  meaningful for whatever's inside the container — it isn't, for a bare pybind11 class with no
  `__eq__`. Compare dict *keys* (`sorted(resultDict.keys())`) or specific scalar fields
  (`result.name`, `result.modTypeId`) instead of the whole wrapper object, unless the class in
  question is confirmed to have a real `__eq__` binding.

### Known-broken/WIP test modules — don't chase these as regressions
**Not every test module in `Tests/` is finished/passing right now** — some are known
work-in-progress from the maintainer and fail for reasons unrelated to your change. The maintainer
has been actively fixing these incrementally (a large batch — `test_FileService`,
`test_BaseSLR1Parser`, `test_IfPredTokenizer`, `test_IfPredParser`, `test_SympyParser`,
`test_IfPredLogicGenerator`, `test_SympyIfPredGenerator`, `test_IntTools`, `test_Version`,
`test_IfTemplateNormTree`, `test_IfTemplateTree`, and the old pre-C++-port `test_IfContentPart` —
all went from broken to fully passing in one pass), so **don't trust this list blindly; re-run and
re-verify rather than assuming stale entries are still accurate**, in either direction. Confirmed
right after this file's own warning above was written: the `test_Mod`/`test_ModType`/etc. root
cause named two paragraphs below (`ModMappedAssets.updateKeys`, `TypeError: string indices must be
integers`) had *already* drifted by the time it was re-checked — `test_ModType` now fails with a
completely different error, `AttributeError: 'VGRemaps' object has no attribute 'updateRepo'`, at
`setupMod`'s `self._vgRemaps.updateRepo(...)` call. Same practical effect (these modules are still
broken, still pre-existing, still not worth chasing as a regression from unrelated work) — but if
you're specifically trying to *fix* this cascade, re-derive the current root cause from a fresh
traceback rather than trusting the `ModMappedAssets.updateKeys` diagnosis below; it's stale. The
total test count has also grown a lot since (other sessions adding modules) — last confirmed clean
run was **1391 tests, 0 failures, 37 errors** (same *error count* as the older 1075-test snapshot
below, for whatever that consistency is worth — the specific errors have partially changed
underneath it, not just accumulated). Older snapshot, kept for the parts that are still accurate:
`IfTemplate`/`IfTemplateNode`/`IfTemplateTree`/`CallGraph`/`SectionIterData`/`IniSectionGraph` are
now fully C++-backed with fresh, fully-passing black-box test files of their own (plus a dedicated
`test_GraphTools.py`, split out after the `GraphTools` coverage gap described in
[Architecture](../Architecture/CLAUDE.md)'s deletion-checklist section) — see [Ini Graph
Editing](../IniGraphEditing/CLAUDE.md) — and their deprecated pure-Python `...Old` originals have
been deleted outright, not just renamed, so don't go looking for `test_IfTemplateOld.py`/
`IfTemplateOld.py`/etc.; they no longer exist anywhere in this repo):

- `test_IniFile` — still broadly broken, with several genuinely different root causes (not one
  bug): some tests fail deep in `IniParseBuilder._getBuilderArgs`, others with a `KeyError` on a
  `self.patches[...]` lookup (a mock-patch target string that no longer matches), and more. Don't
  assume a fix to one failing test here fixes the others.
- The GIMI parser/fixer family: `test_GIMIFixer`, `test_GIMIObjRegEditFixer`,
  `test_GIMIObjSplitFixer`, `test_GIMIObjMergeFixer`, `test_GIMIParser`, `test_GIMIObjParser`.
- `test_GraphGroupRemap` (1 error).
- **`test_Mod`, `test_ModType`, `test_ModTypes`, `test_MultiModFixer`, `test_RemapService`,
  `test_ResGroupCollect`, and `test_ResRegCollect` all cascade from the exact same single bug** —
  `ModMappedAssets.updateKeys` (`model/assets/ModMappedAssets.py`), `TypeError: string indices
  must be integers` at `stack.append((childKey, val[childKey], depth + 1, addState))`. Confirmed
  via full traceback for each: `test_Mod`/`test_ModType` hit it directly constructing
  `Indices()`/calling `.map`; `test_ModTypes`/`test_RemapService` hit it indirectly via
  `GIBuilder.amber`'s `Indices(map = ...)` (itself reached through `DeferredEnum.value` /
  `StrEnum._setupAhocorasick`); `test_MultiModFixer`/`test_ResGroupCollect`/`test_ResRegCollect`
  hit it in their own `setUpClass` building a custom `ModType`. **A single fix to this one method
  would very likely clear all 7 modules at once** — don't treat these as 7 separate bugs to
  investigate independently. Also note: a `setUpClass` failure aborts every test in that class
  silently, so the "1 error" the summary shows per module understates how many individual tests
  are actually blocked underneath it.
This list will keep drifting as the maintainer continues fixing modules — treat it as "expect some
unrelated red, but verify which red" rather than a precise, permanent inventory. If you're about to
spend time on a module not listed here, or need to confirm one of these is still actually broken,
just re-run it (`py -3 main.py SomeTestClass -v`) rather than trusting this snapshot.

- **The pure-Python `GIBuilder` (`constants/GIBuilder.py`) is part of this same broken cascade** —
  every one of its 43 classmethods (`amber()`, `raiden()`, ...) constructs a pure-Python `ModType`
  via `Indices(map = ...)`, which routes into the broken `ModMappedAssets`/`VGRemaps` machinery
  above. **Don't cross-check a new C++-side builder (e.g. `CppGIBuilder`) against the live
  pure-Python `GIBuilder` in a formal test** — confirmed by actually calling it
  (`FRB.constants.GIBuilder.GIBuilder.amber()` inside a `Testing/Unit Tester`-style import) rather
  than trusting either version of the diagnosis above. Compare against hardcoded expected literals
  or against a reliable, unrelated C++-side source of truth instead (`test_CppGIBuilder.py` checks
  each method's `name`/`modTypeId` against `ModTypeIdTools.getName`/`getEnum`, both unaffected by
  this bug, plus a few hardcoded alias-list spot checks) — this is fine to do in a throwaway
  verification script even while it stays unsafe to depend on in the committed suite.

- **`test_IniClassifier.py` tests a different, older, pure-Python `IniClassifier`/
  `IniClassifierBuilder` pair — not any new C++-backed classifier work.** The live pure-Python
  original has already been renamed to `IniClassifierOld`/`IniClassifierBuilderOld`
  (`model/strategies/iniClassifiers/`), but `test_IniClassifier.py` still constructs
  `FRB.IniClassifier(...)` by its old bare name, which no longer exists —
  `AttributeError: module 'src.py.FixRaidenBoss2' has no attribute 'IniClassifier'`, one of the
  pre-existing errors in the count above. If you're asked to test "the classifier" and it turns out
  to mean the new C++ `AGRemapCore::IniClassifier`/`CppIniClassifier` pybind binding (see
  [Architecture](../Architecture/CLAUDE.md)'s note on that class having gone unbound to Python for a
  long stretch), the right file is the separate `test_CppIniClassifier.py` — don't edit
  `test_IniClassifier.py` for that work, and don't be misled by the name collision into thinking
  the new classifier already has coverage just because a same-named test file exists.

**`test_RegSurroundedAdd`, `test_IniSectionGraph`, `test_IfTemplate`, `test_IfTemplateNode`,
`test_IfTemplateTree`, `test_CallGraph`, and `test_SectionIterData` are *not* on this list** — all
are clean, comprehensive, and fully passing, as of (in order) a full fixpoint/reachability redesign
of `RegSurroundedAdd`, a follow-up extraction of its reusable graph machinery into
`IniSectionGraph`/`GraphTools`/`CallGraph`, and later a full C++ port of `IniSectionGraph`/
`IfTemplate`/`IfTemplateNode`/`IfTemplateTree`/`CallGraph`/`SectionIterData` with fresh black-box
test files for each (see [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)). If any of these starts
failing, treat it as a real regression from your change, not pre-existing noise — don't assume it
belongs on this list just because an earlier version of this file once listed
`test_RegSurroundedAdd` here.

Don't chase those down as regressions from your work — scope your "did I break anything" check to
the test module(s) actually relevant to what you touched (plus anything that imports it), not a
full-suite green bar. If genuinely unsure whether a failure is pre-existing, check on a clean
`git stash` before attributing it to your change.

## Integration Tester (`Testing/Integration Tester`)
End-to-end tests of the actual script/API output. Run from that directory:
```bash
py -3 main.py [command name]
```
See its own `README.md` for the command list — not something exercised deeply while writing this
file, so defer to that file and to CI's `integration-test-workflow.yml` as the source of truth.

## What CI actually runs
`.github/workflows/unit-test-workflow.yml` / `integration-test-workflow.yml` do exactly
`pip install -r requirements.txt` then `python3 main.py`, on Linux, with **no C++/Cython build
step**. Earlier drafts of these docs claimed this runs against "whatever binary is already
committed" — that's wrong: `*.pyd`/`*.so` are both gitignored (verified with `git check-ignore`/
`git ls-files`), so nothing is committed for CI to fall back on. `FixRaidenBoss2/__init__.py`
does unconditional `from .core import ...` / `from .CyDictTools import ...` at module load, with
no fallback path, so whether CI's fresh-checkout job can even import the package at all is
unverified from this angle — don't assume a green CI run validates a `core`/`cy` change, and
don't assume a red one is your fault either without checking. What's certain regardless: rebuild
locally (see [Building](../Building/CLAUDE.md)) before trusting local test results for any
`core`/`cy` change — the installed copy under `api/src/py/FixRaidenBoss2/` is what the local
suite actually imports, and it only updates when you rebuild.

**Verifying via the Bash tool vs. the PowerShell tool matters here.** A rebuilt native
extension (`core.pyd`, `CyDictTools.pyd`, `CyListTools.pyd`, ...) fails to import when Python is
invoked through the Bash tool's Git Bash (`ImportError: DLL load failed ... The parameter is
incorrect`) but imports fine from the PowerShell tool — this is a tool/environment quirk, not a
real failure. See [Building](../Building/CLAUDE.md)'s "Verifying a build/binding change in Python
directly" for the confirmed repro. Run the Unit Tester itself (`py -3 main.py ...`) from the
PowerShell tool when testing anything that touches `core`/`cy`.
