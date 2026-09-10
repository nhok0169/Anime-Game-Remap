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

#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"

#include <cstdint>
#include <filesystem>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"

namespace AGRemapCore {

    TexCreator::TexCreator(int width, int height, Colour colour, bool compress):
        width(width), height(height), colour(colour), compress(compress) {}

    void TexCreator::fix(TextureFile &texFile, const std::string &fixedTexFile) {
        std::error_code ec;
        if (std::filesystem::is_regular_file(FileService::strToPath(texFile.getSrc()), ec)) {
            return;
        }

        std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4);
        for (std::size_t i = 0; i + 3 < pixels.size(); i += 4) {
            pixels[i] = static_cast<std::uint8_t>(colour.red);
            pixels[i + 1] = static_cast<std::uint8_t>(colour.green);
            pixels[i + 2] = static_cast<std::uint8_t>(colour.blue);
            pixels[i + 3] = static_cast<std::uint8_t>(colour.alpha);
        }

        texFile.setSrc(fixedTexFile);
        texFile.setPixels(std::move(pixels), width, height);
        texFile.save(compress);
    }
}
