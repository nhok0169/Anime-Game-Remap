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

#include "AGRemapCore/model/strategies/texEditors/texFilters/PixelFilter.h"

#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"

namespace AGRemapCore {

    PixelFilter::PixelFilter(std::vector<Filter> transforms): transforms_(std::move(transforms)) {}

    const std::vector<PixelFilter::Filter>& PixelFilter::getTransforms() const {
        return transforms_;
    }

    void PixelFilter::setTransforms(std::vector<Filter> transforms) {
        transforms_ = std::move(transforms);
    }

    void PixelFilter::transform(TextureFile &texFile) {
        if (transforms_.empty()) {
            return;
        }

        int width = texFile.getWidth();
        int height = texFile.getHeight();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Colour pixel = texFile.getPixel(x, y);

                for (const auto &transform : transforms_) {
                    transform(pixel, x, y);
                }

                texFile.setPixel(x, y, pixel);
            }
        }
    }
}
