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
        const std::uint8_t SkinBandLow = 115;
        const std::uint8_t SkinBandHigh = 127;
        const std::uint8_t TargetSkinBand = 255;
        const std::uint8_t SourceFurBand = 255;
        const std::uint8_t TargetFurBand = 0;

        // Tranquil's shader darkens by the diffuse alpha; Yelan's ignores it. Alpha 1, not 0: the
        // maintainer's Copy28 recipe, confirmed in game.
        const int HeadDiffuseAlpha = 1;


        bool skinColoured(int r, int g, int b) {
            return r >= 96 && r >= g && g >= b && (r - b) >= 16 && (r - b) <= 140;
        }


        void alphaOne(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, HeadDiffuseAlpha);
        }


        // The lightmap edit for one object, closed over that object's diffuse: fur 255 -> 0, then
        // skin 115-127 -> 255 where the diffuse agrees the pixel is skin. The diffuse is sampled
        // nearest-neighbour where the two textures differ in size.
        TexEditor::Filter liftBands(const std::string& diffusePath) {
            return [diffusePath](TextureFile& texFile) {
                const int width = texFile.getWidth();
                const int height = texFile.getHeight();
                std::vector<std::uint8_t> pixels = texFile.getPixels();
                if (width <= 0 || height <= 0 || pixels.size() < static_cast<std::size_t>(width) * height * 4) {
                    return;
                }

                std::vector<std::uint8_t> diffuse;
                int diffuseWidth = 0;
                int diffuseHeight = 0;
                if (!diffusePath.empty()) {
                    TextureFile diffuseFile(diffusePath);
                    diffuseFile.open();
                    if (diffuseFile.hasImage()) {
                        diffuse = diffuseFile.getPixels();
                        diffuseWidth = diffuseFile.getWidth();
                        diffuseHeight = diffuseFile.getHeight();
                    }
                }

                const bool haveDiffuse = !diffuse.empty() && diffuseWidth > 0 && diffuseHeight > 0;

                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        std::uint8_t& alpha = pixels[(static_cast<std::size_t>(y) * width + x) * 4 + 3];

                        if (alpha == SourceFurBand) {
                            alpha = TargetFurBand;
                            continue;
                        }

                        if (alpha < SkinBandLow || alpha > SkinBandHigh) {
                            continue;
                        }

                        if (haveDiffuse) {
                            const int dx = static_cast<int>(static_cast<long long>(x) * diffuseWidth / width);
                            const int dy = static_cast<int>(static_cast<long long>(y) * diffuseHeight / height);
                            const std::size_t at = (static_cast<std::size_t>(dy) * diffuseWidth + dx) * 4;
                            if (at + 2 < diffuse.size() && !skinColoured(diffuse[at], diffuse[at + 1], diffuse[at + 2])) {
                                continue;
                            }
                        }

                        alpha = TargetSkinBand;
                    }
                }

                texFile.setPixels(std::move(pixels), width, height);
            };
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
            config.lightMapEdit = &liftBands;

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
