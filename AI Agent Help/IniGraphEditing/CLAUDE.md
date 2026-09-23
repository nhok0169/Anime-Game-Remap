# Ini Graph Editing

## `ResGroupCollect` keys call sites by LOCATION, and owns every replica it writes (2026-09-17)

Two bugs in the group collect, both reachable from the Python API and both silent:

- **Call sites were keyed by the resource section they reach.** `getResCallNewNames` stored one
  query and one location per root name, so when two call sites reach the same root, all but the last
  were dropped and never rewritten. That happens for a resource bound in two branches
  (`if $x == 1 vb1 = Resource1 else vb1 = Resource1` left the first branch pointing at the unremapped
  blend), and for EVERY multi-call-site `TexCreate` since it names one created section per mod.
  `ResRootCalls` now keeps a list per root, and each (call site, file) pair is its own group entry, so
  each call site is rewritten to the replica its own group is given.
- **`combine` borrows sections.** `connectResGraphs` combined replica 2..n into replica 1 and added
  replica 1 to the group; replicas 2..n were in no group and lived only as long as the graph-group
  VIEW. The Python view dies when the edit returns, so every replica after the first rendered as `[]`
  with no body -- freed memory, not a naming bug. The combined graph is now deep-copied before it is
  added. The C++ view happens to outlive the render, which is why the compiled multi-component fixers
  never showed it; don't rely on that for a new consumer of `IniSectionGraph::combine`.

The tell for the second, worth remembering: **a section with an empty name and no body is a
dangling `Section*`**, and it appears only when the replica count is two or more. The compiled
multi-component fixers did not move: 19 real Yelan / Bennett mods (both directions, the identities and
the twelve-branch Bennett3 master) through the CLI, 2463 files byte-identical before and after,
including 365 replica sections. The only differences were two `*RemapDL.dds` downloads that depended on
whether github.com resolved during that run, and re-running the old build alone reproduced one of them.

## THE PREDICATE WRITTEN BACK INTO A MOD NEEDS A DIFFERENT SIMPLIFIER (2026-09-16)

An `if`/`else if` chain accumulates: branch *n* of it means `x != 0 AND x != 1 AND ... AND x == n`,
and that is what `ResGroupCollect` writes into the `.ini` as its resource group's guard. On a merged
mod with two hundred variants the last branch carries a hundred and ninety-nine negations on one
line -- and it is `$swapvar == 199`.

**`z3::expr::simplify()` will not touch it**, which is what the collect was calling. It is a local
rewriter; discharging those negations means knowing the equality holds. Measured on the real
twelve-way chain:

| tactic | result | passes |
| --- | --- | --- |
| `simplify` | all 11 negations, unchanged | 1 |
| `ctx-solver-simplify` | **10** negations -- it drops one and stops | 1 |
| `propagate-values` | `$swapvar == 11` | 1 |
| `(repeat ctx-solver-simplify)` | `$swapvar == 11` | **12** -- one per branch |
| `solve-eqs` | **an empty goal** | 1 |

So `Z3Predicate::solverSimplify` is `propagate-values` then `ctx-solver-simplify`, and the order is
the point: propagation does the work in one rewriting pass with no solver at all -- a 200-branch
chain in well under a second -- and the solver-based one runs second on what is left.

Two of those rows are worth remembering on their own. **`ctx-solver-simplify` is the name that
sounds right and does not do this**: reaching the same answer by repeating it costs a pass per
branch. And **`solve-eqs` is actively wrong here** -- it ELIMINATES the variable rather than
simplifying around it, answering `x == 11` with nothing at all. That is sound for asking whether a
predicate is satisfiable and useless for writing a condition back into a mod, which is the
difference between the two things Z3 is used for in this repo.

Measured: the twelve-variant master's `.ini` goes from 63062 bytes to 58466, and the saving grows
with the square of the branch count. Nothing else moves -- 772 files over 18 characters and all of
Yelan's are byte-identical, because nothing else was emitting an accumulated chain.

<br>

## A PART'S CONDITION WAS WRONG BEHIND A `run =`, AND NOTHING COULD SEE IT (2026-09-16)

`IniSectionGraph::iterByQuery` reports the predicate each part sits under. It tracks, per nesting
depth, how many entries the chain open at that depth has put on the query path, so that cleaning the
chain knows how many to take back off -- and that counter was a `std::vector` **indexed by
`frame.depth`** but **grown by one `push_back` per explored NODE**. Those agree only while depth
rises exactly once per node, and it does not: a content part's children are pushed at `depth + 1`,
and a section reached through `run =` is pushed at `depth + 1` again. Follow one call and the depth
runs past the end of the vector, where `operator[]` is undefined behaviour rather than an error.

What came out was an `if`/`else if` chain whose every branch after the first carried the **first**
branch's predicate as well as its own -- `$swapvar == 0 AND $swapvar != 0 AND $swapvar == 1`, which
is unsatisfiable. Keyed by depth instead of indexed by it, the two cannot drift.

**Why it survived so long is the part worth keeping.** An `.ini` renders from its PARTS, so the
output was well formed and correctly indented the whole time; only a caller that asked the graph
what a part's CONDITION was ever saw it, and until the merge needed per-branch draws
(`RegBranchAdd`) nothing ever had. Measured after the fix: **772 files over 18 characters,
byte-identical** -- because nothing else was asking.

Three things this is worth generalising into:

- **A structure that renders correctly is not a structure that is correct.** Rendering walks the
  parts; reasoning walks the tree and the traversal built on it. Check the one you are relying on.
- **Reproduce with the CALL in place.** Every hand-written reproduction of this passed, because each
  rooted its graph at the branching section directly. The `run =` is the trigger, and so is the
  nested `if` inside a branch that `ResGroupCollect` leaves behind -- `core/tests/
  IniSectionGraph_RunQuery_test.cpp` carries both, and its first version was written flat, passed
  against the broken build, and would have shipped meaning nothing (habit 34, again).
- **A wrong answer here is silent by construction.** An unsatisfiable predicate does not throw; it
  just matches nothing, so a caller asking "which branch is this?" gets "none" and carries on.

<br>


Conventions and gotchas for the subsystem that models `.ini` file structure as a graph and edits
it — `IniSectionGraph`, `CallGraph`, `SectionIterData`/`SectionIterQueryData`, `IfTemplate`,
`IfTemplateNode`, `IfTemplateTree`, `GraphTools` (C++-backed), and the graph-editing strategies
under `model/strategies/iniFixers/graphEdits/` (`RegSurroundedAdd` is the deep, worked example —
read `core/include/AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.tpp` alongside
`Testing/Unit Tester/UnitTester/Tests/test_RegSurroundedAdd.py`, which exercises every case below
concretely; the pure-Python originals of both are deleted). This file was authored from hands-on work building
this subsystem from scratch (the fixpoint/reachability redesign of `RegSurroundedAdd`, then
extracting the reusable pieces into `IniSectionGraph`/`GraphTools`/`CallGraph`), plus — later,
separately — a full pass completing simpler stub classes across `regEdits/` (`RegAdd`, `RegRemap`,
`RegRemove`), `graphGroupEdits/` (`GraphInherit`, `GraphRemove`), and `graphEdits/` itself
(`GraphRename`), plus — later still — the full C++/pybind11 replacement of all three of those
packages, `RegSurroundedAdd`/`GraphTools` included (their pure-Python originals were kept briefly as
`RegSurroundedAddOld`/`GraphToolsOld`, then deleted outright) — see "Completing a simple
`regEdits`/`graphGroupEdits`/`graphEdits` stub" below for what came out of that. It hasn't been exercised as deeply for the *other* `.ini`-parsing
subsystems (`GIMIFixer` family, the non-graph parsers), so verify assumptions there rather than
assuming this file covers them too. See
[Overview](../Overview/CLAUDE.md) for how this fits in the wider repo, and
[Testing](../Testing/CLAUDE.md) for the `IfContentPart` index-renumbering trap that bites
constantly when hand-building synthetic test graphs for this subsystem.

