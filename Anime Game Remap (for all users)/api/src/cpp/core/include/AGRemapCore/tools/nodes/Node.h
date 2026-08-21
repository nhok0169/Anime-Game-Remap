#ifndef AGRemapCore_Node_H
#define AGRemapCore_Node_H

#include <utility>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class for a node in a `graph`_ :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Deliberately has no ``operator==``/hash support, letting a ``Node`` itself be used as a
        dict/set key -- nothing in this codebase's real call sites does that (a node's *id* is
        used as the key, e.g. :cpp:class:`ParseTree`'s ``nodes``/``children`` maps -- the node
        itself never is), and #Id isn't guaranteed hashable in the first place without a
        caller-supplied hasher this class doesn't otherwise need. Add a ``NodeHash``/``NodeEqual``
        pair the same way :cpp:class:`BaseDFA` does for its states if a future caller genuinely
        needs a ``Node`` itself as a key
     @endrst
     *
     * @tparam Id The type for the id of the node
     */
    template <typename Id>
    class Node {
        public:

            /**
             * @brief Constructs a new node
             *
             * @param id The id for the node
             */
            explicit Node(Id id);

            virtual ~Node() = default;

            /**
             * @brief The id of the node
             */
            const Id& id() const;

        protected:

            /**
             * @brief The id of the node
             */
            Id id_;
    };
}

#include "Node.tpp"

#endif
