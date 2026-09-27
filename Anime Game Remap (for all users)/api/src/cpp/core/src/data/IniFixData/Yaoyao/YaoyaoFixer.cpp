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

#include "AGRemapCore/data/IniFixData/Yaoyao/YaoyaoFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"


namespace AGRemapCore {
    namespace {
        // The skin's main-mesh slots -- match_first_index off the frame dump of its outfit preview
        const std::string HeadSlot = "0";
        const std::string BodySlot = "43092";

        // ---- her HEAD on the skin's head shader (2026-09-27) ----
        //
        // Her head (hair, the bells, the basket's rabbit) draws on a PLAIN hair shader; the skin's head slot is a
        // normal-map shader that reads two things hers does not, and on the skin her hair came out washed out --
        // pale, grey, and on a blonde mod (Yaoyao5) plainly grey-green. Each fix was one hand edit of a fixed .ini
        // in game before it went in:
        //
        //   * the light map's ALPHA BAND. Her whole head atlas is band 255 (hair, bells and the little skin it
        //     holds); the skin's head reads 255 as its pale green-white puffball flowers and keeps its hair on
        //     126-128. Moved to 127, the blonde hair came back blonde and her own brown hair matched her own
        //     outfit's to within a few levels (fringe h31 s0.49 v0.71 against h34 s0.46 v0.73). UNGATED: a
        //     skin-colour gate lets blonde hair through as "skin", and the little skin the head atlas holds (her
        //     neck) renders right on 127 too.
        const std::vector<MaterialBandRemapFilter::Band> HeadBands = {{254, 255, 127}};

        //   * the diffuse ALPHA. The skin's head diffuse is alpha 0 under its hair and hers is 255; the skin's
        //     head shader reads it (Yelan's lesson 7), and with the band moved but the alpha left, the bells still
        //     came out pale. Alpha 1, Yelan's value.
        const int HeadDiffuseAlpha = 1;


        void alphaOne(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, HeadDiffuseAlpha);
        }