**`IniSectionGraph`/`CallGraph`/`SectionIterData`/`SectionIterQueryData`/`IfTemplate`/
`IfTemplateNode`/`IfTemplateTree` are now C++-backed** (a later, separate full-replacement port,
`AGRemapCore::IniSectionGraph`/etc. + pybind11 bindings under `py/src/model/`) — the pure-Python
originals this section originally described (`model/IniSectionGraph.py`, `model/CallGraph.py`,
`model/SectionIterData.py`, `model/iftemplate/IfTemplate.py`/`IfTemplateNode.py`/
`IfTemplateTree.py`) no longer exist at all (not even as `...Old.py` — they were kept briefly for
a comparison safety net during the port, then deleted outright once test parity was confirmed).
Every *semantic*/algorithmic gotcha below (the call-vs-jump mental model, the two graph
representations, the fixpoint reachability trap, ...) still applies unchanged — only the
implementation language and the exact bound method surface differ from what's described; verify a
method name against the real C++ binding (`py/src/model/PyIniSectionGraph.cpp`,
`py/src/model/iftemplate/PyIfTemplate.cpp`) rather than trusting a `_leadingUnderscore`-named
method mentioned below still exists under that exact name — pybind11 bindings in this codebase
don't carry Python's private-method-naming convention over; a method the old Python class kept
private (`_getQuery`, `_trueQuery`) may now be a public C++ method that's simply **not exposed to
Python at all** (reachable only indirectly, e.g. through `iterByQuery`/`processIfContentByQuery`
calling it internally) rather than renamed. **Read [Architecture](../Architecture/CLAUDE.md)'s
"pybind11 wrapper... is only alive while something holds a real Python reference to it" section
before touching any of these classes' bindings** — the exact bug class it describes (a silent
`id(part)` collision, and separately a real crash) was found and fixed in this exact subsystem,
and a new binding method here that hands back a raw pointer or an `id()`-keyed correlation is the
most likely place to reintroduce it.

## Predicate queries in this subsystem are Z3-typed, not sympy

`IniSectionGraph`'s (C++-internal-only, not Python-bound) `getQuery`/`trueQuery` and
`SectionIterQueryData.query` carry `FRB.Z3Predicate` values (from the C++ core's
`Z3Context`/`Z3Predicate`, combined here via plain `&`/`|`/`~` operator overloads and
`.simplify()`), not `sympy` expressions — this subsystem was migrated off sympy in the same effort
that ported `IfPredPart` to C++/Z3. If you're adding a new graph-editing strategy or dataflow rule
that needs to build/combine/compare a predicate, reach for `Z3Predicate`'s operators and
`IfPredPart.getLogicQuery`/`.getIfPredStr` (or a real `z3::solver`-based equivalence check, never
string/structural comparison), not `sympy`. The deprecated sympy-typed pure-Python `IfPredPart`
has since been fully removed (there is no `...Old` fallback anymore) — every `.query` you find
anywhere in this codebase is `Z3Predicate`-typed now, no need to check which variant you have. See
[Architecture](../Architecture/CLAUDE.md)'s Z3-wrapping and `IfPredPart` migration-scope sections
for the full story (why two typed variants of "a part with a predicate" now coexist, and the
pimpl/friend pattern behind `Z3Context`/`Z3Predicate` themselves).

**The design is one `Z3Context` per `IniFile`, not one per `IniSectionGraph`** — a graph never
owns its own context. `IniSectionGraph` carries an optional `_z3Ctx` attribute (`z3Ctx` constructor
kwarg), but at every real construction site (`GIMIParser.py`, `ResEdit.py`) that value is just
`ini._z3Ctx` passed through by reference — the *same* `Z3Context` object the owning `IniFile`
already created once in its own `__init__`/`clear`. Don't read "`IniSectionGraph` has a `_z3Ctx`
attribute" as "each graph gets its own context" — it doesn't; it borrows its owning `IniFile`'s.
Every `Z3Predicate` the graph's own `_getQuery`/`_trueQuery` produce (including the
literal `True` reported for a part with no enclosing `if`_/`elif`_/`else`_ at all) belongs to this
context, and it must be the *same* `Z3Context` the graph's `sections` were actually built against
(eg. `IniFile._z3Ctx`) — not a fresh, unrelated `Z3Context()`. **There is deliberately no
lazy-fallback-construct-one-if-missing path**: `_trueQuery()` raises `ValueError` if `_z3Ctx` is
`None` rather than silently building a throwaway context, because a throwaway context's "true"
predicate would belong to a *different* context than the graph's own real predicates and the very
next `&`-combination in `_getQuery` would hit the assert-only mismatched-context gotcha described
in [Architecture](../Architecture/CLAUDE.md)'s Z3 section. If you add a new `IniSectionGraph(...)`
construction site whose graph will ever have `iterByQuery`/`processIfContentByQuery` called on it,
thread `z3Ctx = ini._z3Ctx` (or equivalent) through explicitly — grep the existing call sites in
`ResEdit.py`/`GIMIParser.py` for the pattern. A graph that's only ever built/renamed/structurally
combined (never queried) doesn't need one at all. Also note `IniSectionGraph.__deepcopy__` is a
custom override, not the default `copy.deepcopy(self)` — `Z3Context` is move-only (its C++ copy
constructor is deleted) with no `__copy__`/`__deepcopy__` binding, so a deep copy has to swap
`_z3Ctx` out, deep-copy everything else, then re-attach the *same* `Z3Context` reference to the
result, rather than trying to copy it. If you add a new attribute to `IniSectionGraph` that itself
holds a `Z3Context`/`Z3Predicate`-bearing object graph, this override is where it needs threading
through too, not the plain per-field `deepcopy(minimal=True)` path.

**`ResGroupCollect.py` is the one place that legitimately crosses this one-context-per-`IniFile`
boundary** — a source mod object's graph and a resource's own destination graph can come from
different `.ini` files entirely, each with its own `IniFile`-owned `Z3Context`, so `_combineQueries`
can genuinely receive two predicates that don't share one; this isn't a symptom of graphs owning
their own contexts (they don't, see above), it's the one seam where two *different* `IniFile`s'
contexts legitimately meet. Combining them with a raw `&`/`|` is exactly the unsafe pattern
[Architecture](../Architecture/CLAUDE.md)'s "combining two `z3::expr`s from different contexts"
section warns about — it doesn't reliably throw, it can silently misbehave.
`ResGroupCollect._combineQueries(a, b, targetZ3Ctx)` is the fix and the pattern to reuse for any
new cross-graph query combination in this file: check `Z3Predicate.belongsTo(targetZ3Ctx)` for each
operand *first*, `IfPredPart.reparent(predicate, targetZ3Ctx)` only whichever one doesn't already
belong, *then* combine. **`belongsTo()` is a cheap raw pointer comparison** (`shared_ptr<z3::context>`
address equality, no Z3 solver work at all — see `Z3Predicate::belongsTo` in `Z3Predicate.cpp`),
while `reparent()` is a genuinely expensive full `.ini`-text render + re-tokenize + re-parse + Z3
re-generate round trip — this guard is exactly what keeps the common case (both operands already
share `targetZ3Ctx`, eg. same `IniFile`) down to two pointer comparisons with `reparent()` never
called at all; it only runs on the actual cross-`IniFile` case. Don't skip the `belongsTo()` check
when reusing this pattern elsewhere, and don't call `reparent()` unconditionally "to be safe" — it
isn't free. `_buildResIfCalls` follows the same shape when constructing the new `IfPredPart`s that get
spliced back into a destination graph — it resolves that destination graph's own `_z3Ctx` (via
`_resolveToGraph`) before building anything, precisely so the new parts' `.query` ends up in the
*right* context, not whichever context the source query happened to arrive in.

