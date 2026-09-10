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

#include "AGRemapCore/data/IniFixData/HuTao/HuTaoFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {
    namespace {
        const int NormalMapSize = 1024;
        const Colour NormalMapBlue(128, 128, 255);

        // 1, not 0 -- see XianglingFixer's note on the difference.
        const int NearlyTransparent = 1;


        void makeHeadTransparent(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, NearlyTransparent);
        }

    }


    IniFixBuilder::Factory IniFixBuilderFuncs::hutao6_1() {
        // Remapped onto CherryHuTao -- a SPLIT of BOTH objects, two becoming four, which is the
        // widest split here.
        //
        // HuTao draws head and body. CherryHuTao draws head, body, dress and extra: her Lantern Rite
        // outfit adds a skirt and a pair of glasses that HuTao has no geometry for at all. So each of
        // HuTao's graphs is emitted twice, and the copies that have nothing to show are bound to
        // null rather than left drawing a duplicate.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};
        config.objSplits = {{"head", {"head", "extra"}}, {"body", {"body", "dress"}}};

        // ---- what each target ends up binding ----
        //
        // All target-keyed, because a split gives every target exactly one source -- there is no
        // ambiguity here of the kind CherryHuTaoFixer has to resolve with srcObjRegRemaps.
        //
        // 'extra' is stripped bare: it is the glasses, which HuTao does not have, so it keeps its
        // index range (to stop the geometry drawing) and nothing else.
        config.objRegRemovals = {{"head", {"ps-t2"}},
                                 {"extra", {"ps-t0", "ps-t1", "ps-t2"}},
                                 {"body", {"ps-t2", "ps-t3"}},
                                 {"dress", {"ps-t2", "ps-t3"}}};

        // The head's diffuse is DUPLICATED into ps-t2 rather than moved, and then ps-t0 is nulled
        // below -- which is not the same as moving it, because the null has to be there for the
        // shader to read the slot as empty rather than as whatever was left in it.
        //
        // The dress is a straight shift up: its diffuse keeps a copy on ps-t0 so the texAdd below
        // has a bound slot to overwrite.
        config.objRegRemaps = {{"head", {{"ps-t0", {"ps-t0", "ps-t2"}}}},
                               {"dress", {{"ps-t0", {"ps-t0", "ps-t1"}}, {"ps-t1", {"ps-t2"}}}}};

        // Declared against ps-t0, which is where the diffuse sits when the collectors run -- they
        // go before the register edits, so this names the register BEFORE the duplication, not the
        // ps-t2 it ends up on.
        //
        // THE FACE DIFFUSE IS DELIBERATELY NOT EDITED. It was, briefly, on the strength of an
        // in-game look -- and the thing being looked at was 3dmigoto serving a cached copy of the
        // texture from an earlier run, not this fix's output. A screenshot is evidence about the
        // GAME's state, and the game's state includes its caches.
        config.texEdits = {{"head", "ps-t0", "TransparentHeadDiffuse", &makeHeadTransparent}};

        // ...and the normal map the dress copy needs, which nothing in a HuTao mod carries.
        config.texAdds = {{"dress", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapBlue)}};

        // The two copies that must not draw, plus the head's now-vacant diffuse slot.
        config.objNewRegVals = {{"head", {{"ps-t0", "null"}}},
                                {"dress", {{IniKeywords::Ib, "null"}}},
                                {"extra", {{IniKeywords::Ib, "null"}}}};

        // ---- which library each object re-issues ----
        //
        // NNFix on the body and ORFix on the dress, which is the maintainer's call from seeing it
        // in game rather than anything derivable from the pure-Python row -- that row re-issues
        // neither. The split is the usual one: ORFix is the normal-map library, and the DRESS is
        // the object this fix invents a normal map for (see texAdds below), while the body has
        // none and takes the general case.
        //
        // head and extra stay as they were. 'extra' is bound to nothing at all, so a fix call
        // would have nothing to run against -- an empty list is how "no call" is asked for, since
        // the default is NNFix.
        //
        // TexFx is re-issued on the two objects that carry a diffuse, naming ps-t1 as its home
        // (TN.1). It is opt-in: the sub-command is added only where the mod bound ps-t69 or
        // ps-t70, so a mod that never heard of TexFx is untouched.
        config.objFixCalls = {{"head", {IniKeywords::TexFxTransparency1}},
                              {"body", {IniKeywords::NNFixPath}},
                              {"dress", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}},
                              {"extra", {}}};

        // The pure-Python row's IbRemapData / IbDrawIndexedRename / IbTempToDrawIndexed plus its
        // postModel drawindexed removal are together what this flag does.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory HuTaoFixer::v6_1() {
        return IniFixBuilderFuncs::hutao6_1();
    }
}
