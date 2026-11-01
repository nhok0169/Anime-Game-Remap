#ifndef AGRemapCore_GraphCollectRow_H
#define AGRemapCore_GraphCollectRow_H

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
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     One occurrence of a key a :cpp:class:`GraphCollect` collected -- its own header so that
     :cpp:class:`IniParseData`, which files rows, does not need the collect itself
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    struct GraphCollectRow {
        /**
         * @brief The graph a row is found in
         */
        using GraphId = typename BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphId;

        /**
         * @brief The occurrences of one key in a part, each with its line, or the value carried in (no line)
         */
        using KeyState = std::vector<std::pair<std::optional<long long>, V>>;

        /**
         * @brief The graph it was found in
         */
        GraphId graphId;

        /**
         * @brief The `section`_ it is in
         */
        std::string section;

        /**
         * @brief The root `section`_ of the graph the walk reached it from -- only with :cpp:member:`GraphCollect::withConditions`
         */
        std::string rootSection;

        /**
         * @brief The id of its part within the `section`_
         */
        std::size_t partId = 0;

        /**
         * @brief Its order index within the part
         */
        long long line = 0;

        /**
         * @brief Its key
         */
        K key;

        /**
         * @brief Its value
         */
        V value;

        /**
         * @brief What each of the graph's :cpp:member:`GraphCollect::contextKeys` holds at the part, in the order they are named
         */
        std::vector<std::pair<K, KeyState>> context;

        /**
         * @brief The condition of the ``if`` blocks the part sits in -- only with :cpp:member:`GraphCollect::withConditions`
         */
        std::optional<Z3Predicate> condition;

        /**
         * @brief
         @rst
         The test of each ``if`` / ``else if`` branch the part sits in within its own `section`_, outermost first,
         as written without its surrounding whitespace (``$swapvar == 1``); an ``else`` branch is recorded as ``else``. Unlike \ref condition, this
         is the text of the section alone -- the conditions of a caller's ``run =`` are not in it -- and it is
         always filled
         @endrst
         */
        std::vector<std::string> guards;

        /**
         * @brief
         @rst
         Which branch of each ``if`` block the part sits in within its own `section`_, outermost first: the
         position, among the section's parts, of the ``if`` / ``else if`` / ``else`` line that opens the branch.
         It names the same branches as \ref guards, but tells apart two branches whose tests read the same -- two
         parts share a branch exactly when one's list starts with the other's. Always filled
         @endrst
         */
        std::vector<std::size_t> blocks;

        /**
         * @brief
         @rst
         The part's position among its `section`_'s parts, counted from 0 as \ref blocks counts them (an ``if`` line
         is a part too). Always filled
         @endrst
         */
        std::size_t position = 0;
    };
}

#endif
