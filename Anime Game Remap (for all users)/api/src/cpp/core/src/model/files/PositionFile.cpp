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
