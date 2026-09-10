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

#include "AGRemapCore/data/IniFixData/Shenhe/ShenheFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::shenhe6_1() {
        // Remapped onto ShenheFrostFlower -- a SPLIT, and a smaller one than KeqingOpulent's: only
        // the dress divides.
        //
        // Shenhe draws head, body and dress. ShenheFrostFlower draws those three plus an 'extra',
        // the Lantern Rite skin's separate skirt panel. Nothing in a Shenhe mod carries geometry
        // for it, so the dress graph is emitted twice -- once at the skin's dress index (66588) and
        // once at its extra index (70068), both under the skin's own ib hash.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // 'head' and 'body' map to themselves and are listed anyway: objSplits is all-or-nothing,
        // and an object left out of it is dropped from the remap entirely.
        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"dress", "extra"}}};

        // ---- the dress's register shift ----
        //
        // Shenhe's dress binds four textures; the skin's dress shader reads three, and its third
        // slot holds what Shenhe keeps in her fourth. So ps-t2 goes and ps-t3 slides down into the
        // hole it leaves.
        //
        // BOTH TARGETS NEED IT, and that is the half worth pausing on. These fields name the
        // TARGET's objects, applied after the split -- so one entry for 'dress' would leave the
        // 'extra' copy, which came off exactly the same source graph, still binding a ps-t2 the
        // shader was not expecting. The pure-Python row does not have to say this twice because its
        // filters run BEFORE the split, on the single source object.
        //
        // A mod whose dress binds neither register is untouched by both, which is the common case:
        // most Shenhe mods carry only a diffuse and a lightmap.
        config.objRegRemovals = {{"dress", {"ps-t2"}}, {"extra", {"ps-t2"}}};
        config.objRegRemaps = {{"dress", {{"ps-t3", {"ps-t2"}}}},
                                {"extra", {{"ps-t3", {"ps-t2"}}}}};

        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries that
        // switch it on.
        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory ShenheFixer::v6_1() {
        return IniFixBuilderFuncs::shenhe6_1();
    }
}
