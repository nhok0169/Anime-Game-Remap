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

#include "AGRemapCore/data/IniFixData/Yelan/YelanFixer.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"


namespace AGRemapCore {
    namespace {
        // ---- the band legend ----
        //
        // A lightmap's alpha is a material band, and the legend differs per skin. Read off the two
        // skins' own textures at their vertices (Yelan's from her identity mod):
        //
        //   Yelan:     0 = hair and dark cloth, 64-89 metal, 115-127 skin, 165-189 ornaments,
        //              255 = white FUR (the shawl, the trims, the jacket lining)
        //   Tranquil:  0 = white fur, 64-89 silver, 115-128 hair, 165-189 silk and the lace cape,
        //              255 = skin
        //
        // So two bands move and the rest already line up: Yelan's fur onto Tranquil's fur (her own
        // shawl would otherwise render as skin, and an author who leaves the alpha opaque has
        // painted every cloth as fur -- grey stockings came out beige), and Yelan's skin onto
        // Tranquil's. But an author need not follow Yelan's legend at all: a Clorinde port keeps
        // Clorinde's bands, hair on 115-127 and skin on 50-99, and lifting that hair onto the skin
        // ramp put speckles all over it. So the skin lift is conditional on the DIFFUSE under the
        // pixel looking like skin; the fur move is not (a mod's 255 is never Tranquil-legend skin).
        // Read as a MaterialBandRemapFilter table: source band (or range) -> target band, with an
        // optional test on the diffuse under the pixel. Order matters only where two ranges would
        // overlap; these do not.
        const std::vector<MaterialBandRemapFilter::Band> Bands = {
            // Yelan's fur onto Tranquil's fur. Unconditional: her 255 is never Tranquil-legend skin.
            {255, 0},

            // Yelan's skin onto Tranquil's, but ONLY where the diffuse agrees the pixel is skin --
            // a Clorinde port keeps Clorinde's legend, with hair on 115-127, and lifting that onto
            // the skin ramp speckled it.
            {115, 127, 255, &MaterialBandRemapFilter::skinColoured},
        };

        // Tranquil's shader darkens by the diffuse alpha; Yelan's ignores it. Alpha 1, not 0: the
        // maintainer's Copy28 recipe, confirmed in game.
        const int HeadDiffuseAlpha = 1;


        void alphaOne(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, HeadDiffuseAlpha);
        }



        GIMIComponentFixerConfig yelanTranquilConfig() {
            // Remapped onto YelanTranquil -- the first skin of SEVERAL components, and the shape
            // every GI character from Bennett on takes. See GIMIComponentFixerConfig for what each
            // field does; every value here was read off the prototype
            // (Tools/Misc/Prototypes/yelanTranquilFix.py), which four mods confirmed in game:
            // the china dress, the Fontaine outfit with its cape, a Clorinde port, and Yelan's own
            // identity mod.
            GIMIComponentFixerConfig config{};
            config.targetSkin = ModTypeIdTools::getName(ModTypeId::YelanTranquil);
            config.drawnObjs = {"head", "body", "dress", "extra"};

            // Which of Tranquil's draw slots each component draws the mod through, by the slot's
            // register layout: her Body's slot C and her Bang read the normal-map layout (ps-t0
            // normal map, ps-t1 diffuse, ps-t2 lightmap, under ORFix), her Eye reads ps-t0 diffuse
            // / ps-t1 lightmap under NNFix. The Body takes the graph cut, the Bang the negative
            // index (it draws the mod's whole head and keeps the head weight on its own head bone),
            // the Eye the cut. Only the Body carries the face.
            GIMIComponentFixerConfig::Component body{};
            body.name = "Body";
            body.modTypeName = ModTypeIdTools::getName(ModTypeId::YelanTranquilBody);
            body.slot = "C";
            body.slotIndex = "67374";
            body.negativeIndex = false;
            body.normalMap = true;
            body.face = true;

            GIMIComponentFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.modTypeName = ModTypeIdTools::getName(ModTypeId::YelanTranquilBang);
            bang.slot = "A";
            bang.slotIndex = "0";
            bang.negativeIndex = true;
            bang.normalMap = true;
            bang.face = false;

            GIMIComponentFixerConfig::Component eye{};
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::YelanTranquilEye);
            eye.slot = "A";
            eye.slotIndex = "0";
            eye.negativeIndex = false;
            eye.normalMap = false;
            eye.face = false;

            config.components = {body, bang, eye};

            // The texture recipe (the maintainer's Copy28, plus the band legend above): the head
            // diffuse at alpha 1, every lightmap through the band table, a flat normal map created
            // per normal-map slot -- pre-corrected to (55, 55, 255), what the reference's 127 under
            // an sRGB header samples as, since a created texture is written untagged -- and every
            // texture with its mip chain.
            config.diffuseEdits = {{"head", &alphaOne}};
            config.lightMapEdit = MaterialBandRemapFilter::lightMapEdit(Bands);

            // The generated second, third and fourth files explain themselves.
            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::yelanTranquilBody6_1() {
        return makeGIMIComponentFixer(yelanTranquilConfig(), "Body");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::yelanTranquilBang6_1() {
        return makeGIMIComponentFixer(yelanTranquilConfig(), "Bang");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::yelanTranquilEye6_1() {
        return makeGIMIComponentFixer(yelanTranquilConfig(), "Eye");
    }


    IniFixBuilder::Factory YelanFixer::body6_1() {
        return IniFixBuilderFuncs::yelanTranquilBody6_1();
    }


    IniFixBuilder::Factory YelanFixer::bang6_1() {
        return IniFixBuilderFuncs::yelanTranquilBang6_1();
    }


    IniFixBuilder::Factory YelanFixer::eye6_1() {
        return IniFixBuilderFuncs::yelanTranquilEye6_1();
    }
}
