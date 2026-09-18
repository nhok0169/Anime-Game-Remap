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

#include "AGRemapCore/data/IniFixData/ShenheFrostFlower/ShenheFrostFlowerFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::shenheFrostFlower5_7() {
        // THE 5.7 FIX for ShenheFrostFlower -> Shenhe, verified only against the old script at
        // --version 5.7 --fromVersion 5.7 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress", "extra"};

        // Its map is {"head": ["head", "head", "head"], "body": ["head", "body", "extra"]} --
        // TARGET <- sources. Shenhe's head is fed by ShenheFrostFlower's head three times AND
        // her body by that same head once, so the head has FOUR targets once inverted. That is
        // the three-file merge this character is known for.
        config.objSplits = {{"head", {"head", "head", "head", "body"}},
                            {"body", {"body"}}, {"extra", {"body"}},
                            {"dress", {"dress", "dress", "dress"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // The head copies draw no geometry of their own.
        config.objNewRegVals = {{"head", {{"ib", "null"}}}};

        // ---- the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;

        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::shenheFrostFlower4_4() {
        // THE 4.4 FIX for ShenheFrostFlower -> Shenhe, verified only against the old script at
        // --version 4.4 --fromVersion 4.4 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress", "extra"};

        // THE MERGE: Shenhe has no extra, so ShenheFrostFlower's body and extra both land on
        // her body. Its map is {"body": ["body", "extra"]} -- head and dress are OMITTED, so
        // each goes into every generated file and is written out twice here.
        config.objSplits = {{"head", {"head", "head"}}, {"body", {"body"}},
                            {"extra", {"body"}}, {"dress", {"dress", "dress"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;

        // The external libraries this row re-issues, keyed by TARGET. An empty list means
        // none, and REPLACES the template's default NNFix.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::shenheFrostFlower6_1() {
        // Remapped onto Shenhe -- the widest MERGE here, and the mirror of shenhe6_1's split.
        //
        // The skin draws head, body, dress and extra. Shenhe draws head, body and dress. What makes
        // this wider than Keqing's merge is that THREE of the skin's objects have to come through
        // Shenhe's single body draw call -- her head included, because the skin's head mesh carries
        // outfit geometry Shenhe's head index range is too small to show.
        //
        // Three sources on one target means THREE .ini files: the mod's own, plus RemapFix1 and
        // RemapFix2. The game loads all three and overlaps them.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress", "extra"};

        // ---- how to read this ----
        //
        // Each entry is one source object and the targets it becomes, and a target's claims are
        // handed out in the order they appear ACROSS the whole list: first claim keeps the mod's
        // own file, second goes to RemapFix1, third to RemapFix2. Reading the claims out in order:
        //
        //     mod's own file    head <- head    body <- head     dress <- dress
        //     RemapFix1         head <- head    body <- body     dress <- dress
        //     RemapFix2         head <- head    body <- extra    dress <- dress
        //
        // which is exactly the pure-Python row's three columns
        // ({"head": ["head", "head", "head"], "body": ["head", "body", "extra"],
        //   "dress": ["dress", "dress", "dress"]}), written the other way round.
        //
        // Listing the same source/target pair repeatedly is how a graph is asked for in EVERY file
        // rather than just the first -- the same mechanism makeGIMICharFixer uses internally to put
        // the blend, position, texcoord and face in all three. Without the repeats, RemapFix1 and
        // RemapFix2 would carry a body and nothing to wear it on.
        config.objSplits = {{"head", {"head", "body", "head", "head"}},
                            {"body", {"body"}},
                            {"extra", {"body"}},
                            {"dress", {"dress", "dress", "dress"}}};

        // The generated files explain themselves -- see IniComments::GIMIObjMergerPreamble.
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // SHENHE'S HEAD IS NEVER DRAWN, in any of the three files. It is here to bind the head's
        // textures, not its geometry: the skin's head mesh is already coming through the body draw
        // call above, and letting the head index range draw it as well puts a second copy of the
        // same triangles in the same place.
        //
        // Existing values are replaced and a part with no 'ib' does not grow one, so this is a
        // no-op on a mod that never bound one.
        config.objNewRegVals = {{"head", {{IniKeywords::Ib, "null"}}}};

        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries that
        // switch it on.
        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory ShenheFrostFlowerFixer::v6_1() {
        return IniFixBuilderFuncs::shenheFrostFlower6_1();
    }
}
