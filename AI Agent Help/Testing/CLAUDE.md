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
- **A `test_Xxx.py` for a not-yet-implemented `model/strategies/iniFixers/regEdits/`- or
  `graphGroupEdits/`-style stub often already exists on disk as a literal one-line
  `# TODO: Add tests for Xxx class` placeholder**, not a genuinely missing file — confirmed for
  `test_RegAdd.py`, `test_RegNewVals.py`, `test_RegRemap.py`, and `test_RegRemove.py`. `Write`
  refuses to overwrite a file you haven't `Read` first, so `Read` it (even though you already know
  it's just the TODO line) before writing the real test module over it, and don't assume the
  absence of a `find`/`Glob` hit for some other naming guess means no test file exists yet — check
  the exact `test_<ClassName>.py` path directly.

### Known-broken/WIP test modules — don't chase these as regressions
**Not every test module in `Tests/` is finished/passing right now** — some are known
work-in-progress from the maintainer and fail for reasons unrelated to your change. The maintainer
has been actively fixing these incrementally (a large batch — `test_FileService`,
`test_BaseSLR1Parser`, `test_IfPredTokenizer`, `test_IfPredParser`, `test_SympyParser`,
`test_IfPredLogicGenerator`, `test_SympyIfPredGenerator`, `test_IntTools`, `test_Version`,
`test_IfTemplateNormTree`, `test_IfTemplateTree`, and the old pre-C++-port `test_IfContentPart` —
all went from broken to fully passing in one pass), so **don't trust this list blindly; re-run and
re-verify rather than assuming stale entries are still accurate**, in either direction. Confirmed
still erroring/failing on a clean run as of this writing (936 tests, 2 failures + 53 errors):

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
- `test_IfTemplate` — 2 broken out of 12: `test_addParts_newPartsAddedToEnd` (`ERROR`,
  `AttributeError: ... has no attribute 'src'` — asserts the old pure-Python `IfContentPart`'s
  `.src` attribute, which the C++ port doesn't have) and `test_hasParts_filteredParts` (`FAIL`, a
  plain value mismatch, not obviously the same root cause — don't assume fixing one fixes both).
  The other 10 tests in the module (including one that directly `assertIsInstance`s against the
  shared `IfTemplatePart` base) pass fine.

This list will keep drifting as the maintainer continues fixing modules — treat it as "expect some
unrelated red, but verify which red" rather than a precise, permanent inventory. If you're about to
spend time on a module not listed here, or need to confirm one of these is still actually broken,
just re-run it (`py -3 main.py SomeTestClass -v`) rather than trusting this snapshot.

**`test_RegSurroundedAdd` and `test_IniSectionGraph` are *not* on this list** — both are clean,
comprehensive, and fully passing as of a full fixpoint/reachability redesign of `RegSurroundedAdd`
plus a follow-up extraction of its reusable graph machinery into `IniSectionGraph`/`GraphTools`/
`CallGraph` (see [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)). If either starts failing, treat
it as a real regression from your change, not pre-existing noise — don't assume it belongs on this
list just because an earlier version of this file once listed `test_RegSurroundedAdd` here.

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
