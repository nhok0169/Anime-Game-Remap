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

## Cython bindings (`api/src/cy`)

A separate, simpler native-extension layer from the C++ core/pybind11 stuff above — no CMake
subproject knowledge needed beyond what's already in [Building](../Building/CLAUDE.md)'s "Cython
pieces" (it's built automatically as part of the same top-level orchestration, no separate step).
One verified pass through this layer so far: adding `CyDictTools.getVal` +
`DictTools.getVal` (`api/src/cy/src/tools/DictTools.pyx` /
`api/src/py/FixRaidenBoss2/tools/DictTools.py`). What that surfaced:

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
part = FRB.CppIfContentPart(content=somePurePythonIOrderedMultiMapSubclassInstance)
part.getVals(...)  # this crosses back into Python through the vtable
```
When you change an *existing* `IOrderedMultiMap` virtual method's signature, always: (1) update
both `PyListOMM` copies to match, and (2) explicitly test the trampoline path this way, not just
the C++-native (`OrderedMultiMap`/`OrderedMultiMapSqrt`-backed) path.

## A newly pybind11-bound class supports neither `copy.copy()` nor `copy.deepcopy()` unless you bind them

Unlike a plain Python class, a fresh `py::class_<...>` registration does **not** get
`copy.copy()`/`copy.deepcopy()` for free — calling either on an instance raises
`TypeError: cannot pickle '...' object` (pybind11 objects aren't picklable by default, and
`copy`'s generic fallback for arbitrary objects goes through pickling). This bit
`IfContentPartColouring` immediately: its pure-Python predecessor (a plain `UserDict` subclass)
supported `copy.deepcopy()` for free, and a caller (`IniSectionGraph.py`) relied on that — the
port silently broke it until `clone()`/`__copy__`/`__deepcopy__` were added, exactly matching
`CppIfContentPart`'s own existing pattern (see the section right below). When porting a class
that gets deep-copied anywhere in its call sites, bind these three from the start rather than
discovering the gap via a downstream test failure; grep call sites for `copy.deepcopy`/
`copy.copy` on the type being ported before assuming it isn't needed.

## Copying/cloning a Python subclass of a pybind11-bound class loses the subclass type

`clone()`/`__copy__()`/`__deepcopy__()` (or any C++ method that constructs-and-returns a fresh
`unique_ptr<T>`) always produces an instance of whatever type was **statically registered** with
pybind11 for `T` — never a Python-level subclass, even if the original object being cloned was
one. `CppIfContentPart.clone()` has no way to know that a given instance is actually a Python
`IfContentPart` (`FixRaidenBoss2/model/iftemplate/IfContentPart.py`, which inherits from
`CppIfContentPart`) — `copy.deepcopy()` on such an instance would silently downgrade the result
to a bare `CppIfContentPart`.

If a pybind11-bound class is meant to be subclassed from Python, that Python subclass needs its
own `clone`/`__copy__`/`__deepcopy__` overrides that go through its own constructor rather than
inheriting the C++ ones — see `IfContentPart.py`'s overrides for the pattern (uses `type(self)`,
not a hardcoded class name, so it keeps working if the subclass is itself subclassed further).

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
If a bare name (`IfContentPart`, `RemappedKeyData`, ...) already exists as a deprecated
pure-Python class in `FixRaidenBoss2`, the new C++-backed pybind11 binding must be named
`CppXxx` to avoid shadowing it — check `api/src/py/FixRaidenBoss2/` for an existing class of that
bare name before picking a binding name.

Register the `Cpp`-prefixed name **directly in the `py::class_<...>(m, "CppXxx", ...)` call**,
not as a bare name later renamed on import (`from .core import Xxx as CppXxx`) — the latter
silently breaks Sphinx doc rendering for that class (see
[Documentation](../Documentation/CLAUDE.md) for why) even though it works fine at runtime.

## Two different outcomes for porting a class to C++/pybind11 — pick one deliberately

`IfContentPart` and `IfContentPartColouring`/`IfContentPartColourChange` are both C++ ports of
pure-Python classes, but they ended up as two genuinely different patterns. Both are legitimate;
which one applies is a judgment call (ask the user if it's not obvious from the request), not
something to default to blindly:

1. **Wrapper (what `IfContentPart` did)**: the C++-backed class is bound as `CppXxx` (per the
   `Cpp`-prefix rule above) and stays that way permanently. The pure-Python file of the same bare
   name (`Xxx.py`) is rewritten to *subclass* `CppXxx`, adding back whatever pure-Python-only
   behavior it still needs (e.g. an `id` field, `clone`/`__copy__`/`__deepcopy__` overrides that
   preserve the subclass type — see the section above). The bare name `Xxx` keeps meaning "the
   pure-Python wrapper" everywhere; `CppXxx` is the lower-level building block underneath it.
2. **Full replacement (what `IfContentPartColouring`/`IfContentPartColourChange` did)**: the bare
   name itself moves onto the new C++-backed class, and the old pure-Python implementation is
   renamed in place to a `...Old` suffix and kept only as a deprecated fallback — the same
   established pattern as this codebase's existing `GIMIFixerOld`/`BaseIniFixerOld`/
   `OldRegNewVals`. Nothing wraps or subclasses anything; every call site switches straight to the
   C++-backed class.

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
   (add that block if the file doesn't already have one — see `IfContentPart.py`'s own
   `from ...core import CppIfContentPart, IOrderedMultiMap` for the established shape).
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
   [Testing](../Testing/CLAUDE.md) for a gotcha with that file specifically).
8. Rebuild with `-d` (regenerates the `.pyi` stub and Doxygen XML) and run an actual Sphinx build
   to catch stub/doc issues a plain recompile won't surface.
