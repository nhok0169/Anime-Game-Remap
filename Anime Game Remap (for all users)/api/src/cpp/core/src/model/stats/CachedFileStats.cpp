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
