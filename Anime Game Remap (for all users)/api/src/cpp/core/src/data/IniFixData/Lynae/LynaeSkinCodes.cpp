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

#include "AGRemapCore/data/IniFixData/Lynae/LynaeSkinCodes.h"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"


namespace AGRemapCore {
    namespace {
        // Measured over the two skins' game textures by the diffuse under each texel: flesh is
        // material 0 on 98% of Lynae's upper and lower body, material 4 on 94% / 82% of the skin's
        constexpr std::uint8_t LynaeSkin = 0;
        constexpr std::uint8_t PeppermintSkin = 4;

        std::uint8_t swapCode(std::uint8_t v) {
            const std::uint8_t material = v & 0x0F;
            const std::uint8_t flags = v & 0xF0;
            if (material == LynaeSkin) {
                return static_cast<std::uint8_t>(flags | PeppermintSkin);
            }
            if (material == PeppermintSkin) {
                return static_cast<std::uint8_t>(flags | LynaeSkin);
            }
            return v;
        }
    }

    TexEditor::Filter lynaeSkinCodeSwap() {
        return [](TextureFile& tex) {
            tex.setGamma(std::nullopt);          // these bytes are codes, not colour
            std::vector<std::uint8_t> px = tex.getPixels();

            // The shader reads the red channel; a single-channel source comes in as (v, v, v, 255),
            // and the copy is written the same way
            for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                const std::uint8_t v = swapCode(px[i]);
                px[i] = v;
                px[i + 1] = v;
                px[i + 2] = v;
            }

            tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
        };
    }
}
