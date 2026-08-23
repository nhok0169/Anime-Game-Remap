#include "AGRemapCore/model/files/BlendFile.h"

#include <cmath>
#include <utility>

#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/buffers/BufInt.h"


namespace AGRemapCore {

    namespace {
        constexpr const char* BlendWeightKey = "BLENDWEIGHT";
        constexpr const char* BlendIndicesKey = "BLENDINDICES";

        long long asIndex(const BufValue& value) {
            return std::visit([](auto&& v) -> long long { return static_cast<long long>(v); }, value);
        }

        double asWeight(const BufValue& value) {
            return std::visit([](auto&& v) -> double { return static_cast<double>(v); }, value);
        }
    }

    std::vector<std::unique_ptr<BufElementType>> BlendFile::defaultElements() {
        std::vector<std::unique_ptr<BufElementType>> elements;

        std::vector<std::unique_ptr<BufDataType>> weightTypes;
        for (int i = 0; i < 4; ++i) {
            weightTypes.push_back(std::make_unique<BufFloat>());
        }
        elements.push_back(std::make_unique<BufElementType>(BlendWeightKey, "R32G32B32A32_FLOAT", std::move(weightTypes)));

        std::vector<std::unique_ptr<BufDataType>> indexTypes;
        for (int i = 0; i < 4; ++i) {
            indexTypes.push_back(std::make_unique<BufSignedInt>());
        }
        elements.push_back(std::make_unique<BufElementType>(BlendIndicesKey, "R32G32B32A32_SINT", std::move(indexTypes)));

        return elements;
    }

    BlendFile::BlendFile(BinarySrc src, std::vector<std::unique_ptr<BufElementType>> elements):
        BufFile(std::move(src), elements.empty() ? defaultElements() : std::move(elements), "Blend.buf") {}

    std::unordered_map<long long, long long> BlendFile::getMissingIndicesRemap(const BufLineData& src, const VGRemap& vgRemap) {
        const std::vector<BufValue>& blendWeights = src.at(BlendWeightKey);
        const std::vector<BufValue>& blendIndices = src.at(BlendIndicesKey);
        std::size_t minBlendLen = std::min(blendWeights.size(), blendIndices.size());

        std::unordered_map<long long, long long> result;
        const auto& remap = vgRemap.getRemap();

        for (std::size_t i = 0; i < minBlendLen; ++i) {
            long long index = asIndex(blendIndices[i]);
            if (remap.find(index) == remap.end()) {
                result[index] = -std::abs(index) - 1;
            }
        }

        return result;
    }

    BufLineData BlendFile::remapIndices(BufLineData src, const VGRemap& vgRemap, bool remapMissingIndices) {
        std::unordered_map<long long, long long> tempMissingIndexRemap;
        if (remapMissingIndices) {
            tempMissingIndexRemap = getMissingIndicesRemap(src, vgRemap);
        }

        std::vector<BufValue>& blendWeights = src.at(BlendWeightKey);
        std::vector<BufValue>& blendIndices = src.at(BlendIndicesKey);
        std::size_t minBlendLen = std::min(blendWeights.size(), blendIndices.size());

        const auto& remap = vgRemap.getRemap();

        for (std::size_t i = 0; i < minBlendLen; ++i) {
            double weight = asWeight(blendWeights[i]);
            long long index = asIndex(blendIndices[i]);

            if (weight == 0) {
                continue;
            }

            auto remapIt = remap.find(index);
            if (remapIt != remap.end()) {
                blendIndices[i] = remapIt->second;
            } else {
                auto tempIt = tempMissingIndexRemap.find(index);
                if (tempIt != tempMissingIndexRemap.end()) {
                    blendIndices[i] = tempIt->second;
                }
            }
        }

        return src;
    }

    std::variant<std::monostate, std::string, ByteVec> BlendFile::remap(const VGRemap& vgRemap, const std::optional<std::string>& fixedBlendFile, bool remapMissingIndices) {
        bool blendIsFile = std::holds_alternative<std::string>(getSrc());

        if (vgRemap.getRemap().empty() && blendIsFile) {
            return std::monostate{};
        }
        if (vgRemap.getRemap().empty()) {
            return std::get<ByteVec>(getSrc());
        }

        std::vector<Filter> filters = {
            [&vgRemap, remapMissingIndices](const BufLineData& data, long long, double, long long) {
                return remapIndices(data, vgRemap, remapMissingIndices);
            }
        };

        FixResult result = fix(fixedBlendFile, filters);
        return std::visit([](auto&& value) -> std::variant<std::monostate, std::string, ByteVec> { return value; }, result);
    }
}
