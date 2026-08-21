#ifndef AGRemapCore_ParseTree_H
#define AGRemapCore_ParseTree_H

#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/tools/nodes/ParseNode.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The generated parse tree after parsing some text
     @endrst
     *
     * @tparam Id The type for the id of a node in the tree
     * @tparam IdHash The hash function for ``Id``
     * @tparam IdEq The equality function for ``Id``
     */
    template <typename Id, typename IdHash = std::hash<Id>, typename IdEq = std::equal_to<Id>>
    class ParseTree {
        public:

            /**
             * @brief
             @rst
             The nodes in the tree :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids of the node and the values are the nodes
             @endrst
             */
            using Nodes = std::unordered_map<Id, ParseNode<Id>, IdHash, IdEq>;

            /**
             * @brief
             @rst
             The children relations of the nodes :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids of the parent nodes and the values are the ids of the children nodes
             @endrst
             */
            using Children = std::unordered_map<Id, std::vector<Id>, IdHash, IdEq>;

            /**
             * @brief Constructs a new parse tree
             *
             * @param nodes The nodes in the tree, keyed by the id of each node
             * @param children The children relations of the nodes
             * @param rootId The id of the root node
             *
             * @throws std::invalid_argument If 'rootId' does not reference an existing node in 'nodes'
             */
            ParseTree(Nodes nodes, Children children, Id rootId);

            /**
             * @brief
             @rst
             The nodes in the tree :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids of the node and the values are the nodes
             @endrst
             */
            Nodes nodes;

            /**
             * @brief
             @rst
             The children relations of the nodes :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids of the parent nodes and the values are the ids of the children nodes
             @endrst
             */
            Children children;

            /**
             * @brief The id of the root node
             */
            const Id& rootId() const;

            /**
             * @brief Sets the new id of the root node
             *
             * @param newRootId The new id of the root node
             *
             * @throws std::invalid_argument If 'newRootId' does not reference an existing node in #nodes
             */
            void setRootId(Id newRootId);

            /**
             * @brief Retrieves a node based on the passed id
             *
             * @param nodeId The node id to search for
             * @param errorOnNotFound Whether to throw if no matching node is found
             *
             * @note
             @rst
             Unlike the pure-Python original's ``getNode``, there is no separate 'default' parameter
             for the not-found-and-not-erroring case -- a ``nullptr`` return already serves as that
             default, and none of this codebase's real call sites ever pass a custom one (matching
             :cpp:class:`BaseDFA`'s own ``get*Ptr`` accessors, which have the same shape)
             @endrst
             *
             * @throws std::out_of_range If no matching node is found and 'errorOnNotFound' is ``true``
             *
             * @return A pointer to the corresponding node, or ``nullptr`` if none is found and
             *      'errorOnNotFound' is ``false`` -- valid only as long as #nodes isn't subsequently
             *      mutated
             */
            const ParseNode<Id>* getNode(const Id& nodeId, bool errorOnNotFound = true) const;

            /**
             * @brief
             @rst
             Determines whether the id of some node has no children of its own :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Ported as-is from the pure-Python original's own ``isChild``, name and all -- despite
                the name, this checks whether 'nodeId' has *no entry* in #children (i.e. it's a leaf/
                token node produced by a shift, as opposed to an internal node produced by reducing a
                production), not whether it appears as some other node's child
             @endrst
             *
             * @param nodeId The id of the node
             *
             * @return Whether 'nodeId' exists in #nodes and has no children of its own
             */
            bool isChild(const Id& nodeId) const;

        private:
            Id rootId_;

            void validateRootId(const Id& newRootId) const;
    };
}

#include "ParseTree.tpp"

#endif
