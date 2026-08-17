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
   blindly.
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
   to catch stub/doc issues a plain recompile won't surface.

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
