#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/TempControl.h"

#include <cmath>

namespace AGRemapCore {

    namespace {
        constexpr double PaintTempIncRedFactor = 0.41;
        constexpr double PaintTempIncBlueFactor = 0.44;
        constexpr double PaintTempDecRedFactor = 0.5;
        constexpr double PaintTempDecBlueFactor = 2.0;
    }

    TempControl::TempControl(double temp):
        temp(temp),
        redFactor_(temp >= 0 ? PaintTempIncRedFactor : PaintTempDecRedFactor),
        blueFactor_(temp >= 0 ? PaintTempIncBlueFactor : PaintTempDecBlueFactor) {}

    void TempControl::transform(Colour &pixel, int, int) {
        pixel.red = Colour::boundColourChannel(static_cast<int>(std::lround(pixel.red + temp * redFactor_ * pixel.red)));
        pixel.blue = Colour::boundColourChannel(static_cast<int>(std::lround(pixel.blue - temp * blueFactor_ * pixel.blue)));
    }
}
