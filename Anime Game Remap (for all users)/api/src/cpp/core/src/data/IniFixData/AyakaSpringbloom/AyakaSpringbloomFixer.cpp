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

#include "AGRemapCore/data/IniFixData/AyakaSpringbloom/AyakaSpringbloomFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/data/IniFixData/RegValChecks.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/ColourReplaceFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/TransparencyAdjustFilter.h"
#include "AGRemapCore/model/textures/Colour.h"
#include "AGRemapCore/model/textures/ColourRange.h"


namespace AGRemapCore {

    namespace {
        // The green a lightmap uses to mark an ordinary, non-emissive surface.
        const Colour LightMapGreen(0, 128, 0, 255);

        // 1, not 0 -- see XianglingFixer's note.
        const int NearlyTransparent = 1;

        // Fully opaque, as an ADJUSTMENT (clamped) rather than a set, which is what
        // TransparencyAdjustFilter takes.
        const int Opaque = 255;


        // The band the green lightmap occupies, Colours.LightMapGreenMin..Max in the pure-Python
        // table. Alpha spans the whole range here, unlike CherryHuTao's emissive bands.
        ColourOrRangeSet greenBand() {
            return ColourOrRangeSet{ColourRange(Colour(0, 125, 0, 0), Colour(50, 160, 50, 255))};
        }


        /**
         * Ayaka's head shade lightmap, in two passes.
         *
         * The skin marks its shaded areas with a green that Ayaka's own shader reads differently, so
         * the fully-opaque band is flattened to a near-zero alpha first and the partly-transparent
         * remainder (including a black band) is then pulled back to the ordinary lightmap green.
         * Both passes come straight from the pure-Python row; the bands are hers, not derived.
         */
        void editHeadShadeLightMap(TextureFile& texFile) {
            ColourReplaceFilter opaqueBand(
                Colour(0, 128, 0, 1),
                ColourOrRangeSet{ColourRange(Colour(0, 125, 0, 255), Colour(50, 160, 50, 255))});
            opaqueBand.transform(texFile);

            ColourReplaceFilter partialBands(
                LightMapGreen,
                ColourOrRangeSet{ColourRange(Colour(0, 125, 0, 100), Colour(50, 160, 50, 254)),
                                 ColourRange(Colour(0, 0, 0, 100), Colour(0, 0, 0, 200))});
            partialBands.transform(texFile);
        }


        void makeBodyDiffuseTransparent(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, NearlyTransparent);
        }


        void makeGreenOpaque(TextureFile& texFile) {
            TransparencyAdjustFilter filter(Opaque, greenBand());
            filter.transform(texFile);
        }


