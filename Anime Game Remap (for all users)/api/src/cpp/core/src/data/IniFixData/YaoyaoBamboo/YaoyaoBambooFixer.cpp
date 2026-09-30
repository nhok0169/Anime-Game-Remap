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

#include "AGRemapCore/data/IniFixData/YaoyaoBamboo/YaoyaoBambooFixer.h"

#include <utility>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {
    namespace {
        // Her body diffuse's alpha -- see the body edit below.
        const int BodyDiffuseAlpha = 0;


        void alphaZero(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, BodyDiffuseAlpha);
        }


        GIMIMergeFixerConfig yaoyaoConfig() {
            // YaoyaoBamboo -> Yaoyao: the sixth remap of a skin of SEVERAL components onto a target of one, and the
            // inverse of the Yaoyao -> YaoyaoBamboo rows. Every value here was read off the prototype
            // (Tools/Misc/Prototypes/yaoyaoFromBambooFix.py), which stays the oracle, confirmed in game on the
            // identity mod, the one real mod of the skin (both its toggles) and four synthetic ones (a merged
            // master, a texture-only recolour, a mod without its Eye, a 16-bit one -- yaoyaoBambooSynth.py).
            //
            // WHERE EACH SLOT LANDS: on the object whose TEXTURES it draws with -- the Head set (main Head, Bang,
            // Eye) onto her head, the Body onto her body. The trailing numbers are the GAME model's index count per
            // slot, off the download folder's index buffers. Merge order: the biggest first.
            GIMIMergeFixerConfig config{};

            GIMIMergeFixerConfig::Component main{};
            main.name = "";
            main.modTypeName = ModTypeIdTools::getName(ModTypeId::YaoyaoBambooMain);
            main.slots = {{"Head", "0", "head", true, "", 43092, true},
                          {"Body", "43092", "body", true, "", 61554, true}};
            main.vertexCount = 27836;

            GIMIMergeFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.modTypeName = ModTypeIdTools::getName(ModTypeId::YaoyaoBambooBang);
            bang.slots = {{"A", "0", "head", true, ";Head", 10713, true}};
            bang.vertexCount = 2829;

            GIMIMergeFixerConfig::Component eye{};
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::YaoyaoBambooEye);
            eye.slots = {{"A", "0", "head", false, ";Head", 696, true}};
            eye.vertexCount = 238;

            config.components = {std::move(main), std::move(bang), std::move(eye)};
            config.targetObjs = {"head", "body"};

            // She draws both objects on the PLAIN shader (diffuse ps-t0, light map ps-t1, no normal map -- the
            // 0fa35364 her asset hash.json calls one is a global bound on every draw), under NNFix.
            config.targetLayout = GIMIMergeFixerConfig::TargetLayout::Plain;

            // Every carried binding onto the register its resource NAME says.
            config.texRegsByName = true;

            // A mod carrying none of a component merges it from its downloads (the synthetic mod without an Eye).
            config.downloadPrefix = "YaoyaoBamboo";

            // Both characters' Texcoord is 12 bytes a vertex.
            config.texcoordStride = 12;

            // Both bind the face diffuse at ps-t1 (GI 6.x), on the SAME shared face meshes and the SAME hash
            // (c70ae897): the mod's own face section already fires on her, so it is copied only when its diffuse
            // has to move -- a copy is a second override on one hash, "Possible Mod Conflict" on every reload.
            config.faceReg = "ps-t1";
            config.faceOnlyWhenMoved = true;

            // Her BODY shader reads the diffuse alpha as a glow: hers is ~0, the skin's 255 all over, and the
            // skin's identity mod came out lit up white from the collar down on her (2026-09-27). Settled by one
            // hand edit before any code. Her head's alpha (hers 255, the skin's ~0) changed nothing measurable, so
            // it is left. No band move either: the skin's hair on 126-128 renders as hair on her head shader.
            config.diffuseEdits = {{"body", &alphaZero}};
            config.lightMapEdit = nullptr;
            config.compressTextures = false;

            // Her face meshes are the skin's own, so there are no side meshes to translate.
            config.sideMeshes = {};

            // Every one of her objects is reached, but a mod may still leave a TexFx request pending.
            config.texFxGuardUnreached = true;

            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::yaoyaoBambooToYaoyao6_3() {
        return makeGIMIMergeFixer(yaoyaoConfig());
    }


    IniFixBuilder::Factory YaoyaoBambooFixer::toYaoyao6_3() {
        return IniFixBuilderFuncs::yaoyaoBambooToYaoyao6_3();
    }
}
