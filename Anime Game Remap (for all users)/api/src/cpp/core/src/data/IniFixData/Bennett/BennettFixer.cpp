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

#include "AGRemapCore/data/IniFixData/Bennett/BennettFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"


namespace AGRemapCore {
    namespace {
        // ---- the band legend ----
        //
        // A lightmap's alpha is a material band, and the legend differs per skin. Read off the two
        // models' own textures at their vertices:
        //
        //   Bennett:           0 = his silver HAIR on the head, and his dark CLOTH on the body
        //                      255 = his brass goggles and his skin
        //   BennettAdventure:  176-178 = her pale / hair ramp,  255 = her SKIN ramp
        //
        // ONE band moves. His hair sits on 0, which her legend reads as something else, so it is
        // lifted onto the middle of her pale ramp. His skin is already at 255 on both sides, and his
        // brass goggles are deliberately left where they are: none of her bands is a brass or metal
        // one, so there is nowhere measured to send them. A wrong lift is not a no-op -- it puts a
        // band value on a pixel that meant something else -- so nothing moves without a measurement.
        //
        // The gate is the same bright-and-close-to-grey test the prototype called `paleNeutral`.
        // whiteFurColoured is that test with slightly looser thresholds, and the two differ by 614
        // pixels out of 888305 on his head lightmap -- 0.07% -- so it is reused rather than
        // near-duplicated. What the gate CANNOT do is separate his two objects, which is why
        // lightMapObjs exists: see below.
        const std::vector<MaterialBandRemapFilter::Band> Bands = {
            {0, 177, &MaterialBandRemapFilter::whiteFurColoured},
        };


        GIMIComponentFixerConfig bennettAdventureConfig() {
            // Remapped onto BennettAdventure -- the second skin of SEVERAL components, after
            // YelanTranquil. See GIMIComponentFixerConfig for what each field does; every value here
            // was read off the prototype (Tools/Misc/Prototypes/bennettAdventureFix.py), which four
            // rounds on real mods confirmed in game.
            GIMIComponentFixerConfig config{};
            config.targetSkin = ModTypeIdTools::getName(ModTypeId::BennettAdventure);
            config.drawnObjs = {"head", "body"};

            // Which of her draw slots each component draws the mod through. All three take slot A,
            // and the Body's slotIndex is spelled out as "0" rather than left to IndexData for the
            // reason that field exists: a reverse lookup there resolves through the newest version
            // bucket holding a value, so a 5.x skin's slot at index 0 would shadow every classic
            // character's head.
            //
            // The Body takes the graph cut and carries the face; the Eye takes the cut. Her Body
            // reads the normal-map layout, her Eye does not.
            //
            // THERE IS NO BANG COMPONENT HERE, AND THAT IS THE DIFFERENCE FROM YELAN. Her Bang
            // genuinely receives geometry; BennettAdventure's cannot. Bennett has no hair bone --
            // his hair and his face are both on his head bone -- so his forward Bang vertex-group
            // row is EMPTY, and there is no partition that gives her Bang his hair without also
            // giving it his face. A Bang component here would draw nothing whatever it was handed.
            // His hair still arrives: the Body component draws it, as part of his head object.
            //
            // The consequence is NOT yet handled by this template -- see the note at the bottom of
            // this file.
            GIMIComponentFixerConfig::Component body{};
            body.name = "Body";
            body.modTypeName = ModTypeIdTools::getName(ModTypeId::BennettAdventureBody);
            body.slot = "A";
            body.slotIndex = "0";
            body.negativeIndex = false;
            body.normalMap = true;
            body.face = true;
            // Her Body carries a second UV set; vanilla Bennett does not, so his 12-byte Texcoord is
            // widened to her 20. Her Eye reads 12, so a mod that DOES carry one is narrowed there.
            body.texcoordStride = 20;

            GIMIComponentFixerConfig::Component eye{};
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::BennettAdventureEye);
            eye.slot = "A";
            eye.slotIndex = "0";
            eye.negativeIndex = false;
            eye.normalMap = false;
            eye.face = false;
            eye.texcoordStride = 12;

            config.components = {body, eye};

            // Her Bang receives nothing (see above) -- but it still DRAWS. Left alone, her own front
            // fringe sits on top of the hair the Body component draws, which in game reads as parts
            // of the hair having two different shades of white. Suppress its draw instead.
            config.hiddenComponents = {ModTypeIdTools::getName(ModTypeId::BennettAdventureBang)};

            // NO diffuse edit. Yelan's recipe puts her head diffuse at alpha 1, because Tranquil's
            // shader darkens by diffuse alpha and Yelan's ignores it. Neither half of that was
            // measured for this pair, and an alpha change is not a no-op: it is the material band
            // selector on a lightmap and an opacity on a diffuse. His textures pass through
            // untouched until a measurement says otherwise.
            config.diffuseEdits = {};

            // The hair lift, on the HEAD ONLY. The band legend is per OBJECT here, which is the one
            // way this config differs in kind from Yelan's: band 0 is his silver hair on his head
            // and his dark cloth on his body, and the colour gate does not separate them. Measured
            // on his shipped 4.0 assets, the gate takes 84.7% of the head lightmap -- the whole
            // hair -- and still 6.0% of the BODY's, 62462 pixels that mean cloth there.
            config.lightMapEdit = MaterialBandRemapFilter::lightMapEdit(Bands);
            config.lightMapObjs = {"head"};

            // NOT BC7-compressed, because the thing being edited is the alpha and the alpha is a
            // band SELECTOR: a 4x4 block holding both a moved pixel (177) and an unmoved one (255)
            // is split the difference by the encoder, and a band legend reads the result as a
            // different material. Measured on the prototype: re-compression moved an UNEDITED body
            // map's band from 255 to 254.
            config.compressTextures = false;

            // The generated second and third files explain themselves.
            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::bennettAdventureBody6_1() {
        return makeGIMIComponentFixer(bennettAdventureConfig(), "Body");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::bennettAdventureEye6_1() {
        return makeGIMIComponentFixer(bennettAdventureConfig(), "Eye");
    }


    IniFixBuilder::Factory BennettFixer::body6_1() {
        return IniFixBuilderFuncs::bennettAdventureBody6_1();
    }


    IniFixBuilder::Factory BennettFixer::eye6_1() {
        return IniFixBuilderFuncs::bennettAdventureEye6_1();
    }
}

// ============================================================================================
// Why her Bang is HIDDEN rather than remapped onto (2026-09-15)
// ============================================================================================
//
// Bennett has no hair bone -- his hair and his face are both on his head bone -- so his forward
// Bang vertex-group row is empty and there is no partition that gives her Bang his hair without
// also giving it his face. A Bang component would draw nothing whatever it was handed, which is why
// there is no Bang entry in `components` and no Bang row in IniFixBuilderData.
//
// But a target component nothing is remapped onto STILL DRAWS the skin's own geometry: her front
// fringe would sit on top of the hair his Body component draws, and in game that reads as parts of
// the hair having two different shades of white. `hiddenComponents` suppresses that draw with a
// TextureOverride on her Bang's ib hash carrying `handling = skip` and no drawindexed -- the same
// shape the fix leaves on a component it DOES remap, minus the re-issued draw.
//
// See AI Agent Help/CreatingRemaps/CLAUDE.md, "A TARGET COMPONENT NOTHING IS REMAPPED ONTO STILL
// DRAWS THE SKIN'S OWN GEOMETRY", which is the round of in-game work this came from.
