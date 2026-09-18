# Testing

> **BUILD THE STANDALONE TESTS BY GLOB, NEVER BY A LIST YOU TYPED (2026-09-12).** `core/tests/
> *.cpp` are built by no target, so whatever script you use to compile them *is* the coverage --
> and a list grown by hand, one suite at a time, as each session needed one, silently stops at
> whatever that session cared about. Measured: such a list ran **24 of 46** files. What was
> hiding in the other 22:
>
> * `IniNamingTools_test` -- four assertions broken that same morning by a deliberate behaviour
>   change (`getFixedFile` now runs its result through `FileService::pathToIniStr`, so a path
>   inside a `.ini` comes out `a\b\foo` rather than the mixed `a/b\foo` the test still
>   expected). A real break, in the session's own commit, invisible for a day.
> * `IfPredPart_test`, `Z3Predicate_test`, `Z3IfPredGenerator_test` -- never compiled by that
>   runner at all. They include `tools/z3/Z3Internal.h`, a **private** header that lives under
>   `core/src` rather than `core/include`, so the compile line needs `/I <core>/src` alongside
>   `/I <core>/include`. Without it they fail with `C1083: Cannot open include file`.
>
> So: iterate `core/tests/*_test.cpp`, add `core/src` to the include path, and expect **45 of
> 46** to build and pass on a clean tree. If more than that fails, check whether another agent
> is mid-edit before assuming it is yours -- a header changed in the working tree against a
> `.lib` built before it gives `LNK2019 unresolved external` on a signature that plainly exists,
> which reads like a broken test and is really two people in one checkout.
>
> **AND CALL `vcvarsall.bat` ONCE FOR THE WHOLE GLOB, NOT ONCE PER SUITE (2026-09-15).** The
> obvious way to run them all is a loop that `call`s a single-suite `.bat`, and that script starts
> with `vcvarsall`. Each call APPENDS to `PATH`; after about four the environment overflows and the
> run dies with **exit 255** -- having already printed `ok` for the four it managed. That output
> reads exactly like a short suite finishing cleanly, and the summary line never prints, so the
> only tell is counting the `ok`s against `ls core/tests/*_test.cpp | wc -l`. Set the environment
> up once in the outer script and loop the `cl` invocation inside it, and **print the suite total
> next to the pass count** so a truncated run is visible.

<br>

> **AND A CHECK YOU WROTE FOR THIS FIX MUST BE RUN AGAINST THE BROKEN BUILD FIRST (2026-09-14).**
> Neither suite above can see the bugs that matter most here, so most real verification in this
> repo is a scratch script that fixes a mod and asserts something about the output. Such a script
> is written *after* the fix, which means its failing branch has never executed -- and three of
> them passed a broken fix in a single session, one of which shipped and was reported back from
> in game. **Keep the previous output directory, point the new check at it, and require it to
> FAIL before you point it at the new one.** It costs one command. Two ways they passed anyway,
> both worth checking for by eye: a loop over inputs that `continue`s past one it cannot find
> (a missing input is not a pass), and a property that is true either way -- "the draw exists"
> and "the draw runs on every path" differ by exactly the bug. The full three are
> [Overview](../Overview/CLAUDE.md)'s habit **34**.

