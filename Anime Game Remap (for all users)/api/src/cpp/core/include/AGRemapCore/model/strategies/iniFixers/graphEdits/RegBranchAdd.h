#ifndef AGRemapCore_RegBranchAdd_H
#define AGRemapCore_RegBranchAdd_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphEdit`

     Adds `KVPs`_ **inside** a conditional branch, with the entries decided by the condition that
     branch runs under :raw-html:`<br />` :raw-html:`<br />`

     .. code-block:: ini

        [CommandListCharacterBody]
        if $swapvar == 0
           ib = ResourceBodyIb0
           ; these entries go HERE, and are computed for THIS branch
        else if $swapvar == 1
           ib = ResourceBodyIb1
           drawindexed = 200, 0, 0
           ; ...and separately HERE, computed for this one
        endif

     **What this is for.** A mod whose branches are different models -- a merged mod's ``$swapvar``
     chain is a dozen mods behind one ``.ini`` -- where what the fix has to add differs per branch
     because the geometry does. A ``drawindexed`` is the case that forced this: its count and offset
     come from the index buffers of the branch being drawn, and one merged mod's twelve branches have
     twelve different pairs of numbers :raw-html:`<br />` :raw-html:`<br />`

     **How a branch is identified.** Not by position: \\ref branchOf is handed the
     :cpp:class:`Z3Predicate` the part runs under and answers with a key naming the branch, so the
     caller decides by satisfiability and nothing here assumes the branches are a flat ordered list.
     An empty key means "nothing belongs here", which is how the caller declines a part it cannot
     attribute to exactly one branch -- a `section`_'s unconditional preamble, say, which every
     branch's condition is satisfiable with

     .. note::
        Where a key appears in several parts -- a branch that carries its ``ib`` in a nested ``if``
        and its draws after it -- the entries go in the **last** of them, in the graph's own
        iteration order. That is the one everything else in the branch has already run before

     .. note::
        This is deliberately NOT :cpp:class:`RegBottomAdd` with a computed list.
        :cpp:class:`RegBottomAdd` lands one block at the `section`_'s own depth, outside every
        ``if``, which is right precisely when the addition does not depend on which branch is taken
        -- and wrong when it does. Reach for that one first; this one only when the entries differ
        per branch :raw-html:`<br />` :raw-html:`<br />`

        Putting a draw in the `section`_ instead also moves the ``NNFix`` / ``ORFix`` call: a
        :cpp:class:`RegDelimitedAdd` in :cpp:enumerator:`RegDelimitedAddMode::PerPath` treats a
        content part as atomic, so a part holding both a ``run =`` and a ``drawindexed`` takes the
        call before its own draw and counts every path covered -- leaving the draws inside the
        callee with none. Keeping the branch's additions in the branch avoids creating that part at
        all (2026-09-16)
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegBranchAdd: public BaseIniGraphEdit<K, V, KeyHash, KeyEqual> {
        public:
            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseIniGraphEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniGraphEdit::Graph
             */
            using Graph = typename Base::Graph;

            /**
             * @copydoc BaseIniGraphEdit::PartFilter
             */
            using PartFilter = typename Base::PartFilter;

            /**
             * @copydoc BaseIniGraphEdit::IterData
             */
            using IterData = typename Base::IterData;

            /**
             * @copydoc BaseIniGraphEdit::KeySet
             */
            using KeySet = typename Base::KeySet;

            /**
             * @brief The type of the parts this edits
             */
            using ContentPart = typename Graph::ContentPart;

            /**
             * @brief The `KVP`_ entries added for one branch
             */
            using Additions = std::vector<std::pair<K, V>>;

            /**
             * @brief What belongs in one branch, and which branch it is
             */
            struct Branch {
                /**
                 * @brief
                 @rst
                 Names the branch these entries belong to. **Empty means add nothing here**, which
                 is how a part that cannot be attributed to exactly one branch is declined
                 @endrst
                 */
                std::string key;

                /**
                 * @brief The `KVP`_ entries to add, in order
                 */
                Additions additions;

                /**
                 * @brief
                 @rst
                 `KVP`_ entries whose value is SET in the branch's part rather than appended to it
                 -- replacing whatever the branch carries, and added when the key is absent
                 :raw-html:`<br />` :raw-html:`<br />`

                 For the keys a branch already answers for itself and answers differently from its
                 neighbours: a ``draw`` whose count is that variant's own vertex count, say. Adding a
                 second one of those would not correct the first, it would draw twice
                 @endrst
                 */
                Additions replacements;
            };

            /**
             * @brief
             @rst
             Decides what a part's branch is and what belongs in it, from the condition the part
             runs under
             @endrst
             */
            using BranchOf = std::function<Branch(const Z3Predicate&, const IterData&)>;

            /**
             * @brief Decides what belongs in each branch -- empty makes the edit a no-op
             */
            BranchOf branchOf;

            /**
             * @brief Constructs a new branch-adding edit
             *
             * @param branchOf Decides what belongs in each branch. **Default**: empty
             */
            explicit RegBranchAdd(BranchOf branchOf = {});

            /**
             * @copydoc BaseIniGraphEdit::edit
             */
            Graph& edit(Graph& graph, const ModType* modType, const std::string& modName,
                         const PartFilter& partFilter, bool trackKeys,
                         const std::optional<KeySet>& keysToTrack) override;
    };
}

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBranchAdd.tpp"

#endif
