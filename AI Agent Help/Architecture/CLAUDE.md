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

## The inverse also holds: an **existing `Py*.cpp` binding is a hard contract on the core class's API** — read it before writing or rewriting that core header

The section above warns against trusting an unexercised `core/` file. The opposite mistake is
cheaper to make and more destructive: writing (or redesigning) a `core/` class **while a binding for
it already exists on disk**. Bindings in this codebase are sometimes written before, or in the same
pass as, the core class they wrap — so `py/src/.../PyXxx.h`/`.cpp` can already pin the exact method
names, arities and typedef shapes the core must provide (`PyRegFillMissing.cpp` calls
`Core::makeFillMissing(reg, value, toFront)` and treats `Core::FillMissingFunc` as a one-argument
`std::function`; a core rewrite that renamed those and made the function two-argument compiled
nowhere).

**Before creating or overwriting anything under `core/include/.../<family>/`:**
1. `ls py/src/model/strategies/.../<family>/` — if a `PyXxx.h`/`.cpp` pair is there, it is the
   specification, not a downstream consumer to fix up afterwards.
2. `git status --short` over `api/src/cpp`, `api/src/py`, `Testing`, and `Docs` — untracked `??`
   files are prior work that `git` cannot restore for you if you clobber them (unlike a tracked
   ` M` file, where `git diff` still shows what was there).
3. Build early rather than at the end. `ninja core` is the cheapest possible inventory of "what
   already exists and what it expects" — a first build that fails with
   `'makeFillMissing': is not a member of ...` tells you in one shot that a binding is waiting on a
   different core API than the one you just wrote.

When the two genuinely disagree, **reshape the core to satisfy the binding**, not the reverse — the
binding encodes decisions already made against the Python original's observable behaviour (identity
of stored arguments, which shapes `refresh()` re-derives per call, the `assertIs` contracts in
`test_Xxx.py`), and those are the expensive things to re-derive. See the "three options for how a
binding holds a Python-supplied argument" section below for why those choices are rarely free.

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

## Wrapping a third-party C++ library (e.g. `Z3`) without leaking it into public headers

`AGRemapCore` has zero dependency on Z3 in any of its *public* headers (`tools/z3/Z3Context.h`,
`tools/z3/Z3Predicate.h`, `model/iftemplate/IfPredZ3Generator.h`, etc.) even though the whole
`IfPredZ3Generator`/`Z3IfPredGenerator`/`IfPredPart` family is built entirely on top of `z3++.h`.
This works via a specific pimpl shape, established while building this from scratch (there was no
precedent in this codebase for wrapping a *third-party* dependency this way before Z3 — the
existing pimpl-adjacent patterns, like `BaseOrderedMultiMap`'s CRTP, solve a different problem):

1. **Every public class that needs to hold Z3 state (`Z3Context`, `Z3Predicate`) only ever
   forward-declares a nested `class Impl;`** and holds it via `std::unique_ptr<Impl>`. The public
   header never includes `<z3++.h>`, so nothing that `#include`s `Z3Context.h` transitively needs
   Z3 on its include path.
2. **`Impl` is actually defined in exactly one place**: `core/src/tools/z3/Z3Internal.h` — a
   private header that lives under `core/src/`, *not* `core/include/`, so it's never installed as
   part of the public SDK (`install(DIRECTORY include/ ...)` in `core/CMakeLists.txt` never
   touches `src/`). This is the one file that `#include <z3++.h>`.
3. **Any `.cpp` that genuinely needs to build/read a real `z3::expr`/`z3::context` (not just pass
   the opaque wrapper around) includes `Z3Internal.h`** — currently `Z3Context.cpp`,
   `Z3Predicate.cpp`, `IfPredZ3Generator.cpp`, `Z3IfPredGenerator.cpp`, `IfPredPart.cpp` (needs it
   directly for the trivial `IfPredPartType::Else` case — see its own comment), and `core/tests/`'s
   verification harnesses. Reaching `Z3Internal.h` from a `.cpp` under a different subdirectory
   needs a real include path, not a relative `../../` guess —
   `core/CMakeLists.txt`'s `target_include_directories(... PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/src)` makes `"tools/z3/Z3Internal.h"` resolve from anywhere under
   `core/src/`, mirroring the public `AGRemapCore/...`-rooted include convention for the private
   tree too.
4. **Construction of the wrapper types is deliberately friended to a short, explicit allowlist**,
   not made public — `Z3Predicate`'s real constructor is `private`, with
   `friend class IfPredZ3Generator; friend class Z3IfPredGenerator; friend class IfPredPart;` (the
   only real, non-test callers) plus a
   `friend Z3Predicate makeZ3PredicateForTesting(std::unique_ptr<Impl>)` hook defined *in*
   `Z3Internal.h` for `core/tests/` harnesses to hand-build one from an arbitrary `z3::expr`
   without opening the constructor to real callers. **Friendship does not extend to a free helper
   function just because it lives in the same `.cpp` file as a friended class** — a first draft
   had a free `makeZ3Predicate(z3::expr)` helper inside `IfPredZ3Generator.cpp`'s own anonymous
   namespace calling the private constructor, which doesn't compile (`C2248`) even though it's
   textually right next to `IfPredZ3Generator::generate`, because C++ friendship is granted to the
   *named class*, not its translation unit. Fix: do the construction inline inside the actual
   friended member function, not a sibling free function.
5. For read access instead of construction (`Z3Context::impl()`, `Z3Predicate::impl()`), the same
   friend-allowlist-plus-test-hook shape repeats: private accessor methods, friended to the real
   consumer classes, plus `getZ3ContextImplForTesting`/`getZ3PredicateImplForTesting` free
   functions (also defined in `Z3Internal.h`, also friended) so `core/tests/` can reach a real
   `z3::solver`-buildable `z3::context&` for provable-equivalence checks without loosening the
   production API at all.

If you're wrapping a different third-party C++ library the same way in the future, this is the
shape to copy: opaque-`Impl`-behind-`unique_ptr` in the public header, the *only* file that
includes the real library header lives under `src/` with its own `PRIVATE` include-dir entry, and
both construction and internal-state access go through a short, explicit `friend` allowlist rather
than a public escape hatch.

## `Z3Context`/`Z3Predicate` lifetime: a real dangling-pointer bug, and a real Z3-library constraint neither of us can fix

Two distinct lifetime issues, both confirmed empirically (not by inspection), came out of giving
`IfPredPart` a pybind11 binding and exercising it from real Python code — neither ever surfaced
during this subsystem's own `core/tests/` C++ harnesses, because C++ stack-scoped lifetimes
(declared first, destroyed last) happen to always dodge both.

**Bug #1 (fixed): a `z3::expr` only holds a raw, non-owning `z3::context*`.** Z3's own C++ API
contract is that the `z3::context` must outlive every `z3::expr` built from it — there's no
refcounting on Z3's side at all. `Z3Context`/`Z3Predicate`'s first cut didn't enforce this: nothing
kept a `Z3Predicate`'s owning context alive if the `Z3Context` wrapper it came from went out of
scope first. This is invisible in a C++ test (a `Z3Context` declared at the top of `main()`
naturally outlives everything built from it), but Python's GC/interpreter-shutdown teardown order
is not stack-scoped at all — a `Z3Context` and several `Z3Predicate`s built from it are
independently collectible objects, and whichever gets torn down first, the other's use (even just
its own destructor) dereferences a dangling pointer. Reproduced reliably (`0xC0000005`) with a
script that builds many `IfPredPart`s across a `Z3Context`, drops the `Z3Context`, then reads
`.query` afterward. **Fix**: `Z3Context::Impl::ctx` is a `std::shared_ptr<z3::context>`, not a
plain value, and `Z3Predicate::Impl` holds a second `shared_ptr` copy of the *same* context
(`ctxKeepAlive`, never read, only held) alongside its `z3::expr` — see `Z3Internal.h`'s own comment
for the full reasoning. Every call site that builds a `Z3Predicate::Impl` needs to pass this
second argument now; grep `Z3Predicate::Impl(` for the full list if you're adding a new one.

**Bug #2 (not fixable at this level — a real Z3 library constraint, confirmed in pure C++ with
zero Python involved)**: several *independent* `Z3Context`s, each already garbage-collected/out-of-
scope (kept alive only via the shared_ptr from their own still-live `Z3Predicate`s), then having
those predicates destroyed in an order that **interleaves across the different underlying
contexts** (predicate-of-context-A, predicate-of-context-B, another predicate-of-context-A, ...)
reproducibly access-violates. Destroying them grouped by context (any group order, including
shuffled) is fine; destroying them while every context is still alive is fine. Isolated with a
standalone, zero-pybind11 C++ repro (20 `std::shared_ptr<z3::context>`s + 40 exprs, `std::shuffle`
the destruction order) before concluding this — don't assume a Python-side symptom has a
Python-side cause without checking. See `Z3Context.h`'s own `.. warning::` doc comment for the
full writeup and the practical mitigation: **one long-lived `Z3Context` shared by everything that
needs to be comparable/combinable together** (matching `IniFile._z3Ctx`'s own shape — one per
`.ini` file, not one per predicate) is the safe pattern; don't create-and-discard many short-lived
contexts whose predicates might end up interleaved during teardown. If you're adding code that
builds many `Z3Context`s (e.g. one per test case, or one per worker), keep this constraint in
mind.

**When debugging either of these (or any future Z3/pybind crash) via a standalone `cl`-compiled
reproduction script under the scratchpad**: redirect-to-file output (`cmd //c script.bat >
log.txt`) is *fully* buffered, not line-buffered — a crash mid-run silently loses every `printf`
since the last flush, making the log look like it crashed on the very first line even when it got
much further. Always `std::setvbuf(stdout, nullptr, _IONBF, 0);` at the top of `main()` in any
standalone diagnostic `.cpp` before trusting where in the output it stopped.

## Combining two `z3::expr`s from different contexts is a silent no-op-safety-net, not a catchable error — a third Z3 gotcha, distinct from the two lifetime ones above

`z3++.h`'s own cross-expression guard, `check_context(a, b)`, is implemented as a plain
`assert(a.m_ctx == b.m_ctx)` (see `z3++.h` around `inline void check_context(...)`) — **not** a
thrown exception. In a build where `NDEBUG` is defined (a normal release build), this assert
compiles out to nothing at all: `operator&&`/`operator||`/`operator!`/etc. on two `z3::expr`s from
different `z3::context`s then either silently produces a nonsense expression or crashes deep
inside the Z3 C API, with no C++-level exception to `catch` on the way. **Do not rely on
`try`/`except` around a `Z3Predicate` combination to safely detect a context mismatch** — it isn't
guaranteed to raise anything at all.

