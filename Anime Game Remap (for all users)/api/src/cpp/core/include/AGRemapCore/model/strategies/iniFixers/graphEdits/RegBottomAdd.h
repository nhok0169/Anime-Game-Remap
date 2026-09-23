#ifndef AGRemapCore_RegBottomAdd_H
#define AGRemapCore_RegBottomAdd_H

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

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphEdit`

     Adds `KVPs`_ **at the bottom of every root** `section`_ of a caller/callee graph, at that
     `section`_'s own nesting depth :raw-html:`<br />` :raw-html:`<br />`

     The additions land together, in the order given, in a fresh last :cpp:class:`IfContentPart`
     (:cpp:func:`IfTemplate::addBottomContentPart`). Being at the `section`_'s own depth is the
     whole point: a position inside an ``if`` block runs only when that block is taken, and a
     position "as late as possible" is exactly such a position whenever the `section`_ ends in one

     .. code-block:: ini

        [TextureOverrideCharacterHead]
        ...
        drawindexed = 3252, 0, 0
        if $accessory == 1
           drawindexed = 618, 3252, 0
        endif
        ; the additions go HERE, outside the block, whatever the toggles are set to

     **What this is for.** A fix that has something more to say after everything the mod does --
     another draw call, another binding, a trailing command -- and needs it to run unconditionally.
     :cpp:class:`RegFillMissing` in :cpp:enumerator:`RegFillMissingMode::BottomCover` reaches the
     same position but only for a register the graph is MISSING, which is a different question:
     asking it about a register the mod already has makes it do nothing at all

     .. note::
        This exists because the alternative was measurably worse. Before it, the same placement was
        reached by filling a register nothing reads, in `BottomCover`, and renaming it afterwards --
        the trick the pure-Python original plays with its ``tempDrawIndexed``. That works, but it
        costs one edit per `KVP`_ plus a :cpp:class:`RegRemap` to undo the disguise, and it reads
        like a workaround because it is one (2026-09-14)

     .. note::
        Every root gets the additions, so a graph whose roots overlap gets them once per root by
        design -- the same convention :cpp:class:`RegFillMissing`'s cover modes follow

     A \\ref BaseIniGraphEdit::PartFilter given to \\ref edit is asked about the root's existing last
     :cpp:class:`IfContentPart` -- the part a new one would be appended after -- and a root it
     rejects is left alone entirely
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegBottomAdd: public BaseIniGraphEdit<K, V, KeyHash, KeyEqual> {
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
             * @copydoc BaseIniGraphEdit::OrderRanges
             */
            using OrderRanges = typename Base::OrderRanges;

            /**
             * @copydoc BaseIniGraphEdit::KeySet
             */
            using KeySet = typename Base::KeySet;

            /**
             * @brief The parts this edit appends to
             */
            using ContentPart = typename Graph::ContentPart;

            /**
             * @brief The `section`_ type this edit appends to
             */
            using Section = typename Graph::Section;

            /**
             * @brief The list of `KVP`_ entries this edit adds -- see \\ref additions
             */
            using Additions = std::vector<std::pair<K, V>>;

            /**
             * @brief
             @rst
             The `KVP`_ entries to add. All of them land together at the bottom of each root, as
             consecutive lines in this order -- an empty list makes the edit a no-op
             @endrst
             */
            Additions additions;

            /**
             * @brief
             @rst
             A 3DMigoto condition to put the additions under, eg. ``vs != 037730.0`` -- empty for none
             :raw-html:`<br />` :raw-html:`<br />`

             Non-empty, the additions land in a NEW ``if <condition>`` ... ``endif`` block at the
             bottom of each root, still at the `section`_'s own depth -- so the block runs whenever
             the condition holds, whatever the mod's own toggles are set to. Nothing the `section`_
             already has is moved into it :raw-html:`<br />` :raw-html:`<br />`

             What it is for: a draw that belongs in some of the passes a `section`_ runs in and not
             the others. CitlaliWhisperofStars' dress is outlined by the skin with a cloth-specific
             outline shader Citlali does not have, and drawn through Citlali's outline pass its hull
             covered the skirt's lining in black; ``vs != 037730.0`` -- the ``filter_index`` ORFix
             gives every outline vertex shader -- keeps it to the other passes (2026-09-22). A
             condition that does not parse as a :cpp:class:`Z3Predicate` (``vs`` is not a variable)
             is still written; analyses of the graph then treat the block as possibly taken
             @endrst
             */
            std::string condition;

            /**
             * @brief Constructs a new bottom-adding edit
             *
             * @param additions The `KVP`_ entries to add, in order. **Default**: empty
             * @param condition The condition to put them under -- see \ref condition. **Default**: none
             */
            explicit RegBottomAdd(Additions additions = {}, std::string condition = "");

            /**
             * @copydoc BaseIniGraphEdit::edit
             */
            Graph& edit(Graph& graph, const ModType* modType, const std::string& modName,
                         const PartFilter& partFilter, bool trackKeys,
                         const std::optional<KeySet>& keysToTrack) override;
    };
}

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBottomAdd.tpp"

#endif
