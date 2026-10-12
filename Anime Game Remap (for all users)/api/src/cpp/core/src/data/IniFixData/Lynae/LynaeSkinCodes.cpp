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

#include "AGRemapCore/data/IniFixData/Lynae/LynaeSkinCodes.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <utility>
#include <string>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"


namespace AGRemapCore {
    namespace {
        // Measured over the two skins' game textures by the diffuse under each texel: flesh is
        // material 0 on 98% of Lynae's upper and lower body, material 4 on 94% / 82% of the skin's
        constexpr std::uint8_t LynaeSkin = 0;
        constexpr std::uint8_t PeppermintSkin = 4;

        std::uint8_t swapCode(std::uint8_t v) {
            const std::uint8_t material = v & 0x0F;
            const std::uint8_t flags = v & 0xF0;
            if (material == LynaeSkin) {
                return static_cast<std::uint8_t>(flags | PeppermintSkin);
            }
            if (material == PeppermintSkin) {
                return static_cast<std::uint8_t>(flags | LynaeSkin);
            }
            return v;
        }
    }

    TexEditor::Filter lynaeSkinCodeSwap() {
        return [](TextureFile& tex) {
            tex.setGamma(std::nullopt);          // these bytes are codes, not colour
            std::vector<std::uint8_t> px = tex.getPixels();

            // The shader reads the red channel; a single-channel source comes in as (v, v, v, 255),
            // and the copy is written the same way
            for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                const std::uint8_t v = swapCode(px[i]);
                px[i] = v;
                px[i + 1] = v;
                px[i + 2] = v;
            }

            tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
        };
    }

    TexEditor::Filter lynaeSheerBake(const WWMIFixerConfig::TexEditContext& ctx) {
        std::string codePath;
        const auto own = ctx.fileOfRole.find("lowerDetail");
        if (own != ctx.fileOfRole.end()) {
            codePath = own->second;
        } else {
            const auto game = ctx.gameFileOfRole.find("lowerDetail");
            if (game != ctx.gameFileOfRole.end()) {
                codePath = game->second;
            }
        }

        return [codePath](TextureFile& tex) {
            if (codePath.empty()) {
                return;
            }

            TextureFile codes(codePath);
            codes.open();
            if (!codes.hasImage()) {
                return;
            }

            // Measured on Lynae11 in the menu: her shader renders skin under code 1 at about
            // x(0.83, 0.89, 0.99) and greyer; a bake strong enough to read as stockings on the
            // skin's cloth shading is 90% toward the luminance, then x(0.84, 0.86, 0.93)
            constexpr double Desat = 0.9;
            constexpr double Gain[3] = {0.84, 0.86, 0.93};
            constexpr std::uint8_t SheerCode = 1;

            const int w = tex.getWidth();
            const int h = tex.getHeight();
            const int cw = codes.getWidth();
            const int ch = codes.getHeight();
            const std::vector<std::uint8_t>& code = codes.getPixels();
            std::vector<std::uint8_t> px = tex.getPixels();
            for (int y = 0; y < h; ++y) {
                const int cy = std::min(ch - 1, static_cast<int>((static_cast<long long>(y) * ch) / h));
                for (int x = 0; x < w; ++x) {
                    const std::size_t i = (static_cast<std::size_t>(y) * w + x) * 4;
                    const int cx = std::min(cw - 1, static_cast<int>((static_cast<long long>(x) * cw) / w));
                    const std::size_t c = (static_cast<std::size_t>(cy) * cw + cx) * 4;
                    if (i + 3 >= px.size() || c >= code.size() || (code[c] & 0x0F) != SheerCode) {
                        continue;
                    }

                    const int r = px[i];
                    const int g = px[i + 1];
                    const int b = px[i + 2];
                    if (r < 120 || r - g < 10 || g < b - 8) {
                        continue;                  // not skin: the stocking is sheer only over skin
                    }

                    const double lum = 0.299 * r + 0.587 * g + 0.114 * b;
                    const int in[3] = {r, g, b};
                    for (int k = 0; k < 3; ++k) {
                        const double v = (lum + (in[k] - lum) * (1.0 - Desat)) * Gain[k];
                        px[i + k] = static_cast<std::uint8_t>(std::lround(std::min(255.0, std::max(0.0, v))));
                    }
                }
            }

            tex.setPixels(std::move(px), w, h);
        };
    }

    TexEditor::Filter lynaeMaskRedFill() {
        return [](TextureFile& tex) {
            tex.setGamma(std::nullopt);          // a mask is codes, not colour
            std::vector<std::uint8_t> px = tex.getPixels();
            for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                px[i] = 255;
            }

            tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
        };
    }
}
