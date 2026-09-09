#include "AGRemapCore/data/IniFixData/JeanCN/JeanCNFixer.h"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/data/IniFixData/JeanShading.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::jeanCN6_1ToJean() {
        // The ordinary CN-skin remap -- a genuinely different model, so see makeGIMICharFixer for
        // what that shape does. Nothing here is JeanCN-specific beyond her drawn objects.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // moveDrawIndexed stays false: the pure-Python row for JeanCN carries none of the Ib*
        // entries Amber's does.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::jeanCN6_1ToJeanSea() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT. JeanSea's cape is a 'dress' object JeanCN has no geometry for at all, so JeanCN's
        // body graph is emitted twice -- once carrying JeanSea's body index (7662) and once her
        // dress index (52542), both under JeanSea's own ib hash.
        //
        // 'head' is listed even though it maps to itself: objSplits is all-or-nothing, and an
        // object left out of it is dropped from the remap entirely.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // The dress copy inherited JeanCN's body 'ib', which is not its own. The pure-Python row
        // writes "null" over it and so does this.
        config.objNewRegVals = {{"dress", {{IniKeywords::Ib, "null"}}}};

        // The body's lightmap, shaded so JeanSea's cape does not read as a hard edge against the
        // body underneath it. Deliberately NOT applied to the dress copy: the pure-Python original
        // declares this edit against the mod's own 'body' and its RegTexEdit fires per NEW object,
        // so the dress -- which is not an object JeanCN has an edit for -- keeps the untouched
        // lightmap. The integration golden pins that asymmetry
        // (Testing/Integration Tester/.../expected_fullFix_someFix/multiFix/select/Jean/), where
        // the body section points at ...ShadeLightMapJeanSeaRemapTex0 and the dress section at the
        // original ...BodyLightMap.
        config.texEdits = {{"body", "ps-t1", "ShadeLightMap", &JeanShading::liftLowAlpha}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory JeanCNFixer::v6_1ToJean() {
        return IniFixBuilderFuncs::jeanCN6_1ToJean();
    }


    IniFixBuilder::Factory JeanCNFixer::v6_1ToJeanSea() {
        return IniFixBuilderFuncs::jeanCN6_1ToJeanSea();
    }
}
