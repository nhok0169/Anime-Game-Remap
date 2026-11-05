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

#include "AGRemapCore/model/buffers/DrawUVCoverage.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>
#include <stdexcept>

#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/model/files/TexcoordFile.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {

    DrawUVCoverage::DrawUVCoverage(std::vector<long long> indices, std::vector<double> us, std::vector<double> vs, std::size_t vertices):
        indices_(std::move(indices)), us_(std::move(us)), vs_(std::move(vs)), vertices_(vertices) {}

    DrawUVCoverage DrawUVCoverage::ofMesh(const std::string& indexFile, const std::string& texcoordFile, std::size_t vertices) {
        const std::size_t uvBytes = static_cast<std::size_t>(FileService::fileSize(texcoordFile).value_or(0));
        if (vertices == 0 || uvBytes % vertices != 0 || uvBytes / vertices < 4) {
            throw std::runtime_error("the texcoord buffer is not a whole number of vertices of at least one UV each");
        }

        TexcoordFile uvFile{texcoordFile, TexcoordFile::halfElements(uvBytes / vertices)};
        return DrawUVCoverage(IbFile{indexFile}.getIndices(), uvFile.column(TexcoordFile::UVKey, 0),
                              uvFile.column(TexcoordFile::UVKey, 1), vertices);
    }

    std::pair<long long, long long> DrawUVCoverage::texelOf(double u, double v, long long width, long long height) {
        const auto wrap = [](double value) { return std::isfinite(value) ? value - std::floor(value) : 0.0; };
        return {std::min(width - 1, static_cast<long long>(wrap(u) * static_cast<double>(width))),
                std::min(height - 1, static_cast<long long>(wrap(v) * static_cast<double>(height)))};
    }

    DrawUVCoverage::Count DrawUVCoverage::count(long long firstIndex, long long indexCount, long long width, long long height,
                                                const TexelTest& test) const {
        Count result;
        std::set<std::uint32_t> seen;
        for (long long i = firstIndex; i < firstIndex + indexCount; ++i) {
            if (i < 0 || static_cast<std::size_t>(i) >= indices_.size()) {
                break;
            }

            const auto vertex = static_cast<std::uint32_t>(indices_[static_cast<std::size_t>(i)]);
            if (vertex >= vertices_ || vertex >= us_.size() || vertex >= vs_.size() || !seen.insert(vertex).second) {
                continue;
            }

            const auto [x, y] = texelOf(us_[vertex], vs_[vertex], width, height);
            if (test(x, y)) {
                ++result.matched;
            }
        }

        result.vertices = seen.size();
        return result;
    }

    std::vector<std::pair<long long, long long>> DrawUVCoverage::coveredRanges(const std::vector<std::pair<long long, long long>>& ranges,
                                                                               long long width, long long height,
                                                                               const TexelTest& test, double share) const {
        std::vector<std::pair<long long, long long>> covered;
        for (const auto& range : ranges) {
            const Count counted = count(range.second, range.first, width, height, test);
            if (counted.vertices > 0 && static_cast<double>(counted.matched) > share * static_cast<double>(counted.vertices)) {
                covered.push_back(range);
            }
        }
        return covered;
    }
}
