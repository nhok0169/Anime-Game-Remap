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
#include <set>
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

    VGRemap::Compact VGRemap::compact(std::size_t size) const {
        std::set<long long> used;
        for (const auto& [src, dst] : remap_) {
            (void)src;
            if (dst >= 0 && static_cast<std::size_t>(dst) < size) {
                used.insert(dst);
            }
        }

        Compact result;
        result.forward.assign(size, 0);
        result.reverse.assign(size, 0);
        std::uint16_t local = 0;
        for (const long long bone : used) {
            result.forward[local] = static_cast<std::uint16_t>(bone);
            result.reverse[static_cast<std::size_t>(bone)] = local;
            ++local;
        }
        for (const auto& [src, dst] : remap_) {
            if (dst >= 0 && static_cast<std::size_t>(dst) < size) {
                result.localRow[src] = result.reverse[static_cast<std::size_t>(dst)];
            }
        }
        return result;
    }
}
