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

#include "AGRemapCore/data/IniFixData/BennettAdventure/BennettAdventureFixer.h"

#include <utility>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"


namespace AGRemapCore {
    namespace {
        GIMIMergeFixerConfig bennettConfig() {
            // BennettAdventure -> Bennett: a skin of SEVERAL components onto a target of one, and
            // the inverse of Bennett -> BennettAdventure. See GIMIMergeFixerConfig for what each
            // field does; every value here was read off the prototype
            // (Tools/Misc/Prototypes/adventureToBennettFix.py) and her identity mod.
            //
            // Which source slot lands on which of Bennett's two objects:
            //
            //   Body A  14778 tris  slot A, her main body               -> body   (9879)
            //   Body B  13073 tris  slot B, the rest of her body        -> body   (9879)
            //   Bang     3128 tris  her front fringe                    -> head   (0)
            //   Eye       276 tris  her eyes                            -> head   (0)
            //
            // TWO of his objects, where Yelan has four, and BOTH of them are merged onto -- his body
            // takes her Body's two slots and his head takes her Bang and her Eye. The merge turns
            // each into one SECTION rather than a second .ini file.
            //
            // It is not one draw call, though. A target object several slots merge onto has an index
            // buffer that is member after member, and a mod's own `drawindexed` lines address its
            // own buffer -- covering the first member and stopping where the second begins. The fix
            // appends a draw for every member after the first; see GIMIMergeFixer's extra-draw loop.
            GIMIMergeFixerConfig config{};

            // The trailing number is the GAME model's index count for the slot, read off the
            // download .ib files (R32, so bytes / 4): 177336, 156876, 37536 and 3312. It is the
            // fallback for a slot whose ib has to be downloaded, and is only ever read for a target
            // object several slots merge onto -- which here is BOTH of his.
            GIMIMergeFixerConfig::Component body{};
            body.name = "Body";
            body.slots = {{"A", "0", "body", true, "", 44334},
                          {"B", "44334", "body", true, "", 39219}};

            // The GAME model's vertex count, for a mod that does not carry the component at all --
            // counted off the download Blend.buf files (bytes / 32), the same three numbers the
            // parse row gives its downloads. This is not hypothetical: an NSFW body edit measured on
            // 2026-09-15 has no Eye sections whatsoever, because it has no reason to touch her eyes.
            body.vertexCount = 26057;

            // No textures of their own: her identity mod's Bang and Eye sections both bind Body slot
            // A's diffuse and lightmap, so a mod may leave these with an ib and nothing else. The
            // donor's textures are DOWNLOADED rather than read out of the mod, which matters more
            // here than it looks: a downloaded component carries the GAME's UVs, and those index the
            // GAME's atlas, not the author's. Binding the mod's texture to downloaded geometry
            // samples the wrong atlas -- her eyes rendered as two patches of cheek (2026-09-15).
            GIMIMergeFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.slots = {{"A", "0", "head", true, "Body;A", 9384}};
            bang.vertexCount = 2492;

            GIMIMergeFixerConfig::Component eye{};
            eye.name = "Eye";
            eye.slots = {{"A", "0", "head", false, "Body;A", 828}};
            eye.vertexCount = 202;

            config.components = {std::move(body), std::move(bang), std::move(eye)};
            config.targetObjs = {"head", "body"};

            // The same prefix the parse row downloads under. A mod may carry none of a component --
            // an NSFW body edit has no Eye sections at all -- and the merge then reads that
            // component out of its downloads. See GIMIMergeFixerConfig::downloadPrefix.
            config.downloadPrefix = "BennettAdventure";

            // Bennett binds his face diffuse at ps-t1 -- his identity mod's
            // TextureOverrideBennettFaceHeadDiffuse (hash 50f7dc9a) does, which is the GI 6.x
            // layout. A MOD may still write the pre-6.x ps-t0.
            config.faceReg = "ps-t1";

            // NO band moves, on purpose. Her pale ramp sits at 176-178 and his hair band at 0, which
            // is the move the FORWARD direction makes -- but backwards it cannot be made safely:
            // band 0 is the DEFAULT a lazy or ported mod leaves everything on, and his own band 0 is
            // his hair-and-dark-cloth band, so a move onto it would have to be gated on a diffuse
            // test that has not been measured for this pair. His skin and hers are both already at
            // 255. Add a row here only with a measurement behind it: a wrong move is not a no-op.
            config.lightMapEdit = {};

            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::bennettAdventureToBennett6_1() {
        return makeGIMIMergeFixer(bennettConfig());
    }


    IniFixBuilder::Factory BennettAdventureFixer::toBennett6_1() {
        return IniFixBuilderFuncs::bennettAdventureToBennett6_1();
    }
}
