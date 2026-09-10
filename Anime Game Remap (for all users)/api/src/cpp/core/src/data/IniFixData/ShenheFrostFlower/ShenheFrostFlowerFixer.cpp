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
