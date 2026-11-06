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

#include "AGRemapCore/model/buffers/WWMIBlend.h"

#include <algorithm>
#include <functional>
#include <set>
#include <stdexcept>
#include <unordered_map>

#include "AGRemapCore/model/buffers/BufInt.h"
#include "AGRemapCore/model/buffers/BufValue.h"
#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/model/files/BlendFile.h"
#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/tools/files/FileService.h"

#include <fstream>
#include <iterator>


namespace AGRemapCore {

    namespace {
        // What one weighted influence of a blend line becomes -- std::nullopt to leave the id alone.
        // Handed the vertex, which slot of that vertex this is, and the id the buffer holds.
        using InfluenceRemap = std::function<std::optional<long long>(std::size_t vertex, std::size_t slot, long long id)>;

        // A BufFile::Filter that rewrites the ids of every WEIGHTED influence of each blend line,
        // leaving a weight-zero slot alone as BlendFile::remapIndices does
        BufFile::Filter indexFilter(std::size_t vertices, std::size_t influences, InfluenceRemap remap) {
            return [vertices, influences, remap = std::move(remap)](const BufLineData& line, long long,
                                                                    double index, long long) {
                BufLineData out = line;
                const auto vertex = static_cast<std::size_t>(index);
                if (vertex >= vertices) {
                    return out;
                }

                const auto ids = out.find(BlendFile::BlendIndicesKey);
                const auto weights = out.find(BlendFile::BlendWeightKey);
                if (ids == out.end() || weights == out.end()) {
                    return out;
                }

                for (std::size_t b = 0; b < influences && b < ids->second.size() && b < weights->second.size(); ++b) {
                    if (bufValueAsFloat(weights->second[b]) == 0) {
                        continue;                                 // a weight-zero slot
                    }

                    const std::optional<long long> to = remap(vertex, b, bufValueAsInt(ids->second[b]));
                    if (to.has_value()) {
                        ids->second[b] = static_cast<unsigned long long>(*to);
                    }
                }

                return out;
            };
        }

        // The layout, or the reason there is none
        WWMIBlend::Layout requireLayout(const std::string& blendPath, const std::string& positionPath) {
            const std::optional<WWMIBlend::Layout> layout = WWMIBlend::layoutOf(blendPath, positionPath);
            if (!layout.has_value()) {
                throw std::runtime_error("its Blend.buf and Position.buf do not agree on a vertex count and an"
                                         " even number of influences per vertex");
            }

            return *layout;
        }

        std::string fileName(const std::string& path) {
            return FileService::pathToStr(FileService::strToPath(path).filename());
        }
    }


    std::vector<std::unique_ptr<BufElementType>> WWMIBlend::blendElements(std::size_t influences) {
        // The four-wide name is what a dump of a four-influence character writes; WWMI Tools' own
        // `.fmt` calls any other width `R8_UINT`
        const auto byte = [] { return std::make_unique<BufUnSignedInt>("UnsignedInt8", 1, false); };
        const std::string format = influences == 4 ? "R8G8B8A8_UINT" : "R8_UINT";

        std::vector<std::unique_ptr<BufElementType>> elements;
        elements.push_back(BufElementType::repeated(BlendFile::BlendIndicesKey, influences, byte, format));
        elements.push_back(BufElementType::repeated(BlendFile::BlendWeightKey, influences, byte, format));
        return elements;
    }

    std::vector<std::unique_ptr<BufElementType>> WWMIBlend::vertexVGElements(std::size_t influences) {
        std::vector<std::unique_ptr<BufElementType>> elements;
        elements.push_back(BufElementType::repeated(
            BlendFile::BlendIndicesKey, influences,
            [] { return std::make_unique<BufUnSignedInt>("UnsignedInt16", 2, false); }, "R16_UINT"));

        return elements;
    }

