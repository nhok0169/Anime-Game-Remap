##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### ExtImports
from collections import deque
from typing import Any, Dict, List, Set, Tuple
##### EndExtImports


##### Script
class GraphTools():
    """
    Tools for handling with generic directed graphs, represented as adjacency lists (``Dict[node, List[node]]``) :raw-html:`<br />` :raw-html:`<br />`

    Nodes can be any hashable value -- these tools have no notion of what a node "is" (a `section`_, an
    :class:`IfContentPart`, a plain :class:`str`, ...); that meaning is entirely up to the caller. See
    :meth:`IniSectionGraph.buildCallGraph` for how a `call graph`_ suitable for these tools gets built out of
    an actual .ini file's `sections`_
    """

    @classmethod
    def getReachableNodes(cls, forwardEdges: Dict[Any, List[Any]], rootNodes: Set[Any]) -> Set[Any]:
        """
        Computes every node reachable from 'rootNodes', via plain forward graph reachability (`BFS`_/`DFS`_ --
        no dataflow facts involved, just "is there some path here at all")

        .. note::
            This is the tool that grounds :meth:`runForwardMustFixpoint`/:meth:`runBackwardMustFixpoint`'s
            otherwise-optimistic `fixpoint iteration`_ against nodes that can provably never be reached at all --
            see :meth:`clampFactsToReachable`

        Parameters
        ----------
        forwardEdges: Dict[Any, List[Any]]
            The graph to search, as ``node -> list of the nodes directly reachable from it``

        rootNodes: Set[Any]
            The nodes to start searching from

        Returns
        -------
        Set[Any]
            Every node reachable from 'rootNodes' (including 'rootNodes' themselves)
        """

        reachable: Set[Any] = set(rootNodes)
        stack: List[Any] = list(rootNodes)

        while (stack):
            node = stack.pop()
            for succ in forwardEdges.get(node, []):
                if (succ not in reachable):
                    reachable.add(succ)
                    stack.append(succ)

        return reachable

    @classmethod
    def clampFactsToReachable(cls, facts: Dict[Any, bool], reachableNodes: Set[Any]) -> Dict[Any, bool]:
        """
        Forces every fact about a node not in 'reachableNodes' down to ``False``

        .. note::
            A node with no path back to any true root (eg. a virtual "continuation" node for a call that never
            returns, because every path out of the callee always loops back around with no escape) keeps
            whatever optimistic starting value :meth:`runForwardMustFixpoint`/:meth:`runBackwardMustFixpoint`
            gave it forever, since nothing in a disconnected component ever forces it to change -- a vacuous,
            unsound "fact" about a position that can provably never even execute. Always pair a `fixpoint`_
            result with :meth:`getReachableNodes` through this method before trusting it

        Parameters
        ----------
        facts: Dict[Any, :class:`bool`]
            The raw facts to clamp, as returned by :meth:`runForwardMustFixpoint`/:meth:`runBackwardMustFixpoint`

        reachableNodes: Set[Any]
            See :meth:`getReachableNodes`

        Returns
        -------
        Dict[Any, :class:`bool`]
            The clamped facts
        """

        return {node: (value and node in reachableNodes) for node, value in facts.items()}

    @classmethod
    def runForwardMustFixpoint(cls, forwardEdges: Dict[Any, List[Any]], backwardEdges: Dict[Any, List[Any]], rootNodes: Set[Any],
                               localFacts: Dict[Any, Tuple[bool, bool]]) -> Dict[Any, bool]:
        """
        Runs a forward, `MUST`_ (`available expressions`_-style) `dataflow analysis`_ over a graph, computing
        whether some boolean property has been established entering every node -- correctly handling cycles via
        `fixpoint iteration`_ (`Kildall's algorithm`_/`worklist algorithm`_), rather than assuming a single
        forward pass ever "finishes" with a graph that may not have an end at all

        .. note::
            A node is only ever *forced* unsatisfied (the boundary condition) if it's in 'rootNodes', or if it
            has no predecessors at all (per 'backwardEdges') -- everywhere else starts optimistic (already
            satisfied) and gets refined downward by the `worklist algorithm`_ as actual predecessors are
            discovered not to satisfy it, converging once nothing changes anymore :raw-html:`<br />` :raw-html:`<br />`

            A cyclic, unreachable-from-any-root component (eg. a call that never returns) can end up keeping its
            optimistic value forever, since nothing ever forces it down from within an isolated cycle -- pair
            this result with :meth:`getReachableNodes`/:meth:`clampFactsToReachable` before trusting a fact
            about such a node

        Parameters
        ----------
        forwardEdges: Dict[Any, List[Any]]
            The graph to analyze, as ``node -> list of the nodes that can run directly after it``

        backwardEdges: Dict[Any, List[Any]]
            The reverse of 'forwardEdges'

        rootNodes: Set[Any]
            The nodes that are true entry points of the graph (forced unsatisfied on entry, regardless of
            'backwardEdges')

        localFacts: Dict[Any, Tuple[:class:`bool`, :class:`bool`]]
            For every node with content of its own worth examining (as opposed to a purely pass-through, virtual
            node), a tuple containing:

                #. Whether the node's own content touches the property being tracked at all
                #. Whether the property is still satisfied by the very end of the node's own content (only
                   meaningful if the first value is ``True``) :raw-html:`<br />` :raw-html:`<br />`

            A node missing from this dict is treated as a pure pass-through (its own value is whatever was true
            entering it)

        Returns
        -------
        Dict[Any, :class:`bool`]
            Every node reachable in the graph structure, mapped to whether the property is satisfied entering
            that node
        """

        allNodes = set(forwardEdges.keys()) | set(backwardEdges.keys()) | set(localFacts.keys())
        roots = set(rootNodes) | {node for node in allNodes if (not backwardEdges.get(node))}

        def computeExit(node: Any, entryFact: Dict[Any, bool]) -> bool:
            fact = localFacts.get(node)
            if (fact is not None):
                touches, localSatisfied = fact
                return localSatisfied if (touches) else entryFact[node]
            return entryFact[node]

        entryFact = {node: (node not in roots) for node in allNodes}
        exitFact = {node: computeExit(node, entryFact) for node in allNodes}

        worklist = deque(allNodes)
        queued = set(allNodes)
        while (worklist):
            node = worklist.popleft()
            queued.discard(node)

            if (node not in roots):
                preds = backwardEdges.get(node, [])
                entryFact[node] = all(exitFact.get(pred, True) for pred in preds) if (preds) else entryFact[node]

            newExit = computeExit(node, entryFact)
            if (newExit != exitFact[node]):
                exitFact[node] = newExit
                for succ in forwardEdges.get(node, []):
                    if (succ not in queued):
                        worklist.append(succ)
                        queued.add(succ)

        return entryFact

    @classmethod
    def runBackwardMustFixpoint(cls, forwardEdges: Dict[Any, List[Any]], backwardEdges: Dict[Any, List[Any]],
                                localFacts: Dict[Any, bool]) -> Dict[Any, bool]:
        """
        The mirror of :meth:`runForwardMustFixpoint`: a backward `MUST`_ (`very busy expressions`_-style)
        `dataflow analysis`_ over the same kind of graph, computing whether some boolean property is
        *guaranteed* to be established somewhere after every node exits -- again via `fixpoint iteration`_ so a
        cyclic graph resolves correctly (eg. a property only established after looping back around)

        .. note::
            A node is only ever *forced* unsatisfied (the boundary condition) if it has no outgoing edges at all
            (per 'forwardEdges') -- a genuine dead end, nothing can possibly run after it. Everywhere else starts
            optimistic and gets refined downward as actual successors are discovered not to guarantee it :raw-html:`<br />` :raw-html:`<br />`

            Same caveat as :meth:`runForwardMustFixpoint` applies for unreachable, isolated cyclic components --
            pair this result with :meth:`getReachableNodes`/:meth:`clampFactsToReachable`

        Parameters
        ----------
        forwardEdges: Dict[Any, List[Any]]
            The graph to analyze, as ``node -> list of the nodes that can run directly after it``

        backwardEdges: Dict[Any, List[Any]]
            The reverse of 'forwardEdges'

        localFacts: Dict[Any, :class:`bool`]
            For every node, whether its own content, by itself, already establishes the property being tracked
            (a plain existential check, eg. "does this node's own content contain *any* accepted occurence of
            X"). A node missing from this dict is treated as ``False`` (its own content alone establishes nothing)

        Returns
        -------
        Dict[Any, :class:`bool`]
            Every node reachable in the graph structure, mapped to whether the property is guaranteed to be
            satisfied somewhere after that node exits
        """

        allNodes = set(forwardEdges.keys()) | set(backwardEdges.keys()) | set(localFacts.keys())
        terminals = {node for node in allNodes if (not forwardEdges.get(node))}

        def computeEntry(node: Any, exitFact: Dict[Any, bool]) -> bool:
            if (localFacts.get(node, False)):
                return True
            return exitFact[node]

        exitFact = {node: (node not in terminals) for node in allNodes}
        entryFact = {node: computeEntry(node, exitFact) for node in allNodes}

        worklist = deque(allNodes)
        queued = set(allNodes)
        while (worklist):
            node = worklist.popleft()
            queued.discard(node)

            if (node not in terminals):
                succs = forwardEdges.get(node, [])
                exitFact[node] = all(entryFact.get(succ, True) for succ in succs) if (succs) else exitFact[node]

            newEntry = computeEntry(node, exitFact)
            if (newEntry != entryFact[node]):
                entryFact[node] = newEntry
                for pred in backwardEdges.get(node, []):
                    if (pred not in queued):
                        worklist.append(pred)
                        queued.add(pred)

        return exitFact
##### EndScript