This matters concretely wherever code combines two `Z3Predicate`s that don't already provably
share one `Z3Context` — which happens for real in `ResGroupCollect.py` (see [Ini Graph
Editing](../IniGraphEditing/CLAUDE.md)'s section on this), since two `IniSectionGraph`s being
combined there routinely come from two different `.ini` files, each with its own `Z3Context`. The
pattern that's actually safe: check first, via `Z3Predicate.sameContext(other)` (predicate vs.
predicate) or `Z3Predicate.belongsTo(ctx)` (predicate vs. a specific `Z3Context`) — both are cheap
raw-pointer-identity comparisons, no solver involved — and only reach for
`IfPredPart.reparent(predicate, targetCtx)` (a real `.ini`-text round trip: render via
`IfPredPart.getIfPredStr`, re-parse against the target context via `IfPredPart.getLogicQuery`) when
the check actually fails. Never combine two `Z3Predicate`s with `&`/`|`/`~`-adjacent operations
speculatively "and see if it throws."

## A pybind11 wrapper for a raw, non-owning pointer is only alive while *something* holds a real Python reference to it — `py::cast(ptr, reference)` does not create that reference itself

This is a distinct lifetime bug class from the `Z3Context`/`Z3Predicate` section above (that one is
about a C++-side `shared_ptr` keeping a `z3::context` alive; this one is about the **Python
wrapper object itself** disappearing) — but the same root shape: a lifetime invariant that's
invisible from a C++ test (stack-scoped, deterministic) and only breaks once Python's GC/refcounting
owns the teardown order. Found three times porting `IniSectionGraph`/`IfTemplate`/`CallGraph`
(`AGRemapCore::IniSectionGraph`, see [Ini Graph Editing](../IniGraphEditing/CLAUDE.md) for the
subsystem) — once as a real access-violation crash, once as a silent, wrong-answer data
regression, both confirmed empirically, neither caught by the port's own initial test pass.

**The mechanism.** A binding that stores a raw C++ pointer (`Section*`, `ContentPart*`,
`Z3Context*`, ...) and later needs to hand a *Python* value back for it — an accessor returning the
object itself, or an integer id used as a dict key (`pyIdOfPart`'s `id(part)` correlation, see
`PyNodeIdentity.h`) — does this via `py::cast(ptr, py::return_value_policy::reference)`. pybind11's
instance registry means this *reuses* an already-alive wrapper for that pointer if one currently
exists — but it does **not** itself keep anything alive. If nothing else in Python holds a
reference to that pointer's wrapper, `py::cast` constructs a brand-new one, and it dies the instant
the local `py::object` holding it goes out of scope (the end of the C++ function, typically). Two
distinct failure shapes follow from this, depending on what happens next:

- **A crash**, if the *underlying C++ object itself* is only kept alive by that dying Python
  wrapper (e.g. an optional constructor argument like `z3Ctx` — nothing else in the C++ side owns
  it once the wrapper is gone). Reproduced with `IniSectionGraph(..., z3Ctx = Z3Context())`
  (inline-constructed, no separate reference held) — constructs fine, works fine, and only
  access-violates later, on interpreter shutdown, well after the code that "worked" already ran.
  `faulthandler`'s own crash report shows `<no Python frame>` for this shape — the crash happens
  during a destructor chain, not at any line you can point to.
- **A silent, wrong-answer regression**, if the wrapper's only purpose was to compute an **id**
  (`reinterpret_cast<uintptr_t>(wrapper.ptr())`) used as a dict key, and the underlying C++ object
  itself is fine (owned elsewhere, e.g. by the section's own `parts_` vector). The dying wrapper's
  address gets reused by CPython's allocator for the *next* temporary wrapper created in the same
  loop (very likely, since it's freed and re-requested at nearly the same moment, same size class)
  — two different C++ pointers silently collide onto the same integer id. No exception, no crash;
  a `dict.get(id(part), default)`-shaped lookup elsewhere in the codebase (`RegSurroundedAdd.py`'s
  own `predecessors.get(id(part), [])`) just quietly misses and falls through to its default,
  producing a plausible-looking but wrong result. This is *much* harder to catch than the crash —
  it doesn't announce itself at all, and the very same code path can look correct in a slightly
  different test that happens to warm the registry first (see the testing note below).

**The fix, applied three times so far (`PyIniSectionGraph.h`'s `keepAlive_`/`partsKeepAlive_`/
`z3CtxKeepAlive_`)**: a real Python-level container (`py::dict`/`py::list`, or a single
`py::object` for a single optional value) on the *owning* wrapper class, holding a genuine strong
reference to every child wrapper (or the one optional argument) that needs to outlive individual
accessor calls — refreshed synchronously, before returning to Python, at the end of *every* binding
method that can introduce a pointer not already wrapped elsewhere (the constructor is the obvious
one; also any method that builds new owned data, like `deepcopy`/`combine`/`build`). This mirrors
the "for free" protection a pure-Python `self.sections = sections` gets automatically from the
dict's own refcounting — a raw-pointer-only C++ port has no such automatic protection and must
recreate it explicitly. `PyIniSectionGraph`/`PyCallGraph`/`PySectionIterData` are covered this way;
standalone `IfTemplate.parts`/`IfTemplate`'s own static `computeSectionPredecessors` are a known,
deliberately-scoped-out gap (fixing it needs `PyIfTemplate` to become a real C++ subclass with its
own `keepAlive_`, which ripples into every other binding that currently treats it as a plain
alias for `Section*` — judged too large/risky to fold into an unrelated session; flag this if
asked to touch `IfTemplate.parts` identity again).

**Whenever you add a new class in this style** (stores raw non-owning pointers into other
Python-constructible objects, and exposes an accessor or an `id()`-keyed correlation for them):
think through what happens if every argument is constructed **100% inline**, with no separate
Python variable ever holding a reference to it (`IniSectionGraph({"a": IfTemplate([IfContentPart(...)])}, ...)`, not `parts = [IfContentPart(...)]; t = IfTemplate(parts); graph = IniSectionGraph({"a": t}, ...)`)
— this is an extremely common calling convention in this codebase's own tests and real fixer code,
not a contrived edge case. **Test it that way specifically** — a test that happens to hold a named
variable for every constructed piece can pass by pure accident (the variable's own reference keeps
the wrapper alive, masking the bug entirely), which is exactly how this went undetected through the
port's first test pass. If a repro only fails when everything is inline-constructed and passes the
moment you add one `x = IfContentPart(...)` line before using it, that's the signature of this bug
class, not a flaky test.

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

- **A pure-Python subclass of a pybind11-bound class gets its own `__dict__` for free — no
  `py::dynamic_attr()` needed on the C++-registered base.** `py::dynamic_attr()` is only required
  to set arbitrary attributes directly on instances of the *bare* pybind11 type itself (confirmed:
  `CppPixelFilter()` has no `__dict__` and raises on `.transforms = [...]`, see
  [Testing](../Testing/CLAUDE.md)'s note on this). The moment you subclass it from ordinary Python
  (`class PixelFilter(CppPixelFilter): ...`, `class TextureFile(CppTextureFile): ...`), standard
  CPython heap-type rules kick in and the subclass gets a real `__dict__` automatically — this is
  exactly what every "Wrapper" outcome class (see "Two different outcomes" above) relies on to hold
  genuine Python-only state the C++ core never sees at all (`TextureFile.img`/`.info`/`.engine`/
  `.readPillowImg`, `TexEditor.filters`, `PixelFilter.transforms`, ...). Don't reach for
  `py::dynamic_attr()` (or any other ceremony) to support this — it isn't needed, and this codebase
  doesn't use it anywhere.
- **A binding for a "run this" dispatch/entry-point method (`__call__` on a filter, or any similar
  driver that's supposed to invoke another one of the class's own methods) must call that inner
  method through genuine Python attribute lookup (`self.attr("innerMethod")(...)`), never by
  binding straight to the C++ member (`&Base::innerMethod`) or calling it directly in C++.** The
  latter silently skips any override a pure-Python subclass provides for just the inner method —
  e.g. `class Foo(BaseTexFilter): def transform(self, texFile): ...` (no `__call__` override at
  all) still needs `filterInstance(texFile)` to run `Foo`'s `transform`, not `BaseTexFilter`'s
  no-op. See `PyBaseTexFilter.cpp`'s `__call__` binding for the reference shape — its own comment
  spells out exactly why `self.attr("transform")(texFile)` is required instead of the more obvious
  `&AGRC::BaseTexFilter::transform`. This is a distinct, more actionable case of the "no trampoline
  needed when the call originates from Python" rule above — it's specifically about how to *write*
  that Python-originating call correctly, not just whether a trampoline is needed at all. Hit twice
  in the texture-editing port (`BaseTexFilter::__call__`, then again for `PixelFilter`'s per-pixel
  native-fast-path classification below) — check for this shape whenever a new dispatch method is
  added to a bindable base class.
- **Classifying whether a Python object carries a *genuine* C++-level override of a virtual method
  (to decide whether a fast native path is safe) can't rely on `py::isinstance<Base>(obj)` alone**
  — a pure-Python subclass that only overrides the inner method (not the dispatch entry point, see
  above) still passes `isinstance`, since it's a real bound C++ subobject; calling straight into
  its C++ method would silently run the no-op base implementation instead of the Python override.
  The reliable discriminator: ordinary Python attribute lookup on a *class* returns the exact same
  descriptor object for every class in the MRO that doesn't itself define the attribute, so
  `type(obj).method is Base.method` (via `py::type::of<Base>().attr("method")`, compared with
  `.is()`) is true for every native C++ leaf subclass and false for any genuine Python override.
  See `PyPixelFilter.cpp`'s `classifyTransforms`/`TransformEntry` for the reference implementation
  (used to pick between calling a `CppBasePixelTransform*` directly per-pixel vs. falling back to a
  real Python call). **Gotcha**: `.is()` fails to compile (`C2664`, template deduction failure) when
  called directly on a chained `.attr(...)` accessor — materialize the result as a `py::object`
  local first (`py::object x = obj.attr("__class__").attr("method"); x.is(other)`), don't chain
  `.is()` straight onto the accessor expression.

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

## The same arity trap exists with **no trampoline at all**, via keyword-argument dispatch — and it's the more likely one in this codebase now

The gotcha above is about `PYBIND11_OVERRIDE*` forwarding C++ arguments positionally. There is a
second, independent version of it that bites classes with **no trampoline whatsoever**, and most of
this codebase's bound edit base classes (`BaseRegEdit`, `BaseIniGraphEdit`,
`BaseIniGraphGroupEdit`) are exactly that shape — plain aliases, deliberately trampoline-free,
because every call originates from Python.

Those classes are driven by a C++ *dispatcher* that calls into Python by attribute lookup with
keyword arguments — `filter_.attr("edit")(graphObj, modType_, py::arg("modName") = modName, ...)`
in `PyGraphGroupEdit.cpp`'s `PyPartEdit::editGraph`. **Adding a parameter to the bound base's
`edit`/`editFromIni` means adding a keyword to that call, and every pre-existing pure-Python
subclass whose override still has the old parameter list dies immediately** with
`TypeError: edit() got an unexpected keyword argument 'trackKeys'`. Confirmed hands-on adding
`trackKeys`/`keysToTrack` to `BaseIniGraphEdit`: it broke `RegSurroundedAdd.edit` (the one
production subclass) plus five test spies across `test_BaseIniGraphEdit.py`,
`test_GraphGroupEdit.py`, and `test_RegFillMissing.py` — none of which the C++ compile flagged at
all, since nothing about it is a C++-level error.

**Do the subclass inventory first, before writing the signature change** — a two-minute grep that
turns an unbounded surprise into a known edit list:
```bash
grep -rn "BaseIniGraphEdit)" --include=*.py "Anime Game Remap (for all users)" Testing
grep -rn 'def edit(self, graph, modType, modName = "", partFilter = None)' --include=*.py .
```
Grep for the *override signature text*, not just the subclass declaration — the spies inside test
files are the bulk of the hits, are easy to forget, and are exactly what a "does it compile / does
the feature work" check never exercises.

**Design corollary for the parameter you're adding**: a plain `bool` parameter carrying "the
caller's default" cannot be told apart from "the subclass explicitly said false", since the
subclass's own member default is also `false`. Decide deliberately between (a) combining with `||`
— the caller can turn the behaviour *on*, a subclass can never opt out; correct when the flag is a
pure performance/observability choice that cannot change results — and (b) making the subclass's
own member a `std::optional<bool>` so "unset" is expressible; correct when opting out has real
semantics. `RegFillMissing::effectiveTrackKeys` took (a) deliberately (key tracking only ever adds
information for a filter to read, so suppressing it could never change which parts get filled); its
`keysToTrack` needed no such trick, since `std::nullopt` already meant "every key" and doubles as
"unset".

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
   `GIMIFixerOld`/`BaseIniFixerOld`/`OldRegNewVals`. **Exception**: if the
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

**A step 9 exists, on explicit user request only: fully deleting an `...Old` class once it's
served its purpose.** The `...Old` convention above is a deliberate comparison/rollback safety
net, not a permanent fixture — once the new C++ class has real test parity and nothing in the
codebase still depends on the old one (confirmed with a repo-wide grep for the exact class name,
not just its module path), the user may ask for it to be removed outright rather than kept as
permanent dead weight (done for the full `IniSectionGraph`/`IfTemplate`/`IfTemplateNode`/
`IfTemplateTree`/`CallGraph`/`SectionIterData` family, and separately for `IfPredPartOld`/
`IfTemplatePartOld`/an orphaned `IfContentPartColour.py`). **Don't do this unprompted** — the
whole point of keeping it as `...Old` in step 2 above was to give the user a comparison/rollback
option, so silently deleting it later defeats that. When asked to, the full cleanup checklist:
delete the `...Old.py` file(s) and any dedicated `test_...Old.py`/base-test files for them; remove
the corresponding import line and `__all__` entry from **both** `__init__.py` fragments (step 5
above) and both `Tests/__init__.py` places (import + `__all__`, see
[Testing](../Testing/CLAUDE.md)); grep the whole repo for the exact `...Old` class name one more
time — not just `__init__.py` — since a doc-comment/prose mention (this project's own Doxygen
comments explaining "the deprecated original is renamed `XxxOld`", or a comparative comment in an
unrelated file explaining why new code is simpler than the old approach) can reference it by name
without ever importing it, and that mention becomes actively misleading (a dangling pointer to a
file that no longer exists) once the file is gone — reword rather than leave it; rebuild with `-d`
(the auto-generated `.pyi` stub needs refreshing too, same as step 8); full-suite re-run and
confirm the failure/error *count* is unchanged from immediately before the deletion (only the
tests belonging to the deleted `...Old` file(s) should disappear from the total).

**Before deleting an `...Old` test file specifically, check whether it's the *only* place testing
some other, genuinely unrelated module that just happened to live inside it** — an unchanged
failure/error count (per the checklist above) does *not* catch this, since a module going from
"tested" to "completely untested" produces zero new failures, only a silent coverage gap. Confirmed
missing after `test_IniSectionGraphOld.py` was deleted: `tools/GraphTools.py` (a generic,
`IniSectionGraph`-agnostic dataflow-fixpoint engine, unrelated to and untouched by this exact port
— see [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)) had its *entire* test suite living as a
handful of methods inside that one file, with no dedicated `test_GraphTools.py` of its own. Deleting
the file dropped that coverage to zero without a single test turning red anywhere. Before deleting
any `...Old` test file, skim its test method names for ones that don't actually mention the class
being deleted (a `test_graphTools_...`-prefixed method sitting inside `test_IniSectionGraphOld.py`
was the tell here) — if a chunk of it is really testing something else, split those out into their
own dedicated test file for that other module first, using plain, self-contained inputs (a
dict-graph literal for `GraphTools`, not a whole `IniSectionGraph` built just to reach it) rather
than porting the test through whatever class happened to be convenient to construct at the time.

**If asked to restore something from a `...Old` file after it's already been deleted** (production
code, tests, or a docstring example — happens more often than expected, since "delete the old
implementation" and "keep its test coverage/docs" are two independent decisions the user can revisit
separately), the content is very likely still recoverable from git history even with no distinct
commit of its own — see [Documentation](../Documentation/CLAUDE.md)'s dedicated bullet on the
`git show <deletion-commit>^:<path>` technique for exactly this case, including why it still works
after a rename-then-delete that only ever landed as one big squashed commit.

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

## A "port class X to C++" request can secretly be two separate migrations — check what downstream code depends on X's *value types*, not just what imports X

Before starting a full-replacement (outcome 2) port, grep every real call site for more than just
`from ... import X` / `isinstance(x, X)` / attribute reads whose type doesn't change (`.type`,
`.src`, an `int` id, ...) — also check whether anything depends on the *type* of a value-typed
attribute the port is about to change. This is a materially bigger, separate piece of work than
the class rename itself, and it's easy to miss because the class's own file/import path looks
completely migrated while a downstream consumer silently breaks the moment it's actually
exercised.

