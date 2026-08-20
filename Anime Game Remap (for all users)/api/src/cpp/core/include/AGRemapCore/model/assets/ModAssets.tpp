#include <algorithm>
#include <limits>
#include <string>


namespace AGRemapCore {

    template <typename K, typename T, typename KeyEqual>
    ModAssets<K, T, KeyEqual>::ModAssets(std::vector<bool> isVersionColumn, VersionParser parseVersion, std::vector<Row<K, T>> rows):
        totalIndices_(isVersionColumn.size()), isVersionColumn_(std::move(isVersionColumn)), parseVersion_(std::move(parseVersion))
    {
        if (totalIndices_ == 0) {
            throw std::invalid_argument("ModAssets: isVersionColumn must have at least 1 element");
        }

        for (std::size_t i = 0; i < totalIndices_; ++i) {
            if (isVersionColumn_[i]) {
                versionColumnPositions_.push_back(i);
            } else {
                nonVersionColumnPositions_.push_back(i);
            }
        }

        addRows(std::move(rows));
    }

    template <typename K, typename T, typename KeyEqual>
    void ModAssets<K, T, KeyEqual>::addRow(const Row<K, T>& row) {
        if (row.indexVals.size() != totalIndices_) {
            throw std::invalid_argument("ModAssets::addRows: row has " + std::to_string(row.indexVals.size()) + " index values, expected " + std::to_string(totalIndices_));
        }

        StoredRow stored;
        stored.nonVersionVals.reserve(nonVersionColumnPositions_.size());
        for (std::size_t pos : nonVersionColumnPositions_) {
            stored.nonVersionVals.push_back(row.indexVals[pos]);
        }

        stored.versionVals.reserve(versionColumnPositions_.size());
        for (std::size_t pos : versionColumnPositions_) {
            std::optional<Version> parsed = parseVersion_(row.indexVals[pos]);
            if (!parsed.has_value()) {
                throw std::invalid_argument("ModAssets::addRows: failed to parse a version from a version column's value");
            }
            stored.versionVals.push_back(*parsed);
        }

        stored.value = row.value;

        KeyEqual keyEqual;
        auto matchesFullKey = [&](const StoredRow& existing) {
            for (std::size_t i = 0; i < stored.nonVersionVals.size(); ++i) {
                if (!keyEqual(existing.nonVersionVals[i], stored.nonVersionVals[i])) {
                    return false;
                }
            }
            for (std::size_t i = 0; i < stored.versionVals.size(); ++i) {
                if (existing.versionVals[i] != stored.versionVals[i]) {
                    return false;
                }
            }
            return true;
        };

        for (StoredRow& existing : rows_) {
            if (matchesFullKey(existing)) {
                existing.value = std::move(stored.value);
                return;
            }
        }

        rows_.push_back(std::move(stored));
    }

    template <typename K, typename T, typename KeyEqual>
    void ModAssets<K, T, KeyEqual>::addRows(std::vector<Row<K, T>> newRows) {
        for (const Row<K, T>& row : newRows) {
            addRow(row);
        }
    }

    template <typename K, typename T, typename KeyEqual>
    std::optional<T> ModAssets<K, T, KeyEqual>::get(const std::vector<std::optional<K>>& nonVersionVals, const std::vector<std::optional<Version>>& versionVals, bool errorOnNotFound) const {
        if (nonVersionVals.size() != nonVersionColumnPositions_.size()) {
            throw std::invalid_argument("ModAssets::get: expected " + std::to_string(nonVersionColumnPositions_.size()) + " non-version values, got " + std::to_string(nonVersionVals.size()));
        }
        if (versionVals.size() != versionColumnPositions_.size()) {
            throw std::invalid_argument("ModAssets::get: expected " + std::to_string(versionColumnPositions_.size()) + " version values, got " + std::to_string(versionVals.size()));
        }

        KeyEqual keyEqual;
        std::vector<const StoredRow*> candidates;
        for (const StoredRow& row : rows_) {
            bool matches = true;
            for (std::size_t i = 0; i < nonVersionVals.size(); ++i) {
                if (nonVersionVals[i].has_value() && !keyEqual(*nonVersionVals[i], row.nonVersionVals[i])) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                candidates.push_back(&row);
            }
        }

        if (candidates.empty()) {
            if (errorOnNotFound) {
                throw std::out_of_range("ModAssets::get: no matching entry found for the given non-version values");
            }
            return std::nullopt;
        }

        for (std::size_t vi = 0; vi < versionVals.size(); ++vi) {
            const std::optional<Version>& target = versionVals[vi];
            Version resolved = candidates.front()->versionVals[vi];

            if (!target.has_value()) {
                // Latest -- max across the current candidate set for this column.
                for (const StoredRow* c : candidates) {
                    if (c->versionVals[vi] > resolved) {
                        resolved = c->versionVals[vi];
                    }
                }
            } else {
                // Floor-match: the largest value <= target; if none qualifies, the smallest
                // value > target (the same "clamp up to the smallest available" fallback as
                // VersionSet::findClosest/ModDictAssets::get).
                std::optional<Version> maxLE;
                std::optional<Version> minGT;
                for (const StoredRow* c : candidates) {
                    const Version& v = c->versionVals[vi];
                    if (v <= *target) {
                        if (!maxLE.has_value() || v > *maxLE) {
                            maxLE = v;
                        }
                    } else {
                        if (!minGT.has_value() || v < *minGT) {
                            minGT = v;
                        }
                    }
                }
                resolved = maxLE.has_value() ? *maxLE : *minGT;
            }

            std::vector<const StoredRow*> narrowed;
            for (const StoredRow* c : candidates) {
                if (c->versionVals[vi] == resolved) {
                    narrowed.push_back(c);
                }
            }
            candidates = std::move(narrowed);

            if (candidates.empty()) {
                // Shouldn't happen (resolved was derived from the candidate set itself), but
                // guard anyway rather than dereferencing an empty range on the next iteration.
                if (errorOnNotFound) {
                    throw std::out_of_range("ModAssets::get: no matching entry found after resolving version columns");
                }
                return std::nullopt;
            }
        }

        return candidates.front()->value;
    }

    template <typename K, typename T, typename KeyEqual>
    std::size_t ModAssets<K, T, KeyEqual>::getTotalIndices() const {
        return totalIndices_;
    }

    template <typename K, typename T, typename KeyEqual>
    std::size_t ModAssets<K, T, KeyEqual>::getVersionColumnCount() const {
        return versionColumnPositions_.size();
    }

    template <typename K, typename T, typename KeyEqual>
    std::size_t ModAssets<K, T, KeyEqual>::getNonVersionColumnCount() const {
        return nonVersionColumnPositions_.size();
    }

    template <typename K, typename T, typename KeyEqual>
    std::size_t ModAssets<K, T, KeyEqual>::size() const {
        return rows_.size();
    }
}
