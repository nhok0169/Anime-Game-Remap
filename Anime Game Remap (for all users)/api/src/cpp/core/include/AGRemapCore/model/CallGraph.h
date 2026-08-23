#ifndef AGRemapCore_CallGraph_H
#define AGRemapCore_CallGraph_H

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfContentPart.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The result of `IniSectionGraph::buildCallGraph` -- a `call graph`_ over the
     :cpp:class:`IfContentPart`\\s of an `IniSectionGraph`, suitable for the `dataflow analysis`_
     tools at `GraphTools` -- the C++ port of ``CallGraph.py`` :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Unlike the pure-Python original (whose nodes are either ``id(part)`` or
        ``("exit", id(part))``, using CPython's own built-in ``id()`` as a stand-in for part
        identity), this class's nodes are #Node -- a real ``(part pointer, isExit)`` pair. The
        `pybind11`_ binding layer is responsible for translating a #Node into whatever shape
        `Python`_ callers expect (an ``id(part)``-equal integer, or an ``("exit", ...)`` tuple) --
        see that binding's own top-level note for why this translation has to happen there, not
        here (real callers like ``RegSurroundedAdd.py`` correlate a part they already hold via
        `Python`_'s own builtin ``id()``, which this generic core has no way to compute itself).
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyHash``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyEqual``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class CallGraph {
        public:
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             A node in the `call graph`_ -- either a real #ContentPart (``isExit == false``), or
             the virtual "continue here once this part's own ``run =`` call has returned" node
             (``isExit == true``) -- see :cpp:func:`exitNodeOf`
             @endrst
             */
            struct Node {
                /**
                 * @brief The part this node represents (never ``nullptr``)
                 */
                ContentPart* part;

                /**
                 * @brief Whether this is the "exit" (post-call-return) node for #part
                 */
                bool isExit = false;

                bool operator==(const Node& other) const;
            };

            /**
             * @brief Hasher for #Node, for use as an ``unordered_map``/``unordered_set`` key
             */
            struct NodeHash {
                size_t operator()(const Node& node) const;
            };

            using EdgeMap = std::unordered_map<Node, std::vector<Node>, NodeHash>;

            /**
             * @brief Constructs a new `call graph`_
             *
             * @param forwardEdges ``node -> list of the nodes that can run directly after it``
             * @param backwardEdges The reverse of 'forwardEdges'
             * @param parts Every #ContentPart reachable in the graph this was built from
             * @param rootNodes Every #ContentPart that's a genuine entry point of one of the graph's own target `section`_\\s
             * @param runKey The `.ini`_ ``run`` keyword, as a ``K`` -- see :cpp:func:`exitNodeOf`
             */
            explicit CallGraph(EdgeMap forwardEdges, EdgeMap backwardEdges,
                                std::unordered_set<ContentPart*> parts, std::unordered_set<ContentPart*> rootNodes,
                                K runKey);

            /**
             * @brief ``node -> list of the nodes that can run directly after it``
             */
            const EdgeMap& forwardEdges() const;

            /**
             * @brief The reverse of #forwardEdges
             */
            const EdgeMap& backwardEdges() const;

            /**
             * @brief Every #ContentPart reachable in the graph this was built from
             */
            const std::unordered_set<ContentPart*>& parts() const;

            /**
             * @brief Every #ContentPart that's a genuine entry point of one of the graph's own target `section`_\\s
             */
            const std::unordered_set<ContentPart*>& rootNodes() const;

            /**
             * @brief
             @rst
             Retrieves the node representing "once 'part's own ``run =`` call (if it makes one) has
             returned" :raw-html:`<br />` :raw-html:`<br />`

             For a part that makes no call, this is just ``{part, false}`` -- there's no call to
             distinguish "before" from "after", so the part's own node already serves both purposes
             @endrst
             *
             * @param part The part to look up
             * @return Either ``{part, true}`` (if the part makes a ``run =`` call) or ``{part, false}``
             */
            Node exitNodeOf(ContentPart* part) const;

        private:
            EdgeMap forwardEdges_;
            EdgeMap backwardEdges_;
            std::unordered_set<ContentPart*> parts_;
            std::unordered_set<ContentPart*> rootNodes_;
            K runKey_;
    };

}

#include "CallGraph.tpp"

#endif
