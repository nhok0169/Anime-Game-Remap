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

#ifndef AGRemapCore_GIMIObjPartFilter_TPP
#define AGRemapCore_GIMIObjPartFilter_TPP

#include <type_traits>

#include "GIMIObjPartFilter.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::FilterConfig
            GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::defaultConfig() {
        FilterConfig result{};

        // Same constraint as GIMISectionClassifier::defaultConfig's: only a K that a .ini keyword
        // can literally be spelled as gets real defaults. Every other instantiation supplies its
        // own config.
        if constexpr (std::is_constructible_v<K, const std::string&>) {
            result.hashKey = K(IniKeywords::Hash);
            result.matchFirstIndexKey = K(IniKeywords::MatchFirstIndex);
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::GIMIObjPartFilter(Assets* hashes, Assets* indices, KeySet indexHashKeys,
                                                                   std::optional<Version> version, FilterConfig config):
        indexHashKeys(std::move(indexHashKeys)), version(std::move(version)),
        hashes_(hashes), indices_(indices), config_(std::move(config)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::Assets* GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::hashes() const {
        return hashes_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::setHashes(Assets* newHashes) {
        hashes_ = newHashes;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::Assets* GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::indices() const {
        return indices_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::setIndices(Assets* newIndices) {
        indices_ = newIndices;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const typename GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::FilterConfig&
            GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::config() const {
        return config_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::KeySet
            GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::keysToTrack() const {
        return KeySet{config_.hashKey, config_.matchFirstIndexKey};
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::OrderRanges
            GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::window(const ModObj& modObj, const Colouring& colouring) const {
        std::vector<typename OrderRanges::Range> windows;

        if (hashes_ == nullptr || indices_ == nullptr) {
            return OrderRanges(windows);
        }

        std::vector<std::pair<std::optional<long long>, V>> hashVals = colouring.getIndVals(config_.hashKey);
        std::vector<std::pair<std::optional<long long>, V>> indexVals = colouring.getIndVals(config_.matchFirstIndexKey);

        const std::vector<std::optional<K>> noFilter;

        for (std::size_t i = 0; i < hashVals.size(); ++i) {
            const std::optional<long long>& hashInd = hashVals[i].first;

            // A KVP carried over from an earlier IfContentPart has no index of its own, so there is
            // no window it could open.
            if (!hashInd.has_value()) {
                continue;
            }

            std::optional<std::vector<K>> hashKeyRow = hashes_->getKey(hashVals[i].second, version, noFilter, false);
            if (!hashKeyRow.has_value() || hashKeyRow->empty() || indexHashKeys.find(hashKeyRow->back()) == indexHashKeys.end()) {
                continue;
            }

            // Open-ended when this is the last hash -- everything after it is still governed by it.
            std::optional<long long> windowEnd;
            if (i + 1 < hashVals.size()) {
                windowEnd = hashVals[i + 1].first;
            }

            for (const auto& indexVal : indexVals) {
                const std::optional<long long>& indexInd = indexVal.first;
                if (!indexInd.has_value() || *indexInd < *hashInd) {
                    continue;
                }

                if (windowEnd.has_value() && *indexInd >= *windowEnd) {
                    continue;
                }

                std::optional<std::vector<K>> indexKeyRow = indices_->getKey(indexVal.second, version, noFilter, false);
                if (!indexKeyRow.has_value() || indexKeyRow->size() < 2) {
                    continue;
                }

                // The last two index columns of an Indices row are (component, object) -- the mod
                // object itself.
                ModObj rowObj((*indexKeyRow)[indexKeyRow->size() - 2], indexKeyRow->back());
                if (rowObj == modObj) {
                    windows.emplace_back(*hashInd, windowEnd);
                    break;
                }
            }
        }

        return OrderRanges(windows);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::PartFilter
            GIMIObjPartFilter<K, V, KeyHash, KeyEqual>::filter(const ModObj& modObj) const {
        return [this, modObj](const IterData& iterData, const ModType*, IniFile*) {
            if (iterData.colouring == nullptr) {
                return OrderRanges(std::vector<typename OrderRanges::Range>{});
            }

            return window(modObj, *iterData.colouring);
        };
    }
}

#endif
