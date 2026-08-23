#ifndef AGRemapCore_IfTemplateTree_H
#define AGRemapCore_IfTemplateTree_H

#include <memory>
#include <vector>

#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplateNode.h"
#include "AGRemapCore/model/iftemplate/IfTemplatePart.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The parse tree for some `IfTemplate` -- the C++ port of ``IfTemplateTree.py``'s
     ``IfTemplateTree`` class :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The parse tree for the `IfTemplate` is structured such that:

        * A node is composed of :cpp:class:`IfContentPart`\\<K, V, KeyHash, KeyEqual\\> or other nodes
        * The children to the node occur when the node enters a specific branching condition :raw-html:`<br />` :raw-html:`<br />`

        eg. Suppose we have this branching structure

        .. code-block:: ini
            :linenos:

            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                ...(does stuff)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                    ...(does stuff)...
                endif
                ...(does stuff)...
                if
                endif
            endif
            ...(does stuff)...

        :raw-html:`<br />`

        Let ``C`` be some :cpp:class:`IfContentPart` (the parts that say ``...(does stuff)...``)

        Let ``B`` be some branching point (the parts that say ``if`` or ``else``)

        Let ``[...]`` be some node

        Let ``X`` be a node without any parts

        The parse tree generated for the above code would be:

        .. code-block::

                   [C B B C]
                      | |
                 +----+ +----+
                 |           |
               [B B]     [C B C B]
                | |         |   |
             +--+ +--+    [B C] X
             |       |     |
            [C]     [C]   [C]
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyHash``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyEqual``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IfTemplateTree {
        public:
            /**
             * @brief The concrete node type this tree is built from
             */
            using NodeType = IfTemplateNode<K, V, KeyHash, KeyEqual>;

            IfTemplateTree() = default;
            virtual ~IfTemplateTree() = default;

            // Explicitly deleted/defaulted (not left implicit) -- #nodePool_ is a
            // vector<unique_ptr<NodeType>>, which makes this class naturally move-only. Declaring
            // that outcome explicitly, rather than leaving the compiler to work it out implicitly,
            // sidesteps a real MSVC STL issue: merely *instantiating* this class template (which
            // happens any time a derived construct() algorithm is instantiated, whether or not
            // anything ever actually copies a tree) otherwise triggers a hard, non-SFINAE error
            // from deep inside vector<unique_ptr<T>>'s own implicitly-checked copy constructor --
            // the same category of failure IfTemplateNode.h's own top-level note describes for a
            // near-identical reason, hit for real during this port.
            IfTemplateTree(const IfTemplateTree&) = delete;
            IfTemplateTree& operator=(const IfTemplateTree&) = delete;
            IfTemplateTree(IfTemplateTree&&) = default;
            IfTemplateTree& operator=(IfTemplateTree&&) = default;

            /**
             * @brief The root node in the parse tree, if any
             */
            NodeType* root() const;

            /**
             * @brief Clears the tree
             */
            void clear();

            /**
             * @brief
             @rst
             Constructs the parse tree, using this class's own (base) algorithm -- leaf nodes for
             an empty condition (eg. a bare ``if``/``endif`` with nothing between them) are left
             with no #ContentPart at all, unlike :cpp:class:`IfTemplateNonEmptyNodeTree`
             @endrst
             *
             * @param parts
             @rst
             The parts within the `IfTemplate`, owned by the caller (typically the owning
             `IfTemplate` itself) -- the returned tree's nodes hold non-owning references into
             this same vector's elements, which must outlive the returned tree
             @endrst
             *
             * @return The constructed tree
             */
            static std::unique_ptr<IfTemplateTree<K, V, KeyHash, KeyEqual>> construct(
                const std::vector<std::unique_ptr<IfTemplatePart>>& parts);

            /**
             * @brief
             @rst
             Allocates a new node, owned by this tree's own node pool, and returns a non-owning
             pointer to it -- public (not just for the construction algorithms below) since a
             caller like :cpp:class:`IfTemplate` legitimately needs to grow an already-built tree
             in place (eg. ensuring a root exists)
             @endrst
             *
             * @param ifPredPart The predicate part to associate with the new node, if any
             * @return The newly allocated node
             */
            NodeType* makeNode(IfPredPart* ifPredPart);

            /**
             * @brief Sets #root_ -- 'node' must already belong to this tree's own node pool
             */
            void setRoot(NodeType* node);

        protected:
            /**
             * @brief
             @rst
             Every :cpp:class:`IfTemplateNode` reachable from #root_ is really owned here (a flat
             pool), not by the tree structure itself -- see :cpp:class:`IfTemplateNode`'s own
             top-level note for why. A node is never removed from this pool individually; the
             whole pool is destroyed together when this tree is.
             @endrst
             */
            std::vector<std::unique_ptr<NodeType>> nodePool_;

            /**
             * @brief The root node, a non-owning pointer into #nodePool_
             */
            NodeType* root_ = nullptr;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IfTemplateTree`

     A variation of :cpp:class:`IfTemplateTree` such that leaf nodes that do not have any parts
     (eg. empty conditions) will include an empty #ContentPart placeholder -- the C++ port of
     ``IfTemplateTree.py``'s ``IfTemplateNonEmptyNodeTree`` class :raw-html:`<br />` :raw-html:`<br />`

     .. tip::
        See :cpp:class:`IfTemplateTree` on the basic structure of the parse tree for an `IfTemplate`

     :raw-html:`<br />` :raw-html:`<br />`

     So conditions with forms of:

     .. code-block:: ini

        if
        endif

     that have the following parse subtree:

     .. code-block::

        [B]
         |
         X

     will now become:

     .. code-block:: ini

        if
            ...(does nothing)...
        endif

     with the following parse subtree:

     .. code-block::

        [B]
         |
        [C]

     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        eg. Suppose we have this branching structure (same structure from the example at :cpp:class:`IfTemplateTree`)

        .. code-block:: ini
            :linenos:

            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                ...(does stuff)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                    ...(does stuff)...
                endif
                ...(does stuff)...
                if
                endif
            endif
            ...(does stuff)...

        :raw-html:`<br />`

        Let ``C`` be some :cpp:class:`IfContentPart` (the parts that say ``...(does stuff)...``)

        Let ``B`` be some branching point (the parts that say ``if`` or ``else``)

        Let ``[...]`` be some node

        Let ``X`` be a node without any parts

        The parse tree generated for the above code would be:

        .. code-block::

                   [C B B C]
                      | |
                 +----+ +----+
                 |           |
               [B B]     [C B C B]
                | |         |   |
             +--+ +--+    [B C] |
             |       |     |    |
            [C]     [C]   [C]  [C]
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyHash``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyEqual``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IfTemplateNonEmptyNodeTree : public IfTemplateTree<K, V, KeyHash, KeyEqual> {
        public:
            /**
             * @brief
             @rst
             Constructs the parse tree, inserting a fresh, empty #ContentPart (depth-appropriate)
             into 'parts' wherever a branch would otherwise end up with no #ContentPart at all
             @endrst
             *
             * @param parts
             @rst
             The parts within the `IfTemplate` -- taken by mutable reference (unlike the base
             class's own #IfTemplateTree::construct), since this algorithm may insert new,
             synthetic #ContentPart elements into it. Owned by the caller (typically the owning
             `IfTemplate`), which must outlive the returned tree
             @endrst
             *
             * @return The constructed tree
             */
            static std::unique_ptr<IfTemplateNonEmptyNodeTree<K, V, KeyHash, KeyEqual>> construct(
                std::vector<std::unique_ptr<IfTemplatePart>>& parts);
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IfTemplateNonEmptyNodeTree`

     A variation of :cpp:class:`IfTemplateNonEmptyNodeTree` such that an empty ``else`` clause will
     be added for branches that do not end with a single ``else`` -- the C++ port of
     ``IfTemplateTree.py``'s ``IfTemplateNormTree`` class :raw-html:`<br />` :raw-html:`<br />`

     .. tip::
        See :cpp:class:`IfTemplateTree` on the basic structure of the parse tree for an `IfTemplate`

     :raw-html:`<br />` :raw-html:`<br />`

     So conditions with forms of:

     .. code-block:: ini

        if
            ...(does stuff)...
        else if
            ...(does stuff)...
        endif

     that have the following parse subtree:

     .. code-block::

           [B B]
            | |
          +-+ +-+
          |     |
         [C]   [C]

     will now become:

     .. code-block:: ini

        if
            ...(does stuff)...
        else if
            ...(does stuff)...
        else
            ...(does nothing)...
        endif

     with the following parse subtree:

     .. code-block::

         [B B B]
          | | |
        +-+ | +-+
        |  [C]  |
       [C]     [C]

     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        eg. Suppose we have this branching structure (same structure from the example at :cpp:class:`IfTemplateTree`)

        .. code-block:: ini
            :linenos:

            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                endif
            else ...(bool)...
                ...(does stuff)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    endif
                    ...(does stuff)...
                endif
                ...(does stuff)...
                if
                endif
            endif
            ...(does stuff)...

        :raw-html:`<br />`

        This class will turn this branching structure into:

        .. code-block:: ini
            :linenos:

            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                else if ...(bool)...
                    ...(does stuff)...
                else
                    ...(does nothing)...
                endif
            else ...(bool)...
                ...(does stuff)...
                if ...(bool)...
                    if ...(bool)...
                        ...(does stuff)...
                    else
                        ...(does nothing)...
                    endif
                    ...(does stuff)...
                else
                    ...(does nothing)...
                endif
                ...(does stuff)...
                if
                    ...(does nothing)...
                else
                    ...(does nothing)...
                endif
            endif
            ...(does stuff)...

        Let ``C`` be some :cpp:class:`IfContentPart` (the parts that say ``...(does stuff)...``)

        Let ``B`` be some branching point (the parts that say ``if`` or ``else``)

        Let ``[...]`` be some node

        Let ``X`` be a node without any parts

        The parse tree generated for the above code would be:

        .. code-block::

                     [C B B C]
                        | |
                    +----+ +-------+
                    |              |
               [B B B]         [C B B C B B]
                | | |             | |   | |
             +--+ | +-+         +-+ +-+ | +--+
             |    |   |         |     | |    |
            [C]  [C] [C]     [B B C]  | +-+  |
                              | |     |   | [C]
                            +-+ |    [C]  |
                            |   |        [C]
                           [C] [C]
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyHash``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyEqual``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IfTemplateNormTree : public IfTemplateNonEmptyNodeTree<K, V, KeyHash, KeyEqual> {
        public:
            /**
             * @brief
             @rst
             Constructs the parse tree, additionally synthesizing an empty ``else`` branch (an
             #IfPredPart\\::Else\\ + empty #ContentPart pair, inserted into 'parts') for any
             conditional that doesn't already end with a single ``else``
             @endrst
             *
             * @param parts
             @rst
             Same meaning as :cpp:class:`IfTemplateNonEmptyNodeTree`'s own ``parts`` -- also
             mutated in place
             @endrst
             *
             * @return The constructed tree
             *
             @rst
             .. note::
                Each synthetic ``else``'s #IfPredPart::query is built against its own fresh,
                throwaway :cpp:class:`Z3Context` (not one supplied by the caller) -- matching the
                pure-Python original exactly (a literal ``Z3Context()`` per synthesized ``else``).
                This is safe specifically because a synthetic ``else``'s query is always just the
                literal ``true``, which never needs to share variable identity with anything else
                -- see this method's own implementation comment for the fuller reasoning.
             @endrst
             */
            static std::unique_ptr<IfTemplateNormTree<K, V, KeyHash, KeyEqual>> construct(
                std::vector<std::unique_ptr<IfTemplatePart>>& parts);
    };

}

#include "IfTemplateTree.tpp"

#endif
