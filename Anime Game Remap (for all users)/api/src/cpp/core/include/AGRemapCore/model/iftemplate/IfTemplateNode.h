#ifndef AGRemapCore_IfTemplateNode_H
#define AGRemapCore_IfTemplateNode_H

#include <cstddef>
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/tools/nodes/Node.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`Node`\\<size_t\\>

     A node within the parse tree of some `IfTemplate` -- the C++ port of ``IfTemplateNode.py``.
     This node contains a subset of the :cpp:class:`IfContentPart`\\<K, V, KeyHash, KeyEqual\\> from
     the original `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

     Unlike the pure-Python original (whose id is a random 128-bit ``uuid.uuid4().int``), this
     class's id is a plain, sequentially-generated ``size_t`` -- the same
     :cpp:class:`IncIdGenerator`\\<size_t\\> shape :cpp:class:`IfTemplatePart` itself already uses.
     Nothing depends on a node's id being random or non-monotonic: it's only ever used as a key
     into #children *within one tree*, never compared across trees, so this is a safe
     simplification rather than a literal port of the original's id scheme :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Neither this node nor #children own anything -- every node reachable from one
        `IfTemplate`'s parse tree (including this one) is really owned by the
        :cpp:class:`IfTemplateTree` that built it (a flat node pool), the same way none of this
        node's #parts are owned by it either (those are owned by the owning `IfTemplate`'s own
        parts list). This mirrors the pure-Python original's actual memory model directly -- plain
        reference-counted objects with no ownership hierarchy of their own, just a web of
        cross-references -- more faithfully than an owning-tree design would, and it sidesteps a
        real C++-specific problem an owning design hits: a node being built during tree
        construction can move in and out of a working stack many times before it ever finds a
        permanent parent (see :cpp:class:`IfTemplateTree`'s own construction algorithm), which has
        no single natural owner until the whole tree is finished -- and, concretely, a
        ``unique_ptr``-valued ``children`` map makes this class non-copy-constructible in a way
        that trips a hard (non-SFINAE) error inside MSVC's STL the moment anything (including
        `pybind11`_ itself, just registering the type) asks whether this class is copyable.
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyHash``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyEqual``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IfTemplateNode : public Node<size_t> {
        public:
            /**
             * @brief The concrete :cpp:class:`IfContentPart` type this node's #parts can reference
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             One element of #parts -- either a (non-owning) reference to an :cpp:type:`ContentPart`,
             or a (non-owning) reference to a child :cpp:class:`IfTemplateNode`
             @endrst
             */
            using PartsElement = std::variant<ContentPart*, IfTemplateNode<K, V, KeyHash, KeyEqual>*>;

            /**
             * @brief Constructs a new node
             *
             * @param id
             @rst
             The id for the node. If not provided, a new id is generated (see #generateId)
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param ifPredPart
             @rst
             The predicate part associated with this node, if any -- not owned by this node
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit IfTemplateNode(const std::optional<size_t>& id = std::nullopt, IfPredPart* ifPredPart = nullptr);

            /**
             * @brief The predicate part associated with this node, if any (not owned)
             */
            IfPredPart* ifPredPart;

            /**
             * @brief Generates a new id for a node (a fresh, sequential ``size_t``)
             */
            static size_t generateId();

            /**
             * @brief
             @rst
             Adds a (non-owning) reference to a child to this node
             @endrst
             *
             * @param node The child to reference -- not owned by this node (see this class's own
             *      top-level note on ownership)
             */
            void addChild(IfTemplateNode<K, V, KeyHash, KeyEqual>* node);

            /**
             * @brief
             @rst
             Adds a reference to an :cpp:type:`ContentPart` to this node -- 'part' is not owned by
             this node
             @endrst
             *
             * @param part The content part to reference
             */
            void addIfContentPart(ContentPart* part);

            /**
             * @brief The children of this node (not owned by this node -- see this class's own top-level note)
             */
            const std::unordered_map<size_t, IfTemplateNode<K, V, KeyHash, KeyEqual>*>& children() const;

            /**
             * @brief
             @rst
             The parts of the owning `IfTemplate` within this node, in original ordering -- a mix
             of #ContentPart references and child :cpp:class:`IfTemplateNode` references, none
             owned by this node (see this class's own top-level note)
             @endrst
             */
            const std::vector<PartsElement>& parts() const;

            /**
             * @copydoc parts() const
             */
            std::vector<PartsElement>& parts();

            /**
             * @brief
             @rst
             Purely checks whether 'key' exists within #parts, without accounting for whether the
             key exists in other subcommands called by this node or other children nodes that have
             the key
             @endrst
             *
             * @param key The key to check
             * @return Whether the key exists
             */
            bool hasKey(const K& key) const;

            /**
             * @brief Retrieves the latest :cpp:type:`ContentPart` (in #parts) that contains 'key'
             *
             * @param key The key to find
             * @return The found part, or ``nullptr`` if not found
             */
            ContentPart* getKeyPart(const K& key) const;

            /**
             * @brief
             @rst
             Retrieves the latest value that corresponds to 'key', from :cpp:func:`getKeyPart`'s
             result :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             *
             * @param key The key to find
             * @return The found value, if available
             */
            std::optional<V> getKeyVal(const K& key) const;

            /**
             * @brief Retrieves all the corresponding values to a certain key within the node
             *
             * @param key The key to find
             * @return
             @rst
             All the corresponding values to the key in the node -- the outer elements are the
             values for each part in the node that contains 'key'; the inner elements are the
             different instances of the KVP within each part (index, value)
             @endrst
             */
            std::vector<std::vector<std::pair<long long, V>>> getKeyValues(const K& key) const;

            /**
             * @brief
             @rst
             Retrieves the first #ContentPart if 'key' is not found in this node, without
             accounting for the key being in any other subcommands or other children nodes
             @endrst
             *
             * @param key The key to find
             * @return
             @rst
             A pair containing: (1) the first part found if none of the #ContentPart\\s within the
             node contain the key (``nullptr`` otherwise), and (2) whether any #ContentPart was
             found within the node at all
             @endrst
             */
            std::pair<ContentPart*, bool> getKeyMissingPart(const K& key) const;

        private:
            std::vector<PartsElement> parts_;
            std::unordered_map<size_t, IfTemplateNode<K, V, KeyHash, KeyEqual>*> children_;
    };

}

#include "IfTemplateNode.tpp"

#endif