    std::optional<WWMIBlend::Layout> WWMIBlend::layoutOf(const std::string& blendPath, const std::string& positionPath) {
        const std::optional<std::uintmax_t> positionSize = FileService::fileSize(positionPath);
        if (!positionSize.has_value() || *positionSize < PositionStride) {
            return std::nullopt;
        }

        const auto vertices = static_cast<std::uintmax_t>(*positionSize / PositionStride);
        const std::optional<std::uintmax_t> blendSize = FileService::fileSize(blendPath);
        if (vertices == 0 || !blendSize.has_value() || *blendSize == 0 || *blendSize % vertices != 0) {
            return std::nullopt;
        }

        const std::uintmax_t stride = *blendSize / vertices;
        if (stride < 2 || stride % 2 != 0) {
            return std::nullopt;
        }

        return Layout{static_cast<std::size_t>(vertices), static_cast<std::size_t>(stride / 2)};
    }

    std::size_t WWMIBlend::influencesOf(const std::string& blendPath, long long vertices,
                                        const std::string& positionPath, std::size_t fallback) {
        // The files win over the declaration: a mod may declare a `$mesh_vertex_count` that is not
        // its buffers' (one Lynae mod declares 240090 for 240078 real vertices)
        const std::optional<Layout> layout = layoutOf(blendPath, positionPath);
        if (layout.has_value()) {
            return layout->influences;
        }

        if (vertices <= 0) {
            return fallback;
        }

        const std::optional<std::uintmax_t> blendSize = FileService::fileSize(blendPath);
        if (!blendSize.has_value() || *blendSize == 0 || *blendSize % static_cast<std::uintmax_t>(vertices) != 0) {
            return fallback;
        }

        const std::uintmax_t stride = *blendSize / static_cast<std::uintmax_t>(vertices);
        return (stride < 2 || stride % 2 != 0) ? fallback : static_cast<std::size_t>(stride / 2);
    }

    std::vector<int> WWMIBlend::componentOfVertex(const std::string& indexPath,
                                                  const std::map<int, std::vector<std::pair<long long, long long>>>& drawRanges,
                                                  std::size_t vertices) {
        std::vector<long long> indexList;
        try {
            indexList = IbFile{indexPath}.getIndices();
        } catch (const std::exception& exception) {
            throw std::runtime_error(std::string("its index buffer could not be read: ") + exception.what());
        }

        const std::size_t indexCount = indexList.size();
        std::vector<int> componentOf(vertices, -1);
        for (const auto& entry : drawRanges) {
            for (const auto& range : entry.second) {
                for (long long k = range.second; k < range.second + range.first && k >= 0; ++k) {
                    if (static_cast<std::size_t>(k) >= indexCount) {
                        break;
                    }

                    const auto vertex = static_cast<std::uint32_t>(indexList[static_cast<std::size_t>(k)]);
                    if (vertex < vertices) {
                        componentOf[vertex] = entry.first;
                    }
                }
            }
        }

        return componentOf;
    }

    void WWMIBlend::remapThroughVertexVG(const std::string& blendPath, const std::string& fixedPath,
                                         const std::string& vertexVGPath, const std::string& positionPath,
                                         const VGRemap& row) {
        const Layout shape = requireLayout(blendPath, positionPath);
        const std::size_t vertices = shape.vertices;
        const std::size_t influences = shape.influences;

        std::vector<std::vector<long long>> trueIds;
        try {
            BufFile vgFile{vertexVGPath, vertexVGElements(influences)};
            if (!vgFile.isValid()) {
                throw std::runtime_error("its BlendRemapVertexVG.buf could not be read");
            }

            trueIds = vgFile.rows(BlendFile::BlendIndicesKey, influences);
        } catch (const std::exception& exception) {
            throw std::runtime_error(std::string("its BlendRemapVertexVG.buf could not be read: ") + exception.what());
        }

        if (trueIds.size() != vertices) {
            throw std::runtime_error("its BlendRemapVertexVG.buf holds a different number of vertices than its Position.buf");
        }

        // The id to remap comes from BlendRemapVertexVG.buf, not from the Blend.buf line: past 256
        // bones the line's 8-bit id is an index into the remap, and #trueIds holds what it names
        const std::unordered_map<long long, long long>& remap = row.getRemap();
        const BufFile::Filter filter = indexFilter(
            vertices, influences,
            [&trueIds, &remap](std::size_t vertex, std::size_t slot, long long) -> std::optional<long long> {
                const auto target = remap.find(static_cast<long long>(trueIds[vertex][slot]));
                return target == remap.end() ? std::nullopt : std::optional<long long>(target->second);
            });

        try {
            BufFile blend{blendPath, blendElements(influences)};
            if (!blend.isValid()) {
                throw std::runtime_error("its Blend.buf could not be read");
            }

            blend.fix(fixedPath, {filter});
        } catch (const std::exception& exception) {
            throw std::runtime_error(std::string("its Blend.buf could not be read: ") + exception.what());
        }
    }

