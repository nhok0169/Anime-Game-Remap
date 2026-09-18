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

#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    Colour::Colour(int red, int green, int blue, int alpha):
        red(boundColourChannel(red)), green(boundColourChannel(green)),
        blue(boundColourChannel(blue)), alpha(boundColourChannel(alpha)) {}

    int Colour::boundColourChannel(int val, int min, int max) {
        if (val > max) {
            val = max;
        } else if (val < min) {
            val = min;
        }
        return val;
    }

    int Colour::boolToColourChannel(bool val, int min, int max) {
        return val ? max : min;
    }

    std::size_t Colour::hash() const {
        return std::hash<std::string>{}(getId());
    }

    void Colour::fromTuple(const std::tuple<int, int, int, int> &colourTuple) {
        red = std::get<0>(colourTuple);
        green = std::get<1>(colourTuple);
        blue = std::get<2>(colourTuple);
        alpha = std::get<3>(colourTuple);
    }

    std::tuple<int, int, int, int> Colour::getTuple() const {
        return std::make_tuple(red, green, blue, alpha);
    }

    std::string Colour::getId() const {
        return std::to_string(red) + std::to_string(green) + std::to_string(blue) + std::to_string(alpha);
    }

    void Colour::copy(const Colour &colour, bool withAlpha) {
        red = colour.red;
        green = colour.green;
        blue = colour.blue;

        if (withAlpha) {
            alpha = colour.alpha;
        }
    }

    bool Colour::match(const Colour &colour) const {
        return colour.red == red && colour.green == green && colour.blue == blue && colour.alpha == alpha;
    }
}
