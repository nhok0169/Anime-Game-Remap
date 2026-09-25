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

#include "AGRemapCore/data/IniFixData/NeuvilletteMelusent/NeuvilletteMelusentFixer.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"


namespace AGRemapCore {
    namespace {
        GIMIMergeFixerConfig neuvilletteConfig() {
            // NeuvilletteMelusent -> Neuvillette: the fifth remap of a skin of SEVERAL components onto a target of
            // one, and the inverse of the Neuvillette -> NeuvilletteMelusent rows. Every value here was read off the
            // prototype (Tools/Misc/Prototypes/neuvilletteFromMelusentFix.py), which stays the oracle, confirmed in
            // game on the identity mod, the one real mod of the skin and four synthetic ones (a merged master, a
            // texture-only recolour, a mod without its Eye, a 16-bit one).
            //
            // WHERE EACH SLOT LANDS: on the object whose TEXTURES it draws with, so a merged object keeps one set
            // as far as it can -- the Head set (main Head, main Dress, Bang, Eye) onto his head, the Body set (main
            // Body, Coat) onto his body. His dress (his cravat and ruffles) receives nothing, and the whole-ib skip
            // keeps his own hidden. The trailing numbers are the GAME model's index count per slot, off the
            // download folder's index buffers.
            //
            // The main mesh is the UNNAMED component; its hashes are filed under NeuvilletteMelusentMain
            // (Component::modTypeName). Merge order: the biggest first.
            GIMIMergeFixerConfig config{};

            GIMIMergeFixerConfig::Component main{};
            main.name = "";
            main.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentMain);
            main.slots = {{"Head", "0", "head", true, "", 46620, true},
                          {"Body", "46620", "body", true, "", 24405, true},
                          {"Dress", "71025", "head", true, ";Head", 3567, true}};
            main.vertexCount = 22503;

            GIMIMergeFixerConfig::Component coat{};
            coat.name = "Coat";
            coat.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentCoat);
            coat.slots = {{"A", "0", "body", true, ";Body", 26532, true}};
            coat.vertexCount = 8670;

            GIMIMergeFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentBang);
            bang.slots = {{"A", "0", "head", true, ";Head", 9336, true}};
            bang.vertexCount = 2644;

            GIMIMergeFixerConfig::Component eye{};
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentEye);
            eye.slots = {{"A", "0", "head", false, ";Head", 528, true}};
            eye.vertexCount = 168;

            config.components = {std::move(main), std::move(coat), std::move(bang), std::move(eye)};
            config.targetObjs = {"head", "body", "dress"};

            // ONE layout for all three of his objects, the normal-map one under ORFix, although his head and
            // dress draw on a plain shader: his own mods say ORFix serves that shader from the normal-map layout
            // too (one writes its head that way and renders right on his card).
            config.targetLayout = GIMIMergeFixerConfig::TargetLayout::NormalMap;

            // Every carried binding onto the register its resource NAME says.
            config.texRegsByName = true;

            // A mod carrying none of a component (the one real mod ships no Coat) merges it from its downloads.
            config.downloadPrefix = "NeuvilletteMelusent";

            // His Texcoord is 20 bytes a vertex (a second UV set); every one of the skin's components carries 12.
            config.texcoordStride = 20;

            // Neuvillette binds his face diffuse at ps-t1, the GI 6.x layout.
            config.faceReg = "ps-t1";
            config.compressTextures = false;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::neuvilletteMelusentToNeuvillette6_3() {
        return makeGIMIMergeFixer(neuvilletteConfig());
    }


    IniFixBuilder::Factory NeuvilletteMelusentFixer::toNeuvillette6_3() {
        return IniFixBuilderFuncs::neuvilletteMelusentToNeuvillette6_3();
    }
}
