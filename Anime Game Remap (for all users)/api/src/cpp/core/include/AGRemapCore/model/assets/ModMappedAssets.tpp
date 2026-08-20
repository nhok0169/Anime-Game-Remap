#include <algorithm>
#include <string>


namespace AGRemapCore {

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::ModMappedAssets(ModDictAssets<K, T, KeyHash, KeyEqual> repo, std::unordered_map<K, std::vector<K>, KeyHash, KeyEqual> map):
        repo_(std::move(repo)), map_(std::move(map))
    {
        rebuildKeys();
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    void ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::rebuildKeys() {
        keys_.clear();

        repo_.forEachEntry([this](const std::vector<K>& nonVersionVals, const Version& version, const T& value) {
            std::vector<VersionBucket>& buckets = keys_[value];

            auto pos = std::lower_bound(buckets.begin(), buckets.end(), version, [](const VersionBucket& bucket, const Version& target) {
                return bucket.version < target;
            });

            if (pos != buckets.end() && pos->version == version) {
                pos->candidates.push_back(nonVersionVals);
            } else {
                buckets.insert(pos, VersionBucket{version, std::vector<std::vector<K>>{nonVersionVals}});
            }
        });
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    void ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::addRepoRows(std::vector<Row<K, T>> newRows) {
        repo_.addRows(std::move(newRows));
        rebuildKeys();
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    void ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::addMap(const std::unordered_map<K, std::vector<K>, KeyHash, KeyEqual>& assetMap, std::vector<Row<K, T>> newRows) {
        if (!newRows.empty()) {
            addRepoRows(std::move(newRows));
        }

        KeyEqual keyEqual;
        for (const auto& entry : assetMap) {
            std::vector<K>& existing = map_[entry.first];
            for (const K& toAsset : entry.second) {
                bool found = false;
                for (const K& present : existing) {
                    if (keyEqual(present, toAsset)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    existing.push_back(toAsset);
                }
            }
        }
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::optional<T> ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::get(const std::vector<K>& nonVersionVals, const std::optional<Version>& version, bool errorOnNotFound) const {
        return repo_.get(nonVersionVals, version, errorOnNotFound);
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::optional<typename ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::InternalKeyResult>
    ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::getKeyInternal(const T& asset, const std::optional<Version>& fromVersion, const std::vector<std::optional<K>>& fromNonVersionVals, bool errorOnNotFound) const {
        if (!fromNonVersionVals.empty() && fromNonVersionVals.size() != repo_.getTotalIndices() - 1) {
            throw std::invalid_argument("ModMappedAssets::getKey: expected " + std::to_string(repo_.getTotalIndices() - 1) + " non-version value filters, got " + std::to_string(fromNonVersionVals.size()));
        }

        auto keysIt = keys_.find(asset);
        if (keysIt == keys_.end() || keysIt->second.empty()) {
            if (errorOnNotFound) {
                throw std::out_of_range("ModMappedAssets::getKey: no keys found for the given asset");
            }
            return std::nullopt;
        }

        const std::vector<VersionBucket>& buckets = keysIt->second;  // sorted ascending by version

        std::size_t idx;
        if (!fromVersion.has_value()) {
            idx = buckets.size() - 1;
        } else {
            auto pos = std::lower_bound(buckets.begin(), buckets.end(), *fromVersion, [](const VersionBucket& bucket, const Version& target) {
                return bucket.version < target;
            });
            if (pos != buckets.end() && pos->version == *fromVersion) {
                idx = static_cast<std::size_t>(pos - buckets.begin());
            } else if (pos != buckets.begin()) {
                idx = static_cast<std::size_t>((pos - 1) - buckets.begin());
            } else {
                idx = 0;
            }
        }

        const VersionBucket& bucket = buckets[idx];

        for (const std::vector<K>& candidate : bucket.candidates) {
            bool matches = true;
            if (!fromNonVersionVals.empty()) {
                for (std::size_t i = 0; i < candidate.size(); ++i) {
                    if (fromNonVersionVals[i].has_value() && !KeyEqual{}(*fromNonVersionVals[i], candidate[i])) {
                        matches = false;
                        break;
                    }
                }
            }
            if (matches) {
                return InternalKeyResult{bucket.version, candidate};
            }
        }

        if (errorOnNotFound) {
            throw std::out_of_range("ModMappedAssets::getKey: no candidate key matched the given non-version value filter");
        }
        return std::nullopt;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::optional<std::vector<K>> ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::getKey(const T& asset, const std::optional<Version>& fromVersion, const std::vector<std::optional<K>>& fromNonVersionVals, bool errorOnNotFound) const {
        std::optional<InternalKeyResult> result = getKeyInternal(asset, fromVersion, fromNonVersionVals, errorOnNotFound);
        if (!result.has_value()) {
            return std::nullopt;
        }
        return result->nonVersionVals;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    bool ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::hasFrom(const T& asset, const std::optional<Version>& version, const std::vector<std::optional<K>>& nonVersionVals) const {
        return getKeyInternal(asset, version, nonVersionVals, false).has_value();
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::optional<std::vector<K>> ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::resolveToAssetNames(const K& fromAsset, const std::vector<K>& toAssetsFilter) const {
        auto mapIt = map_.find(fromAsset);
        if (mapIt == map_.end()) {
            return std::nullopt;
        }
        if (toAssetsFilter.empty()) {
            return mapIt->second;
        }

        const std::vector<K>& mapped = mapIt->second;
        KeyEqual keyEqual;
        std::vector<K> result;
        for (const K& candidate : toAssetsFilter) {
            for (const K& present : mapped) {
                if (keyEqual(candidate, present)) {
                    result.push_back(candidate);
                    break;
                }
            }
        }
        return result;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::optional<T> ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::replace(const T& asset, const std::optional<Version>& fromVersion, const std::vector<std::optional<K>>& fromNonVersionVals, const std::optional<Version>& toVersion, const K& toAssetName, bool errorOnNotFound) const {
        std::optional<InternalKeyResult> key = getKeyInternal(asset, fromVersion, fromNonVersionVals, errorOnNotFound);
        if (!key.has_value()) {
            return std::nullopt;
        }

        const K& fromAsset = key->nonVersionVals[0];
        std::optional<std::vector<K>> toAssetNames = resolveToAssetNames(fromAsset, {toAssetName});
        if (!toAssetNames.has_value()) {
            if (errorOnNotFound) {
                throw std::out_of_range("ModMappedAssets::replace: the resolved asset's name is not present in the map");
            }
            return std::nullopt;
        }
        if (toAssetNames->empty()) {
            return std::nullopt;  // toAssetName isn't actually mapped from fromAsset
        }

        std::vector<K> currentKey = key->nonVersionVals;
        currentKey[0] = toAssetNames->front();
        return repo_.get(currentKey, toVersion, false);
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::unordered_map<K, T, KeyHash, KeyEqual> ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::replaceAll(const T& asset, const std::optional<Version>& fromVersion, const std::vector<std::optional<K>>& fromNonVersionVals, const std::optional<Version>& toVersion, const std::vector<K>& toAssetNames, bool errorOnNotFound) const {
        std::unordered_map<K, T, KeyHash, KeyEqual> result;

        std::optional<InternalKeyResult> key = getKeyInternal(asset, fromVersion, fromNonVersionVals, errorOnNotFound);
        if (!key.has_value()) {
            return result;
        }

        const K& fromAsset = key->nonVersionVals[0];
        std::optional<std::vector<K>> resolvedToAssetNames = resolveToAssetNames(fromAsset, toAssetNames);
        if (!resolvedToAssetNames.has_value()) {
            if (errorOnNotFound) {
                throw std::out_of_range("ModMappedAssets::replaceAll: the resolved asset's name is not present in the map");
            }
            return result;
        }

        std::vector<K> currentKey = key->nonVersionVals;
        for (const K& toAsset : *resolvedToAssetNames) {
            currentKey[0] = toAsset;
            std::optional<T> value = repo_.get(currentKey, toVersion, false);
            if (value.has_value()) {
                result.emplace(toAsset, std::move(*value));
            }
        }

        return result;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::vector<T> ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::getFromAssets() const {
        std::vector<T> result;
        result.reserve(keys_.size());
        for (const auto& entry : keys_) {
            result.push_back(entry.first);
        }
        return result;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    const ModDictAssets<K, T, KeyHash, KeyEqual>& ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::getRepo() const {
        return repo_;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    const std::unordered_map<K, std::vector<K>, KeyHash, KeyEqual>& ModMappedAssets<K, T, KeyHash, KeyEqual, ValueHash, ValueEqual>::getMap() const {
        return map_;
    }
}
