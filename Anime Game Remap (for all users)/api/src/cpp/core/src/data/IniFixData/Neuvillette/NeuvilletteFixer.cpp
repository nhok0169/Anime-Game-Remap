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

#include "AGRemapCore/data/IniFixData/Neuvillette/NeuvilletteFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"


namespace AGRemapCore {
    namespace {
        // The skin's main-mesh slots -- match_first_index off the frame dump of its outfit preview
        const std::string HeadSlot = "0";
        const std::string BodySlot = "46620";
        const std::string DressSlot = "71025";


        GIMIComponentFixerConfig neuvilletteMelusentConfig() {
            // Remapped onto NeuvilletteMelusent ("Melusent Gift", 6.3) -- a skin of FOUR components. Every
            // value was read off the prototype (Tools/Misc/Prototypes/neuvilletteMelusentFix.py), confirmed
            // in game on his identity mod and seven real ones, and off the two frame dumps of the outfit
            // shop's previews (Tools/Misc/Diagnostics/giDrawTable.py).
            GIMIComponentFixerConfig config{};
            config.targetSkin = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusent);
            config.drawnObjs = {"head", "body", "dress"};

            // Per object: a section binding ps-t2 is on the normal-map layout (his body); otherwise it is
            // plain (his head, his dress) and shifted up onto the skin's normal-map slots with a flat normal.
            config.sourceLayout = GIMIComponentFixerConfig::SourceLayout::Detect;

            // ...and each binding read by its resource NAME, where the names can be believed: one of his
            // mods writes its dress in the GAME's register order (ps-t0 light map, ps-t1 diffuse, no fix
            // call), which read positionally drew his hair ribbon's tails flat green (2026-09-24).
            config.texRegsByName = true;

            // Both characters read the face diffuse at ps-t1 (GI 6.x). Swap only a mod still on ps-t0.
            config.faceSwapOnlyFromDiffuseReg = true;