**`isSatisfiable()` (a real `z3::solver`, via `Z3Predicate.isSatisfiable`) needs no equivalent of
the old sympy `.replace(sympy.Ne, lambda a, b: sympy.Or(sympy.Lt(a, b), sympy.Gt(a, b)))` rewrite**
that `ResGroupCollect.py` used to need before calling `sympyLogicInference.satisfiable(...,
use_lra_theory=True)` — that rewrite existed only because sympy's LRA-theory satisfiability check
needed `!=` desugared into a disjunction of strict inequalities first. Z3's solver decides `!=`
over reals natively; don't reintroduce an `Ne`-rewrite-shaped workaround if you're touching this
code again, it's solving a problem Z3 doesn't have.

**Writing a test that needs an expected `Z3Predicate` value**: don't hand-build a `z3::expr`-shaped
tree of sympy-style calls and don't try to give `Z3Predicate` a public constructor for tests to call
directly — the class's own constructor is intentionally private (see
[Architecture](../Architecture/CLAUDE.md)'s friend-allowlist pattern). Instead, build the expected
value the same way real code does: write the equivalent `.ini`-predicate-syntax text and parse it,
eg. `FRB.IfPredPart(f"if {text} then", FRB.IfPredPartType.If, z3Ctx).query` (a small
`q = lambda text: ...` helper defined once per test method is the established convention for
this). This is how the pure-Python `IniSectionGraph`'s own now-deleted sympy-based test suite was
converted when it was migrated off `sympy.Eq`/`And`/`Not`/`Ne`-shaped expected values — every one
of them converted mechanically into `.ini` text this way (eg.
`And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0))` → `q("!($x * 6 == 0) && $x / 5 == 0")`) via a
one-off `ast`-module script that walked each sympy-call expression's AST and rendered it as text —
worth reaching for again if another sympy-based test file in this area ever needs the same
treatment, rather than converting many expressions by hand. Compare the result with
`BaseUnitTest.compareZ3Query(result, expected)` (`Testing/Unit Tester/UnitTester/Tests/
baseUnitTest.py`) — a real solver-backed equivalence check (asserts `sameContext` first, then that
neither `result & ~expected` nor `~result & expected` is satisfiable), not `compareQuery` (which
stays sympy-typed, still used by the live sympy-based `test_IfPredLogicGenerator.py`/
`test_SympyIfPredGenerator.py` — don't repoint those at `compareZ3Query`, and don't repoint a
Z3-typed test at `compareQuery`).

## The mental model: `run =` is a call, not a jump

A `.ini` `section`_ can invoke another section via a `run =` `KVP`_. This is **call-with-return
semantics** (confirmed with the maintainer, not assumed) — the callee eventually hands control
back to whatever comes after the call, not a plain `goto`_. Two real consequences fall out of
this that are easy to get wrong:

- A section can call itself (directly, or through a cycle of several sections) — this is a
  genuinely supported case to design for, not an edge case to special-case away, even though (per
  the maintainer) real mods never actually write recursion this way. The graph tooling is built to
  be *mathematically* sound for it regardless, since "nobody writes it in practice" doesn't help
  you if the algorithm silently does the wrong thing on the input it validates as safe.
- "Guaranteed via the call" (what happens once you jump *into* the callee) and "guaranteed once
  the call has *returned*" (what happens once you're back in the caller) are genuinely different
  facts, needed for different candidate positions. Conflating them is exactly the class of bug
  this subsystem hit twice during development (see below) — once for positions *before* a call
  vs *after* it, and once for "some future occurrence exists" vs "the *nearest* one exists".

## Real mods contain structurally malformed sections — every graph builder must tolerate them

A production mod folder (~2100 `.ini` files) turned up five sections with a stray `endif` and no
matching `if` (e.g. a `[Resource...]` section ending in a lone `endif`; two of them were in
`...RemapFix` sections, i.e. this tool's own earlier output). `IfTemplateTree::construct` already
skips a stray `elif`/`else`/`endif` at depth 0; `IniSectionGraph::computeSectionPredecessors` did
not — it indexed its (empty) frame stack, which was an immediate access violation on one mod and
silent heap corruption (a `pop_back()` on a size-0 vector) on others, surfacing much later as
"random" crashes at teardown. It now mirrors the tree and treats such a part as a pass-through
(`current` unchanged); `test_IniSectionGraph.py`'s `strayEndIf`/`strayElseAndElif` tests pin that.
**If you write a new walker over `IfTemplate.parts` that keeps an `if`-frame stack, guard every
`back()`/`pop_back()` with an empty check, and null-check the `dynamic_cast<IfPredPart*>` — do not
assume the parser handed you balanced input, because it doesn't validate that.** Also note the
Python benchmark that found this needed one subprocess per file to isolate it: the corruption from
one malformed file only crashed several files later, in code that had nothing to do with it.

**The OTHER half -- an `if` that is never closed -- was dropped until 2026-09-22.** A Chisa mod's
`[TextureOverrideTexture9]` opens `if $object_detected`, nests an `if $haircolor == 0 / elif` chain,
closes the inner one and ends the section. 3dmigoto closes it at the section's end and the mod works
in game; all three `IfTemplateTree` builders left the still-open node off the stack, so the tree
reached 1 of the section's 3 content parts. **Nothing errored and the `.ini` still rendered from its
parts**: every edit that walks the tree (`RegRemap`, `RegRemove`, the collects) simply never saw the
two `this =` lines, and the prototype's hair toggle lost its diffuse. All three builders now close
whatever is still open when the parts run out (`test_IfTemplateTree.py`'s
`test_unclosedIf_closedAtTheEndOfTheSection`, which fails against the old build). The norm tree
adds no synthetic `else` for such a block, because that would insert parts the section never had.

## Two graph representations — don't reach for the wrong one

- **`IniSectionGraph.buildPartPredecessorGraph()`** — a static, part-level "who runs immediately
  before whom" graph (also `IniSectionGraph.computeSectionPredecessors(parts)` for the
  single-section, no-`run=` version of the same question). No notion of a call "returning" at
  all — it's built purely for **dedup**: "has this part's `surrounded`_ window already been
  claimed by something that runs before it, so I should skip it." A part that (transitively)
  calls back into its own section ends up listed as its own predecessor; this is harmless here
  specifically because `IniSectionGraph.iterByContentPart`'s own cycle-pruning guarantees each
  part is only ever visited (and decided) once, so the self-referential edge never actually gets
  consulted. Don't reach for this when you need to reason about anything *after* a call returns —
  it has no representation for that at all.