    void WWMIBlend::liftLegacy(const std::string& blendPath, const std::string& fixedPath,
                               const std::string& positionPath, const LegacyLift& legacy, const VGRemap& row) {
        const Layout shape = requireLayout(blendPath, positionPath);
        const std::size_t vertices = shape.vertices;
        const std::size_t influences = shape.influences;

        const std::vector<int> componentOf = componentOfVertex(legacy.indexPath, legacy.drawRanges, vertices);

        BufFile blend{blendPath, blendElements(influences)};
        if (!blend.isValid()) {
            throw std::runtime_error("its Blend.buf could not be read for the legacy lift");
        }

        // Here the line's id IS a component-local vertex group, so it is carried through that
        // component's own vg_map before the remap row
        const std::unordered_map<long long, long long>& remap = row.getRemap();
        const std::map<int, std::vector<int>>& vgMaps = legacy.vgMaps;
        const BufFile::Filter lift = indexFilter(
            vertices, influences,
            [&componentOf, &vgMaps, &remap](std::size_t vertex, std::size_t, long long id) -> std::optional<long long> {
                const int component = componentOf[vertex];
                auto vgMap = vgMaps.find(component);
                if (component < 0 || vgMap == vgMaps.end()) {
                    return std::nullopt;
                }

                const auto local = static_cast<std::size_t>(id);
                if (local >= vgMap->second.size()) {
                    return std::nullopt;
                }

                auto target = remap.find(vgMap->second[local]);
                return target == remap.end() ? std::nullopt : std::optional<long long>(target->second);
            });

        try {
            blend.fix(fixedPath, {lift});
        } catch (const std::exception& exception) {
            throw std::runtime_error(std::string("its lifted Blend.buf could not be written: ") + exception.what());
        }
    }

    void WWMIBlend::writeRemap(const std::string& blendPath, const std::string& fixedPath,
                               const std::string& srcVertexVGPath, const std::string& positionPath,
                               const VGRemap& row, const RemapFiles& out, const std::optional<LegacyLift>& legacy) {
        const Layout shape = requireLayout(blendPath, positionPath);
        const std::size_t vertices = shape.vertices;
        const std::size_t influences = shape.influences;

        std::vector<std::vector<long long>> ids;                  // per vertex, `influences` of them
        std::vector<std::vector<long long>> weights;
        try {
            BufFile blendFile{blendPath, blendElements(influences)};
            if (!blendFile.isValid()) {
                throw std::runtime_error("its Blend.buf could not be read");
            }

            ids = blendFile.rows(BlendFile::BlendIndicesKey, influences);
            weights = blendFile.rows(BlendFile::BlendWeightKey, influences);
        } catch (const std::exception& exception) {
            throw std::runtime_error(std::string("its Blend.buf could not be read: ") + exception.what());
        }

        if (ids.size() != vertices) {
            throw std::runtime_error("its Blend.buf holds a different number of vertices than its Position.buf");
        }

        // The TRUE ids: the mod's own 16-bit ones when it carries a blend remap of its own,
        // otherwise Blend.buf's, which are the whole truth for a source under 256 bones
        std::vector<std::uint16_t> trueIds(vertices * influences, 0);
        bool haveVertexVG = false;
        if (!srcVertexVGPath.empty()) {
            try {
                BufFile vgFile{srcVertexVGPath, vertexVGElements(influences)};
                if (vgFile.isValid()) {
                    const std::vector<std::vector<long long>> rows = vgFile.rows(BlendFile::BlendIndicesKey, influences);
                    haveVertexVG = rows.size() * influences == trueIds.size();
                    if (haveVertexVG) {
                        for (std::size_t vertex = 0; vertex < rows.size(); ++vertex) {
                            for (std::size_t b = 0; b < influences; ++b) {
                                trueIds[vertex * influences + b] = static_cast<std::uint16_t>(rows[vertex][b]);
                            }
                        }
                    }
                }
            } catch (const std::exception&) {
                haveVertexVG = false;                  // its own ids are optional; Blend.buf's stand
            }
        }

        if (!haveVertexVG) {
            for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
                for (std::size_t b = 0; b < influences; ++b) {
                    trueIds[vertex * influences + b] = static_cast<std::uint16_t>(ids[vertex][b]);
                }
            }
        }