Concretely: porting `IfPredPart` from a pure-Python/`sympy`-based class to a new
`AGRemapCore::IfPredPart` (Z3-based) looked, from the class's own call sites, like a normal
outcome-2 migration — `IfTemplate.py`/`IfTemplateTree.py`/`IniFile.py`/`BaseIniFixerOld.py` all
just construct it or check `isinstance`/`.type`, none of which care what `.query`'s *type* is.
But `IniSectionGraph.py`/`ResGroupCollect.py` (a wholly separate, `sympy`-based dataflow-analysis
subsystem — see [Ini Graph Editing](../IniGraphEditing/CLAUDE.md)) read `.query` and combine it
via `sympy.And`/`Or`/`Not`/`simplify`/`.replace(sympy.Ne, ...)`, plus an actual SMT
satisfiability check (`sympyLogicInference.satisfiable(..., use_lra_theory=True)`) — none of that
continues to work once `.query` becomes a `Z3Predicate` instead of a `sympy.Boolean`. This wasn't
a small fixup; it required its own separate design pass (extending `Z3Predicate` with real
boolean-combination operators, `.simplify()`, `.isSatisfiable()` via a real `z3::solver`, and a
`reparent()` operation to move a predicate across `Z3Context`s when two graphs being combined
don't already share one) before that subsystem could be migrated at all. **Trace the actual data
flow, not just the direct call sites** — `.query`'s value doesn't stay inside `IfPredPart`; it
flows into `SectionIterQueryData.query` (`model/SectionIterData.py`), out through
`IniSectionGraph.iterByQuery`'s `queryPath`, and back into a freshly-constructed `IfPredPart` in
`ResGroupCollect.py`, all without a single `IfPredPart`-typed variable name anywhere in between to
grep for.

If you find this kind of split scope mid-migration, surface it and ask how the user wants to
scope it (do both now, do the mechanical part now and spin the value-type migration off
separately, ...) rather than silently doing only the part that "looks done" from the import graph
alone — this is the same "ask about wrapper vs. full replacement" judgment call as the rest of this
section, just one level less obvious because the class's own call sites don't reveal it.

**Test files need the exact same call-site sweep as `src/py`, and can hide a much larger number of
mechanical breakages behind one constructor-signature change.** A new required constructor
parameter (`IfPredPart` gained a mandatory `z3Ctx` argument) broke 221 scattered
`FRB.IfPredPart(src, type)` call sites across 7 test files (`test_IfTemplate.py`,
`test_IfTemplateTree.py`, `test_RegSurroundedAdd.py`, `test_GIMIObjParser.py`,
`test_GraphInherit.py`, `test_IfTemplateNormTree.py`, `test_IniFile.py`) that a first pass through
"real" production call sites never touched at all, since none of them are under `src/py`. None of
these were funneled through one shared test-fixture helper, so hand-editing 221 call sites wasn't
realistic — a small, bracket-depth-and-string-literal-aware Python script (find each
`FRB.IfPredPart(`, walk forward tracking paren depth while skipping over string-literal contents,
insert the new argument right before the matching close-paren) fixed all of them in one pass,
plus one `_Z3CTX = FRB.Z3Context()` module-level constant inserted per file. This is the same "a
small script is far less error-prone than N manual edits" guidance already given elsewhere in this
file for a ~30-call-site import rename — it applies at least as strongly here, just for a
different kind of call-site edit (inserting an argument, not rewriting an import line). Verify with
a git diff spot-check on a couple of the files before trusting it, then actually run the affected
test modules (not just a syntax/`py_compile` check) — `Testing/CLAUDE.md`'s "known-broken module"
list exists precisely so you can tell a genuinely new regression (a test module *not* on that list
starting to fail) apart from pre-existing, unrelated noise.

## Porting a class that touches `IfContentPart`/`OrderedMultiMap` data: it has to become a class template, even if the pure-Python original had no generics at all

The Python-facing `IfContentPart` is `IfContentPart<py::object, py::object, PyObjectHash,
PyObjectEqual>` (see `PyIfContentPart.h`) — every key and value that crosses the pybind11 boundary
is a `py::object`. A plain C++ caller of `AGRemapCore`, meanwhile, wants
`IfContentPart<std::string, std::string>`. **A core class that names either instantiation
concretely can only ever serve one of those two callers**, so anything meant to be both
standalone-C++-usable *and* reachable from Python has to be templated over the same
`<K, V, KeyHash, KeyEqual>` and let the caller pick. Confirmed the hard way porting the `regEdits`
family: a pre-existing `BaseRegEdit` stub pinned to `<std::string, std::string>` looked perfectly
reasonable and was completely unusable from the binding layer.

Practicalities when converting an existing non-template core class:
- **Give the template parameters defaults** (`template <typename K = std::string, typename V =
  std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>`). Every
  existing C++ call site then reads identically apart from a `<>`, and the "obvious"
  instantiation stays the obvious one.
- **The `.cpp` becomes a `.tpp`**, included at the bottom of the header (`#include "Xxx.tpp"`,
  matching `IfContentPart.h`) — and it must be **removed from `core/CMakeLists.txt`**, since
  there's no longer a translation unit to compile.
- `.tpp` files are **not** in `Doxyfile`'s `FILE_PATTERNS`, so all Doxygen-visible documentation
  has to live in the `.h`. That's the existing convention (`IfContentPart.tpp` carries no doc
  comments either), not an oversight to correct.
