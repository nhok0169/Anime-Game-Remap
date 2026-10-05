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

#include "AGRemapCore/data/IniFixData/Citlali/CitlaliFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"


namespace AGRemapCore {
    namespace {
        // ---- the band legend (measured 2026-09-21, over the texels each object's UVs cover) ----
        //
        //   Citlali head          0 (pale),  255 = her navy HAIR
        //   Citlali body          0 (purple cloth),  78-80 (navy cloth),  255 = her SKIN (96% skin-coloured)
        //   CitlaliWhisperofStars Body A: 0, 78 (pale cloth), 128, 177 = SKIN (92% skin-coloured),
        //                         255 = HAIR (its Bangs, which share A's textures, are 100% alpha 255)
        //
        // ONE band moves: her body's skin from 255 onto the skin's skin ramp at 177, gated on a
        // skin-coloured diffuse and on the BODY object only -- 255 on her head is her hair, which
        // already sits on the skin's hair ramp. Her cloth bands (0, 78-80) have ramps of the same
        // number on slot A.
        const std::vector<MaterialBandRemapFilter::Band> Bands = {
            {255, 177, &MaterialBandRemapFilter::skinColoured},
        };


        GIMIComponentFixerConfig citlaliWhisperofStarsConfig() {
            // Remapped onto CitlaliWhisperofStars -- the third skin of SEVERAL components. Every
            // value was read off the prototype (Tools/Misc/Prototypes/citlaliWhisperofStarsFix.py),
            // confirmed in game on her identity mod and one real mod, and off the two 6.7 frame
            // dumps (Tools/Misc/Diagnostics/giDrawTable.py).
            GIMIComponentFixerConfig config{};
            config.targetSkin = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStars);
            config.drawnObjs = {"head", "body"};

            // What differs in KIND from Bennett: Citlali's own sections are already the normal-map
            // layout her target slots read, so a mod's normal map, diffuse and light map pass through
            // -- no register shift and no invented normal map -- and a mod still on the plain layout
            // (no ps-t2) gets Bennett's shift. Per object, read off the mod's own section.
            config.sourceLayout = GIMIComponentFixerConfig::SourceLayout::Detect;

            // Both characters read the face diffuse at ps-t1 (GI 6.x). Swap only a mod still on ps-t0.
            config.faceSwapOnlyFromDiffuseReg = true;

            // Every component draws through slot A, spelled out as "0" rather than read from
            // IndexData -- see Component::slotIndex. Slot A: the only Body slot with the skin's
            // skin ramp at any size, and its hair ramp is at Citlali's hair's 255. Head and body
            // both land on it, so the Body fixer writes a second .ini file (the merge).
            //
            // All three components read the normal-map layout (shader 2c157719180b096c in the
            // frame dump). Her hair has bones of its own (21-34), so the Bangs is a real target,
            // unlike BennettAdventure's. All three take the graph cut: on her identity mod every
            // triangle is drawn by exactly one component.
            GIMIComponentFixerConfig::Component body{};
            body.name = "Body";
            body.modTypeName = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsBody);
            body.slot = "A";
            body.slotIndex = "0";
            body.negativeIndex = false;
            body.normalMap = true;
            body.face = true;
            body.texcoordStride = 20;
            body.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // ---- the ground plane ----
            //
            // THE TWO MODELS STAND ON DIFFERENT HEIGHTS. A mod's vertices are Citlali's, and the
            // skinning is the identity at rest, so a remapped mod renders at CITLALI's coordinates
            // while the game plants the character by the SKIN's -- and Citlali's sole sits 0.045
            // lower than CitlaliWhisperofStars'. Measured off the two download folders: both models'
            // lowest 400 vertices are the soles, dominated by the matching toe and foot groups
            // (Citlali 109/113/133/137 -> the skin's 25/29/49/53), and each sole is a flat plane --
            // 50 vertices inside 3 mm at y = -0.0629 for Citlali and y = -0.0178 for the skin.
            // Unshifted, the whole remapped model sits 4.5 cm low and her feet sink into the ground
            // in the overworld (reported 2026-10-04).
            //
            // Set on `body` BEFORE the two copies below, so all three components move together --
            // shifting one of them alone shears the model apart at its seams.
            body.positionOffset = {0.0f, 0.0450f, 0.0f};

            // AND IT FADES OUT BY THE FACE, which is the half this did not have on its first run
            // (2026-10-04). This mod carries NO face mesh -- only a face DIFFUSE bound by hash -- so
            // the GAME draws the skin's face, at the skin's fixed height. Lifting every vertex put
            // the feet right and left the face 4.5 cm below the head it belongs to, which the user
            // reported straight back. Full lift at Citlali's sole, none at or above y 1.400, which
            // is just over her eye groups at 1.384: the head stays where the game's face is and the
            // 4.5 cm is taken up through the body, which is where the two models differ anyway (the
            // skin's foot and knee groups sit 2-6 cm higher than hers, her head group 1 cm lower).
            body.positionOffsetFade = {-0.0629f, 1.400f};

