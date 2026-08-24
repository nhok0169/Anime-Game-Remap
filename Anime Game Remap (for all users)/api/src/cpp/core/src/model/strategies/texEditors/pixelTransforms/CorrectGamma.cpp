#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/CorrectGamma.h"

#include <cmath>

#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    CorrectGamma::CorrectGamma(double gamma): gamma(gamma) {}

    int CorrectGamma::correctGamma(int pixelValue, double gamma) {
        double normalized = static_cast<double>(pixelValue) / 255.0;
        double corrected = std::pow(normalized, 1.0 / gamma) * 255.0;
        return Colour::boundColourChannel(static_cast<int>(std::lround(corrected)));
    }

    void CorrectGamma::transform(Colour &pixel, int, int) {
        pixel.red = correctGamma(pixel.red, gamma);
        pixel.green = correctGamma(pixel.green, gamma);
        pixel.blue = correctGamma(pixel.blue, gamma);
    }
}
