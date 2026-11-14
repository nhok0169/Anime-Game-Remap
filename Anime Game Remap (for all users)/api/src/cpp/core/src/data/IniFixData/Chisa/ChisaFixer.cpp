#include "AGRemapCore/data/IniFixData/Chisa/ChisaFixer.h"
#include "AGRemapCore/data/IniFixData/Chisa/ChisaTextures.h"

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
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/buffers/DrawUVCoverage.h"
#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/ChannelMapFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/RegionFilter.h"
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

        // ---- the three filters ----------------------------------------------------------------

        // Chisa's bb73967a is a matcap whose four channels are four GRAYSCALE sheen profiles, blended
        // by the normal map's alpha into ONE scalar; it only looks pink and purple viewed as RGB. The
        // skin's is a HOLOGRAPHIC FOIL -- rainbow RGB with a matcap ring in alpha. Bound raw, the
        // packed channels are read as foil COLOUR and a white shirt comes out pink-lavender pearly.
        // Translated: every channel is her FIRST profile, the one a low normal-map alpha selects, so
        // the intensity survives and there is no rainbow.
        TexEditor::Filter sheenFilter() {
            ChannelMapFilter filter;
            filter.data = true;                      // these bytes are data, not colour
            for (std::size_t c = 1; c < 4; ++c) {
                filter.channels[c] = {0, {}};        // the profile, R, into G, B and A
            }
            return TexEditor::filterOf(std::move(filter));
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
            ChannelMapFilter filter;
            filter.data = true;                      // these bytes are data, not colour
            filter.channels[0] = {0, ChannelMapFilter::constant(0)};     // R: the target's is 0 over 99.4%
            filter.channels[2] = {2, ChannelMapFilter::constant(0)};     // B: ...and its B over 93.7%
            filter.channels[3] = {3, ChannelMapFilter::constant(255)};   // A: ...and its A is 255 over 98.3%
            return TexEditor::filterOf(std::move(filter));
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

        // Each vertex's height, off a WWMI ``Position.buf`` (three 32-bit floats a vertex)
        std::vector<double> heightsOf(const std::string& positionFile) {
            std::vector<std::unique_ptr<BufElementType>> elements;
            elements.push_back(BufElementType::repeated("POSITION", 3, [] { return std::make_unique<BufFloat>(); }));
            return BufFile(positionFile, std::move(elements), "Position.buf").column("POSITION", 2);
        }

        // The UV island component 'comp' paints above 'splitZ', from the MOD's own mesh.
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
        // halves claim is dropped, since the sash must not be tinted, and the island grows into the
        // gutter only.
        RegionFilter::Region islandRegion(const WWMIFixerConfig::TexEditContext& ctx, int comp, float splitZ) {
            return [ctx, comp, splitZ](long long width, long long height) -> std::vector<bool> {
                const auto ranges = ctx.drawRanges.find(comp);
                if (ranges == ctx.drawRanges.end() || ranges->second.empty() || ctx.positionFile.empty()
                    || ctx.texcoordFile.empty() || ctx.indexFile.empty()) {
                    return {};
                }

                try {
                    const std::vector<double> heights = heightsOf(ctx.positionFile);
                    const DrawUVCoverage coverage = DrawUVCoverage::ofMesh(ctx.indexFile, ctx.texcoordFile, heights.size());
                    const std::vector<DrawUVCoverage::Triangle> triangles = coverage.triangles(ranges->second);
                    std::vector<bool> accepted;
                    accepted.reserve(triangles.size());
                    for (const DrawUVCoverage::Triangle& triangle : triangles) {
                        const float centroid = (static_cast<float>(heights[static_cast<std::size_t>(triangle[0])])
                                                + static_cast<float>(heights[static_cast<std::size_t>(triangle[1])])
                                                + static_cast<float>(heights[static_cast<std::size_t>(triangle[2])])) / 3.0f;
                        accepted.push_back(centroid >= splitZ);
                    }
                    return coverage.region(triangles, accepted, width, height, 2);
                } catch (const std::exception&) {
                    return {};                   // no island to grade is better than grading the sash
                }
            };
        }

        TexEditor::Filter gradeFilter(const WWMIFixerConfig::TexEditContext& ctx) {
            // the gamma is NOT cleared here: this is colour, and the sRGB pre-correction is what
            // makes the untagged output sample the way the sRGB source did
            auto grade = std::make_shared<ChannelMapFilter>();
            for (int c = 0; c < 3; ++c) {
                std::vector<std::uint8_t> table(256);
                for (int v = 0; v < 256; ++v) {
                    table[static_cast<std::size_t>(v)] =
                        static_cast<std::uint8_t>(std::min(255.0, std::max(0.0, v * GradeGain[c])));
                }
                grade->channels[static_cast<std::size_t>(c)] = {c, std::move(table)};
            }
            return TexEditor::filterOf(RegionFilter(std::move(grade), islandRegion(ctx, GradeComponent, GradeSplitZ)));
        }
    }


    WWMIFixerConfig ChisaFixer::parfait3_7Config() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::ChisaParfait;
        // The target's version is her newest model (WWMIFixerConfig::version), 3.7 and not her release
        // 3.5: that update moved her vb0 (e611d493 -> 95ecef77) and nothing else of hers, so a fix built
        // at 3.5 writes a hash the game no longer binds. Every other lookup resolves from the 3.5
        // bucket by the inclusive floor -- see HashData's note.
        config.sourceVersion = "2.8";          // hers, and not her skin's -- see the field's note


        // ---- the passes the TARGET draws each slot on, off her frame dumps ----
        // slotPasses: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

        // ---- every pass gated through its VERTEX shaders, each with its NAMED filter_index ----
        // passVertexShaders and filterIndices: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData),
        // its tags the values both directions of the pair share (a shader holds ONE filter_index across every loaded
        // .ini; `Tools/Misc/Diagnostics/wwmiShaderTags.py` is the check). The literal entries this config carried for
        // passes with no row went on 2026-11-12: on all 41 Chisa / ChisaParfait / Changli mods they changed no file,
        // at 3.7 or 3.8.

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
            // ps-t4 as well as ps-t1: the eye draw sets six registers and two of them are the
            // character's own -- the 512 greyscale structure map at ps-t1 and the 2048 COLOURED iris
            // at ps-t4. Naming only ps-t1 left the skin's own eye rendering on every remap, and a
            // mod whose eye TOGGLE swaps the coloured one (Chisa6's heart eyes) had nothing to swap.
            {6, {6, {{"ps-t1", "irisDiffuse"}, {"ps-t4", "eyeDiffuse"}}}},
        };

        // ---- a slot's OTHER passes bind the same art at different registers ----
        config.extraPassRegs = {
            {0, {
                {"other2", {{"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
                {"other", {{"ps-t0", "hairDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {1, {
                {"other2", {{"ps-t0", "hairDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
                {"other", {{"ps-t0", "hairDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {2, {
                {"other", {{"ps-t0", "upperDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {3, {
                {"other", {{"ps-t0", "upperDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {4, {
                {"other", {{"ps-t0", "lowerDiffuse"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}},
            }},
            {5, {
                {"other", {{"ps-t0", "Flat050000FF"}, {"ps-t1", "FlatDE5A7E00"}, {"ps-t2", "accessoryNormal"}, {"ps-t3", "accessoryDiffuse"}}},
                {"other2", {{"ps-t0", "Flat050000FF"}, {"ps-t1", "FlatDE5A7E00"}, {"ps-t2", "accessoryNormal"}, {"ps-t3", "accessoryDiffuse"}}},
            }},
        };

        // ---- her textures by the hash the game binds them under ----

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
        config.sourceTextures = chisaTextureFacts();

        // ---- the flats the fix invents, named as the prototype names them ----
        config.createdTextures = {
            {"DetailZero00000000", Colour(0, 0, 0, 0)},
            {"DetailZero000000FF", Colour(0, 0, 0, 255)},
            {"Flat050000FF", Colour(5, 0, 0, 255)},
            {"FlatDE5A7E00", Colour(222, 90, 126, 0)},
        };

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
        // the masks a flat or aliased file is refused for are the source's: see WWMITextureFacts::regionRoles
        config.flatLeftToGame = {"hairMask", "frontHairMask"};


        // ---- the three lines that would undo the whole fix, and RabbitFX's SetTextures ----

        // ---- parts the skin has no counterpart for, pinned to one bone each ----
        //   `collar` is deliberately not here: the prototype leaves it opt-in, and props and
        //   skirt are the two confirmed in game.
        config.anchorChains = {
            {3, {409, 410, 411, 412, 413, 414, 415, 416, 417, 418}},   // props
            {193, {292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315}},   // skirt
        };

        // ---- a mod from before WWMI's merged skeleton holds per-component LOCAL ids ----
        // sourceVgMaps: read off VGRemaps (component -> "") at the run's versions (WWMIFixerConfig::withGameData)

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

        // a batched export's dispatch height is the TARGET's (her Metadata.json's dispatch_y), or WWMI loads
        // too few of the mod's offsets on her draws (Lynae9's spikes; Chisa14 kept Chisa's 1441)
        // shapeKeyDispatchSize: read off ShapeKeyDispatchSizes at the run's versions (WWMIFixerConfig::withGameData)

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

        return config;
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::chisaParfait3_7() {
        return makeWWMIFixer(ChisaFixer::parfait3_7Config());
    }


    IniFixBuilder::Factory ChisaFixer::parfait3_7() {
        return IniFixBuilderFuncs::chisaParfait3_7();
    }
}
