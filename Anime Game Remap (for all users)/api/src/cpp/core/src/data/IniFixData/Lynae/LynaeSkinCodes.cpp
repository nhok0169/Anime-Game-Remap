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
#include <array>
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

    namespace {
        using CodeRatios = std::array<std::array<double, 3>, 16>;

        // The code map a bake of 'detailRole' reads: the mod's own when it binds one by default, else
        // Lynae's game map (a role the mod binds only under a toggle is left out of fileOfRole)
        std::string codeMapOf(const WWMIFixerConfig::TexEditContext& ctx, const std::string& detailRole) {
            const auto own = ctx.fileOfRole.find(detailRole);
            if (own != ctx.fileOfRole.end()) {
                return own->second;
            }

            const auto game = ctx.gameFileOfRole.find(detailRole);
            return game == ctx.gameFileOfRole.end() ? std::string() : game->second;
        }

        // Every texel whose material (low four bits of the code map, sampled at the texel's UV) has a
        // ratio other than 1 is multiplied by it -- only flesh-coloured texels when 'fleshOnly'
        TexEditor::Filter codeRatioBake(const std::string& codePath, const CodeRatios& ratios, bool fleshOnly) {
            return [codePath, ratios, fleshOnly](TextureFile& tex) {
                if (codePath.empty()) {
                    return;
                }

                TextureFile codes(codePath);
                codes.open();
                if (!codes.hasImage()) {
                    return;
                }

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
                        if (i + 3 >= px.size() || c >= code.size()) {
                            continue;
                        }

                        const std::array<double, 3>& ratio = ratios[code[c] & 0x0F];
                        if (ratio[0] == 1.0 && ratio[1] == 1.0 && ratio[2] == 1.0) {
                            continue;
                        }

                        const int in[3] = {px[i], px[i + 1], px[i + 2]};
                        if (fleshOnly && (in[0] < 120 || in[0] - in[1] < 10 || in[1] < in[2] - 8)) {
                            continue;
                        }

                        for (int k = 0; k < 3; ++k) {
                            const double v = in[k] * ratio[k];
                            px[i + k] = static_cast<std::uint8_t>(std::lround(std::min(255.0, std::max(0.0, v))));
                        }
                    }
                }

                tex.setPixels(std::move(px), w, h);
            };
        }

        CodeRatios identityRatios() {
            CodeRatios out{};
            for (auto& r : out) {
                r = {1.0, 1.0, 1.0};
            }

            return out;
        }
    }

    TexEditor::Filter lynaeSheerBake(const WWMIFixerConfig::TexEditContext& ctx) {
        // Per Lynae code: the median leg colour Lynae renders with that code bound flat, over the median
        // LynaePeppermint renders with the code it becomes after lynaeSkinCodeSwap (menu, Lynae11,
        // 2026-10-12). Code 0 is skin on Lynae and stays as it is
        CodeRatios ratios = identityRatios();
        ratios[1] = {0.866, 0.884, 0.935};
        ratios[2] = {0.910, 1.000, 1.043};
        ratios[3] = {0.883, 0.903, 0.918};
        ratios[4] = {0.890, 0.987, 1.022};
        return codeRatioBake(codeMapOf(ctx, "lowerDetail"), ratios, true);
    }

    TexEditor::Filter lynaeFilmBake(const WWMIFixerConfig::TexEditContext& ctx) {
        // The coat lining, Lynae code 4: her render of it (a pink-red film) over LynaePeppermint's with the
        // code moved to 49 by lynaeFilmCode (menu, Lynae11 -- whose coat texture is Lynae's own, 2026-10-12)
        CodeRatios ratios = identityRatios();
        ratios[4] = {2.0, 0.89, 0.88};
        return codeRatioBake(codeMapOf(ctx, "jacketDetail"), ratios, false);
    }

    TexEditor::Filter lynaeFilmCode() {
        return [](TextureFile& tex) {
            tex.setGamma(std::nullopt);          // codes, not colour
            std::vector<std::uint8_t> px = tex.getPixels();
            for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                if ((px[i] & 0x0F) == 4) {
                    px[i] = px[i + 1] = px[i + 2] = 49;   // LynaePeppermint's coat film: material 1, flags 3
                }
            }

            tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
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