- **`IniSectionGraph.buildCallGraph()` → `CallGraph`** — a proper call graph with a virtual
  `("exit", id(part))` node for every part that makes a `run =` call, representing "control has
  returned here." Use this (via `GraphTools`, below) for anything that's a genuine dataflow
  question — "is X guaranteed true at this point, given everything that could have run before/after
  it." Use `CallGraph.exitNodeOf(partId)` to get the right node for "after this part's own call
  returns" rather than re-deriving `("exit", partId) if part.getVals(IniKeywords.Run.value) else
  partId` by hand (this exact line was duplicated in three places before being pulled out).

## `GraphTools` — generic, not `.ini`-specific

`tools/GraphTools.py` holds `getReachableNodes`, `clampFactsToReachable`,
`runForwardMustFixpoint`, and `runBackwardMustFixpoint` — a small forward/backward MUST-style
dataflow fixpoint engine (Kildall's/worklist algorithm) that knows nothing about `IfContentPart`
or sections; it operates purely on `Dict[node, List[node]]` adjacency plus a per-node
`localFacts` dict the caller supplies. **Any new analysis that needs to answer "is some boolean
property guaranteed at point X, across a graph that may contain cycles" should reuse this rather
than hand-rolling another worklist loop inside a single edit class** — that duplication (three
near-identical copies of predecessor-graph-building and fixpoint code inside `RegSurroundedAdd`
alone) is exactly what this module exists to have fixed.

### The cousin nobody fixed: a never-ending `run =` cycle counts as "satisfied" too

Clamping handles *unreachable* nodes. It does nothing for nodes that are reachable but sit on a
cycle no path ever leaves -- `A` runs `B`, `B` runs `A`, nothing else. The backward MUST fixpoint
asks "does the after-register lie ahead on **every** path?", and a cycle with no escaping path has
no path on which it *doesn't*, so the greatest fixpoint keeps the optimistic `true` there even when
the register exists nowhere at all. Measured 2026-09-06: on that
two-section cycle `RegSurroundedAdd(afterRegs={"drawindexed"}, latest=True)` inserts once in `B`,
right before it calls back into `A`, exactly as if a `drawindexed` were inside the call. Even a
*guarded* recursion (`B` only runs `A` inside an `if`) behaves the same, because the return edges
are context-insensitive and the exit nodes end up on the cycle too. It is pinned, not fixed, by
`test_edit_cycle_afterRegNotOnTheCycle_neverEndingCycleCountsAsClosingTheWindow`:
an infinitely recursive `.ini` is not something a real mod contains, and switching the analysis
to a least fixpoint would change every existing cycle test. Decide deliberately if it ever matters.

### The gotcha that actually shipped a bug: unreachable nodes keep their optimistic default forever

A MUST-style fixpoint starts every non-boundary node **optimistic** (assume satisfied) and only
ever refines it *down* as real predecessors/successors are found not to satisfy it. That's correct
for any node actually reachable from a root — but a node that's structurally part of the graph yet
**unreachable from every root** (eg. the `("exit", id(part))` continuation node for a `run =` call
that never returns, because the callee always recurses again with no escape) sits in its own
disconnected component. Nothing ever pulls its value down, so it silently keeps reporting
"satisfied" forever — a vacuous, unsound "fact" about a position that can provably never execute.

This is not hypothetical: it shipped in this exact codebase for one review cycle. `RegSurroundedAdd`
briefly proposed inserting a KVP *after* an unconditional, never-returning `run =` call, reasoning
"the fixpoint says the register is guaranteed satisfied there" — genuinely dead code, caught only
because the maintainer manually traced the unrolled execution by hand and noticed the inserted
line never actually ran. **Always run a raw `runForwardMustFixpoint`/`runBackwardMustFixpoint`
result through `GraphTools.getReachableNodes` + `clampFactsToReachable` before trusting it** — see
`IniSectionGraph.buildCallGraph` (now C++-backed, see the port note above) combined with
`RegSurroundedAdd._computeKeyFacts` for the pattern. A minimal, direct construction of the
disconnected-component shape that causes this: a section whose only `run =` target is itself, with
nothing else reachable — `CallGraph.forwardEdges`/`backwardEdges` for that section's own
`("exit", id(part))` node then sits with no path back to any root at all (`test_CallGraph.py`'s
`test_buildCallGraph_parentCallsChild_edgesReflectTheCall` shows the general shape of these
`("exit", id(part))` nodes; a self-referencing-`run=` variant of it is a fresh test still worth
writing if you're working in this exact area, not something already covered).

### The subtler follow-up: prefer the *nearest* true reason over a technically-true one

Even after reachability-clamping fixes the "provably dead" case, a second, subtler issue remains:
a fixpoint answer can be **technically sound but semantically surprising** when it's satisfied only
by looping all the way back around a cycle to a *future* occurrence, while a *nearer*, already-
present occurrence would satisfy the exact same requirement without needing the cycle at all. Both
are valid per a literal reading of "some accepted occurrence exists after this point" — but a
human reading the result expects the nearer one. `RegSurroundedAdd._getValidRangeForPart` handles
this by computing a "local-only" candidate (with every call/return-graph credit disabled) first,
and only falling back to the full, cycle-exploiting answer when the local-only one is empty. If
you're building a new analysis over `CallGraph`/`GraphTools` and multiple candidates can satisfy a
requirement for different underlying reasons, consider whether this same "prefer the boring,
locally-obvious answer" preference applies before shipping the numerically-latest/cycle-exploiting
one as the default.

## The `model/` "return data" class convention

A function/method that needs to return several related values together should define a small,
plain data-holder class instead of returning a tuple — `SectionIterData`/`SectionIterQueryData`
were the original pure-Python precedent for this, and `CallGraph` (holding
`IniSectionGraph.buildCallGraph()`'s four return values plus the `exitNodeOf` convenience method)
was built specifically to replace a positional 4-tuple mid-refactor. All three are now C++-backed
(see the port note near the top of this file) — the convention lives on in `AGRemapCore`'s own
Doxygen doc-comment shape (`@param`s doubling as the equivalent of the old `Parameters`/
`Attributes` docstring split), not the original bare-`__init__`-plus-numpydoc-docstring Python
shape, but the underlying design idea (a named data holder over a positional tuple) is the same
one to reach for if a new method needs to return several related values together.

## Completing a simple `regEdits`/`graphGroupEdits`/`graphEdits` stub

**All three families — `regEdits/`, `graphGroupEdits/`, and `graphEdits/` — are now C++-backed**,
across three separate full-replacement ports (`AGRemapCore::BaseRegEdit`/`RegAdd`/`RegNewVals`/
`RegRemap`/`RegRemove`; `BaseIniGraphGroupEdit`/`GraphRemove`/`GraphInherit`/`GraphGroupRemap`/
`GraphGroupEdit`/the `resEdits/` family/`ResRegCollect`/`ResGroupCollect`; and
`BaseIniGraphEdit`/`GraphRename`/`RegFillMissing`) — class templates under
`core/include/AGRemapCore/model/strategies/iniFixers/<family>/`, plus pybind11 bindings under
`py/src/model/strategies/iniFixers/<family>/`. Every pure-Python package this section describes has
been **deleted outright**, not renamed to `...Old` — don't go looking for `RegAdd.py`/
`BaseRegEdit.py`/`GraphRename.py`/`RegFillMissing.py`/etc.

**`RegSurroundedAdd` (and the generic `GraphTools` dataflow engine it depends on) has since been
ported to C++/pybind11 too** — it was, for a while, the one class left pure Python in all three
families (deliberately scoped out of the initial `graphEdits/` port), but that gap has since been
closed. Its pure-Python original was kept briefly as `RegSurroundedAddOld` (and `GraphToolsOld`)
while the port was verified, then deleted outright like the rest of the family. One lesson from
that interim state is still worth keeping: a pure-Python class that subclasses a *pybind11-bound*
base (as `RegSurroundedAddOld` did with `BaseIniGraphEdit`) must call `super().__init__()` from its
own `__init__`, or the C++ subobject is never constructed. Watch for that whenever a
still-pure-Python class's base becomes C++-backed. The live `RegSurroundedAdd`/`GraphTools` are C++
all the way down and have no such concern.

### How `partFilter` and key tracking actually flow — read this before adding either to an edit

Two things here look like they should already work and, until recently, silently didn't. Both are
now fixed, but the *shape* of them is what matters when extending this:

- **A `partFilter` reaching an edit does not mean the edit uses it.** `GraphGroupEdit` hands every
  edit a `partFilter` (the caller's own `keyFilters` entry, or a `defaultPartFilter()` returning
  `Ranges.createFull()`), and an edit is free to accept and ignore it — which is exactly what the
  pure-Python `RegFillMissing` did, and what its C++ port faithfully preserved. `RegSurroundedAdd`
  reads it; `GraphRename` genuinely has no use for it; `RegFillMissing` now honours it. **Before
  building a new "restrict this edit to certain parts" feature, check whether the edit is simply
  dropping the `partFilter` it already receives** (`grep -n partFilter` in both the core `.tpp` and
  the `Py*.cpp` — a `(void)partFilter;` is the tell). Making it honour the existing parameter is
  usually the whole feature, and needs no new argument.
- **The convention for a `partFilter` used as a *part selector*: an empty `Ranges` skips that part,
  any non-empty result accepts it, and the actual ranges are not consulted.** That's what
  `GraphGroupEdit` already does for register edits (`if (keyRanges.isEmpty()) continue;`), and what
  `RegFillMissing` matches. Don't invent a second selection concept (a separate `bool` predicate
  argument) when this one is already threaded everywhere.
- **`GraphGroupEdit`'s `trackKeys`/`keysToTrack` only ever reached *register* edits.** They are read
  on exactly one line — `result->iterByContentPart(1, trackKeys, keysToTrack)` — inside the
  `PartEditKind::RegEdit` branch of `filterGroupEdit`; the `GraphEdit` branch returns before it. A
  graph edit walks the graph itself, so nothing `GraphGroupEdit` builds ever reached it, and setting
  the flag for one was a silent no-op. They are now **handed down** as `trackKeys`/`keysToTrack`
  parameters on `BaseIniGraphEdit::edit`/`editFromIni`, which `PartEdit::editGraph` passes through.
  An edit with its own key-tracking setting combines the two (`RegFillMissing::effectiveTrackKeys` =
  `own || caller`; `effectiveKeysToTrack` = own if set, else the caller's); one without simply
  ignores them.
- **Adding a parameter to `BaseIniGraphEdit::edit` is a migration, not a signature tweak** — every
  pure-Python override breaks on the new keyword. See [Architecture](../Architecture/CLAUDE.md)'s
  section on the no-trampoline arity trap for the inventory grep to run *first*.
- **`editFromIni` must forward whatever `edit` now consumes.** `GraphGroupEdit` routes through
  `editFromIni` (not `edit`) whenever it has an `.ini` file, so an `editFromIni` that drops an
  argument silently disables that feature for exactly the callers who configured it. The pure-Python
  originals dropped `partFilter` here; the ports deliberately do not.
- **A binding's `editFromIni` reaches `edit` via `self.attr("edit")`** (so a pure-Python subclass's
  override still wins), and `edit`'s signature has nowhere to carry an `.ini` — so a `partFilter`
  invoked through the *Python* `editFromIni` receives `None` as its third argument. A plain C++
  caller gets the real one (that's what `RegFillMissing::editImpl` exists for). Don't "fix" this by
  binding straight to the core `editFromIni`; that would skip subclass overrides.

Everything below still applies to writing a *new* edit in any of the three families, and the rows in
the primitive table still name the right primitive for each task — just implemented in C++ now.
Things specific to these ports, if you're extending any of the families:

- `BaseRegEdit` and its subclasses are **class templates** over the same `K`/`V`/`KeyHash`/
  `KeyEqual` as the `IfContentPart` they edit — they have to be, since the pybind11 layer edits
  `IfContentPart<py::object, py::object, ...>` while a plain C++ caller wants
  `IfContentPart<std::string, std::string>` (the template defaults). Header + `.tpp` only; nothing
  to add to `core/CMakeLists.txt`.
- `editFromIni`'s `ini` and `edit`'s `modType` are **nullable pointers** in the C++ core, and the
  pybind11 layer always passes `nullptr` for both: every one of these edits ignores them (exactly
  as the pure-Python originals did), the Python-side `ModType` is still a pure-Python class with no
  C++ counterpart to hand over, and `AGRemapCore::IniFile` isn't bound to Python at all.
- Each `PyRegXxx` binding keeps the **exact Python object** the caller passed for its
  `vals`/`keyRemap`/`removeKeys` argument and re-derives the C++ member from it at the start of
  every `edit()` (`PyRegXxx::refresh()`). That preserves both of the pure-Python originals'
  observable behaviours — `someEdit.vals is theThingYouPassed`, and an in-place mutation of that
  object changing what the edit does — neither of which survives a parse-once-into-a-C++-copy
  design. It matters most for `RegRemove`, whose values are Python callables that pybind11 cannot
  hand back as the *same* callable (its `std::function` caster re-wraps them in a fresh
  `cpp_function`).
- The same **keep-the-Python-object-and-re-derive-per-edit** pattern carried straight over to
  `graphEdits/`: `GraphRename` stores the caller's own `renameFunc` object (`test_GraphRename.py`
  pins `assertIs` on it), and `RegFillMissing` stores the caller's own `fillMissing` *and*
  `fillMode`. `RegFillMissing`'s two have to be re-derived **together**, not independently — which
  end of a part a bare value/`KVP` list is added to is decided by the *mode*
  (`TopdownCover` → front), exactly as the pure-Python original's
  `_getFillMissingFunc(self.fillMissing, toFront = isCoverMode)` did.
- **`RegFillMissingMode` has a third mode, `BottomCover` (2026-09-12).** `FillMissing` fills the
  FIRST content part that lacks the key (`IfTemplateNode::getKeyMissingPart`), `TopdownCover` adds
  a fresh first part; `BottomCover` mirrors it with a fresh LAST part at each root
  (`RegFillMissing::addBottomCover`, `addBottomContentPart`). It exists because a `ResRegCollect` /
  `ResGroupCollect` splices its register into an `if 1 ... endif` block, which splits the section
  into parts, after which a `drawindexed` filled with `FillMissing` landed BEFORE the ib and the
  textures. Bound as `"bottomCover"`, five tests in `test_RegFillMissing.py`.
- **A binding that mutates an `IniSectionGraph`'s structure must call
  `PyIniSectionGraph::refreshKeepAlive()` before returning.** `GraphRename` relabels sections;
  `RegFillMissing`'s `TopdownCover`/`addCover` appends brand-new `IfContentPart`s, and its
  `DownloadMode.Always` path calls `normalize()`, which splits sections into fresh parts. None of
  those new parts are in the graph's Python-side keep-alive until it is refreshed — see
  `PyIniSectionGraph.h`'s own note on the `id(part)`-collision class of bug that causes.
- **`AGRemapCore::IniFile` carries no `downloadMode`**, so `RegFillMissing`'s core class exposes a
  second, `DownloadMode`-taking `editFromIni` overload for a plain C++ caller, and the binding reads
  `ini.downloadMode` off the *Python* object instead (via `.value`, the same way `PyResEdit.cpp`
  maps `IniGraphReplaceMode`). Two new core enums (`constants/DownloadMode.h`,
  `constants/RegFillMissingMode.h`) exist purely to mirror the still-pure-Python `Enum`s by value —
  neither side replaced the other.
- **A `RegFillMissing`-shaped binding whose `editFromIni` branches before delegating must still
  reach `edit` through `self.attr("edit")`**, not through the C++ core's own `editFromIni` — with no
  trampoline in play, a C++-internal virtual call silently skips a pure-Python subclass's `edit`
  override. That is why the download-mode branching is duplicated in the binding rather than
  delegated to the core overload.

Most stubs in `model/strategies/iniFixers/regEdits/`, `graphGroupEdits/`, and the simpler
`graphEdits/` classes (i.e. not `RegSurroundedAdd`-style dataflow features) are **thin wrappers
around one existing primitive** — `__init__` just stores the constructor args as attributes, and
`edit()` is a one-or-two-line delegation, then returns the mutated object. Find the matching
primitive before writing anything by hand:

| Task | Primitive | Existing wrapper |
| --- | --- | --- |
| Bulk-add KVPs | `IfContentPart.addKVPs`/`addKVPsToFront`/`addKVPAt` | `RegAdd` |
| "Exactly one `NNFix` per draw-free stretch of every execution path": right before every `drawindexed`, plus once at the very end of the path, never twice in a row | `RegDelimitedAdd(additions, delimiterRegs={"drawindexed": None, "drawindexedinstanced": None})` -- `additions` (on `RegSurroundedAdd` too, since 2026-09-08) is a **list** of KVPs, or one tuple; the whole list lands together at each chosen position, in order | Two placement rules, no fixpoint, no claiming: the addition goes **immediately before every delimiter occurrence**, and **at the end of every path-terminal part** (a part whose call-graph exit node has no successor -- the end of a root that nobody `run`s). That is exactly-once per delimiter-free segment on every path, by construction. Do NOT reach for `RegSurroundedAdd` here: it is once-per-*window* across the graph, so two draws in sequence get one insertion (before the last), and a per-part "fallback" on it -- built and deleted on 2026-09-06 -- doubled the addition on the paths that skip the draw. Known limit: a section that is both a root and a `run` target gets no trailing insertion on its direct path (its exit has a return edge), by choice never double-applying. Register roles elsewhere are the usual: `beforeRegs` come *before* the addition, `afterRegs` *after* it -- getting these backwards was a real review catch, twice |
| Bulk-rename keys | `IfContentPart.remapKeys` | `RegRemap` |
| Bulk-remove keys | `IfContentPart.removeKeys` | `RegRemove` |
| Rename every `section`_ in a graph (rewrites `run =` refs too) | `IniSectionGraph.rename` | `GraphRename` |
| Add a bottom/front `IfContentPart` to a `section`_ | `IfTemplate.addBottomContentPart`/`addTopContentPart` (+ `addKVPsToBack`/`addKVPsToFront`) | `GraphInherit` |
| Look up / add / remove a graph by `(iniFileIndex, component, object)` id | `BaseIniGraphGroupEdit.getGraph`/`addGraph`, `IniGraphGroup.removeGraph` | `GraphInherit`, `GraphRemove` |

- **`IfTemplate.addBottomContentPart`/`addKVPsToBack`/`addKVPToBack` now exist**, mirroring the
  pre-existing `addTopContentPart`/`addKVPsToFront`/`addKVPToFront` (added while building
  `GraphInherit`, which needed to append at the very end of a root `section`_). They're safe for
  the same structural reason the front versions are: a well-formed `IfTemplate`'s **last** part is
  always depth 0 (either plain content, or the `EndIf` that closes an outermost conditional) —
  mirroring the front invariant that the *first* part is always depth 0 — so appending a fresh
  depth-0 `IfContentPart` after it (or reusing it if it's already a depth-0 `IfContentPart`) can
  never land inside a branch. Verified empirically against a section ending (and, separately,
  starting) mid-conditional before trusting this by hand-derivation alone.
- **The `partRanges`-boundary insertion formula** (for anything shaped like `RegAdd`: add N items
  either at the front or back of an optional `Ranges` window, honoring a `latest` flag): compute
  one boundary index — `ranges[0][0]` (clamp `None` → `0`) for the front, `ranges[-1][1]` (clamp
  `None` → `len(part)`) for the back — then call `part.addKVPAt(insertInd + i, key, val)` for each
  item in order, `i` from `0`. This single increasing-index loop is correct for **both** directions
  (`RegAdd.edit` verified this empirically for both `latest=True`/`False`, including multi-range
  `Ranges` and unbounded endpoints). **Do not copy `RegSurroundedAdd._pickInsertInd`'s `- 1`
  adjustment for the `latest` case here** — that method's `Ranges` already represents *valid
  insertion indices themselves* (computed via `IfContentPartColouring.getRanges`'s dataflow), while
  `partRanges` here follows the same convention as `replaceVals`/`removeKey`/`getVals` (an
  occurrence's *true positional index*, half-open `[start, end)`) — the two `Ranges` mean different
  things at the same numeric value, and blending the two formulas by surface pattern-matching
  produces an off-by-one.
- **Verify the stub's already-filled-in base class against its sibling files before implementing
  — a scaffolded stub can have the wrong one.** `RegRemap.py`'s stub inherited
  `BaseIniGraphPartEdit` directly instead of `BaseRegEdit` (its `regEdits/` siblings' actual base),
  which would have silently broken `editFromIni`: `BaseIniPartEdit.editFromIni(self, ini, *args,
  modType, ...)` takes `ini` as its *first* positional argument, while `BaseRegEdit.editFromIni(self,
  part, sectionName, ini, modType, ...)` takes `part` first — a caller using the correct
  `BaseRegEdit`-shaped call convention would have silently passed its arguments to the wrong
  parameters. Nothing catches this at write time (both base classes exist and both are valid
  Python); it only breaks when `editFromIni` is actually invoked. Compare the parent-class import
  and the constructor/`edit()` signature shape against a working sibling in the same directory,
  don't just trust what's already filled in.
- **A missing-graph/missing-key "not found" case is a real behavior decision, not a default to
  guess at — ask if the request doesn't say.** `BaseIniGraphGroupEdit.getGraph` defaults to raising
  `KeyError`, but a caller passing `errorOnNotFound=False` gets a graceful `None`/skip instead —
  both are legitimate depending on the class's contract (`GraphInherit` started as raise-by-default,
  then was explicitly changed to skip-silently on request; `GraphRemove` was skip-silently from the
  start, per spec). Don't default to whichever is easier to write; confirm which one the task
  actually wants.
- **"Merge these two graphs" is ambiguous between two very different operations — don't assume
  either without asking.** `IniSectionGraph.combine()` performs a full structural merge (copies the
  other graph's `sections` into this one, recomputes `.roots`/`.neighbours` via a fresh `_build()`
  DFS) — appropriate when the two graphs genuinely belong in the same `.ini` file's output.
  `GraphInherit`, by contrast, only inserts reference `KVP`s (e.g. `run = <otherGraph'sRootName>`)
  into the source graph and leaves both `IniSectionGraph` objects — and, critically, both graphs'
  entries in `graphGroups` — otherwise untouched, since `src`/`dst` ids each carry their own
  `.ini`-file index and unconditionally combining could silently relocate one graph's `section`_s
  into the other's output file. If a task says "merge"/"combine" two graphs, that phrasing alone
  doesn't tell you which of these (or something else) is meant — ask, the same way you'd ask about
  a `not found` default above, rather than picking the one that's less work to implement.
