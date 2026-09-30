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

#include "AGRemapCore/data/IniFixData/Lumine/LumineFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {
    namespace {
        // The skin's main-mesh slots -- match_first_index off the frame dump of its Dressing Room preview
        const std::string HeadSlot = "0";
        const std::string BodySlot = "57141";

        // ---- her HEAD on the skin's head shader (2026-09-29) ----
        //
        // The skin's head slot is a normal-map shader that reads the diffuse ALPHA, which hers does not: her head
        // diffuse is alpha 255 all over and the skin's ~0, and left at 255 her hair came out a glowing ORANGE on the
        // skin (every variant without this edit, in game). Alpha 1, Yelan's and Yaoyao's value.
        //
        // NOT moved, unlike Yaoyao's: the light map band. Her head atlas is all band 255 where the skin keeps its hair
        // on 126-128, but moved to 127 her hair came out a saturated gold beside her own outfit's pale cream; left on
        // 255 it matched (one variable at a time, on her identity mod in the Dressing Room preview).
        const int HeadDiffuseAlpha = 1;


        void alphaOne(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, HeadDiffuseAlpha);
        }


        GIMIComponentFixerConfig lumineHeavenConfig() {
            // Remapped onto LumineHeaven ("As Heaven and Earth Are Made Anew", 6.3) -- a skin of THREE components, the
            // YaoyaoBamboo shape. Every value was read off the prototype (Tools/Misc/Prototypes/lumineHeavenFix.py),
            // confirmed in game on her identity mod and all ten of her mods, and off the two frame dumps
            // (Tools/Misc/Diagnostics/giDrawTable.py).
            GIMIComponentFixerConfig config{};
            config.targetSkin = ModTypeIdTools::getName(ModTypeId::LumineHeaven);
            config.drawnObjs = {"head", "body", "dress"};

            // PLAIN, always: she has no normal map on any object, and every one of her mods binds diffuse / light
            // map at ps-t0 / ps-t1 -- while two of them (Lumine4, Lumine6's variants) are the older GIMI shape with a
            // MetalMap / ShadowRamp at ps-t2 / ps-t3 and no fix call. Detect read that ps-t2 as a normal-map layout and
            // put the kimono's light map in the diffuse slot: vivid green (2026-09-29). Bennett's is Plain for the same
            // reason (SourceLayout's own warning).
            config.sourceLayout = GIMIComponentFixerConfig::SourceLayout::Plain;

            // ...and each binding read by its resource NAME, where the names can be believed.
            config.texRegsByName = true;

            // Both characters read the face diffuse at ps-t1 (GI 6.x). Only a copy of the face would be swapped, and
            // there is none -- see Component::face below.
            config.faceSwapOnlyFromDiffuseReg = true;

            // A mod toggling variants of one object on an if / else if chain with no else draws EVERY variant at once
            // under the template's drawindexed = auto (Yaoyao2's lesson).
            config.fillDrawOnlyWhenUndrawn = true;

            // The main mesh. Its component name is EMPTY -- its files are LumineHeavenHead.ib,
            // LumineHeavenPosition.buf -- and only its fix-target id carries a name. Her head (hair, the face skin it
            // holds, her flower) through the Head slot; her body AND her dress through the Body slot -- the skin has
            // no dress slot, and both are the body's cloth and skin.
            GIMIComponentFixerConfig::Component main{};
            main.name = "";
            main.modTypeName = ModTypeIdTools::getName(ModTypeId::LumineHeavenMain);
            main.slot = "Head";
            main.slotIndex = HeadSlot;
            main.objSlotIndices = {{"body", BodySlot}, {"dress", BodySlot}};
            main.slotIndices = {HeadSlot, BodySlot};
            main.negativeIndex = false;
            main.normalMap = true;
            main.texcoordStride = 12;       // LumineHeavenTexcoord.buf: 374148 / 31179
            main.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // NO component carries the face. The skin draws its OWN face meshes (15825079 / 82d9b411; hers are
            // 3049e662 / 92af2d49), and her face diffuse carried onto the skin's face hash lands on a different mesh:
            // the two atlases share a layout but not the eyes -- hers paints open eye-whites and lash lines for her
            // mesh, the skin's closed lid-lines for its -- and in game her face on the skin lost its lashes and washed
            // out, while the skin's own face with her eyes (the Eye component) looked like her (2026-09-29). What it
            // costs: a mod repainting her face keeps the skin's face on the skin.
            main.face = false;

            // Her DRESS object is single-layer cloth -- her own long back drape, and every mod's coat, cape or skirt
            // tails built on it -- and the skin's Body shader lights its back faces as rim light: Lumine2's coat lining
            // came out bright blue, the drapes of Lumine4 / 7 dark, where on her own outfit they are pale. A mirrored
            // inner layer gives the inside a front face (Neuvillette's lesson): the lining came back pale.
            main.mirroredObjs = {"dress"};

            // The Bang: one slot, on the normal-map layout with the Head's textures. Her two front bangs and her
            // flower land here.
            GIMIComponentFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.modTypeName = ModTypeIdTools::getName(ModTypeId::LumineHeavenBang);
            bang.slot = "A";
            bang.slotIndex = "0";
            bang.slotIndices = {"0"};
            bang.negativeIndex = false;
            bang.normalMap = true;
            bang.face = false;
            bang.texcoordStride = 12;       // LumineHeavenBangTexcoord.buf: 32880 / 2740
            bang.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // Her eyes (vertex groups 5 / 6) are in her HEAD object. The skin draws its Eye on the PLAIN shader
            // 95aa6cdb with a diffuse / light map at ps-t0 / ps-t1 under NNFix. Her eyes and the skin's coincide within
            // 0.15 mm (vertex-group centroids): no position offset, unlike Neuvillette's.
            GIMIComponentFixerConfig::Component eye = bang;
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::LumineHeavenEye);
            eye.normalMap = false;
            eye.texcoordStride = 12;        // LumineHeavenEyeTexcoord.buf: 2952 / 246
            eye.slotRegisters = {"ps-t0", "ps-t1"};
            eye.offsetOnlyWithGameFace = true;

            // The Eye LAST: it owns the hidden components and the TexFx guards, so it has to be the last fixer to
            // run -- the same order as the rows in IniFixBuilderData.
            config.components = {main, bang, eye};

            // Every component receives a forward vertex-group row, so none is hidden by request; a mod can still put
            // nothing on a component's bones, and the template hides such a component by the result.
            config.hiddenComponents = {};
            config.unremappedSlots = {};

            // The skin draws its OWN face and head-upper meshes: a mod hiding hers by hash (a mask, a custom face)
            // hides the skin's too.
            config.sideMeshes = {"ib_face", "ib_headupper"};

            config.diffuseEdits = {{"head", &alphaOne}};
            config.lightMapEdit = nullptr;
            config.compressTextures = false;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::lumineHeavenMain6_3() {
        return makeGIMIComponentFixer(lumineHeavenConfig(), "");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::lumineHeavenBang6_3() {
        return makeGIMIComponentFixer(lumineHeavenConfig(), "Bang");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::lumineHeavenEye6_3() {
        return makeGIMIComponentFixer(lumineHeavenConfig(), "Eye");
    }


    IniFixBuilder::Factory LumineFixer::main6_3() {
        return IniFixBuilderFuncs::lumineHeavenMain6_3();
    }


    IniFixBuilder::Factory LumineFixer::bang6_3() {
        return IniFixBuilderFuncs::lumineHeavenBang6_3();
    }


    IniFixBuilder::Factory LumineFixer::eye6_3() {
        return IniFixBuilderFuncs::lumineHeavenEye6_3();
    }
}
