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

#include "AGRemapCore/data/IniFixData/Aether/AetherFixer.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"


namespace AGRemapCore {
    namespace {
        // The skin's main-mesh slots -- match_first_index off the frame dump of its Dressing Room preview
        const std::string HeadSlot = "0";
        const std::string BodySlot = "31407";

        // ---- his HEAD on the skin's head shader (2026-10-24) ----
        //
        // Lumine's situation exactly: the skin's head slot is a normal-map shader that reads the diffuse ALPHA, and his
        // head diffuse is alpha 255 all over where the skin's is ~0 (Lumine's hair glowed orange until it was set to 1).
        // His light map band is left alone, as hers is: his head atlas is all band 254-255 where the skin keeps its hair
        // on band 0, and on his identity mod the hair on the skin matched his own card.
        const int HeadDiffuseAlpha = 1;


        // ---- a GLOW painted in a COOL colour ----
        //
        // LumineFixer.cpp's edit, the same constants: the skin's body shader multiplies every glow (diffuse alpha 255) by
        // a warm gold of its own, so a cool glow loses its alpha and keeps its brightness baked into its colour. His own
        // glow pixels are near-white (36 of 18484 are cleared), which the gold tint barely moves.
        const int CoolGlowBrightness = 60;
        const int CoolGlowDarkFloor = 10;           // a darker texel is black, whatever its hue
        const int CoolGlowDarkMargin = 12;          // a dark texel this much bluer than red AND green is a cool glow
        const double CoolGlowGain = 2.2;            // in double, as the prototype computes it
        const double CoolGlowGainFull = 64.0;       // texels this dark (max RGB) take the whole gain...
        const double CoolGlowGainTop = 192.0;       // ...and this bright, none of it

        void clearCoolGlow(TextureFile& texFile) {
            std::vector<std::uint8_t> pixels = texFile.getPixels();
            for (std::size_t i = 0; i + 3 < pixels.size(); i += 4) {
                const int red = pixels[i], green = pixels[i + 1], blue = pixels[i + 2];
                const int peak = std::max({red, green, blue});
                const bool bright = peak > CoolGlowBrightness && blue > red;
                const bool darkBlue = peak > CoolGlowDarkFloor && blue >= red + CoolGlowDarkMargin && blue >= green + CoolGlowDarkMargin;
                if (pixels[i + 3] <= 200 || !(bright || darkBlue)) {
                    continue;
                }

                pixels[i + 3] = 0;
                const double taper = std::clamp((CoolGlowGainTop - peak) / (CoolGlowGainTop - CoolGlowGainFull), 0.0, 1.0);
                const double gain = 1.0 + (CoolGlowGain - 1.0) * taper;
                for (std::size_t c = 0; c < 3; ++c) {
                    pixels[i + c] = static_cast<std::uint8_t>(std::min(255.0, pixels[i + c] * gain + 0.5));
                }
            }
            const int width = texFile.getWidth();
            const int height = texFile.getHeight();
            texFile.setPixels(std::move(pixels), width, height);
        }


        // ---- a light map band BETWEEN his pale cloth and the skin's gold (2026-10-24) ----
        //
        // His shader shades a band of 150-175 like his pale cloth (126-128); the skin's shades it from a WARM ramp row.
        // Aether8 paints its white trousers on 163, and on the skin their shaded side came out peach where his own card
        // shades them lavender; with 160-170 moved to 126 -- that alone -- they matched his card. Neither bands 0 / 78 /
        // 128 / 255, the light map's R channel, nor the invented normal map was it, each tested alone on that mod. His
        // vanilla atlases hold 0.01% of their texels there, so the move barely touches his own model.
        //
        // ---- and a COOL texel on his pale cloth, between or hair band onto the skin's band 0 (2026-10-25) ----
        //
        // His shader keeps a surface's own hue in shade; the skin's multiplies it by a tinted ramp row -- warm on 126
        // and 255, cool on 0. So a warm surface (his cream cape, blonde hair) shades right where it is, and a cool one
        // went peach: Aether10's blue-grey mane on 255, a lilac cape on 126 (SynthRecolour). On 0 both matched his
        // card, and moving the warm ones too turned Aether7's cream cape lavender and flattened his blonde hair. Cool is
        // bluer than red by a margin, so a neutral grey or white stays put. The first band a texel matches wins, so the
        // gated bands lead.
        const int CoolHueMargin = 4;

