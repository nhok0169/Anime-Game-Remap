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

#include "AGRemapCore/data/IniParseData/CitlaliWhisperofStars/CitlaliWhisperofStarsParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::citlaliWhisperofStars6_7() {
        // The third parser for a skin of SEVERAL components -- see makeGIMIComponentParser for what
        // that means and for every mod object this produces. Only what CitlaliWhisperofStars does
        // differently lives here, and it was read off her 6.7 frame analysis and her asset dump.
        //
        // The slot layouts are the SHADER's, not a guess from the file names: her slots on shader
        // 2c157719180b096c bind normal map / diffuse / light map at ps-t0/1/2 under ORFix, and Body
        // D binds diffuse / light map at ps-t0/1 under NNFix. What a MOD writes is a different
        // question again -- most are written in the game's own register order -- and that is the
        // fixer's texRegsByName, not this.
        //
        // Her Bangs and Eyes have NO textures of their own: the game draws both with the Body slot
        // A set (same hashes), so a mod may leave those sections with an ib and nothing else.
        GIMIComponentParserConfig config{};
        config.modTypeId = ModTypeId::CitlaliWhisperofStars;
        config.downloadCharFolder = "CitlaliWhisperofStars";
        config.downloadVersionFolder = "6_7";
        config.downloadPrefix = "CitlaliWhisperofStars";

        GIMIComponentParserConfig::Component body{};
        body.name = "Body";
        body.modTypeName = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsBody);
        body.texcoordStride = 20;
        body.vertexCount = 37631;
        body.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", false},
                      {"B", "60888", "ps-t1", "ps-t2", "ps-t0", false},
                      {"C", "111096", "ps-t1", "ps-t2", "ps-t0", false},
                      {"D", "122916", "ps-t0", "ps-t1", "", false}};

        // The trailing "Body;A" is the slot's texture DONOR: the slot the GAME draws it with. Such
        // a slot renders with the game's atlas whatever the mod did to its own copy, so the
        // donor's textures are DOWNLOADED rather than read out of the mod. The `true` after it is
        // donorNormalMap, which YelanTranquil's and BennettAdventure's donors do not set: Citlali
        // READS a normal map, so a borrowing slot needs the donor's normal map downloaded too, or
        // the draw takes whatever normal map the game had bound.
        GIMIComponentParserConfig::Component bangs{};
        bangs.name = "Bangs";
        bangs.modTypeName = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsBangs);
        bangs.texcoordStride = 12;
        bangs.vertexCount = 3210;
        bangs.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, "Body;A", true}};

        GIMIComponentParserConfig::Component eyes{};
        eyes.name = "Eyes";
        eyes.modTypeName = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsEyes);
        eyes.texcoordStride = 12;
        eyes.vertexCount = 255;
        eyes.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, "Body;A", true}};

        config.components = {std::move(body), std::move(bangs), std::move(eyes)};

        return makeGIMIComponentParser(std::move(config));
    }


    IniParseBuilder::Factory CitlaliWhisperofStarsParser::v6_7() {
        return IniParseBuilderFuncs::citlaliWhisperofStars6_7();
    }
}
