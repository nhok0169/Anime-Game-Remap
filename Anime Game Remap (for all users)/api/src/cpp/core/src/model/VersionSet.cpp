#include "AGRemapCore/model/VersionSet.h"

#include <algorithm>


namespace AGRemapCore {

    VersionSet::VersionSet(std::size_t cacheCapacity): cache_(cacheCapacity) {}

    void VersionSet::updateLatestVersion(const Version& newVersion) {
        if (!latestVersion_.has_value() || newVersion > *latestVersion_) {
            latestVersion_ = newVersion;
        }
    }

    void VersionSet::add(const Version& newVersion) {
        if (versions_.empty() || newVersion > versions_.back()) {
            versions_.push_back(newVersion);
        } else if (newVersion < versions_.front()) {
            versions_.insert(versions_.begin(), newVersion);
        } else {
            auto it = std::lower_bound(versions_.begin(), versions_.end(), newVersion);
            if (it == versions_.end() || *it != newVersion) {
                versions_.insert(it, newVersion);
            }
        }

        updateLatestVersion(newVersion);
    }

    void VersionSet::clear() {
        versions_.clear();
        latestVersion_.reset();
        cache_.clear();
    }

    const std::optional<Version>& VersionSet::getLatestVersion() const {
        return latestVersion_;
    }

    const std::vector<Version>& VersionSet::getVersions() const {
        return versions_;
    }

    Version VersionSet::findClosestFromSorted(const std::vector<Version>& versions, const Version& version) {
        auto it = std::lower_bound(versions.begin(), versions.end(), version);

        if (it != versions.end() && *it == version) {
            return *it;
        }
        if (it != versions.begin()) {
            return *(it - 1);
        }
        return versions.front();
    }

    std::optional<Version> VersionSet::findClosest(const std::optional<Version>& version, bool fromCache) {
        if (!latestVersion_.has_value()) {
            return std::nullopt;
        }

        if (!version.has_value()) {
            return latestVersion_;
        }

        if (fromCache) {
            std::optional<Version> cached = cache_.get(*version);
            if (cached.has_value()) {
                return cached;
            }
        }

        Version result = findClosestFromSorted(versions_, *version);
        cache_.put(*version, result);
        return result;
    }
}