- **Before registering a newly-completed stub's bare class name in `FixRaidenBoss2/__init__.py`,
  check whether an old, same-named class under `regEditFilters/` has already been renamed to
  `...Old`** (freeing the bare name), or hasn't been yet (a real collision waiting to happen) — `git
  status`/`ls` that directory rather than assuming either way. This repo is mid-migration from the
  old `regEditFilters/`+`GIMIObjRegEditFixer` system to the new `regEdits/`+graph-based one; several
  old classes (`RegNewVals` → `OldRegNewVals`, `RegRemap` → `RegRemapOld`, `RegRemove` →
  `RegRemoveOld`) have already been renamed this way, sometimes mid-session by the user/a linter
  rather than by whichever agent is currently working — don't assume the state you last saw is
  still current. Also remember the general rule from [Building](../Building/CLAUDE.md#adding-a-brand-new-source-file--registration-is-never-automatic):
  a stub already existing on disk (even a fully-implemented one) doesn't mean it's reachable as
  `FRB.Xxx` — check it's actually imported *and* in `__all__` before assuming otherwise (confirmed
  missing for `BaseIniGraphPartEdit` despite the class itself already being complete).
- **A completed `regEdits`/`graphGroupEdits`/`graphEdits` stub often has a matching
  `Testing/Unit Tester/UnitTester/Tests/test_Xxx.py` that already exists as a literal
  `# TODO: ...` one-line placeholder**, not a missing file — `Write` will refuse to overwrite it
  without a prior `Read` (it exists, just empty of real content). Check for this before assuming
  you need to create the test file from scratch.
