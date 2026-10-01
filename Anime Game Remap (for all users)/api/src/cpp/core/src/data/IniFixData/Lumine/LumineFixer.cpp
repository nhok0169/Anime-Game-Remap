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

#include <algorithm>
#include <cstdint>
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


        // ---- a GLOW painted in a COOL colour (2026-09-30) ----
        //
        // Her shader glows a diffuse-alpha-255 pixel in its own colour; the skin's body shader multiplies every glow by a
        // warm gold of its own. Lumine10's blue arm guards, gems and boots came out green / dark red on the skin, TexFx or
        // not (painted white, the glow was white on her and red-orange on the skin). Such a pixel loses its alpha and the
        // skin renders it lit in its own colour: blue again. A dark pixel keeps it (on the skin that alpha keeps black
        // cloth black), and so does a WARM glow (red >= blue), which the tint barely changes -- the skin's own gems glow
        // gold.
        const int CoolGlowBrightness = 60;

        void clearCoolGlow(TextureFile& texFile) {
            std::vector<std::uint8_t> pixels = texFile.getPixels();
            for (std::size_t i = 0; i + 3 < pixels.size(); i += 4) {
                const int red = pixels[i], green = pixels[i + 1], blue = pixels[i + 2];
                if (pixels[i + 3] > 200 && std::max({red, green, blue}) > CoolGlowBrightness && blue > red) {
                    pixels[i + 3] = 0;
                }
            }
            const int width = texFile.getWidth();
            const int height = texFile.getHeight();
            texFile.setPixels(std::move(pixels), width, height);
        }


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

            // ...but not where the mod models its own lining: a twin moved inward from a coat lands in front of the
            // lining a few millimetres behind it, and Lumine2's coat showed flat grey polygons over its flaps (Lumine7's
            // dress brown ones). A triangle with a layer facing the other way within 1 cm behind it gets no twin.
            main.mirrorBackedReach = 0.01f;

            // ...and the layer reads her BACK-face UVs: her dress shader is two-sided and textures a back face through
            // TEXCOORD1 (her own drape lining; Lumine10's starry skirt lining, 1101 back faces into a galaxy quadrant),
            // which with the front UVs showed the outside's black (2026-09-30).
            main.mirrorBackUV = true;

            // ...and her layered clothes lose the outline of their INNER layers (Yaoyao's hair fix, core
            // InnerLayerOutline). The skin's outline shell sits further out than hers, and the shell of a jacket's
            // under-layer came out through the jacket as small dark red squares on the sleeve, the waist and the
            // chest (Lumine1, 2026-09-30); zeroing every outline removed them and her silhouette line with them. The
            // facing-the-axis rule stays on: without it one red triangle stayed at her chest (a sheet facing inward
            // with nothing along its normal), and the inside of her arms looked the same either way.
            main.innerOutlineObjs = {"body", "dress"};

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

            config.diffuseEdits = {{"head", &alphaOne}, {"body", &clearCoolGlow}, {"dress", &clearCoolGlow}};
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
