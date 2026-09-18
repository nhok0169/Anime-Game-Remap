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

#include "AGRemapCore/model/files/PositionFile.h"

#include <utility>

#include "AGRemapCore/model/buffers/BufFloat.h"


namespace AGRemapCore {

    namespace {
        std::vector<std::unique_ptr<BufDataType>> makeFloats(int count) {
            std::vector<std::unique_ptr<BufDataType>> types;
            for (int i = 0; i < count; ++i) {
                types.push_back(std::make_unique<BufFloat>());
            }
            return types;
        }
    }

    std::vector<std::unique_ptr<BufElementType>> PositionFile::defaultElements() {
        std::vector<std::unique_ptr<BufElementType>> elements;
        elements.push_back(std::make_unique<BufElementType>("POSITION", "R32G32B32_FLOAT", makeFloats(3)));
        elements.push_back(std::make_unique<BufElementType>("NORMAL", "R32G32B32_FLOAT", makeFloats(3)));
        elements.push_back(std::make_unique<BufElementType>("TANGENT", "R32G32B32A32_FLOAT", makeFloats(4)));
        return elements;
    }

    PositionFile::PositionFile(BinarySrc src): BufFile(std::move(src), defaultElements(), "Position.buf") {}
}