- Verify empirically (a throwaway script under the session scratchpad, via the PowerShell tool —
  see [Building](../Building/CLAUDE.md)'s note on why Bash fails to import the native `core`
  extension) before writing the formal unit tests, same as the dataflow-feature guidance below —
  this applies even for a change that's **pure Python with zero C++/Cython touched** (e.g. the
  `IfTemplate.addBottomContentPart` addition above), since `FixRaidenBoss2/__init__.py` still does
  an unconditional `from .core import ...` at module load, so the same Bash/Git-Bash DLL quirk
  applies to importing `FRB` at all, not just to a freshly-rebuilt native extension specifically.

## `getKeyMissingParts`: one placement per graph, not one per section

`RegFillMissing` sits on `IniSectionGraph::getKeyMissingParts`, and that function is much less naive
than its name suggests. **It does not return "every part lacking the key".** Understanding it is the
difference between one draw call and three.

### The bubble-up

`IfTemplate::getKeyMissingPartsNode` walks the tree and decides, per node:

```cpp
if (missingKeyChildrenTotal == childrenTotal) {
    return {result.empty() ? childrenResult : result, true};   // bubble up: offer MY part
}
return {childrenResult, false};                                // only SOME missing: those branches
```

- **only some branches missing it** -> fill those branches
- **all of them missing it** -> report `allBranchesMissing`, and the fill lands on the parent

