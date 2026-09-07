#ifndef AGRemapCore_RegAssetRemap_TPP
#define AGRemapCore_RegAssetRemap_TPP

#include "RegAssetRemap.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegAssetRemap<K, V, KeyHash, KeyEqual>::RegAssetRemap(std::vector<std::pair<K, AssetSpec>> assets, std::string toModName,
                                                           std::string fromModName,
                                                           std::optional<Version> fromVersion, std::optional<Version> toVersion):
        assets(std::move(assets)), toModName(std::move(toModName)), fromModName(std::move(fromModName)),
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
        for (const auto& entry : assets) {
            const K& reg = entry.first;
            const AssetSpec& spec = entry.second;

            if (spec.assets == nullptr) {
                continue;
            }

            // Constrains the reverse lookup to the SOURCE mod's rows, which is what makes it
            // deterministic when source and target share a value -- see RegAssetRemap::fromModName.
            // An empty filter (the old behaviour) leaves the lookup free to land on either.
            //
            // Sized PER ASSET TABLE, and that is not optional: getKey wants exactly one entry per
            // non-version index column and throws std::invalid_argument otherwise. Hashes has two
            // (mod name, hash type) where Indices has three (mod name, component, object), so a
            // filter built once outside this loop is wrong for one of them. Only the first column
            // -- the mod name -- is constrained; the rest stay nullopt for "any".
            std::vector<std::optional<K>> reverseFilter;
            const std::size_t filterLen = spec.assets->getRepo().getTotalIndices() - 1;
            if (!fromModName.empty() && filterLen > 0) {
                reverseFilter.assign(filterLen, std::nullopt);
                reverseFilter[0] = fromModName;
            }

            // Collected first, then written: setValByInd mutates the part the indices came from.
            std::vector<std::pair<long long, V>> regVals = part.getValsWithInds(reg, true, ranges);

            for (const auto& regVal : regVals) {
                std::optional<V> replacement =
                    spec.assets->replace(regVal.second, fromVersion, reverseFilter, toVersion, toModName, false);

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
