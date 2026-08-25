#include "AGRemapCore/model/stats/CachedFileStats.h"


namespace AGRemapCore {
    void CachedFileStats::clear() {
        FileStats::clear();
        hit.clear();
    }

    void CachedFileStats::updateHit(const std::unordered_set<std::string>& newHit) {
        hit.insert(newHit.begin(), newHit.end());
    }

    void CachedFileStats::addHit(const std::string& filePath) {
        hit.insert(filePath);
    }

    void CachedFileStats::update(std::optional<std::string> modFolder, std::optional<std::unordered_set<std::string>> newFixed,
                                  std::optional<std::unordered_map<std::string, std::exception_ptr>> newSkipped,
                                  std::optional<std::unordered_set<std::string>> newRemoved,
                                  std::optional<std::unordered_set<std::string>> newUndoed,
                                  std::optional<std::unordered_set<std::string>> newVisitedAtRemoval,
                                  std::optional<std::unordered_set<std::string>> newHit) {
        FileStats::update(modFolder, newFixed, newSkipped, newRemoved, newUndoed, newVisitedAtRemoval);

        if (newHit.has_value()) {
            updateHit(*newHit);
        }
    }
}
