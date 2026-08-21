# Architecture

Conventions and gotchas for working inside `AGRemapCore` (the C++ core), its pybind11 bindings,
and the separate Cython extensions (`api/src/cy`). See [Building](../Building/CLAUDE.md) to
compile, [Testing](../Testing/CLAUDE.md) to verify, and [Documentation](../Documentation/CLAUDE.md)
for how these conventions show up in rendered docs.

## C++ core conventions (`api/src/cpp/core`)
- `AGRemapCore` has zero Python/pybind11 dependency — it's meant to be usable as a standalone
  C++ library too (`-e core` build mode, see [Building](../Building/CLAUDE.md)). Keep it that
  way; Python-specific concerns belong in `api/src/cpp/py`.
- CRTP (`BaseOrderedMultiMap<Derived, ...>`) is used deliberately to avoid vtable overhead on
  hot-path container types. Don't casually add virtual methods to a CRTP base — if you need
  runtime polymorphism over such a type, wrap it in an adapter against a real interface instead
  (see `OrderedMultiMapAdapter` wrapping CRTP-shaped types to satisfy `IOrderedMultiMap`) rather
  than adding a vtable to the CRTP hierarchy itself.
- `KeyHash`/`KeyEqual` template parameters (defaulting to `std::hash<K>`/`std::equal_to<K>`)
  are threaded through the ordered-multimap hierarchy specifically so `py::object` keys can be
  supported via custom hashers (`PyObjectHash`/`PyObjectEqual`) without specializing
  `std::hash<py::object>` globally — follow this pattern (explicit template param, not a global
  specialization) for any new generic container that needs Python-object-keyed support.
  `IfContentPart<K, V, KeyHash, KeyEqual>` carries these too (added specifically so it can build
  a real `std::unordered_set`/`std::unordered_map` locally, e.g. `getKeys()`, even though the
  `IOrderedMultiMap` interface it talks to can't — see the delegation-chain section below).

## `std::make_tuple(*ptr1, *ptr2)` silently returns dangling references when the declared return type is a tuple of references

A method declared `std::tuple<const std::string&, const TrieVal&> getXxx(...)` but implemented as
`return std::make_tuple(*keywordPtr, *valPtr);` compiles and links cleanly, and even *works* under
naive testing — but is genuinely broken. `std::make_tuple` decays its arguments and builds a
**by-value** `std::tuple<std::string, TrieVal>` temporary; that temporary then gets implicitly
converted to the declared reference-tuple return type via `std::tuple`'s converting constructor,
binding the reference members to elements of the temporary. The temporary is destroyed at the end
of the `return` statement, so the tuple actually handed back to the caller holds dangling
references — reading either element is use-after-free UB. Found in `BaseAhoCorasickDFA::getKVP`/
`::getMaximal` (both declare `std::tuple<const std::string&, const TrieVal&>` returns); the
sibling `*Ptr` overloads (`getKVPPtr`/`getMaximalPtr`, returning real pointers) don't have this bug
— only the reference-returning wrappers built with `make_tuple` on top of them did.

**Fix**: `std::tie(*keywordPtr, *valPtr)` instead of `std::make_tuple(...)` — `std::tie` builds the
tuple of references directly, with no by-value intermediate to dangle. Grep any method whose
declared return type is `std::tuple<...&, ...>` (or contains any reference element) for
`make_tuple` in its body; `make_tuple` is only safe when the declared return type is by-value.

**Why this is easy to miss in testing, and how to actually test for it**: an immediate structured
binding read (`auto [k, v] = dfa.getMaximal(txt); use(k, v);`) often "works" by pure luck — the
temporary's stack slot frequently hasn't been reused for anything else yet by the time `k`/`v` are
read, especially in an unoptimized/debug build. A regression test for this class of bug needs to
keep the returned value alive across an intervening call that actually disturbs the stack
(recursion + writes to a local buffer) before reading it — see [Building](../Building/CLAUDE.md)'s
"Fast path: compiling a standalone `core/` regression test" section for a worked example that
reliably fails pre-fix and passes post-fix, confirmed both ways.

**A Python-side pybind11 test cannot currently exercise this specific bug at all — verify a
method's real call graph before assuming a passing test covers it.** It's tempting to assume the
existing `test_CppAhoCorasickDFA.py`'s `getMaximal`/`get` tests already guard against this (or to
add a new Python-side test alongside the C++ one for "extra" coverage) — don't; it would silently
test the wrong thing and pass regardless of whether the bug is present. Traced end-to-end: neither
`PyAhoCorasickDFA::pyGet` nor `::pyGetMaximal` (bound as `.get`/`.getMaximal`) ever calls the core's
reference-returning `getKVP`/`getMaximal` — `pyGet`'s effective binding (`pyOptGet`, registered
first with an identical signature, so it always wins pybind11's overload resolution over the
later-registered `pyGet` that *would* have called `getKVPPtr`) does its own direct `findPtr` lookup,
and `pyGetMaximal`'s `count <= 1` path calls `getMaximalPtr` (the safe pointer-returning sibling),
not `getMaximal`. `grep -n "getKVP\|getMaximal(" api/src/cpp/py` confirms the only real call is the
unrelated vector-returning `getMaximal(txt, count, pred)` overload. As of this writing, **nothing
anywhere in the codebase calls the buggy `getKVP`/`getMaximal` overloads** — their one intended
caller, `IniClassifier::readSectionName`, is still a literal `// TODO: filled in later` stub (see
[Ini Graph Editing](../IniGraphEditing/CLAUDE.md)-adjacent `model/strategies/iniClassifiers/`) —
so the only way to actually exercise these two methods today is a direct C++ instantiation, e.g.
the standalone regression test referenced above. Once `readSectionName` (or any other caller) is
implemented calling `getMaximal`/`getKVP` and made reachable from Python, add a Python-side test
for it at that point — but don't write one against the current, still-unreachable methods and call
it coverage; confirm reachability (`grep` the real call sites, don't assume from the method's
name/section-header prominence in the test file) before trusting a passing Python test as proof
for any specific C++-level method.

## Some `core/` files already sitting in the repo (and even listed in `CMakeLists.txt`) have never actually been built or exercised — don't trust them as a starting point without verifying

Not everything under `core/include`/`core/src` that looks "already ported" is actually correct.
`BaseTokenizer.h`/`.cpp` existed, compiled, and were already wired into `core/CMakeLists.txt`
*before* this codebase had any pybind11 binding, test, or real consumer for them — and turned out
to have four real, independent bugs once actually exercised: `addStartState()`/`addKeyword()`
returned the wrong type entirely (`bool` instead of the documented `str` state id), matching
neither their own header comment nor the Python original's contract; `addKeyword`'s own loop
counter was declared but never incremented, silently breaking every multi-character keyword's
final accepting state; `addASCIIRangeTransitions` used an exclusive range where the Python
original (and its own doc comment) specified inclusive, dropping the last character of every
range; and the constructor accepted a `setup` parameter but never actually called `setup()`. None
of this was caught by "does it compile" — it compiled and linked cleanly throughout every prior
session. The only way any of it surfaced was writing real tests against the class's actual
behavior and comparing to the live pure-Python original's output (see
[Testing](../Testing/CLAUDE.md) and the empirical-verification pattern in
[Building](../Building/CLAUDE.md)). Treat a pre-existing, already-committed `core/` file with no
corresponding pybind binding/test as a *draft*, not a finished port, even if it's already in the
CMake source list — re-derive its correctness from the Python original and verify empirically
before building on top of it, the same as you would for code you're writing fresh.

## `std::vector::emplace_back`/`push_back` (or any reallocating mutation) invalidates references into that same vector — even ones taken earlier in the same loop iteration