        std::vector<GIMICharFixerConfig::RegRef> reflectionKeys(const std::string& obj) {
            return {"ResourceRef" + obj + "Diffuse", "ResourceRef" + obj + "LightMap", "$CharacterIB"};
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::ayakaSpringbloom6_1() {
        // Remapped onto Ayaka -- a MERGE, and the only one in this batch. Two of the skin's objects
        // land on one of Ayaka's, so the targets collide and the fix writes TWO .ini files which the
        // importer overlaps. See makeGIMICharFixer and CreatingRemaps' "The merge".
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // ---- the transpose, and it SWAPS head and body ----
        //
        // The pure-Python row is written {target: [sources]}:
        //
        //     {"head": ["body", "body"], "body": ["head", "body"], "dress": ["dress", "dress"]}
        //
        // read as "Ayaka's head is drawn from the skin's body in BOTH groups; her body from the
        // skin's head in group 0 and from its body in group 1; her dress from the skin's dress in
        // both". objSplits is the same fact the other way round, {source: [targets]}.
        //
        // ORDER IS CLAIM ORDER for a shared target, so 'head' is listed before 'body': it is what
        // puts the skin's head into Ayaka's BODY in the file a reader opens, and the body-sourced
        // copy in RemapFix1. Backwards still works and just hides the main object in a generated
        // file -- see CreatingRemaps.
        config.objSplits = {{"head", {"body"}},
                            {"body", {"head", "head", "body"}},
                            {"dress", {"dress", "dress"}}};

        // Says why the second file exists, for whoever opens it.
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // ---- what each target stops binding ----
        std::vector<GIMICharFixerConfig::RegRef> headRemovals = reflectionKeys("Head");
        headRemovals.push_back("ps-t3");

        std::vector<GIMICharFixerConfig::RegRef> dressRemovals = reflectionKeys("Dress");
        dressRemovals.push_back("ps-t3");

        config.objRegRemovals = {{"head", headRemovals},
                                 {"body", reflectionKeys("Body")},
                                 {"dress", dressRemovals}};

        // Ayaka's head index would otherwise draw the skin's head mesh a second time, on top of the
        // copy her body draw call already puts there.
        config.objNewRegVals = {{"head", {{IniKeywords::Ib, "null"}}}};

        // ---- six texture edits, every one of them CONDITIONAL ----
        //
        // Each comes in a pair -- one for the register layout the mod is in, one "Alt" for the other
        // -- and the check on the register's VALUE is what picks between them. That is the
        // already-hand-fixed case RegValChecks exists for: a mod whose author swapped these
        // registers themselves has its shade lightmap a slot away from where this row expects it,
        // and editing whatever happens to be sitting there instead would wreck it.
        //
        // TRANSCRIBED AS WRITTEN, including the two that look wrong: "BodyTransparentDiffuse" and
        // "BodyAltTransparentDiffuse" are gated on isLightMap, not isDiffuse. That is what the
        // pure-Python row says, and porting it faithfully is the rule -- if it is a mistake it is
        // one this fix has always had, and the A/B against the old script is what would show it.
        // EVERY ONE IS PINNED TO ITS SOURCE OBJECT, and on this character that is the difference
        // between working and not.
        //
        // The edit NAMES come from the pure-Python PARSER row, where they are declared against the
        // object they belong to BEFORE any merge: "HeadShadeLightMap" is the skin's head. But
        // TexEdit::obj is the TARGET object, and this merge SWAPS the two -- Ayaka's head is drawn
        // from the skin's body. Keyed by target alone, every edit here lands on the wrong texture:
        // the head's shade lightmap edit was applied to the BODY's shadow map, and the neck (which
        // is what that lightmap shades) rendered pale and flat in game.
        //
        // So obj says where the copy ENDS UP and srcObj says whose texture it IS. The skin's head
        // goes to Ayaka's body; its body goes to her head and her body, which is why the
        // body-sourced edits need an entry for each.
        const std::string Head = "head";
        const std::string Body = "body";

        config.texEdits = {
            // the skin's HEAD -> Ayaka's body
            {Body, "ps-t2", "HeadShadeLightMap", &editHeadShadeLightMap, true, Head, "", &RegValChecks::isShadow},
            {Body, "ps-t1", "HeadAltShadeLightMap", &editHeadShadeLightMap, true, Head, "", &RegValChecks::isShadow},

            // the skin's BODY -> Ayaka's head AND her body
            {Head, "ps-t1", "BodyTransparentDiffuse", &makeBodyDiffuseTransparent, true, Body, "", &RegValChecks::isLightMap},
            {Body, "ps-t1", "BodyTransparentDiffuse", &makeBodyDiffuseTransparent, true, Body, "", &RegValChecks::isLightMap},
            {Head, "ps-t0", "BodyAltTransparentDiffuse", &makeBodyDiffuseTransparent, true, Body, "", &RegValChecks::isLightMap},
            {Body, "ps-t0", "BodyAltTransparentDiffuse", &makeBodyDiffuseTransparent, true, Body, "", &RegValChecks::isLightMap},
            {Head, "ps-t2", "BodyOpaqueGreenLightMap", &makeGreenOpaque, true, Body, "", &RegValChecks::isShadow},
            {Body, "ps-t2", "BodyOpaqueGreenLightMap", &makeGreenOpaque, true, Body, "", &RegValChecks::isShadow},
            {Head, "ps-t1", "BodyAltOpaqueGreenLightMap", &makeGreenOpaque, true, Body, "", &RegValChecks::isShadow},
            {Body, "ps-t1", "BodyAltOpaqueGreenLightMap", &makeGreenOpaque, true, Body, "", &RegValChecks::isShadow}};

        // ---- which external library each object re-issues ----
        //
        // ORFix for the head and body, NNFix for the dress -- the dress's is the default, named here
        // only so all three read together.
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath}},
                              {"body", {IniKeywords::ORFixPath}},
                              {"dress", {IniKeywords::NNFixPath}}};

        // The row's IbRemapData / IbDrawIndexedRename / IbTempToDrawIndexed plus its per-file
        // drawindexed removal (iniPostModelRegEditFilters, one entry per .ini file) are together
        // what this flag does.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory AyakaSpringbloomFixer::v6_1() {
        return IniFixBuilderFuncs::ayakaSpringbloom6_1();
    }
}
