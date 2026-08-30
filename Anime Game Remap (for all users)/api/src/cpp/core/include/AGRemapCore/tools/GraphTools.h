#ifndef AGRemapCore_GraphTools_H
#define AGRemapCore_GraphTools_H

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Tools for handling with generic directed graphs, represented as adjacency lists (``node ->
     list of the nodes directly reachable from it``) -- the C++ port of ``GraphTools.py``
     :raw-html:`<br />` :raw-html:`<br />`

     Nodes can be any type satisfying ``NodeHash``/``NodeEqual`` as a hash-map key -- these tools
     have no notion of what a node "is" (a `section`_, an :cpp:class:`IfContentPart`, a plain
     ``std::string``, ...); that meaning is entirely up to the caller. See
     :cpp:func:`IniSectionGraph::buildCallGraph` for how a `call graph`_ suitable for these tools
     gets built out of an actual `.ini`_ file's `sections`_
     @endrst
     */
    class GraphTools {
        public:

            /**
             * @brief
             @rst
             Computes every node reachable from 'rootNodes', via plain forward graph reachability
             (`BFS`_/`DFS`_ -- no dataflow facts involved, just "is there some path here at all") :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This is the tool that grounds #runForwardMustFixpoint/#runBackwardMustFixpoint's
                otherwise-optimistic `fixpoint iteration`_ against nodes that can provably never be
                reached at all -- see #clampFactsToReachable
             @endrst
             *
             * @tparam Node The node type
             * @tparam NodeHash A hasher for ``Node``. Defaults to ``std::hash<Node>``
             * @tparam NodeEqual An equality comparator for ``Node``. Defaults to ``std::equal_to<Node>``
             *
             * @param forwardEdges The graph to search, as ``node -> list of the nodes directly reachable from it``
             * @param rootNodes The nodes to start searching from
             *
             * @return Every node reachable from 'rootNodes' (including 'rootNodes' themselves)
             */
            template <typename Node, typename NodeHash = std::hash<Node>, typename NodeEqual = std::equal_to<Node>>
            static std::unordered_set<Node, NodeHash, NodeEqual> getReachableNodes(const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& forwardEdges,
                                                                                     const std::unordered_set<Node, NodeHash, NodeEqual>& rootNodes);

            /**
             * @brief
             @rst
             Forces every fact about a node not in 'reachableNodes' down to ``false`` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A node with no path back to any true root (eg. a virtual "continuation" node for a
                call that never returns, because every path out of the callee always loops back
                around with no escape) keeps whatever optimistic starting value
                #runForwardMustFixpoint/#runBackwardMustFixpoint gave it forever, since nothing in a
                disconnected component ever forces it to change -- a vacuous, unsound "fact" about a
                position that can provably never even execute. Always pair a `fixpoint`_ result with
                #getReachableNodes through this method before trusting it
             @endrst
             *
             * @tparam Node The node type
             * @tparam NodeHash A hasher for ``Node``. Defaults to ``std::hash<Node>``
             * @tparam NodeEqual An equality comparator for ``Node``. Defaults to ``std::equal_to<Node>``
             *
             * @param facts The raw facts to clamp, as returned by #runForwardMustFixpoint/#runBackwardMustFixpoint
             * @param reachableNodes See #getReachableNodes
             *
             * @return The clamped facts
             */
            template <typename Node, typename NodeHash = std::hash<Node>, typename NodeEqual = std::equal_to<Node>>
            static std::unordered_map<Node, bool, NodeHash, NodeEqual> clampFactsToReachable(const std::unordered_map<Node, bool, NodeHash, NodeEqual>& facts,
                                                                                                const std::unordered_set<Node, NodeHash, NodeEqual>& reachableNodes);

            /**
             * @brief
             @rst
             Runs a forward, `MUST`_ (`available expressions`_-style) `dataflow analysis`_ over a
             graph, computing whether some boolean property has been established entering every
             node -- correctly handling cycles via `fixpoint iteration`_ (`Kildall's algorithm`_/
             `worklist algorithm`_), rather than assuming a single forward pass ever "finishes" with
             a graph that may not have an end at all :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A node is only ever *forced* unsatisfied (the boundary condition) if it's in
                'rootNodes', or if it has no predecessors at all (per 'backwardEdges') --
                everywhere else starts optimistic (already satisfied) and gets refined downward by
                the `worklist algorithm`_ as actual predecessors are discovered not to satisfy it,
                converging once nothing changes anymore :raw-html:`<br />` :raw-html:`<br />`

                A cyclic, unreachable-from-any-root component (eg. a call that never returns) can
                end up keeping its optimistic value forever, since nothing ever forces it down from
                within an isolated cycle -- pair this result with #getReachableNodes/
                #clampFactsToReachable before trusting a fact about such a node
             @endrst
             *
             * @tparam Node The node type
             * @tparam NodeHash A hasher for ``Node``. Defaults to ``std::hash<Node>``
             * @tparam NodeEqual An equality comparator for ``Node``. Defaults to ``std::equal_to<Node>``
             *
             * @param forwardEdges The graph to analyze, as ``node -> list of the nodes that can run directly after it``
             * @param backwardEdges The reverse of 'forwardEdges'
             * @param rootNodes The nodes that are true entry points of the graph (forced unsatisfied on entry, regardless of 'backwardEdges')
             * @param localFacts
             @rst
             For every node with content of its own worth examining (as opposed to a purely
             pass-through, virtual node), a pair containing:

             #. Whether the node's own content touches the property being tracked at all
             #. Whether the property is still satisfied by the very end of the node's own content
                (only meaningful if the first value is ``true``)

             A node missing from this map is treated as a pure pass-through (its own value is
             whatever was true entering it)
             @endrst
             *
             * @return Every node reachable in the graph structure, mapped to whether the property is satisfied entering that node
             */
            template <typename Node, typename NodeHash = std::hash<Node>, typename NodeEqual = std::equal_to<Node>>
            static std::unordered_map<Node, bool, NodeHash, NodeEqual> runForwardMustFixpoint(const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& forwardEdges,
                                                                                                 const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& backwardEdges,
                                                                                                 const std::unordered_set<Node, NodeHash, NodeEqual>& rootNodes,
                                                                                                 const std::unordered_map<Node, std::pair<bool, bool>, NodeHash, NodeEqual>& localFacts);

            /**
             * @brief
             @rst
             The mirror of #runForwardMustFixpoint: a backward `MUST`_ (`very busy expressions`_-style)
             `dataflow analysis`_ over the same kind of graph, computing whether some boolean
             property is *guaranteed* to be established somewhere after every node exits -- again
             via `fixpoint iteration`_ so a cyclic graph resolves correctly (eg. a property only
             established after looping back around) :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A node is only ever *forced* unsatisfied (the boundary condition) if it has no
                outgoing edges at all (per 'forwardEdges') -- a genuine dead end, nothing can
                possibly run after it. Everywhere else starts optimistic and gets refined downward
                as actual successors are discovered not to guarantee it :raw-html:`<br />` :raw-html:`<br />`

                Same caveat as #runForwardMustFixpoint applies for unreachable, isolated cyclic
                components -- pair this result with #getReachableNodes/#clampFactsToReachable
             @endrst
             *
             * @tparam Node The node type
             * @tparam NodeHash A hasher for ``Node``. Defaults to ``std::hash<Node>``
             * @tparam NodeEqual An equality comparator for ``Node``. Defaults to ``std::equal_to<Node>``
             *
             * @param forwardEdges The graph to analyze, as ``node -> list of the nodes that can run directly after it``
             * @param backwardEdges The reverse of 'forwardEdges'
             * @param localFacts
             @rst
             For every node, whether its own content, by itself, already establishes the property
             being tracked (a plain existential check, eg. "does this node's own content contain
             *any* accepted occurence of X"). A node missing from this map is treated as ``false``
             (its own content alone establishes nothing)
             @endrst
             *
             * @return Every node reachable in the graph structure, mapped to whether the property is guaranteed to be satisfied somewhere after that node exits
             */
            template <typename Node, typename NodeHash = std::hash<Node>, typename NodeEqual = std::equal_to<Node>>
            static std::unordered_map<Node, bool, NodeHash, NodeEqual> runBackwardMustFixpoint(const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& forwardEdges,
                                                                                                  const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& backwardEdges,
                                                                                                  const std::unordered_map<Node, bool, NodeHash, NodeEqual>& localFacts);
    };
}

#include "GraphTools.tpp"

#endif
