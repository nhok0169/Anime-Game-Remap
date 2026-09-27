#include "AGRemapCore/data/IniFixData/Chisa/ChisaFixer.h"

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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/Chisa/ChisaThumbprints.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {
        // Her mask's own legend, and the target's. Both skins mark bare skin with R = 255 -- an
        // earlier reading had them INVERSE, from a percentage-of-flesh-like-texels test over the two
        // atlases that her pale cream art defeats, and the repack it produced shaded her kimono with
        // subsurface scattering. Rendering the two masks' R channels beside their diffuses settles
        // it in one glance.
        constexpr std::uint8_t SourceMaskSkinAbove = 230;   // R at or above this is bare skin, always
        constexpr std::uint8_t SourceMaskFleshBand = 128;   // ...and at or above this where the diffuse is flesh
        constexpr std::uint8_t TargetMaskSkin[4] = {255, 0, 126, 0};
        constexpr std::uint8_t TargetMaskCloth[4] = {0, 0, 126, 0};

        // The accessory grade, and the island it applies to: component 5 above this height.
        constexpr double GradeGain[3] = {0.772, 0.616, 0.577};
        constexpr int GradeComponent = 5;
        constexpr float GradeSplitZ = 123.0f;

        // Warm, bright and ordered R > G >= B the way skin is -- a white or grey shirt is not
        bool fleshColoured(int r, int g, int b) {
            return r >= 150 && (r - g) >= 15 && g >= b;
        }

        std::vector<std::uint8_t> readFile(const std::string& path) {
            std::ifstream in(FileService::strToPath(path), std::ios::binary);
            if (!in) {
                return {};
            }

            return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(in)),
                                             std::istreambuf_iterator<char>());
        }

        // ---- the three filters ----------------------------------------------------------------

        // Chisa's bb73967a is a matcap whose four channels are four GRAYSCALE sheen profiles, blended
        // by the normal map's alpha into ONE scalar; it only looks pink and purple viewed as RGB. The
        // skin's is a HOLOGRAPHIC FOIL -- rainbow RGB with a matcap ring in alpha. Bound raw, the
        // packed channels are read as foil COLOUR and a white shirt comes out pink-lavender pearly.
        // Translated: every channel is her FIRST profile, the one a low normal-map alpha selects, so
        // the intensity survives and there is no rainbow.
        TexEditor::Filter sheenFilter() {
            return [](TextureFile& tex) {
                tex.setGamma(std::nullopt);          // these bytes are data, not colour
                std::vector<std::uint8_t> px = tex.getPixels();
                for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                    const std::uint8_t profile = px[i];
                    px[i + 1] = profile;
                    px[i + 2] = profile;
                    px[i + 3] = profile;
                }

                tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
            };
        }

        // The hair's ps-t5 is a 2048 x 2048 UV-MAPPED map, so it cannot be left to the game: the
        // geometry drawn through that slot is Chisa's, and the skin's own art there lands at UVs it
        // was never authored for -- irregular magenta blotches on the strands and a band of wrong
        // shadow (2026-09-26, reported twice and confirmed by nulling the register by hand).
        //
        // Only G carries signal. Measured on the target's own d547f3c6: R = 0 over 99.4% of its
        // texels, B = 0 over 93.7%, A = 255 over 98.3%, and G 170 distinct values -- the strand
        // term. So the mod's own G is kept, at the mod's own UVs, and the other three channels take
        // the target's packing. Binding the mod's file RAW renders the hair copper-orange, and so
        // does repacking only B and A: R has to go too, which is what that attempt missed.
        //
        // Nulling the register and binding a flat (0, 75, 0, 255) both also clear the blotches and
        // are indistinguishable from this in game (the maintainer, 2026-09-26) -- this one is
        // chosen because it is the only one of the three that keeps the mod's own strand detail.
        TexEditor::Filter hairNormalFilter() {
            return [](TextureFile& tex) {
                tex.setGamma(std::nullopt);          // these bytes are data, not colour
                std::vector<std::uint8_t> px = tex.getPixels();
                for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                    px[i] = 0;                       // R: the target's is 0 over 99.4%
                    px[i + 2] = 0;                   // B: ...and its B over 93.7%
                    px[i + 3] = 255;                 // A: ...and its A is 255 over 98.3%
                }

                tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
            };
        }

        // Repack a material mask from the SOURCE's layout into the TARGET's. Every non-skin texel
        // keeps the source's own R and G -- R is the shared band legend and G is how SHINY a surface
        // is, not a material id -- and takes only the target's packing in B and A. Skin is R >= 0.9,
        // or the matte band over a flesh-coloured diffuse: the same author paints a bare chest and a
        // shirt at the same R, so the band alone cannot say which is which and the diffuse under the
        // texel can.
        TexEditor::Filter maskFilter(const std::string& diffusePath) {
            return [diffusePath](TextureFile& tex) {
                tex.setGamma(std::nullopt);
                std::vector<std::uint8_t> px = tex.getPixels();
                const int w = tex.getWidth();
                const int h = tex.getHeight();

                std::vector<std::uint8_t> diffuse;
                int dw = 0;
                int dh = 0;
                if (!diffusePath.empty()) {
                    TextureFile d(diffusePath);
                    d.open();
                    if (d.hasImage()) {
                        diffuse = d.getPixels();
                        dw = d.getWidth();
                        dh = d.getHeight();
                    }
                }

                for (int y = 0; y < h; ++y) {
                    for (int x = 0; x < w; ++x) {
                        const std::size_t i = (static_cast<std::size_t>(y) * w + x) * 4;
                        if (i + 3 >= px.size()) {
                            continue;
                        }

                        const std::uint8_t r = px[i];
                        bool skin = r >= SourceMaskSkinAbove;
                        if (!skin && r >= SourceMaskFleshBand) {
                            // With no diffuse to ask, the band keeps the old answer (skin) rather
                            // than guessing cloth
                            skin = true;
                            if (!diffuse.empty() && dw > 0 && dh > 0) {
                                const std::size_t j =
                                    (static_cast<std::size_t>(y * dh / h) * dw + (x * dw / w)) * 4;
                                if (j + 2 < diffuse.size()) {
                                    skin = fleshColoured(diffuse[j], diffuse[j + 1], diffuse[j + 2]);
                                }
                            }
                        }

                        if (skin) {
                            px[i] = TargetMaskSkin[0];
                            px[i + 1] = TargetMaskSkin[1];
                            px[i + 2] = TargetMaskSkin[2];
                            px[i + 3] = TargetMaskSkin[3];
                        } else {
                            px[i + 1] = TargetMaskCloth[1];
                            px[i + 2] = TargetMaskCloth[2];
                            px[i + 3] = TargetMaskCloth[3];
                        }
                        // G TOO, and it used to be the one channel kept from the mod, on the
                        // grounds that it is how shiny the surface is and so the author's to
                        // choose (2026-09-26). The author never chose it: Chisa's own masks carry
                        // a CONSTANT green -- median 102 over the whole texture, range 97..105, on
                        // both body slots -- where the skin's is a real gloss map with a median of
                        // 0 and highlights to 243. Handing a flat 102 to the target's shader says
                        // "every texel of this body is moderately glossy", which is the metallic
                        // skin the maintainer reported on most Chisa mods, and it survives any
                        // change to the sheen because the sheen is what green MODULATES.
                        //
                        // A constant has nothing in it to preserve, so green takes the target's
                        // packing like every other channel -- which both TargetMask constants
                        // already spell as 0. A mod that really does paint gloss would need this
                        // rescaled rather than replaced, and none of the 24 here does.
                    }
                }

                tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
            };
        }

        // The UV island component 'comp' occupies above 'splitZ', from the MOD's own mesh.
        //
        // A shader family is a colour grade -- Chisa's ribbon is painted by a HAIR shader and the
        // skin has nowhere to draw it but a CLOTH one -- and the texture is the only place to put
        // that back. But component 5 is not only the ribbon: it runs from her hair down to her hip,
        // and the lower half is a broad WHITE sash that the same gain turns warm brown. The split is
        // given by the geometry: her heights are bimodal with an empty band between them.
        //
        // Rasterised from the MOD's texcoords, never the character's. Of nineteen test mods three
        // have her island overlapping THEIR sash, so a character-level island grades the wrong
        // geometry. A triangle is classified by its CENTROID, which cannot straddle; a texel both
        // halves claim is dropped, since the sash must not be tinted.
        std::vector<bool> islandMask(const WWMIFixerConfig::TexEditContext& ctx, int comp,
                                     float splitZ, int width, int height) {
            const auto ranges = ctx.drawRanges.find(comp);
            if (ranges == ctx.drawRanges.end() || ranges->second.empty()) {
                return {};
            }

            const std::vector<std::uint8_t> posRaw = readFile(ctx.positionFile);
            const std::vector<std::uint8_t> uvRaw = readFile(ctx.texcoordFile);
            const std::vector<std::uint8_t> idxRaw = readFile(ctx.indexFile);
            if (posRaw.empty() || uvRaw.empty() || idxRaw.empty()) {
                return {};
            }

            const std::size_t vertices = posRaw.size() / 12;
            if (vertices == 0 || uvRaw.size() % vertices != 0) {
                return {};
            }

            const std::size_t stride = uvRaw.size() / vertices;
            if (stride < 4) {
                return {};
            }

            auto zOf = [&posRaw](std::size_t v) {
                float z = 0.0f;
                std::memcpy(&z, posRaw.data() + v * 12 + 8, sizeof(float));
                return z;
            };

            // float16 at offset 0, the layout every WWMI texcoord buffer here uses. A float32
            // reading also puts these UVs inside [0, 1] and collapses the component to a sliver, so
            // a range check cannot tell the two apart -- the SPREAD can, and it is asserted below.
            auto uvOf = [&uvRaw, stride](std::size_t v, int axis) {
                std::uint16_t bits = 0;
                std::memcpy(&bits, uvRaw.data() + v * stride + static_cast<std::size_t>(axis) * 2, 2);
                const int sign = (bits >> 15) & 0x1;
                const int exponent = (bits >> 10) & 0x1F;
                const int mantissa = bits & 0x3FF;
                float value = 0.0f;
                if (exponent == 0) {
                    value = std::ldexp(static_cast<float>(mantissa), -24);
                } else if (exponent != 0x1F) {
                    value = std::ldexp(static_cast<float>(mantissa + 1024), exponent - 25);
                }

                return sign ? -value : value;
            };

            const std::uint32_t* index = reinterpret_cast<const std::uint32_t*>(idxRaw.data());
            const std::size_t indexCount = idxRaw.size() / 4;

            std::vector<std::uint32_t> tris;
            for (const auto& range : ranges->second) {
                for (long long k = 0; k < range.first; ++k) {
                    const std::size_t at = static_cast<std::size_t>(range.second + k);
                    if (at < indexCount && index[at] < vertices) {
                        tris.push_back(index[at]);
                    }
                }
            }

            if (tris.size() < 3) {
                return {};
            }

            float lo = 1.0f;
            float hi = 0.0f;
            for (std::uint32_t v : tris) {
                lo = std::min(lo, uvOf(v, 0));
                hi = std::max(hi, uvOf(v, 0));
            }

            if (hi - lo < 0.05f) {
                return {};                       // not the float16 layout; grade nothing
            }

            const std::size_t texels = static_cast<std::size_t>(width) * height;
            std::vector<bool> island(texels, false);
            std::vector<bool> other(texels, false);
            for (std::size_t t = 0; t + 2 < tris.size(); t += 3) {
                const std::uint32_t a = tris[t];
                const std::uint32_t b = tris[t + 1];
                const std::uint32_t c = tris[t + 2];
                const bool ribbon = ((zOf(a) + zOf(b) + zOf(c)) / 3.0f) >= splitZ;
                std::vector<bool>& into = ribbon ? island : other;

                const float xs[3] = {uvOf(a, 0) * width, uvOf(b, 0) * width, uvOf(c, 0) * width};
                const float ys[3] = {uvOf(a, 1) * height, uvOf(b, 1) * height, uvOf(c, 1) * height};
                const int x0 = std::max(0, static_cast<int>(std::floor(std::min({xs[0], xs[1], xs[2]}))));
                const int x1 = std::min(width - 1, static_cast<int>(std::ceil(std::max({xs[0], xs[1], xs[2]}))));
                const int y0 = std::max(0, static_cast<int>(std::floor(std::min({ys[0], ys[1], ys[2]}))));
                const int y1 = std::min(height - 1, static_cast<int>(std::ceil(std::max({ys[0], ys[1], ys[2]}))));
                const float det = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2]);
                if (std::fabs(det) < 1e-9f) {
                    continue;
                }

                for (int y = y0; y <= y1; ++y) {
                    for (int x = x0; x <= x1; ++x) {
                        const float px = x + 0.5f;
                        const float py = y + 0.5f;
                        const float w0 = ((ys[1] - ys[2]) * (px - xs[2]) + (xs[2] - xs[1]) * (py - ys[2])) / det;
                        const float w1 = ((ys[2] - ys[0]) * (px - xs[2]) + (xs[0] - xs[2]) * (py - ys[2])) / det;
                        if (w0 >= -1e-6f && w1 >= -1e-6f && (w0 + w1) <= 1.0f + 1e-6f) {
                            into[static_cast<std::size_t>(y) * width + x] = true;
                        }
                    }
                }
            }

            // A texel BOTH halves map to belongs exclusively to neither, and the sash must not be
            // tinted, so it is dropped rather than tolerated.
            std::size_t kept = 0;
            for (std::size_t i = 0; i < texels; ++i) {
                if (island[i] && other[i]) {
                    island[i] = false;
                }

                if (island[i]) {
                    ++kept;
                }
            }

            if (kept == 0) {
                return {};
            }

            // grow into the gutter ONLY -- the two islands are adjacent, and growing into the other
            // reintroduces exactly the bug this restricts
            for (int pass = 0; pass < 2; ++pass) {
                std::vector<bool> grown = island;
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        const std::size_t i = static_cast<std::size_t>(y) * width + x;
                        if (island[i] || other[i]) {
                            continue;
                        }

                        const bool near =
                            (x > 0 && island[i - 1]) || (x + 1 < width && island[i + 1])
                            || (y > 0 && island[i - width]) || (y + 1 < height && island[i + width]);
                        if (near) {
                            grown[i] = true;
                        }
                    }
                }

                island.swap(grown);
            }

            return island;
        }

        TexEditor::Filter gradeFilter(const WWMIFixerConfig::TexEditContext& ctx) {
            return [ctx](TextureFile& tex) {
                const int w = tex.getWidth();
                const int h = tex.getHeight();
                const std::vector<bool> island = islandMask(ctx, GradeComponent, GradeSplitZ, w, h);
                if (island.empty()) {
                    return;                      // no island to grade is better than grading the sash
                }

                // the gamma is NOT cleared here: this is colour, and the sRGB pre-correction is what
                // makes the untagged output sample the way the sRGB source did
                std::vector<std::uint8_t> px = tex.getPixels();
                for (std::size_t i = 0; i < island.size(); ++i) {
                    if (!island[i]) {
                        continue;
                    }

                    const std::size_t at = i * 4;
                    if (at + 2 >= px.size()) {
                        continue;
                    }

                    for (int c = 0; c < 3; ++c) {
                        const double graded = px[at + c] * GradeGain[c];
                        px[at + c] = static_cast<std::uint8_t>(std::min(255.0, std::max(0.0, graded)));
                    }
                }

                tex.setPixels(std::move(px), w, h);
            };
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::chisaParfait3_5() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::ChisaParfait;
        config.version = "3.5";
        config.sourceVersion = "2.8";          // hers, and not her skin's -- see the field's note


        // ---- the passes the TARGET draws each slot on, off her frame dumps ----
        config.slotPasses = {
            {"c0ad88a930c4d853", "71f60c461ae3f166"},    // front hair
            {"71f60c461ae3f166"},    // hair
            {"2060326dcea397fb"},    // face
            {"3311e8a58d8c5d20"},    // upper body
            {"a99f09b6f36e94af"},    // lower body
            {"3df800c350681ec9"},    // the skin's own (nothing maps onto it)
            {"da00ec8f7c73d5e3"},    // eyes
        };

        // ---- every pass gated through its VERTEX shaders: see passVertexShaders ----
        config.passVertexShaders = {
            {"c0ad88a930c4d853", {"d83a54772fc666f9"}},
            {"71f60c461ae3f166", {"d83a54772fc666f9"}},
            {"2060326dcea397fb", {"683e019f389b2624", "c277738ca4039045"}},
            {"3311e8a58d8c5d20", {"d24888b5b268a084"}},
            {"a99f09b6f36e94af", {"22195a190e37d3cf"}},
            {"3df800c350681ec9", {"bbabe18b97a63509"}},
            {"87825a9a29529f9b", {"6594231b96dfca5f"}},
            {"ced9a47fb6ad4d16", {"6594231b96dfca5f"}},
            {"da00ec8f7c73d5e3", {"72f45530b1e1f75a", "a6e9eb6303b1b631"}},
            {"259b766b59f72419", {"fd12d3374ac7a7dd", "1479e3f5a626af60"}},
            {"21176cf68a65ab7a", {"0ccd030bff8b515c", "5d60ebdc89fe3833"}},
            {"32414b557630d98d", {"ba4eee7b53cf726e", "60b893ec7f585976", "f906b8aa4c220a6f", "3edca9a0f68c8b15", "59585b690c6e1f01", "4cf784b1b2c7ca1c"}},
            {"ca134b7ad59cdf8c", {"0b22e4a80375c4d0", "a5cd08444f0fca2e"}},
            {"a7bdec26cf254853", {"ee6166816ce9f788"}},
            {"21a483170781cfeb", {"da98d2d08d937357"}},
            {"94d9d5e981938d52", {"5102d7edd774359e"}},
            {"320a753b019eff67", {"ac592389c85c3e38", "aef4fc536fbff1e7"}},
            {"92ca4bd985fe6887", {"676fdbd61b302294", "89577c176b52b351"}},
        };
        config.filterBase = 3381.71;
        config.filterStep = 0.001;

        // ---- source component -> target slot and the registers it binds ----
        config.plan = {
            {0, {0, {{"ps-t0", "frontHairMask"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}}},
            // ps-t5 must NOT be left to the game -- see hairNormalFilter for why, and for why
            // the mod's own map is repacked rather than bound raw
            {1, {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"},
                     {"ps-t5", "hairNormal"}}}},
            {2, {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}}}},
            {3, {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t3", "upperDiffuse"}, {"ps-t8", "bodySheen"}, {"ps-t2", "DetailZero000000FF"}, {"ps-t4", "DetailZero00000000"}, {"ps-t10", "DetailZero00000000"}}}},
            {4, {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t3", "lowerDiffuse"}, {"ps-t5", "bodySheen"}, {"ps-t2", "DetailZero000000FF"}}}},
            {5, {5, {{"ps-t0", "accessoryDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t3", "accessorySheen"}, {"ps-t5", "frontHairNormal"}}}},
            {6, {6, {{"ps-t1", "irisDiffuse"}}}},
        };

        // ---- a slot's OTHER passes bind the same art at different registers ----
        config.extraPassRegs = {
            {0, {
                {"21176cf68a65ab7a", {{"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
                {"32414b557630d98d", {{"ps-t0", "hairDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {1, {
                {"21176cf68a65ab7a", {{"ps-t0", "hairDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
                {"32414b557630d98d", {{"ps-t0", "hairDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {2, {
                {"259b766b59f72419", {{"ps-t0", "upperDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {3, {
                {"21176cf68a65ab7a", {{"ps-t0", "upperDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {4, {
                {"21176cf68a65ab7a", {{"ps-t0", "lowerDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {5, {
                {"87825a9a29529f9b", {{"ps-t0", "Flat050000FF"}, {"ps-t1", "FlatDE5A7E00"}, {"ps-t2", "accessoryNormal"}, {"ps-t3", "accessoryDiffuse"}}},
                {"ced9a47fb6ad4d16", {{"ps-t0", "Flat050000FF"}, {"ps-t1", "FlatDE5A7E00"}, {"ps-t2", "accessoryNormal"}, {"ps-t3", "accessoryDiffuse"}}},
            }},
        };

        // ---- the other mesh she draws: her hair ribbon ----
        config.sharedMeshes = {
            {"b00403dc", {
                {"ca134b7ad59cdf8c", {{"ps-t0", "hairDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
                {"a7bdec26cf254853", {{"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
                {"21a483170781cfeb", {{"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
        };

        // ---- her textures by the hash the game binds them under ----
        config.roles = {
            {"019c268e", "accessoryDiffuse"},
            {"06790f7e", "skinRamp"},
            {"165f3a1b", "upperDiffuse"},
            {"226b31fc", "irisDiffuse"},
            {"232c2dbc", "hairRamp"},
            {"2b16c5ac", "hairTipRamp"},
            {"2b6f8bcb", "lowerNormal"},
            {"3f0e6f21", "lowerMask"},
            {"40528957", "accessoryNormal"},
            {"4eaa9816", "accessorySheen"},
            {"526b9ed0", "upperNormal"},
            {"6ae8dd10", "faceMask"},
            {"90196068", "upperMask"},
            {"9ccd7ea7", "frontHairNormal"},
            {"a842d51f", "hairMask"},
            {"bb73967a", "bodySheen"},
            {"cbab5910", "hairDiffuse"},
            {"d030af95", "faceDiffuse"},
            {"d3b9ba76", "frontHairMask"},
            {"e921181d", "hairNormal"},
            {"f2646d21", "frontHairDiffuse"},
            {"f642139e", "lowerDiffuse"},
        };

        // ---- and the register her OWN sections bind each role at ----
        // The SOURCE's own binding per component, which is how a role finds the mod's file when
        // no hash names it. Two kinds of key: the game's `ps-t` registers, off Chisa's own max-LOD
        // draws (wwmiPassLayout.py), and RabbitFX's three resource lines -- a RabbitFX mod sets
        // those and binds no `ps-t` at all, so without them five of one mod's roles fell through to
        // a download of the GAME's texture and its painted outfit rendered vanilla.
        //
        // RabbitFX's "lightmap" is this fix's mask. The normal map IS taken, following the
        // prototype's table (2026-09-21) rather than an older comment beside it that says it is
        // deliberately not -- if orange hair comes back, component 1's normalmap row goes first.
        config.sourceRegisterRoles = {
            {0, {{"ps-t0", "frontHairMask"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}, {"Resource\\RabbitFX\\Diffuse", "frontHairDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "frontHairNormal"}, {"Resource\\RabbitFX\\Lightmap", "frontHairMask"}}},
            {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"}, {"ps-t5", "hairNormal"}, {"Resource\\RabbitFX\\Diffuse", "hairDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "hairNormal"}, {"Resource\\RabbitFX\\Lightmap", "hairMask"}}},
            {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "faceDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "faceMask"}}},
            {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "upperDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "upperNormal"}, {"Resource\\RabbitFX\\Lightmap", "upperMask"}}},
            {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "lowerDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "lowerNormal"}, {"Resource\\RabbitFX\\Lightmap", "lowerMask"}}},
            {5, {{"ps-t0", "accessoryDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "accessoryDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "accessoryNormal"}}},
            {6, {{"ps-t1", "irisDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "irisDiffuse"}}},
        };

        // ---- the flats the fix invents, named as the prototype names them ----
        config.createdTextures = {
            {"DetailZero00000000", Colour(0, 0, 0, 0)},
            {"DetailZero000000FF", Colour(0, 0, 0, 255)},
            {"Flat050000FF", Colour(5, 0, 0, 255)},
            {"FlatDE5A7E00", Colour(222, 90, 126, 0)},
        };

        // ---- a role the mod ships no file for falls back to HER game texture ----
        config.downloadCharFolder = "Chisa";
        config.downloadVersionFolder = "2_8";
        config.downloadPrefix = "Chisa";
        // Her masks mark regions, so a mod shipping a constant one has given us nothing to place.
        // The body's take the source's own, whose regions land right because the mod's UVs ARE the
        // source's; the HAIR's are left to the game instead (the maintainer's call, 2026-09-26, and
        // what the hair-mask finding already said). Her diffuses and normal maps are in neither
        // list: a flat diffuse is a plausible art choice and a normal map is nearly flat anyway.
        //
        // These two lists only reach a file the MOD shipped. A mod that ships no mask at all takes
        // the fallbackTextures route below instead, and no flatness test stands between it and the
        // register. That is deliberate: see the `hairMask` row below for why a flat fallback is
        // still the right thing to bind here.
        config.flatFallsBackToSource = {"upperMask", "lowerMask", "faceMask"};
        config.flatLeftToGame = {"hairMask", "frontHairMask"};

        config.fallbackTextures = {
            {"accessoryDiffuse", "019c268e"},
            {"accessoryNormal", "40528957"},
            {"accessorySheen", "4eaa9816"},
            {"bodySheen", "bb73967a"},
            {"faceDiffuse", "d030af95"},
            {"faceMask", "6ae8dd10"},
            {"frontHairDiffuse", "f2646d21"},
            {"frontHairMask", "d3b9ba76"},
            {"frontHairNormal", "9ccd7ea7"},
            {"hairDiffuse", "cbab5910"},
            // `hairMask` IS downloaded, and the flat test above must not reach it (2026-09-26).
            // a842d51f is one RGBA value over all 1024x1024 texels -- (255, 0, 126, 0) -- which
            // reads as "carries no information, so binding it is pointless". It is not: that value
            // is ChisaParfait's OWN dominant hair code, 49.2% of her own hair mask's texels. Bound,
            // every texel of the mod's hair is ordinary hair material.
            //
            // Dropped, the register falls to the game, and the game binds the TARGET's structured
            // mask (129 distinct R values, 16.3% of them R = 0) sampled at CHISA's UVs -- so the
            // codes land in patches laid out for a different head. Patchy codes shade in patches.
            //
            // The "R = 255 means bare skin" reading that argued for dropping it is a fact about the
            // BODY mask and does not carry to the hair pass: 60.4% of the target's own hair mask is
            // R = 255. Ask the target's own texture at a register what its values mean.
            {"hairMask", "a842d51f"},
            {"hairNormal", "e921181d"},
            {"hairRamp", "232c2dbc"},
            {"hairTipRamp", "2b16c5ac"},
            {"irisDiffuse", "226b31fc"},
            {"lowerDiffuse", "f642139e"},
            {"lowerMask", "3f0e6f21"},
            {"lowerNormal", "2b6f8bcb"},
            {"skinRamp", "06790f7e"},
            {"upperDiffuse", "165f3a1b"},
            {"upperMask", "90196068"},
            {"upperNormal", "526b9ed0"},
        };
        config.textureThumbprints = chisaTextureThumbprints();

        // ---- the three lines that would undo the whole fix, and RabbitFX's SetTextures ----
        config.removedRegs = {
            {"ResourceBlendBufferOverride", "ref"},
            {"ResourceMergedSkeletonOverride", "ref"},
            {"ResourceExtraMergedSkeletonOverride", "ref"},
            {"Resource\\RabbitFX\\Diffuse", ""},
            {"Resource\\RabbitFX\\Lightmap", ""},
            {"Resource\\RabbitFX\\Normalmap", ""},
            {"Resource\\RabbitFX\\Materialmap", ""},
            {"Resource\\RabbitFX\\Cutoutmap", ""},
            {"Resource\\RabbitFX\\Specialmap", ""},
            {"run", "commandlist\\rabbitfx\\settextures"},
        };

        // ---- parts the skin has no counterpart for, pinned to one bone each ----
        //   `collar` is deliberately not here: the prototype leaves it opt-in, and props and
        //   skirt are the two confirmed in game.
        config.anchorChains = {
            {3, {409, 410, 411, 412, 413, 414, 415, 416, 417, 418}},   // props
            {193, {292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315}},   // skirt
        };

        // ---- a mod from before WWMI's merged skeleton holds per-component LOCAL ids ----
        config.sourceVgMaps = {
            {0, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26}},
            {1, {27, 28, 29, 30, 31, 32, 33, 3, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140}},
            {2, {3}},
            {3, {142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 45, 3, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269}},
            {4, {152, 154, 155, 167, 187, 185, 179, 163, 193, 175, 177, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 245, 242, 343, 344, 253, 254, 255, 196, 172, 251, 256, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 249, 246, 368, 199, 166, 261, 372, 263, 264, 265, 266, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390}},
            {5, {146, 148, 145, 149, 147, 144, 143, 153, 152, 154, 156, 402, 142, 159, 160, 158, 157, 3, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418}},
            {6, {3}},
        };

        // ---- what the per-slot table and the hide sections say ----
        config.sourceLabels = {
            {0, "front hair"},
            {1, "hair"},
            {2, "face"},
            {3, "upper body"},
            {4, "lower body"},
            {5, "left-side accessory"},
            {6, "eyes"},
        };
        config.targetLabels = {
            {0, "front hair"},
            {1, "hair"},
            {2, "face"},
            {3, "upper body"},
            {4, "lower body"},
            {5, "the skin's own (nothing maps onto it)"},
            {6, "eyes"},
            {7, "the skin's own, right hip (nothing maps onto it)"},
        };
        // ---- the shape keys are RETARGETED, not hidden ------------------------------------------
        // The template's defaults hide the two shape-key overrides and bind every remapped draw a
        // zero vb6 stream. That is the maintainer's hand remap's behaviour, and it BREAKS a mod
        // that really uses its keys -- on the source as well as the target, because hiding
        // comments sections out of the mod's OWN text. The prototype moved to retargeting: the
        // overrides keep firing and the shared asset remap rewrites their hashes and the
        // shape-key checksum onto the target's, so WWMI's own pipeline fills vb6 and no zero
        // stream is wanted.
        config.hiddenObjs = {};
        config.zeroShapeKeyStream = false;

        // ---- a remap-only texcoord copy ---------------------------------------------------------
        // Her fox mask, hairpins and bells are drawn, placed and textured correctly and INVISIBLE,
        // because their second UV is NaN and the skin's upper-body shader reads it where hers does
        // not. And a mod may UV a part into the next tile relying on the sampler wrapping, which
        // the two skins do not agree about. See WWMIFixerConfig::cleanTexcoords.
        config.cleanTexcoords = true;

        // ---- the four texture edits -------------------------------------------------------------
        config.texEdits = {
            {"hairNormal", "Repack", [](const WWMIFixerConfig::TexEditContext&) {
                 return hairNormalFilter();
             }},
            {"upperMask", "Repack", [](const WWMIFixerConfig::TexEditContext& ctx) {
                 const auto diffuse = ctx.fileOfRole.find("upperDiffuse");
                 return maskFilter(diffuse == ctx.fileOfRole.end() ? "" : diffuse->second);
             }},
            {"lowerMask", "Repack", [](const WWMIFixerConfig::TexEditContext& ctx) {
                 const auto diffuse = ctx.fileOfRole.find("lowerDiffuse");
                 return maskFilter(diffuse == ctx.fileOfRole.end() ? "" : diffuse->second);
             }},
            {"bodySheen", "Foil", [](const WWMIFixerConfig::TexEditContext&) { return sheenFilter(); }},
            {"accessoryDiffuse", "Grade", [](const WWMIFixerConfig::TexEditContext& ctx) {
                 return gradeFilter(ctx);
             }},
        };

        return makeWWMIFixer(std::move(config));
    }


    IniFixBuilder::Factory ChisaFixer::parfait3_5() {
        return IniFixBuilderFuncs::chisaParfait3_5();
    }
}
