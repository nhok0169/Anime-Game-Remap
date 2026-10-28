#ifndef AGRemapCore_RegDrawRangeRemap_TPP
#define AGRemapCore_RegDrawRangeRemap_TPP

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
    RegDrawRangeRemap<K, V, KeyHash, KeyEqual>::RegDrawRangeRemap(std::vector<std::size_t> keptTriangleIds, Reroute reroute):
        keptTriangleIds(std::move(keptTriangleIds)), reroute(std::move(reroute)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::vector<long long>> RegDrawRangeRemap<K, V, KeyHash, KeyEqual>::parseRange(const std::string& value) {
        std::vector<long long> numbers;
        std::size_t pos = 0;
        while (pos <= value.size()) {
            const std::size_t comma = value.find(',', pos);
            const std::string field(StringTools::strip(value.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos)));
            if (field.empty() || field.find_first_not_of("-0123456789") != std::string::npos) {
                return std::nullopt;
            }
            numbers.push_back(std::stoll(field));
            if (comma == std::string::npos) {
                break;
            }
            pos = comma + 1;
        }

        if (numbers.size() < 2 || numbers[0] < 0 || numbers[1] < 0) {
            return std::nullopt;
        }
        return numbers;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> RegDrawRangeRemap<K, V, KeyHash, KeyEqual>::remapRange(const std::string& value,
                                                                                      const std::vector<std::size_t>& keptTriangleIds) {
        const std::optional<std::vector<long long>> numbers = parseRange(value);
        if (!numbers.has_value()) {
            return std::nullopt;
        }

        const std::size_t first = static_cast<std::size_t>((*numbers)[1]) / 3;
        const std::size_t last = (static_cast<std::size_t>((*numbers)[1]) + static_cast<std::size_t>((*numbers)[0])) / 3;
        const auto begin = std::lower_bound(keptTriangleIds.begin(), keptTriangleIds.end(), first);
        const auto end = std::lower_bound(keptTriangleIds.begin(), keptTriangleIds.end(), last);
        const std::size_t start = static_cast<std::size_t>(begin - keptTriangleIds.begin());
        const std::size_t count = static_cast<std::size_t>(end - begin);

        std::string out = std::to_string(count * 3) + ", " + std::to_string(start * 3);
        for (std::size_t i = 2; i < numbers->size(); ++i) {
            out += ", " + std::to_string((*numbers)[i]);
        }
        return out;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegDrawRangeRemap<K, V, KeyHash, KeyEqual>::ContentPart& RegDrawRangeRemap<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;

        const K drawKey(IniKeywords::DrawIndexed);
        const auto ranges = Base::toRangeSpec(partRanges);
        const std::vector<std::pair<long long, V>> vals = part.getValsWithInds(drawKey, true, ranges);
        if (vals.empty()) {
            return part;
        }

        std::vector<V> remapped;
        std::vector<std::optional<V>> runs;
        bool changed = false;
        bool anyRun = false;
        for (const auto& entry : vals) {
            const std::optional<std::string> value = remapRange(entry.second, keptTriangleIds);
            changed = changed || value.has_value();
            remapped.push_back(value.has_value() ? V(*value) : entry.second);

            std::optional<V> run;
            if (value.has_value() && reroute) {
                const std::vector<long long> numbers = *parseRange(entry.second);
                run = reroute(static_cast<std::size_t>(numbers[1]), static_cast<std::size_t>(numbers[0]), remapped.back());
            }
            anyRun = anyRun || run.has_value();
            runs.push_back(std::move(run));
        }

        if (changed) {
            part.replaceVals({{drawKey, typename ContentPart::ReplaceSpec(remapped)}}, false, ranges);
        }

        // Each rerouted line swapped for a call, at the same position.
        if (anyRun) {
            const K runKey(IniKeywords::Run);
            const std::vector<std::pair<long long, V>> placed = part.getValsWithInds(drawKey, true, ranges);
            for (std::size_t i = 0; i < placed.size() && i < runs.size(); ++i) {
                if (!runs[i].has_value()) {
                    continue;
                }

                const std::size_t pos = static_cast<std::size_t>(placed[i].first);
                part.removeKVPAt(pos);
                part.addKVPAt(static_cast<long long>(pos), runKey, *runs[i]);
            }
        }
        return part;
    }
}

#endif