        bool isCool(int red, int green, int blue) {
            (void)green;
            return blue >= red + CoolHueMargin;
        }

        // (the band typed: a bare 0 also converts to the predicate, which picks the other constructor)
        const std::uint8_t CoolBand = 0;

        const std::vector<MaterialBandRemapFilter::Band> BetweenBands = {
            MaterialBandRemapFilter::Band(118, 135, CoolBand, &isCool),
            MaterialBandRemapFilter::Band(150, 175, CoolBand, &isCool),
            MaterialBandRemapFilter::Band(240, 255, CoolBand, &isCool),
            MaterialBandRemapFilter::Band(150, 175, 126),
        };


        void alphaOne(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, HeadDiffuseAlpha);
        }


        GIMIComponentFixerConfig aetherHeavenConfig() {
            // Remapped onto AetherHeaven ("As Heaven and Earth Are Made Anew", 6.3) -- a skin of THREE components, the
            // LumineHeaven shape, and Aether is Lumine with one object fewer (no dress). Every value was read off the
            // prototype (Tools/Misc/Prototypes/aetherHeavenFix.py), checked in game on his identity mod and all ten of
            // his mods (restored copies of the ones whose own outfit renders green on current GIMI), and off the two
            // frame dumps.
            GIMIComponentFixerConfig config{};
            config.targetSkin = ModTypeIdTools::getName(ModTypeId::AetherHeaven);
            config.drawnObjs = {"head", "body"};

            // PLAIN, always: he has no normal map on any object, and an older GIMI mod's MetalMap / ShadowRamp at
            // ps-t2 / ps-t3 reads as a normal-map layout to Detect (Lumine's kimono went vivid green).
            config.sourceLayout = GIMIComponentFixerConfig::SourceLayout::Plain;

            // ...and each binding read by its resource NAME, where the names can be believed.
            config.texRegsByName = true;

            // Both characters read the face diffuse at ps-t1 (GI 6.x). Only a copy of the face would be swapped, and
            // there is none -- see Component::face below.
            config.faceSwapOnlyFromDiffuseReg = true;

            // A mod toggling variants of one object on an if / else if chain with no else draws EVERY variant at once
            // under the template's drawindexed = auto (Yaoyao2's lesson).
            config.fillDrawOnlyWhenUndrawn = true;

            // The main mesh. Its component name is EMPTY -- its files are AetherHeavenHead.ib,
            // AetherHeavenPosition.buf -- and only its fix-target id carries a name. His head (hair, the face skin it
            // holds) through the Head slot; his body (with its cape and scarf) through the Body slot.
            GIMIComponentFixerConfig::Component main{};
            main.name = "";
            main.modTypeName = ModTypeIdTools::getName(ModTypeId::AetherHeavenMain);
            main.slot = "Head";
            main.slotIndex = HeadSlot;
            main.objSlotIndices = {{"body", BodySlot}};
            main.slotIndices = {HeadSlot, BodySlot};
            main.negativeIndex = false;
            main.normalMap = true;
            main.texcoordStride = 12;       // AetherHeavenTexcoord.buf: 317052 / 26421
            main.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // His back cape, link for link, SHARED between the skin's back coat chain (70%) and its upper torso (50)
            // (three in-game rounds, 2026-10-25). On the chain alone his cape's two panels closed over the split
            // between them -- its top links meet at the skin's centre seam; on the torso alone they kept the gap they
            // are MODELLED with, far wider than his own card shows once his cape chains have pulled it in.
            const double CapeChainShare = 0.7;
            const std::vector<std::pair<long long, long long>> capeChain = {
                {42, 74}, {44, 74}, {43, 75}, {50, 75}, {45, 76}, {46, 76}, {51, 77}, {52, 77},
                {47, 78}, {48, 78}, {53, 79}, {54, 79}, {49, 80}, {28, 80}, {55, 81}, {27, 81}};
            for (const auto& [group, chainBone] : capeChain) {
                main.splitGroups[group] = {{chainBone, CapeChainShare}, {50, 1.0 - CapeChainShare}};
            }

            // NO component carries the face. The skin draws its OWN face meshes (013d13f1 / 3d359360; his are
            // 3e0837e2 / 3ff23b6d) with its own face atlas -- Lumine's lesson: a face atlas does not move between two
            // face meshes, and on the skin the skin's own face with his eyes (the Eye component) is drawn.
            main.face = false;

            // His cape and scarf are single-layer cloth INSIDE his body object, so the mirrored inner layer Lumine's
            // DRESS gets would twin his whole body; none is given. In the Dressing Room the inside of his cape matched
            // his own card from behind.
            main.mirrorBackUV = true;

            // ...and his layered clothes lose the outline of their INNER layers (core InnerLayerOutline), as Lumine's
            // do: the skin's outline shell sits further out than his.
            main.innerOutlineObjs = {"body"};

            // The Bang: one slot, on the normal-map layout with the Head's textures. His side locks and ahoge land here.
            GIMIComponentFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.modTypeName = ModTypeIdTools::getName(ModTypeId::AetherHeavenBang);
            bang.slot = "A";
            bang.slotIndex = "0";
            bang.slotIndices = {"0"};
            bang.negativeIndex = false;
            bang.normalMap = true;
            bang.face = false;
            bang.texcoordStride = 12;       // AetherHeavenBangTexcoord.buf: 38316 / 3193
            bang.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // ...and where his side locks blend into his head, the scalp is a HARD switch between two components: 28
            // vertices sit at one point in both buffers, on the main mesh's head (4) on one side and the Bang's lock
            // bones (13 / 15 / 8) on the other, which opens as a rip when the locks swing (Citlali's temple,
            // CitlaliFixer.cpp). So the Bang also draws one ring of the main mesh's triangles there, its head weight
            // standing in as the Bang's own head-parented bulk (Bang:0) instead of renormalising onto the locks:
            // measured on his identity mod, 34 of the 50 coincident points then move with the head on both sides.
            bang.overlapRings = 1;
            bang.standIns = {{2, 0}};

            // His eyes (vertex groups 0 / 1) are in his HEAD object. The skin draws its Eye on the PLAIN shader 95aa6cdb
            // with a diffuse / light map at ps-t0 / ps-t1 under NNFix. His eyes and the skin's coincide within 0.7 mm
            // (vertex-group centroids): no position offset.
            GIMIComponentFixerConfig::Component eye{};
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::AetherHeavenEye);
            eye.slot = "A";
            eye.slotIndex = "0";
            eye.slotIndices = {"0"};
            eye.negativeIndex = false;
            eye.normalMap = false;
            eye.face = false;
            eye.texcoordStride = 12;        // AetherHeavenEyeTexcoord.buf: 4800 / 400
            eye.slotRegisters = {"ps-t0", "ps-t1"};
            eye.offsetOnlyWithGameFace = true;

