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

#ifndef AGRemapCore_RegRestrict_TPP
#define AGRemapCore_RegRestrict_TPP

#include <algorithm>
#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegRestrict<K, V, KeyHash, KeyEqual>::RegRestrict(std::optional<std::vector<K>> allowedKeys, KeyFilter keyFilter,
                                                      bool keepFirstOnly):
        allowedKeys(std::move(allowedKeys)), keyFilter(std::move(keyFilter)), keepFirstOnly(keepFirstOnly) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegRestrict<K, V, KeyHash, KeyEqual>::ContentPart& RegRestrict<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;

        const auto ranges = Base::toRangeSpec(partRanges);
        std::vector<std::pair<K, std::optional<typename ContentPart::RemoveKeyCheck>>> removals;

        for (const K& key : part.getKeys()) {
            if (keyFilter && !keyFilter(key)) {
                continue;
            }

            if (allowedKeys.has_value() && std::find_if(allowedKeys->begin(), allowedKeys->end(), [&key](const K& allowed) {
                    return KeyEqual{}(allowed, key);
                }) == allowedKeys->end()) {
                removals.emplace_back(key, std::nullopt);
                continue;
            }

            if (!keepFirstOnly) {
                continue;
            }

            const std::vector<std::pair<long long, V>> bound = part.getValsWithInds(key, true, ranges);
            if (bound.size() <= 1) {
                continue;
            }

            const long long first = bound.front().first;
            removals.emplace_back(key, typename ContentPart::RemoveKeyCheck(
                [first](long long index, const V&) { return index != first; }));
        }

        if (!removals.empty()) {
            part.removeKeys(removals, ranges);
        }

        return part;
    }
}

#endif
