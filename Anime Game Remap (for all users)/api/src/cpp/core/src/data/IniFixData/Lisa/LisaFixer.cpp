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

#include "AGRemapCore/data/IniFixData/Lisa/LisaFixer.h"

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {

    namespace {
        // The flat normal map LisaStudent is given, Lisa having none to bring. 1024x1024
        // is the size every RegTexAdd in the pure-Python tables uses.
        const int NormalMapSize = 1024;

        // The maintainer's colour for this pair, given directly rather than taken from a
        // named constant: it is ONE off Colours.NormalMapPurple1 (128, 98, 128), which
        // lisa5_4 uses, and that near-miss is deliberate enough to be worth spelling out
        // rather than rounding to the constant.
        const Colour NormalMapPurple(128, 96, 128, 255);
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::lisa6_1ToLisaStudent() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE. LisaStudent draws head and body only, so Lisa's dress has nowhere of its
        // own to go and is drawn through LisaStudent's BODY. Same shape as
        // KleeBlossomingStarlight -> Klee, with the characters the other way round.
        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}};

        // The generated second file explains itself -- see IniComments::GIMIObjMergerPreamble,
        // which is the paragraph the pure-Python merge wrote for exactly this.
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // Three registers LisaStudent has no use for, one per object and each a DIFFERENT slot --
        // lisa6_1's RegRemove(remove = {"head": {"ps-t2"}, "body": {"ps-t3"}, "dress": {"ps-t2"}}).
        // The asymmetry is real rather than a transcription slip; her body is the odd one out.
        //
        // head and body are TARGETS of this merge and go in objRegRemovals. The DRESS IS NOT:
        // it is a source that lands on LisaStudent's body, so a target-keyed entry named
        // "dress" builds an edit for a mod object that never appears and removes nothing.
        // Source-keyed is the merge's half of the same field -- see srcObjRegRemovals.
        config.objRegRemovals = {{"head", {"ps-t2"}},
                                 {"body", {"ps-t3"}}};

        config.srcObjRegRemovals = {{"dress", {"ps-t2"}}};

        // ---- SHE IS GAINING A NORMAL MAP, WHICH LISA DOES NOT HAVE ----
        //
        // The exact mirror of lisaStudent6_1ToLisa, which drops one. Lisa reads ps-t0
        // diffuse, ps-t1 lightmap; LisaStudent reads ps-t0 normal map, ps-t1 diffuse,
        // ps-t2 lightmap. So everything shifts UP a slot and the vacated ps-t0 is filled
        // with an invented normal map.
        //
        // TWO TARGETS ON ps-t0 is what makes room for it: the diffuse lands on ps-t1 where
        // LisaStudent reads it AND stays on ps-t0, where the texAdd below overwrites it.
        // Writing only {"ps-t0", {"ps-t1"}} leaves ps-t0 unbound in any part the texAdd
        // does not reach. Ganyu -> GanyuTwilight is the other character that does this.
        //
        // Both renames of an object in ONE entry so they apply in a single pass -- ps-t0 ->
        // ps-t1 must not then be re-read as the input to ps-t1 -> ps-t2.
        //
        // The body carries one more than the head: its ps-t2 shadow ramp shifts to ps-t3,
        // the slot its metal map just vacated above. The head's shadow ramp was the ps-t2
        // removed above, so it has nothing there to move.
        //
        // Transcribed from lisa5_4/lisa4_0, NOT from lisa6_1 -- the 6.1 row carries only the
        // removals and the NNFix re-issue, which is the gap this closes.
        config.objRegRemaps = {{"head", {{"ps-t0", {"ps-t0", "ps-t1"}}, {"ps-t1", {"ps-t2"}}}},
                               {"body", {{"ps-t0", {"ps-t0", "ps-t1"}}, {"ps-t1", {"ps-t2"}},
                                          {"ps-t2", {"ps-t3"}}}}};

        // ...and the normal map that fills the slot the shift vacated, one per target.
        // Declared against ps-t0 because that is where it sits when the collectors run:
        // they go before the register edits, so this names the register BEFORE the shift.
        //
        // BOTH ENTRIES LAND ON ONE FILE, deliberately. A created texture is named
        // <target><name>RemapTex with no mod object in it, so two texAdds sharing a name share a
        // .dds -- here LisaStudentNormalMapRemapTex.dds, written once instead of twice for 4MB of
        // identical flat colour. That is only right because the two ARE identical: giving the head
        // and the body different colours would need different names, or the second would silently
        // overwrite the first.
        config.texAdds = {{"head", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple)},
                          {"body", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple)}};

        // ORFix rather than the default NNFix, because the TARGET now has a normal map --
        // ORFix is the normal-map one. Same single line that separates ganyu6_1 from
        // ganyuTwilight6_1, and the reason lisaStudent6_1ToLisa keeps the default.
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath}},
                              {"body", {IniKeywords::ORFixPath}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory LisaFixer::v6_1ToLisaStudent() {
        return IniFixBuilderFuncs::lisa6_1ToLisaStudent();
    }
}
