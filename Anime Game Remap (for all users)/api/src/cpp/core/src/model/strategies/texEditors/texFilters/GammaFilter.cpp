#include "AGRemapCore/model/strategies/texEditors/texFilters/GammaFilter.h"

#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/CorrectGamma.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    GammaFilter::GammaFilter(double gamma): gamma(gamma) {}

    void GammaFilter::transform(TextureFile &texFile) {
        CorrectGamma correctGamma(gamma);
        int width = texFile.getWidth();
        int height = texFile.getHeight();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Colour pixel = texFile.getPixel(x, y);
                correctGamma.transform(pixel, x, y);
                texFile.setPixel(x, y, pixel);
            }
        }
    }
}