            GIMIComponentFixerConfig::Component bangs = body;
            bangs.name = "Bangs";
            bangs.modTypeName = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsBangs);
            bangs.face = false;
            bangs.texcoordStride = 12;

            GIMIComponentFixerConfig::Component eyes = bangs;
            eyes.name = "Eyes";
            eyes.modTypeName = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsEyes);

            // ---- the two long front braids ----
            //
            // THE CHAIN USED TO BE CUT IN HALF ACROSS TWO COMPONENTS, AND THAT WAS THE WHOLE BUG.
            // Each braid is a four-link chain of Citlali's (23 -> 24 -> 25 -> 26, mirror
            // 29 -> 30 -> 31 -> 32) hanging from the temple to the chest. Its top two links went to
            // the Bangs' own front-hair chain and its bottom two to a bone of the Body, so one braid
            // was skinned in two index spaces by two unrelated bones. A user reported both halves of
            // that in one sentence: "the braid is stationary sticking to her body instead of flowing
            // naturally like hair", and "the braid is dislocated from her hair" -- the bottom half
            // rode a bone that does not move, and a `pushAway` put here to clear the arm displaced
            // only that half, so the braid visibly broke at the seam. It is the same fault as
            // Neuvillette's capes ripping along a component boundary, and the same rule applies:
            // ONE part belongs on ONE component's bones. VGRemapData.cpp sends all four links to
            // the skin's own front lock, Bangs 3 -> 5 -> 7 (mirror 4 -> 6 -> 8).
            //
            // AND THE AMPLITUDE IS DAMPED HERE RATHER THAN BY PICKING A DIFFERENT BONE, which is
            // what three earlier attempts did and none of them fixed anything. Bone 7 pivots at
            // y 1.279 and the skin's own lock ends at 1.241 -- a 3.8 cm lever -- while Citlali's
            // braid runs on to 1.037, 24 cm below it, so the same few degrees of hair sim move her
            // tip about six times as far, which is why continuing the chain bare threw it into the
            // arm when that was tried. splitGroups shares each lower link's weight with the Bangs'
            // own head bone, in the ratio of the skin's lever to that link's: 3.8 / 10.8 cm for
            // group 25 (centroid y 1.171) and 3.8 / 18.4 cm for group 26 (y 1.095). The braid swings
            // with the hair and hangs from the head, as it does on Citlali -- it simply swings the
            // distance the skin's rig was built to move rather than six times it.
            bangs.splitGroups = {{25, {{7, 0.35}, {0, 0.65}}}, {26, {{7, 0.20}, {0, 0.80}}},
                                 {31, {{8, 0.35}, {0, 0.65}}}, {32, {{8, 0.20}, {0, 0.80}}}};

            config.components = {body, bangs, eyes};

            // Every component receives geometry, so nothing of the skin's is hidden. Its Body slot D
            // (a small plain-shader piece) stops drawing with the rest of its Body, whose ib section
            // the fix leaves skipping.
            config.hiddenComponents = {};

            // Body slots B, C and D -- every Body draw but slot A's. Slot B's outline draws before
            // slot A's, so without these a TexFx request from the mod's slot A draw was served
            // there: the skin's whole ib over the mod's buffers, as a shiny web between the arms
            // and the hair (2026-09-22). Bangs and Eyes are one slot each.
            config.unremappedSlots = {{ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsBody), {"60888", "111096", "122916"}}};
            config.diffuseEdits = {};

            config.lightMapEdit = MaterialBandRemapFilter::lightMapEdit(Bands);
            config.lightMapObjs = {"body"};

            // NOT BC7-compressed: the alpha being edited is a band SELECTOR -- see Bennett's config.
            config.compressTextures = false;

            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::citlaliWhisperofStarsBody6_7() {
        return makeGIMIComponentFixer(citlaliWhisperofStarsConfig(), "Body");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::citlaliWhisperofStarsBangs6_7() {
        return makeGIMIComponentFixer(citlaliWhisperofStarsConfig(), "Bangs");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::citlaliWhisperofStarsEyes6_7() {
        return makeGIMIComponentFixer(citlaliWhisperofStarsConfig(), "Eyes");
    }


    IniFixBuilder::Factory CitlaliFixer::body6_7() {
        return IniFixBuilderFuncs::citlaliWhisperofStarsBody6_7();
    }


    IniFixBuilder::Factory CitlaliFixer::bangs6_7() {
        return IniFixBuilderFuncs::citlaliWhisperofStarsBangs6_7();
    }


    IniFixBuilder::Factory CitlaliFixer::eyes6_7() {
        return IniFixBuilderFuncs::citlaliWhisperofStarsEyes6_7();
    }
}
