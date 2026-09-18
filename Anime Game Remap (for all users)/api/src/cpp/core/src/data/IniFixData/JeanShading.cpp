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

#include "AGRemapCore/data/IniFixData/JeanShading.h"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"


namespace AGRemapCore {

    const int JeanShading::LowAlpha = 77;


    void JeanShading::liftLowAlpha(TextureFile& texFile) {
        if (!texFile.hasImage()) {
            return;
        }

        // getPixels() hands back a const reference, so the buffer is copied, rewritten and handed
        // back through setPixels -- which is also what keeps the width/height bookkeeping honest.
        // Same shape as TexEditor::setTransparency, which is the other edit of this kind.
        std::vector<std::uint8_t> pixels = texFile.getPixels();
        for (std::size_t i = 3; i < pixels.size(); i += 4) {
            if (pixels[i] > static_cast<std::uint8_t>(LowAlpha)) {
                continue;
            }

            pixels[i] = static_cast<std::uint8_t>(std::min(255, static_cast<int>(pixels[i]) + LowAlpha));
        }

        texFile.setPixels(std::move(pixels), texFile.getWidth(), texFile.getHeight());
    }
}