        GIMIComponentFixerConfig yaoyaoBambooConfig() {
            // Remapped onto YaoyaoBamboo ("Rainlit Bamboo Reverie", 6.3) -- a skin of THREE components, the
            // NeuvilletteMelusent shape without a Coat. Every value was read off the prototype
            // (Tools/Misc/Prototypes/yaoyaoBambooFix.py), confirmed in game on her identity mod and ten real ones,
            // and off the two frame dumps of the outfit shop's previews (Tools/Misc/Diagnostics/giDrawTable.py).
            GIMIComponentFixerConfig config{};
            config.targetSkin = ModTypeIdTools::getName(ModTypeId::YaoyaoBamboo);
            config.drawnObjs = {"head", "body"};

            // Per object: a section binding ps-t2 is on the normal-map layout; otherwise it is plain (hers) and
            // shifted up onto the skin's normal-map slots with a flat normal map...
            config.sourceLayout = GIMIComponentFixerConfig::SourceLayout::Detect;

            // ...but a section rendering through its OWN NNFix is plain whatever it binds at ps-t2: Yaoyao3 and
            // Yaoyao10 bind a third texture they call a normal map beside NNFix, and read by ps-t2 their light maps
            // became the skin's diffuse -- the whole outfit vivid green (2026-09-27).
            config.layoutFromOwnFixCall = true;

            // ...and each binding read by its resource NAME, where the names can be believed.
            config.texRegsByName = true;

            // Both characters read the face diffuse at ps-t1 (GI 6.x) -- and it is the SAME texture, c70ae897, on
            // the same shared face meshes. Swap only a mod still on ps-t0.
            config.faceSwapOnlyFromDiffuseReg = true;

            // A mod toggling variants of one object on an if / else if chain with no else (Yaoyao2's three
            // hairstyles on $Hair) drew EVERY variant at once under the template's drawindexed = auto -- the
            // skin's braids over her hair, her hair clips gone (2026-09-27).
            config.fillDrawOnlyWhenUndrawn = true;

            // The main mesh. Its component name is EMPTY -- its files are YaoyaoBambooHead.ib,
            // YaoyaoBambooPosition.buf -- and only its fix-target id carries a name. Her head (hair, face skin,
            // the rabbit) through the Head slot, her body through the Body slot: the same kind of part on each side.
            GIMIComponentFixerConfig::Component main{};
            main.name = "";
            main.modTypeName = ModTypeIdTools::getName(ModTypeId::YaoyaoBambooMain);
            main.slot = "Head";
            main.slotIndex = HeadSlot;
            main.objSlotIndices = {{"body", BodySlot}};
            main.slotIndices = {HeadSlot, BodySlot};
            main.negativeIndex = false;
            main.normalMap = true;
            main.face = true;
            main.texcoordStride = 12;       // YaoyaoBambooTexcoord.buf: 334032 / 27836
            main.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // The Bang: one slot, on the normal-map layout with the Head's textures.
            GIMIComponentFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.modTypeName = ModTypeIdTools::getName(ModTypeId::YaoyaoBambooBang);
            bang.slot = "A";
            bang.slotIndex = "0";
            bang.slotIndices = {"0"};
            bang.negativeIndex = false;
            bang.normalMap = true;
            bang.face = false;
            bang.texcoordStride = 12;       // YaoyaoBambooBangTexcoord.buf: 33948 / 2829
            bang.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // Her eyes (vertex groups 4 / 5) are in her HEAD object. The skin draws its Eye on the PLAIN shader
            // 95aa6cdb with a diffuse / light map at ps-t0 / ps-t1 under NNFix. Her eye mesh already sits where the
            // skin's does -- 0.7 mm apart on average over the identity mod, bounding boxes within 0.2 mm -- so no
            // position offset, unlike Neuvillette's.
            GIMIComponentFixerConfig::Component eye = bang;
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::YaoyaoBambooEye);
            eye.normalMap = false;
            eye.texcoordStride = 12;        // YaoyaoBambooEyeTexcoord.buf: 2856 / 238
            eye.slotRegisters = {"ps-t0", "ps-t1"};
            eye.offsetOnlyWithGameFace = true;

            // The Eye LAST: it owns the hidden components and the TexFx guards, so it has to be the last fixer to
            // run -- the same order as the rows in IniFixBuilderData.
            config.components = {main, bang, eye};

            // Every component receives a forward vertex-group row, so none is hidden by request; a mod can still
            // put nothing on a component's bones, and the template hides such a component by the result.
            config.hiddenComponents = {};
            config.unremappedSlots = {};

            // Her face meshes are drawn by the skin under the SAME hashes, so a mod hiding one by hash hides it on
            // the skin too: no side meshes to translate.
            config.sideMeshes = {};

            config.diffuseEdits = {{"head", &alphaOne}};
            config.lightMapEdit = MaterialBandRemapFilter::lightMapEdit(HeadBands);
            config.lightMapObjs = {"head"};
            config.compressTextures = false;    // a band selector is exact; BC7 would move it

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::yaoyaoBambooMain6_3() {
        return makeGIMIComponentFixer(yaoyaoBambooConfig(), "");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::yaoyaoBambooBang6_3() {
        return makeGIMIComponentFixer(yaoyaoBambooConfig(), "Bang");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::yaoyaoBambooEye6_3() {
        return makeGIMIComponentFixer(yaoyaoBambooConfig(), "Eye");
    }


    IniFixBuilder::Factory YaoyaoFixer::main6_3() {
        return IniFixBuilderFuncs::yaoyaoBambooMain6_3();
    }


    IniFixBuilder::Factory YaoyaoFixer::bang6_3() {
        return IniFixBuilderFuncs::yaoyaoBambooBang6_3();
    }


    IniFixBuilder::Factory YaoyaoFixer::eye6_3() {
        return IniFixBuilderFuncs::yaoyaoBambooEye6_3();
    }
}
