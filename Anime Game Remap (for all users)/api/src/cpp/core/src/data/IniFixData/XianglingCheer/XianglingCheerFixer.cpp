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

#include "AGRemapCore/data/IniFixData/XianglingCheer/XianglingCheerFixer.h"

#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/buffers/BufValue.h"
#include "AGRemapCore/model/files/BufFile.h"


namespace AGRemapCore {
    namespace {
        /**
         * The keys a REFLECTION section binds, which the remap has no use for.
         *
         * A mod can ship a mirror-image copy of a character, drawn from the same geometry through a
         * second set of resources. Those bindings name the SOURCE character's textures and its
         * '$CharacterIB' variable, neither of which means anything once the sections have been
         * remapped onto somebody else -- so they go, and the reflection simply stops drawing rather
         * than drawing the wrong model.
         *
         * A mod without reflections is untouched: a key that is not in the part is not removed.
         */
        std::vector<std::string> reflectionKeys(const std::string& obj) {
            return {"ResourceRef" + obj + "Diffuse", "ResourceRef" + obj + "LightMap",
                    "$CharacterIB"};
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::xianglingCheer6_1() {
        // Remapped onto Xiangling -- the SPLIT that undoes xiangling4_0's merge, and the direction
        // that LOSES a normal map.
        //
        // XianglingCheer is a 5.3-era model reading ps-t0 normal map / ps-t1 diffuse / ps-t2
        // lightmap; Xiangling predates GI 3.x's normal maps and reads ps-t0 diffuse / ps-t1
        // lightmap. So the normal map goes and both textures shift DOWN a slot.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // The skin's head covers geometry Xiangling splits between her head and her dress.
        config.objSplits = {{"head", {"head", "dress"}}, {"body", {"body"}}};

        // ---- the shift ----
        //
        // BOTH TARGETS OFF THE HEAD NEED IT. These name the TARGET's objects and run after the
        // split, so an entry for 'head' alone would leave the 'dress' copy -- the same source graph
        // -- still binding a normal map on ps-t0 and its diffuse a slot too high.
        std::vector<std::string> headRemovals = reflectionKeys("Head");
        headRemovals.push_back("ps-t0");

        std::vector<std::string> bodyRemovals = reflectionKeys("Body");
        bodyRemovals.push_back("ps-t0");

        config.objRegRemovals = {{"head", headRemovals}, {"dress", headRemovals},
                                 {"body", bodyRemovals}};

        // One entry per object holding both renames, so the pass cannot re-read its own output.
        const std::vector<std::pair<std::string, std::vector<std::string>>> shift = {
            {"ps-t1", {"ps-t0"}}, {"ps-t2", {"ps-t1"}}};
        config.objRegRemaps = {{"head", shift}, {"dress", shift}, {"body", shift}};

        // XIANGLING'S HEAD IS NEVER DRAWN. The skin's head mesh is already coming through the dress
        // copy above, and letting the head index range draw it as well puts a second copy of the
        // same triangles in the same place.
        config.objNewRegVals = {{"head", {{IniKeywords::Ib, "null"}}}};

        // ---- the position translation ----
        //
        // Xiangling and XianglingCheer were authored around DIFFERENT ORIGINS -- about 0.78
        // units apart in Y -- so a mod remapped between them is correct in every other way and
        // still lands in mid-air. Every vertex moves to match.
        //
        // This is the only pair in the repo that needs it: 46 of the 47 entries in the
        // pure-Python PositionEditorData table are None. See
        // GIMICharFixerConfig::positionEdit before assuming a new character wants one.
        const double OffsetY = -0.7755;
        const double OffsetZ = 0.0405;

        config.positionEdit = [OffsetY, OffsetZ](const BufLineData& line, long long, double, long long) {
            BufLineData result = line;

            // POSITION is the element name PositionFile gives the first 12 bytes of a vertex;
            // a line that somehow has none is left exactly as it came in rather than guessed at.
            auto position = result.find("POSITION");
            if (position == result.end() || position->second.size() < 3) {
                return result;
            }

            // A Float32 decodes to the variant's double alternative. Anything else is not a
            // coordinate this fix knows how to move, so it is left alone.
            if (std::holds_alternative<double>(position->second[1])) {
                position->second[1] = std::get<double>(position->second[1]) + OffsetY;
            }

            if (std::holds_alternative<double>(position->second[2])) {
                position->second[2] = std::get<double>(position->second[2]) + OffsetZ;
            }

            return result;
        };
        // objFixCalls left alone: NNFix on every object, which is the 6.1 default and what the
        // pure-Python row spells out by hand.
        //
        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries.
        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory XianglingCheerFixer::v6_1() {
        return IniFixBuilderFuncs::xianglingCheer6_1();
    }
}
