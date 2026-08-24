#include "AGRemapCore/model/strategies/texEditors/texFilters/InvertAlphaFilter.h"

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    void InvertAlphaFilter::transform(TextureFile &texFile) {
        int width = texFile.getWidth();
        int height = texFile.getHeight();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Colour pixel = texFile.getPixel(x, y);
                pixel.alpha = 255 - pixel.alpha;
                texFile.setPixel(x, y, pixel);
            }
        }
    }
}
