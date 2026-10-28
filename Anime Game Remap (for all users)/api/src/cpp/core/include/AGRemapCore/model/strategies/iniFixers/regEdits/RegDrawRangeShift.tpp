#ifndef AGRemapCore_RegDrawRangeShift_TPP
#define AGRemapCore_RegDrawRangeShift_TPP

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

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegDrawRangeShift<K, V, KeyHash, KeyEqual>::RegDrawRangeShift(long long offset, long long count, std::vector<K> startKeys):
        offset(offset), count(count), startKeys(std::move(startKeys)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RegDrawRangeShift<K, V, KeyHash, KeyEqual>::shiftRange(const std::string& value, long long offset, long long count) {
        if (StringTools::equalsIgnoreCase(StringTools::strip(value), "auto")) {
            return std::to_string(count) + ", " + std::to_string(offset) + ", 0";
        }

        std::vector<std::string> fields;
        std::size_t pos = 0;
        while (true) {
            const std::size_t comma = value.find(',', pos);
            fields.emplace_back(StringTools::strip(value.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos)));
            if (comma == std::string::npos) {
                break;
            }
            pos = comma + 1;
        }

        if (fields.size() < 2 || fields[1].empty() || fields[1].find_first_not_of("0123456789") != std::string::npos) {
            return value;
        }

        fields[1] = std::to_string(std::stoll(fields[1]) + offset);
        std::string out = fields[0];
        for (std::size_t i = 1; i < fields.size(); ++i) {
            out += ", " + fields[i];
        }
        return out;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegDrawRangeShift<K, V, KeyHash, KeyEqual>::ContentPart& RegDrawRangeShift<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;

        const K drawKey(IniKeywords::DrawIndexed);
        const auto ranges = Base::toRangeSpec(partRanges);
        const std::vector<std::pair<long long, V>> vals = part.getValsWithInds(drawKey, true, ranges);
        if (!vals.empty()) {
            std::vector<V> shifted;
            shifted.reserve(vals.size());
            for (const auto& entry : vals) {
                shifted.push_back(V(shiftRange(entry.second, offset, count)));
            }
            part.replaceVals({{drawKey, typename ContentPart::ReplaceSpec(shifted)}}, false, ranges);
        }

        // The keys of the part that name a start, each once, as the part spells it
        std::vector<K> keys;
        for (const auto& kvp : part.entries()) {
            const bool isStart = std::any_of(startKeys.begin(), startKeys.end(), [&kvp](const K& startKey) {
                return StringTools::equalsIgnoreCase(StringTools::strip(kvp.first), StringTools::strip(startKey));
            });
            if (isStart && std::find(keys.begin(), keys.end(), kvp.first) == keys.end()) {
                keys.push_back(kvp.first);
            }
        }

        for (const K& key : keys) {
            std::vector<V> shifted;
            for (const auto& entry : part.getValsWithInds(key, true, ranges)) {
                const std::string value(StringTools::strip(entry.second));
                shifted.push_back(!value.empty() && value.find_first_not_of("0123456789") == std::string::npos
                                  ? V(std::to_string(std::stoll(value) + offset)) : entry.second);
            }
            part.replaceVals({{key, typename ContentPart::ReplaceSpec(shifted)}}, false, ranges);
        }

        return part;
    }
}

#endif
