#include <algorithm>
#include <string>
#include <utility>


namespace AGRemapCore {

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    std::size_t ModDictAssets<K, T, KeyHash, KeyEqual>::KeyVecHash::operator()(const std::vector<K>& keys) const {
        std::size_t seed = 0;
        KeyHash hasher;
        for (const K& key : keys) {
            seed ^= hasher(key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    bool ModDictAssets<K, T, KeyHash, KeyEqual>::KeyVecEqual::operator()(const std::vector<K>& a, const std::vector<K>& b) const {
        if (a.size() != b.size()) {
            return false;
        }
        KeyEqual equal;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (!equal(a[i], b[i])) {
                return false;
            }
        }
        return true;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    ModDictAssets<K, T, KeyHash, KeyEqual>::ModDictAssets(std::size_t totalIndices, std::size_t versionIndexPos, VersionParser parseVersion, std::vector<Row<K, T>> rows):
        totalIndices_(totalIndices), versionIndexPos_(versionIndexPos), parseVersion_(std::move(parseVersion))
    {
        if (totalIndices_ == 0) {
            throw std::invalid_argument("ModDictAssets: totalIndices must be at least 1");
        }
        if (versionIndexPos_ >= totalIndices_) {
            throw std::invalid_argument("ModDictAssets: versionIndexPos (" + std::to_string(versionIndexPos_) + ") is out of range for totalIndices (" + std::to_string(totalIndices_) + ")");
        }

        addRows(std::move(rows));
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    std::vector<K> ModDictAssets<K, T, KeyHash, KeyEqual>::extractNonVersionVals(const std::vector<K>& indexVals) const {
        std::vector<K> result;
        result.reserve(indexVals.size() - 1);
        for (std::size_t i = 0; i < indexVals.size(); ++i) {
            if (i != versionIndexPos_) {
                result.push_back(indexVals[i]);
            }
        }
        return result;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    void ModDictAssets<K, T, KeyHash, KeyEqual>::addRows(std::vector<Row<K, T>> newRows) {
        for (Row<K, T>& row : newRows) {
            if (row.indexVals.size() != totalIndices_) {
                throw std::invalid_argument("ModDictAssets::addRows: row has " + std::to_string(row.indexVals.size()) + " index values, expected " + std::to_string(totalIndices_));
            }

            std::optional<Version> version = parseVersion_(row.indexVals[versionIndexPos_]);
            if (!version.has_value()) {
                throw std::invalid_argument("ModDictAssets::addRows: failed to parse a version from the row's version index value");
            }

            std::vector<K> nonVersionVals = extractNonVersionVals(row.indexVals);
            std::vector<VersionedEntry>& entries = groups_[nonVersionVals];

            auto pos = std::lower_bound(entries.begin(), entries.end(), *version, [](const VersionedEntry& entry, const Version& target) {
                return entry.version < target;
            });

            if (pos != entries.end() && pos->version == *version) {
                pos->value = std::move(row.value);  // matches _updateAssetContent: new data replaces old for an identical full key
            } else {
                entries.insert(pos, VersionedEntry{*version, std::move(row.value)});
            }
        }
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    std::optional<T> ModDictAssets<K, T, KeyHash, KeyEqual>::get(const std::vector<K>& nonVersionVals, const std::optional<Version>& version, bool errorOnNotFound) const {
        if (nonVersionVals.size() != totalIndices_ - 1) {
            throw std::invalid_argument("ModDictAssets::get: expected " + std::to_string(totalIndices_ - 1) + " non-version values, got " + std::to_string(nonVersionVals.size()));
        }

        auto groupIt = groups_.find(nonVersionVals);
        if (groupIt == groups_.end() || groupIt->second.empty()) {
            if (errorOnNotFound) {
                throw std::out_of_range("ModDictAssets::get: no matching entry found for the given non-version values");
            }
            return std::nullopt;
        }

        const std::vector<VersionedEntry>& entries = groupIt->second;  // sorted ascending by version

        if (!version.has_value()) {
            return entries.back().value;
        }

        auto pos = std::lower_bound(entries.begin(), entries.end(), *version, [](const VersionedEntry& entry, const Version& target) {
            return entry.version < target;
        });

        if (pos != entries.end() && pos->version == *version) {
            return pos->value;
        }
        if (pos != entries.begin()) {
            return (pos - 1)->value;
        }
        return entries.front().value;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    std::size_t ModDictAssets<K, T, KeyHash, KeyEqual>::getTotalIndices() const {
        return totalIndices_;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    std::size_t ModDictAssets<K, T, KeyHash, KeyEqual>::getVersionIndexPos() const {
        return versionIndexPos_;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    std::size_t ModDictAssets<K, T, KeyHash, KeyEqual>::size() const {
        std::size_t total = 0;
        for (const auto& group : groups_) {
            total += group.second.size();
        }
        return total;
    }

    template <typename K, typename T, typename KeyHash, typename KeyEqual>
    template <typename Visitor>
    void ModDictAssets<K, T, KeyHash, KeyEqual>::forEachEntry(Visitor&& visitor) const {
        for (const auto& group : groups_) {
            for (const VersionedEntry& entry : group.second) {
                visitor(group.first, entry.version, entry.value);
            }
        }
    }
}
