#ifndef AGRemapCore_RegAnchorAdd_H
#define AGRemapCore_RegAnchorAdd_H

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
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseRegEdit`

     Adds lines next to every line of a part whose key is one of the \\ref anchorKeys, or replaces that line
     :raw-html:`<br />` :raw-html:`<br />`

     For each such line (an *anchor*), \\ref adder is handed the anchor's key and value and answers with the
     lines to add -- or ``std::nullopt`` to leave that anchor alone. Where the lines go is \\ref placement:
     immediately before the anchor, immediately after it, or in its place. The lines keep the order
     \\ref adder gives them, and the anchors are visited from the last to the first, so every anchor of the
     part is visited exactly once however many lines are added.

     .. code-block:: ini

        ; anchorKeys = {"drawindexed"}, placement = After,
        ; adder: "6, 0, 0" -> {("run", "CommandListMark")}, anything else -> std::nullopt
        drawindexed = 6, 0, 0
        run = CommandListMark       ; added
        drawindexed = 3, 6, 0       ; left alone

     A key matches an anchor key case-insensitively, ignoring surrounding whitespace, as 3DMigoto reads
     keys; \\ref anchorIf, when set, decides instead. Only the lines whose order index is inside 'partRanges' are anchors
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegAnchorAdd: public BaseRegEdit<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseRegEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseRegEdit::ContentPart
             */
            using ContentPart = typename Base::ContentPart;

            /**
             * @copydoc BaseRegEdit::OrderRanges
             */
            using OrderRanges = typename Base::OrderRanges;

            /**
             * @brief Where the added lines go, relative to their anchor
             */
            enum class Placement {
                /** @brief Immediately before the anchor */
                Before,

                /** @brief Immediately after the anchor */
                After,

                /** @brief In the anchor's place: the anchor is removed */
                Replace
            };

            /**
             * @brief
             @rst
             Given an anchor's key and value, the lines to add for it -- or ``std::nullopt`` to leave the anchor
             alone. An empty list with :cpp:enumerator:`Placement::Replace` removes the anchor
             @endrst
             */
            using Adder = std::function<std::optional<std::vector<std::pair<K, V>>>(const K& key, const V& value)>;

            /**
             * @brief The keys whose lines are anchors, matched case-insensitively and ignoring surrounding whitespace
             */
            std::vector<K> anchorKeys;

            /**
             * @brief Decides whether a line's key makes it an anchor
             */
            using AnchorFilter = std::function<bool(const K& key)>;

            /**
             * @brief
             @rst
             Decides which keys make a line an anchor, in place of \\ref anchorKeys -- eg. every key starting
             with ``ps-t``. An empty function leaves the choice to \\ref anchorKeys :raw-html:`<br />` :raw-html:`<br />`

             **Default**: an empty function
             @endrst
             */
            AnchorFilter anchorIf;

            /**
             * @brief
             @rst
             What each anchor gets. An empty function leaves every anchor alone :raw-html:`<br />` :raw-html:`<br />`

             **Default**: an empty function
             @endrst
             */
            Adder adder;

            /**
             * @brief
             @rst
             Where the added lines go :raw-html:`<br />` :raw-html:`<br />`

             **Default**: :cpp:enumerator:`Placement::After`
             @endrst
             */
            Placement placement;

            /**
             * @brief
             @rst
             Whether only the part's FIRST anchor -- the one with the lowest order index inside 'partRanges' -- gets
             lines, eg. to capture a register just before the line that first rebinds it :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             */
            bool firstOnly;

            /**
             * @brief
             @rst
             Whether a line the part already holds -- the same key and the same value, both compared
             case-insensitively and ignoring surrounding whitespace -- is left out, so a part visited twice gets
             its lines once :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             */
            bool skipExisting;

            /**
             * @brief Constructs a new anchored add
             *
             * @param anchorKeys The keys whose lines are anchors. **Default**: empty
             * @param adder What each anchor gets. **Default**: an empty function
             * @param placement Where the added lines go. **Default**: \ref Placement::After
             * @param anchorIf Decides which keys make a line an anchor, in place of 'anchorKeys'. **Default**: an empty function
             * @param firstOnly Whether only the part's first anchor gets lines. **Default**: ``false``
             * @param skipExisting Whether a line the part already holds is left out. **Default**: ``false``
             */
            explicit RegAnchorAdd(std::vector<K> anchorKeys = {}, Adder adder = nullptr, Placement placement = Placement::After,
                                  AnchorFilter anchorIf = nullptr, bool firstOnly = false, bool skipExisting = false);

            /**
             * @brief Whether 'part' holds a line of 'key' and 'value', both compared case-insensitively and ignoring surrounding whitespace
             *
             * @param part The part to search
             * @param key The key of the line
             * @param value The value of the line
             *
             * @return Whether 'part' holds it
             */
            static bool holds(const ContentPart& part, const K& key, const V& value);

            /**
             * @brief
             @rst
             Adds \\ref adder's lines at every anchor of 'part' inside 'partRanges', from the last anchor to the first
             @endrst
             *
             * @param part The part of the `IfTemplate` being edited, modified in place
             * @param sectionName The name of the `section`_ being edited. Unused by this edit
             * @param modType The type of mod to fix. Unused by this edit. **Default**: ``nullptr``
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             * @param partRanges The valid order indices to process for 'part', or ``nullptr`` for all of them. **Default**: ``nullptr``
             *
             * @return The same part that was passed in, after editing
             */
            ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                              const std::string& modName = "", const OrderRanges* partRanges = nullptr) override;
    };
}

#include "RegAnchorAdd.tpp"

#endif