        // A legacy mod's ids are component-local, so they are lifted as liftLegacy lifts them
        // before the row
        if (!haveVertexVG && legacy.has_value()) {
            const std::vector<int> componentOf = componentOfVertex(legacy->indexPath, legacy->drawRanges, vertices);
            for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
                const auto vgMap = legacy->vgMaps.find(componentOf[vertex]);
                if (vgMap == legacy->vgMaps.end()) {
                    continue;
                }

                for (std::size_t b = 0; b < influences; ++b) {
                    const std::size_t at = vertex * influences + b;
                    if (trueIds[at] < vgMap->second.size()) {
                        trueIds[at] = static_cast<std::uint16_t>(vgMap->second[trueIds[at]]);
                    }
                }
            }
        }

        // ---- every id through the row ----
        const std::unordered_map<long long, long long>& remap = row.getRemap();
        std::vector<std::uint16_t> mapped(trueIds);
        for (std::size_t vertex = 0; vertex < vertices; ++vertex) {
            for (std::size_t b = 0; b < influences; ++b) {
                const std::size_t at = vertex * influences + b;
                if (weights[vertex][b] == 0) {
                    continue;                                     // a weight-zero slot
                }

                const auto target = remap.find(static_cast<long long>(trueIds[at]));
                if (target != remap.end() && target->second >= 0 && static_cast<std::size_t>(target->second) < RemapSize) {
                    mapped[at] = static_cast<std::uint16_t>(target->second);
                }
            }
        }

        // ---- the one map, over the ROW's distinct targets ----
        std::set<std::uint16_t> used;
        std::set<long long> unaddressable;
        for (const auto& [srcBone, dstBone] : remap) {
            (void)srcBone;
            if (dstBone < 0) {
                continue;
            }

            if (static_cast<std::size_t>(dstBone) < RemapSize) {
                used.insert(static_cast<std::uint16_t>(dstBone));
            } else {
                unaddressable.insert(dstBone);
            }
        }

        // Refused rather than truncated: a bone the remap cannot name would read local 0 -- a limb
        // pinned to the root, with nothing in the output to say so
        if (!unaddressable.empty()) {
            throw std::runtime_error("the vertex group row maps to target bone " + std::to_string(*unaddressable.begin())
                                     + ", which a WWMI blend remap of " + std::to_string(RemapSize)
                                     + " entries cannot name (" + std::to_string(unaddressable.size()) + " such bone(s))");
        }

        if (used.size() > RemapSize) {
            throw std::runtime_error("the vertex group row names " + std::to_string(used.size())
                                     + " distinct target bones, more than the " + std::to_string(RemapSize)
                                     + " a WWMI blend remap holds");
        }

        std::vector<std::uint16_t> forward(RemapSize, 0);
        std::vector<std::uint16_t> reverse(RemapSize, 0);
        std::uint16_t local = 0;
        for (const std::uint16_t merged : used) {
            forward[local] = merged;
            reverse[merged] = local;
            ++local;
        }

        // ---- the blend, with the LOCAL ids the remap addresses ----
        // What WWMI's BlendRemapper writes there at run time (`RemappedBlend[v*16+i] =
        // ReverseMap[FullRangeVG[v*8+i]]`), so the bytes are right with or without the pass. A bone
        // the row never names keeps the SOURCE's id, which the reverse map answers 0 for -- the
        // same answer the compute pass gives
        const BufFile::Filter writeIds =
            [&mapped, &reverse, vertices, influences](const BufLineData& line, long long, double index, long long) {
                BufLineData result = line;
                const auto vertex = static_cast<std::size_t>(index);
                const auto found = result.find(BlendFile::BlendIndicesKey);
                if (vertex >= vertices || found == result.end()) {
                    return result;
                }

                for (std::size_t b = 0; b < influences && b < found->second.size(); ++b) {
                    const std::size_t id = mapped[vertex * influences + b];
                    found->second[b] = static_cast<unsigned long long>(id < reverse.size() ? reverse[id] : 0);
                }

                return result;
            };

        try {
            BufFile blendOut{blendPath, blendElements(influences)};
            if (!blendOut.isValid()) {
                throw std::runtime_error("its Blend.buf could not be rewritten");
            }

            blendOut.fix(fixedPath, {writeIds});
        } catch (const std::exception& exception) {
            throw std::runtime_error(std::string("its Blend.buf could not be rewritten: ") + exception.what());
        }

        // Written by the same elements that read them back
        const auto write = [](const std::string& path, const std::vector<std::uint16_t>& values, std::size_t perLine) {
            FileService::makeFolderFor(path);
            const BufFile::Filter fill = [&values, perLine](const BufLineData& line, long long, double index, long long) {
                BufLineData result = line;
                const auto at = result.find(BlendFile::BlendIndicesKey);
                const auto first = static_cast<std::size_t>(index) * perLine;
                for (std::size_t k = 0; at != result.end() && k < at->second.size() && first + k < values.size(); ++k) {
                    at->second[k] = static_cast<unsigned long long>(values[first + k]);
                }

                return result;
            };

            try {
                BufFile file{ByteVec(values.size() * 2, 0), vertexVGElements(perLine)};
                if (!file.isValid()) {
                    throw std::runtime_error("could not build " + path);
                }

                file.fix(path, {fill});
            } catch (const std::exception& exception) {
                throw std::runtime_error("could not write " + path + ": " + exception.what());
            }
        };

        write(out.vertexVG, mapped, influences);
        write(out.forward, forward, 1);
        write(out.reverse, reverse, 1);
    }

    std::vector<std::uint16_t> WWMIBlend::reverseOf(const VGRemap& row) {
        std::set<std::uint16_t> used;
        for (const auto& [srcBone, dstBone] : row.getRemap()) {
            (void)srcBone;
            if (dstBone >= 0 && static_cast<std::size_t>(dstBone) < RemapSize) {
                used.insert(static_cast<std::uint16_t>(dstBone));
            }
        }

        std::vector<std::uint16_t> reverse(RemapSize, 0);
        std::uint16_t local = 0;
        for (const std::uint16_t merged : used) {
            reverse[merged] = local++;
        }
        return reverse;
    }

    ByteVec WWMIBlend::componentBlend(const std::string& blendPath, const std::string& positionPath, long long vgOffset,
                                      const std::vector<int>& vgMap, const VGRemap& row, std::size_t outInfluences,
                                      std::set<long long>* unmapped) {
        const std::optional<Layout> layout = layoutOf(blendPath, positionPath);
        if (!layout.has_value()) {
            throw std::runtime_error("its Blend.buf and Position.buf do not agree on a vertex count and an even number of"
                                     " influences per vertex");
        }

        std::ifstream in(FileService::strToPath(blendPath), std::ios::binary);
        const ByteVec src{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
        const std::size_t influences = layout->influences;
        const std::size_t vertices = layout->vertices;
        const std::size_t width = std::max(influences, outInfluences);
        const std::unordered_map<long long, long long>& remap = row.getRemap();
        const std::vector<std::uint16_t> reverse = reverseOf(row);

        ByteVec out(vertices * width * 2, 0);
        for (std::size_t v = 0; v < vertices; ++v) {
            for (std::size_t b = 0; b < influences; ++b) {
                const std::uint8_t weight = src[v * influences * 2 + influences + b];
                out[v * width * 2 + width + b] = weight;
                if (weight == 0) {
                    continue;
                }

                const long long local = static_cast<long long>(src[v * influences * 2 + b]) - vgOffset;
                if (local < 0 || static_cast<std::size_t>(local) >= vgMap.size()) {
                    if (unmapped != nullptr) {
                        unmapped->insert(local);
                    }
                    continue;
                }

                const auto target = remap.find(vgMap[static_cast<std::size_t>(local)]);
                if (target == remap.end() || target->second < 0 || static_cast<std::size_t>(target->second) >= reverse.size()) {
                    if (unmapped != nullptr) {
                        unmapped->insert(local);
                    }
                    continue;
                }

                out[v * width * 2 + b] = static_cast<std::uint8_t>(reverse[static_cast<std::size_t>(target->second)]);
            }
        }

        return out;
    }
}