<br>

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
py -3 main.py ClassA ClassB ClassC     # several classes in one run (confirmed: 21 classes / 316 tests in ~2 s)
```
Run it from the **PowerShell** tool (the Bash tool cannot import the native `.pyd`, see
[Building](../Building/CLAUDE.md)). A `setUpClass` error for a whole class shows up as one `E` per
class in the dotted progress line, so "3 errors" can mean three entire classes never ran --- read
the tracebacks before deciding your change is clean.
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
  `# TODO: Add tests for Xxx class` placeholder**, not a genuinely missing file — originally
  confirmed for `test_RegAdd.py`, `test_RegNewVals.py`, `test_RegRemap.py`, and
  `test_RegRemove.py`. `Write` refuses to overwrite a file you haven't `Read` first, so `Read` it
  (even though you already know it's just the TODO line) before writing the real test module over
  it, and don't assume the absence of a `find`/`Glob` hit for some other naming guess means no test
  file exists yet — check the exact `test_<ClassName>.py` path directly. **Those four specific
  files are no longer placeholders** — all four (plus a new `test_BaseRegEdit.py`) are real,
  fully-passing black-box suites as of the C++/pybind11 port of the whole `regEdits` family (see
  [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)); the rule itself still holds for other stubs.
  The same is now true of the `graphEdits` family: `test_BaseIniGraphEdit.py` and
  `test_RegFillMissing.py` are new, real suites (neither existed in any form before, not even as a
  TODO placeholder), and the pre-existing `test_GraphRename.py` was kept and extended in place
  rather than renamed — its `assertIs(edit.renameFunc, fn)` opener is exactly the kind of
  behavioural contract that constrains the binding's internals, per the bullet below.
- **For a full-replacement port, read the class's existing `test_Xxx.py` *before* designing the
  binding — it is a behavioural contract, and it routinely constrains binding internals rather
  than just outputs.** The obvious reading of "port this class to C++" is that the tests are a
  pass/fail check you run at the end; in practice they encode decisions the pure-Python original
  made implicitly. Confirmed porting `regEdits`: `test_RegRemap.py`/`test_RegRemove.py` each open
  with `self.assertIs(edit.someArg, theDictIPassedIn)`, which rules out the otherwise-natural
  "parse the dict into a C++ member and rebuild it in the getter" binding outright (see
  [Architecture](../Architecture/CLAUDE.md)'s three options for what to do instead). Discovering
  that from a red test after the binding is written costs a redesign plus a rebuild; discovering it
  from a five-minute read costs nothing. The same read also tells you which behaviours are already
  pinned and must survive (`assertIs` on the *returned* object, `Ranges.createFull()`/
  `createEmpty()` edge cases, an unbounded range endpoint falling back to `len(part)`).
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
- **When you promote an assertion from a scratchpad verification script into a formal test,
  re-derive the expected value against the *test file's own* fixture — don't carry the literal
  across.** The empirical-verification-before-formal-tests habit (see
  [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)) is right, but the throwaway script almost never
  builds the same graph/part the test's `makeXxx` helper does, and for this subsystem the expected
  value usually depends on the fixture's exact `KVP`s. Confirmed: a colouring assertion copied from
  a probe whose `parent` section happened to hold a `b` key became `[["b"], ["b"]]` in a test whose
  helper's `parent` has no `b` at all — the correct answer there was `[[], ["b"]]`. The test failed
  for a *good* reason (the code was right, the literal was wrong), which is a genuinely confusing
  way to start debugging. Read the fixture helper, then write the literal.

### Read the "Ran N tests" line before the failure count — a *smaller* failure count often means less ran

The suite can degrade silently instead of failing loudly, and the degraded run looks *better* at a
glance. Confirmed concretely on Linux: when the `orderedset` module can't be imported, mod-type
registration aborts partway and ~176 parameterised tests are simply never generated.

| | degraded | healthy |
| --- | --- | --- |
| `Ran N tests` | 1655 | 1831 |
| errors | 19 | 73 |
| runtime | 135.9s | 11.4s |

Read down that table and the broken run is the one that looks healthier. **Always compare the test
*count* against the baseline first; only then the failures.** A big runtime jump is the other tell
(there, ~30 failed lazy `pip` installs each shelling out to the network — see
[Setup](../Setup/CLAUDE.md)'s `orderedset` section; `orderedset` is a dead package that cannot
build on Python 3.12, so any environment that hasn't got it installed already will hit this).

### Current baselines, per platform

Both from the same commit, same day, so they're directly comparable:

| | Windows (Py 3.13) | Linux (WSL2, Py 3.12) |
| --- | --- | --- |
| Tests | 1831 | 1831 |
| Errors | 73 | 73 |
| Failures | 6 | 15 |

The **error** count and its breakdown are identical across platforms — so none of the pre-existing
WIP breakage below is platform-specific, and an error that appears on only one OS is a real signal.

The 9 extra Linux failures are deterministic (identical sets across repeated runs, so not
`unordered_map` ordering flakiness):
- **8 are tests hardcoding Windows paths** (`test_IniResource`, `test_IniFixResourceModel`,
  `test_RemapBlendResource`, `test_RemapTexAddResource`) — asserting against literals like
  `'C:/mods/EiRemap/EiBlend.buf'`. Test-side assumptions, not product bugs; path resolution itself
  is correct on Linux.
- **1 is unexplained and worth treating as a real open question**, not baseline noise:
  `test_IfTemplateTree.test_nestedAndElifBranches_multiLevelTree` fails `3 != 1` on node part
  counts. Deterministic per platform but differing between them — the signature of iteration-order
  dependence over an unordered container, though that remains a hypothesis. **RESOLVED 2026-09-18: it was iteration order,
  and it was a real (if harmless-to-output) product defect.** `IfTemplateNode::children_` was a
  `std::unordered_map` keyed by node id; MSVC iterated it in insertion order and libstdc++ in
  REVERSE, so the `children` dict Python receives listed a chain's branches backwards on Linux only.
  It is a `tsl::ordered_map` now, insertion (= branch) order everywhere. Core's own two readers fold
  the children with an `and` and a set union, which is why no fix output ever differed.

### The numbers above are a *snapshot*, and the migration moves them constantly

As of **2026-09-03** the Windows suite is **1859 tests / 0 failures / 105 errors**. Do not treat
that as a target either — read the count, then classify. The 105 are two unrelated pre-existing
groups, and knowing which is which saves re-deriving it:

- **~83 — the `baseIniFileTest.py` fixture.** That file is the shared base for eight test modules
  (`test_GIMIFixer`, `test_GIMIParser`, `test_RemapIniRemover`, `test_ResRegCollect`,
  `test_ResGroupCollect`, `test_GraphGroupRemap`, `baseIniObjTest`, and its own). Its `setUpClass`
  used to construct a pure-Python `IniClassifierOld`/`IniClassifierBuilderOld` pair, then
  `createIniFile` built **pure-Python `ModType`s** carrying per-version `Hashes`/`Indices` maps and
  registered them into it via **regexes**. **As of 2026-09-03 that whole pure-Python
  `model/strategies/iniClassifiers/` package (and the live `constants/GlobalIniClassifiers.py`
  module that depended on it) has been deleted outright** — nothing on the live `.ini`
  classification path ever used it (`Mod.py` constructs `IniFile` with no `iniClassifier` argument,
  which already defaulted to the C++ `GlobalIniClassifiers::classifier()` singleton), so deleting it
  was a pure cleanup, not a functional change. But `baseIniFileTest.py` itself was never updated off
  the deleted classes, so its `setUpClass` now fails immediately with
  `AttributeError: module 'src.py.FixRaidenBoss2' has no attribute 'IniClassifierOld'` instead of
  getting partway through and failing later on old constructor keywords — a different symptom of the
  same still-open, still out-of-scope root cause: `CppModType` exposes no
  `hashes`/`indices`/`vertexCounts`/`vgRemaps` at all, `IniClassifier.addGIModType` (graduated from
  `CppIniClassifier` to the bare name once the Python original was deleted — see
  [Architecture](../Architecture/CLAUDE.md)) takes plain keyword sets rather than regexes, and there
  is no C++ `IniClassifierBuilder`. **Fixing these is the ModType/classifier migration, not whatever
  you are doing** — unless that *is* your task, in which case these eight modules are your
  acceptance criteria. Re-verify the exact current failure/error count before trusting the "~83"
  figure — a `setUpClass` failure errors out every test in the class at once, which may shift the
  total from what it was when individual test methods used to fail deeper in the call stack.
- **22 — `test_RemapService`, `AttributeError: HardTexDriven`.** `remapService.py` and
  `controller/CommandBuilder.py` reference `DownloadMode.HardTexDriven`, but the enum defines only
  `Always`/`Disabled`/`Normal` on both the Python and C++ sides. A genuine unimplemented mode; do
  not invent the enum member to make the tests pass.

### Later the same day: 1809 tests / 0 failures / 30 errors (Logger port)

Measured twice on 2026-09-03 after the `IniFile` port landed, once on a clean stash (1770 / 0 / 30)
and once with the C++ `BaseLogger`/`Logger` port applied (1809 / 0 / 30 --- the +39 is exactly the
new `test_Logger.py`/`test_BaseLogger.py` tests). The 30 errors sit in the same nine modules either
way: `test_GIMIFixer`, `test_GIMIParser`, `test_GraphGroupRemap`, `test_RemapIniRemover`,
`test_ResGroupCollect`, `test_ResRegCollect` (the `baseIniFileTest.py` fixture group above),
`test_Mod`, `test_ModType`, and `test_RemapService`. The drop from 105 happened *before* the Logger
port and was not investigated by it --- re-measure rather than reasoning from either figure.

Two Logger-specific notes for that suite: `test_Logger.py` is a black-box rewrite against the bound
`Logger` (its output is captured by patching `builtins.print`/`builtins.input`, which the binding
routes through on purpose --- see Architecture's "The view is C++ now"), and `test_BaseLogger.py`
is where the trampoline is exercised (a Python subclass overriding `write`, `log`, `openHeading`,
`error`, `getStr`, ... is reached from C++ callers like `openHeading` -> `log` -> `write`). The
`self.patch(...)` helper returns `None`, not the mock --- capture calls through a `side_effect`
instead of `assert_called_once_with`.

### As of 2026-09-05: **1930 tests / 0 failures / 7 errors** (superseded --- see the 2026-09-17 box below)

The `RemapService` migration landed and took two whole test modules with it. **`test_Mod.py` and
`test_RemapService.py` are deleted** --- their subjects (`model/Mod.py`, `remapService.py`) no longer
exist --- which accounts for the entire drop from the 30 above: 22 erroring tests in
`test_RemapService` (the `HardTexDriven` group described two sections up --- that entry is now
**resolved by deletion**, not by fixing) plus one `setUpClass` error in `test_Mod`. Attribute a
baseline change that precisely before reporting it; a big drop in the error count is as often a
module that stopped *running* as one that started passing.

The count went 1913 -> **1930** later the same day when `test_RemapServiceCLI.py` was added (17
tests for the binding surface --- see "A C++ class bound to Python needs a Python test").

The surviving **7** are one group, all `setUpClass`, all the `baseIniFileTest.py` fixture story
above: `test_GIMIFixer`, `test_GIMIParser`, `test_GlobalRemapIniRemover`, `test_GraphGroupRemap`,
`test_RemapIniRemover`, `test_ResGroupCollect`, `test_ResRegCollect`. Nothing else errors, so **any
eighth error is yours**.

### The `.ini` fixture classes run on the C++ `IniFile` (2026-09-17)

`baseIniFileTest.py` and its seven subclasses (`test_GIMIFixer`, `test_GIMIParser`,
`test_GlobalRemapIniRemover`, `test_GraphGroupRemap`, `test_RemapIniRemover`, `test_ResGroupCollect`,
`test_ResRegCollect`) had not run a single test since the pure-Python `IniFile` was deleted. They do
now, and how they were made to run is the template for any new test that needs a real `.ini` file:

- **It derives from `BaseUnitTest`, not `BaseFileUnitTest`.** The latter's `os` mocks break
  `tempfile` ("No usable temporary directory") and the C++ side never sees them anyway. Write the text
  to a REAL file in a `tempfile.mkdtemp()` folder (`writeIniTxt`) and construct
  `FRB.IniFile(file = ...)` on it; re-write and `clear()` between cases.
- **Strategies go in through `CppStrategyOverrides`, not by assigning `_iniParser`/`_iniFixer`.**
  `useStrategies(parser, fixer)` registers factories returning the test's own objects and adds a
  cleanup that clears them --- the overrides are process-wide, so a test that forgets leaks its
  strategy into every later class.
- **Made-up mod types are runtime `FRB.ModType`s passed as `overrideModTypes`**, with
  `Hashes`/`Indices` rows added through `addRepoRows`, and the fixture builds its OWN
  `FRB.IniClassifier` (`addGIModType(ModTypeIdData, hashes, keywords)`) rather than relying on the
  global one. Custom types borrow Raiden's builders.

What the port of the expectations taught, each of which cost a debugging cycle:

- **A looped test stops at its first mismatch, so every later case in it is UNVERIFIED.** Fixing the
  first diff of `test_ResGroupCollect` exposed count failures in cases that had never executed. When
  regenerating expectations, run with an `assertEqual` that records instead of raising, so you see
  every case at once --- and then READ the diff, because a recorded "actual" is not a correct one.
  One regenerated parser expectation had to be reverted: changing a filter's `includeKeyDefs` to make
  an old expectation reachable also edited a section the filter exists to exclude.
- **Behaviours that changed on purpose, and what the tests now say:** an `.ini` file with no mod
  sections is not classified, and `IniFile.fix()` returns `{}` for it (pure Python wrote a bare credit
  block); a group built by `ResGroupCollect` is in `getGroupedResources()`, not `getResources()`;
  paths inside the `.ini` are Windows paths with `\`; `TexCreate` mints ONE name per mod; one blank
  line, not two, precedes the credit block; `IniFileFixContext::modsToFix()` is empty by design, so a
  fixer bound to a C++ `IniFile` fixes only an explicit `modsToFix`; download graphs come back in
  the order the parse found them, not the order of `downloads`; and an edit's `keyFilters` ranges
  with `includeKeyDefs = False` exclude the key lines themselves, so a `RegNewVals` on that key
  cannot fire.
- **A Python-built section is kept alive by the EDIT that built it.** `MyCreate(...).buildResources(...)`
  on a temporary left the graph pointing at freed memory: the test passed alone and failed after
  `test_ResRegCollect`, and a check repeated in one process flipped between 1 and 0 parts. Hold the
  edit in a variable. A test that passes alone and fails in the suite with an `IndexError` on
  `parts[0]` is this, not ordering.
- **Reusing a Python-built parser across `IniFile.clear()` used to corrupt the heap --- FIXED
  2026-09-17, and the looped fixture tests reuse one parser again.** Nothing failed where it
  happened; the WHOLE run died tests later with `Windows fatal exception: access violation` inside
  `test_GraphInherit`, a class sharing no code with the culprit. The cause: the parser's
  `PyIniGraphGroups` view never let go of a graph, and every graph's keep-alive pins pybind11
  wrappers for the `.ini` file's sections and parts. `IniFile.clear()` freed those objects under
  wrappers that stayed REGISTERED at the freed addresses, so a later `IfTemplate` allocated at the
  same address was cast back to the stale wrapper --- `IniSectionGraph({"s": IfTemplate(...)})` then
  pinned the stale one, and the inline `IfTemplate` died under its graph. Fix:
  `PyGIMIParser::clear` calls `PyIniGraphGroups::releaseDetached`, and `IniFile::clear` clears its
  built strategies BEFORE freeing its sections (see Architecture's keep-alive section). Pinned by
  `GIMIParserTest.test_clear_releasesThePreviousParsesGraphs` and the two `test_reusedAcrossClear_*`
  tests, which FAIL on the old build (413 and 435 of 500 inline sections got a stale wrapper) rather
  than crashing somewhere else. Two things about the hunt: the earlier "clear the strategies first"
  experiment was NECESSARY but not sufficient, because `PyGIMIParser::clear` itself released
  nothing; and the crash was only nondeterministic at the crash site, while an identity check
  (`graph.sections["s"] is section`) showed the mechanism deterministically, with no rebuild and no
  debugger. If a suite run dies with no traceback, set `PYTHONFAULTHANDLER=1` to get the Python
  frame, then bisect CLASSES before tests --- and do not trust `PYTHONMALLOC=debug` here, which
  reports a corrupt block at shutdown for a bare `import FixRaidenBoss2`.

Four binding bugs these tests found, all fixed, each invisible to every other suite because nothing
else builds a strategy from Python and runs it through the C++ `IniFile`: the remover's context did
not delegate to the core `IniFileRemoveContext`; a strategy built without a `modTypeId` never fell
back to the `.ini` file's classified type (so no section name carried the mod's prefix); a parser
DISOWNED the caller's `FileDownload` into the `RemapIniDownload` it built (the second parse raised
"Python instance was disowned"); and a `PyGIMIFixer` never took `GIMIFixer::snapshotPreEditSectionNames()`
--- so `hideOrig` commented out NOTHING for any fixer built from Python, while the compiled fixers,
whose core `getFix` takes the snapshot, were fine. `RemapBlendReplace` with no VG remap for the target
now builds a plain `IniFixResource`, as core's `VGRemapBlendReplace` does, instead of raising.

**Fixed the same day, and it was two bugs, not one:** a `ResGroupCollect` resource edit reached from
two or more call sites of one mod lost every call site but the last (the two `oneTexCreateManyCallSites`
tests were pinned as `expectedFailure` for it), AND any resource graph replicated for a second group
rendered that replica as an empty `[]` section. The second was not about `TexCreate` at all --- a
plain blend copied twice did it too --- and it only LOOKED like part of the first because the first
was what made a created texture get replicated. See [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)'s
"`ResGroupCollect` keys call sites by LOCATION". The re-derived expectations differ from the
pure-Python ones only in naming (one `NormalMap` per mod, a replica suffix per group) and the one
blank line; `test_editOneResourceManyCallSites_everyCallSiteRewritten` and
`test_editResourceInSeveralGroups_everyReplicaWritten` pin the buffer half, and all four fail against
the previous build.

### `mock.patch` targets are `src.py.FixRaidenBoss2`, not `src.FixRaidenBoss2`

The Unit Tester imports the package as `src.py.FixRaidenBoss2`, so that is the prefix every
`mock.patch("...")` string needs. Around 21 of them across `test_RemapService`, `test_Mod`,
`test_CppTrie` and `test_DFA` had the shorter form and produced
`ModuleNotFoundError: No module named 'src.FixRaidenBoss2'`. Worth knowing not just for the fix but
for the shape of the bug: **a wrong patch target masks whatever error is underneath it**. Correcting
these did not change the error count at all — it swapped 22 `ModuleNotFoundError`s for the 22
`HardTexDriven` ones above, which were the real problem all along.

**Rebuild before comparing against these numbers**, and on a checkout shared between Windows and
Linux remember that a plain build on either OS deletes the other's installed binaries (see
[Overview](../Overview/CLAUDE.md)) — testing right after the *other* platform's build measures a
stale or missing extension, not your change.

### Compare failure *identities* against the baseline, not just the counts

"6 failures / 73 errors, same as before" is a weaker claim than it looks: a change can fix one test
and break another and leave the totals untouched. Print the names and diff those:

```powershell
$o = py -3 main.py 2>&1
$o | Select-String -Pattern "^(Ran |FAILED|OK)" | ForEach-Object { $_.Line }   # .Line, or the PowerShell tool dumps whole MatchInfo objects
$o | Select-String -Pattern "^FAIL: " | ForEach-Object { $_.ToString() }
$o | Select-String -Pattern "^ERROR: " | ForEach-Object { ($_.ToString() -split "\(")[1] } | Sort-Object -Unique
```

On Linux (or anywhere the Python runner is easier than PowerShell), `Tools/Misc/Diagnostics/
unitTestIds.py <out.txt>` writes the IDs to a file to `diff`, and `--api <copy>` runs the same suite
against an OLD build without touching the shared module --- Overview's habit 50 has the recipe.

Also expect the **total** to drift as you add tests, so don't treat a changed "Ran N" as a red flag
on its own --- reconcile it against what you added. And when a number you quoted earlier no longer
matches, re-measure rather than assuming: an earlier figure in a long session is easy to quote
stale.

### The harness's mocks hand out **shared mutable state** --- one test can poison the rest of its class

`baseIniFileTest.setUp` patches `FileService.read` to return the class-level `_iniTxtLines` list.
`IniFile._commentSection` edits the lines it is given **in place**, so before this was fixed, a
single test running `fix(hideOrig = True)` left every later test in that class reading an
already-commented `.ini` file --- and because tests run alphabetically, which tests broke depended
on their names. The patch now returns `list(self._iniTxtLines)`, matching what a real
`FileService.read` does.

The general lesson, which applies to any new mock you add here: **if production code may mutate what
a mock returns, return a fresh copy per call.** A mock that hands out one shared object is shared
state between tests, and the symptom (a test that passes alone and fails in the suite) reads like a
product bug rather than a harness one.

Two related traps already documented elsewhere in this file, worth re-linking mentally: a class that
becomes C++-backed stops honouring the Python-level `os.path`/`open` mocks entirely, and a
`Py*` strategy context must forward to Python for exactly that reason (see
[Architecture](../Architecture/CLAUDE.md)'s context-seam section).

### A C++-side behaviour swap can show **zero** test delta and still be unverified

When you repoint something live at a ported class, check whether any *passing* test actually covers
that path before calling the suite evidence. Confirmed the hard way: repointing
`ModType.__init__` and `IniFixBuilderData.giDefault` from `GIMIFixerOld` to the C++ `GIMIFixer`
moved not a single test, because every class exercising the default fix path
(`test_ModType`, `test_Mod`, `test_RemapService`, `test_MultiModFixer`, `test_GIMIFixerOld`) was
already in the known-broken error list. Say so plainly in the write-up rather than reporting an
unchanged baseline as a pass.

### Known-broken/WIP test modules — don't chase these as regressions
**Not every test module in `Tests/` is finished/passing right now** — some are known
work-in-progress from the maintainer and fail for reasons unrelated to your change. The maintainer
has been actively fixing these incrementally (a large batch — `test_FileService`,
`test_BaseSLR1Parser`, `test_IfPredTokenizer`, `test_IfPredParser`, `test_SympyParser`,
`test_IfPredLogicGenerator`, `test_SympyIfPredGenerator`, `test_IntTools`, `test_Version`,
`test_IfTemplateNormTree`, `test_IfTemplateTree`, and the old pre-C++-port `test_IfContentPart` —
all went from broken to fully passing in one pass), so **don't trust this list blindly; re-run and
re-verify rather than assuming stale entries are still accurate**, in either direction.

> **Current baseline --- Windows, 2026-09-17: 2288 tests, 0 failures, 0 errors, 0 expected
> failures** (measured twice in a row, on the build carrying both of that day's binding/core fixes).
> The seven `baseIniFileTest.py` classes described in the box below RUN now, on the C++ `IniFile`
> and the C++ `IniClassifier` (see "The `.ini` fixture classes run on the C++ `IniFile`" further
> down), and the two `expectedFailure`s that box used to carry are gone --- the `ResGroupCollect`
> bug they pinned is fixed. **Anything red is yours.**
>
> **Linux, 2026-09-18: the same 2288 tests, 0 failures, 0 errors** --- see "The first CI run" for
> the three causes of the 11 below, two of which were product bugs, not the test-side assumptions
> this box used to call all of them.
>
> *Before that* --- **Linux, 2026-09-17: the same 2288 tests, 0 errors, 11 failures**, then read as a
> test-side assumption that predates this work --- **10** assert a Windows path literal
> (`self.assertEqual(r.srcPath..., "C:/mods/shared/EiBlend.buf")`, which POSIX reads as a RELATIVE
> path and prefixes with the launch directory) in `test_IniResource`, `test_IniFixResourceModel`,
> `test_RemapBlendResource`, `test_RemapTexAddResource`, `test_BaseResEdit` and `test_ResEdits`, and
> the 11th is the open `test_IfTemplateTree.test_nestedAndElifBranches_multiLevelTree` question below.
> The two Linux failures the box further down lists as stale (`texCreate_numbersSuccessiveTexturesApart`
> and `versionAndDownloadModeConvert`) are gone, and so are its 7 errors. **The two platforms now run
> the same NUMBER of tests, which is the first thing to check** --- a Linux count below Windows means
> registration aborted, not that fewer tests exist.
>
> **Previous baseline — verified 2026-09-06: 2005 tests, 0 failures, 7 errors, all from ONE cause.**
> **Re-verified 2026-09-08: 2038 tests, 0 failures, still exactly these same 7 errors and the
> same 7 modules.** The count drifts as tests are added (see below); the *identities* have not.
> Every one of the seven is a `setUpClass` error reading
> `AttributeError: module 'src.py.FixRaidenBoss2' has no attribute 'IniClassifierOld'`, and they
> all come from two lines: `baseIniFileTest.py:19-20` calls `FRB.IniClassifierOld()` /
> `FRB.IniClassifierBuilderOld()`, but the package exports `IniClassifier` and `BaseIniClassifier`
> (both C++-backed) and **no `...Old` variant of either** — the test base was left pointing at names
> that don't exist. Blocked modules: `test_GIMIFixer`, `test_GIMIParser`,
> `test_GlobalRemapIniRemover`, `test_GraphGroupRemap`, `test_RemapIniRemover`,
> `test_ResGroupCollect`, `test_ResRegCollect`. A `setUpClass` failure aborts its whole class
> silently, so "7 errors" badly understates how many individual tests are actually blocked.
> **If you see exactly these 7, that is the baseline, not your change** — and if you want to fix it,
> it's a two-line edit in one file, not seven investigations.
>
> **Linux, 2026-09-17: 2191 tests, 13 failures, 7 errors** --- the same 7 errors, and all 13 failures
> accounted for, none from code: **10** assert Windows paths (`C:/mods/...` literals, or a `.\` a
> test expects where Linux writes `./`) in `test_IniResource`, `test_IniFixResourceModel`,
> `test_RemapBlendResource`, `test_RemapTexAddResource`, `test_BaseResEdit`, `test_ResEdits`;
> `test_IfTemplateTree.test_nestedAndElifBranches_multiLevelTree` (`3 != 1`, the open question
> below); `test_ResEdits.test_texCreate_numbersSuccessiveTexturesApart` (TexCreate's names stopped
> being numbered on purpose, see the top-level CLAUDE.md); and
> `test_RemapServiceCLI.test_versionAndDownloadModeConvert`, which still expects `version` to set
> `fromVersion` --- stale since the two were split on 2026-09-13. A suite that dies part way with
> `Bus error` on `/mnt/e` has so far been the mount, not a test: re-run before hunting.
>
> Note how much smaller this is than every snapshot below it: the long `ModMappedAssets.updateKeys`
> / `VGRemaps.updateRepo` / stale-`src.FixRaidenBoss2`-import cascades those describe are **gone**.
> Treat everything after this box as history that explains how the suite got here, not as a
> description of what you'll see today.

Confirmed
right after this file's own warning above was written: the `test_Mod`/`test_ModType`/etc. root
cause named two paragraphs below (`ModMappedAssets.updateKeys`, `TypeError: string indices must be
integers`) had *already* drifted by the time it was re-checked — `test_ModType` now fails with a
completely different error, `AttributeError: 'VGRemaps' object has no attribute 'updateRepo'`, at
`setupMod`'s `self._vgRemaps.updateRepo(...)` call. Same practical effect (these modules are still
broken, still pre-existing, still not worth chasing as a regression from unrelated work) — but if
you're specifically trying to *fix* this cascade, re-derive the current root cause from a fresh
traceback rather than trusting the `ModMappedAssets.updateKeys` diagnosis below; it's stale. The
total test count has also grown a lot since (other sessions adding modules) — last confirmed clean
run was **1637 tests, 0 failures, 37 errors** (2026-08-28; the 37 break down as 22 stale
`src.FixRaidenBoss2` import errors, 12+1 missing `IniClassifier`/`IniClassifierBuilder` attributes,
1 `flattenNestedDict`, 1 `VGRemaps.updateRepo` --- none of them related to the C++ core) (same *error count* as the older 1075-test snapshot
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
  investigate independently. **Two of the seven are gone as of 2026-09-05**: `test_Mod` and
  `test_RemapService` were deleted along with their subjects (`model/Mod.py`, `remapService.py`), so
  they can no longer be used to reproduce this — the remaining five still can. Also note: a `setUpClass` failure aborts every test in that class
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

- **`test_IniClassifier.py` used to test a different, older, pure-Python `IniClassifier`/
  `IniClassifierBuilder` pair, with a name collision against the new C++-backed classifier's own
  test file — that collision is resolved now, don't trust an older note describing it as live.** As
  of 2026-09-03 the whole pure-Python `model/strategies/iniClassifiers/` package (the
  `IniClassifierOld`/`IniClassifierBuilderOld`/etc. deprecated classes this file used to test) has
  been deleted outright, and the C++-backed classifier's binding graduated from `CppIniClassifier`
  to the bare `IniClassifier` name (see [Architecture](../Architecture/CLAUDE.md)). `test_IniClassifier.py`
  now IS the test file for the C++-backed classifier — `test_CppIniClassifier.py` was renamed into
  it (replacing the old broken file at that path) rather than living alongside it. If you're asked
  to test "the classifier," this is the one file, no name-collision trap to route around anymore.

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
`git stash` before attributing it to your change. The exact recipe that worked, as one PowerShell
call so nothing is left stashed if a later step fails: `git stash push -u -q -m wip; <run the suite
into a variable>; git stash pop -q`. `-u` matters (your new test files and new `core/` sources are
untracked), and the rebuilt `.pyd` deliberately stays in place because it is gitignored — that is
still a valid baseline, since the restored Python never imports the classes only the new binary has.
Compare the `Ran N` line first: the delta should be exactly the tests you added.

## Deleting or renaming an exported class: the four places the suite cannot see

Every one of these was missed in a single 2026-09-05 change that deleted `remapService.py` and
`model/Mod.py`, and the unit suite stayed green through all four. Work the list.

1. **`apiMirror`.** `import AnimeGameRemap` broke outright (`ImportError: cannot import name 'Mod'`)
   and nothing in either suite imports it. See [Overview](../Overview/CLAUDE.md)'s "`apiMirror` rots
   silently" for the three-second check --- and note that section's "already broken" caveat was
   itself stale, which is how it got skipped.
2. **`FixRaidenBoss2/__init__.py` in the other direction.** Deleting the Python original is only
   half of it; if a bound C++ class is taking over the name, it needs a `from .core import ...` line
   and an `__all__` entry, or the replacement is unreachable from the package namespace. The
   C++ `RemapService` sat bound-but-unexported this way.
3. **`Docs/src/api.rst`.** A deleted class leaves a dangling `autoclass` (harmless only if the block
   was already commented out) and the replacement gets no page at all. For a class that subclasses a
   **bound** base, `:members:` alone documents almost nothing --- `RemapServiceCLI` rendered with
   just its two Python methods until `:inherited-members:` was added, which brought the other eight
   in. Build and check the rendered ids, don't assume:
   ```bash
   grep -o 'id="FixRaidenBoss2.YourClass\.[a-zA-Z]*"' Docs/build/html/api.html | sort -u
   ```
4. **A Python-side test for the binding.** A C++ standalone suite cannot see `py::arg` names,
   exception translation, trampoline dispatch, or how a bound enum crosses the boundary. See the
   next section but one; `test_RemapServiceCLI.py` is the worked example.

## Read a shared edit class's `test_Xxx.py` BEFORE changing its rule, not after

**Testing**'s advice to read a class's existing test as a behavioural contract is usually given for
*porting*. It matters at least as much when you are changing an existing class's semantics, and
skipping it cost two full build-and-revert cycles in one session.

`test_RegDelimitedAdd.py` opens with a header comment stating its invariant outright -- "every
delimiter-free segment (start -> first delimiter, delimiter -> delimiter, **last delimiter -> end of
path**) holds the addition exactly once". A change that dropped that final segment looked obviously
right from the `.ini` output, broke 14 tests, and turned out to be removing a call the reference
genuinely makes. The third attempt made the new behaviour **opt-in with the old one as the default**,
which broke nothing and let the one caller that wanted it ask.

Two rules of thumb:

- **A bound class is external API.** `RegDelimitedAdd` and `RegFillMissing` both have pybind
  bindings, so changing their default behaviour changes it for callers outside this repo. Default to
  a flag.
- **When tests do have to change, change the setup, not the expectation, wherever the test's subject
  allows it.** Thirteen `RegFillMissing` tests failed on the targets-only change; none of them was
  *about* fill scope (they cover `partFilter`/`trackKeys`) and their graph's target list was
  incidental, so declaring both sections as targets kept every assertion measuring what it was
  written to measure. Weakening thirteen expectations would have hidden whatever else moved. Then
  add a test for the contract that actually changed -- it had none.

## A C++ class bound to Python needs a Python test even when its C++ tests are thorough

`core/tests/RemapServiceCLI_test.cpp` has 25 tests and `RemapService_fix_test.cpp` has 35, and
between them they covered **none** of what `test_RemapServiceCLI.py` covers, because the binding is
a separate artifact from the class. What only a Python test can catch:

- **`py::arg` names.** `main.py` passes all sixteen constructor arguments *by keyword*. Rename one
  in the binding and the CLI breaks with a `TypeError` at runtime while every C++ test still passes.
- **Exception translation.** The point of the `raisePyError` machinery in `PyRemapServiceCLI.cpp` is
  that a caller catches `FixRaidenBoss2.exceptions.InvalidModType`, not a `RuntimeError` carrying a
  message. Only Python can assert the *class*.
- **Trampoline dispatch.** That a Python subclass's `printModsToFix`/`addTips` override is actually
  reached from a C++ `fix()`.
- **How a bound value crosses.** Worth knowing before writing assertions: a `DownloadMode` comes
  back as the enum's **string value** (`'normal'`), not as the Python `DownloadMode` member, because
  core has its own enum and the two are mapped by value. `assertEqual(x, FRB.DownloadMode.Normal)`
  fails; `.value` is what you compare. Expect the same shape for any other dual-sided enum.

**What not to assert:** anything about the *content* the fix produces. The `IniFixer`/`IniParser`
strategies are stubbed with their base classes, so a run generates no remapped sections; an
assertion on fix output written today would bake the stub in as expected behaviour and have to be
deleted later.

## A green suite does not mean the product works --- run the real entry point over a real mod

**This is the single highest-value check in the repo and it takes two minutes.** Neither suite can
see the class of bug it catches, because both exercise *steps* and the product is a *sequence*.

Confirmed the expensive way (2026-09-05, during the `RemapService` migration): a default run
**emptied every `.ini` file it touched** --- a 31-line mod came out as 9 lines of credit header, its
own sections gone, and with `--deleteBackup` there was no backup either --- while 10 C++ standalone
suites and 1913 Python tests were green. The bug lived exactly where nothing looked: **undo-only
passed, fix-only passed, only undo-then-fix was broken**, and undo-then-fix is what every real run
does. (Cause, for the record: a removal writes the file back and then clears the `.ini` file's read
cache while leaving it *classified*, and `IniFile::parse()`/`fix()` only re-read on "not classified"
--- see [Architecture](../Architecture/CLAUDE.md)'s "`IniFile`'s read cache".)

The lesson generalises past that one bug: **the C++ tests build their `.ini` fixtures from short
synthetic strings**, so any defect that needs realistic content, or two operations in sequence, or a
file on disk that some earlier step already rewrote, is invisible to them.

> **Do not trim a native command's output with `Select-Object -First N`.** It stops the pipeline,
> which kills the process being read: the command reports **exit 255 / -1** with truncated output,
> which is indistinguishable from the program crashing. Confirmed 2026-09-10 --- a script that
> "failed" under `| Select-Object -First 16` exited 0 through `| Out-String`. Capture the whole
> output into a variable and inspect that instead.

### The check

Real sample mods are already in the repo --- use them rather than writing a fixture:

```
Testing/Integration Tester/IntegrationTester/Tests/APIDocsTests/inputs/fullFix/RaidenShogun/
```

Copy that tree somewhere scratch, delete the `.py` harness files it carries, then drive the **real**
entry point (not `RemapService` directly --- the point is to cover the wiring too):

```python
import sys
sys.path.insert(1, r"<repo>/Anime Game Remap (for all users)/api/src/py")
from FixRaidenBoss2.main import remapMain
import FixRaidenBoss2 as FRB

FRB.Logger.waitExit = lambda self: None      # it blocks on input() otherwise
sys.argv = ["FixRaidenBoss2", "-s", workFolder, "-d"]
remapMain()
```

Run it from the **PowerShell** tool, not the Bash tool (see this file's last section for why a
rebuilt `.pyd` fails to import under Git Bash). Then compare every file's size before and after.

### Reading the result

- **Files that GREW** --- expected. The fix appends to a mod, it does not replace it.
- **Any file that SHRANK** --- stop and investigate. That is the failure mode above.
- **No remapped sections, and `getResources()` empty** --- this used to be the expected state for
  *everyone* and this file said so. **It is not any more (checked 2026-09-07).** Nine characters have
  real fixers, so over one of those you should see remap sections, `RemapBlend.buf` files and (for
  Jean) `.dds` files appear. Over a character with no fixer you still get only the credit header, and
  that is still the stub rather than a bug --- so **check which case you are in** before drawing a
  conclusion, with
  `ls "Anime Game Remap (for all users)/api/src/cpp/core/src/data/IniFixData/"`.
- **`.buf`/`.dds` files unchanged** --- read the summary line the run prints rather than guessing.
  It says exactly what it touched (`Out of the 2 *.dds files within the found mods, editted 2 ...`),
  which distinguishes "nothing was there to fix" from "the fix skipped them".
- **The goldens are current C++ output (regenerated 2026-09-17), not the pre-migration script's.** To
  read what the old script produced, use git history: `git show 87e9e5e9:"Testing/Integration
  Tester/IntegrationTester/Tests/APIDocsTests/expected_<test>/..."`. Regenerate them (on Linux) when a
  change legitimately moves the output, and read what moved first.

### Real mod data: what is in the repo, and which path each fixture exercises

**You do not need the maintainer's GIMI install to verify a fix end to end.** Everything below is
committed, and picking the right fixture is the difference between "I could not test this" and a
two-minute proof. Copy a fixture to a scratch dir first --- never run over the repo's own copy.

| Fixture | Path (under `Testing/Integration Tester/IntegrationTester/Tests/APIDocsTests/inputs/`) | Exercises |
| --- | --- | --- |
| Raiden | `fullFix/RaidenShogun/` | `.ini` rewriting, 4 files across nested folders. **No textures** --- `RaidenFixer` has no `texEdits` |
| **Jean** | `multiFix/select/Jean/` | **the texture path** --- 3 `.ini` files, and the Jean -> JeanSea remap writes `.dds` files |
| Amber | `multiFix/select/Amber/` | the CN-skin remap shape (hashes replaced, indices forward-looked-up) |
| Kequeen | `multiFix/select/Kequeen/` | a `[KeySwap]`/`$swapvar` mod, and several `.ini` files in one mod folder |
| (the whole `select/` tree) | `multiFix/select/` | **`--types` name/alias filtering.** Its own `fullFix_someFixed.py` runs `types = ["kequeen", "aMbEr", "ACTINGGRANDMASTER"]` --- a misspelling, odd casing and an alias, deliberately |
| Old versions | `multiFix/oldVers/` | `--version` / `--hideOriginal`, over an AmberCN mod |

Plus **`Data/Mod Downloads/GI/<Character>/<version>/`** --- real `.dds`/`.buf`/`.ib` assets the
download step pulls from. A run resolves downloads out of here, so a `*RemapDL.dds` in the output is
a byte-identical copy of one of these, not something the fix encoded. **Read from it, never write to
it**, and confirm afterwards with `git status --porcelain -- Data` (must be empty).

**Textures: only Jean and JeanCN currently write one.** Their fixer carries
`config.texEdits = {{"body", "ps-t1", "ShadeLightMap", &JeanShading::liftLowAlpha}}`
(`core/src/data/IniFixData/Jean/JeanFixer.cpp`), which is the only live `texEdits` row in the repo --- so
a change to the texture pipeline that you verify against the Raiden fixture has been verified against
nothing at all. Measured over the Jean fixture, a default run against `-c`
(`--compressTextures`):

| written file | default | `-c` |
| --- | --- | --- |
| `SmollerJeanRemapTex.dds` (50x50) | 10128 | 2852 |
| `CuteJean/JeanBodyLightMapRemapDLRemapTex.dds` (1024x1024) | 4194432 | 1048724 |

**Re-measured 2026-09-10, and the two columns are the other way round from what this table said
until then** --- the byte counts were right, the headings were not, and the earlier `-c` column is
what you now get with *no* flag. `3ed7903` ("Route in the gameType and uncompress options") landed
2026-09-07, the same day the original figures were taken, and after it `-c` genuinely compresses:
the **default** writes RGBA8 and `-c` writes BC, so the default is the 4x-*larger* one. If a run of
yours produces 10128/4194432 without `-c`, that is current correct behaviour, not a regression. (The
large file's name moved too --- the fixer's `texEdits` row is keyed on `"body"`, so it writes
`JeanBodyLightMapRemapDLRemapTex.dds`, not `...Head...`.)

The second row is the interesting one: a texture that is **downloaded and then edited**, so it covers
download -> edit chaining in one file. Those numbers are a useful regression baseline --- exactly 4x
on the large one is RGBA8 against BC.

**Checking a written texture is worth more than checking its size.** A size change alone does not
prove a valid file. Open both with `FRB.TextureFile`, confirm `hasImage` and the dimensions, and
compare `getPixels()` --- they should differ only slightly (~5-9% of bytes, small deltas), which is
BC being lossy. Identical sizes, a failure to open, or wildly different pixels all mean something
else went wrong.

### Two entry-point bugs this check has already caught that no test could

Both were in code no unit test imports, and both made the CLI unrunnable end to end:

- `CommandBuilder.__init__` raised `AttributeError: HardTexDriven` --- its `--download` help text
  named an enum member that had been removed --- so `main.py` died before parsing a single argument.
- The pure-Python `RemapService` could not be *constructed* for the same reason, and once past it,
  walked the sample tree and found **0 `.ini` files**.

If you change anything in `controller/`, `main.py`, or `remapServiceCLI.py`, the suites will not
tell you whether the program still starts. Run it.

## "Is my change the cause?" -- put it behind an env var and answer it in ONE build

When something regresses and you have several changes in the tree, the instinct is to revert one and
rebuild, then revert another and rebuild. At two minutes a header build that is slow, and at ten it
is a session.

Instead, make the suspect switchable at runtime:

```cpp
static const bool guardOff = (std::getenv("AGR_NO_HASHIND_GUARD") != nullptr);
if (!guardOff && !hashInd.has_value()) {
    continue;
}
```

One build, then run the harness twice -- once with the variable set, once without. This settled in a
single cycle that a classifier change was **not** what stopped the Raiden fixture producing a remap
(byte-identical output either way), which redirected the search instead of burning two more builds
confirming a hunch. `static const` means one `getenv` per process, so it costs nothing to leave in
while you work; remove it in the same patch that concludes the investigation.

Pair it with the arithmetic: if a suspect cannot reach the symptom at all -- the fixer in question
registers no downloads, has its own fixer rather than the shared template, and does not use the edit
you changed -- say so and look elsewhere rather than testing it anyway.

## Grepping `core/tests/` for the changed TYPE name is not enough --- grep for its CALL SITES too

`core/tests/*.cpp` are built by nothing, so a `core/` interface change silently breaks them (this is
covered elsewhere in this file). The refinement, learned the hard way on 2026-09-07: **following the
"grep for the changed name" rule to the letter can still leave a test broken.**

The change was `RemapService::gameTypeId` (an `optional<int>`) becoming `gameTypeIds` (an
`optional<unordered_set<int>>`). Grepping `core/tests/` for `GameTypeId` found four files, all
fixed. `RemapService_fix_test.cpp` was not among them --- it never mentions the type. It just
constructs a `RemapService` positionally with `..., AGRC::DownloadMode::Always, 0)`, and that `0` no
longer converts. Nothing in the file names anything that changed.

So when you change a **signature**, grep for the *thing being called*, not only the types in it:

```bash
grep -rln "ClassName(" core/tests/          # constructor call sites
grep -rln "methodName(" core/tests/         # method call sites
grep -rln "<TypeYouRenamed>" core/tests/    # the obvious one
```

Two related traps from the same change:

- **Appending a parameter is not source-compatible when a trailing parameter is commonly passed
  positionally.** `RemapService`'s last parameter is `logger`; inserting `compressTextures` before
  it turned `..., std::nullopt, capture)` into "pass a `shared_ptr` where a `bool` goes". It fails to
  compile rather than silently misbehaving, which is the good case --- but only because nothing
  builds those files by default did it stay invisible.
- **A test that "compiles and passes" after your change may have been broken *before* it.** Two of
  the files in that sweep were already un-compilable for unrelated reasons
  (`RemapIniRemover_test.cpp` had not built since `removeBackup` joined `IniRemoveContext`). Compile
  every hit *first*, note which were already broken, and say so --- otherwise you cannot tell your
  breakage from the inherited kind.

The cheap way to do all of this at once is one `.bat` that compiles **and runs** every affected test
and prints a per-test `COMPILE_OK`/`COMPILE_FAILED` + `RUN_OK`/`RUN_FAILED` line. 19 tests took
about four minutes end to end. Note `if exist "<out>.exe"` rather than `%errorlevel%` to decide
whether the compile worked --- inside a `for` loop, `%errorlevel%` is expanded once before the loop
body runs and reports the *previous* command's code, so a failing `cl` prints `COMPILE EXIT: 0`.

**A standalone test that touches `TextureFile` needs one extra include dir** beyond the link line
documented in [Building](../Building/CLAUDE.md): `TextureFile.h` includes `compressonator.h`, so add
`/I "<api>/extern/Compressonator/cmp_compressonatorlib"` or you get
`fatal error C1083: Cannot open include file: 'compressonator.h'`.

## Known-flaky: `IniParseBuilder_test`'s post-clear identity check

`[FAIL] the post-clear parser is a genuinely new instance` failed once and passed on every rerun of
the same binary (2026-09-07). It compares pointer identity after a registry clear, so a fresh
allocation landing on the freed address makes it fail with nothing wrong. Re-run before treating it
as a regression.

## C++-only work is invisible to the Python suite --- write a standalone C++ test

An `AGRemapCore` class with no pybind11 binding cannot be reached from `Testing/Unit Tester`: there
is nothing to import. A whole feature can land in `core/` --- new classes, new members on `ModType`,
new data tables --- and the Python suite will neither exercise nor notice it. Running the suite after
such work is still worth doing, but be clear what it proves: **no regression**, *not* **new code
covered**. Don't report a green suite as evidence your core change works.

Write a standalone `core/tests/Xxx_test.cpp` for that coverage instead --- see
[Building](../Building/CLAUDE.md)'s standalone-test sections for how to compile one (including the
static-lib fallback for anything touching `IniFile::parse`/`fix`).

**Those tests are not wired into anything.** `core/tests/*.cpp` are hand-compiled files: not listed
in `core/CMakeLists.txt`, no CTest target, not run by `main.py` and not run by CI. They execute only
when a human or an agent compiles them. If you add one, say so explicitly when reporting --- a reader
will otherwise reasonably assume it runs somewhere automatically.

**Because nothing builds them, they rot silently --- and the damage is done by changes to
`core/`, not by changes to the tests.** Make a core class a template, add a parameter to a `virtual`,
rename a method, and every `core/tests/*.cpp` that mentions it stops compiling. Nothing tells you:
not the CMake build, not CI, not the Python suite. Confirmed the expensive way ---
`IniRemoveBuilder_test.cpp` sat uncompilable for several sessions after `BaseIniRemover` became a
class template, still saying `public BaseIniRemover` / `std::shared_ptr<BaseIniRemover>`, and was
only noticed when an unrelated change happened to rebuild it.

**So make this part of finishing any `core/` interface change, not an optional extra:**

```bash
grep -rl "<the name you changed>" "Anime Game Remap (for all users)/api/src/cpp/core/tests/"
```

and rebuild every file that comes back (see [Building](../Building/CLAUDE.md)'s static-lib recipe).
That grep is the only "build" those files ever get. It is cheap, and it is the difference between
leaving the next agent a working test suite and leaving them a landmine --- one that will look like
*their* change broke it.

**A failed compile leaves the previous `test.exe` on disk, and re-running it prints `ALL PASSED` from
the stale binary.** This bites hardest right after renaming something several test files reference:
the files you forgot to update fail to compile, while their old executables keep passing. Delete the
`.exe`s before a sweep, or check the compiler's exit status --- never trust the run output alone.

Three more things learned writing `StringTools_grapheme_test.cpp` (2026-09-03) that save a
compile-diagnose cycle each:
- **`BaseAhoCorasickDFA::build()` with no argument rebuilds the trie from *nothing*** -- it does not
  "finalise" keywords you `add()`ed, it discards them, and every `find*` then returns nothing. Either
  `add()` and never call `build()` (what the existing lifetime test does), or pass the whole map to
  `build(data)`. The symptom is that even an unchanged baseline call fails, which reads like a
  regression in code you never touched.
- **Write non-ASCII test literals as UTF-8 byte escapes** (`"\xC3\xA9"`, not `"é"`). MSVC guesses the
  source charset without `/utf-8`, and a test that needs that flag has a recipe nobody else's does;
  escapes compile identically everywhere. Keep a comment naming the character next to each.
- **The utf8proc-only recipe is cheap** -- `utf8proc.c` plus the `StringTools`/grapheme cone compiles
  and runs in roughly a minute, so there's no reason to skip re-running every hand-built test whose
  source set you changed. `IniNamingTools_test.cpp` now needs that cone too (its header says so); a
  test whose header recipe stops linking is your signal that a dependency moved.

## When the output has to match an *external* tool, check where your reference came from

Some of this project's output is not ours to define -- it has to match byte-for-byte what some
other program produces (a 3dmigoto frame analysis dump, most notably; see
[Buf Files](../BufFiles/CLAUDE.md)). For that kind of work, **the test that matters is a comparison
against output the external tool actually produced**, and picking the reference is the part that
goes wrong.

Two failure modes, both hit for real:

- **Validating against this repo's own reimplementation of the format.** `Tools/` holds notebooks
  that read and write 3dmigoto dumps, and there are folders of dumps those notebooks generated
  sitting next to folders of genuine ones. They look equally authoritative. Diffing against the
  notebook's output proves only that you reproduced the notebook -- which in this case meant a
  formatter that was carefully built, thoroughly tested against 6,000 values, and wrong, because
  the notebook was a reverse-engineering that used Python's `str()` where the real tool uses C's
  `"%.9g"`. **Confirm the provenance of sample data before you diff against it**, and if the user
  hands you a folder, it is worth one sentence checking which kind it is.
- **Testing the round trip only against yourself.** `write -> read -> write` agreeing proves your
  two halves are consistent, not that either is correct. Anchor at least one end to real external
  output.

The useful shape, when you have genuine samples but no matching binary: **`real text -> your
reader -> binary -> your writer -> compare against the original text`**. That exercises both
directions against a real artifact in one test, and it caught the trailing-blank-line and
CRLF details that no unit test of ours would have thought to assert. When a residual difference
survives, quantify it rather than eyeballing it (`692,492 of 692,496 values identical, the 4
differing only by a float tie-break and all parsing back to the same float32`) -- that is the
difference between a known, accepted deviation and an unexamined bug.

## Comparing against the last *published* library (old-vs-new benchmarks and equivalence checks)

The pre-migration, pure-Python library is still on PyPI, which makes it a genuine oracle: you can
run the old and new implementations over the same real inputs and diff both the timings and the
output. The maintainer has asked for exactly this, so know the mechanics.

```bash
py -3 -m pip install AnimeGameRemap        # installs AnimeGameRemap + FixRaidenBoss2 (4.6.4)
# ... benchmark ...
py -3 -m pip uninstall AnimeGameRemap
py -3 -m pip uninstall FixRaidenBoss2      # both -- see the shadowing warning below
```

- **PyPI is not reachable from this machine without `--trusted-host`.** A plain install dies with
  `CERTIFICATE_VERIFY_FAILED: unable to get local issuer certificate` -- the same broken local cert
  store that makes every Sphinx build warn about unreachable intersphinx inventories, not anything
  about the package. Add `--trusted-host pypi.org --trusted-host files.pythonhosted.org`. Note this
  disables verification for that install, so prefer it resolving from pip's local cache (the log
  says `Using cached`) over a fresh download, and say what you did.
- **Uninstall `FixRaidenBoss2` as well as `AnimeGameRemap` when you are done.** The pip package
  installs a *top-level* `FixRaidenBoss2`, the same name the dev tree exposes. Anything that does a
  bare `import FixRaidenBoss2` without first `sys.path.insert`-ing `api/src/py` will silently pick
  up the published one -- a very confusing way to "reproduce" a bug that no longer exists. Run the
  old and new benchmarks as separate processes and control `sys.path` explicitly in each.
- **Diff the output, not just the clock.** A speedup that quietly changes results is not a win. For
  a ported subsystem the bar is: byte-identical where the format did not change, and where it did,
  a value-by-value comparison showing zero *semantic* differences (see the section above). Real
  numbers reported this way -- "0 value-changing differences across 17.4 M values, 16.1 M
  spelling-only" -- are worth far more than "looks the same".
- Real mods to run over, and the rule about not writing into them, are in
  [Overview](../Overview/CLAUDE.md)'s operating norms.

## Integration Tester (`Testing/Integration Tester`)

**It works again, and its goldens are current (2026-09-17).** End-to-end tests of the API's real output:
each test copies `Tests/<Suite>/inputs/` into `expected_<test>/` (`produceOutputs`) or `output_<test>/`
(`runSuite`), `exec`s one script inside the copy, and compares the whole tree --- `.ini` text by
line, `.buf` by bytes, `.dds` by decoded pixels. Two suites: `APIDocsTests` (17 tests, **each one
backs an example in `Docs/src/apiExamples.rst`**) and `MixedModsTests` (7).

```bash
python3 -m pip install -r requirements.txt   # directory-tree is the one nothing else installs
python3 main.py runSuite                      # or produceOutputs / printOutputs / clearOutputs
python3 main.py runSuite ApiDocTests.test_fullFix_modFixed
```

**Produce and run it on LINUX**, as its README says and as the user asked --- CI is Linux, and a
golden written on Windows differs in path separators inside logs. From this Windows host that means
WSL with the Linux `.so` rebuilt first (`Tools/Misc/Linux/linuxBuild.sh`, run through an LF copy:
the committed script is CRLF and bash chokes on `set -u\r`), then
**`Tools/Misc/Linux/integrationTest.sh <command> [tests...]`**, which activates the venv, puts
`~/itlib` on `PYTHONPATH`, prints the `.so` it is about to test and the result lines. A full
`produceOutputs` or `runSuite` takes about **10-13 minutes** with the checkout on `/mnt/e`; a single
test about 45 seconds. Install `directory-tree` with `pip install --target ~/itlib` rather than into
the shared dev venv.

**THE FIRST CI RUN FAILED 8 OF 24, AND NOT ONE OF THEM WAS A REGRESSION (2026-09-18).** (The workflows
themselves -- what runs where, caches, branch protection -- are in [CI](../CI/CLAUDE.md).) Two
unrelated causes, and both will come back unless you know them:

- **NTFS hands a folder's entries back SORTED; ext4 hands them back in hash order --- and the output
  depended on it.** `FileService::getFilesAndDirs` sorted nothing (nor did the `os.walk` it ports), so
  the order `RemapService` visits mod folders and a folder's `.ini` files in was whatever the
  filesystem said. That order reaches the output: which `.ini` is fixed first decides the names its
  fix generates, and which mod is visited last decides what `summaryLog.txt` opens with. Every Windows
  run, and every golden produced from the checkout on `/mnt/e`, saw NTFS order; the GitHub Actions
  runner is ext4. The walk now sorts into NTFS order (ASCII case-folded, compared component by
  component so a recursive walk stays pre-order), which changed nothing on Windows and made Linux
  match it. `core/tests/FileService_walkOrder_test.cpp` pins the rule --- and **it cannot fail on
  Windows**, because NTFS sorts for it: run it on Linux, where it failed 3 of 3 against the unsorted
  build.
- **A merge from `nhok0169` (now `master`) wrote goldens no code ever produced.** `ccfbf338` merged the pure-Python
  branch into `development`, and git's line-by-line merge applied that branch's 2025 golden edits on
  top of the freshly regenerated C++ ones: `oldVers/AmberCN.ini` and `select/Jean/merged.ini` came out
  matching **neither** parent. The current output matched the pre-merge goldens byte for byte, so they
  were restored. **After any merge that touches `Tests/*/expected_*`, compare each changed golden's
  blob against both parents** (`git rev-parse <merge>^1:<path>` / `^2:` / `<merge>:`) --- a golden
  equal to neither is not an expectation of anything, and a test built on it fails for a reason
  that has nothing to do with the code.

**To see what CI sees, run the suite from an ext4 copy, not from `/mnt/e`.** A CI-shaped copy is the
tracked files with LF endings, which is exactly what this gives:

```bash
git -c core.autocrlf=false archive HEAD -- Testing Tools/Utilities \
    "Anime Game Remap (for all users)/api/src/py" "Anime Game Remap (for all users)/script build" \
  | tar -x -C ~/itest-ext4          # then copy the five Linux .so files into its FixRaidenBoss2/
AG_REMAP_REPO=~/itest-ext4 bash integrationTest.sh runSuite
```

It is also four times faster (about 2.5 minutes against 8-13). Note that ext4's hash order differs
between machines too: the same unsorted build failed 9 tests here and 8 on the runner. A failure that
moves between machines is an ordering question before it is anything else.

**The Unit Tester's 11 Linux failures went the same day, and they were NOT all test-side, as this
guide had said.** Three causes:

- **8 were the tests' own**: they handed the core a mod folder of `C:/mods/EiRemap`, absolute only on
  Windows, and POSIX resolved it against the launch directory. They now use `ModsRoot` /
  `nativePath` from `baseUnitTest.py` --- `C:/mods` on Windows, byte for byte what they always used,
  and `/mods` elsewhere. **A new test that needs an absolute path uses those**, not a drive letter.
- **2 were the pure-Python `IniNamingTools`**: `getFixedFile` / `getFixedElementFile` built a path to
  write INTO a `.ini` with `os.path.join`, so on Linux they returned `./x` where the core, and every
  `.ini` the product writes, says `.\x`. They now parse with `PureWindowsPath` and join with `ntpath`
  --- which on Windows *are* `pathlib.Path` and `os.path`, so Windows output is provably unchanged
  (checked: identical over four inputs). `getFixedTexFile` was left OS-native on purpose, because the
  core's is too.
- **1 was `IfTemplateNode.children`'s order** --- the "open question" further down, resolved there.

**One thing this turned up and deliberately did NOT fix**: the core's `IniNamingTools::getFixedFile`
and `getFixedElementFile` build their input with `fs::path(file)` rather than
`FileService::strToPath`, so on Linux a backslash in the `.ini` value is not a separator ---
`.\Sub\RaidenBlend.buf` comes back `.\.\Sub\RaidenBlendrikaRemapFix.buf` on Linux and
`.\Sub\...` on Windows. The same file on disk, different `.ini` text. It is also the call
Architecture's UTF-8 path rule bans in `core/`. Fixing it may move the Integration Tester's
Linux-produced goldens, so it wants its own change and its own golden check.

What the 2026-09-17 repair changed, each of which is a trap for whoever touches it next:

- **The scripts import `FixRaidenBoss2` from `api/src/py`** (`constants/Paths.py`'s `APIPath`), not
  `src.FixRaidenBoss2`. Two import names for one extension module load it twice.
- **Every fixture `.ini` carries a real `hash`.** The pre-migration script matched sections by NAME;
  the C++ parsers match by HASH. The fixtures were hand-written without hashes, so after the
  migration the Raiden ones produced only the credit header and AmberCN ignored the mod's own blend
  and substituted a downloaded one --- a golden regenerated from that pins "nothing happens" and every
  docs example shows an empty fix. The blend hashes added are the 4.0 `blend_vb` rows of
  `HashData.cpp`, matching the 4.3 `ib` hashes the fixtures already had (no blend changed between
  them). **A new fixture needs a real hash too**, or its test passes vacuously.
- **Downloads are ON in `APIDocsTests` and OFF in `MixedModsTests`** (`downloadMode = "disabled"` on
  every constructor there) --- the maintainer's call: the docs examples should show the downloaded
  files. The API docs goldens therefore contain `*RemapDL*` files fetched from GitHub during the run,
  and a network failure in CI shows up as a missing file rather than an error.
- **`getFixStr` is gone** (dropped in the C++ `IniFile` port), so its test and docs example were
  removed. `IniFile.fix()` returns `{path or group index: text}` --- one entry per `.ini` file written,
  because a merge writes more than one.
- **`RemapServiceCLI` holds the model on `.service`**: `cli.service.undoOnly = True`,
  `cli.service.clear()`, `cli.service.stats`; `cli.log` stays on the CLI.
- **The custom Kirara test (`overrideFix/iniPath_ImplOverride.py`) is a `GIMICharFixerConfig`** handed
  to `makeGIMICharFixer` and registered with `CppStrategyOverrides.setFixer` --- the same route as
  `Tools/Misc/Prototypes/overrideScript.py`. Writing it found two binding bugs, both fixed: the
  register-removal/remap fields could not be assigned from Python at all, and a Python texture filter
  edited a COPY of the texture (see Architecture's pybind section).

**Read the goldens by what each test PRODUCED, not by `git diff`.** Every golden carries a full copy of
the inputs, so a one-line fixture change shows up in every test's tree. Diffing each `expected_*`
against `inputs/` (added / removed / changed files) is what surfaced the two core bugs the repair
found: an undo-only run that never reached a `.ini` file found only through a resource's folder
(`fullFix_modFixUndoed` kept a fix after undoing it), and any text-only `IniFile` whose fix built a
resource throwing `filesystem error: cannot make absolute path`. **Keep the previous output folder
before regenerating** and check the new golden would have failed against it.

**The loop, when a change legitimately moves the output** --- each step has a tool, and skipping the
review step is how a regression gets baked into a golden:

1. Rebuild the Linux `.so` (`linuxBuild.sh`) --- and if a long run is importing the current one, build
   with `ninja core` only and copy afterwards (Overview habit 49).
2. `integrationTest.sh produceOutputs <the tests you touched>` --- a single test is ~45 seconds, so
   iterate on those, not on the whole suite.
3. **`Tools/Misc/Diagnostics/goldenChanges.py [filter]`** --- what each golden added / removed /
   changed relative to its inputs. Read every entry. An EMPTY entry for a test that should fix
   something is a vacuous test; an `.ini` still carrying `Remap` after an undo is a bug, not a golden.
4. `integrationTest.sh produceOutputs`, then `integrationTest.sh runSuite` --- the full suite twice.
   The second run is the determinism check: the API docs tests download from GitHub, and a golden
   that only passes on the run that produced it is not a golden.
5. **`Tools/Misc/Docs/genApiExamples.py --write`**, then build the docs --- see
   [Documentation](../Documentation/CLAUDE.md)'s "`apiExamples.rst` is GENERATED".

Two tester-side facts that bit on the way: the tester compares a file as **binary** only if
`TestFileTools.BinaryFiles` names its extension (`.buf|.ib` now), and a file it does not name is read
as UTF-8 text --- so a NEW binary output type (`.ib` arrived with downloads) errors with
`UnicodeDecodeError` in `runSuite` while `produceOutputs` never notices. And **a new API docs test
needs a new `SECTIONS` entry in `genApiExamples.py`**, or its example silently stays hand-written.

Known cosmetic difference, not a bug: an undo writes a trailing newline the input did not have, so
the `...filesSameAsBefore` tests are the same as before up to that newline.

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


## The suites cannot see a remap at all — run the real CLI and diff against the old script

[CreatingRemaps](../CreatingRemaps/CLAUDE.md) has the full recipe; this is the short version of why
you cannot skip it.

Building Raiden's 6.1 remap produced **five** independent bugs that a green suite reported nothing
about: a fix that generated no remapped sections, a `Blend.buf` written as an unremapped copy, a
resource counted nowhere, an `--undo` that discarded its own result, and every `.ini` file reflowed
from CRLF to LF. The Python suite cannot see `AGRemapCore` work with no binding, and the standalone
`core/tests/*.cpp` only assert on a strategy's *shape* — that a fixer has two group edits and the
right `hiddenModObjs`, not what it emits.

The A/B loop that does catch them: copy a real mod, undo it with `FixRaidenBoss6.py -u` to get a
genuinely unfixed baseline, run the old script and the new API against separate copies, and diff.
The decisive check is that the emitted `Blend.buf` **differs from its source** — an unremapped copy
is byte-identical, produces a correct-looking `.ini`, and reports success.

Then run the fix **3-4 times in a row**. Repeat-run stability is where re-fixing your own output,
unbounded trailing newlines, and line-ending reflow show up; none of them appear on a single run.

## Tests that only run where the core is built (2026-09-12)

When the Windows `.pyd` is behind the C++, a Python test of a new binding can only run on the
Linux side: `wsl -d Ubuntu-22.04 -- bash -lc 'source ~/agremap-venv/bin/activate && cd "<repo>/Testing/Unit Tester" && python main.py <ClassName>'`.
The Yelan session's tests are the ones to run first after a Windows rebuild, since none has run
there yet: `test_VGComponentSplit.py` (3 classes, the negative-index and fill-cut split against
hand-built buffers), the five `BottomCover` cases in `test_RegFillMissing.py`,
`test_TextureFile.test_save_noGammaKey_keepsTheGammaOpenDetected` (the Python save clobbering the
sRGB gamma) and `test_save_mipmaps_writesTheFullChain`. All pass on Linux; the suite's baseline
failures (the `BaseIniFileTest` classes and one Windows-path test) are unchanged.

**The standalone `core/tests/*.cpp` build on Linux too, and three of them changed on
2026-09-13**: `Tools/Misc/Linux/buildTests.sh <Test> ...` compiles each named test against the
native build tree's `libAGRemapCore.a` and the static Compressonator / utf8proc libraries, links
the package's `libz3.so`, and runs it. `BuilderData_test` (parse 56 rows / fix 124 / remove 45,
after Yelan), `ModTypeRemaps_test` (45 built types; the three YelanTranquil component ids are
targets only and have no row) and `IniClassifierPopulation_test` (45 keyword rows) all pass. Two
things that cost time: a test whose oracle row lists NO keyword indexes `keywords[0]` and
segfaults with no output -- that is the test, not the library, but the library really does not
hold a keyword-less registered type either; and `test_CppIfContentPart` / `test_IfContentPart`
segfaulted on the Linux build inside `parseIniReplaceVals`, a binding nobody had touched, while
passing on the older Windows build. That was a real bug the compiler change exposed (a range-for
over a reference into a temporary -- see Architecture), fixed the same day; **a "passes on
Windows, crashes on Linux" test in unmodified code is a defect to find, not a platform to
excuse.** The fault handler (`python -X faulthandler main.py`) names the test when the crash is
inside one; a crash it reports with NO Python frames is a teardown crash instead. With that
fix the Linux suite runs to the end again: **2137 tests, 12 failures, 7 errors** on
2026-09-13, every one of them the Linux baseline above (the Windows-path-literal tests --
`test_IniResource`, `test_IniFixResourceModel`, `test_RemapBlendResource`,
`test_RemapTexAddResource`, plus `test_BaseResEdit` / `test_ResEdits` of the same kind --
the `IfTemplateTree` question, and the `BaseIniFileTest` setUpClass classes); all 12 failing
classes pass on the Windows build.

**On 2026-09-16 it is 2173 tests, 13 failures, 7 errors.** The extra failure is
`test_RemapServiceCLI.test_versionAndDownloadModeConvert`, which still expects `version = "4.0"` to
set `service.fromVersion` -- stale since `--version` became the fix-TO version and `--fromVersion`
was split off (see the top-level CLAUDE.md's "TWO GAME VERSIONS"). It is not a regression of
anything newer; fix the test, not the service, if you are asked to.

## Running every standalone C++ test on Linux: the runner, and three ways it lies (2026-09-14)

Measured on a full pass over `core/tests/*_test.cpp`: **47 files, 44 pass, 3 fail**, and the three
failures are Linux-only and have nothing to do with whatever you just changed. Getting to that
number needed four fixes to the obvious runner, and each of the first three makes the run *look*
like something it is not.

**1. `Tools/Misc/Linux/buildTests.sh` locates Z3 with `find` over `$API/extern` and `cextlin`.**
On a checkout mounted at `/mnt/e` those two `find`s take many minutes, and while they run the log
is EMPTY and no compiler is running -- which reads exactly like a script that failed to start.
Hardcode the two paths instead; on this checkout they are

```
z3 include:  <api>/extern/z3/src/api          (and <api>/extern/z3/src/api/c++ for z3++.h)
z3 lib:      <repo>/cextlin/z3/lib/libz3.so
```

**2. Judging pass/fail by grepping the output marks every passing suite as failed.** A passing test
prints `ALL PASSED (0 failure(s))`, and a case-insensitive grep for `FAILURE` matches the word
"failures" in it. Every one of these tests ends `return (failures == 0) ? 0 : 1`, so **use the exit
code** and nothing else.

**3. Four of the 47 need include paths the core build does not need**, and without them they report
"did not build" rather than failing -- the same silent-gap shape as the typed-list lesson above:

| suite | needs |
| --- | --- |
| `IfPredPart_test`, `Z3Predicate_test`, `Z3IfPredGenerator_test` | `-I <api>/extern/z3/src/api/c++` (for `z3++.h`, which is NOT beside `z3.h`) |
| `CompressTextures_test` | `-I <api>/extern/Compressonator/cmp_compressonatorlib -I <api>/extern/Compressonator/cmp_framework` |

With those, all 47 build.

**4. The three that fail are the Linux baseline. Do not "fix" them, and do not read them as your
regression:**

- `IniNamingTools_test` (3 checks) -- `got .\fooRaidenRemapFix.ini, expected ./foo...`. The product
  deliberately writes Windows separators into a `.ini` on every OS (Architecture's "A path INSIDE a
  `.ini` is a Windows path"); these expectations were never updated to match.
- `IniResources_test` (1 check, `absPathOfRelPath: an already-absolute dstPath ignores relFolder`) --
  the literal is `"C:/mods/EiRemap"`, which is absolute on Windows and relative on Linux. Same family
  as the eight hardcoded-Windows-path failures [Setup](../Setup/CLAUDE.md) records for the *Python*
  suite; that list does not cover the C++ suites, so this is the note for them.
- `FileDownload_curl_test` -- aborts on an uncaught `std::runtime_error`, *"URL rejected: Malformed
  input to a URL function"*. Exit 134 (SIGABRT), not a check failure.

**And the cheap way to decide whether a failure is yours** without paying for a second build:
`grep` the failing test file for the names you touched. `IniNamingTools_test.cpp` mentions no
`ModTypeId`, `GIBuilder`, `HashData`, `VGRemap` or `VertexCount` at all, which settles it in one
command.

<br>

## When you add a row to a builder table, `BuilderData_test.cpp` breaks silently

`core/tests/BuilderData_test.cpp` asserts exact row and version counts for all three builder tables
(measured 2026-09-13: `56 rows / 10 versions` for parse, `124` for fix — 78 historical plus 46 at
6.1 — and `45` for remove, one per GI mod type). Adding a character's row breaks it, and since
nothing builds `core/tests/*.cpp`, nothing tells you. This is the same trap already described above
for interface changes — it applies to *data* changes too.

**Don't re-add a per-character enumeration to a count's message.** Two of them rotted here: the fix
table's listed 36 characters' worth of 6.1 rows while the literal said 124 (it was ten rows behind),
and this file's own quote above was two characters stale. State the total and where the inventory
lives; the table file is its own list.

**When the Windows build is busy, run these on Linux** —
`Tools/Misc/Linux/buildTests.sh <TestName> ...`, after `ninja AGRemapCore` in `~/cbuildlin-native`;
see [Building](../Building/CLAUDE.md)'s Linux-side section for the two traps (stale objects that
report green, and the `cextlin` z3 path).

**Two other suites hardcode data counts the same way**, so a new character breaks them too:
`VertexCounts_test.cpp` (`44` vertex-count rows) and `VGRemaps_test.cpp` (`58` remap rows). Yelan's
remap grew past both and nothing said so for a day.

**The GI mod type count is two numbers, not one**, so check which side a sentence is about before
bumping it: `GIBuilder::all()` builds **45** mod types, while the pure-Python `ModTypes.getAll()` is
still **43** (no Yelan, no YelanTranquil) — measured 2026-09-13. Three of the four "all 43"s in
`core/src/constants/GIBuilder.cpp` describe the *pure-Python* GIBuilder's own `ModType(...)` calls
and are still correct; only the fourth, about the C++ mod types, was stale. Same for
`ModTypeRemaps_test.cpp`'s "all 42 others", which is about Raiden's pure-Python factory.

**When you bump one of those, check the comment's *provenance* claim, not just the number.**
`VertexCounts_test`/`VGRemaps_test` used to say their counts came "straight from the live
pure-Python `VertexCountData` dict" / "`vgRemapDataBuilder.build()` output" — true when the tables
were generated, false since the Yelan → YelanTranquil remap was compiled into C++ **only**. Measured
2026-09-13: `VertexCountData` is 44 rows in C++ against the dict's 43 (the extra is `Yelan @4.0`);
`VGRemapData` is 58 rows / 5542 pairs against the builder's 52 / 5229 (the extra six are
`Yelan <-> YelanTranquil`, per component — and the **first** rows to use `fromComp`/`toComp` for
anything but `""`). So these tables can no longer be regenerated from their Python counterparts, and
a number "corrected" by reading the Python side back silently deletes a character. Say *which* table
a count belongs to in the comment; the data headers
(`core/include/AGRemapCore/data/{VertexCountData,VGRemapData}.h`) now spell the divergence out.
