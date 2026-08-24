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
