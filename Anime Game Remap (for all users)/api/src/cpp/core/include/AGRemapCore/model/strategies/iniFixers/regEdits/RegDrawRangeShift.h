#ifndef AGRemapCore_RegDrawRangeShift_H
#define AGRemapCore_RegDrawRangeShift_H

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
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseRegEdit`

     Moves the ranges a part's ``drawindexed`` lines draw by a fixed number of indices, for an index buffer
     that now sits behind other indices :raw-html:`<br />` :raw-html:`<br />`

     When a fix concatenates several index buffers into one, every buffer after the first starts \\ref offset
     indices in, and the ``drawindexed = <count>, <start>, <base>`` lines written for it address the start of
     the merged buffer instead of its own part of it. This edit adds \\ref offset to each line's start and keeps
     every other field as written; ``drawindexed = auto`` becomes the buffer's whole range,
     ``<count>, <offset>, 0``. A line whose start is not a plain non-negative number is left as it is.

     .. code-block:: ini

        ; offset = 30, count = 12
        drawindexed = 6, 0, 0       ; -> 6, 30, 0
        drawindexed = 6, 6          ; -> 6, 36
        drawindexed = auto          ; -> 12, 30, 0
        drawindexed = $n, $s, 0     ; untouched

     Some draws take their start from a variable the part sets (eg. a library's command list drawing
     ``drawindexed = $_1, $_2, 0``): every key of \\ref startKeys -- matched case-insensitively, ignoring
     surrounding whitespace -- holds such a start, and its plain non-negative values are moved by \\ref offset too
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegDrawRangeShift: public BaseRegEdit<K, V, KeyHash, KeyEqual> {
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
             * @brief How many indices the ranges are moved by
             */
            long long offset;

            /**
             * @brief How many indices the buffer holds, which is what ``drawindexed = auto`` becomes
             */
            long long count;

            /**
             * @brief
             @rst
             Further keys whose values are start indices into the same buffer, matched case-insensitively and
             ignoring surrounding whitespace :raw-html:`<br />` :raw-html:`<br />`

             **Default**: empty
             @endrst
             */
            std::vector<K> startKeys;

            /**
             * @brief Constructs a new draw-range shift
             *
             * @param offset How many indices the ranges are moved by. **Default**: ``0``
             * @param count How many indices the buffer holds. **Default**: ``0``
             * @param startKeys Further keys whose values are start indices. **Default**: empty
             */
            explicit RegDrawRangeShift(long long offset = 0, long long count = 0, std::vector<K> startKeys = {});

            /**
             * @brief
             @rst
             The value of a ``drawindexed`` line moved by 'offset': ``auto`` becomes ``<count>, <offset>, 0``, a
             line whose second field is a plain non-negative number has 'offset' added to it (every field then
             written stripped, separated by ``", "``), and anything else is returned as it is
             @endrst
             *
             * @param value The value of the ``drawindexed`` line
             * @param offset How many indices to move the range by
             * @param count How many indices the buffer holds
             *
             * @return The moved value
             */
            static std::string shiftRange(const std::string& value, long long offset, long long count);

            /**
             * @brief
             @rst
             Moves every ``drawindexed`` line of 'part' by \\ref offset, then every plain non-negative value of
             the keys \\ref startKeys names
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

#include "RegDrawRangeShift.tpp"

#endif
