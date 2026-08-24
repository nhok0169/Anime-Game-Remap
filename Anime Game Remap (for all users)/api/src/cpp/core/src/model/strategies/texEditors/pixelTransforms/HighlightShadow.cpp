#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/HighlightShadow.h"

#include <cmath>

namespace AGRemapCore {

    HighlightShadow::HighlightShadow(double highlight, double shadow): highlight(highlight), shadow(shadow) {}

    void HighlightShadow::transform(Colour &pixel, int, int) {
        constexpr double lumR = 0.299;
        constexpr double lumG = 0.587;
        constexpr double lumB = 0.114;

        double normRed = pixel.red / 255.0;
        double normGreen = pixel.green / 255.0;
        double normBlue = pixel.blue / 255.0;

        double luminance = std::sqrt(lumR * std::pow(normRed, 2.0) + lumG * std::pow(normGreen, 2.0) + lumB * std::pow(normBlue, 2.0));

        double h = highlight * 0.07 * (std::pow(18.0, luminance) - 1.0);
        double s = shadow * 0.07 * (std::pow(18.0, 1.0 - luminance) - 1.0);

        pixel.red = Colour::boundColourChannel(static_cast<int>(std::lround((normRed + h + s) * 255.0)));
        pixel.green = Colour::boundColourChannel(static_cast<int>(std::lround((normGreen + h + s) * 255.0)));
        pixel.blue = Colour::boundColourChannel(static_cast<int>(std::lround((normBlue + h + s) * 255.0)));
    }
}