            // The main mesh. Its component name is EMPTY -- its files are NeuvilletteMelusentHead.ib,
            // NeuvilletteMelusentPosition.buf -- and only its fix-target id carries a name.
            //
            // Each of his objects goes through the slot that shades the same KIND of part: his head (hair,
            // face skin) through the Head slot, his body through the Body slot, and his DRESS through the
            // skin's Dress slot (ps 26dbacaa). On his own outfit that object is his white cravat, lace and
            // cuff ruffles, which through the Body slot came out clean and through the Dress slot showed dark
            // blotches (2026-09-24) -- but on Neuvillette2 it is a whole long skirt, and the Body slot's
            // shader lights the INSIDE of cloth as if it faced out: her inner skirt came out flat bright blue
            // where her own outfit shades it navy. The Dress slot shades it right, and the maintainer chose
            // it over the cravat's blotches (2026-09-25).
            GIMIComponentFixerConfig::Component main{};
            main.name = "";
            main.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentMain);
            main.slot = "Head";
            main.slotIndex = HeadSlot;
            main.objSlotIndices = {{"body", BodySlot}, {"dress", DressSlot}};
            main.slotIndices = {HeadSlot, BodySlot, DressSlot};
            main.negativeIndex = false;
            main.normalMap = true;
            main.face = true;
            main.texcoordStride = 12;       // NeuvilletteMelusentTexcoord.buf: 270036 / 22503
            main.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // The Coat and the Bang: one slot each, on the normal-map layout like the main mesh.
            GIMIComponentFixerConfig::Component coat{};
            coat.name = "Coat";
            coat.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentCoat);
            coat.slot = "A";
            coat.slotIndex = "0";
            coat.slotIndices = {"0"};
            coat.negativeIndex = false;
            coat.normalMap = true;
            coat.face = false;
            coat.texcoordStride = 12;
            coat.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            GIMIComponentFixerConfig::Component bang = coat;
            bang.name = "Bang";
            bang.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentBang);

            // His eyes (vertex groups 13 / 14) are in his HEAD object. The skin draws its Eye on the PLAIN
            // shader with a diffuse / light map at ps-t0 / ps-t1 under NNFix.
            GIMIComponentFixerConfig::Component eye = coat;
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentEye);
            eye.normalMap = false;
            eye.slotRegisters = {"ps-t0", "ps-t1"};

            // His eye mesh IS the skin's, vertex for vertex, 1.24 cm higher: the eyes sit in the GAME's
            // face mesh, and at his height the irises were behind the skin's upper lids (white eyes, no
            // pupils). Measured as the skin's EyePosition.buf minus this fix's Eye for his identity mod,
            // over all 168 vertices (residual under 0.3 mm).
            eye.positionOffset = {0.0f, -0.01237f, -0.00021f};

            // ...but only into the GAME's face. Neuvillette2 hides it (`handling = skip` on 81e80510) and
            // draws its own inside his head mesh, which reaches the skin unshifted: shifted, its eyes sat
            // below that face and looked down.
            eye.offsetOnlyWithGameFace = true;

            // The Eye LAST: it owns the hidden components and the TexFx guards, so it has to be the last
            // fixer to run -- the same order as the rows in IniFixBuilderData.
            // The Coat draws nothing: his whole outfit is on the main mesh (VGRemapData.cpp says why -- a cut
            // between the two tears), and the template hides a component whose output draws nothing.
            //
            // TexFx does not serve the skin's main-mesh shaders: Neuvillette8's sheer shirt (ps-t69 +
            // CommandList\TexFx\T.0) vanished outright while it was skinned and drawn every frame, and
            // came back opaque with the TexFx lines gone (2026-09-25). Opaque is a texture fault; missing
            // is a geometry one.
            main.dropTexFx = true;

            // ...and its see-through draws blended instead, at the opacity that matched the TexFx original
            // in game (Neuvillette8's sheer shirt, 2026-09-26). The mod's own TexFx mask says which.
            main.texFxBlend = 0.9f;

            // His dress object gets a mirrored inner layer: single-layer cloth, whose back faces the skin's
            // Dress shader lights like rim light (Neuvillette2's inner skirt came out flat light blue,
            // 2026-09-26; a cull test showed it was the inside). See Component::mirroredObjs.
            main.mirroredObjs = {"dress"};

            // His front coat panels (48-51 one side, 52-55 the other; 48 / 52 the top links) SHARE the skin's pelvis and
            // its thigh (23 / 43), graded down the chain: 75/25, 50/50, 25/75. On the pelvis alone the panel is rigid and
            // the leg went through it as it stepped (Neuvillette3's front flap, 2026-09-26); on the thigh alone it swung its
            // face round with the leg and showed the lining; on the skin's skirt root (70 / 71) it folded back as it did on
            // the skirt chain. Graded, it moves part of the way with the leg -- neither fault in a timed series in game.
            main.splitGroups = {{49, {{0, 0.75}, {23, 0.25}}}, {50, {{0, 0.5}, {23, 0.5}}}, {51, {{0, 0.25}, {23, 0.75}}},
                                {53, {{0, 0.75}, {43, 0.25}}}, {54, {{0, 0.5}, {43, 0.5}}}, {55, {{0, 0.25}, {43, 0.75}}}};

            config.components = {main, coat, bang, eye};

            // Every component receives a forward vertex-group row, so none is hidden by request. A mod can
            // still put nothing on a component's bones (a summer outfit with no coat), and the template
            // hides such a component by the result.
            config.hiddenComponents = {};
            config.unremappedSlots = {};

            // A mod that hides his face, head-upper or eyebrows by hash (Neuvillette9's mask) hides the
            // skin's own too: they are other hashes on the skin, and its face showed through the mask in
            // pieces (in game, 2026-09-26). His eyebrows are the skin's too and need nothing.
            config.sideMeshes = {"ib_face", "ib_headupper"};

            // The flat normal map invented for a plain-layout object has BLUE = 0: every GI 6.x normal map
            // in play here is R, G ~128, B ~0, and this skin's shaders read B as a GLITTER mask -- the
            // template's default (B = 255) covered his boots and trousers in white sparkles (2026-09-24).
            config.flatNormal = Colour(55, 55, 0, 255);

            // No band move: his white mods render right on the skin's legend as they are, and moving his
            // silver-cloth band (126-128, the skin's cyan) onto its white cloth gave harder, darker shadows
            // on the cravat. The faint cyan cast that leaves is the maintainer's call.
            config.lightMapEdit = nullptr;
            config.compressTextures = false;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::neuvilletteMelusentMain6_3() {
        return makeGIMIComponentFixer(neuvilletteMelusentConfig(), "");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::neuvilletteMelusentCoat6_3() {
        return makeGIMIComponentFixer(neuvilletteMelusentConfig(), "Coat");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::neuvilletteMelusentBang6_3() {
        return makeGIMIComponentFixer(neuvilletteMelusentConfig(), "Bang");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::neuvilletteMelusentEye6_3() {
        return makeGIMIComponentFixer(neuvilletteMelusentConfig(), "Eye");
    }


    IniFixBuilder::Factory NeuvilletteFixer::main6_3() {
        return IniFixBuilderFuncs::neuvilletteMelusentMain6_3();
    }


    IniFixBuilder::Factory NeuvilletteFixer::coat6_3() {
        return IniFixBuilderFuncs::neuvilletteMelusentCoat6_3();
    }


    IniFixBuilder::Factory NeuvilletteFixer::bang6_3() {
        return IniFixBuilderFuncs::neuvilletteMelusentBang6_3();
    }


    IniFixBuilder::Factory NeuvilletteFixer::eye6_3() {
        return IniFixBuilderFuncs::neuvilletteMelusentEye6_3();
    }
}
