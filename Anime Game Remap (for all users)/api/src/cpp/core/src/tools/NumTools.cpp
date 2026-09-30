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


#include "AGRemapCore/tools/NumTools.h"

#include <algorithm>
#include <array>
#include <cstdio>


namespace AGRemapCore {

    std::string NumTools::formatDouble(double value, int decimals) {
        // snprintf rather than a stream: the "%.*f" conversion is locale-dependent only through the
        // decimal point, and the C locale this library runs in writes it as '.', which is what a
        // .ini has to hold whatever machine produced it
        const int places = std::max(0, decimals);
        std::array<char, 64> buffer{};
        const int written = std::snprintf(buffer.data(), buffer.size(), "%.*f", places, value);
        if (written <= 0) {
            return "0";
        }

        std::string out(buffer.data(), static_cast<std::size_t>(std::min(written, static_cast<int>(buffer.size() - 1))));
        if (out.find('.') == std::string::npos) {
            return out;
        }

        while (!out.empty() && out.back() == '0') {
            out.pop_back();
        }

        if (!out.empty() && out.back() == '.') {
            out.pop_back();
        }

        return out.empty() ? "0" : out;
    }
}
