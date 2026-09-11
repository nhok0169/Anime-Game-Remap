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

#include "AGRemapCore/data/IniFixData/Nilou/NilouFixer.h"

#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/data/IniFixData/RegValChecks.h"


namespace AGRemapCore {

    namespace {
        // The three keys a GIMI reflection block leaves behind -- two resource references and the
        // '$CharacterIB' variable, neither of which means anything once the sections have been
        // renamed for a different character.
        std::vector<GIMICharFixerConfig::RegRef> reflectionKeys(const std::string& obj) {
            return {"ResourceRef" + obj + "Diffuse", "ResourceRef" + obj + "LightMap", "$CharacterIB"};
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::nilou5_7() {
        // Remapped onto NilouBreeze, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what Nilou does differently lives here.
        //
        // ===== REGISTERED AT toVersion 5.7, WITH NO 6.1 ROW, AND THAT IS DELIBERATE =====
        //
        // The pure-Python table has no nilou6_1 either, and the reason is the same one Xiangling
        // records: GI 6.1 swapped which registers the shader reads the diffuse and the lightmap
        // out of, and ORFix's authors baked that swap INTO ORFix. Every object of this fix already
        // re-issues ORFix (see objFixCalls below), so all three get the 6.1 correction for free and
        // there is nothing for a 6.1 row to add.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // ---- what each object stops binding ----
        //
        // The normal map first, and it is the one entry here that has to ASK rather than name:
        // 'ps-t0' is where a 4.0-era Nilou keeps her normal map, but a mod whose author already
        // hand-fixed it for 6.1 has moved something else there. RegValChecks::isNormalMap makes the
        // removal fire only on an occurrence that still looks like a normal map, so a mod that has
        // already been fixed keeps whatever it put there. See RegValChecks for why a NAME is the
        // signal and why reading the texture's pixels was rejected.
        std::vector<GIMICharFixerConfig::RegRef> headRemovals = reflectionKeys("Head");
        headRemovals.push_back({"ps-t0", &RegValChecks::isNormalMap});

        std::vector<GIMICharFixerConfig::RegRef> bodyRemovals = reflectionKeys("Body");
        bodyRemovals.push_back({"ps-t0", &RegValChecks::isNormalMap});

        std::vector<GIMICharFixerConfig::RegRef> dressRemovals = reflectionKeys("Dress");
        dressRemovals.push_back({"ps-t0", &RegValChecks::isNormalMap});

        config.objRegRemovals = {{"head", headRemovals},
                                 {"body", bodyRemovals},
                                 {"dress", dressRemovals}};

        // ---- which external library each object re-issues ----
        //
        // ORFix on all three, which is what makes the missing 6.1 row correct. The pure-Python row
        // expresses this as a temp register threaded next to the lightmap and then renamed to
        // 'run'; none of that is transcribed, because placement is makeGIMICharFixer's job here and
        // it deliberately does NOT copy the old script's topology -- see CreatingRemaps' "Where
        // drawindexed goes decides whether the mod's own effects work".
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath}},
                              {"body", {IniKeywords::ORFixPath}},
                              {"dress", {IniKeywords::ORFixPath}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory NilouFixer::v5_7() {
        return IniFixBuilderFuncs::nilou5_7();
    }
}