That second case is what puts a re-issued `drawindexed` on the `TextureOverride`, after the whole
`CommandList` (and anything it set up) has run.

Three things about it have each been wrong in this codebase, all fixed on 2026-09-08:

1. **The guard used to require the node to own a content part.** `if (!result.empty() && ...)`. A
   section ending on an `endif`, with nothing at root level after it, could therefore never report
   `allBranchesMissing` -- so a fill dropped into its branches while its sibling sections bubbled
   correctly. It looked like a per-character quirk and was one rule. Whether a node owns a part
   decides *which* parts to offer, never whether the subtree is missing the key.

2. **An external `run` target must count as a missing leaf.** `childrenTotal` counts every `run`
   target, but the walk skips names that are not sections in this graph -- `CommandList\TexFx\...`,
   `CommandList\global\ORFix\NNFix`. Skipping without counting made `missingKeyChildrenTotal ==
   childrenTotal` unreachable, so **any section calling an external library could never bubble up**.
   The pure-Python original's own comment names the right treatment: such a name is "a sink in the
   command call graph and a leaf in the DFS tree", and a leaf holds no occurrence of the key.

3. **`getKeyMissingParts` reports every section it visited**, including ones whose verdict a parent
   then superseded by bubbling up. Filling all of them gives one placement *per section* instead of
   per graph -- a second, redundant addition, which for a draw call is a second draw. Use
   **`targetsGetKeyMissingParts`** (the roots/targets-only view, mirroring the pure-Python
   `targetsGetKeyMissingParts`) for anything that wants one placement per graph. `RegFillMissing`
   does.

**If you write a new consumer, pick deliberately between the two.** `getKeyMissingParts` is still
right for downloads, which are placed per section.

## `RegDelimitedAddMode::PerBindingGeneration`: the rule `PerPath` approximates (2026-09-22)

`PerPath` assumes an addition is invalidated only by its **delimiter**. The fix libraries are
invalidated by something else too: they re-slot the registers bound when they run, so **re-binding
one starts a new generation** that needs its own call, while a second call over registers that have
NOT moved undoes the first. One call per path is therefore too few for a section that binds twice
and too many for nothing. `RegDelimitedAddMode.h` had carried a note about exactly this since
2026-09-14 ("a Citlali mod found since does bind, `ORFix`, draw, bind again, `ORFix`, draw"); this
is that note closed.

Three register maps, which is the whole model:

* **`invalidatorRegs`** opens a generation (the `ps-t` registers)
* **`coveredRegs`** is a generation already served -- **what the MOD wrote for itself**. A carried
  section may already call the library over its own bindings, and that call is the author's
  placement: keeping it and adding none is right, adding one beside it is the double call the mode
  exists to avoid
* **`delimiterRegs`** consumes it (the draw), where the addition goes, as late as possible

Empty `invalidatorRegs` makes it exactly `PerPath`, which is the test that says the mode is not
silently the old one -- `test_RegDelimitedAdd.py` runs the SAME fixture both ways and requires
`PerPath` to leave the second generation unserved.

**Implementation note, if you extend it.** `editPerPath`'s unit is the call-graph NODE, and that
cannot express two generations inside one part (bind, draw, bind, draw is one node). `editPerGeneration`
splits the two concerns instead: one ordered pass of the three maps **within** each part, which is
what places several additions in one part, plus a fixpoint over the call graph carrying a single
bool ("a generation is live and uncovered here") **between** parts. The fixpoint is a MUST analysis
like the two in `editPerPath`, optimistic and only ever cleared, so a `run =` cycle converges
instead of needing a special case. The same `walkPart` closure serves the analysis and the
insertion pass, so the two cannot disagree about where a generation ends.

