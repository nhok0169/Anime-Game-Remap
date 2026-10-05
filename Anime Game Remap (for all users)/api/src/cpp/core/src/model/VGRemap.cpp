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

#include "AGRemapCore/model/VGRemap.h"

#include <algorithm>
#include <utility>


namespace AGRemapCore {
    VGRemap::VGRemap(std::unordered_map<long long, long long> remap) {
        setRemap(std::move(remap));
    }

    const std::unordered_map<long long, long long>& VGRemap::getRemap() const {
        return remap_;
    }

    void VGRemap::setRemap(std::unordered_map<long long, long long> remap) {
        remap_ = std::move(remap);

        if (remap_.empty()) {
            maxIndex_ = std::nullopt;
            return;
        }

        long long maxIndex = remap_.begin()->first;
        for (const auto& [key, value] : remap_) {
            maxIndex = std::max(maxIndex, key);
        }
        maxIndex_ = maxIndex;
    }

    std::optional<long long> VGRemap::getMaxIndex() const {
        return maxIndex_;
    }
}
