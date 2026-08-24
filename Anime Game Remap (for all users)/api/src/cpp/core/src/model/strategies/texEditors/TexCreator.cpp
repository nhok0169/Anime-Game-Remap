#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"

#include <cstdint>
#include <filesystem>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"

namespace AGRemapCore {

    TexCreator::TexCreator(int width, int height, Colour colour): width(width), height(height), colour(colour) {}

    void TexCreator::fix(TextureFile &texFile, const std::string &fixedTexFile) {
        std::error_code ec;
        if (std::filesystem::is_regular_file(texFile.getSrc(), ec)) {
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
        texFile.save();
    }
}
