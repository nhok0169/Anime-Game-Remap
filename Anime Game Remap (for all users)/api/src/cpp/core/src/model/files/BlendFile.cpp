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

#include "AGRemapCore/model/files/BlendFile.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <stdexcept>
#include <variant>
#include <utility>

#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/buffers/BufInt.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {

    namespace {

    }

    std::vector<std::unique_ptr<BufElementType>> BlendFile::defaultElements(std::size_t influences) {
        // The four-wide names are the ones a 3dmigoto dump writes, so they are kept for that width
        // and generated for any other.
        std::vector<std::unique_ptr<BufElementType>> elements;
        elements.push_back(BufElementType::repeated(
            BlendWeightKey, influences, [] { return std::make_unique<BufFloat>(); },
            influences == 4 ? "R32G32B32A32_FLOAT" : ""));
        elements.push_back(BufElementType::repeated(
            BlendIndicesKey, influences, [] { return std::make_unique<BufSignedInt>(); },
            influences == 4 ? "R32G32B32A32_SINT" : ""));

        return elements;
    }

    std::vector<std::unique_ptr<BufElementType>> BlendFile::byteElements(std::size_t influences) {
        // The four-wide name is what a dump of a four-influence character writes; any other width is `R8_UINT`
        const auto byte = [] { return std::make_unique<BufUnSignedInt>("UnsignedInt8", 1, false); };
        const std::string format = influences == 4 ? "R8G8B8A8_UINT" : "R8_UINT";

        std::vector<std::unique_ptr<BufElementType>> elements;
        elements.push_back(BufElementType::repeated(BlendIndicesKey, influences, byte, format));
        elements.push_back(BufElementType::repeated(BlendWeightKey, influences, byte, format));
        return elements;
    }

    std::vector<std::unique_ptr<BufElementType>> BlendFile::idElements(std::size_t count, std::size_t idBytes) {
        const std::string bits = std::to_string(idBytes * 8);
        std::vector<std::unique_ptr<BufElementType>> elements;
        elements.push_back(BufElementType::repeated(
            BlendIndicesKey, count, [&bits, idBytes] { return std::make_unique<BufUnSignedInt>("UnsignedInt" + bits, idBytes, false); },
            "R" + bits + "_UINT"));
        return elements;
    }

    std::optional<std::size_t> BlendFile::influencesOf(const std::string& blendPath, std::size_t vertices,
                                                       std::size_t bytesPerInfluence) {
        const std::optional<std::uintmax_t> size = FileService::fileSize(blendPath);
        if (vertices == 0 || bytesPerInfluence == 0 || !size.has_value() || *size == 0 || *size % vertices != 0) {
            return std::nullopt;
        }

        const std::uintmax_t perVertex = *size / vertices;
        if (perVertex < bytesPerInfluence || perVertex % bytesPerInfluence != 0) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(perVertex / bytesPerInfluence);
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
            long long index = bufValueAsInt(blendIndices[i]);
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
            double weight = bufValueAsFloat(blendWeights[i]);
            long long index = bufValueAsInt(blendIndices[i]);

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

    std::size_t BlendFile::influences() const {
        std::size_t weights = 0;
        std::size_t indices = 0;
        bool hasWeights = false;
        bool hasIndices = false;
        for (const auto& element : getElements()) {
            if (element->getName() == BlendWeightKey) {
                weights = element->getDataTypes().size();
                hasWeights = true;
            } else if (element->getName() == BlendIndicesKey) {
                indices = element->getDataTypes().size();
                hasIndices = true;
            }
        }
        return hasWeights && hasIndices ? std::min(weights, indices) : 0;
    }

    std::vector<std::unique_ptr<BufElementType>> BlendFile::elementsWithInfluences(
            const std::vector<std::unique_ptr<BufElementType>>& elements, std::size_t influences) {
        std::vector<std::unique_ptr<BufElementType>> result;
        for (const auto& element : elements) {
            auto copy = std::make_unique<BufElementType>(*element);
            if ((element->getName() == BlendWeightKey || element->getName() == BlendIndicesKey) && !element->getDataTypes().empty()) {
                std::vector<std::unique_ptr<BufDataType>> types;
                for (std::size_t k = 0; k < influences; ++k) {
                    types.push_back(element->getDataTypes().front()->clone());
                }
                copy->setDataTypes(std::move(types));
            }
            result.push_back(std::move(copy));
        }
        return result;
    }

    namespace {
        double asDouble(const BufValue& value) {
            return std::visit([](auto v) { return static_cast<double>(v); }, value);
        }

        long long asInteger(const BufValue& value) {
            return std::visit([](auto v) { return static_cast<long long>(v); }, value);
        }

        // 'value' in the same alternative as 'like'
        BufValue sameKind(const BufValue& like, double value) {
            if (std::holds_alternative<double>(like)) {
                return value;
            }
            if (std::holds_alternative<unsigned long long>(like)) {
                return static_cast<unsigned long long>(value);
            }
            return static_cast<long long>(value);
        }
    }

    BlendFile::KeptSlots BlendFile::setInfluences(std::size_t influences, const std::vector<std::vector<long long>>* bones) {
        const std::size_t from = this->influences();
        const ByteVec& data = getData();
        const std::size_t lineBytes = getBytesPerLine();
        const std::size_t vertices = lineBytes == 0 ? 0 : data.size() / lineBytes;

        bool hasWeights = false;
        bool hasIndices = false;
        bool floatWeights = false;
        unsigned long long weightCap = 0;
        for (const auto& element : getElements()) {
            if (element->getName() == BlendWeightKey && !element->getDataTypes().empty()) {
                hasWeights = true;
                const std::size_t bytes = element->getDataTypes().front()->getSize();
                weightCap = bytes >= 8 ? ~0ull : ((1ull << (8 * bytes)) - 1);
            } else if (element->getName() == BlendIndicesKey) {
                hasIndices = true;
            }
        }
        if (!hasWeights || !hasIndices) {
            throw std::runtime_error("a blend needs a " + BlendWeightKey + " and a " + BlendIndicesKey + " element to change its width");
        }
        if (bones != nullptr && bones->size() != vertices) {
            throw std::runtime_error("the bones beside the blend have " + std::to_string(bones->size()) + " vertices where the blend has "
                                     + std::to_string(vertices));
        }

        KeptSlots kept(vertices);
        if (influences == from) {
            for (auto& slots : kept) {
                for (std::size_t k = 0; k < from; ++k) {
                    slots.push_back(k);
                }
            }
            return kept;
        }

        std::vector<BufLineData> lines;
        lines.reserve(vertices);
        for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
            const ByteVec line(data.begin() + static_cast<std::ptrdiff_t>(vertex * lineBytes),
                               data.begin() + static_cast<std::ptrdiff_t>((vertex + 1) * lineBytes));
            BufLineData values = decodeLine(line);
            std::vector<BufValue>& weights = values[BlendWeightKey];
            std::vector<BufValue>& ids = values[BlendIndicesKey];
            if (vertex == 0 && !weights.empty()) {
                floatWeights = std::holds_alternative<double>(weights.front());
            }

            if (bones != nullptr && (*bones)[vertex].size() < from) {
                throw std::runtime_error("the bones beside the blend name " + std::to_string((*bones)[vertex].size())
                                         + " slots of vertex " + std::to_string(vertex) + " where the blend has " + std::to_string(from));
            }

            std::vector<BufValue> outWeights;
            std::vector<BufValue> outIds;
            std::vector<std::optional<std::size_t>>& slots = kept[vertex];
            if (influences > from) {
                for (std::size_t k = 0; k < from; ++k) {
                    outWeights.push_back(weights[k]);
                    outIds.push_back(ids[k]);
                    slots.push_back(k);
                }
                for (std::size_t k = from; k < influences; ++k) {
                    outWeights.push_back(sameKind(weights.front(), 0.0));
                    outIds.push_back(sameKind(ids.front(), 0.0));
                    slots.push_back(std::nullopt);
                }
            } else {
                const auto boneOf = [&](std::size_t k) {
                    return bones != nullptr ? (*bones)[vertex][k] : asInteger(ids[k]);
                };

                // one bone in several slots: its weight is summed onto the first of them
                std::vector<double> merged(from);
                for (std::size_t k = 0; k < from; ++k) {
                    merged[k] = floatWeights ? asDouble(weights[k]) : static_cast<double>(asInteger(weights[k]));
                }
                for (std::size_t k = 0; k < from; ++k) {
                    if (merged[k] == 0) {
                        continue;
                    }
                    for (std::size_t j = k + 1; j < from; ++j) {
                        if (merged[j] != 0 && boneOf(j) == boneOf(k)) {
                            merged[k] = floatWeights ? merged[k] + merged[j]
                                                     : static_cast<double>(std::min<unsigned long long>(
                                                           weightCap, static_cast<unsigned long long>(merged[k] + merged[j])));
                            merged[j] = 0;
                        }
                    }
                }

                std::vector<std::size_t> order(from);
                for (std::size_t k = 0; k < from; ++k) {
                    order[k] = k;
                }
                std::stable_sort(order.begin(), order.end(), [&merged](std::size_t a, std::size_t b) { return merged[a] > merged[b]; });

                double total = 0;
                double keptTotal = 0;
                for (std::size_t k = 0; k < from; ++k) {
                    total += merged[k];
                }
                for (std::size_t k = 0; k < influences; ++k) {
                    keptTotal += merged[order[k]];
                }

                // scaled so the total is kept; for integers the rounding remainder goes to the heaviest slot
                std::vector<double> scaled(influences, 0);
                if (floatWeights) {
                    for (std::size_t k = 0; k < influences; ++k) {
                        scaled[k] = keptTotal == 0 ? merged[order[k]] : merged[order[k]] * total / keptTotal;
                    }
                } else {
                    const unsigned long long t = static_cast<unsigned long long>(total);
                    const unsigned long long kt = static_cast<unsigned long long>(keptTotal);
                    long long scaledSum = 0;
                    std::vector<long long> ints(influences, 0);
                    for (std::size_t k = 0; k < influences; ++k) {
                        const unsigned long long w = static_cast<unsigned long long>(merged[order[k]]);
                        ints[k] = static_cast<long long>(kt == 0 ? w : (w * t + kt / 2) / kt);
                        scaledSum += ints[k];
                    }
                    if (kt != 0 && scaledSum != static_cast<long long>(t)) {
                        ints[0] += static_cast<long long>(t) - scaledSum;
                    }
                    for (std::size_t k = 0; k < influences; ++k) {
                        scaled[k] = static_cast<double>(std::min<unsigned long long>(weightCap, static_cast<unsigned long long>(ints[k])));
                    }
                }

                for (std::size_t k = 0; k < influences; ++k) {
                    outWeights.push_back(sameKind(weights[order[k]], scaled[k]));
                    outIds.push_back(ids[order[k]]);
                    slots.push_back(order[k]);
                }
            }

            weights = std::move(outWeights);
            ids = std::move(outIds);
            lines.push_back(std::move(values));
        }

        setElements(elementsWithInfluences(getElements(), influences));
        ByteVec out;
        for (const BufLineData& line : lines) {
            const ByteVec encoded = encodeLine(line);
            out.insert(out.end(), encoded.begin(), encoded.end());
        }
        // #getData has no setter: going back in through read() re-validates the result too
        setSrc(std::move(out));
        read();
        return kept;
    }

    ByteVec BlendFile::keepSlots(const ByteVec& ids, std::size_t idBytes, std::size_t fromSlots, const KeptSlots& kept) {
        const std::size_t fromLine = fromSlots * idBytes;
        if (ids.size() != kept.size() * fromLine) {
            throw std::runtime_error("the ids beside the blend are " + std::to_string(ids.size()) + " bytes, not "
                                     + std::to_string(fromSlots) + " ids of " + std::to_string(idBytes) + " bytes for each of "
                                     + std::to_string(kept.size()) + " vertices");
        }

        ByteVec out;
        for (std::size_t vertex = 0; vertex < kept.size(); ++vertex) {
            for (const std::optional<std::size_t>& slot : kept[vertex]) {
                for (std::size_t b = 0; b < idBytes; ++b) {
                    out.push_back(slot.has_value() ? ids[vertex * fromLine + *slot * idBytes + b] : static_cast<unsigned char>(0));
                }
            }
        }
        return out;
    }
}
