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

#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"

#include <algorithm>
#include <cstddef>
#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"

namespace AGRemapCore {

    MaterialBandRemapFilter::Band::Band(std::uint8_t band, std::uint8_t to, ColourPredicate when, bool negate):
        low(band), high(band), to(to), when(std::move(when)), negate(negate) {}

    MaterialBandRemapFilter::Band::Band(std::uint8_t low, std::uint8_t high, std::uint8_t to, ColourPredicate when, bool negate):
        low(low), high(high), to(to), when(std::move(when)), negate(negate) {}


    MaterialBandRemapFilter::MaterialBandRemapFilter(std::vector<Band> bands, std::string diffusePath):
        bands(std::move(bands)), diffusePath(std::move(diffusePath)) {}


    bool MaterialBandRemapFilter::skinColoured(int red, int green, int blue) {
        return red >= 96 && red >= green && green >= blue && (red - blue) >= 16 && (red - blue) <= 140;
    }


    bool MaterialBandRemapFilter::whiteFurColoured(int red, int green, int blue) {
        const int high = std::max(red, std::max(green, blue));
        const int low = std::min(red, std::min(green, blue));
        return high >= 140 && (high - low) <= 70;
    }


    void MaterialBandRemapFilter::transform(TextureFile &texFile) {
        const int width = texFile.getWidth();
        const int height = texFile.getHeight();
        std::vector<std::uint8_t> pixels = texFile.getPixels();
        if (width <= 0 || height <= 0 || pixels.size() < static_cast<std::size_t>(width) * height * 4) {
            return;
        }

        // Only read the diffuse when some band actually gates on it.
        const bool gated = std::any_of(bands.begin(), bands.end(), [](const Band& band) {
            return static_cast<bool>(band.when);
        });

        std::vector<std::uint8_t> diffuse;
        int diffuseWidth = 0;
        int diffuseHeight = 0;
        if (gated && !diffusePath.empty()) {
            TextureFile diffuseFile(diffusePath);
            diffuseFile.open();
            if (diffuseFile.hasImage()) {
                diffuse = diffuseFile.getPixels();
                diffuseWidth = diffuseFile.getWidth();
                diffuseHeight = diffuseFile.getHeight();
            }
        }

        const bool haveDiffuse = !diffuse.empty() && diffuseWidth > 0 && diffuseHeight > 0;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::uint8_t& alpha = pixels[(static_cast<std::size_t>(y) * width + x) * 4 + 3];

                // EVERY test reads 'was', never a value a previous band wrote -- the moves are a
                // permutation and applied in sequence a permutation chases itself.
                const std::uint8_t was = alpha;

                int red = 0;
                int green = 0;
                int blue = 0;
                bool known = false;
                if (haveDiffuse) {
                    const int dx = static_cast<int>(static_cast<long long>(x) * diffuseWidth / width);
                    const int dy = static_cast<int>(static_cast<long long>(y) * diffuseHeight / height);
                    const std::size_t at = (static_cast<std::size_t>(dy) * diffuseWidth + dx) * 4;
                    if (at + 2 < diffuse.size()) {
                        red = diffuse[at];
                        green = diffuse[at + 1];
                        blue = diffuse[at + 2];
                        known = true;
                    }
                }

                for (const Band& band : bands) {
                    if (was < band.low || was > band.high) {
                        continue;
                    }

                    // No diffuse to check against: the band legend is the better of the two
                    // guesses, so the gate passes rather than blocking the move.
                    bool passes = true;
                    if (band.when && known) {
                        passes = band.when(red, green, blue);
                        if (band.negate) {
                            passes = !passes;
                        }
                    }

                    if (passes) {
                        alpha = band.to;
                    }

                    // The FIRST band containing this pixel decides, whether or not its gate passed
                    // -- a later band whose range also contains it describes a different material.
                    break;
                }
            }
        }

        texFile.setPixels(std::move(pixels), width, height);
    }


    std::function<TexEditor::Filter(const std::string&)> MaterialBandRemapFilter::lightMapEdit(std::vector<Band> bands) {
        return [bands = std::move(bands)](const std::string& diffusePath) -> TexEditor::Filter {
            return [bands, diffusePath](TextureFile& texFile) {
                MaterialBandRemapFilter(bands, diffusePath).transform(texFile);
            };
        };
    }
}