- A class template still gets a perfectly normal `.. doxygenclass:: AGRemapCore::Xxx` entry in
  `coreAPI.rst` — Breathe handles templates fine.

## A ported class whose signature names a *still-pure-Python* collaborator (`ModType`, `IniFile`) can't take it by reference

`AGRemapCore::ModType` and the Python API's `ModType` (`model/strategies/ModType.py`) are **two
unrelated classes that happen to share a name** — the former is bound as `CppModType`, the latter
is still pure Python and does not subclass it. `AGRemapCore::IniFile` isn't bound to Python at all.
So a newly-ported class whose methods mention either type has nothing castable to hand over from
the binding layer, no matter how the signature is written.

The shape that works, used by `BaseRegEdit`/`RegAdd`/`RegNewVals`/`RegRemap`/`RegRemove`:
- **Core takes them as nullable pointers** (`IniFile* ini = nullptr`, `const ModType* modType =
  nullptr`), not references. This is the same convention `partRanges` already used for "optional
  collaborator", so it doesn't read as a special case.
- **The binding takes them as opaque `py::object` and passes `nullptr` down.** Say so in a comment
  at the call site; it looks like a bug otherwise.
- **If the Python object is genuinely needed** (e.g. to hand to a user-supplied callback), route it
  through the *Python* side instead — capture it in the binding's own lambda rather than trying to
  smuggle it through the C++ parameter. See the next section, and `PyRegNewVals::refresh` for the
  worked example.

Re-check this whenever the Python-side class later becomes C++-backed: at that point the pointer
can start being real, and the `nullptr` comments become stale rather than merely ugly.

## How a binding should hold a Python-supplied container/callable constructor argument — three options, and when the choice is forced

A ported class whose `__init__` just stored a `dict`/`list`/callable (`self.vals = vals`) has three
plausible bindings. They are **not** interchangeable, and the existing tests usually decide it for
you:

1. **Parse once into a C++ member; reconstruct a Python value in the getter.** The
   `PyColourReplace.coloursToReplace` pattern (`parseColourOrRangeSet` / `colourOrRangeSetToPy`).
   Cleanest when the value is plain data. Loses object identity (`someEdit.vals is theDictYouPassed`
   is `False`) and ignores in-place mutation of the original.
2. **Store the `py::dict`/`py::object` and use it directly.** `PyIniGraphGroup`'s pattern, chosen
   there because call sites depend on dict aliasing.
3. **Store the raw `py::object` *and* re-derive the C++ member from it at the start of every
   operation** (`PyRegXxx::refresh(modType)`). Keeps identity *and* honours in-place mutation, at
   the cost of re-parsing a small container per call.

**Three things force you off option 1**, all of which applied to `regEdits`:
- An existing test asserting `self.assertIs(edit.someArg, theThingIPassedIn)`. Read the class's
  `test_Xxx.py` *before* designing the binding — see [Testing](../Testing/CLAUDE.md).
- The pure-Python original's in-place-mutation semantics being observable.
- **A member holding Python callables.** pybind11's `<pybind11/functional.h>` caster cannot hand a
  `std::function` back to Python as the *same* callable it was built from — it re-wraps it in a
  fresh `cpp_function` (see its `type_caster<std::function<...>>::cast`). So any getter that
  reconstructs from a parsed `std::function` member silently breaks callable identity. This is
  exactly why `RegRemove` (whose dict values are predicates) can't use option 1.

**The same reasoning applies to a shared spec/marker class whose callback arity differs per
consumer.** `ReplaceIf` originally stored a pre-baked `std::function<bool(const py::object&)>`,
which locked every consumer to that one signature — and `RegNewVals` needs to call the predicate as
`predicate(oldValue, modType)`. Fix: store the **raw `py::object`** and expose both a raw accessor
(`predicateObj()`, for a consumer supplying its own argument list) and a narrowing adapter
(`predicate()`, returning the 1-arg `std::function` every `replaceVals` still wants). Two things to
carry over if you do this to another marker class: re-add the callability check the functional
caster used to give you for free (`PyCallable_Check` + `py::type_error`), and bind the
Python-visible property to the *raw* accessor so identity round-trips.

## A pybind11 constructor taking `vector<unique_ptr<T>>` must pick disown-and-transfer vs. clone-and-copy deliberately — don't default to `IfTemplate`'s pattern

`IfTemplate`'s own constructor takes ownership of its `IfTemplatePart` children by disowning the
Python-side objects passed in (`cast<unique_ptr<T>>()`-style extraction), which is correct *because*
each `IfTemplatePart` is a unique, identity-bearing node — nothing else in the codebase holds onto
that same Python object afterward, so nothing observes it becoming a husk. That pattern is not the
default answer for every `vector<unique_ptr<T>>`-typed constructor parameter; it's specifically
right for unique/identity-bearing children and specifically wrong for shareable *value* types.

This bit `BufElementType`'s constructor, which takes a `vector<unique_ptr<BufDataType>>` for its
component types. Copying `IfTemplate`'s disown-on-construction pattern here broke the moment the
same Python `BufDataType` instance got reused across more than one `BufElementType` — which is
exactly what real call sites do: `constants/BufElementTypes.py` builds entries like
`[BufDataTypes.Float32.value] * 3`, and `BufDataTypes.Float32` is a `DeferredEnum` member whose
`.value` is lazily constructed *once* and cached forever — every subsequent `.value` access (and
every `* 3` repetition in that same list literal) hands back the *same* Python object. The first
`BufElementType` built from it disowned that shared instance; the next attempt to reuse it raised
`ValueError: ... Python instance was disowned`. This is easy to miss because per-class unit tests
that each construct their own fresh `BufDataType` never reuse an instance, so the bug only surfaces
once something exercises the *real* shared-instance call sites end-to-end (see
[Testing](../Testing/CLAUDE.md) for the corresponding test-coverage gotcha).

**The fix**: give the value type a real polymorphic `clone()` (pure virtual on the abstract base,
`return std::make_unique<ClassName>(*this);` on every concrete leaf) and a real copy
constructor/assignment operator on the owning class (via a small `cloneAll()`-style static helper
that clones each element of an existing `vector<unique_ptr<T>>`), then change the pybind11
constructor binding to **clone rather than disown** each incoming element —
`arg.cast<const BufDataType&>().clone()` (or the owning class's copy constructor over a
`cast<const BufElementType&>()` reference), not `arg.cast<unique_ptr<BufDataType>>()`.

Note there's a separate, real MSVC gotcha in this area: MSVC can try to instantiate a class's
implicit copy constructor's body from deep inside pybind11's `smart_holder` unique_ptr caster and
fail with a `C2672`/deleted-function error, for a class that's never actually copied at runtime.
For a genuinely identity-bearing, disown-on-construction type (`IfTemplatePart`-style), the correct
fix is an explicit `=delete` on the copy ctor/assignment — that's the right design regardless of
the MSVC symptom, not a workaround for it. But don't generalize that fix into a blanket default of
`=delete` for *every* `vector<unique_ptr<...>>`-owning class — `BufElementType` hit the identical
MSVC symptom initially, and the correct fix there was the opposite: give it a real copy
constructor (via `clone()`), not delete it, because real callers do reuse instances. Decide
disown-vs-clone per class first, based on whether real callers ever reuse/share the same instance,
then let that decision drive whether the copy ctor is deleted or implemented — don't let an MSVC
error message alone decide it:

**Before choosing, grep the class's real call sites for reuse, not just construction** — a plain
`grep "ClassName("` only finds *construction*, and misses every place a single already-constructed
instance gets handed to more than one parent. Specifically check: (1) list/tuple literals
repeating the same variable or cached constant (`[x] * n`, `[a, a, b]`); (2) any `DeferredEnum`
(or similar lazy-cached-singleton) member whose `.value` your class's constructor might receive,
since every access after the first returns the identical cached object — grep the enum's own
`_generate`/definition for where that value flows, not just the enum's call sites; (3) any
module-level constant list built once and referenced from multiple places. If none of that turns
up, unique-ownership/disown is still fine (and cheaper); if it does, clone-and-copy is required.

## A concrete derived class can sit unbound to Python for a long time after its abstract base was bound "temporarily" to unblock other work

