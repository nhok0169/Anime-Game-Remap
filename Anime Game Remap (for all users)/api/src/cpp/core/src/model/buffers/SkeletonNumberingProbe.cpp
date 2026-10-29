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

#include "AGRemapCore/model/buffers/SkeletonNumberingProbe.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <memory>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/buffers/WWMIBlend.h"
#include "AGRemapCore/model/files/BlendFile.h"
#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/tools/files/FileService.h"

namespace AGRemapCore {
    namespace {
        const std::string PositionElement = "POSITION";
        constexpr std::size_t PositionStride = 12;

        std::vector<std::unique_ptr<BufElementType>> positionElements() {
            std::vector<std::unique_ptr<BufElementType>> elements;
            elements.push_back(BufElementType::repeated(PositionElement, 3, [] { return std::make_unique<BufFloat>(); },
                                                        "R32G32B32_FLOAT"));
            return elements;
        }
    }

    std::optional<SkeletonNumberingProbe::Result> SkeletonNumberingProbe::probe(
            const std::string& positionFile, const std::string& blendFile, const std::optional<std::string>& vertexVGFile,
            const std::vector<Numbering>& numberings, const std::unordered_map<long long, std::array<double, 3>>& centroids) {
        if (numberings.size() < 2 || centroids.empty()) {
            return std::nullopt;
        }

        // a vertex is a 12-byte position, and the blend's line is its ids then its weights
        const std::size_t positionBytes = static_cast<std::size_t>(FileService::fileSize(positionFile).value_or(0));
        const std::size_t blendBytes = static_cast<std::size_t>(FileService::fileSize(blendFile).value_or(0));
        const std::size_t vertices = positionBytes / PositionStride;
        if (vertices == 0 || positionBytes % PositionStride != 0 || blendBytes % vertices != 0 || (blendBytes / vertices) % 2 != 0) {
            return std::nullopt;
        }

        const std::size_t influences = blendBytes / vertices / 2;
        std::vector<std::vector<long long>> blendIds;
        std::vector<std::vector<long long>> blendWeights;
        std::array<std::vector<double>, 3> position;
        try {
            BufFile blend{blendFile, WWMIBlend::blendElements(influences)};
            blendIds = blend.rows(BlendFile::BlendIndicesKey, influences);
            blendWeights = blend.rows(BlendFile::BlendWeightKey, influences);

            BufFile positions{positionFile, positionElements()};
            for (std::size_t c = 0; c < 3; ++c) {
                position[c] = positions.column(PositionElement, c);
            }
        } catch (const std::exception&) {
            return std::nullopt;
        }

        if (blendIds.size() < vertices || position[0].size() < vertices) {
            return std::nullopt;
        }

        // the 16-bit ids, where the mod has them for every vertex
        std::vector<std::vector<long long>> wide;
        if (vertexVGFile.has_value()) {
            try {
                BufFile vg{*vertexVGFile, WWMIBlend::vertexVGElements(influences)};
                wide = vg.rows(BlendFile::BlendIndicesKey, influences);
            } catch (const std::exception&) {
                wide.clear();
            }
            if (wide.size() != vertices) {
                wide.clear();
            }
        }

        const std::size_t count = numberings.size();
        std::vector<std::vector<double>> distances(count);
        std::vector<long long> refs(count);
        for (std::size_t v = 0; v < vertices; ++v) {
            const std::vector<long long>& lineIds = blendIds[v];
            const std::vector<long long>& lineWeights = blendWeights[v];
            std::size_t heaviest = 0;
            for (std::size_t b = 1; b < influences; ++b) {
                if (lineWeights[b] > lineWeights[heaviest]) {
                    heaviest = b;
                }
            }
            if (lineWeights[heaviest] <= 127) {
                continue;                     // no bone carries over half the vertex
            }

            const long long id = wide.empty() ? lineIds[heaviest] : wide[v][heaviest];
            bool differ = false;
            bool known = true;
            for (std::size_t n = 0; n < count; ++n) {
                const auto& map = numberings[n].toReference;
                const auto at = map.find(id);
                refs[n] = at == map.end() ? id : at->second;
                differ = differ || refs[n] != refs[0];
                known = known && centroids.count(refs[n]) > 0;
            }
            if (!differ || !known) {
                continue;                     // the numberings agree here, so it says nothing
            }

            // single precision, as the buffer holds it
            const float p[3] = {static_cast<float>(position[0][v]), static_cast<float>(position[1][v]),
                                static_cast<float>(position[2][v])};
            for (std::size_t n = 0; n < count; ++n) {
                const std::array<double, 3>& c = centroids.at(refs[n]);
                distances[n].push_back(std::sqrt((p[0] - c[0]) * (p[0] - c[0]) + (p[1] - c[1]) * (p[1] - c[1])
                                                 + (p[2] - c[2]) * (p[2] - c[2])));
            }
        }

        if (distances[0].size() < MinVertices) {
            return std::nullopt;
        }

        Result result;
        result.vertices = distances[0].size();
        result.medians.resize(count);
        for (std::size_t n = 0; n < count; ++n) {
            std::vector<double>& d = distances[n];
            std::nth_element(d.begin(), d.begin() + static_cast<std::ptrdiff_t>(d.size() / 2), d.end());
            result.medians[n] = d[d.size() / 2];
            if (result.medians[n] < result.medians[result.best]) {
                result.best = n;
            }
        }
        return result;
    }
}
