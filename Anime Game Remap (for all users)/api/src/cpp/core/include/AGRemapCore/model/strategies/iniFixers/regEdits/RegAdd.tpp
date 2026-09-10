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

#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAdd.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegAdd<K, V, KeyHash, KeyEqual>::RegAdd(std::vector<std::pair<K, V>> vals, bool latest):
        vals(std::move(vals)), latest(latest) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegAdd<K, V, KeyHash, KeyEqual>::ContentPart& RegAdd<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;

        if (vals.empty()) {
            return part;
        }

        // no window restriction -- add straight to the true beginning/end of 'part'
        if (partRanges == nullptr) {
            if (latest) {
                part.addKVPs(vals);
            } else {
                part.addKVPsToFront(vals);
            }

            return part;
        }

        const auto& ranges = partRanges->ranges;
        if (ranges.empty()) {
            return part;
        }

        long long insertInd = 0;

        // add right after the last valid index of the window (unbounded -> the true end of 'part')
        if (latest) {
            const auto& bound = ranges.back().second;
            insertInd = bound.has_value() ? *bound : static_cast<long long>(part.size());

        // add right before the first valid index of the window (unbounded -> the true beginning of 'part')
        } else {
            const auto& bound = ranges.front().first;
            insertInd = bound.has_value() ? *bound : 0;
        }

        const size_t valsLen = vals.size();
        for (size_t i = 0; i < valsLen; ++i) {
            part.addKVPAt(insertInd + static_cast<long long>(i), vals[i].first, vals[i].second);
        }

        return part;
    }
}