`const Id& itemId = items[i].first;` followed later in the *same loop body* by
`items.emplace_back(...)` (discovering a new item during closure-computation, e.g.
`BaseSLR1Parser::addImpliedProductions`) is a dangling-reference bug: if the `emplace_back`
triggers a reallocation, every existing reference/iterator into `items` — including `itemId`,
taken just above — is invalidated, and the rest of that loop iteration reads freed memory. This
compiles clean, links clean, and **silently "works" for `Id = int`/`std::string`** (the freed
memory usually still looks like a plausible, if occasionally wrong, value) — it only reliably
crashed once instantiated with `Id = py::object`, throwing a CPython-internal `SystemError: bad
argument to internal function` from deep inside the next line that touched the corrupted
reference. Reproduced minimally with any grammar needing 2+ rounds of closure expansion (e.g.
`S -> T`, `T -> U`, `U -> d`) before finding the cause — a single-level grammar never triggers a
reallocation large enough to relocate the backing storage, so it can pass every test you think to
write against a "works fine" build.

**Fix**: copy by value (`const Id itemId = items[i].first;`) instead of binding a reference,
whenever a loop both reads from and (indirectly, via a helper it calls) appends to the same
`std::vector` it's iterating. **General lesson, not specific to this one bug**: any `std::vector`
(or `std::string`, or `tsl::ordered_map`'s `values_` under some operations) that is mutated by
inserting new elements *while a reference/iterator/pointer into it from before the insertion is
still live* is a latent bug, regardless of `Id`/element type — the only reason it surfaces as an
actual crash for `py::object` and not `int`/`std::string` is that CPython's own internal sanity
checks on the corrupted `PyObject*` bit pattern happen to catch it; a plain trivially-copyable
type just reads garbage silently. When reviewing or writing a loop that both reads an element
and can grow the same container mid-iteration, treat "does it crash under `py::object`" as a real
empirical test worth running even if the feature is otherwise generic over `Id`, not just a
nice-to-have extra instantiation.

## pybind11 bindings (`api/src/cpp/py`)

This project pins **pybind11 3.0.4**.

- Passing a `std::unique_ptr<T>` **from Python into C++** (not just returning one to Python)
  requires the target's `py::class_<T, ...>` registration to include `py::smart_holder` as a
  template arg — the default holder only supports the return-to-Python direction.
- A trampoline class combined with `py::smart_holder` must additionally inherit from
  `py::trampoline_self_life_support`, or you get a `static_assert` at compile time.
- To let a C++ interface be subclassed **from pure Python** (not just C++), bind it as a real
  `py::class_<Interface, TrampolineClass, py::smart_holder>(...)` with a `PYBIND11_OVERRIDE*`
  trampoline — see `PyIOrderedMultiMap.h`'s `PyBindIOrderedMultiMap` for the reference
  implementation, including the shared, non-anonymous-namespace parsing helpers other binding
  files reuse (`parseRanges`, `parseKeyRemapList`, etc. — check there before duplicating a
  dict-parsing helper for a new binding).
- `std::is_default_constructible_v<SomePolymorphicClassTemplate<...>>` forces full instantiation
  of that template's virtual method bodies (unlike ordinary lazy members) — if you need an
  `if constexpr` gate based on default-constructibility of a *component* type (e.g. "does `K`
  have a usable `std::hash`?"), check a plain non-polymorphic proxy (`std::hash<K>`) instead of
  the polymorphic type itself, or you'll get instantiation errors for types that were never
  meant to hit that branch.
- To give a Python-bound class **real** inheritance from another bound class (so `isinstance()`
  and attribute inheritance work, not just a doc comment claiming it): register the base first
  via its own `py::class_<Base>(m, "Base")` call, then pass it as a template arg when registering
  the derived class: `py::class_<Derived, Base>(m, "Derived", ...)`. See `PyDFA.cpp`'s
  `BaseDFA`/`DFA` pair for the canonical example.
- **Subclassing a pybind11-bound class from pure Python needs no trampoline at all**, as long as
  every call to the overridden method is made *from* Python (`myInstance.someMethod(...)`) — this
  already works out of the box for any bound class, since Python method resolution happens
  entirely in Python before anything reaches C++. A trampoline (`PYBIND11_OVERRIDE*` +
  `py::smart_holder`) is only needed for the different, narrower case of **C++ code holding the
  object through the base type and calling the method itself** (true C++ → Python virtual
  dispatch) — e.g. `IfContentPart`'s `content_->getAll(...)` reaching into a Python-implemented
  `IOrderedMultiMap`. Don't add virtual methods + a trampoline to a class just to let it be
  subclassed and overridden from Python if nothing on the C++ side ever calls through it.
- **Binding an inherited (CRTP-base-defined) method directly via `&Derived::method` works fine
  when the base type only appears as the implicit `this` parameter** — e.g. a plain
  `toHexString() const` — because pybind's generated wrapper just `static_cast`s the instance
  pointer to the base type, a compile-time C++ operation that needs no RTTI/`type_caster` for the
  base at all (see `BaseOrderedMultiMap`'s `insert`/`size`/etc., bound directly on
  `PyOrderedMultiMap` even though none of them are redeclared there). **This breaks the moment the
  base type shows up in an *argument or return* position instead** — e.g. `operator==(const
  Base&)` — since pybind then needs a registered `type_caster` for that parameter type, and a CRTP
  base meant purely as an internal implementation detail (e.g. `HashInt<Derived, ByteSize>`,
  never itself given its own `py::class_<Base>(...)`) has none. Fix: write an explicit lambda that
  types *both* sides as the concrete, registered `Derived` type and let ordinary C++ overload
  resolution find the inherited operator internally (`self == other` inside the lambda body is
  just C++ — the base type never has to cross the pybind11 boundary at all). See
  `PyHash64.cpp`/`PyHash128.cpp`'s `__eq__`/`__ne__`/`__lt__` bindings (lambdas) contrasted with
  `toHexString`/`hashCode` (bound directly) on the same classes for both halves of this side by
  side.
- **Sharing a method surface across pybind11-bound classes that are *unrelated* at the C++ level**
  (different instantiations of the same class template, e.g. `BaseSLR1Parser<py::object, ...>` vs.
  `BaseSLR1Parser<std::string>` backing `SympyParser`/`IfPredParser`) **needs no pybind11
  inheritance at all** — `py::class_<Derived, Base>` requires a real, single C++ inheritance
  relationship, which two unrelated template instantiations don't have (there's no `isinstance`
  relationship in C++ between them either). Instead, write one plain C++ function template
  (`template <typename T, typename PyClass> void bindXxxCommonMethods(PyClass &cls)`) that takes
  the already-constructed `py::class_<T, ...>` and chains `.def(...)` calls onto it, then call it
  once per concrete binding (see `bindBaseSLR1ParserCommonMethods` in `PyBaseSLR1Parser.h`, reused
  by `PyBaseSLR1Parser.cpp`/`PySympyParser.cpp`/`PyIfPredParser.cpp`). This gets you shared
  method-binding code (including shared docstrings, written once) without a false inheritance
  claim — and per the Documentation doc, don't write a "this class inherits from" line for this
  case either, since there's no real pybind11-level relationship to back it.
- **A type with a `unique_ptr`-typed member and a user-declared (even `= default`) destructor
  loses its implicit move constructor** — plain `py::init<Args...>()` (which move-constructs the
  return value) then fails to compile for that type. Use `py::init(factory)` instead, where
  `factory` returns `std::unique_ptr<T>` by value (pybind11 has a dedicated overload for a factory
  returning a smart pointer, so no move of `T` itself is ever needed) — see `BaseSLR1Parser`'s
  constructor bindings (it owns three `unique_ptr<BaseIdGenerator<Id>>` id-generator members) for
  the pattern: a free function `makeXxx(...) -> std::unique_ptr<Xxx>` that forwards its arguments
  into `std::make_unique<Xxx>(...)`, bound via `.def(py::init(&makeXxx), py::arg(...)...)`.
- **`tsl::ordered_map`'s iterator (`for (auto &[k, v] : someOrderedMap)`) always yields a `const`
  key AND a `const` value, even from a non-`const` `begin()`/`end()`** — unlike
  `std::unordered_map`, where only the key half of the pair is `const` through a mutable
  iterator. Trying to mutate `v` through the structured binding silently does nothing (or fails to
  compile, depending on the operation) rather than mutating the map. To genuinely mutate a value
  in place while iterating (e.g. `constructDFA`'s `neighbours` map), collect the keys first (or use
  a separate pass), then mutate through `someOrderedMap.at(key)` — a real mutable reference — not
  through the loop variable.
- **C++17 does not guarantee left-to-right (or any particular) evaluation order for a single
  function call's arguments** — a constructor call like
  `Base(makeSomething(startToken, endToken), std::move(startToken), std::move(endToken))`, where
  `startToken`/`endToken` are read by one argument (`makeSomething`) and `std::move`d away by
  others *in the same call*, is a real, silent bug: if the compiler evaluates the `std::move`
  arguments before `makeSomething`'s arguments, `makeSomething` reads already-moved-from (empty)
  strings. This compiles clean and can even pass casual testing depending on evaluation order
  luck for a given compiler/optimization level — found in `SympyParser`/`IfPredParser`'s own
  constructors, isolated by noticing an otherwise-identical int-keyed reproduction of the same
  grammar shape worked fine, narrowing the bug to the constructor's own argument list rather than
  the base class. **Fix**: never both read-and-move (or read-twice-with-one-move) the same local
  in one function call's argument list — pass plain copies for every value that's used more than
  once across that call's arguments, and reserve `std::move` for a value used exactly once,
  in that call, guaranteed.

## Raising a Python-specific exception from `AGRemapCore` code, without giving the core a Python dependency

`AGRemapCore` has zero Python/pybind11 dependency (see above) — but a core algorithm can still
need to signal a failure that existing Python code catches by exact class (`except SyntaxErr as
e: ...`, several call sites in `IfPredPart.py`/`BaseSLR1Parser.py`). The pattern used for
`BaseTokenizer::simplifiedMaximalMunch`, which needs to raise the exact same
`FixRaidenBoss2.exceptions.SyntaxErr.SyntaxErr` the pure-Python tokenizer already raised:

1. **In `core/`**: a plain C++ exception (`AGRemapCore::SyntaxErr`, deriving `std::runtime_error`)
   that carries only the raw data (`ParseContext`, `Token`, a `process` string) needed to
   reconstruct the real Python exception later — it deliberately does *not* reimplement that
   exception's message/location-report formatting logic, which stays living in exactly the one
   pure-Python class. This keeps `core/` itself Python-free.
2. **In `py/`**: the pybind11 binding wraps the throwing call in a try/catch, and on catching the
   core exception, imports the real Python exception class and raises a genuine instance of it
   (`py::module_::import(...).attr("SyntaxErr")`, constructed with the carried `ctx`/`token`/
   `process`, then `PyErr_SetObject` + `throw py::error_already_set()`) — not
   `py::register_exception<T>` (which only carries a plain string message, losing the structured
   `.ctx`/`.token` attributes real callers read) and not a bespoke new Python exception type
   (which wouldn't be catchable by the existing `except SyntaxErr` call sites at all).
3. **Don't hardcode the Python import path.** `"FixRaidenBoss2.exceptions.SyntaxErr"` looks like
   the obvious string to import, but this repo's own Unit Tester harness imports the whole package
   as `src.py.FixRaidenBoss2` instead (see [Testing](../Testing/CLAUDE.md)) — a hardcoded
   `FixRaidenBoss2....` path raises `ModuleNotFoundError` specifically under that harness, a gap
   that only surfaces when a test actually exercises the error path (confirmed the hard way: every
   `SyntaxErr`-raising test passed the "does the .pyd import" smoke check fine and only failed once
   a test actually triggered the exception). Fix: derive the prefix from the *binding module's own*
   `__name__` at `initCppXxx(m)` time (`m.attr("__name__").cast<std::string>()`, strip the trailing
   `.core`) instead of a literal string — this resolves correctly whether `core` was imported as
   `FixRaidenBoss2.core` (normal install) or `src.py.FixRaidenBoss2.core` (Unit Tester harness).

## Cython bindings (`api/src/cy`)

A separate, simpler native-extension layer from the C++ core/pybind11 stuff above — no CMake
subproject knowledge needed beyond what's already in [Building](../Building/CLAUDE.md)'s "Cython
pieces" (it's built automatically as part of the same top-level orchestration, no separate step).
Verified across many small passes through this layer now — most of `CyDictTools`/`DictTools`
(`getVal`, `contains`, `setVal`, `getKeys`/`getCommonKeys`, `getCommonPaths`, `iterPaths`/
`getPaths`, `update`/`updateMany`, `combine`/`combineMany`) plus `CyListTools.updateMany` — each
added as its own two-file (`.pyx` + `.py` wrapper) change, rebuilt and empirically verified before
writing formal tests. What that surfaced, roughly in the order you'll hit it:

- **Every feature here is a two-file change, not one.** The real implementation lives in a
  `cdef class CyXxxTools` in `api/src/cy/src/tools/XxxTools.pyx` (e.g. `CyDictTools`,
  `CyListTools`) — this is what actually compiles to the `.pyd`. But nothing calls it directly;
  a pure-Python convenience class of the un-prefixed name (`DictTools`, `ListTools`, ...) in
  `api/src/py/FixRaidenBoss2/tools/XxxTools.py` holds a module-level `_CyTools = CyXxxTools()`
  instance and exposes each capability as a thin `@classmethod` that just forwards to it
  (`cls._CyTools.getVal(dct, keys, errorOnNotFound = errorOnNotFound, default = default)`). Adding
  a method to the `.pyx` class without adding the matching wrapper classmethod leaves the feature
  effectively unreachable from the documented public API (`FRB.DictTools.xxx`) — `FRB.CyDictTools`
  is not meant to be called by users directly. Do both halves.
- **Docstrings on both sides use the same numpydoc-ish convention** as the pybind11/C++ side
  (`Parameters`/`Raises`/`Returns` sections, `:raw-html:`<br />`` for a blank line inside a
  parameter description). The wrapper method's docstring additionally gets a
  `.. note:: This function is a convenience for calling :meth:`CyXxxTools.method`` line pointing
  back at the Cython method — copy this pattern from an existing wrapper method
  (`DictTools.forDict`/`iterDict`/`getKeys`) rather than inventing new docstring phrasing.
- **The `errorOnNotFound` / `default` parameter pair is this codebase's established convention**
  for any "look something up, maybe it's not there" method — see `ModDictAssets.get` for the
  pure-Python precedent this was matched against. Default `errorOnNotFound=False` returning
  `default` (itself defaulting to `None`); `errorOnNotFound=True` raises (`KeyError` for a
  dict-keyed lookup) instead of returning `default`. Reuse this shape rather than inventing a new
  one (e.g. returning a sentinel, or a `Optional[T]` with no way to distinguish "found None" from
  "not found") when asked for a similar query method.
- **The Cython `.pyx` files favor raw CPython C-API traversal over Python-level iteration** for
  performance — `PyDict_Check`/`PyDict_Next`/`PyObject *` and an explicit `list`-as-stack for
  nested-structure walks (see `forDict`/`iterDict`/`nestedDictToNdArray` in `DictTools.pyx` for the
  established shape), rather than a plain `for k, v in dct.items():`. Match this style for new
  traversal-heavy methods in the same file rather than dropping back to plain Python iteration,
  which would undercut the reason this code is in Cython at all. A method that's a simple
  bounded-depth walk (like `getVal`, which just chases a key list) doesn't need the stack
  machinery — a straight loop with `PyDict_Check`/dict-indexing is enough; don't force the stack
  pattern where a simpler loop already matches the existing methods' complexity budget.
- **The `.pyi` stub (`api/src/py/FixRaidenBoss2/CyXxxTools.pyi`) is auto-generated and near-empty**
  — it just re-exports the class name (`from CyDictTools import CyDictTools`), with no per-method
  signatures. Don't hand-edit it when adding a method; there's nothing there to update.
- **Sphinx picks up a new method automatically** via `autoclass FixRaidenBoss2.CyDictTools` +
  `:members:` in `Docs/src/api.rst` — no manual `.rst` edit needed for a new method on an
  *existing*, already-documented Cy class (only adding a whole new class needs an `.rst` entry;
  see [Documentation](../Documentation/CLAUDE.md)).
- **Rebuild the same way as a C++ core change** (`Tools/APIBuilder`, `py -3 main.py`, see
  [Building](../Building/CLAUDE.md)) — there is no Cython-only fast path, and the build's exact
  MSVC/CMake plumbing is identical regardless of whether you touched `core` or `cy` sources.
- **Verify the rebuilt `.pyd` from the PowerShell tool, not the Bash tool** — see
  [Building](../Building/CLAUDE.md)'s "Verifying a build/binding change in Python directly" for
  the confirmed `DLL load failed ... The parameter is incorrect` failure mode that's specific to
  running Python through Git Bash on this machine, not a real problem with the build.
- **Unit tests for a Cython-backed wrapper method are pure-Python `unittest` tests** against the
  wrapper class (`FRB.DictTools.getVal(...)`), living in the existing `Tests/test_XxxTools.py` —
  no new test module, and no `Tests/__init__.py` registration needed for a new *method* on an
  already-registered wrapper class (only a brand-new test module needs that — see
  [Testing](../Testing/CLAUDE.md)). Follow the file's existing `# ============ methodName =======`
  section-comment convention and `BaseUnitTest`'s `compareDict`/etc. helpers.
- **When porting/reimplementing an *existing* pure-Python method (not writing a brand-new one),
  cross-check the docstring's stated contract against what the current code actually does —
  don't assume current behavior is automatically what to preserve.** `DictTools.combine`'s
  docstring documented `combineDuplicate(key, srcVal, newVal)` (dict1's value, then dict2's), but
  the implementation actually called it `(key, dict2Val, dict1Val)` — reversed. Both of the
  method's existing tests missed this because they used order-insensitive combine functions
  (default-`None`, and an average). What caught it: grepping every real call site for `.combine(`
  and reading the combine-callback lambdas' own parameter names (`srcVal`/`newVal`,
  `srcObjTextures`/`currentObjTextures`, `model`/`curModel` — three unrelated call sites, all
  consistently assuming the documented order) — every one was silently getting the wrong value on
  every conflicting key. Grep real call sites, not just the existing tests, before deciding a
  docs/code mismatch is intentional and should be preserved rather than fixed.
- **Query/read methods and mutate/construct methods handle bad top-level input differently, on
  purpose — match whichever fits the new method.** `contains`/`getVal` (asking "does this exist" /
  "what's here") gracefully degrade for a non-`dict` top-level argument — `contains` returns
  `False`, `getVal` returns `default`/raises exactly like a not-found key — since "the input isn't
  even a dict" is just another flavor of "not found." `combine`/`update`/`updateMany`/
  `combineMany`/`setVal` (building or mutating a dict) raise `TypeError` instead, because there's
  no sensible fallback value to hand back for a construction method. Don't reuse one family's
  error-handling shape for the other kind of method.
- **A plain Python `set` can never expose insertion order, no matter how it's populated** — so
  adding an `ordered` flag to a method that currently returns `Set[...]` is not a pure
  implementation detail, it's a return-type decision, and worth surfacing to the user rather than
  guessing. This happened to `getCommonKeys`: making `ordered=True` actually observable required
  changing its return type from `Set[Hashable]` to `List[Hashable]` (mirroring `getKeys`'s existing
  precedent in the same file) — a breaking change to the method's public contract, including
  existing tests that called `compareSet` directly on the result (needed `set(result)` wrapping
  afterward, see [Testing](../Testing/CLAUDE.md)). If a genuinely set-*shaped*-but-ordered return
  type is ever wanted instead of a plain list, this codebase already has one:
  `GlobalPackageManager.get(PackageModules.OrderedSet.value).OrderedSet(...)` (the `orderedset`
  package, lazy-loaded the same way `nestedDictToDataFrame` lazy-loads pandas) — see its use in
  `ModMappedAssets.py` for the pattern. Nothing in `DictTools` uses it today (the `List[Hashable]`
  route was chosen instead for `getCommonKeys`), but it's the established answer if a future method
  specifically needs "acts like a set, but remembers order."
- **Delegate to sibling methods on `self` instead of duplicating traversal logic, and reach for a
  private `cdef` helper when two or more *public* methods need to share logic that itself shouldn't
  be public.** `getCommonPaths` doesn't reimplement ordered/unordered key-intersection inline — it
  calls `self.getCommonKeys(nodes, ordered = ordered)` at each recursion step. `updateMany`/
  `combineMany` share a `cdef object _mergeMany(self, object target, list allDicts, object
  combineDuplicate)` helper for their "build an occurrence map, then resolve" logic — a `cdef`
  method (unlike `def`/`cpdef`) never becomes part of the Python-visible API at all, so it's the
  right tool specifically for internal-only sharing, not just any refactor.

## Cython gotcha: a `dict`/`list`-typed parameter or local variable requires the *exact* type, not a subclass

Cython's generated argument-type check for a `def`/`cpdef` parameter declared as a builtin
container type (`dict x`, `list x`, also a plain `cdef dict x`/`cdef list x` local variable
assignment) is **stricter than `isinstance`** — it rejects a subclass instance outright:
```
TypeError: Argument 'dct' has incorrect type (expected dict, got collections.defaultdict)
```
This is easy to miss because it's inconsistent with the raw CPython C-API macros this same file
cimports from `cpython.dict` — `PyDict_Check`/`PyDict_Contains` (as declared there) **do** accept
`dict` subclasses (`collections.defaultdict`, etc.), matching normal `isinstance` semantics. Only
`PyDict_Next` still needs an explicit `<dict>` cast when called on an `object`-typed variable
(its cimported signature wants a literal `dict`). So a method can compile and work fine for plain
dicts while silently rejecting anyone who passes a `defaultdict`, until someone actually tries it.

**Fix**: type the parameter (and any local variable that might end up holding a subclass instance,
e.g. `combine`'s `result`, `update`'s `shortDict`/`longDict`) as `object` instead of `dict`/`list`,
then validate explicitly with the permissive `PyDict_Check` macro at the top of the method body —
raising `TypeError` (mutate methods) or returning a graceful default (query methods, see the
bullet above) as appropriate. This is now the standard shape for every `dict`-accepting parameter
in `DictTools.pyx` (`contains`, `getVal`, `setVal`, `combine`, `update`, `updateMany`,
`combineMany` all take `object`, not `dict`) — new methods should follow it too rather than typing
a parameter `dict` and only discovering the rejection later. `ListTools.pyx` hasn't needed this
yet (nothing has asked for `list`-subclass support there) — its `interleave`/`filterInPlace`/
`updateMany` still type `list` directly, which is fine until that changes.

**Related, and reassuring rather than dangerous**: an unchecked `<dict>` cast used purely for a
subscript, e.g. `(<dict>result)[key]`, bypasses `__missing__`/subclass dispatch entirely — it
behaves like a raw dict lookup and raises `KeyError` for a genuinely absent key, the same as a
plain `dict` would, **without** ever invoking a `defaultdict`'s `default_factory`. Verified
empirically (a nested `defaultdict` queried through `getVal` for a missing key left the
`defaultdict` completely unmutated — no accidental auto-vivification). This means the
try/except-`KeyError` style already used in a few of these methods (`getVal`) is safe as-is even
though it doesn't explicitly check `PyDict_Contains` first the way `contains`/`setVal` do — but
don't rely on this being obvious without testing it if you write a new method that indexes a
possibly-missing key this way; confirm it empirically rather than assuming.

## Pure-Python "Tools" wrapper classes: two delegation styles, pick based on whether you're adding behavior

`IntTools`/`Algo`/`ListTools`/`DictTools`/`HashTools` are all pure-Python classes made of nothing
but `@classmethod`s wrapping a `Cpp`-prefixed pybind11 class and/or a `CyXxxTools` instance — but
they use two different mechanisms, and picking the wrong one for what you're doing adds needless
code or loses capability silently:

1. **Pure forwarding (the default — `IntTools`, `Algo`, `ListTools`, `DictTools`)**: no
   inheritance at all. Each classmethod just calls through and returns the result —
   `CppXxx.method(...)` directly for a pybind11-backed method, `cls._CyTools.method(...)` (the
   held `CyXxxTools()` instance from the Cython section above) for a Cython-backed one — and a
   *single* wrapper class is free to mix both within itself (`ListTools.removeParts` calls
   `CppListTools` directly; `ListTools.interleave` calls `cls._CyTools.interleave` a few lines
   later; nothing unusual about that split). Use this when the wrapper isn't adding real behavior
   beyond docs/naming.
2. **Real subclassing (`HashTools(CppHashTools)`)**: use this when the wrapper needs to *extend*
   what the bound class does — e.g. accept a wider argument type than the C++ method understands,
   or share static state (`clear()`) automatically instead of re-declaring a forwarder for it.
   Real Python inheritance from a bound class needs no trampoline (see the pybind11 bindings
   section above) and works with **zero concern about whether the bound base has a registered
   constructor** — subclassing only builds a new *type* object; it never instantiates the base
   unless something actually calls `Subclass()`, which a static-method-only "Tools" class never
   does. `HashTools` subclasses `CppHashTools` even though `CppHashTools` has no `py::init<>()`
   bound at all — confirmed working via `issubclass()`/MRO inspection, not just assumed.

   Watch for the CRTP/pybind11 binding trap from the section above when doing this: any inherited
   method whose signature mentions the *base* type in argument/return position (not just as
   `self`) can't be called through `&Derived::method` the normal way and needs the same
   lambda-over-the-concrete-type fix — applies whether the base is a CRTP template or an ordinary
   pybind11-bound class.

## Adding a new method to `IfContentPart` / `OrderedMultiMap`

`IfContentPart` never touches `BaseOrderedMultiMap`/`OrderedMultiMap` directly — it only ever
holds a `content_` pointer typed as the abstract `IOrderedMultiMap<K, V>` interface. So "add
method X to IfContentPart" is never a one-file change; it's a checklist down the whole
delegation chain. In the order you'll actually touch them:

1. **`BaseOrderedMultiMap.h`/`.tpp`** — the real implementation, if this is a genuinely new
   capability (not already expressible via existing primitives). Free real types are fine here
   (`Ranges<long long>` directly, real hash-based containers with this instantiation's own
   `KeyHash`/`KeyEqual`) — this class is never exposed to Python directly, so it isn't
   constrained by pybind11's RTTI/polymorphism limits.
2. **`IOrderedMultiMap.h`** — a new pure virtual declaration. Types here must stay
   interface-safe: plain `std::vector`-shaped params (`RangeSpec`, not `Ranges<long long>`; a
   plain `std::vector<K>` for "a set of keys", not a real hash set — the interface has no
   `KeyHash`/`KeyEqual` of its own and can't assume an arbitrary `K` is hashable). Adding a
   pure virtual here is a breaking change for existing Python subclasses — see the trampoline
   gotcha below before doing this.
3. **`OrderedMultiMapAdapter.h`** — an `override` forwarding to `impl_`, converting between the
   interface's plain shapes and the concrete type's real ones (`toRanges()`, iterating a real
   set into a vector, etc. — helpers for this already exist in the file, check before
   duplicating).
4. **`IfContentPart.h`/`.tpp`** — a thin method that calls `content_->method(...)`, converting
   back the other way using `IfContentPart`'s own `KeyHash`/`KeyEqual` if you need a real
   container built from the interface's plain-vector result (see the `KeyHash`/`KeyEqual` bullet
   above).
5. **pybind — `PyIOrderedMultiMap.h`/`.cpp`**: the trampoline (`PyBindIOrderedMultiMap`) needs a
   matching override for any new/changed pure virtual, or the binding won't compile. Then bind
   the method on the `IOrderedMultiMap` Python class itself.
6. **pybind — `PyOrderedMultiMap.cpp`, `PyOrderedMultiMapSqrt.cpp`, `PyIfContentPart.cpp`**: bind
   the method on all three concrete Python classes too, even if the request only named one or
   two of them by name — the capability exists on all of them once it's on the shared base, and
   leaving one out is an inconsistent surface a user will trip over later. Each binding file has
   its own established convention for how a `ranges`-shaped param gets parsed from Python
   (`PyIfContentPart.cpp`/`PyIOrderedMultiMap.cpp` use the flexible `parseRanges(py::object)`
   that accepts a bound `Ranges` or a raw list; `PyOrderedMultiMap.cpp`/`PyOrderedMultiMapSqrt.cpp`
   use the stricter `std::optional<PyRanges<long long>>` + `toCoreRanges()` — match whichever
   file you're in, don't introduce a third convention.
7. **Test fixtures**: `PyListOMM`, the pure-Python reference `IOrderedMultiMap` implementation,
   exists as **two separate copies** — one in `test_IOrderedMultiMap.py`, one in
   `test_CppIfContentPart.py`. Both need updating for any interface signature change, or they'll
   silently stop being valid implementations (see the trampoline-arity gotcha immediately below
   for why "silently" is doing real work in that sentence).

## pybind11 trampoline gotcha: changing an existing virtual method's arity breaks every call, not just new-param ones

This is the single most expensive-to-discover gotcha found while extending this codebase.
Adding a new parameter to an `IOrderedMultiMap` virtual method (e.g. `getAll` gained a `ranges`
param) changes its arity. `PYBIND11_OVERRIDE_PURE` forwards **all** C++-side arguments to the
Python override positionally, regardless of what that override's own `def` declares. A
pre-existing pure-Python subclass whose method still has the old, shorter parameter list (e.g.
`def getAll(self, key, ordered=True):`) will raise `TypeError: takes N positional arguments but
N+1 were given` on **every** call through the trampoline — including calls that never touch the
new parameter at all. This is not a hypothetical: it broke this project's own `PyListOMM` test
fixture in exactly this way.

The dangerous part: **this is not caught by "does it compile" or even by a full test-suite run**
unless some test happens to route a call through a Python-backed implementation specifically.
Calling the Python subclass's method directly from Python (`pyomm.getAll(...)`) proves nothing —
that never goes through the trampoline at all, since there's no C++ caller invoking it through
the base pointer. To actually exercise the trampoline path, you have to call through a C++
consumer that holds the object as `IOrderedMultiMap*`, e.g.:
```python
part = FRB.IfContentPart(content=somePurePythonIOrderedMultiMapSubclassInstance)
part.getVals(...)  # this crosses back into Python through the vtable
```
When you change an *existing* `IOrderedMultiMap` virtual method's signature, always: (1) update
both `PyListOMM` copies to match, and (2) explicitly test the trampoline path this way, not just
the C++-native (`OrderedMultiMap`/`OrderedMultiMapSqrt`-backed) path.

## A class whose constructor conditionally calls a virtual `setup()`-style method needs a specific subclassing idiom

A C++ virtual call made from *within a base class's own constructor* can never reach a
more-derived override, even through a `PYBIND11_OVERRIDE` trampoline — the object's dynamic type
for virtual-dispatch purposes is exactly the class currently under construction, never anything
more derived, regardless of what the eventual most-derived type will be. This bit `BaseTokenizer`,
whose constructor does `if (setup) { this->setup(); }`, where `setup()` is itself virtual and
calls two more virtuals (`addStates()`/`addTransitions()`) meant for subclasses to override.
Naively giving `FilteredTokenizer` (and, one level further, `IfPredTokenizer`/`SympyTokenizer`)
the same "constructor takes a `setup` bool and conditionally calls `this->setup()`" shape would
silently run the *base* class's `setup()`/`addStates()`/`addTransitions()` even when constructing
the derived type, since that call happens from inside `BaseTokenizer`'s own constructor body.

**The fix, applied at every level of this hierarchy**: a subclass's constructor always passes
`setup = false` up to its base's constructor (suppressing the base's own internal setup call
entirely), then calls `this->setup()` itself *from its own constructor body*, after the base
constructor has returned. At that point the object's dynamic type is the subclass being
constructed, so the virtual call correctly reaches the most-derived `setup()`/`addStates()`/
`addTransitions()` overrides. This composes cleanly through multiple inheritance levels — each
class in the chain (`FilteredTokenizer`, then `IfPredTokenizer`/`SympyTokenizer` on top of it)
repeats the same "construct base with `setup=false`, self-call `setup()` after" pattern, verified
empirically end-to-end (a real `.ini` predicate parses correctly through the whole chain). If you
add a class that needs specialized `setup()`/`addStates()`/`addTransitions()` behavior and its
constructor takes a `setup: bool = true`-style parameter, follow this same idiom rather than
copying the base constructor's own `if (setup) { this->setup(); }` line as-is.

## A pybind11 property bound over a `std::vector`/`std::map`-typed member (via `pybind11/stl.h`) returns a fresh copy on every access, not a live view

Unlike a real Python list/dict attribute, a C++ member exposed through `.def_readwrite`/
`.def_property` where the type is `std::vector<T>`/`std::unordered_map<K, V>`/etc. gets converted
to/from a brand-new Python `list`/`dict` object on *every single attribute access* — there is no
persistent Python-side object identity backing it. Concretely: `someParseContext.lines.append("x")`
silently does nothing observable, because `.lines` constructs a throwaway Python list, `.append`
mutates that throwaway, and it's immediately discarded; a subsequent `.lines` access shows the
original, unmodified value. This is a real behavior difference from the pure-Python predecessor
this codebase has been porting (`ParseContext.lines` was a real, in-place-mutable Python list
before the C++ port). Caught while writing `test_ParseContext.py`, where a test asserting
in-place-append independence between two instances passed for the wrong reason (both looked
"independent" because *neither* instance's `.lines` was actually mutated by the append at all).
Whole-value reassignment (`ctx.lines = ctx.lines + ["x"]`, or `ctx.lines = newList`) works fine and
*is* what every current real call site in this codebase already does — nothing currently relies on
in-place container mutation through such a property. If you're porting a Python class whose
callers rely on in-place mutation of a list/dict-typed attribute, this is a real gap you'd need to
close (e.g. `pybind11/stl_bind.h`'s `py::bind_vector` for an opaque, mutate-in-place container
type) rather than something the default `pybind11/stl.h` caster gives you for free.

## A newly pybind11-bound class supports neither `copy.copy()` nor `copy.deepcopy()` unless you bind them

Unlike a plain Python class, a fresh `py::class_<...>` registration does **not** get
`copy.copy()`/`copy.deepcopy()` for free — calling either on an instance raises
`TypeError: cannot pickle '...' object` (pybind11 objects aren't picklable by default, and
`copy`'s generic fallback for arbitrary objects goes through pickling). This bit
`IfContentPartColouring` immediately: its pure-Python predecessor (a plain `UserDict` subclass)
supported `copy.deepcopy()` for free, and a caller (`IniSectionGraph.py`) relied on that — the
port silently broke it until `clone()`/`__copy__`/`__deepcopy__` were added, exactly matching
`IfContentPart`'s own existing pattern (see the section right below). When porting a class
that gets deep-copied anywhere in its call sites, bind these three from the start rather than
discovering the gap via a downstream test failure; grep call sites for `copy.deepcopy`/
`copy.copy` on the type being ported before assuming it isn't needed.

**The same gap bites through inheritance, not just a direct port** — a pure-Python class doesn't
have to be *replaced* by a pybind11 class to hit this; giving it a pybind11-bound **base class**
is enough, since the base's missing copy support is inherited too. This happened to `IfPredPart`:
it stayed a plain pure-Python class throughout, but was later re-pointed to inherit from the
pybind-bound `IfTemplatePart` instead of a plain-Python one (see "Two different outcomes" for why),
and immediately started raising `TypeError: cannot pickle 'IfPredPart' object` on `copy.copy()`/
`copy.deepcopy()` at real call sites (`IfTemplate.py`'s own `copy.deepcopy(self)` over its `parts`
list, `GIMIFixerOld.py`, `ResGroupCollect.py`, ...) — caught only when a user hit it live, not
during the migration itself, because the migration's own grep-for-copy-call-sites step (see the
delegation-chain-style checklist elsewhere in this file) wasn't applied to *this* kind of change.
Fixed the same way: `clone()`/`__copy__`/`__deepcopy__` added to `IfPredPart` itself, reconstructing
via `type(self)(...)` with an `id` param threaded through so a copy preserves the original's id by
default (matching what plain-Python inheritance gave it for free before the pybind base existed).
**Lesson**: whenever a pure-Python class's base class changes to (or starts transitively including)
a pybind11-bound one — not just when porting the class's own implementation — re-check copy/pickle
support the same way, even though nothing about the class's *own* code changed.

## Copying/cloning a Python subclass of a pybind11-bound class loses the subclass type

`clone()`/`__copy__()`/`__deepcopy__()` (or any C++ method that constructs-and-returns a fresh
`unique_ptr<T>`) always produces an instance of whatever type was **statically registered** with
pybind11 for `T` — never a Python-level subclass, even if the original object being cloned was
one.

Historical example (no longer reproducible in this exact shape, since the class involved was
later fully replaced — see "Two different outcomes" above): while `IfContentPart.py` still existed
as a pure-Python subclass of `CppIfContentPart`, `CppIfContentPart.clone()` had no way to know a
given instance was actually the Python subclass — `copy.deepcopy()` on such an instance silently
downgraded the result to a bare `CppIfContentPart`, losing whatever extra Python-side state/type
the subclass carried. The general lesson still applies to any *current* pybind11-bound class meant
to be subclassed from Python: that Python subclass needs its own `clone`/`__copy__`/`__deepcopy__`
overrides that go through its own constructor rather than inheriting the C++ ones, using
`type(self)` (not a hardcoded class name) so it keeps working if the subclass is itself subclassed
further — this was the fix applied to `IfContentPart.py` at the time, before it was removed
entirely, and again to `IfPredPart` once it needed it (see the section right above — that one's
root cause was the base having *no* copy support to lose type through in the first place, a
distinct but sibling gap, not this exact "clone() downgrades the type" one).

## Cache-invalidation audits: check every raw-primitive call site, not just the shared helpers

`BaseOrderedMultiMap` has a `mutable`-cache-plus-dirty-bit pattern used twice now (`KeyBucket`'s
own `sortedCache`/`dirty`, and the whole-map `indexMapCache_`/`indexMapDirty_`). When adding a
new cache like this (or auditing an existing one after a new mutating method appears), don't just
find the "shared helper" mutation points (`insertBefore`, `eraseHandle`) and assume that's
everything — grep for every direct call to the underlying raw primitives too
(`rawInsertBefore`, `rawErase`, `rawClear`, `rawRelinkInOrder`). At least one method
(`removeKey`'s own unconditional-removal fast path) calls `rawErase` **directly**, bypassing
`eraseHandle` entirely for a performance shortcut — an invalidation point that's easy to miss if
you only search for calls to the wrapper function. Verify empirically with a script that warms
the cache, performs the mutation, and checks the result is fresh — not just that it compiles.

## Naming: the `Cpp` prefix
If a bare name (`RemappedKeyData`, `KeyRemapData`, ...) already exists as a deprecated
pure-Python class in `FixRaidenBoss2`, the new C++-backed pybind11 binding must be named
`CppXxx` to avoid shadowing it — check `api/src/py/FixRaidenBoss2/` for an existing class of that
bare name before picking a binding name.

Register the `Cpp`-prefixed name **directly in the `py::class_<...>(m, "CppXxx", ...)` call**,
not as a bare name later renamed on import (`from .core import Xxx as CppXxx`) — the latter
silently breaks Sphinx doc rendering for that class (see
[Documentation](../Documentation/CLAUDE.md) for why) even though it works fine at runtime.

## Two different outcomes for porting a class to C++/pybind11 — pick one deliberately

`IfContentPart` went through **both** outcomes over this codebase's history — first as a
`CppIfContentPart`-wrapping outcome-1 class, later converted to outcome 2 once the wrapper turned
out to have no independent behavior left worth keeping separate. `IfContentPartColouring`/
`IfContentPartColourChange` went straight to outcome 2. Both outcomes are legitimate; which one
applies to a new port is a judgment call (ask the user if it's not obvious from the request), not
something to default to blindly:

1. **Wrapper**: the C++-backed class is bound as `CppXxx` (per the `Cpp`-prefix rule above) and
   stays that way as long as the wrapper lives. The pure-Python file of the same bare name
   (`Xxx.py`) is rewritten to *subclass* `CppXxx`, adding back whatever pure-Python-only behavior
   it still needs (e.g. an `id` field, `clone`/`__copy__`/`__deepcopy__` overrides that preserve
   the subclass type — see the section above). The bare name `Xxx` keeps meaning "the pure-Python
   wrapper" everywhere; `CppXxx` is the lower-level building block underneath it. Treat this as a
   waypoint, not necessarily a permanent end state — if the wrapper's own behavior ever shrinks to
   nothing beyond forwarding constructor args (as eventually happened to `IfContentPart`'s), that's
   a signal it's ready to collapse into outcome 2.
2. **Full replacement (what `IfContentPartColouring`/`IfContentPartColourChange`, and eventually
   `IfContentPart` itself, did)**: the bare name itself moves onto the new C++-backed class.
   Normally the old pure-Python implementation is renamed in place to a `...Old` suffix and kept
   only as a deprecated fallback — the same established pattern as this codebase's existing
   `GIMIFixerOld`/`BaseIniFixerOld`/`OldRegNewVals`/`IfTemplatePartOld`. **Exception**: if the
   thing moving out of the way is a thin wrapper with no independent behavior of its own (outcome 1
   collapsing into outcome 2, rather than a genuinely separate legacy implementation being
   deprecated), there's nothing worth preserving under an `...Old` name — delete it outright
   instead, after folding any behavior it *did* still add (e.g. `IfContentPart.py`'s
   `clone(newId=False)` id-preservation default) directly into the C++ core method + its pybind
   binding first, so nothing is silently lost. This is what happened when `IfContentPart.py` was
   removed: no `IfContentPartOld` exists anywhere in this codebase, unlike every other full
   replacement here. Nothing wraps or subclasses anything after this outcome; every call site
   switches straight to the C++-backed class.

If you're doing a full replacement (outcome 2), the concrete steps, in order:
1. Build and verify the new C++ core class + pybind11 binding **first, still under the temporary
   `CppXxx` name**, alongside the still-live old pure-Python `Xxx` — get it fully working and
   tested in isolation (see [Testing](../Testing/CLAUDE.md) for gotchas specific to writing tests
   against a freshly-ported class) before touching any call site. This keeps the change bisectable
   and means a binding mistake never blocks on renaming plumbing too.
2. Rename the old pure-Python class(es) to `...Old`, in their existing file/module, otherwise
   unchanged.
3. Re-register the pybind11 binding under the now-free **bare** name (drop the `Cpp` prefix) —
   the prefix rule above only applies while a live pure-Python class still holds the bare name.
4. Update every call site that imported the old class from its pure-Python module to instead
   `from [...]core import Xxx`, under a `##### CppLocalImports` / `##### EndCppLocalImports` block
   (add that block if the file doesn't already have one — see `IfPredPart.py`'s own
   `from ...core import IfTemplatePart` for the established shape). If there are many call sites
   (e.g. `IfContentPart`'s rename touched ~30 files), a small script that computes each file's
   relative-import depth from its path and rewrites the one import line is far less error-prone
   than 30 manual edits — verify with a git diff spot-check afterward rather than trusting it
   blindly. **A plain `grep` for the import line alone misses indirect references through a
   `DeferredEnum`-style singleton registry** — `GlobalCompilerParts.py` holds lazily-constructed
   tokenizer/parser instances as enum values (`GlobalCompilerParts.IfPredTokenizer.value`), and
   renaming `IfPredTokenizer` to `IfPredTokenizerOld` requires updating both the enum's own
   import/member name *and* every `GlobalCompilerParts.IfPredTokenizer.value`-style attribute
   access elsewhere (`IfPredPart.py`'s `getLogicQuery`/`getIfPredStr`, in this case) — grepping
   only for `from ... import IfPredTokenizer` misses these entirely. Missing one produced a plain
   `AttributeError` at the enum attribute access, not an import error, so it doesn't fail loudly
   at module-load time — it cascaded into ~30 unrelated-looking test failures across `IniFile`,
   `IfTemplate`, `IniSectionGraph`, and the GIMI fixer family (anything that actually parses a
   `.ini` predicate) before being traced back to the two missed call sites. After renaming a class
   that's ever stored as a `DeferredEnum`/registry value, grep for `EnumClassName.OldClassName`
   too, not just the import statement.
5. In `FixRaidenBoss2/__init__.py`: move the import to sit with the other `from .core import ...`
   lines (dropping the `Cpp` prefix there too), and keep re-exporting the renamed `...Old` class
   from its original module — this repo keeps deprecated classes importable at package top level
   rather than dropping them (matches `GIMIFixerOld`/`BaseIniFixerOld`/`OldRegNewVals`, which are
   all still imported and listed in `__all__`). `__init__.py` has **two separate `__all__` list
   fragments** for this (one alongside the C++ imports, one alongside the deprecated-Python ones)
   — both need updating, or you'll end up with a stale/duplicate entry.
6. Update `Docs/src/api.rst`: relocate the class's doc entry to its new bare-name alphabetical
   position (see [Documentation](../Documentation/CLAUDE.md) for a caveat about this section's
   ordering not being perfectly enforced already). Deprecated `...Old` classes don't get their own
   doc entry — check the existing `GIMIFixerOld`/`BaseIniFixerOld` for confirmation there aren't
   any, and don't add one for a new `...Old` class either.
7. Add `test_Xxx.py` under the new bare name (not `test_CppXxx.py` — that naming is only for a
   class that keeps its `Cpp` prefix permanently) and register it in `Tests/__init__.py` (see
   [Testing](../Testing/CLAUDE.md) for a gotcha with that file specifically). **Known gap**: when
   `IfContentPart` went through this outcome, `test_bare_Xxx.py`'s name was already taken by a
   stale, pre-C++-port `test_IfContentPart.py` (already known-broken, unrelated attributes) — the
   still-current, passing `test_CppIfContentPart.py`/`CppIfContentPartTest` was left under its old
   name rather than resolving that collision, since deleting/merging an existing test file is a
   bigger call than a same-turn mechanical rename. If you hit this again, surface the collision to
   the user instead of silently picking a side.
8. Rebuild with `-d` (regenerates the `.pyi` stub and Doxygen XML) and run an actual Sphinx build
   to catch stub/doc issues a plain recompile won't surface. Also clean up any "the C++
   counterpart to the pure-Python ``Xxx`` (``path/Xxx.py``)" framing left in the new class's own
   doc comments from when it was first ported, now that the file that framing points at is
   actually gone — see [Documentation](../Documentation/CLAUDE.md)'s dedicated bullet on this.

## A third outcome: full replacement whose associated literal *data* also moves into C++

Extends the "Two different outcomes" section above for a class that owns a big, hand-authored
literal data table as well as algorithmic code (`Hashes`+`HashData`, `Indices`+`IndexData`, and
similarly-shaped `VertexCounts`/`VGRemaps`). Whether the class's *data* also moves into C++ is a
separate decision from whether the class itself goes full-replacement — default to leaving
genuinely frequently-updated content data (per-game-version character hashes/indices, in this
project's case) in Python even after the class itself is fully C++-backed, and only migrate the
data too on the user's explicit go-ahead; ask rather than assuming either way. See
[Building](../Building/CLAUDE.md)'s dedicated section for the mechanical-generation-plus-
round-trip-verification process this requires — hand-transcribing this kind of data is not an
acceptable risk, and the "wrapper vs. full replacement" framing above doesn't cover it.

When you do migrate the data, the class ends up past outcome 2 into effectively a new one: no
Python source file behind it at all, not even a thin wrapper. Concretely, for `Hashes`/`Indices`:
the pybind-bound class (`py::class_<PyHashes, PyModMappedAssets>`) builds its own `ModDictAssets`
repo directly from an embedded C++ literal table at construction time, with no Python-side
data-loading step of any kind. `git status` after this outcome should show the old `Xxx.py`
**deleted**, not renamed to `XxxOld.py` — unlike outcome 2's usual "old renamed to `...Old`, kept
importable" convention. Reasoning: the `...Old` convention exists to preserve an alternate,
comparison-worthy *algorithm* implementation; a stale, no-longer-updated copy of pure *content
data* serves no comparable purpose and would only silently drift from the real,
actively-maintained C++ table with every future game-patch update, becoming actively misleading
rather than a useful fallback. Git history is the real archive here, not an `...Old.py` file.

**Check for other public entry points exposing the same raw data independently of the class being
ported**, before assuming the class is the only consumer — see
[Building](../Building/CLAUDE.md)'s note on `HashData`/`ModData.Hashes` both needing to keep
returning the same nested-dict shape after the migration, via a new, genuinely reusable
`ModDictAssets::toNestedDict()` export capability rather than a second copy of the data.

## Giving one specific, pre-populated instance of a generic pybind11-bound class extra Python-side argument convenience, without touching the generic class's contract for other users

`ModMappedAssets`/`ModDictAssets` are deliberately generic and strictly positional (full
replacement, `K`/`T` = `py::object`, no notion of column *names* at all). But real callers of
`Hashes`/`Indices` (specific, pre-populated instances of `ModMappedAssets`) pass a flexible
bare-value/list/dict-keyed-by-name filter, matching the pure-Python originals' own contract — e.g.
`GIMIParser.py`'s `getKey(hashVal, version, {"name": "Amber"})`. Making the *generic* class's
`get`/`hasFrom`/`getKey`/`replace`/`replaceAll` accept this shape unconditionally would mean every
other, unrelated use of `ModMappedAssets`/`ModDictAssets` pays for a feature it never needs and
can't use correctly (there are no column names to key by for a generic instance).

The pattern that keeps both working: an **optional, pybind-layer-only** member on the wrapper
class (`PyModMappedAssets::nonVersionIndexNames`, `std::optional<std::vector<std::string>>`, no
equivalent in the Python-free `core/` template — this is purely a Python-convenience concept),
set only at construction time by the specific pre-populated subclasses that want it
(`PyHashes`/`PyIndices`, via a `nonVersionIndexNames` constructor kwarg). Every method taking a
non-version-values argument routes through one shared resolver — `toWildcardList(raw,
*nonVersionIndexNames)` if the member is set, else the original strict already-positional-list-only
path otherwise — so a generic `ModMappedAssets()` constructed without it keeps its original,
unchanged contract exactly, while `Hashes`/`Indices` transparently gain the flexible shape.
`toWildcardList` itself is a faithful C++ port of the pure-Python `BaseModAssets.toWildcardList`:
bare value → position 0 only; `list` → positional with `None`-padding, deliberately
`isinstance(x, list)`-strict (not any generic iterable) so a bare `str`/`tuple` doesn't get
iterated char-by-char/element-by-element by accident; `dict` → keyed by name; `None` → every
position wildcarded. Reach for this same shape whenever a generic, reusable core class needs one
specific pre-populated instance to gain Python-convenience behavior the generic case shouldn't pay
for or be constrained by.

**One sentinel value needs explicit handling beyond plain `None`** when porting this kind of
argument normalization: this codebase's own `FixRaidenBoss2.tools.DictTools.UnHashableNone` class
is a *second*, still-live "no value given" marker, distinct from `None`, that several real call
sites (`GIMIParser.py`'s `hashNonVersionVals`/`indexNonVersionVals` constructor defaults,
`ModType.py`'s `getHashRanges`) use as their documented default instead of plain `None`. Treat both
as equivalent, resolved via a **lazy**, cached `py::module_::import("FixRaidenBoss2.tools.
DictTools")` lookup (first real call, well after package init has finished — no circular-import
risk in practice, since nothing can call an instance method before the whole package has already
finished importing enough to construct that instance) rather than an eager import at
binding-init time. Grep real call sites for `UnHashableNone` (not just `None`) before assuming a
plain `raw.is_none()` check is sufficient when porting this kind of Python-side
argument-normalization convention to C++.

**Before step 2 (renaming the old pure-Python class away), grep for every other class that
subclasses it** — not just the one class that motivated the port. A base class can be shared by
classes you weren't asked to touch and don't have open. Concretely: when `IfTemplatePart` went
through this outcome (full replacement), the request was scoped to `IfTemplatePart`/`IfContentPart`
only, but `IfPredPart` (a wholly separate, pure-Python class, not part of the request) also
subclassed the pure-Python `IfTemplatePart` directly, for its own unrelated reasons (predicate
parts needed an `id` too). Nothing in the codebase does `isinstance(x, IfTemplatePart)`, so this
wouldn't have crashed at runtime — but `IfContentPart` and `IfPredPart` would have silently ended
up on two *different, unrelated* base classes (one on the new C++-backed `IfTemplatePart`, the
other still on the renamed `IfTemplatePartOld`), quietly breaking the "both share a common
`IfTemplatePart` ancestor" assumption `List[IfTemplatePart]` type hints across `IfTemplate.py`/
`IfTemplateTree.py` rely on. `grep -rn "class .*(OldBaseName)"` (and check type hints like
`List[OldBaseName]` for how many concrete subclasses are actually meant to satisfy it) before
renaming a base class away, and flag any subclass outside the request's stated scope to the user
rather than silently deciding whether to migrate it too.
