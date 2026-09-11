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

#include "AGRemapCore/data/IniFixData/KiraraBoots/KiraraBootsFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/data/IniFixData/RegValChecks.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {

    namespace {
        const int NormalMapSize = 1024;

        // Kirara's own normal map is YELLOW rather than the usual flat blue -- Colours.NormalMapYellow
        // in the pure-Python table, which is (128, 128, 0) and not the (128, 128, 255) every other
        // character here invents. A normal map is a direction field, so the colour IS the data:
        // inventing the wrong one tilts every surface normal on the model, and nothing reports it.
        const Colour NormalMapYellow(128, 128, 0);

        std::vector<GIMICharFixerConfig::RegRef> reflectionKeys(const std::string& obj) {
            return {"ResourceRef" + obj + "Diffuse", "ResourceRef" + obj + "LightMap", "$CharacterIB"};
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::kiraraBoots6_1() {
        // Remapped onto Kirara, a genuinely different model -- see makeGIMICharFixer for what that
        // shape does. Only what KiraraBoots does differently lives here.
        //
        // The MIRROR of her base's fix: there the dress shifts DOWN a slot to meet the skin, here
        // the head shifts UP to meet Kirara -- who reads a normal map on ps-t0 that this skin does
        // not have, so the fix invents one.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        config.objRegRemovals = {{"head", reflectionKeys("Head")},
                                 {"body", reflectionKeys("Body")},
                                 // Kirara's dress does not read ps-t2; this skin's binds one.
                                 {"dress", [] {
                                      std::vector<GIMICharFixerConfig::RegRef> keys = reflectionKeys("Dress");
                                      keys.push_back("ps-t2");
                                      return keys;
                                  }()}};

        // ---- ONLY THE HEAD SHIFTS, and only it gets a normal map ----
        //
        // Worth stating because the obvious reading is wrong: her head and body have the SAME
        // download layout (see KiraraBootsParser), so it looks like they should shift together.
        // The pure-Python row shifts the head alone and adds the normal map to the head alone --
        // its body entry is an identity rename carrying nothing but the ORFix placement this
        // template owns. Giving the body the head's shift would move its diffuse to a slot Kirara
        // reads as a lightmap.
        //
        // ps-t1's lightmap goes up to ps-t2, and ps-t0's diffuse goes to ps-t1 while KEEPING a
        // copy of itself -- the slot has to stay bound for the texAdd below to overwrite, which is
        // the same trick Ganyu and Xiangling use.
        //
        // Both rules ask what is actually bound before moving it, for the reason KiraraFixer's own
        // shift gives: a mod already hand-fixed for 6.1 has these where the fix wants them.
        config.objRegRemaps = {{"head", {{"ps-t1", {{"ps-t2", &RegValChecks::isLightMap}}, true},
                                         {"ps-t0", {{"ps-t0", &RegValChecks::isDiffuse},
                                                    {"ps-t1", &RegValChecks::isDiffuse}}, true}}}};

        // ...and the normal map Kirara reads on ps-t0, which this skin never had.
        config.texAdds = {{"head", "ps-t0", "NormMap", TexCreator(NormalMapSize, NormalMapSize, NormalMapYellow)}};

        config.objFixCalls = {{"head", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency0}},
                              {"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency0}},
                              {"dress", {IniKeywords::NNFixPath, IniKeywords::TexFxTransparency0}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory KiraraBootsFixer::v6_1() {
        return IniFixBuilderFuncs::kiraraBoots6_1();
    }
}
