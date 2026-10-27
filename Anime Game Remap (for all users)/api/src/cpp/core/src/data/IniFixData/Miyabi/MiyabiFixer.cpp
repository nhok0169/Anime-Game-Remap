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

#include "AGRemapCore/data/IniFixData/Miyabi/MiyabiFixer.h"

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniParseBuilderData.h"


namespace AGRemapCore {

    ZZMIFixerConfig miyabiBlossomConfig() {
        // Every value here was read off the prototype (Tools/Misc/Prototypes/miyabiBlossomFix.py), which
        // the identity mod and eleven real mods confirmed in game against each mod on Miyabi.
        ZZMIFixerConfig config{};
        config.source = "Miyabi";
        config.target = "MiyabiBlossom";
        config.downloadCharFolder = "Miyabi";
        config.downloadVersionFolder = "3_2";

        // The textures Miyabi's Body and Legs draw that the skin draws under other hashes. The sword's
        // index buffers are shared by both outfits and its textures are not: they ride with the Body as
        // carry-only roles (a weapon-only .ini classifies by them).
        const std::vector<ZZMIFixerConfig::Texture> outfitTextures = {
            {"tex_a_diffuse", "diffuse"}, {"tex_a_lightmap", "lightmap"}, {"tex_a_materialmap", "materialmap"}};
        std::vector<ZZMIFixerConfig::Texture> bodyTextures = outfitTextures;
        bodyTextures.push_back({"tex_weapon_diffuse", "weapondiffuse", false});
        bodyTextures.push_back({"tex_weapon_lightmap", "weaponlightmap", false});
        bodyTextures.push_back({"tex_weapon_materialmap", "weaponmaterialmap", false});

        ZZMIFixerConfig::Component body;
        body.name = "Body";
        body.target = "Dress";
        body.slots = {{"a", "0", "0", 24567}};
        body.srcTexcoordStride = 24;
        body.targetTexcoordStride = 20;
        body.textures = bodyTextures;

        // THE BAND EDIT: a light map band picks a tint pair out of the draw's ps-cb3 (rows 39-43 lit,
        // 44-48 shade), and the skin's Dress table puts its SKIN tone on band 0, where Miyabi's is a cool
        // blue: her navy hakama rendered salmon. Each band onto the skin band whose tint pair is nearest
        // (the two 3.2 frame dumps). The Legs get none: moving them by the same rule turned a black dress
        // grey, and their own bands render as on Miyabi.
        body.bandMoves = {{0, 179}, {77, 179}, {128, 0}, {255, 179}};

        ZZMIFixerConfig::Component legs;
        legs.name = "Legs";
        legs.target = "Body";
        legs.slots = {{"a", "0", "0", 56682}};
        legs.srcTexcoordStride = 20;
        legs.targetTexcoordStride = 20;
        legs.textures = outfitTextures;

        ZZMIFixerConfig::Component hair;
        hair.name = "Hair";
        hair.target = "Hair";
        hair.slots = {{"a", "0", "0", 14916}, {"b", "15132", "14916", 6702}};

        ZZMIFixerConfig::Component hairShadow;
        hairShadow.name = "HairShadow";
        hairShadow.target = "HairShadow";
        hairShadow.slots = {{"a", "0", "0", 6702}};
        hairShadow.srcTexcoordStride = 24;
        hairShadow.targetTexcoordStride = 24;

        config.components = {body, legs, hair, hairShadow};

        // Mods that pre-skin their Body and draw it from their Legs section (Miyabi1 / 4 / 5 / 6 / 8):
        // MiyabiBlossom's Dress palette sits ~4 cm off its Body palette, so those draws go back to the Dress
        config.crossSlotDraws = {{"Body", "Legs"}};

        config.copyPreamble = IniComments::GIMIObjMergerPreamble;
        return config;
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::miyabi3_2() {
        return makeZZMIParser(miyabiBlossomConfig());
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::miyabiBlossomDress3_2() {
        return makeZZMIFixer(miyabiBlossomConfig(), "Body");
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::miyabiBlossomBody3_2() {
        return makeZZMIFixer(miyabiBlossomConfig(), "Legs");
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::miyabiBlossomHair3_2() {
        return makeZZMIFixer(miyabiBlossomConfig(), "Hair");
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::miyabiBlossomHairShadow3_2() {
        return makeZZMIFixer(miyabiBlossomConfig(), "HairShadow");
    }
}
