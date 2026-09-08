#include "AGRemapCore/data/IniFixData/JeanSeaFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {
    namespace {
        /**
         * The merge, shared by both targets -- Jean and JeanCN draw the same objects as each other,
         * so what JeanSea has to collapse is the same either way.
         *
         * 'head' is listed even though it maps to itself: objSplits is all-or-nothing, and an object
         * left out of it is dropped from the remap entirely.
         *
         * ORDER IS LOAD-BEARING. The first claimant of a target keeps the main .ini file and later
         * ones go to generated copies, so listing 'dress' before 'body' would put the cape in the
         * mod's own file and the body in the copy -- working, but backwards from what a reader
         * opening the mod expects to find.
         */
        GIMICharFixerConfig makeConfig() {
            GIMICharFixerConfig config{};
            config.drawnObjs = {"head", "body", "dress"};
            config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}};

            // The generated second file explains itself -- see IniComments::GIMIObjMergerPreamble,
            // which is the paragraph the pure-Python merge wrote for exactly this.
            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            // No texEdits: the ShadeLightMap edit belongs to the fixes that remap ONTO JeanSea (see
            // JeanFixer), not to the ones that remap off her. The pure-Python jeanSea6_1 row has no
            // RegTexEdit either.
            //
            // moveDrawIndexed stays false, as for the rest of the Jean family.
            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::jeanSea6_1ToJean() {
        return makeGIMICharFixer(makeConfig());
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::jeanSea6_1ToJeanCN() {
        return makeGIMICharFixer(makeConfig());
    }


    IniFixBuilder::Factory JeanSeaFixer::v6_1ToJean() {
        return IniFixBuilderFuncs::jeanSea6_1ToJean();
    }


    IniFixBuilder::Factory JeanSeaFixer::v6_1ToJeanCN() {
        return IniFixBuilderFuncs::jeanSea6_1ToJeanCN();
    }
}