**What NOT to do with it:** do not switch an existing `PerPath` caller over. Every compiled
character's `PerPath` placement is verified in game, and the two modes agree only while no section
rebinds. The merge template uses the new mode for CARRIED sections only, where the mod's own
bindings and its own calls both survive; its other call site (bindings replaced, calls stripped)
stays `PerPath`.

## Mandatory vs optional additions: which edit to reach for

Two register-keyed "add a `run =` call" jobs that look identical and are not:

- **Mandatory, keyed on a delimiter** -- `RegDelimitedAdd`. Cut every execution path at each
  delimiter; every delimiter-free segment holds the addition once, as late as possible.
  **`pathEndOnlyWhenUndelimited`** (default `false`, so no existing caller moved) narrows the last
  segment: with it, the end-of-path addition is made only for a path that never delimits at all.
  Needed because a surplus call is not always harmless -- `ORFix` swaps the diffuse and lightmap
  registers on every call. The condition must consult callees, since a part can end a path having
  delimited nothing itself while the section it ran delimited before returning.

- **Optional, keyed on the registers it reads** -- `RegSurroundedAdd` with **`optBeforeRegs`**, an
  any-of "at least one of these must come before" group. Empty group -> no addition at all, which is
  the whole point for an opt-in library. Reach for this before inventing a gate on
  `RegDelimitedAdd`; one was written and thrown away when `optBeforeRegs` turned out to say it
  already.

**Before changing the placement rule of any of these, read its `test_Xxx.py` first.** They state
their invariants deliberately in a header comment, and `test_RegDelimitedAdd.py`'s "every
delimiter-free segment ... last delimiter -> end of path" is a specification, not an accident. A
change to that rule went in twice and was reverted twice before the third attempt made it opt-in.

## Three edits the multi-component fixers grew, promoted to modules (2026-09-16)

Each started as a class in a fixer's anonymous namespace and each is general, so each is now a
proper module with a `pybind11` binding, a `test_Xxx.py` and live Sphinx entries -- **the
maintainer's standing rule for anything added to `regEdits/`, `graphEdits/` or `graphGroupEdits/`:
all three surfaces, not the core class alone.** Name them the way the family is named, noun first
(`GraphGroupRemove`, not `RemoveGraphGroup`).

- **`RegRestrict`** (`regEdits/`) -- restricts the registers a part binds to an allowed list, over the
  keys a `keyFilter` governs, and keeps only the FIRST binding of a repeated one. For a remapped
  section, whose registers come from the SOURCE's slot and whose surplus the target reads as
  something else. **Per part, not per section**: a merged master binds `ps-t2` once per branch, and
  those are separate paths, not one register bound twice.
- **`RegBranchAdd`** (`graphEdits/`) -- adds `KVPs` inside a conditional branch, the entries decided
  from the `Z3Predicate` the part runs under: `branchOf(query, iterData)` answers
  `(key, additions[, replacements])` or `None`. Replacements SET a key the branch already carries (a
  blend's `draw`), where an addition would draw twice. It sat unbound for a day after it landed; its
  tests build their graphs with `z3Ctx = ...`, without which a part outside every `if` has no query
  and the edit raises.
- **`GraphGroupRemove`** (`graphGroupEdits/`) -- removes whole groups, or all of them. A fixer that
  gives up must remove every group: `GIMIFixer` renders every graph the parser handed it whether or
  not an edit touched it, so "no edits" means the mod's own sections written again after the remap
  header, under the source's names, which the remover cannot strip.

## `GraphInherit` composes a graph INTO another, and an `adder` decides where (2026-09-18)

Asked for as a new `GraphCompose` edit, and built onto `GraphInherit` instead at the maintainer's
choice: `GraphInherit` already inserted `<reg> = <each root of dst>` into `src`, which is all
"compose" means, and nothing but tests used it. **Before building a graph-group edit, grep this
family for one whose `reg` is a parameter** -- `reg = run` is only the default reading of the name.

The use case is WuWa. A WuWa `TextureOverrideComponentN` binds no `ps-t` at all; each texture is its
own `TextureOverride<Obj>Diffuse` with `this = Resource<Obj>Diffuse`. Composing the resource graph
into the component with `reg = ps-t0` gives it the GI shape, `ps-t0 = Resource<Obj>Diffuse`. So
`dst` is the graph rooted at the **resource** section: its root names are the values.

- **`adder(srcGraph, kvps, ini, modType, modName)`** returns a `BaseRegEdit` / `BaseIniGraphEdit`
  built from the KVPs, or `None` having inserted them itself. The KVPs exist only at edit time, so
  the edit cannot be handed over pre-built -- that is why it is a factory, not an edit argument.
- **The returned edit runs exactly as a `GraphGroupEdit` would run it on `src`**, with `partFilter` as
  its key filter (`GraphGroupEdit::editSectionGraph` + the `RegPartEdit`/`GraphPartEdit` adapters). So
  a `RegAdd` with **no** `partFilter` lands in EVERY part, branches included, which is almost never
  what a register binding wants. The WuWa recipe is `RegAdd(kvps, latest = False)` with a filter
  selecting the part holding `hash`, after its last `hash`/`match_*`/`ps-t` key -- counting `ps-t`
  so a second compose (`ps-t1`) lands after the first rather than before it. It is pinned against
  the real section in `test_GraphInherit.py` and in `core/tests/GraphInherit_Adder_test.cpp`.
- **The two halves are tested in two places, on purpose.** From Python the binding runs the returned
  edit through a real Python `GraphGroupEdit` (so a pure-Python subclass's own `edit` runs), which
  means the core's variant dispatch is unreachable from the Python suite. The C++ test covers it, and
  was checked by mutation: dropping the key filter, or the replacement of `src` by a graph a graph
  edit returns, each fails it.
- **`GraphInherit` now has an `editFromIni`** (core override + binding). Before, the base forwarded to
  `edit` and the `.ini` never reached `partFilter`. The binding publishes the `.ini` on the instance
  for the forwarded `self.attr("edit")` call, the `PyRegFillMissing::currentIni` arrangement.

## When adding a new graph-editing feature

- Check `IniSectionGraph`/`CallGraph`/`GraphTools` first for a primitive that already does what
  you need before writing a private helper inside your own edit class — that's the whole point of
  where this subsystem ended up after starting as `RegSurroundedAdd`-only private methods.
- If your feature needs genuinely new graph-algorithm machinery (not just register-availability
  facts), put the `.ini`/`IfContentPart`-specific graph-*building* logic on `IniSectionGraph`, and
  any generic, node-type-agnostic *algorithm* on `GraphTools` — that split (domain-specific graph
  construction vs. domain-agnostic graph algorithms) is deliberate, not incidental, and keeps
  `GraphTools` reusable for a future analysis that isn't about `.ini` files at all.
- Verify a new cyclic-graph scenario **empirically** (a throwaway script under the session
  scratchpad, run via the PowerShell tool — see [Building](../Building/CLAUDE.md)'s note on why
  Bash fails to import the native `core` extension) before writing the formal unit test and before
  trusting your own hand-derivation of what the fixpoint *should* produce — this subsystem's own
  development repeatedly found that manual dataflow-equation tracing missed edge cases (the
  reachability gotcha above; the "which of two valid positions is more natural" gotcha above) that
  only surfaced by actually running the code against a constructed cyclic graph.
