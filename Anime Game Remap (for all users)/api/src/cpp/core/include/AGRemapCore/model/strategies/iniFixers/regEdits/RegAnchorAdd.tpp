#ifndef AGRemapCore_RegAnchorAdd_TPP
#define AGRemapCore_RegAnchorAdd_TPP

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

#include <algorithm>
#include <utility>

#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegAnchorAdd<K, V, KeyHash, KeyEqual>::RegAnchorAdd(std::vector<K> anchorKeys, Adder adder, Placement placement,
                                                         AnchorFilter anchorIf, bool firstOnly, bool skipExisting):
        anchorKeys(std::move(anchorKeys)), anchorIf(std::move(anchorIf)), adder(std::move(adder)), placement(placement),
        firstOnly(firstOnly), skipExisting(skipExisting) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool RegAnchorAdd<K, V, KeyHash, KeyEqual>::holds(const ContentPart& part, const K& key, const V& value) {
        for (const auto& entry : part.entries()) {
            if (StringTools::equalsIgnoreCase(StringTools::strip(entry.first), StringTools::strip(key))
                    && StringTools::equalsIgnoreCase(StringTools::strip(entry.second), StringTools::strip(value))) {
                return true;
            }
        }

        return false;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegAnchorAdd<K, V, KeyHash, KeyEqual>::ContentPart& RegAnchorAdd<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;

        if (!adder || (anchorKeys.empty() && !anchorIf)) {
            return part;
        }

        // The anchors, in the part's order
        const auto items = part.items();
        std::vector<std::size_t> anchors;
        for (std::size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            const bool isAnchor = anchorIf ? anchorIf(item.key) : std::any_of(anchorKeys.begin(), anchorKeys.end(), [&item](const K& anchorKey) {
                return StringTools::equalsIgnoreCase(StringTools::strip(item.key), StringTools::strip(anchorKey));
            });
            if (isAnchor && (partRanges == nullptr || partRanges->has(static_cast<long long>(item.orderIndex)))) {
                anchors.push_back(i);
                if (firstOnly) {
                    break;
                }
            }
        }

        // From the last anchor to the first, so a line added at one never moves an anchor not yet reached
        for (auto anchor = anchors.rbegin(); anchor != anchors.rend(); ++anchor) {
            const auto& item = items[*anchor];
            std::optional<std::vector<std::pair<K, V>>> lines = adder(item.key, item.value);
            if (!lines.has_value()) {
                continue;
            }

            if (skipExisting) {
                lines->erase(std::remove_if(lines->begin(), lines->end(), [&part](const std::pair<K, V>& line) {
                    return holds(part, line.first, line.second);
                }), lines->end());

                if (lines->empty() && placement != Placement::Replace) {
                    continue;
                }
            }

            const auto at = static_cast<long long>(item.orderIndex);
            long long insertAt = at;
            if (placement == Placement::After) {
                insertAt = at + 1;
            } else if (placement == Placement::Replace) {
                part.removeKVPAt(static_cast<size_t>(at));
            }

            for (const auto& line : *lines) {
                part.addKVPAt(insertAt++, line.first, line.second);
            }
        }

        return part;
    }
}

#endif
