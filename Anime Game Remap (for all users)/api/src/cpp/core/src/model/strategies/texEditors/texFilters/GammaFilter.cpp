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

#include "AGRemapCore/model/strategies/texEditors/texFilters/GammaFilter.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/CorrectGamma.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    GammaFilter::GammaFilter(double gamma): gamma(gamma) {}

    void GammaFilter::transform(TextureFile &texFile) {
        int width = texFile.getWidth();
        int height = texFile.getHeight();

        // CorrectGamma::correctGamma is a static, pure function of (channel value, gamma), and a
        // channel value is 8 bits -- so the whole filter has only 256 possible answers. Computing
        // them once turns three std::pow calls PER PIXEL into three table lookups: a 4096x4096
        // texture went from 67 million pow() calls to 256.
        //
        // Byte-identical by construction rather than by approximation: the table is filled by
        // calling the very same function that used to be called per pixel, so every output value
        // is the one it always was.
        //
        // Stored already narrowed to a byte, because that is what the write did: setPixel casts
        // (it does NOT clamp), so a table entry outside 0-255 has always been truncated. Keeping
        // the cast here rather than dropping it is what makes this rewrite byte-identical instead
        // of merely nearly so.
        std::array<std::uint8_t, 256> corrected{};
        for (int value = 0; value < 256; ++value) {
            corrected[static_cast<std::size_t>(value)] =
                static_cast<std::uint8_t>(CorrectGamma::correctGamma(value, gamma));
        }

        // ONE LINEAR PASS OVER THE BUFFER, rather than getPixel/setPixel per pixel.
        //
        // Those two are tiny, but they live in another translation unit and this build does not
        // use LTO (see Building's "Build speed" -- it is off for python_dev on purpose), so each
        // one is a real call: 33 million of them for a 4096x4096 texture, plus 50 million more for
        // the per-channel bounds check. Measured at 0.30s, which -- once the BC7 decode stopped
        // being 2.4s -- was the largest thing left in a texture edit.
        //
        // The bounds check goes with them and loses nothing: the values come out of an RGBA8
        // buffer, so they are already 0-255 and clamping them is a no-op. Colour is not built at
        // all now, which is the other half of the saving.
        std::vector<std::uint8_t> pixels = texFile.getPixels();
        if (pixels.empty() || width <= 0 || height <= 0) {
            return;
        }

        // The old loop visited exactly width*height pixels and left anything past them alone.
        const std::size_t covered =
            std::min(pixels.size(), static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u);

        for (std::size_t at = 0; at + 3u < covered; at += 4u) {
            // Alpha is deliberately untouched, exactly as CorrectGamma::transform left it --
            // in these textures it is very often a mask rather than opacity.
            pixels[at] = corrected[pixels[at]];
            pixels[at + 1u] = corrected[pixels[at + 1u]];
            pixels[at + 2u] = corrected[pixels[at + 2u]];
        }

        texFile.setPixels(std::move(pixels), width, height);
    }
}
