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

        // A map stored sRGB reaches the shader DECODED: the sampler linearises each byte, and the
        // shader truncates value * 255 to its code (Lynae9's 250 is 243, material 3, on her own card:
        // rounding to 244 rendered her tights paler). The edits below write linear bytes, so they
        // decode an sRGB source first -- 'truncate' for codes, nearest for a mask's other channels
        std::array<std::uint8_t, 256> srgbDecodeTable(bool truncate) {
            std::array<std::uint8_t, 256> table{};
            for (int i = 0; i < 256; ++i) {
                const double c = i / 255.0;
                const double linear = (c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4)) * 255.0;
                table[i] = static_cast<std::uint8_t>(std::min(255.0, std::floor(linear + (truncate ? 1e-6 : 0.5))));
            }

            return table;
        }

        const std::array<std::uint8_t, 256>& srgbCodes() {
            static const std::array<std::uint8_t, 256> table = srgbDecodeTable(true);
            return table;
        }

        const std::array<std::uint8_t, 256>& srgbValues() {
            static const std::array<std::uint8_t, 256> table = srgbDecodeTable(false);
            return table;
        }

        // The red-channel codes of 'tex' as its shader reads them: decoded when the source was sRGB
        // (TextureFile sets a gamma exactly then), the bytes otherwise
        std::vector<std::uint8_t> shaderCodes(const TextureFile& tex) {
            const std::vector<std::uint8_t>& px = tex.getPixels();
            const bool srgb = tex.getGamma().has_value();
            std::vector<std::uint8_t> codes(px.size() / 4);
            for (std::size_t i = 0; i < codes.size(); ++i) {
                codes[i] = srgb ? srgbCodes()[px[i * 4]] : px[i * 4];
            }

            return codes;
        }
    }

    TexEditor::Filter lynaeSkinCodeSwap() {
        return [](TextureFile& tex) {
            const std::vector<std::uint8_t> codes = shaderCodes(tex);
            tex.setGamma(std::nullopt);          // these bytes are codes, not colour
            std::vector<std::uint8_t> px = tex.getPixels();

            // The shader reads the red channel; a single-channel source comes in as (v, v, v, 255),
            // and the copy is written the same way
            for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                const std::uint8_t v = swapCode(codes[i / 4]);
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
        // ratio other than 1 is multiplied by it. What a texel IS comes from the code map alone, never
        // from its colour: a mod may paint skin any colour (dark, blue), and a test on the diffuse would
        // misread it -- one such test took Lynae14's gold filigree for skin and turned it grey-green
        TexEditor::Filter codeRatioBake(const std::string& codePath, const CodeRatios& ratios) {
            return [codePath, ratios](TextureFile& tex) {
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
                const std::vector<std::uint8_t> code = shaderCodes(codes);
                std::vector<std::uint8_t> px = tex.getPixels();
                for (int y = 0; y < h; ++y) {
                    const int cy = std::min(ch - 1, static_cast<int>((static_cast<long long>(y) * ch) / h));
                    for (int x = 0; x < w; ++x) {
                        const std::size_t i = (static_cast<std::size_t>(y) * w + x) * 4;
                        const int cx = std::min(cw - 1, static_cast<int>((static_cast<long long>(x) * cw) / w));
                        const std::size_t c = static_cast<std::size_t>(cy) * cw + cx;
                        if (i + 3 >= px.size() || c >= code.size()) {
                            continue;
                        }

                        const std::array<double, 3>& ratio = ratios[code[c] & 0x0F];
                        if (ratio[0] == 1.0 && ratio[1] == 1.0 && ratio[2] == 1.0) {
                            continue;
                        }

                        const int in[3] = {px[i], px[i + 1], px[i + 2]};
                        for (int k = 0; k < 3; ++k) {
                            const double v = in[k] * ratio[k];
                            px[i + k] = static_cast<std::uint8_t>(std::lround(std::min(255.0, std::max(0.0, v))));
                        }
                    }
                }

                tex.setPixels(std::move(px), w, h);
            };
        }

        // Lynae's lower-body shader reads the NORMAL MAP's alpha as a shading band; LynaePeppermint's
        // does not. Per alpha, the multiplier (in linear colour) that makes LynaePeppermint land where Lynae
        // does: a flat grey 120 under code 1 on both cards, the normal map flat at each alpha (menu, Lynae14,
        // 2026-10-12). From 128 up she renders as the skin does; around 102 (Lynae14's dress) a third as bright
        // (0.39 at grey 120; 0.46 after one closed-loop round on the dress's own red, darker inputs dipping less)
        double shadeBandMultiplier(int alpha) {
            static const std::pair<double, double> Points[] = {
                {0, 0.88}, {51, 0.78}, {72, 0.77}, {84, 0.80}, {96, 0.87}, {102, 0.46}, {108, 0.57},
                {114, 0.77}, {120, 0.93}, {128, 1.0}, {255, 1.0}};
            for (std::size_t i = 1; i < std::size(Points); ++i) {
                if (alpha <= Points[i].first) {
                    const double t = (alpha - Points[i - 1].first) / (Points[i].first - Points[i - 1].first);
                    return Points[i - 1].second + t * (Points[i].second - Points[i - 1].second);
                }
            }
            return 1.0;
        }

        // Every texel's colour scaled, in linear, by the band its normal map's alpha (sampled at the texel's
        // UV) puts it in. A buffer read from an sRGB source holds gamma-encoded bytes (TextureFile::save
        // pre-corrects by the same 2.2), anything else linear ones
        TexEditor::Filter shadeBandBake(const std::string& normalPath) {
            return [normalPath](TextureFile& tex) {
                if (normalPath.empty()) {
                    return;
                }

                TextureFile normals(normalPath);
                normals.open();
                if (!normals.hasImage()) {
                    return;
                }

                const bool srgb = tex.getGamma().has_value();
                std::vector<std::array<std::uint8_t, 256>> table(256);
                for (int a = 0; a < 256; ++a) {
                    const double r = shadeBandMultiplier(a);
                    for (int b = 0; b < 256; ++b) {
                        const double lin = srgb ? std::pow(b / 255.0, 2.2) * 255.0 : b;
                        const double y = std::min(255.0, lin * r);
                        const double o = srgb ? std::pow(y / 255.0, 1.0 / 2.2) * 255.0 : y;
                        table[a][b] = static_cast<std::uint8_t>(std::floor(o + 0.5));
                    }
                }

                const int w = tex.getWidth();
                const int h = tex.getHeight();
                const int nw = normals.getWidth();
                const int nh = normals.getHeight();
                const std::vector<std::uint8_t>& nrm = normals.getPixels();
                std::vector<std::uint8_t> px = tex.getPixels();
                for (int y = 0; y < h; ++y) {
                    const int ny = std::min(nh - 1, static_cast<int>((static_cast<long long>(y) * nh) / h));
                    for (int x = 0; x < w; ++x) {
                        const std::size_t i = (static_cast<std::size_t>(y) * w + x) * 4;
                        const int nx = std::min(nw - 1, static_cast<int>((static_cast<long long>(x) * nw) / w));
                        const std::size_t n = (static_cast<std::size_t>(ny) * nw + nx) * 4 + 3;
                        if (i + 3 >= px.size() || n >= nrm.size()) {
                            continue;
                        }

                        const std::array<std::uint8_t, 256>& row = table[nrm[n]];
                        px[i] = row[px[i]];
                        px[i + 1] = row[px[i + 1]];
                        px[i + 2] = row[px[i + 2]];
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
        // No per-material colour ratio: her stocking materials (codes 1-4) share every non-colour input
        // with ordinary cloth -- Lynae14 paints 88% of its atlas code 1, fur, gold and dresses alike, on
        // the same normal-alpha band and mask values as her legs -- so a ratio could only be aimed by
        // the diffuse's colour, which a mod is free to paint any way it likes. Her legs stay a little
        // warmer than on Lynae (the maintainer's call, 2026-10-12)
        return shadeBandBake(codeMapOf(ctx, "lowerNormal"));
    }

    TexEditor::Filter lynaeFilmBake(const WWMIFixerConfig::TexEditContext& ctx) {
        // The coat lining, Lynae code 4: her render of it (a pink-red film) over LynaePeppermint's with the
        // code moved to 49 by lynaeFilmCode (menu, Lynae11 -- whose coat texture is Lynae's own, 2026-10-12)
        CodeRatios ratios = identityRatios();
        ratios[4] = {2.0, 0.89, 0.88};
        return codeRatioBake(codeMapOf(ctx, "jacketDetail"), ratios);
    }

    TexEditor::Filter lynaeFilmCode() {
        return [](TextureFile& tex) {
            const std::vector<std::uint8_t> codes = shaderCodes(tex);
            tex.setGamma(std::nullopt);          // codes, not colour
            std::vector<std::uint8_t> px = tex.getPixels();
            for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                const std::uint8_t v = codes[i / 4];
                // LynaePeppermint's coat film: material 1, flags 3
                px[i] = px[i + 1] = px[i + 2] = (v & 0x0F) == 4 ? 49 : v;
            }

            tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
        };
    }

    TexEditor::Filter lynaePropsPlainCode() {
        return [](TextureFile& tex) {
            const std::vector<std::uint8_t> codes = shaderCodes(tex);
            tex.setGamma(std::nullopt);          // codes, not colour
            std::vector<std::uint8_t> px = tex.getPixels();
            for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                // her film (flag 0x10: her own sweater is 16) keeps its code, which the pouch draws as its
                // foil; everything else takes the pouch's one plain material, 15, with its flags kept
                const std::uint8_t v = codes[i / 4];
                px[i] = px[i + 1] = px[i + 2] = (v & 0x10) ? v : static_cast<std::uint8_t>((v & 0xF0) | 0x0F);
            }

            tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
        };
    }

    TexEditor::Filter lynaeMaskRedFill() {
        return [](TextureFile& tex) {
            const bool srgb = tex.getGamma().has_value();
            tex.setGamma(std::nullopt);          // a mask is codes, not colour
            std::vector<std::uint8_t> px = tex.getPixels();
            for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                px[i] = 255;
                if (srgb) {
                    px[i + 1] = srgbValues()[px[i + 1]];
                    px[i + 2] = srgbValues()[px[i + 2]];
                }
            }

            tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
        };
    }
}
