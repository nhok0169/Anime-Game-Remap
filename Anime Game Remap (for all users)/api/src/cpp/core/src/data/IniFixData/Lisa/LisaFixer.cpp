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
        // The flat normal map a 4.0-era row invents, matching that row's
        // TexCreator(1024, 1024, colour = Colours.NormalMapYellow) exactly.
        const int NormalMapSize4_0 = 1024;
        const Colour NormalMapYellow4_0(128, 128, 0);
    }


    namespace {
        // The flat normal map LisaStudent is given, Lisa having none to bring. 1024x1024
        // is the size every RegTexAdd in the pure-Python tables uses.
        const int NormalMapSize = 1024;

        // Colours.NormalMapPurple1, which is what lisa5_4's own RegTexAdd uses -- the muted
        // purple a Sumeru-era normal map is mostly made of, NOT the flat (128, 128, 255) blue.
        // Spelled out here rather than shared because core has no Colours constant class; Ayaka
        // declares the same value the same way.
        const Colour NormalMapPurple1(128, 98, 128);
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::lisa5_7() {
        // THE 5.7 FIX for Lisa -> LisaStudent, verified only against the old script at
        // --version 5.7 --fromVersion 5.7 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // AND NOTHING ELSE. By 5.7 the register shift, the invented normal map and the TexFx
        // re-issue are all gone -- lisa5_7's body is three removals and the merge. A row can
        // get SIMPLER with the version, which is why none of these is safe to interpolate.
        config.objRegRemovals = {{"head", {"ps-t2"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t2"}}};

        // ---- the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;

        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::lisa5_4() {
        // THE 5.4 FIX for Lisa -> LisaStudent, verified only against the old script at
        // --version 5.4 --fromVersion 5.4 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE, same shape as lisa4_0: TARGET <- sources, so her body AND dress both land
        // on LisaStudent's body.
        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        config.objRegRemovals = {{"head", {"ps-t2"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t2"}}};

        config.objRegRemaps = {{"head", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true}}},
                               {"body", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t2", {{"ps-t3"}}, true}}}};

        // PURPLE from 5.4, where lisa4_0 invents a yellow one.
        config.texAdds = {{"head", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)},
                          {"body", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)}};

        // ---- the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        config.objFixCalls = {{"head", {IniKeywords::TexFxTransparency1}},
                              {"body", {IniKeywords::TexFxTransparency1}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::lisa4_0() {
        // THE 4.0 FIX for Lisa -> LisaStudent, verified only against the old script at
        // --version 4.0 --fromVersion 4.0 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE onto LisaStudent, and the direction matters: lisa4_0's map is
        // {"head": ["head"], "body": ["body", "dress"]}, which in the pure-Python MERGE notation
        // reads TARGET <- sources -- her body AND her dress both land on LisaStudent's body.
        // objSplits here is keyed the other way, by SOURCE, so the same thing is written out as
        // three entries. Reading the map as source-keyed inverts the remap into a split.
        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        config.objRegRemovals = {{"head", {"ps-t2"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t2"}}};

        config.objRegRemaps = {{"head", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true}}},
                               {"body", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t2", {{"ps-t3"}}, true}}}};

        // TWO ENTRIES, ONE FILE, and that is deliberate. The old script writes
        // LisaStudentHeadNormMapRemapTex.dds AND LisaStudentBodyNormMapRemapTex.dds; this writes a
        // single LisaStudentNormMapRemapTex.dds that both objects bind. They are the same 1024x1024
        // flat yellow, so TexCreate::getFixResourceName memoises the name per mod and the identical
        // copies collapse into one -- the de-duplication added 2026-09-12, when four $swapvar
        // branches were producing four identical 4MB files.
        //
        // So an A/B against the old script reports this as one only-old file per extra object and
        // one only-new, with nothing DIFFERING. Same for nilouBreeze4_8 (3 -> 1) and kiraraBoots4_8
        // (1 -> 1, name only). A row with a single texAdd, like ganyu4_0, shows no divergence at
        // all, which is what makes this easy to misread as a missing texture.
        config.texAdds = {{"head", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize4_0, NormalMapSize4_0, NormalMapYellow4_0)},
                          {"body", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize4_0, NormalMapSize4_0, NormalMapYellow4_0)}};

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;
        //   ^ no ORFix/NNFix entry in its removal set, so the mod's own survive.
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // The external libraries this row re-issues, keyed by TARGET. An empty list means
        // none, and REPLACES the template's default NNFix.
        config.objFixCalls = {{"head", {IniKeywords::TexFxTransparency1Pre5_0}},
                              {"body", {IniKeywords::TexFxTransparency1Pre5_0}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
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
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)},
                          {"body", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)}};

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
