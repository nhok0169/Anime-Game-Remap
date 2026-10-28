#ifndef AGRemapCore_RegDrawRangeRemap_H
#define AGRemapCore_RegDrawRangeRemap_H

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

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseRegEdit`

     Remaps the ranges a part's ``drawindexed`` lines draw through an index buffer that lost some of its
     triangles :raw-html:`<br />` :raw-html:`<br />`

     A mod that draws for itself writes ``drawindexed = <count>, <start>, <base>``, and a mod with
     toggles writes several such lines, each a range of its object's index buffer that one toggle shows or
     hides. When a fix writes that index buffer again without some of its triangles (the ones another
     component draws, say), the mod's own numbers overrun the written buffer and every range after a
     removed triangle draws the wrong triangles.

     \\ref keptTriangleIds lists, in ascending order, the SOURCE index of every triangle the written buffer
     keeps. A range's new start is the number of kept triangles before it and its new count the number
     inside it, both times 3; any further number on the line (the base vertex) is kept as it is. A line
     that is not at least two non-negative numbers -- ``drawindexed = auto`` -- is left alone.

     .. code-block:: ini

        ; keptTriangleIds = [0, 1, 2, 5, 6, 7]: triangles 3 and 4 were removed
        drawindexed = 9, 0, 0       ; -> 9, 0, 0
        drawindexed = 9, 9, 0       ; -> 3, 9, 0   (only triangle 5 of 3..5 is kept)
        drawindexed = auto          ; untouched

     A range may also be drawn some other way: \\ref reroute is asked about every remapped line, and when
     it answers with a name the line is replaced, at the same position, by ``run = <name>`` -- eg. a
     ``CustomShader`` `section`_ that draws the remapped range under a blend state of its own.
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegDrawRangeRemap: public BaseRegEdit<K, V, KeyHash, KeyEqual> {
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
             * @brief
             @rst
             Decides whether a remapped range is drawn by a ``run =`` instead :raw-html:`<br />` :raw-html:`<br />`

             Takes the range as the MOD wrote it -- its first index and its index count, before the remap --
             and the remapped value of the line. Returns the name to ``run``, or ``std::nullopt`` to keep the
             ``drawindexed`` line
             @endrst
             */
            using Reroute = std::function<std::optional<V>(std::size_t firstIndex, std::size_t indexCount, const V& remapped)>;

            /**
             * @brief The source index of every triangle the written index buffer keeps, ascending
             */
            std::vector<std::size_t> keptTriangleIds;

            /**
             * @brief
             @rst
             Which remapped ranges are drawn by a ``run =`` instead, or empty to keep every
             ``drawindexed`` line :raw-html:`<br />` :raw-html:`<br />`

             **Default**: empty
             @endrst
             */
            Reroute reroute;

            /**
             * @brief Constructs a new draw-range remap
             *
             * @param keptTriangleIds The source index of every kept triangle, ascending
             * @param reroute Which remapped ranges are drawn by a ``run =`` instead. **Default**: empty
             */
            explicit RegDrawRangeRemap(std::vector<std::size_t> keptTriangleIds = {}, Reroute reroute = {});

            /**
             * @brief
             @rst
             The numbers of a ``drawindexed`` value -- ``count, start[, base, ...]`` -- or ``std::nullopt``
             when it is not at least two numbers, or its count or start is negative (``auto``)
             @endrst
             *
             * @param value The value of the ``drawindexed`` line
             *
             * @return The numbers, in the order they are written
             */
            static std::optional<std::vector<long long>> parseRange(const std::string& value);

            /**
             * @brief The value of a ``drawindexed`` line remapped through 'keptTriangleIds', or ``std::nullopt`` for one #parseRange cannot read
             *
             * @param value The value of the ``drawindexed`` line
             * @param keptTriangleIds The source index of every kept triangle, ascending
             *
             * @return The remapped value, written ``count, start[, base, ...]``
             */
            static std::optional<std::string> remapRange(const std::string& value, const std::vector<std::size_t>& keptTriangleIds);

            /**
             * @brief
             @rst
             Remaps every ``drawindexed`` line of 'part' through \\ref keptTriangleIds, then replaces each one
             \\ref reroute names with a ``run =`` line at the same position
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

#include "RegDrawRangeRemap.tpp"

#endif
