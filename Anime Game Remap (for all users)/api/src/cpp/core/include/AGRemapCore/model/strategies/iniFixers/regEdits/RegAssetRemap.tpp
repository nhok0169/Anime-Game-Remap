#ifndef AGRemapCore_RegAssetRemap_TPP
#define AGRemapCore_RegAssetRemap_TPP

#include "RegAssetRemap.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegAssetRemap<K, V, KeyHash, KeyEqual>::RegAssetRemap(std::vector<std::pair<K, AssetSpec>> assets, std::string toModName,
                                                           std::optional<Version> fromVersion, std::optional<Version> toVersion):
        assets(std::move(assets)), toModName(std::move(toModName)),
        fromVersion(std::move(fromVersion)), toVersion(std::move(toVersion)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegAssetRemap<K, V, KeyHash, KeyEqual>::ContentPart& RegAssetRemap<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;

        if (toModName.empty()) {
            return part;
        }

        std::optional<typename ContentPart::RangeSpec> ranges = Base::toRangeSpec(partRanges);
        const std::vector<std::optional<K>> noFilter;

        for (const auto& entry : assets) {
            const K& reg = entry.first;
            const AssetSpec& spec = entry.second;

            if (spec.assets == nullptr) {
                continue;
            }

            // Collected first, then written: setValByInd mutates the part the indices came from.
            std::vector<std::pair<long long, V>> regVals = part.getValsWithInds(reg, true, ranges);

            for (const auto& regVal : regVals) {
                std::optional<V> replacement =
                    spec.assets->replace(regVal.second, fromVersion, noFilter, toVersion, toModName, false);

                if (replacement.has_value()) {
                    part.setValByInd(regVal.first, *replacement);
                } else if (spec.notFoundVal.has_value()) {
                    // Deliberately loud -- see this class's own note on why leaving an unmapped
                    // value in place is the more dangerous option.
                    part.setValByInd(regVal.first, *spec.notFoundVal);
                }
            }
        }

        return part;
    }
}

#endif