            // The Eye LAST: it owns the hidden components and the TexFx guards, so it has to be the last fixer to
            // run -- the same order as the rows in IniFixBuilderData.
            config.components = {main, bang, eye};

            // Every component receives a forward vertex-group row, so none is hidden by request.
            config.hiddenComponents = {};
            config.unremappedSlots = {};

            // The skin draws its OWN face and head-upper meshes: a mod hiding his by hash (a mask, a custom face)
            // hides the skin's too.
            config.sideMeshes = {"ib_face", "ib_headupper"};

            config.diffuseEdits = {{"head", &alphaOne}, {"body", &clearCoolGlow}};
            config.lightMapEdit = MaterialBandRemapFilter::lightMapEdit(BetweenBands);
            config.lightMapObjs = {"head", "body"};
            config.compressTextures = false;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::aetherHeavenMain6_3() {
        return makeGIMIComponentFixer(aetherHeavenConfig(), "");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::aetherHeavenBang6_3() {
        return makeGIMIComponentFixer(aetherHeavenConfig(), "Bang");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::aetherHeavenEye6_3() {
        return makeGIMIComponentFixer(aetherHeavenConfig(), "Eye");
    }


    IniFixBuilder::Factory AetherFixer::main6_3() {
        return IniFixBuilderFuncs::aetherHeavenMain6_3();
    }


    IniFixBuilder::Factory AetherFixer::bang6_3() {
        return IniFixBuilderFuncs::aetherHeavenBang6_3();
    }


    IniFixBuilder::Factory AetherFixer::eye6_3() {
        return IniFixBuilderFuncs::aetherHeavenEye6_3();
    }
}
