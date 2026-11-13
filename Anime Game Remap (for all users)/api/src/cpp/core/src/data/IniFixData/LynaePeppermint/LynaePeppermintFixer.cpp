#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintFixer.h"

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

#include <string>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintTextures.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {


    // Transcribed from Tools/Misc/Prototypes/peppermintLynaeFix.py, which stays the oracle this is
    // A/B'd against. Pairing: 0-4 and 7 one to one (the face and eyes are the same mesh); the skin's
    // coat (5) onto Lynae's jacket (5), and her shirt / jacket / shoes (6) onto Lynae's LOWER BODY (4)
    // with the skin's own see-through shader -- see the plan's note. Lynae's props slot (6) renders
    // through a special-material (glass / foil) layer that barely reaches the G-buffer -- 137 pixels
    // for her OWN props in a vanilla frame dump -- so nothing of the skin goes there.
    //
    // (The skin's component 5 roles are named "pouch" after the asset folder's first reading of that
    // component; it is her coat.)
    WWMIFixerConfig LynaePeppermintFixer::lynae3_7Config() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::Lynae;
        config.sourceVersion = "3.7";

        // ---- the TARGET draws her early depth passes with her skeleton in vs-cb3 alone ----
        config.currentPoseInCb3Only = true;
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // ---- the passes the TARGET (Lynae) draws each slot on (her dumps, wwmiDrawTable.py) ----
        // slotPasses: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

        // ---- every pass gated through its VERTEX shaders, so RabbitFX's pixel-shader tags survive ----
        // passVertexShaders: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

        // ---- ...and every tag NAMED. A shader holds ONE filter_index across every loaded .ini, so
        // every shader another pair tags keeps THAT value: Chisa's 3e7bb648 .71, 343a49bd .713,
        // 5fd6e5bb .718, 6a6650a9 .73, e4a3da6d .731, ChisaParfait -> Chisa's 7c0b4db3 .76, and this
        // pair's forward direction's .61x. The shaders only this direction tags take 3381.62x ----
        // filterIndices: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

        // ---- source component -> Lynae's slot and the registers it binds there ----
        config.plan = {
            // ps-t2 (36c90686) and ps-t3 (4eaa9816) are the same texture on both skins: left to the game
            {0, {0, {{"ps-t0", "bangsMask"}, {"ps-t1", "bangsDiffuse"}, {"ps-t4", "hairShadeRamp"}, {"ps-t5", "bangsStrand"}}}},
            {1, {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"},
                     {"ps-t4", "hairShadeRamp"}, {"ps-t5", "hairStrand"}}}},
            // Lynae's face shader reads the diffuse at ps-t1; her mask (packed differently) stays -- the
            // face is the same mesh, so its UVs are hers
            {2, {2, {{"ps-t1", "faceDiffuse"}}}},
            {3, {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDetail"}, {"ps-t3", "upperDiffuse"}}}},
            // her lower body through her own lower-body pass -- see lowerBody below. It reads her noise at
            // ps-t7 and her holographic foil at ps-t8; ps-t4..t6 and the ramp at ps-t9 are the same game
            // textures on Lynae's lower body
            {4, {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDetail"}, {"ps-t3", "lowerDiffuse"},
                     {"ps-t7", "jacketNoise"}, {"ps-t8", "jacketFoil"}}}},
            // the coat onto Lynae's LOWER body too, as a second section like the shirt, through the skin's
            // own see-through coat pass -- see sheerCoat below. The skin's register order, her noise and
            // the coat's foil are what it reads. (Lynae's jacket slot is left with nothing, and hidden)
            {5, {4, {{"ps-t0", "pouchNormal"}, {"ps-t1", "pouchMask"}, {"ps-t2", "pouchDetail"}, {"ps-t3", "pouchDiffuse"},
                     {"ps-t4", "jacketNoise"}, {"ps-t5", "pouchFoil"}}, true}},
            // the shirt is SEE-THROUGH on the skin, and no shader of Lynae's draws that: her jacket's drew
            // it opaque white. The skin's own sheer pass draws it -- see sheerJacket below -- and it has
            // to come AFTER the body it shows: the skin draws her jacket after her upper body, Lynae
            // draws her jacket slot before hers (the shirt's depth then hides the body behind it), and
            // her lower body after both. So the shirt goes through the lower body's draw, as a second
            // section in the mod's own file: a copy .ini never fills the shape-keyed positions a mod
            // with custom shape keys draws from (sameFileSection). The skin's register order, her noise
            // and her foil are what that shader reads
            {6, {4, {{"ps-t0", "jacketNormal"}, {"ps-t1", "jacketMask"}, {"ps-t2", "jacketDiffuse"}, {"ps-t3", "jacketDetail"},
                     {"ps-t4", "jacketNoise"}, {"ps-t5", "jacketFoil"}}, true}},
            {7, {7, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"}}}},
        };

        // ---- a slot's OTHER passes: the outlines take the slot's diffuse at ps-t0 ----
        config.extraPassRegs = {
            {0, {{"other", {{"ps-t0", "bangsDiffuse"}}}}},
            {1, {{"other", {{"ps-t0", "hairDiffuse"}}}}},
            {3, {{"other", {{"ps-t0", "upperDiffuse"}}}}},
            // her lower body's outline reads a mask at ps-t0 and the diffuse at ps-t1. Her own mask there
            // is a flat white ("outline everywhere"); the skin's outline reads its DIFFUSE at ps-t0, and
            // with the white the outline shell of the skin's layered thigh overlay drew as two black
            // bands down the backs of her thighs. So the skin's diffuse goes at both. That slot takes TWO
            // sources (the lower body and the shirt), and "the diffuse" is a different file for each:
            // without srcComponent both land in one list and the last one's wins
            {4, {{"other", {{"ps-t0", "lowerDiffuse", 4}, {"ps-t1", "lowerDiffuse", 4},
                                       {"ps-t0", "jacketDiffuse", 6}, {"ps-t1", "jacketDiffuse", 6},
                                       {"ps-t0", "pouchDiffuse", 5}, {"ps-t1", "pouchDiffuse", 5}}}}},
            // her second eye pass SETS its own t0, and it is the eye MASK (a506a70d in her dump)
            {7, {{"other", {{"ps-t0", "eyeMask"}}}}},
        };

        config.sourceTextures = lynaePeppermintTextureFacts();

        // ---- the skin's sheer pass, with the state its own draw has (her frame dumps, read losslessly
        // with dump_rt_dds and dump_depth). It writes only some of the G-buffer: nothing of targets 0
        // and 6, and only the colour of 1 and 2 -- 2's alpha is the shading model, so the BODY's stays
        // under the shirt. It ALPHA-BLENDS its base colour (target 3) over the body's by the opacity it
        // writes in that target's alpha: that blend is the see-through (the body's colour under the shirt
        // depends on what was there before by about a fifth, in her frame and with this) -- and ONLY over
        // her body: her frame has it draw no pixel over the background, her other jacket pass draws the
        // shirt opaque there, and her hair is not seen through it either. Her body marks itself in the
        // stencil and Lynae's does not, so the mod's body marks bit 1 (a bit no draw of either character
        // sets; 16, the first tried, blackened the body -- the lighting reads it) where it is the visible
        // surface, and the shirt is drawn twice: blended where bit 1 is set, opaque where it is not.
        // Blended everywhere, the sleeves and edges over the background were a ghostly see-through. And
        // it marks its pixels in the stencil: ref 14 written through mask 0x8a (the background's 128 becomes
        // 10, her body's 14 stays 14). Lynae's own draws never set that 8 bit, and without it the
        // lighting shades the shirt as an opaque one. depth_stencil_state_merge keeps the rest of the
        // lower body's state -- its reversed-Z depth test above all: without it the default LESS test
        // throws away every pixel in front of the body. The shader reads her lighting ramp at ps-t6:
        // HERS, downloaded -- Lynae's slots hold the same game texture refilled with Lynae's ramp ----
        // (a register of her shaders left empty is where her own draw binds the role: her TextureRegisters)
        WWMIFixerConfig::Shader sheerJacket = WWMIFixerConfig::Shader::ofGame("LynaePeppermintSheer");
        sheerJacket.gameRoles = {{"", "ramp"}};
        sheerJacket.lines = {
            {IniKeywords::DepthStencilStateMerge, "true"},
            {IniKeywords::StencilEnable, "true"},
            {IniKeywords::StencilRef, "15"},                 // her 14, plus the body mark (bit 1) to test against
            {IniKeywords::StencilReadMask, "1"},
            {IniKeywords::StencilWriteMask, "0x8a"},
            // TWO-SIDED: her shirt and coat draws use the rasterizer state her hair and effects do, which
            // Lynae's hair uses too, where Lynae's jacket and body slots cull back faces -- so the coat's
            // inside was never drawn, and its far wall showed through dark. The merge keeps the rest of
            // the slot's state: a `cull` alone builds one from defaults, and the defaults' winding turned
            // every face around (the whole coat came out deep blue)
            {IniKeywords::RasterizerStateMerge, "true"},
            {IniKeywords::Cull, "none"},
        };
        // The render-target masks are the SEE-THROUGH pass's alone. Where the shirt is solid it writes every
        // target, as her solid jacket pass does there: with the sheer masks, render targets 0 and 6 kept the
        // scene behind it, and the shirt's edge past her bust came out a translucent band with the sky
        // showing through, where hers is solid white
        sheerJacket.passes = {
            {{IniKeywords::Mask + "[0]", "0"}, {IniKeywords::Mask + "[1]", "0x7"}, {IniKeywords::Mask + "[2]", "0x7"}, {IniKeywords::Mask + "[6]", "0"},
             {IniKeywords::BlendState + "[3]", "ADD SRC_ALPHA INV_SRC_ALPHA"}, {IniKeywords::Alpha + "[3]", "ADD ONE ZERO"},
             {IniKeywords::StencilFront, "equal replace keep keep"}, {IniKeywords::StencilBack, "equal replace keep keep"}},
            {{IniKeywords::Mask + "[0]", "0xf"}, {IniKeywords::Mask + "[1]", "0xf"}, {IniKeywords::Mask + "[2]", "0xf"}, {IniKeywords::Mask + "[6]", "0xf"},
             {IniKeywords::StencilFront, "not_equal replace keep keep"}, {IniKeywords::StencilBack, "not_equal replace keep keep"}},
        };
        // ...and that OPAQUE pass with her SOLID jacket shader (efc2690a), as her own draw does first: drawn with
        // the see-through one, her lemon earrings, necklace and bag strap came out pale and half see-through --
        // a paint of the see-through shader's output showed it never touches them on her card (epoch 6)
        sheerJacket.passGame = {"", "LynaePeppermintSolid"};
        // ...after her upper body, whichever slot the game draws first: in the overworld her jacket slot
        // draws BEFORE the upper body (the character menu after it), so the shirt blended over her hair,
        // the coat's back panel and the scene behind her, and the body then lost the depth test under it.
        // Her upper body makes two G-buffer draws a pass (the second fills what the first leaves where
        // the coat's stencil is), and the shirt follows the second
        WWMIFixerConfig::Shader::After afterBody;
        afterBody.component = 3;
        afterBody.draws = 2;
        afterBody.carried = {"ps-t0", "ps-t1", "ps-t2", "ps-t3", "ps-t4", "ps-t5", "ps-t9"};
        sheerJacket.after = afterBody;
        // only where the pass binds the G-buffer's base colour: the overworld's depth pre-pass draws the
        // shirt with the game's own pre-pass shader, as any other component
        sheerJacket.onlyWith = IniKeywords::WWMIColourTarget;

        // ---- and her COAT is drawn the same way: a solid pass, then a see-through one (4470be47) that
        // blends over the body wherever the coat's material says so -- in her own frame it leaves 4120
        // pixels of her body showing through the coat, and a mod may paint a whole garment there
        // (LynaePeppermint2's shirt is the coat component, and Lynae's jacket shader drew it solid
        // cyan). The same state as the shirt's, on the same slot and after the same body: drawn on her
        // jacket slot it was released before the lower body in the character menu and drew opaque over
        // the hips, and released from inside the lower body's own section (After::also) it drew a
        // saturated blue with dark bands -- the shirt's arrangement, its own section on the lower body's
        // draw, is the one that renders as the skin does ----
        WWMIFixerConfig::Shader sheerCoat = WWMIFixerConfig::Shader::ofGame("LynaePeppermintSheerCoat");
        sheerCoat.gameRoles = sheerJacket.gameRoles;
        sheerCoat.lines = sheerJacket.lines;
        sheerCoat.passes = sheerJacket.passes;
        sheerCoat.after = afterBody;
        sheerCoat.onlyWith = IniKeywords::WWMIColourTarget;

        config.plan.at(6).shader = std::move(sheerJacket);
        config.plan.at(5).shader = std::move(sheerCoat);

        // ---- and her LOWER BODY through her own lower-body pass (0a52e215): the holographic parts of her
        // outfit are a foil only that shader draws, and Lynae's lower-body shader drew them flat grey (a
        // mod's jacket panel modelled on the lower body). Her body's stencil -- ref 14, which turns the
        // background's 128 into 14 -- and two-sided, as her own draw is ----
        WWMIFixerConfig::Shader lowerBody = WWMIFixerConfig::Shader::ofGame("LynaePeppermintLower");
        lowerBody.lines = {
            {IniKeywords::DepthStencilStateMerge, "true"},
            {IniKeywords::StencilEnable, "true"},
            {IniKeywords::StencilRef, "14"},
            {IniKeywords::StencilWriteMask, "0x8e"},
            {IniKeywords::StencilFront, "always replace keep keep"},
            {IniKeywords::StencilBack, "always replace keep keep"},
            {IniKeywords::RasterizerStateMerge, "true"},
            {IniKeywords::Cull, "none"},
        };
        lowerBody.gameRoles = {{"", "ramp"}};
        lowerBody.onlyWith = IniKeywords::WWMIColourTarget;
        config.plan.at(4).shader = std::move(lowerBody);

        // ---- and her UPPER BODY through her own upper-body pass (ed4fe222). Drawn by Lynae's upper-body
        // shader with her maps edited to approximate her, her materials came out as Lynae's: a satin bikini
        // flat purple, a gold starfish pale cream, the seams of the shirt showing on bare skin with it off.
        // Her register order is Lynae's slot's (normal, mask, code, diffuse, the ramp at ps-t6); her noise
        // (ps-t4) and foil (ps-t5) are the game's, bound only while her shader draws. Her body's stencil, as
        // her lower body: the shirt's see-through pass tests those bits ----
        WWMIFixerConfig::Shader upperBody = WWMIFixerConfig::Shader::ofGame("LynaePeppermintUpper");
        upperBody.lines = {
            {IniKeywords::DepthStencilStateMerge, "true"},
            {IniKeywords::StencilEnable, "true"},
            {IniKeywords::StencilRef, "14"},
            {IniKeywords::StencilWriteMask, "0x8e"},
            {IniKeywords::StencilFront, "always replace keep keep"},
            {IniKeywords::StencilBack, "always replace keep keep"},
        };
        upperBody.gameRoles = {{"", "ramp"}};
        upperBody.registers = {{"", "jacketNoise"}, {"", "upperFoil"}};
        upperBody.onlyWith = IniKeywords::WWMIColourTarget;
        config.plan.at(3).shader = std::move(upperBody);

        // ...and the body it is seen through marks itself (bit 1), where it is the visible surface
        const std::vector<std::pair<std::string, std::string>> bodyMark = {
            {IniKeywords::DepthStencilStateMerge, "true"},
            {IniKeywords::DepthFunc, "equal"},
            {IniKeywords::DepthWriteMask, "zero"},
            {IniKeywords::StencilEnable, "true"},
            {IniKeywords::StencilRef, "1"},
            {IniKeywords::StencilWriteMask, "1"},
            {IniKeywords::StencilReadMask, "0xff"},
            {IniKeywords::StencilFront, "always replace keep keep"},
            {IniKeywords::StencilBack, "always replace keep keep"},
        };
        config.plan.at(3).markLines = bodyMark;
        // (the lower body draws two-sided now, so its mark does too, or its inner faces stay unmarked and
        // the shirt draws opaque over them)
        std::vector<std::pair<std::string, std::string>> lowerMark = bodyMark;
        lowerMark.emplace_back(IniKeywords::RasterizerStateMerge, "true");
        lowerMark.emplace_back(IniKeywords::Cull, "none");
        config.plan.at(4).markLines = lowerMark;

        // In the overworld only, Lynae also draws a black long-sleeved top under her shirt, its own mesh
        // (2679 vertices). The skin's body is slimmer at the neck and arms, so it showed through there as a
        // black collar and sleeves -- in the character menu it is not drawn at all. 8 of the 14 Lynae mods
        // skip it themselves (`FixBlackModel.ini`, `[TextureOverrideBlackShirt]`)
        config.hiddenMeshes = {"blackShirt"};      // Lynae's HashData mesh type


        // Masks mark regions: a flat one from a mod is replaced by HER game texture
        // the masks a flat or aliased file is refused for are the source's: see WWMITextureFacts::regionRoles

        // ---- the three lines that undo the remap on a source past 256 bones, and RabbitFX's ----

        // ---- the shape keys are RETARGETED; a batched export's dispatch height is LYNAE's (her
        // Metadata.json's dispatch_y) ----
        // shapeKeyDispatchSize: read off ShapeKeyDispatchSizes at the run's versions (WWMIFixerConfig::withGameData)

        // ---- the game drives shape keys by SLOT, and the two skins number the same face keys
        // differently (2 of 105 at the same index): moved across unchanged, a blink opened the mouth ----
        // shapeKeyOrder: read off ShapeKeyRemaps at the run's versions (WWMIFixerConfig::withGameData)
        config.cleanTexcoords = true;

        // A mod's own line that puts a texture into another kind of slot on purpose keeps that slot:
        // Lynae3's `$Wish` (U) binds each part's diffuse into the hair ramp / detail slot, `ps-t2`,
        // which the two skins read alike -- moved by the texture's role it landed on the diffuse
        // register and the toggle did nothing
        config.carryByRegisterRole = true;


        // ---- a mod from before WWMI's merged skeleton: the skin was never renumbered ----
        // sourceVgMaps: read off VGRemaps (component -> "") at the run's versions (WWMIFixerConfig::withGameData)

        config.sourceLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "lower body"},
            {5, "coat"}, {6, "shirt, jacket and shoes"}, {7, "eyes"},
        };
        config.targetLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "lower body"},
            {5, "jacket"}, {6, "props (headphones, pin, ID card)"}, {7, "eyes"},
        };

        return config;
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::lynae3_7() {
        return makeWWMIFixer(LynaePeppermintFixer::lynae3_7Config());
    }


    IniFixBuilder::Factory LynaePeppermintFixer::lynae3_7() {
        return IniFixBuilderFuncs::lynae3_7();
    }
}
