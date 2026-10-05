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

#include "AGRemapCore/data/IniFixData/Ningguang/NingguangFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/DarkDiffuse.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::ningguang4_0() {
        // THE 4.0 FIX for Ningguang -> NingguangOrchid. Kept for the historical record: the game cannot be
        // rolled back, so an A/B against the old script at --version 4.0 --fromVersion 4.0 is
        // the whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // The head diffuse, blanked and gamma-corrected. The 4.0 row's whole body is this one
        // RegTexEdit -- none of the ps-t3 stripping her 6.1 row adds.
        config.texEdits = {{"head", "ps-t0", "DarkDiffuse", &DarkDiffuse::edit}};

        // ---- the three 6.1-era defaults this row predates ----
        //
        // The face register swap corrects something GI 6.x did to the shader; at 4.0 the diffuse
        // still belongs on faceDiffuseReg.
        config.swapFaceRegs = false;

        // Nothing in this row's pure-Python body removes the mod's own ORFix/NNFix calls, and
        // nothing re-issues them, so both halves of that machinery stay off. Leaving the removal
        // on would delete the modder's call with nothing putting it back.
        config.removeSrcFixCalls = false;

        // ...and the default NNFix re-issue. An objFixCalls entry REPLACES the default
        // for its target, including with an empty list, and it is keyed by TARGET -- a
        // split's second copy is its own target and needs its own entry.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::ningguang6_1() {
        // Remapped onto NingguangOrchid, a genuinely different model -- see makeGIMICharFixer for
        // what that shape does. Only what Ningguang does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // NingguangOrchid does not read ps-t3 on any of the three. Ningguang binds one, and a
        // register the target's shader never samples is at best ignored and at worst read as
        // something else -- the pure-Python row strips it from all three objects and so does this.
        config.objRegRemovals = {{"head", {"ps-t3"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t3"}}};

        // The head diffuse, blanked and gamma-corrected -- see DarkDiffuse for what the two halves
        // are for. Declared on the head's ps-t0 in the pure-Python parser row and repointed at the
        // same register by the fixer's RegTexEdit, so unlike Jean's there is no shift to follow.
        config.texEdits = {{"head", "ps-t0", "DarkDiffuse", &DarkDiffuse::edit}};

        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory NingguangFixer::v6_1() {
        return IniFixBuilderFuncs::ningguang6_1();
    }
}
