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

#include "AGRemapCore/data/IniFixData/Kirara/KiraraFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/data/IniFixData/RegValChecks.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {

    namespace {
        // 1, not 0 -- see XianglingFixer's note on why a 1 survives the BC7 round trip where a 0
        // can be optimised away.
        const int NearlyTransparent = 1;


        // The copy of the face diffuse that ends up on ps-t0, with its alpha taken down. The
        // ORIGINAL is untouched and keeps ps-t1 -- see the texEdits entry below for how one edit
        // produces both.
        void makeFaceOpaque(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, NearlyTransparent);
        }


        // The three keys a GIMI reflection block leaves behind -- two resource references and the
        // '$CharacterIB' variable, neither of which means anything once the sections have been
        // renamed for a different character.
        std::vector<GIMICharFixerConfig::RegRef> reflectionKeys(const std::string& obj) {
            return {"ResourceRef" + obj + "Diffuse", "ResourceRef" + obj + "LightMap", "$CharacterIB"};
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::kirara6_1() {
        // Remapped onto KiraraBoots, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what Kirara does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        config.objRegRemovals = {{"head", reflectionKeys("Head")},
                                 {"body", reflectionKeys("Body")},
                                 {"dress", reflectionKeys("Dress")}};

        // ---- the dress shifts down a slot, CONDITIONALLY ----
        //
        // KiraraBoots' dress reads its diffuse from ps-t0 and its lightmap from ps-t1; Kirara's
        // sits a slot higher on each (see KiraraParser's objDownloadRegs for the same fact from
        // the download side). So the shift is ps-t1 -> ps-t0 and ps-t2 -> ps-t1.
        //
        // BOTH RULES ASK WHAT IS ACTUALLY BOUND THERE, and keep the register where it is when the
        // answer is no. That is not defensive coding -- it is the case where the mod's author
        // already hand-swapped these registers instead of re-issuing NNFix/ORFix, which leaves the
        // diffuse ALREADY on ps-t0. Shifting again would move it to a slot nothing reads. See
        // RegValChecks for why the resource's NAME is the signal.
        //
        // One entry holding both renames, so the pass cannot re-read its own output: remapKeys
        // consults the rules once per ORIGINAL key, which is what stops ps-t1 -> ps-t0 and
        // ps-t2 -> ps-t1 colliding.
        config.objRegRemaps = {{"dress", {{"ps-t1", {{"ps-t0", &RegValChecks::isDiffuse}}, true},
                                          {"ps-t2", {{"ps-t1", &RegValChecks::isLightMap}}, true}}}};

        // ---- the face keeps its diffuse AND gets an alpha-1 copy beside it ----
        //
        // toReg is what makes this a COPY rather than a move: without it the edit would repoint the
        // register it read from, and the untouched original would be referenced by nothing.
        //
        // READ FROM ps-t0, WRITTEN TO ps-t1, AND THAT IS NOT BACKWARDS. Both are PRE-swap
        // registers: texture edits run before the register edits, and the face's chain ends in the
        // diffuse <-> lightmap swap. So the copy written to ps-t1 here is the one that comes out on
        // ps-t0, and the original read from ps-t0 is the one that comes out on ps-t1 -- which is
        // exactly the arrangement asked for.
        config.texEdits = {{"face", "ps-t0", "OpaqueFaceDiffuse", &makeFaceOpaque, true, "", "ps-t1"}};

        // ---- which external library each object re-issues ----
        //
        // ORFix for the head and body, which carry a normal map; NNFix for the dress, which does
        // not. TexFx on all three, naming ps-t0 as the diffuse's home (TN.0) -- it is opt-in, so
        // the sub-command is added only where the mod bound ps-t69 or ps-t70.
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency0}},
                              {"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency0}},
                              {"dress", {IniKeywords::NNFixPath, IniKeywords::TexFxTransparency0}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory KiraraFixer::v6_1() {
        return IniFixBuilderFuncs::kirara6_1();
    }
}
