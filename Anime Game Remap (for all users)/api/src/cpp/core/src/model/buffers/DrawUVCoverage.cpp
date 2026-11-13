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
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
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

    std::vector<DrawUVCoverage::Triangle> DrawUVCoverage::triangles(const std::vector<std::pair<long long, long long>>& ranges) const {
        std::vector<Triangle> result;
        for (const auto& range : ranges) {
            for (long long k = 0; k + 2 < range.first; k += 3) {
                Triangle triangle{};
                bool inMesh = true;
                for (std::size_t c = 0; c < 3; ++c) {
                    const long long at = range.second + k + static_cast<long long>(c);
                    if (at < 0 || static_cast<std::size_t>(at) >= indices_.size()
                        || indices_[static_cast<std::size_t>(at)] < 0
                        || static_cast<std::size_t>(indices_[static_cast<std::size_t>(at)]) >= vertices_) {
                        inMesh = false;
                        break;
                    }
                    triangle[c] = indices_[static_cast<std::size_t>(at)];
                }

                if (inMesh) {
                    result.push_back(triangle);
                }
            }
        }
        return result;
    }

    std::vector<std::size_t> DrawUVCoverage::islands(const std::vector<Triangle>& triangles) {
        // union-find over the vertices, then each triangle takes its first vertex's root
        std::vector<long long> vertices;
        for (const Triangle& triangle : triangles) {
            vertices.insert(vertices.end(), triangle.begin(), triangle.end());
        }
        std::sort(vertices.begin(), vertices.end());
        vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());

        const auto slot = [&vertices](long long vertex) {
            return static_cast<std::size_t>(std::lower_bound(vertices.begin(), vertices.end(), vertex) - vertices.begin());
        };

        std::vector<std::size_t> parent(vertices.size());
        std::iota(parent.begin(), parent.end(), std::size_t{0});
        const auto root = [&parent](std::size_t at) {
            while (parent[at] != at) {
                parent[at] = parent[parent[at]];
                at = parent[at];
            }
            return at;
        };

        for (const Triangle& triangle : triangles) {
            const std::size_t first = root(slot(triangle[0]));
            for (std::size_t c = 1; c < 3; ++c) {
                const std::size_t other = root(slot(triangle[c]));
                if (other != first) {
                    parent[other] = first;
                }
            }
        }

        std::vector<std::size_t> number(vertices.size(), vertices.size());
        std::size_t next = 0;
        std::vector<std::size_t> result;
        result.reserve(triangles.size());
        for (const Triangle& triangle : triangles) {
            const std::size_t at = root(slot(triangle[0]));
            if (number[at] == vertices.size()) {
                number[at] = next++;
            }
            result.push_back(number[at]);
        }
        return result;
    }

    std::vector<bool> DrawUVCoverage::region(const std::vector<Triangle>& triangles, const std::vector<bool>& accepted,
                                             long long width, long long height, int grow) const {
        if (width <= 0 || height <= 0) {
            return {};
        }

        const std::size_t w = static_cast<std::size_t>(width);
        const std::size_t texels = w * static_cast<std::size_t>(height);
        std::vector<bool> inside(texels, false);
        std::vector<bool> outside(texels, false);
        const auto finite = [](double value) { return std::isfinite(value) ? value : 0.0; };

        for (std::size_t t = 0; t < triangles.size(); ++t) {
            const Triangle& triangle = triangles[t];
            bool known = true;
            for (long long vertex : triangle) {
                if (vertex < 0 || static_cast<std::size_t>(vertex) >= us_.size() || static_cast<std::size_t>(vertex) >= vs_.size()) {
                    known = false;
                }
            }
            if (!known) {
                continue;
            }

            std::vector<bool>& into = (t < accepted.size() && accepted[t]) ? inside : outside;
            float us[3];
            float vs[3];
            for (std::size_t c = 0; c < 3; ++c) {
                us[c] = static_cast<float>(finite(us_[static_cast<std::size_t>(triangle[c])]));
                vs[c] = static_cast<float>(finite(vs_[static_cast<std::size_t>(triangle[c])]));
            }

            // the tile the triangle starts in is moved onto the texture, and the texels it covers past an edge wrap
            const float tileU = std::floor(std::min({us[0], us[1], us[2]}));
            const float tileV = std::floor(std::min({vs[0], vs[1], vs[2]}));
            float xs[3];
            float ys[3];
            for (std::size_t c = 0; c < 3; ++c) {
                xs[c] = (us[c] - tileU) * static_cast<float>(width);
                ys[c] = (vs[c] - tileV) * static_cast<float>(height);
            }

            const float det = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2]);
            if (std::fabs(det) < 1e-9f) {
                continue;
            }

            const long long x0 = std::max(0LL, static_cast<long long>(std::floor(std::min({xs[0], xs[1], xs[2]}))));
            const long long x1 = std::min(2 * width - 1, static_cast<long long>(std::ceil(std::max({xs[0], xs[1], xs[2]}))));
            const long long y0 = std::max(0LL, static_cast<long long>(std::floor(std::min({ys[0], ys[1], ys[2]}))));
            const long long y1 = std::min(2 * height - 1, static_cast<long long>(std::ceil(std::max({ys[0], ys[1], ys[2]}))));
            for (long long y = y0; y <= y1; ++y) {
                for (long long x = x0; x <= x1; ++x) {
                    const float px = static_cast<float>(x) + 0.5f;
                    const float py = static_cast<float>(y) + 0.5f;
                    const float w0 = ((ys[1] - ys[2]) * (px - xs[2]) + (xs[2] - xs[1]) * (py - ys[2])) / det;
                    const float w1 = ((ys[2] - ys[0]) * (px - xs[2]) + (xs[0] - xs[2]) * (py - ys[2])) / det;
                    if (w0 >= -1e-6f && w1 >= -1e-6f && (w0 + w1) <= 1.0f + 1e-6f) {
                        into[static_cast<std::size_t>(y % height) * w + static_cast<std::size_t>(x % width)] = true;
                    }
                }
            }
        }

        // a texel another triangle paints too belongs to neither part
        for (std::size_t i = 0; i < texels; ++i) {
            if (inside[i] && outside[i]) {
                inside[i] = false;
            }
        }

        // grow into the gutter only: growing into a neighbouring part would paint it
        for (int pass = 0; pass < grow; ++pass) {
            std::vector<bool> grown = inside;
            for (long long y = 0; y < height; ++y) {
                for (long long x = 0; x < width; ++x) {
                    const std::size_t i = static_cast<std::size_t>(y) * w + static_cast<std::size_t>(x);
                    if (inside[i] || outside[i]) {
                        continue;
                    }

                    if ((x > 0 && inside[i - 1]) || (x + 1 < width && inside[i + 1]) || (y > 0 && inside[i - w])
                        || (y + 1 < height && inside[i + w])) {
                        grown[i] = true;
                    }
                }
            }
            inside.swap(grown);
        }

        return inside;
    }
}
