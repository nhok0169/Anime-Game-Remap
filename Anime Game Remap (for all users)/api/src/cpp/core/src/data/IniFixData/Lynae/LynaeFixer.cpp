#include "AGRemapCore/data/IniFixData/Lynae/LynaeFixer.h"

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

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeBangsShader.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeFilmShader.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeHairShader.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeLowerShader.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeSkeleton.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeSkinCodes.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeTextures.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeUpperShader.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {
    // Transcribed from Tools/Misc/Prototypes/lynaePeppermintFix.py, which stays the oracle this is
    // A/B'd against. Pairing (per-component overlap of the two download folders' Position.buf, and
    // shader family): 0-4 and 7 one to one (the face and eyes are the same mesh), her jacket (5)
    // onto the skin's jacket (6), her props (6) onto the skin's hip pouch (5) -- same register order,
    // and her props' glass / foil passes do not carry; the pouch's own foil MATERIAL is moved off them
    // (lynaePropsPlainCode).
    IniFixBuilder::Factory IniFixBuilderFuncs::lynaePeppermint3_7() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::LynaePeppermint;
        config.version = "3.7";

        // Hers, for a mod whose vb0 is not in sourceVersionByVb0. Both skins share cb4 f02baf77,
        // which at 3.6 only Lynae has
        config.sourceVersion = "3.6";

        // ---- her 3.7 update moved her vb0 AND renumbered her skeleton ----
        // The hash lookups follow the mod's vb0; the blend's vertex group row follows its GEOMETRY,
        // because a hash-update tool rewrote one mod's vb0 to 3.7's and kept its 3.6 bone ids
        config.sourceVersionByVb0 = {{"0c33d628", "3.6"}, {"7e400733", "3.7"}};
        config.skeletonNumberings = {{"3.6", lynaeRenumbered36To37()}, {"3.7", {}}};
        config.referenceBoneCentroids = lynaeBoneCentroids();

        // ---- the passes the TARGET draws each slot on (her Peppermint dump, wwmiDrawTable.py) ----
        config.slotPasses = {
            {"dfeea2d5f7210740", "8485dc12c5851fa9"},   // bangs (the second pass inherits the first's set)
            {"dfeea2d5f7210740"},                       // hair
            {"640fac991b3599a7"},                       // face
            {"ed4fe222718a4497"},                       // upper body
            {"0a52e215e81518f3"},                       // lower body
            {"12fcca8e31aad1f5", "4470be479e212ca1"},   // hip pouch (both passes set the whole set)
            {"efc2690a4acd3c11", "0b6e3ef7b7c30cd3"},   // jacket (the second inherits the first's)
            {"fa9e4d98ed0a570e"},                       // eyes
        };

        // ---- every pass gated through its VERTEX shaders, so RabbitFX's pixel-shader tags survive ----
        config.passVertexShaders = {
            {"dfeea2d5f7210740", {"c30da0cb86079064"}},
            {"8485dc12c5851fa9", {"c30da0cb86079064"}},
            // the face too: 1c42858c or 595d6591, frame to frame (log-only dumps in the overworld, 3 of 7)
            {"640fac991b3599a7", {"1c42858ceb438917", "595d6591285d716e"}},
            {"ed4fe222718a4497", {"1f324d354402418c"}},
            {"0a52e215e81518f3", {"8af0aa3dcb3903ee"}},
            {"12fcca8e31aad1f5", {"fd92e896d5503d96"}},
            {"4470be479e212ca1", {"fd92e896d5503d96"}},
            {"efc2690a4acd3c11", {"1f324d354402418c"}},
            {"0b6e3ef7b7c30cd3", {"1f324d354402418c"}},
            // the eyes are drawn with EITHER of two vertex shaders, frame to frame, on the same pixel
            // shaders (the menu's and the overworld's dumps disagree): with one listed, the other
            // frames kept the game's eye and the mod's eyes flickered to the skin's colour
            {"fa9e4d98ed0a570e", {"c32a69851757154a", "ceff9a81afb1de70"}},
            {"e04f4df80ee6b0ab", {"a54621ce48ed541b", "5fd6e5bb6ff81c53"}},
            {"bce1512f1c6b82fe", {"a57b6349c93b6107"}},
            {"f8c96a270bf847dd", {"6a6650a9db8983ce", "e4a3da6d1d1068b9"}},
            {"0fbe7ebba08cd1b0", {"641c11c9ee112caf", "d87c4657c08f2cdd"}},
        };

        // ---- ...and every tag NAMED. A shader holds ONE filter_index across every loaded .ini, so
        // the three outline vertex shaders Chisa -> ChisaParfait already tags keep its values; the
        // rest take 3381.610 up, a range no other pair uses (Chisa .710-.734 / .76-.768, Sanhua
        // .81-.84 / .91-.96). The reverse direction must reuse every value here it also tags ----
        config.filterIndices = {
            {"6a6650a9db8983ce", "3381.73"},
            {"e4a3da6d1d1068b9", "3381.731"},
            {"641c11c9ee112caf", "3381.715"},
            {"c30da0cb86079064", "3381.61"},
            {"1c42858ceb438917", "3381.611"},
            {"1f324d354402418c", "3381.612"},
            {"8af0aa3dcb3903ee", "3381.613"},
            {"fd92e896d5503d96", "3381.614"},
            {"c32a69851757154a", "3381.615"},
            {"a54621ce48ed541b", "3381.616"},
            // the eyes' second vertex shaders, at the values the reverse direction gives them
            {"ceff9a81afb1de70", "3381.625"},
            {"5fd6e5bb6ff81c53", "3381.718"},
            {"595d6591285d716e", "3381.628"},
            {"a57b6349c93b6107", "3381.617"},
            {"d87c4657c08f2cdd", "3381.618"},
        };

        // ---- source component -> target slot and the registers it binds there ----
        config.plan = {
            // ps-t2 (36c90686) and ps-t3 (4eaa9816) are the same texture on both skins: left to the game
            {0, {0, {{"ps-t0", "bangsMask"}, {"ps-t1", "bangsDiffuse"}, {"ps-t4", "hairShadeRamp"}, {"ps-t5", "bangsStrand"}}}},
            {1, {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"},
                     {"ps-t4", "hairShadeRamp"}, {"ps-t5", "hairStrand"}}}},
            // The face is the same mesh on both, and the skin's face shader reads its diffuse at ps-t2
            // where hers reads it at ps-t1. The two face MASKS are packed differently (hers R 246 /
            // G 28 / B 0, the skin's R 118 / B 101 / A 202), so the skin's own mask stays -- its UVs
            // are hers
            {2, {2, {{"ps-t2", "faceDiffuse"}}}},
            {3, {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDetail"}, {"ps-t3", "upperDiffuse"}}}},
            // Her lower body goes through the skin's UPPER-body slot, as a second section in the mod's
            // own file: the skin draws her lower body first and her upper body after it with a different
            // rasterizer state, so a garment in Lynae's lower body over skin in her upper body (Lynae10:
            // a full body in 3, a skin-tight suit in 4) lost to the skin it covers. One pass for both
            // lets depth decide; the two passes read the same four roles at ps-t0..t3, and t4 / t5 are
            // global ramps. Not a copy .ini: a copy has no shape-keyed positions (sameFileSection)
            {4, {3, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDetail"}, {"ps-t3", "lowerDiffuse"}}, true}},
            // The skin's jacket shader reads mask / diffuse / detail at ps-t1 / t2 / t3 -- hers reads
            // the detail at t2 and the diffuse at t3
            {5, {6, {{"ps-t0", "jacketNormal"}, {"ps-t1", "jacketMask"}, {"ps-t2", "jacketDiffuse"}, {"ps-t3", "jacketDetail"}}}},
            // ...and her foil where the pouch reads its own (ps-t5): the film the pouch draws is hers
            {6, {5, {{"ps-t0", "propsNormal"}, {"ps-t1", "propsMask"}, {"ps-t2", "propsDetail"}, {"ps-t3", "propsDiffuse"}, {"ps-t5", "filmFoil"}}}},
            {7, {7, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"}}}},
            // her jacket's FILM draws (see drawReroutes), through the hip pouch with HER jacket shader:
            // her jacket's register order, her foil at ps-t4
            {105, {5, {{"ps-t0", "jacketNormal"}, {"ps-t1", "jacketMask"}, {"ps-t2", "jacketDetail"}, {"ps-t3", "jacketDiffuse"}, {"ps-t4", "filmFoil"}}, true}},
            // ...and her PROPS' film draws, the same way: her props film is a post-lighting translucent pass
            // that no pre-lighting draw of the skin can host, so it takes her jacket's film
            {106, {5, {{"ps-t0", "propsNormal"}, {"ps-t1", "propsMask"}, {"ps-t2", "propsDetail"}, {"ps-t3", "propsDiffuse"}, {"ps-t4", "filmFoil"}}, true}},
            // ...and her lower body's 0x80 draws (tights, shorts), drawn as plan 4 is but kept off the slot's
            // outline pass (see offPasses below)
            {104, {3, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDetail"}, {"ps-t3", "lowerDiffuse"}}, true}},
        };

        // ---- her jacket shader draws a code with flag 0x10 as a see-through film (Lynae9's sweater,
        // codes 211-226), and the skin's jacket slot has no film -- even its own film code 49 with its
        // film mask stays matte, and the hip pouch's foil is a different material (a violet-rimmed
        // holographic sheen where hers is a soft see-through film). Those draws go through the hip
        // pouch's slot -- a deferred pass with her jacket pass's inputs and outputs -- drawn by her own
        // jacket shader with her jacket's material constants, reading the mod's art as it is. Her
        // shader reads her lighting ramp at ps-t5 (see the ramp below) ----
        // Her PROPS mark film the same way (Lynae9's sweater on its other X state): her own props passes
        // draw it as post-lighting translucency (a depth prepass, a separate translucency target and a
        // fullscreen composite), which nothing the skin draws before lighting can host, so those draws
        // take her jacket shader too. Her jacket shader reads the code at t2.x, as the props' R8 map has it ----
        WWMIFixerConfig::Shader filmShader = lynaeFilmShader();
        filmShader.lines = {{"ps-t5", "role ramp"}};
        config.plan.at(105).shader = filmShader;
        config.plan.at(106).shader = filmShader;

        // ---- HER OWN SHADERS ON THE SKIN'S SLOTS (2026-10-14). Drawn with the skin's shaders and her
        // textures edited to approximate her, her parts came out as the skin's materials: a pink-violet
        // holographic lining where hers is a teal film, a yellow-green tie, a green sheen on her hair, a
        // black skirt gone metallic. Each part now draws with her own pixel shader and her own draw's
        // material constants (her 3.7 dump, through wwmiShaderData.py), in her register order, with her
        // draw's culling and nothing blended. The STENCIL is left as the skin's slot sets it: the skin's depth
        // prepass has already written bits there that the deferred lighting reads to draw a part sheer, and
        // writing hers over them (ref 2, mask 0xff) drew Lynae14's sheer skirt opaque. Where the
        // skin's slot holds one of her game textures at another register it is bound by `ref`. Her
        // LIGHTING RAMP is not one of them, though the skin's slot has a texture of the same name there:
        // the game refills that one texture with the ramp of the character on screen, so a `ref` read the
        // SKIN's ramp -- her shaders bind her own, downloaded (`role ramp`, at t5 / t6 / t9). Her lower
        // body's three the skin's upper-body slot lacks are captured off the skin's own lower-body draw,
        // which comes first in every scene (stashRegs). On a slot the skin draws twice (a solid pass and a
        // see-through one) hers is drawn on both, the state spelled out so the second repeats the first.
        // A mod that hands a part's maps to RabbitFX (Lynae8 / 10 / 11's SetTextures, Lynae3's glow) is
        // carried by her shaders themselves: each was generated with RabbitFX's patch written into it
        // (wwmiShaderData.py --rabbitfx, its sites read by wwmiRabbitFXSites.py), so on the skin's slot it
        // reads the mod's maps at ps-t60..t65 where RabbitFX's patch of HER game shader reads them, by
        // the same routing flags -- pinned to hers below (SourceComponent::rabbitFX). Routed by hand
        // instead (binding the maps by role name), her jacket's film went off and Lynae11's legs drew as
        // skin, because RabbitFX does not route by role: on her lower body it reads the mod's LIGHTMAP
        // at her normal-map sample, and leaves her diffuse alone unless the mod hands it a Materialmap ----
        const auto ownState = [](const std::string& cull, std::vector<std::pair<std::string, std::string>> lines) {
            for (const auto& line : std::vector<std::pair<std::string, std::string>>{
                     {"blend", "disable"},
                     {"rasterizer_state_merge", "true"},
                     {"cull", cull}}) {
                lines.push_back(line);
            }
            return lines;
        };
        const auto own = [&ownState](WWMIFixerConfig::Shader shader, const std::string& cull,
                                     std::vector<std::pair<std::string, std::string>> lines) {
            shader.lines = ownState(cull, std::move(lines));
            shader.onlyWith = "o3";
            return shader;
        };

        // her jacket on the skin's jacket slot: her register order (detail at t2, diffuse at t3), her foil
        WWMIFixerConfig::Shader jacket = own(lynaeFilmShader(), "back", {{"ps-t5", "role ramp"}});
        jacket.name = "LynaeJacket";
        jacket.registers = {{"ps-t2", "jacketDetail"}, {"ps-t3", "jacketDiffuse"}, {"ps-t4", "filmFoil"}};
        config.plan.at(5).shader = std::move(jacket);

        // her hair and bangs, two-sided as her own draws are
        config.plan.at(0).shader = own(lynaeBangsShader(), "none", {{"ps-t6", "role ramp"}});
        config.plan.at(1).shader = own(lynaeHairShader(), "none", {{"ps-t6", "role ramp"}});

        // her upper body: her matcap at t4 (a texture both skins share, at t8 on the skin's slot) and her ramp at t5
        config.plan.at(3).shader = own(lynaeUpperShader(), "back", {{"ps-t4", "ref ps-t8"}, {"ps-t5", "role ramp"}});

        // her lower body (on the skin's upper-body slot): three game textures off the skin's lower-body draw
        WWMIFixerConfig::Shader lower = own(lynaeLowerShader(), "back",
                                            {{"ps-t4", "stash 4 ps-t4"}, {"ps-t5", "stash 4 ps-t5"}, {"ps-t6", "stash 4 ps-t6"},
                                             {"ps-t9", "role ramp"}});
        lower.registers = {{"ps-t7", "filmFoil"}, {"ps-t8", "lowerSheer"}};
        config.plan.at(104).shader = lower;
        config.plan.at(4).shader = std::move(lower);
        config.stashRegs = {{4, {"ps-t4", "ps-t5", "ps-t6"}}};
        WWMIFixerConfig::DrawReroute propsFilmDraws{6, 106, "propsDetail", 0x10};
        propsFilmDraws.sourceFlags = lynaePropsFilmFlags();   // Lynae9 draws its props with the game's code map
        // (Lynae9's hat brim and shorts, in codes of bit 0x80 her own maps never use, came out lime here and were
        // sent through the skin's shader for a while. The lime was the slot's outline pass -- see offPasses below
        // -- and the skin's shader drew the hat matte where hers draws it satin, so they are hers again.)
        config.drawReroutes = {{5, 105, "jacketDetail", 0x10}, std::move(propsFilmDraws), {4, 104, "lowerDetail", 0x80}};

        // ---- RABBITFX ON EVERY PART HER OWN SHADERS DRAW. A mod may hand a part's maps to RabbitFX instead of
        // binding them (Lynae11's robotic legs: white plastic on Lynae, skin without RabbitFX), and RabbitFX
        // routes them by the kind of texture at ps-t0..t3 -- which on the skin's slot are other files. So the
        // mod's RabbitFX lines are kept and the routing pinned to hers (SourceComponent::rabbitFX), for her
        // shaders' RabbitFX sites to read. Her own game textures there (a vanilla dump's, at Ultra High: the
        // 2048 BC7 maps are typeless with 12 mips): a BC7 normal map or diffuse is 3 (1718.3), a BC3 mask
        // none, an R8 code map 4 (1718.4), a square B8G8R8A8 map under 512 3 (the bangs' 128 x 128 strand
        // map), anything else none. Read off RabbitFX's own `dump = desc` (a frame dump of Lynae11 on her card
        // logs the flags it set: x187 0 / y187 0 / w187 1 on her lower body, where the mod's own files sit) ----
        using RabbitFX = WWMIFixerConfig::SourceComponent::RabbitFX;
        config.plan.at(0).rabbitFX = RabbitFX{{{"ps-t0", 0}, {"ps-t1", 3}, {"ps-t2", 3}, {"ps-t3", 0}}};
        config.plan.at(1).rabbitFX = RabbitFX{{{"ps-t0", 0}, {"ps-t1", 3}, {"ps-t2", 0}, {"ps-t3", 0}}};
        config.plan.at(3).rabbitFX = RabbitFX{{{"ps-t0", 3}, {"ps-t1", 0}, {"ps-t2", 4}, {"ps-t3", 3}}};
        config.plan.at(4).rabbitFX = RabbitFX{{{"ps-t0", 3}, {"ps-t1", 0}, {"ps-t2", 4}, {"ps-t3", 3}}};
        config.plan.at(5).rabbitFX = RabbitFX{{{"ps-t0", 3}, {"ps-t1", 0}, {"ps-t2", 4}, {"ps-t3", 3}}};
        config.plan.at(104).rabbitFX = config.plan.at(4).rabbitFX;

        // ---- HER TIGHTS ARE KEPT OFF THE SLOT'S OUTLINE PASS (f8c96a27). The skin's outline shader draws
        // the shell of her tights through the shorts over them, lime (Lynae9, 2026-10-15: a frame dump put
        // the lime on that pass, and her own card shows no outline there). ONLY the draws her code map flags
        // 0x80 (Lynae9's tights and shorts), rerouted to plan 104 (2026-10-16): the rest of her lower body
        // keeps its outline, which some mods draw with -- Lynae2's torn top is dark only through the outline
        // shell of the fabric under the skin, and with the whole lower body off the pass it came out pale ----
        config.plan.at(104).offPasses = {"f8c96a270bf847dd"};

        // ---- a slot's OTHER passes: the outlines take the slot's diffuse at ps-t0; the eye's second
        // pass sets ps-t0 only ----
        config.extraPassRegs = {
            {0, {{"bce1512f1c6b82fe", {{"ps-t0", "bangsDiffuse"}}}}},
            {1, {{"f8c96a270bf847dd", {{"ps-t0", "hairDiffuse"}}}}},
            {3, {{"f8c96a270bf847dd", {{"ps-t0", "upperDiffuse", 3}, {"ps-t0", "lowerDiffuse", 4}}}}},
            {5, {{"0fbe7ebba08cd1b0", {{"ps-t0", "propsDiffuse", 6}, {"ps-t0", "jacketDiffuse", 105}, {"ps-t0", "propsDiffuse", 106}}}}},
            {6, {{"0fbe7ebba08cd1b0", {{"ps-t0", "jacketDiffuse"}}}}},
            {7, {{"e04f4df80ee6b0ab", {{"ps-t0", "eyeMask"}}}}},
        };

        config.sourceTextures = lynaeTextureFacts();

        // ---- the body's material-code maps (ps-t2) mark skin with 0 on Lynae and 4 on the skin, so
        // a map carried across unchanged shaded her body's skin as cloth -- whiter, with lavender
        // shadows, under a warm face: see lynaeSkinCodeSwap ----
        config.texEdits = {
            {"upperDetail", "SkinCode", [](const WWMIFixerConfig::TexEditContext&) { return lynaeSkinCodeSwap(); }},
            {"lowerDetail", "SkinCode", [](const WWMIFixerConfig::TexEditContext&) { return lynaeSkinCodeSwap(); }},
            // ...and her lower-body / jacket masks' red channel, which her shaders ignore and the skin's
            // reads as a dark material (Lynae10's suit, R = 0, came out navy): see lynaeMaskRedFill
            {"lowerMask", "Red", [](const WWMIFixerConfig::TexEditContext&) { return lynaeMaskRedFill(); }},
            {"jacketMask", "Red", [](const WWMIFixerConfig::TexEditContext&) { return lynaeMaskRedFill(); }},
        };

        // ---- her lower-body shader shades by the normal map's alpha, the skin's does not: the bands are
        // baked into the diffuse, see lynaeSheerBake ----
        WWMIFixerConfig::TexEdit sheer{"lowerDiffuse", "Sheer", [](const WWMIFixerConfig::TexEditContext& ctx) {
            return lynaeSheerBake(ctx);
        }};
        sheer.sourceGameRoles = {"lowerNormal"};
        config.texEdits.push_back(std::move(sheer));

        // ---- her coat lining is a film (code 4 of the jacket's code map) the skin draws as plain cyan
        // diffuse: the code moves to the skin's film code and the texels under it take the measured tint
        // (see lynaeFilmCode / lynaeFilmBake) ----
        config.texEdits.push_back({"jacketDetail", "Film", [](const WWMIFixerConfig::TexEditContext&) { return lynaeFilmCode(); }});
        WWMIFixerConfig::TexEdit film{"jacketDiffuse", "Film", [](const WWMIFixerConfig::TexEditContext& ctx) {
            return lynaeFilmBake(ctx);
        }};
        film.sourceGameRoles = {"jacketDetail"};
        config.texEdits.push_back(std::move(film));

        // ---- her props land on the skin's hip pouch, whose shader draws every material her props maps
        // use as its holographic foil (Lynae14's red dress: blue-violet rims): all but her film (flag
        // 0x10) become its plain material 15, see lynaePropsPlainCode. The mod's diffuse is left as it is ----
        config.texEdits.push_back({"propsDetail", "Plain", [](const WWMIFixerConfig::TexEditContext&) { return lynaePropsPlainCode(); }});

        // Masks mark regions, and her own are flat in places on purpose (her hair mask is one value
        // over the whole texture, her lower-body and jacket masks R = 255 throughout), so a flat mask
        // is replaced by HER game texture rather than left to the game: the skin's hair mask is
        // structured and laid out for the skin's UVs, and would land in patches on hers
        config.flatFallsBackToSource = {"upperMask", "lowerMask", "jacketMask", "propsMask", "faceMask", "hairMask", "bangsMask"};

        // ---- the three lines that undo the remap on a source past 256 bones, and RabbitFX's ----
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

        // ---- the shape keys are RETARGETED: the overrides keep firing and the asset remap moves
        // their hashes and checksum onto the skin's; a batched export's dispatch height is the skin's
        // (her Metadata.json's dispatch_y), or WWMI loads too few of the mod's offsets on her draws ----
        config.hiddenObjs = {};
        config.zeroShapeKeyStream = false;
        config.shapeKeyDispatchSize = "1854";

        // ---- the game drives shape keys by SLOT, and the two skins number the same face keys
        // differently (2 of 105 at the same index): moved across unchanged, a blink opened the mouth ----
        config.shapeKeyOrder = lynaeToPeppermintShapeKeys();
        config.cleanTexcoords = true;

        // A mod's own line that puts a texture into another kind of slot on purpose keeps that slot:
        // Lynae3's `$Wish` (U) binds each part's diffuse into the hair ramp / detail slot, `ps-t2`,
        // which the two skins read alike -- moved by the texture's role it landed on the diffuse
        // register and the toggle did nothing
        config.carryByRegisterRole = true;

        // The fix's own merged skeleton is sized for the TARGET: LynaePeppermint's slots reach bone 314
        // (315 x 3 float4 = 945), past the 768 default
        config.mergedSkeletonSlots = 1536;

        // ---- a mod from before WWMI's merged skeleton: every such mod of hers is a 3.6 export ----
        config.sourceVgMaps = lynae36VgMaps();

        config.sourceLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "lower body"},
            {5, "jacket"}, {6, "props (headphones, pin, ID card)"}, {7, "eyes"},
        };
        config.targetLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "lower body"},
            {5, "hip pouch"}, {6, "jacket"}, {7, "eyes"},
        };

        return makeWWMIFixer(std::move(config));
    }


    IniFixBuilder::Factory LynaeFixer::peppermint3_7() {
        return IniFixBuilderFuncs::lynaePeppermint3_7();
    }
}
