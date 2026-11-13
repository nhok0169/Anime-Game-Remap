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

#include "AGRemapCore/model/strategies/texEditors/texFilters/RegionFilter.h"

#include <cstdint>
#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"

namespace AGRemapCore {

    RegionFilter::RegionFilter(std::shared_ptr<BaseTexFilter> filter, Region region):
        filter(std::move(filter)), region(std::move(region)) {}

    void RegionFilter::transform(TextureFile &texFile) {
        if (!filter || !region) {
            return;
        }

        const long long width = texFile.getWidth();
        const long long height = texFile.getHeight();
        const std::vector<bool> texels = region(width, height);
        if (texels.empty() || texels.size() != static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {
            return;
        }

        const std::vector<std::uint8_t> before = texFile.getPixels();
        filter->transform(texFile);
        if (texFile.getWidth() != width || texFile.getHeight() != height) {
            texFile.setPixels(before, static_cast<int>(width), static_cast<int>(height));
            return;
        }

        std::vector<std::uint8_t> after = texFile.getPixels();
        for (std::size_t i = 0; i < texels.size() && i * 4 + 3 < after.size() && i * 4 + 3 < before.size(); ++i) {
            if (!texels[i]) {
                for (std::size_t c = 0; c < 4; ++c) {
                    after[i * 4 + c] = before[i * 4 + c];
                }
            }
        }
        texFile.setPixels(std::move(after), static_cast<int>(width), static_cast<int>(height));
    }
}
