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

#include "AGRemapCore/data/IniFixData/Arlecchino/ArlecchinoFixer.h"

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/ColourReplaceFilter.h"
#include "AGRemapCore/model/textures/Colour.h"
#include "AGRemapCore/model/textures/ColourRange.h"


namespace AGRemapCore {

    namespace {
        // Colours.NormalMapYellow, and the purple band Colours/ColourRanges call NormalMapPurple1 --
        // (128, 0, 128) up to (128, 98, 128).
        const Colour NormalMapYellow(128, 128, 0);
        const Colour NormalMapPurpleMin(128, 0, 128);
        const Colour NormalMapPurple1(128, 98, 128);


        // HER HEAD: only the purple is repainted yellow. arlecchino5_4's
        // ColourReplaceFilter(NormalMapYellow, coloursToReplace = {ColourRanges.NormalMapPurple1}).
        void yellowHeadNormal(TextureFile& texFile) {
            ColourReplaceFilter filter(NormalMapYellow,
                                        ColourOrRangeSet{ColourRange(NormalMapPurpleMin, NormalMapPurple1)});
            filter.transform(texFile);
        }


        // HER BODY: the whole texture goes yellow -- the same filter with NO coloursToReplace, which
        // is the difference between the two and the reason they are separate edits rather than one.
        void yellowBodyNormal(TextureFile& texFile) {
            ColourReplaceFilter filter(NormalMapYellow);
            filter.transform(texFile);
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::arlecchino5_4() {
        // THE 5.4 FIX for Arlecchino -> ArlecchinoBoss, verified only against the old script at
        // --version 5.4 --fromVersion 5.4 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // The WHOLE row: two normal-map edits, one per object, both on ps-t0. No removals, no
        // shifts, no re-issues. arlecchino5_7 is what adds the rest.
        config.texEdits = {{"head", "ps-t0", "YellowHeadNormal", &yellowHeadNormal},
                           {"body", "ps-t0", "YellowBodyNormal", &yellowBodyNormal}};

        // ---- the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;

        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::arlecchino5_7() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // ONE-TO-ONE, and NOT the Raiden shape despite the target being a boss. Raiden's remap keeps
        // the source's hashes and hides the originals; hers REPLACES them -- ib e811d2a1 becomes
        // 480f1267, and every vertex buffer likewise -- while the indices are identical on both sides
        // (0 / 40179 / 74412), so the forward index lookup is a no-op that still has to happen. That
        // is the Mona/MonaCN shape, which is why this goes through the template and Raiden does not.
        //
        // No objSplits: arlecchino5_7 is a GIMIObjRegEditFixer with an empty object list.

        // The reflection-support keys, stripped per object exactly as KaeyaSailwind -> Kaeya strips
        // them: ResourceRef<Obj>Diffuse, ResourceRef<Obj>LightMap and $CharacterIB name the SOURCE's
        // slots. Unlike that row, this one covers every object it draws -- there is no fourth copy
        // to leave out.
        config.objRegRemovals = {{"head", {"ResourceRefHeadDiffuse", "ResourceRefHeadLightMap",
                                           "$CharacterIB"}},
                                 {"body", {"ResourceRefBodyDiffuse", "ResourceRefBodyLightMap",
                                           "$CharacterIB"}},
                                 {"dress", {"ResourceRefDressDiffuse", "ResourceRefDressLightMap",
                                            "$CharacterIB"}}};

        // HER NORMAL MAPS GO YELLOW, and the two objects go yellow DIFFERENTLY -- the head keeps
        // everything that is not purple, the body is repainted wholesale. Both sit on ps-t0 and both
        // come from arlecchino5_4's RegTexEdit, which 5_7 carries forward unchanged. The dress has no
        // entry in either row and keeps its own normal map.
        config.texEdits = {{"head", "ps-t0", "YellowHeadNormal", &yellowHeadNormal},
                           {"body", "ps-t0", "YellowBodyNormal", &yellowBodyNormal}};

        // ORFIX ON THE TWO OBJECTS THAT CARRY A NORMAL MAP, NNFix on the one that does not.
        // ORFix is the normal-map library, and head and body are exactly the two the texEdits
        // above repaint -- the dress has no entry there and keeps the default. The pure-Python
        // arlecchino5_7 re-issues neither (it carries no NNFix/ORFix machinery at all), so this
        // is a deliberate divergence from that row, confirmed in game by the maintainer.
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath}},
                              {"body", {IniKeywords::ORFixPath}}};

        // The Ib* entries on all three objects plus the postModel drawindexed removal.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory ArlecchinoFixer::v5_7() {
        return IniFixBuilderFuncs::arlecchino5_7();
    }
}