`AGRemapCore::IniClassifier` (the derived class holding *all* the real logic — `addGIModType`,
`addWuWaModType`, `getModType`, `readHash`, `readSectionName`, the whole hash/keyword DFA-wiring
subsystem, several sessions' worth of feature work) had **no pybind11 binding at all** for a long
stretch of this porting effort — only its abstract base, `BaseIniClassifier`, was ever bound (as
`CppBaseIniClassifier`), plus the standalone `IniClassifyStats` data class (`CppIniClassifyStats`).
`BaseIniClassifier::classify()` itself is a stub — `return IniClassifyStats();`, nothing else — so
every one of `CppBaseIniClassifier`'s Python-facing methods was silently inert the whole time. This
is easy to miss because nothing *fails*: `CppBaseIniClassifier()` constructs fine, `.classify(...)`
runs fine and returns a valid (just empty) `CppIniClassifyStats`, and the class's own pybind
docstring even says outright "classify() is expected to be overridden by future C++ subclasses
(e.g. IniClassifier)" — a comment that reads as forward-looking design intent, not as "the concrete
subclass isn't reachable from Python yet." Confirmed the gap by grepping `bindings.cpp` for
`initCppIniClassifier` (nothing) and `py/src/model/strategies/iniClassifiers/` for a
`PyIniClassifier.*` file pair (didn't exist) — `git grep`/`Glob` for the derived class's own binding
file is the reliable check here, not "does `CppBase...` importing/running without error."

**General lesson**: when a subsystem uses the "wrapper-outcome-1"/`Cpp`-prefix-while-in-progress
naming convention (see the pybind11 bindings section below and
[Documentation](../Documentation/CLAUDE.md)'s naming-pitfall bullet), a bound, working, `Cpp`-named
*base* class is not evidence that the concrete class holding the actual feature work has been bound
too — check explicitly (grep the binding `.cpp` directory and `bindings.cpp`'s `initCppXxx` call
list for the derived class's own name) before assuming "the classifier is already testable from
Python," especially if asked to add tests or docs for it. The fix, once noticed, is the same shape
as any other new binding in this codebase: a `PyIniClassifier.h`/`.cpp` pair, real pybind11
inheritance (`py::class_<AGRC::IniClassifier, AGRC::BaseIniClassifier>(m, "CppIniClassifier", ...)`,
base registered first — see the inheritance bullet below), binding only the genuinely new methods
(`addGIModType`/`addWuWaModType`/`getModType`/constructor) since `classify()`/`clear()` are
inherited and dispatch to the derived override automatically via the real C++ vtable, no rebinding
needed.

**Update, 2026-09-03**: the whole pure-Python `model/strategies/iniClassifiers/` package (the
`Old`-suffixed `IniClassifierOld`/`BaseIniClassifierOld`/`IniClassifierBuilderOld`/
`BaseIniClassifierBuilderOld`/`IniClassifyStatsOld` plus the un-suffixed `states/IniCls*.py` DFA
plumbing they alone depended on, and the live `constants/GlobalIniClassifiers.py` module that still
imported them) has been **deleted outright** — there was no live call site left ([`Mod.py`](../../Anime%20Game%20Remap%20%28for%20all%20users%29/api/src/py/FixRaidenBoss2/model/Mod.py)
constructs `IniFile` with no `iniClassifier` argument, which already defaults to the C++
`GlobalIniClassifiers::classifier()` singleton). With the pure-Python originals gone, the three
`Cpp`-prefixed names in this section have **graduated to their bare names** per the "Two different
outcomes for porting a class" rule below: `CppBaseIniClassifier` → `BaseIniClassifier`,
`CppIniClassifier` → `IniClassifier`, `CppIniClassifyStats` → `IniClassifyStats` (`initCppXxx` the
internal C++ function names are unchanged — only the `py::class_<...>(m, "...")` registration
string moved). Everything else in this section above is otherwise-accurate historical narrative of
how the binding gap was diagnosed and fixed — just mentally read every `CppXxx` name in it as the
bare name now. `test_CppIniClassifier.py` was likewise renamed to `test_IniClassifier.py`
(replacing the old, permanently-broken pure-Python test of that same name).

## A `std::optional<T>`/return-or-parameter-type dependency between two `initCppXxx(m)` calls does *not* need call-order enforcement — only real `py::class_<Derived, Base>` inheritance does

Empirically confirmed while adding `ModTypeIdTools::getModType`/`registerModType` (returning/taking
`ModType`, i.e. `CppModType`): binding `initCppModTypeId(m)` (which registers these methods)
*before* `initCppModType(m)` (which registers the `CppModType` type itself) still works correctly
at runtime — Python calls to `getModType`/`registerModType` succeed and return real `CppModType`
instances, with zero reordering needed. This makes sense once you separate *bind time* from *call
time*: `.def_static("getModType", &Fn, ...)` only needs `CppModType` to be a valid, at-least-
forward-declared C++ type at the point `.def_static` is compiled (a C++-level, compile-time
concern) — the actual Python-facing type_caster lookup for the return value happens lazily, via
`typeid`, the moment a Python caller actually invokes the function, which is always after every
`initCppXxx(m)` in the module has already run. Contrast this with **real pybind11 inheritance**
(`py::class_<Derived, Base>(m, "Derived", ...)`), which genuinely does need `Base`'s `py::class_<Base>`
registration to have already executed *at the point the `py::class_<Derived, Base>` template
instantiates* (see the existing inheritance bullet in this section) — that's a hard, bind-time
requirement, not a lazy one. **Practical effect**: don't add a defensive "must come after" comment
or reorder `initCppXxx` calls in `bindings.cpp` just because one function's signature mentions a
type registered elsewhere — reserve that discipline for genuine `py::class_<Derived, Base>`
relationships (where it's actually required) and for default-argument values needing
`py::cast`-at-bind-time (the real, different reason `pybind11/stl.h` must be included — see the
next bullet), not for ordinary parameter/return types.

## `std::optional<T>` in a `.def_static(...)`/`.def(...)` signature needs `#include <pybind11/stl.h>`, and its absence fails at import time, not compile time

Adding a `std::vector<std::string> aliases = {}` default-valued parameter to a pybind11 binding
compiled and linked cleanly, but the very first `import` of the rebuilt module raised
`ImportError: arg(): could not convert default argument into a Python object (type not registered
yet?)`. The cause: `py::arg("aliases") = std::vector<std::string>{}` needs pybind11 to `py::cast`
that empty vector into a real Python object *at module-init time*, which requires the
`type_caster<std::vector<std::string>>` specialization from `<pybind11/stl.h>` — a header the
binding file (`PyModType.h`) hadn't included, since nothing about a plain `std::string`/`int`
parameter had needed it before. This is the one case from the bullet above where signature-level
type support genuinely *is* a bind-time (not lazy call-time) requirement — a default argument's
value must be materialized into a `py::object` immediately, unlike a plain parameter/return type
declaration. **Fix**: `#include <pybind11/stl.h>` in the binding header whenever a `std::vector`/
`std::optional`/`std::unordered_map`/etc. appears as a *default-valued* argument (return-only or
required-argument uses of the same containers had already worked fine elsewhere in this codebase
without it, e.g. `ModTypeIdTools::getModType`'s `std::optional<ModType>` return type) — and verify
by actually importing the rebuilt module (see [Building](../Building/CLAUDE.md)'s PowerShell-vs-
Bash note), not just by a clean compile/link, since this specific failure mode is invisible to the
build step entirely.

## `py::arg("x") = <a mutable object>` is pybind11's version of Python's mutable-default-argument bug

A default argument is `py::cast`ed into a Python object **once, at module-init time**, and that one
object is then handed to every call that omits the argument. If it is mutable, a caller that mutates
what it was given has silently changed the default for the rest of the process:

```cpp
// WRONG -- one shared IniRemovalContext for the life of the module.
.def("remove", ..., py::arg("context") = AGRC::IniRemovalContext())

// WRONG -- same problem, and the shape this most often takes.
.def(py::init<...>(), py::arg("aliases") = std::vector<std::string>{})
```

The fix is the same one Python itself needs: **default to `py::none()` and construct a fresh one
inside the lambda / `py::init` factory.**

```cpp
.def("remove", [](T &self, ..., py::object context) {
    AGRC::IniRemovalContext removalContext;             // fresh, per call
    if (!context.is_none()) {
        removalContext = context.cast<AGRC::IniRemovalContext>();
    }
    return self.remove(..., removalContext);
}, ..., py::arg("context") = py::none())
```

This has now bitten in at least three separate subsystems (`iniresources`, `IniGraphGroup`,
`iniRemovers`), and it is invisible to every automated check the repo has --- it compiles, it
imports, and it passes any test that only ever calls with the default. **Write a test that mutates
the object it passed and then calls with the default again**; that is the only thing that catches
it. (Note the taking-it-as-`py::object` form also sidesteps needing `<pybind11/stl.h>` in that
translation unit, which is the *other* default-argument trap, documented just above.)

## `std::filesystem::absolute("")` throws on MSVC, so an empty folder path is not a harmless "use the working directory"

`FileService::absPathOfRelPath` and anything else that eventually reaches `std::filesystem::absolute`
must never be handed an empty string. On MSVC that throws a `std::filesystem::filesystem_error`
rather than resolving to the current directory --- and since the throw usually escapes a method
nobody wrapped in a `try`, the process `std::terminate`s and the run dies with **exit code
`0xC0000409` and no output at all**, which reads like a memory-corruption crash rather than a bad
argument.

The empty string is easy to reach without noticing: an `IniFile` with no path has an empty folder,
and so does one whose path is a bare relative file name. Substitute `"."` --- the same working
directory, spelled in a way `absolute()` accepts:

```cpp
std::string iniFolder = ctx_->iniFolder();
if (iniFolder.empty()) {
    iniFolder = ".";
}
```

If you are staring at a silent `0xC0000409` from a standalone test, this is worth ruling out before
anything else --- alongside the `setvbuf` note in [Building](../Building/CLAUDE.md), since a
buffered stdout is what hides the progress markers that would otherwise tell you where it died.

## A static class data member of a non-copyable, non-move-constructible type can't be initialized via `Type Class::member = factoryFunction();`, even for a directly-returned prvalue

Tried to give `ModTypeIdTools::_nameDFA` (a `BaseAhoCorasickDFA<std::unordered_set<int>>` static
member) its required one-time `setHandleDuplicate(...)` setup — matching what `IniClassifier`'s own
constructor does for its instance-level `sectionKeywordsDFA` — via a small factory function
returning the configured DFA by value: `BaseAhoCorasickDFA<...> ModTypeIdTools::_nameDFA =
makeNameDFA();`. This fails to compile (MSVC `C2280`, "attempting to reference a deleted function"):
`BaseAhoCorasickDFA`/its base `BaseTrie` holds a `std::unique_ptr<BaseIdGenerator<uint64_t>>`
member, which deletes the implicit copy constructor — and since `makeNameDFA()`'s `return dfa;`
returns a *named* local (NRVO territory, not a directly-constructed prvalue), C++17's *guaranteed*
copy elision (which only applies to prvalues, not named-return-value cases) doesn't kick in; the
compiler falls back to move-or-copy, finds no viable move (implicitly suppressed alongside the
deleted copy), and hard-errors on the deleted copy ctor. This is a real trap for any "static/global
member of a type from `tools/tries/`, `tools/dfa/`, or `tools/orderedMultiMap/`" (any type built
around an owning `unique_ptr`) needing one-time configuration outside a real constructor body — a
static-method-only "Tools" class (`ModTypeIdTools`-style) has no constructor to put this setup in.

**Fix**: never construct-and-return the value by value; default-construct the static member in
place (no initializer needed) and mutate it *in place* via a private static setup method, triggered
exactly once through a second static `bool` member's own initializer:
```cpp
// header
static BaseAhoCorasickDFA<std::unordered_set<int>> _nameDFA;
static bool _setupNameDFA();      // mutates _nameDFA in place, returns true
static bool _nameDFAInitialized;  // its initializer runs _setupNameDFA() once

// .cpp -- declaration order within one translation unit is well-defined (unlike cross-TU order),
// so _nameDFA is guaranteed constructed before _nameDFAInitialized's initializer runs
BaseAhoCorasickDFA<std::unordered_set<int>> ModTypeIdTools::_nameDFA;
bool ModTypeIdTools::_setupNameDFA() { _nameDFA.setHandleDuplicate(...); return true; }
bool ModTypeIdTools::_nameDFAInitialized = ModTypeIdTools::_setupNameDFA();
```
See `ModTypeId.h`/`.cpp`'s real `_nameDFA`/`_setupNameDFA`/`_nameDFAInitialized` for the working
version. If you're tempted to reach for a factory-function-returning-by-value for *any* static/
global of a `unique_ptr`-owning type in this codebase, this is why it won't compile, and this
in-place-mutation-behind-a-second-static-bool shape is the established fix.

## The strategy **context seam**: how a C++ strategy reaches a `.ini` file it isn't allowed to know about

This is the load-bearing architectural pattern in `model/strategies/`, and you will meet it the
moment you touch a parser, fixer, remover or resource edit. Read this before designing anything
there.

A ported strategy (`GIMIParser`, `GIMIFixer`, `RemapIniRemover`, `ResEdit`) needs a `.ini` file ---
its text, its `sections`_, where it lives on disk. But `AGRemapCore` must stay Python-free, and the
*live* `.ini` file is still the pure-Python `IniFile`. So no strategy takes an
`AGRemapCore::IniFile*`. Each family instead defines a pure-virtual **context interface**, and the
strategy is written against that:

| Family | Seam interface | Core implementation | pybind11 implementation |
| --- | --- | --- | --- |
| `iniParsers/` | `IniParseContext` | `IniFileParseContext` | `PyIniParseContext` |
| `iniFixers/` | `IniFixContext` | `IniFileFixContext` | `PyIniFixContext` |
| `iniRemovers/` | `IniRemoveContext` | `IniFileRemoveContext` | `PyIniRemoveContext` |
| `graphGroupEdits/resEdits/` | `IniResEditContext` | `IniFileResEditContext` | `PyIniResEditContext` |

**Every seam needs both implementations, and they are siblings --- neither derives from the other.**
The `Py*` one forwards through genuine Python attribute lookup on the caller's `IniFile`; the
`IniFile*` one is a thin adapter over `AGRemapCore::IniFile*`. If you add a method to a seam, you
have written half the change until both sides implement it, and the compiler only tells you about
the C++ half.

Three rules that keep falling out of this:

- **A `Py*` context must forward, not reimplement.** The unit-test harness patches
  `builtins.open`, `os.path` and `FileService.read` at the *Python* level, so a `std::filesystem`
  call inside a `Py*` context silently bypasses every one of those mocks. `PyIniFixContext`
  deliberately calls Python's own `os.path.exists` for exactly this reason.
- **The core implementation derives what it can rather than demanding new `IniFile` API.** There is
  no `FilePath` class in `AGRemapCore` and no `folder` accessor; `IniFileRemoveContext::iniFolder()`
  is just `std::filesystem::path(*iniFile_->getFile()).parent_path()`. Copy that instinct before
  growing `IniFile`.
- **Something must own what the seam hands out.** `IniParseContext::graphGroups()` and
  `IniFixContext::makeGraphGroups()` exist *because* an `IniGraphGroupsVec` is only a **view** over
  a caller-owned vector --- the context owns the storage. Likewise
  `IniResEditContext::takeCollectedResources` returns raw `IniResource*` while promising the models
  stay alive, so both implementations keep a separate keep-alive store (`captureKeepAlive_` / a
  `py::list`) that `take` never touches.

### `XxxContext` vs `XxxingContext`: two different things wearing near-identical names

`IniRemoveContext` is the seam above. **`IniRemovalContext` is not** --- it is a plain struct of
*per-call options*, passed by value to `remove()`, that knows nothing about any file. The fixer side
mirrors it exactly: `IniFixContext` is the seam, **`IniFixingContext`** is the per-call options
(`isFirstModType` / `isLastModType`). Both option structs are deliberately non-templates, so the
pybind layer binds each once and hands the same type to every instantiation. When adding a per-call
knob, put it there, not on the seam.

Those two flags exist because **`AGRemapCore::IniFile::fix()` runs several fixers over one file** ---
one per mod type it was classified as, and one per target mod each of those fixes to. Anything that
rewrites the *file* rather than only adding to the fix must happen exactly once: the first mod type
takes the backup (`keepBackup`), the last one hides the original mod's `sections`_ (`hideOrig`).
`IniFile::fix` resolves both before its loop, and deliberately only among mod types that actually
contribute a fixer --- a mod type that runs nothing must not be able to claim the file's first or
last word.

### State of the core-only pipeline: parse, fix and remove are all live now

Historical note, because the old text here said the opposite and you may find that claim quoted
elsewhere: `IniFile::parse()` and `fix()` used to return nothing for a plain C++ caller. Both were
fixed on 2026-09-02, and there were **three** blockers rather than the single one documented:

1. `IniParseBuilder::defaultFactory`/`IniFixBuilder::defaultFactory` returned do-nothing bases.
   They now build a real `GIMIParser`/`GIMIFixer` that owns its context.
2. A `GIMIFixer` with no `sectionToStr` renders **empty** -- it builds its groups correctly and
   writes nothing. The renderer is `AGRemapCore::renderIfTemplate`
   (`model/iftemplate/IfTemplateRender.h`), handed in as a callback. `IfTemplate`/`IfContentPart`
   still deliberately have no `toStr` of their own; that half of the old warning stands -- do not
   add one.
3. The owning parser must call `setIniFile(iniFile)`. `BaseIniFixer::setParser` derives its `.ini`
   file from `parser->getIniFile()`, so a parser that never set it silently blinds every fixer
   downstream. Caught only by a core standalone test; the Python suite stayed green throughout.

What is *still* inert is the per-mod-type **argument layer**: the default factory passes
`graphGroupEdits = {}`, so a remapped section body comes out empty. That is the
`IniFixBuilderData`/`IniParseBuilderData` tables, tracked separately.

## Splicing a Python-state-carrying base into a ported class: the `XxxBase` template parameter

When you port a class whose *base* also needs Python-side state, give the derived class a trailing
template parameter for its own base:

```cpp
template <typename K, typename V, typename KeyHash, typename KeyEqual,
          typename FixerBase = BaseIniFixer<K, V, KeyHash, KeyEqual>>
class GIMIFixer: public FixerBase { ... };
```

The pybind layer then instantiates it as `GIMIFixer<py::object, ..., PyBaseIniFixer>`, splicing its
own subclass (which carries `_parser`/`_iniFile`/`_modsToFix` as real `py::object`s) between the
concrete class and the core base. That makes `py::class_<PyGIMIFixer, PyBaseIniFixer>` genuine C++
inheritance with no virtual bases. `GIMIParser`/`ParserBase` uses the identical shape.

One trap it creates: those spliced bases take a `py::object` in their constructor, so forwarding
`nullptr` through the base's constructor turns into a *null* `py::object` rather than an unbound
pointer. Construct with the base's default constructor and call the setter in the body instead ---
`GIMIFixer` does `Base()` then `this->setParser(parser)`.

## `cond ? py::none() : someObject` collapses to `py::none` and throws at runtime

A real pybind11 trap, and its error message points nowhere near the cause:

```cpp
// WRONG -- the common type of the two branches deduces to py::none
py::object hashes = modType.is_none() ? py::none() : modType.attr("hashes");
// TypeError: Object of type 'Hashes' is not an instance of 'none'
```

`py::none`'s converting constructor rejects a real object, so this compiles and fails at runtime.
Wrap **both** branches:

```cpp
py::object hashes = modType.is_none() ? py::object(py::none()) : py::object(modType.attr("hashes"));
```

## `IniFile::getIfTemplates()` is a `tsl::ordered_map`, and that is load-bearing

Not an `std::unordered_map`. `IniParseContext::sectionNames()` documents declaration order as
required --- the pure-Python original gets it free from `ini.sectionIfTemplates` being a `dict`, and
a parser both classifies `sections`_ by name and seeds `GIMIParser::buildGlobalGraph`'s target list
from that order, which the rendered output then inherits. `IniFile::addSection` appends a new name to
that order, and `removeSection` uses `tsl::ordered_map::erase` (order-preserving) rather than
`unordered_erase` (swap-with-last). Don't "optimize" either back.

## The `ModAssets` / `ModDictAssets` / `ModMappedAssets` family --- picking the right one, and what `get` can and cannot do

Every version-keyed lookup table in `core/` (`Hashes`, `Indices`, `VertexCounts`, `VGRemaps`, the
three `Ini*BuilderData` tables) is one of these three. Pick by two questions, in this order:

1. **How many version columns?** One --- `ModDictAssets` (hash on the full non-version key, then a
   binary search over that key's versions). Two or more --- **`ModAssets`**, which is a linear scan;
   `ModDictAssets` simply cannot express a second version column. `VGRemaps`
   (`fromVersion`/`toVersion`) and `IniFixBuilderData` (same) are the two that need it.
2. **Does it need a fix-from -> fix-to adjacency list** (`getMap`/`hasFrom`)? If yes, `ModMappedAssets`,
   which wraps a `ModDictAssets`. `Hashes`/`Indices` use it; `VertexCounts` does not (a count is
   looked up, never remapped), which is why it derives from `ModDictAssets` directly.

**`get` returns a leaf value, never a subtree.** This surprises people, so state it plainly:

- `ModDictAssets::get` **requires the complete non-version key** --- it throws
  `std::invalid_argument` on a short vector rather than treating it as a prefix, and its element
  type is a plain `K`, so there is no way to say "any" at a position. A prefix query isn't merely
  unimplemented, it's inexpressible: `groups_` is hashed on the *whole* key tuple, so nothing can
  descend a level. The nesting was flattened at construction.
- `ModAssets::get` **does** accept `std::nullopt` per non-version column as a wildcard --- but it
  still returns a single `std::optional<T>`. Wildcards there narrow to one hit; they don't collect.
- For multiple results use **`ModAssets::getAll`**, which returns one entry per distinct combination
  of the `std::nullopt` columns. Its load-bearing property: **version resolution runs per group, not
  once globally.** A single global resolve would pick one winning version and silently drop every
  group with no row at it (`Jean -> JeanSea` has no 5.5 row while `Jean -> JeanCN` does). `get` and
  `getAll` share `resolveVersionColumns` precisely so the two can't drift.
- To sweep a table without a query, `ModDictAssets::forEachEntry` visits every row. That's O(rows),
  but so would any prefix query be without adding a second index.

**Floor-matching does not "miss" below a key's earliest version.** Asking for a version *older* than
every row for that key returns the **oldest** row, not `std::nullopt`. A mod whose only row is 4.6
still answers at 4.0. Assertions written expecting a miss there will fail --- this cost a red test.

**Binding a pre-populated table whose value type isn't `std::string`: it cannot subclass the bound
base, so bind it standalone.** `Hashes`/`Indices` get `py::class_<PyHashes, PyModMappedAssets>`
because they really are `ModMappedAssets<std::string, std::string>`, the same instantiation that is
registered. `VertexCounts` (`ModDictAssets<std::string, int>`) and `VGRemaps`
(`ModAssets<std::string, VGRemap>`) are **different C++ types** from the registered
`ModDictAssets<std::string, std::string>` / `CppModAssets` (`ModAssets<std::string, py::object>`) ---
not derived from them --- so naming one as a `py::class_` base doesn't compile. Bind them as their
own `py::class_` and re-declare the handful of methods (`get`/`addRows`/`__len__`/the count
properties). Three things that shape those bindings, all learned the hard way:

- **`get`'s "unspecified column" means different things per table, and both are right.** For a
  `ModDictAssets`-backed one (`VertexCounts`) an unspecified non-version column must be filled in
  with `""`, *not* wildcarded --- the table is hashed on the whole key tuple and has no wildcard to
  give (see `get`'s bullet above), and `""` is what every shipped row carries and what C++
  `ModType::getVertexCount` passes. For a `ModAssets`-backed one (`VGRemaps`) `std::nullopt` really
  is a wildcard. `toWildcardList` (`PyModDictAssets.h`) produces the `optional` list for both; only
  what you do with a `nullopt` differs.
- **Accept both a bound value object and the plain dict it is built from** for a leaf. `VGRemaps`
  rows are `VGRemap`s, but real callers (and this repo's own tests) pass `{0: 7, 1: 6}` --- a dict is
  *not* implicitly convertible across the boundary, so the binding has to branch on it explicitly.
- **Bind `clone`/`__copy__`/`__deepcopy__`.** `ModDataAssets.VGRemaps` hands out a *shared*
  instance, so real call sites deep-copy before mutating (`test_Mod.py` does), and a fresh
  `py::class_` supports neither until bound --- see this file's own note on that gap.

## Generating a C++ data table from live Python: always script it, then diff it

`core/src/data/*.cpp` tables are mechanically generated by importing the *real* Python module and
walking it --- never hand-transcribed --- and then verified by dumping both sides and set-diffing.
Follow that for any new one; the existing headers' `@danger` blocks say so for a reason. Two
practical notes:

- Row keys use `ModTypeIdTools::getName(ModTypeId::X)`, not string literals, so a registry rename
  can't silently desync a table. Note `ModTypeId::AyakaSpringbloom` spells the name
  `"AyakaSpringBloom"` --- capital B. Don't hand-write these; map them from the enum.
- **Some tables have now deliberately diverged from their still-live Python originals** (the C++
  `VertexCountData` has a `component` column the Python dict lacks; `IniFixBuilderData` is 4-column
  in C++ and 2-column in Python). Both copies exist and are live. A naive regeneration from Python
  would silently *undo* the C++ shape --- the headers carry `@danger` notes saying so, and the
  Python files carry a matching comment. Read them before regenerating anything.

## `ModType` facts that are easy to get wrong

- **Constructor order is `gameTypeId, modTypeId, name, aliases, hashes, indices, vertexCounts,
  vgRemaps, iniParseBuilder, iniFixBuilder, iniRemoveBuilder`.** Every caller passes positionally,
  so inserting a parameter mid-list breaks `GIBuilder` plus 4-5 test files at once. Expect that and
  budget for it; appending is cheaper when the semantics allow.
- **Don't subclass `ModType`.** It is held **by value** in five places --- `ModTypeIdTools::_modTypes`
  / `getModType` / `registerModType`, and `IniFile::modTypes` / `overrideModTypes_` / `getModType` ---
  plus all 43 `GIBuilder` factories return it by value. Any subclass gets **sliced** and its extra
  members silently vanish. A `GIModType : ModType` experiment was built and then removed for exactly
  this. Subclassing only becomes viable once those slots hold `shared_ptr<ModType>`.
- **The asset members' defaults are not uniform.** `hashes`/`indices`/`vertexCounts` each get a
  *fresh* fully-populated table when not supplied; `vgRemaps` falls back to the **shared**
  `ModDataAssets::vgRemaps()`. That asymmetry is upstream (the pure-Python `ModType` defaults to
  `ModDataAssets.VGRemaps.value`), and it means mutating a defaulted `vgRemaps` affects every other
  `ModType` that also defaulted. Same story for `iniRemoveBuilder` and
  `GlobalIniRemoveBuilders::removeBuilder()`.
- **`ModMappedAssets::fixTo` is declared but never populated** anywhere in the Python package --- the
  pybind binding deliberately returns an empty set. So `ModType.getModsToFix()` yields nothing, and
  it is *not* a usable source of from -> to mod relationships. **`VGRemapData` is** --- it is keyed
  `(fromVersion, fromChar, fromComp, toVersion, toChar, toComp)` and is where `Jean -> {JeanCN,
  JeanSea}` actually lives.

## `IniFile` is the C++ class now --- `IniFile.py` is gone

As of 2026-09-03 `FixRaidenBoss2.IniFile` **is** `AGRemapCore::IniFile` via pybind11. The
2525-line pure-Python `model/files/IniFile.py` was deleted and the old `CppIniFile` name retired.
What follows from that, in rough order of how easy it is to trip over:

- **It takes mod-type *ids*, not `ModType` objects**, resolved through the global registry
  (`ModTypeIdTools`), so `GlobalModTypes::registerAll()` must have run. The pure-Python `ModType`
  carries no id of its own --- bridge by name:
  `int(ModTypeIdTools.findByName(modType.name))`. That is what `Mod._modTypeIds` does.
- **Several constructor keywords have no C++ equivalent.** `logger` is simply gone --- logging is
  the view's job, and the view is C++ now too (see "The view is C++ now" below); nothing in
  `AGRemapCore` takes a logger yet. `version` split into `fromVersion`/`toVersion`. `modTypes`/`forcedModType`
  became `filteredFromModTypeIds`/`forcedFromModTypeIds`, `modsToFix` became
  `filteredToModTypeIds`, `defaultModType` became `defaultModTypeId`. The last two have **no
  constructor parameter at all** --- assign them after construction.
- **~33 methods did not survive, deliberately.** The fix-boilerplate family
  (`getFixHeader`/`getFixFooter`/`getFixCredit`/`addFixBoilerPlate`) moved into
  `RemapIniFixContext`/`GIMIFixer`; the section-options family
  (`getSectionOptions`/`removeSectionOptions`) into `RemapIniRemover`. Wanting one of them on
  `IniFile` means you are in the wrong class.
- **There are no per-kind resource accessors** (`getTexAddModels`, `getTexEditModels`, ...) and
  there should not be. `getResources()` is generic and you filter on `IniResource.type` --- the
  maintainer's explicit call: a resource is a resource whether it is a texture, a buffer or any
  other media. Current vocabulary: `resourceRemapBlend`, `resourceRemapTexAdd`,
  `resourceRemapTexEdit`.

## Adding a resource type: the base is add-vs-edit, and it is unreachable until a `resEdits/` class builds it

Two rules, both learned by getting them wrong.

**1. The base class is decided by whether the operation writes a *different* file.**

| Operation | Base | Paths | Examples |
| --- | --- | --- | --- |
| add / create | `RemapIniResource` | `srcPath` only | `RemapTexAddResource` |
| edit / replace | `RemapIniFixResource` | `srcPath` **and** `fixedPath` | `RemapBlendResource`, `RemapTexEditResource` |

Media type is irrelevant to the choice --- a texture *edit* belongs with the blend edit, not with
the texture add. Getting it right settles the details for free: `src*` predicates key on
`srcPath` and `fix*` on `fixedPath`, and `fixExists` is *inherited* from `RemapIniFixResource`
("is `fixedPath` on disk") rather than overridden.

**2. Binding the resource does nothing on its own.** What attaches it to an `.ini` file lives under
`core/.../iniFixers/graphGroupEdits/resEdits/`, and the split there mirrors rule 1: `ResCreate` for
an add, `ResReplace` for an edit. The difference is literally whether the **original** resource
name is used --- `TexCreate::getFixResourceName` discards its `resource` argument,
`TexReplace::getFixResourceName` builds on it. Beware that filenames there do not name their
classes: `resEdits/TexEdit.h` holds **`TexCreate`** (and now `TexReplace`). Grep for the class, not
the file.

Like `RemapBlendReplace`, a core `resEdits` class does **not** override `buildResModel` --- the
pybind layer does, because the editor/creator object comes from Python. And every resEdit pybind
class registers against `PyBaseResEditCore`, so `isinstance(x, ResReplace)` is `False` even for
`RemapBlendReplace`; only `BaseResEdit` is a real Python base. Consistent across the family --- do
not "fix" it.

## A Python object handed back through a bound factory loses its identity unless you pin it

If a binding takes a Python callable that *returns an object* (`IniParseBuilder`'s factory, and
anything shaped like it), `result.cast<std::shared_ptr<PyThing>>()` is a trap. That `shared_ptr`
owns only the **C++** half; the `PyObject` is freed as soon as the caller's last reference drops
--- immediately, since the factory's local goes out of scope. Casting it back later builds a
**brand new wrapper of the registered base type**: a Python subclass goes in and a plain base
object comes out, attributes gone, silently, with no error.

Use `holdPyStrategy<PyThing, CoreThing>` (`py/src/model/strategies/PyStrategyFactory.h`). It uses
`shared_ptr`'s *aliasing* constructor so the control block owns a `PyStrategyKeepAlive` pinning the
`py::object`, whose destructor re-acquires the GIL --- the last reference is usually released from
C++ with none held. The regression test is `assertIs(built, made)`; `assertIsInstance` passes
either way and will not catch it.

Related: a builder must capture its Python factory **by value**, because a builder outlives the
expression that made it (`ModType` holds one in a `shared_ptr`).

## Register the core template base, or the boundary can only ever return `None`

The Python-facing wrappers (`PyBaseIniParser`, `PyBaseIniFixer`, `PyBaseIniRemover`) are
*subclasses* of `AGRC::BaseIniParser<>` and friends. For a long time only the subclass was
registered, so a `shared_ptr<BaseIniParser<>>` coming from a **C++-side** factory had no registered
type to cast to, and the workaround was `dynamic_pointer_cast` plus `None`.

The fix, now in place: register the core base too (`CppBaseIniParser`, `CppBaseIniFixer`,
`CppBaseIniRemover`) and declare the wrapper as
`py::class_<PyBaseIniParser, PyBaseIniParserCore, py::smart_holder>`. pybind11's polymorphic
downcast then returns the *Python* object when there is one and the core base otherwise. Every
class in such a hierarchy needs `py::smart_holder`, since a holder must stay consistent down a
chain.

## `# TOREMOVE` in `FixRaidenBoss2/__init__.py` is the authoritative deletion signal

When a task says "remove the old X", read those markers before deciding scope. They beat the
`...Old` suffix as a signal, because some doomed classes keep their bare name (`GIMIObjParser`,
`GIMIObjMergeFixer`) while some `...Old` files are deliberately kept. Two counter-signals that mean
**keep**:

- a class bound under a `Cpp`-prefixed name --- that is the wrapper outcome, and the bare-named
  pure-Python original stays. (The day the Python one *is* deleted, the binding takes the bare
  name; that is exactly how `CppIniFile` became `IniFile`. Watch out when renaming:
  `initCppIniFile` **contains** `CppIniFile`, so a naive find-and-replace silently breaks the
  module's init function.)
- a core class with **no pybind binding at all** --- the pure-Python one is still the only thing
  Python can reach, however complete the C++ side looks.

## The view is C++ now: `BaseLogger` / `Logger` (`core/.../view/`, bound as `BaseLogger` / `Logger`)

The pure-Python `view/Logger.py` was **deleted outright** on 2026-09-03 (matching the `IniFile` port,
not the older `...Old` rename convention) and replaced by an abstract `AGRemapCore::BaseLogger` plus
two concrete sinks. The split is the point of the port, so know it before touching either:

- **`BaseLogger` owns every bit of formatting and bookkeeping** (prefix, heading stack, `.txt`
  transcript, `verbose`/`logTxt`/`includePrefix`) and funnels each rendered line through two pure
  virtuals, `write(message)` and `read(desc)`. Every higher-level method (`log`, `openHeading`,
  `error`, `list`, `input`, ...) is *also* virtual, so a future GUI/backend view can override at the
  structured level and never see the text. `handleException` is deliberately **not** virtual --- it
  is a formatter that ends in `error()`, and keeping it non-virtual avoids the two-overload arity trap
  a Python override would hit.
- **The Python-facing `Logger` is *not* a binding of `AGRemapCore::Logger`.** The core `Logger` is
  the `std::ostream`/`std::istream` console view for a plain C++ consumer. The bound `Logger` is
  `PyLogger` (`py/src/view/PyLogger.h`), whose `write`/`read` call `builtins.print`/`builtins.input`
  looked up *per call* --- the same "a `Py*` seam forwards to Python, it never reimplements" rule as
  the strategy contexts. Two reasons, both real: `std::cout` and `sys.stdout` are separately
  buffered and interleave out of order, and the unit tester captures output by patching
  `builtins.print`/`builtins.input` (`py::print` would bypass that --- it writes to `sys.stdout`
  directly).
- **One trampoline template serves both bound classes**: `PyBindLoggerT<LoggerBase>`
  (`PyBaseLogger.h`), instantiated for `AGRemapCore::BaseLogger` (abstract) and `PyLogger`
  (concrete). `std::is_abstract_v<LoggerBase>` decides whether a missing Python override of
  `write`/`read` raises or falls through to the base. Both registrations use `py::smart_holder`, and
  the derived one is `py::class_<PyLogger, AGRC::BaseLogger, PyBindLogger, py::smart_holder>`.
  `super().log(...)` inside a Python override works because pybind11's `get_override` detects the
  call is coming from the override's own frame and falls through to the C++ base --- no special
  handling needed, but `test_BaseLogger.py` pins it.
- **`BaseLogger.headings` is a list of `(title, sideLen, headingChar)` tuples, not `Heading`
  objects.** `AGRemapCore::Heading` has no pybind binding: the pure-Python `tools/Heading.py` still
  has three live users outside the logger (`IniConsts`, `ModTypes`, `ModType`), so binding it is a
  separate port with its own call-site sweep. If that lands, this property is the one place to revisit.
- **`Heading::close()` now counts graphemes (`StringTools::countGrapheme`), not bytes** --- a
  non-ASCII title from Python would otherwise render a closing line wider than its opening one.
  This is deliberately *stricter* than the Python original's `len()` (code points): a combining
  mark or emoji ZWJ sequence counts as one column. It also means `Heading.cpp` now pulls in the
  `StringTools`/grapheme/utf8proc cone, so a standalone test touching it needs those sources.
- **`py::arg` names on the bound view are load-bearing.** `Model.print(funcName, *args, **kwargs)`
  reaches the logger by `getattr` and forwards keyword arguments by name (`self.logger.openHeading(
  "Summary", sideLen = 10)` in `remapService.py`), so a binding that renames a parameter breaks call
  sites the C++ compiler never sees. Keep the exact Python parameter names (`txt`, `sideLen`,
  `headingChar`, `lst`, `transform`, `desc`, ...) when porting anything `Model.print` can reach.
- `handleException` is bound twice on purpose: `(exception)` builds the message in Python
  (`type(e).__name__`, `str(e)`, `traceback.format_exc()` looked up per call so `mock.patch` works)
  and `(exceptionType, message, traceback = "")` is the C++-shaped overload for errors that arrive
  without a live exception object. pybind11 resolves them by arity.
- The core console view has a standalone `core/tests/Logger_test.cpp` (needs only three core `.cpp`
  files plus the `StringTools`/grapheme/`utf8proc` cone since `Heading::close` counts graphemes; the
  recipe is in its header); everything reachable from Python is in `test_Logger.py`/`test_BaseLogger.py`.

## Text handling in core is grapheme-aware --- reach for `StringTools`, don't re-add ASCII helpers

As of 2026-09-03, every byte-wise text helper that had accumulated file-locally in the core
(`toLowerAscii`/`stripAscii`/`lstripAscii`/`asciiIEquals`/`trim` in `IfPredPart.cpp`,
`IfPredPartType.cpp`, `IniFile.cpp`, `IniClassifier.cpp`, `ModType.cpp`, `IniNamingTools.cpp`,
`GIMIParser.tpp`, `GIMIFixer.tpp`, `RemapIniRemover.tpp`, `Version.cpp`) was deleted and replaced
by `AGRemapCore::StringTools` (`tools/StringTools.h`), which is utf8proc-backed and works per
grapheme: `strip`/`lstrip`/`rstrip`/`isSpace` (Python's `str.isspace` rule, so NBSP, U+3000 and
U+2028 count as whitespace), `toLower` (per-codepoint simple case mapping, the same trade-off
`TextTools::capitalize` documents), `firstGraphemes`/`lastGraphemes`, `startsWith`/`endsWith`
(never match part of a grapheme), `equalsIgnoreCase`/`endsWithIgnoreCase`, and `countGrapheme`
(now a `GraphemeRange` walk, so it always agrees with the iterator and no longer throws on
malformed UTF-8 --- a stray byte is a 1-byte grapheme everywhere, including in `GraphemeIterator`
itself, which used to add utf8proc's negative error length to its cursor and walk backwards).
The maintainer's stated requirement is grapheme semantics for emojis and special characters
throughout the C++ side. Conventions that came out of that pass:

- **Don't write `std::isspace`/`std::tolower`/`std::transform(..., tolower)`/`for (char c : ...)`
  over user text.** Each of the removed helpers carried a "names are ASCII in every real mod"
  justification; that argument has been rejected. `<cctype>` should not be needed under `model/`
  any more (only `Version.cpp`'s ASCII grammar scan still uses it, deliberately).
- **Byte-wise `find`/`rfind`/`substr`/`starts_with` against ASCII *delimiters* (`[`, `]`, `=`,
  `;`, `#`, `\n`, `\r`) are fine and were left alone** --- UTF-8 is self-synchronising, so those
  give the same result as a grapheme walk. The line to draw: anything involving whitespace, case,
  or an index that gets handed back to a caller goes through `StringTools`/`GraphemeRange`.
- **Indices this library hands out are grapheme indices** (`BaseAhoCorasickDFA::find*`'s
  `resultInd`, `Token::charNo`). When a loop needs both, keep a byte cursor and a grapheme cursor
  as two separate variables --- `findMaximal(txt, count)` used one counter for both and was
  silently wrong for any non-ASCII text until this pass.
- **Lowering can change a character's byte length** (`İ` is 2 bytes, its lowercase `i` is 1), so
  never splice `keyword.size()` bytes out of an original string after comparing a lowered copy.
  Take the span with `firstGraphemes`/`lastGraphemes` on the *original*, compare that span
  lowered, and splice the span --- see `IfPredPart.cpp`'s `stripLeadingKeyword`/
  `stripTrailingKeyword` and `IniNamingTools.cpp`'s `rfindCaseInsensitive` for the pattern.
- **A pybind11 binding that needs a single ASCII byte from a Python `str`** (`PyBaseTokenizer`'s
  `addASCIIRangeTransitions`) should check `countGrapheme(value) == 1` first, so a user passing
  `"é"` is told "must be a single ASCII character" rather than "must be a single character".
- Coverage is the hand-built `core/tests/StringTools_grapheme_test.cpp` (recipe in its header;
  only utf8proc needed, no z3). The Python suite reaches these helpers only indirectly, and the
  three test classes that would exercise the parser/fixer/remover changes (`GIMIParserTest`,
  `GIMIFixerTest`, `RemapIniRemoverTest`) are among the eight still failing in `setUpClass` on the
  `baseIniFileTest.py` fixture (see [Testing](../Testing/CLAUDE.md)).
- Deliberately *not* changed, so don't "fix" them without asking: `TextTools::reverse` (documented
  as codepoint-level to match Python's `[::-1]`), `Version`'s inner grammar scan (ASCII by spec;
  only its outer `\s*` trim went Unicode), `IniFile`'s CRLF normalisation and
  `RemapIniFixContext::cleanModTypeName` (byte compares against `\n`/`\t`/`\r` only), and
  `StringTools::splitlines`, which still splits on `\n`/`\r\n` only --- Python's `str.splitlines`
  also breaks on `\v`, `\f`, `\x1c`-`\x1e`, `\x85`, U+2028 and U+2029, a divergence that was
  noticed and flagged, not fixed, because it would change line numbering for `.ini` files that
  contain those characters.
- `Docs/src/coreAPI.rst` now defines the `` `Unicode`_ ``, `` `utf8proc`_ `` and
  `` `Python's str.lower/lstrip/rstrip/isspace`_ `` link targets; `TextTools.h` had been using the
  first two with no target defined.
