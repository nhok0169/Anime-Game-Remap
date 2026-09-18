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

#include "AGRemapCore/data/IniParseData/YelanTranquil/YelanTranquilParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::yelanTranquil5_7() {
        // The first parser for a skin of SEVERAL components -- see makeGIMIComponentParser for what
        // that means and for every mod object this produces. Only what YelanTranquil does
        // differently lives here.
        //
        // Everything below was read off her 5.7 frame analysis and her asset dump, and each
        // component's hashes are filed in HashData under the COMPONENT's own mod type name, because
        // each is a fix target of its own for the forward direction.
        //
        // The slot layouts are the shader's, not a guess from the file names: ORFix.ini's own
        // CommandListReference reads ps-t0 normal / ps-t1 diffuse / ps-t2 lightmap and re-slots them
        // by the ShaderOverride's filter_index, so a slot whose draw binds a normal map is the
        // three-register layout and one that does not is the two-register one. Her Body slot A
        // (dump draw 45) and slot B (44) read a normal map; slot C (46) does not.
        //
        // The Bang and the Eye have NO textures of their own: the game draws both with the Body
        // slot A set (dump draws 49-52 bind fe0fd573 / 183ca818 / f5cc10b7, which are A's). A mod
        // may therefore leave those sections with an ib and nothing else, and downloading a texture
        // for them would bind one the game never had there.
        GIMIComponentParserConfig config{};
        config.modTypeId = ModTypeId::YelanTranquil;
        config.downloadCharFolder = "YelanTranquil";
        config.downloadVersionFolder = "5_7";
        config.downloadPrefix = "YelanTranquil";

        GIMIComponentParserConfig::Component body{};
        body.name = "Body";
        body.modTypeName = ModTypeIdTools::getName(ModTypeId::YelanTranquilBody);
        body.texcoordStride = 20;
        body.vertexCount = 25954;
        body.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", false},
                      {"B", "53631", "ps-t1", "ps-t2", "ps-t0", false},
                      {"C", "67374", "ps-t0", "ps-t1", "", false}};

        GIMIComponentParserConfig::Component bang{};
        bang.name = "Bang";
        bang.modTypeName = ModTypeIdTools::getName(ModTypeId::YelanTranquilBang);
        bang.texcoordStride = 20;
        bang.vertexCount = 2256;
        // The trailing "Body;A" is the slot's texture DONOR: the slot the GAME draws it with. It
        // matters for any mod that leaves the slot textureless, which is three of the five measured
        // -- such a slot renders with the game's atlas in game, whatever the mod did to its own
        // copy of that atlas, so the donor's textures are DOWNLOADED rather than read out of the
        // mod. See GIMIComponentParserConfig::Slot::textureDonor.
        bang.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, "Body;A"}};

        // No second UV set: her Eye's vertex is 84 bytes where the others are 92, so its
        // Texcoord.buf is stride 12. One buffer has one stride, which is the merge's problem --
        // see VGComponentMerge -- but the download has to declare the real one.
        GIMIComponentParserConfig::Component eye{};
        eye.name = "Eye";
        eye.modTypeName = ModTypeIdTools::getName(ModTypeId::YelanTranquilEye);
        eye.texcoordStride = 12;
        eye.vertexCount = 120;
        eye.slots = {{"A", "0", "ps-t0", "ps-t1", "", true, "Body;A"}};

        config.components = {std::move(body), std::move(bang), std::move(eye)};

        return makeGIMIComponentParser(std::move(config));
    }


    IniParseBuilder::Factory YelanTranquilParser::v5_7() {
        return IniParseBuilderFuncs::yelanTranquil5_7();
    }
}
