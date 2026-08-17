# Ini Graph Editing

Conventions and gotchas for the Python-side subsystem that models `.ini` file structure as a
graph and edits it — `IniSectionGraph` (`api/src/py/FixRaidenBoss2/model/IniSectionGraph.py`),
`CallGraph` (`model/CallGraph.py`), `GraphTools` (`tools/GraphTools.py`), and the graph-editing
strategies under `model/strategies/iniFixers/graphEdits/` (`RegSurroundedAdd` is the deep, worked
example — read it alongside `Testing/Unit Tester/UnitTester/Tests/test_RegSurroundedAdd.py`,
which exercises every case below concretely). This file was authored from hands-on work building
this subsystem from scratch (the fixpoint/reachability redesign of `RegSurroundedAdd`, then
extracting the reusable pieces into `IniSectionGraph`/`GraphTools`/`CallGraph`) — it hasn't been
exercised as deeply for the *other* `.ini`-parsing subsystems (`GIMIFixer` family, the non-graph
parsers), so verify assumptions there rather than assuming this file covers them too. See
[Overview](../Overview/CLAUDE.md) for how this fits in the wider repo, and
[Testing](../Testing/CLAUDE.md) for the `IfContentPart` index-renumbering trap that bites
constantly when hand-building synthetic test graphs for this subsystem.

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
`IniSectionGraph.CallGraph` combined with `RegSurroundedAdd._computeKeyFacts` for the pattern, and
`test_IniSectionGraph.py`'s `test_buildCallGraph_selfReferencingRunCall_ownNodeAndExitNodeAreSeparateSelfLoops`
for a minimal, direct demonstration of the disconnected-component shape that causes this.

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
plain data-holder class in `model/` instead of returning a tuple — see `SectionIterData`/
`SectionIterQueryData` (`model/SectionIterData.py`) for the original precedent, and `CallGraph`
(`model/CallGraph.py`, holding `IniSectionGraph.buildCallGraph()`'s four return values plus the
`exitNodeOf` convenience method) for a case built specifically to replace a positional 4-tuple
mid-refactor. The convention: a bare `__init__` assigning attributes 1:1, with a docstring that
documents the same fields twice — once under `Parameters` (constructor-call framing) and once
under `Attributes` (post-construction framing) — matching every other class in this codebase, not
a dataclass/namedtuple.

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
