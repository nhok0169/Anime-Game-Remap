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

#include "AGRemapCore/data/IniParseData/BennettAdventure/BennettAdventureParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::bennettAdventure5_7() {
        // The second parser for a skin of SEVERAL components, and the same shape as YelanTranquil's
        // -- see makeGIMIComponentParser. Only what BennettAdventure does differently lives here.
        //
        // Every value below was read off her own identity mod (the game's model as a mod, built by
        // Tools/Misc/Prototypes/identityMod.py) and her asset dump, and each component's hashes are
        // filed in HashData under the COMPONENT's own mod type name, because each is a fix target
        // of its own for the forward direction.
        //
        // The slot layouts are the shader's, not a guess from the file names: a slot whose draw
        // binds a normal map is the three-register layout (ps-t0 normal / ps-t1 diffuse / ps-t2
        // lightmap, re-slotted by ORFix) and one that does not is the two-register one under NNFix.
        // Her Body's slots A and B read a normal map; her Eye does not.
        //
        // Her Bang and her Eye have NO textures of their own -- her identity mod's Bang and Eye
        // sections both bind Body slot A's diffuse and lightmap -- so a mod may leave those sections
        // with an ib and nothing else, and downloading a texture for them would bind one the game
        // never had there. Hence the "Body;A" donor on both.
        GIMIComponentParserConfig config{};
        config.modTypeId = ModTypeId::BennettAdventure;
        config.downloadCharFolder = "BennettAdventure";
        config.downloadVersionFolder = "5_7";
        config.downloadPrefix = "BennettAdventure";

        // Vertex counts are the GAME model's, counted off the download Blend.buf files (bytes / 32):
        // 833824 / 32, 79744 / 32 and 6464 / 32. They are what a mod MISSING the component is fixed
        // from -- IniFile::fix runs before the downloads are fetched, so the count cannot be
        // measured then. An NSFW body edit measured on 2026-09-15 carries no Eye sections at all.
        GIMIComponentParserConfig::Component body{};
        body.name = "Body";
        body.modTypeName = ModTypeIdTools::getName(ModTypeId::BennettAdventureBody);
        body.texcoordStride = 20;
        body.vertexCount = 26057;
        body.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", false},
                      {"B", "44334", "ps-t1", "ps-t2", "ps-t0", false}};

        // Stride 12, not 20: her Bang and her Eye carry no second UV set where her Body does
        // (29904 / 2492 and 2424 / 202, against the Body's 521140 / 26057). Getting this wrong is
        // invisible in the output and renders as blank white -- every UV read 8 bytes late.
        GIMIComponentParserConfig::Component bang{};
        bang.name = "Bang";
        bang.modTypeName = ModTypeIdTools::getName(ModTypeId::BennettAdventureBang);
        bang.texcoordStride = 12;
        bang.vertexCount = 2492;
        bang.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, "Body;A"}};

        GIMIComponentParserConfig::Component eye{};
        eye.name = "Eye";
        eye.modTypeName = ModTypeIdTools::getName(ModTypeId::BennettAdventureEye);
        eye.texcoordStride = 12;
        eye.vertexCount = 202;
        eye.slots = {{"A", "0", "ps-t0", "ps-t1", "", true, "Body;A"}};

        config.components = {std::move(body), std::move(bang), std::move(eye)};

        return makeGIMIComponentParser(std::move(config));
    }


    IniParseBuilder::Factory BennettAdventureParser::v5_7() {
        return IniParseBuilderFuncs::bennettAdventure5_7();
    }
}
