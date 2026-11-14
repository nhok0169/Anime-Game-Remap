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

#include "AGRemapCore/model/strategies/texEditors/texFilters/GuidedScaleFilter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"

namespace AGRemapCore {

    void GuidedScaleFilter::transform(TextureFile &texFile) {
        if (guide.empty() || factors.empty()) {
            return;
        }
        if (factors.size() != 256 || channel < 0 || channel > 3) {
            throw std::invalid_argument("GuidedScaleFilter: 'factors' must hold 256 entries and 'channel' be 0 to 3");
        }

        TextureFile guideTex(guide);
        guideTex.open();
        if (!guideTex.hasImage()) {
            return;
        }

        std::vector<std::uint8_t> values;
        if (read == ChannelMapFilter::Read::Raw) {
            const std::vector<std::uint8_t>& stored = guideTex.getPixels();
            values.resize(stored.size() / 4);
            for (std::size_t i = 0; i < values.size(); ++i) {
                values[i] = stored[i * 4 + channel];
            }
        } else {
            values = guideTex.shaderChannel(static_cast<std::size_t>(channel), read == ChannelMapFilter::Read::ShaderCode);
        }

        // per guide value, each channel's 256 results -- built the first time that value is met
        const bool decode = linear && texFile.getGamma().has_value();
        std::vector<std::optional<std::array<std::array<std::uint8_t, 256>, 3>>> tables(256);
        const auto tableOf = [&](std::uint8_t v) -> const std::array<std::array<std::uint8_t, 256>, 3>& {
            if (!tables[v].has_value()) {
                std::array<std::array<std::uint8_t, 256>, 3> table{};
                for (int k = 0; k < 3; ++k) {
                    for (int b = 0; b < 256; ++b) {
                        const double in = decode ? std::pow(b / 255.0, 2.2) * 255.0 : b;
                        const double scaled = std::min(255.0, std::max(0.0, in * factors[v][k]));
                        const double out = decode ? std::pow(scaled / 255.0, 1.0 / 2.2) * 255.0 : scaled;
                        table[k][b] = static_cast<std::uint8_t>(std::floor(out + 0.5));
                    }
                }
                tables[v] = table;
            }
            return *tables[v];
        };

        const int w = texFile.getWidth();
        const int h = texFile.getHeight();
        const int gw = guideTex.getWidth();
        const int gh = guideTex.getHeight();
        std::vector<std::uint8_t> px = texFile.getPixels();
        for (int y = 0; y < h; ++y) {
            const int gy = std::min(gh - 1, static_cast<int>((static_cast<long long>(y) * gh) / h));
            for (int x = 0; x < w; ++x) {
                const std::size_t i = (static_cast<std::size_t>(y) * w + x) * 4;
                const int gx = std::min(gw - 1, static_cast<int>((static_cast<long long>(x) * gw) / w));
                const std::size_t g = static_cast<std::size_t>(gy) * gw + gx;
                if (i + 3 >= px.size() || g >= values.size()) {
                    continue;
                }

                const std::array<double, 3>& factor = factors[values[g]];
                if (factor[0] == 1.0 && factor[1] == 1.0 && factor[2] == 1.0 && !decode) {
                    continue;
                }

                const auto& table = tableOf(values[g]);
                for (int k = 0; k < 3; ++k) {
                    px[i + k] = table[k][px[i + k]];
                }
            }
        }

        texFile.setPixels(std::move(px), w, h);
    }
}
