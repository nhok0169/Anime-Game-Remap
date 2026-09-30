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

#include "AGRemapCore/model/buffers/InnerLayerOutline.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace AGRemapCore {
    namespace {
        using Vec3 = InnerLayerOutline::Vec3;
        using Tri = std::array<unsigned long long, 3>;

        Vec3 sub(const Vec3& a, const Vec3& b) { return {a[0] - b[0], a[1] - b[1], a[2] - b[2]}; }
        float dot(const Vec3& a, const Vec3& b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
        Vec3 cross(const Vec3& a, const Vec3& b) {
            return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
        }

        // Triangles bucketed by every grid cell their bounding box touches: a ray of length `reach` only has
        // to look at the cells its own box touches. The cell is the reach itself, so that is at most 2 x 2 x 2.
        class TriangleGrid {
            public:
                TriangleGrid(const std::vector<Vec3>& positions, const std::vector<const Tri*>& tris, float cell):
                    cell_(cell > 0.0f ? cell : 1.0f) {
                    for (std::size_t t = 0; t < tris.size(); ++t) {
                        Vec3 lo = positions[(*tris[t])[0]];
                        Vec3 hi = lo;
                        for (std::size_t k = 1; k < 3; ++k) {
                            const Vec3& p = positions[(*tris[t])[k]];
                            for (std::size_t a = 0; a < 3; ++a) {
                                lo[a] = std::min(lo[a], p[a]);
                                hi[a] = std::max(hi[a], p[a]);
                            }
                        }
                        forCells(lo, hi, [&](long long key) { cells_[key].push_back(t); });
                    }
                }

                template <typename Fn>
                void forCells(const Vec3& lo, const Vec3& hi, Fn fn) const {
                    const long long x0 = coord(lo[0]), x1 = coord(hi[0]);
                    const long long y0 = coord(lo[1]), y1 = coord(hi[1]);
                    const long long z0 = coord(lo[2]), z1 = coord(hi[2]);
                    for (long long x = x0; x <= x1; ++x) {
                        for (long long y = y0; y <= y1; ++y) {
                            for (long long z = z0; z <= z1; ++z) {
                                fn(key(x, y, z));
                            }
                        }
                    }
                }

                const std::vector<std::size_t>* at(long long key) const {
                    auto it = cells_.find(key);
                    return it == cells_.end() ? nullptr : &it->second;
                }

            private:
                long long coord(float v) const { return static_cast<long long>(std::floor(v / cell_)); }
                static long long key(long long x, long long y, long long z) {
                    // 21 bits a coordinate: a model is a few units across and the cell a tenth of one
                    const long long mask = (1LL << 21) - 1;
                    return ((x & mask) << 42) | ((y & mask) << 21) | (z & mask);
                }

                float cell_;
                std::unordered_map<long long, std::vector<std::size_t>> cells_;
        };
    }


    std::vector<bool> InnerLayerOutline::covered(const std::vector<Vec3>& positions, const std::vector<Vec3>& normals,
                                                 const std::vector<const Triangles*>& occluders, const std::vector<const Triangles*>& targets) const {
        const std::size_t n = positions.size();
        std::vector<bool> result(n, false);

        std::vector<const Tri*> tris;
        for (const Triangles* list : occluders) {
            for (const Tri& t : *list) {
                if (t[0] < n && t[1] < n && t[2] < n) {
                    tris.push_back(&t);
                }
            }
        }

        std::vector<bool> asked(n, false);
        for (const Triangles* list : targets) {
            for (const Tri& t : *list) {
                for (unsigned long long v : t) {
                    if (v < n) {
                        asked[v] = true;
                    }
                }
            }
        }

        const TriangleGrid grid(positions, tris, reach);
        std::vector<std::size_t> candidates;
        std::vector<long long> seenAt;

        for (std::size_t v = 0; v < n; ++v) {
            if (!asked[v] || v >= normals.size()) {
                continue;
            }
            Vec3 d = normals[v];
            const float len = std::sqrt(dot(d, d));
            if (len <= 1e-12f) {
                continue;
            }
            d = {d[0] / len, d[1] / len, d[2] / len};
            const Vec3& o = positions[v];
            const Vec3 end{o[0] + d[0] * reach, o[1] + d[1] * reach, o[2] + d[2] * reach};
            const Vec3 lo{std::min(o[0], end[0]), std::min(o[1], end[1]), std::min(o[2], end[2])};
            const Vec3 hi{std::max(o[0], end[0]), std::max(o[1], end[1]), std::max(o[2], end[2])};

            candidates.clear();
            grid.forCells(lo, hi, [&](long long key) {
                if (const std::vector<std::size_t>* in = grid.at(key)) {
                    candidates.insert(candidates.end(), in->begin(), in->end());
                }
            });
            std::sort(candidates.begin(), candidates.end());
            candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

            for (std::size_t c : candidates) {
                const Tri& t = *tris[c];
                if (t[0] == v || t[1] == v || t[2] == v) {
                    continue;
                }
                // Moller-Trumbore
                const Vec3& a = positions[t[0]];
                const Vec3 e1 = sub(positions[t[1]], a);
                const Vec3 e2 = sub(positions[t[2]], a);
                const Vec3 h = cross(d, e2);
                const float det = dot(e1, h);
                if (std::fabs(det) <= 1e-14f) {
                    continue;
                }
                const float inv = 1.0f / det;
                const Vec3 s = sub(o, a);
                const float u = dot(s, h) * inv;
                if (u < 0.0f || u > 1.0f) {
                    continue;
                }
                const Vec3 q = cross(s, e1);
                const float w = dot(d, q) * inv;
                if (w < 0.0f || u + w > 1.0f) {
                    continue;
                }
                const float dist = dot(e2, q) * inv;
                if (dist > 1e-5f && dist < reach) {
                    result[v] = true;
                    break;
                }
            }
        }

        return result;
    }


    std::vector<bool> InnerLayerOutline::backed(const std::vector<Vec3>& positions, const std::vector<Vec3>& normals,
                                                const std::vector<const Triangles*>& occluders, const Triangles& targets, float reach) {
        const std::size_t n = positions.size();
        std::vector<bool> result(targets.size(), false);
        if (reach <= 0.0f || normals.size() < n) {
            return result;
        }

        std::vector<const Tri*> tris;
        for (const Triangles* list : occluders) {
            for (const Tri& t : *list) {
                if (t[0] < n && t[1] < n && t[2] < n) {
                    tris.push_back(&t);
                }
            }
        }

        const TriangleGrid grid(positions, tris, reach);
        std::vector<std::size_t> candidates;

        for (std::size_t k = 0; k < targets.size(); ++k) {
            const Tri& self = targets[k];
            if (self[0] >= n || self[1] >= n || self[2] >= n) {
                continue;
            }

            // From the centroid, against the corners' mean normal
            Vec3 o{0.0f, 0.0f, 0.0f};
            Vec3 m{0.0f, 0.0f, 0.0f};
            for (unsigned long long v : self) {
                for (std::size_t a = 0; a < 3; ++a) {
                    o[a] += positions[v][a] / 3.0f;
                    m[a] += normals[v][a];
                }
            }
            const float len = std::sqrt(dot(m, m));
            if (len <= 1e-12f) {
                continue;
            }
            m = {m[0] / len, m[1] / len, m[2] / len};
            const Vec3 d{-m[0], -m[1], -m[2]};

            // From the centroid and from a point near each corner (a tenth of the way in): a lining under only part of
            // the triangle is missed by the centroid, and the twin's corner still comes out through it
            std::array<Vec3, 4> origins{o, o, o, o};
            for (std::size_t c = 0; c < 3; ++c) {
                for (std::size_t a = 0; a < 3; ++a) {
                    origins[c + 1][a] = positions[self[c]][a] * 0.9f + o[a] * 0.1f;
                }
            }
            Vec3 lo = origins[0];
            Vec3 hi = origins[0];
            for (const Vec3& from : origins) {
                for (std::size_t a = 0; a < 3; ++a) {
                    lo[a] = std::min({lo[a], from[a], from[a] + d[a] * reach});
                    hi[a] = std::max({hi[a], from[a], from[a] + d[a] * reach});
                }
            }

            candidates.clear();
            grid.forCells(lo, hi, [&](long long key) {
                if (const std::vector<std::size_t>* in = grid.at(key)) {
                    candidates.insert(candidates.end(), in->begin(), in->end());
                }
            });
            std::sort(candidates.begin(), candidates.end());
            candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

            for (std::size_t c : candidates) {
                if (result[k]) {
                    break;
                }
                if (tris[c] == &self) {
                    continue;
                }
                const Tri& t = *tris[c];
                const Vec3& a = positions[t[0]];
                const Vec3 e1 = sub(positions[t[1]], a);
                const Vec3 e2 = sub(positions[t[2]], a);

                // Only a surface facing the other way backs this one
                if (dot(cross(e1, e2), m) >= 0.0f) {
                    continue;
                }

                // Moller-Trumbore, from each origin
                const Vec3 h = cross(d, e2);
                const float det = dot(e1, h);
                if (std::fabs(det) <= 1e-14f) {
                    continue;
                }
                const float inv = 1.0f / det;
                for (const Vec3& from : origins) {
                    const Vec3 s = sub(from, a);
                    const float u = dot(s, h) * inv;
                    if (u < 0.0f || u > 1.0f) {
                        continue;
                    }
                    const Vec3 q = cross(s, e1);
                    const float w = dot(d, q) * inv;
                    if (w < 0.0f || u + w > 1.0f) {
                        continue;
                    }
                    const float dist = dot(e2, q) * inv;
                    if (dist > 1e-6f && dist < reach) {
                        result[k] = true;
                        break;
                    }
                }
            }
        }

        return result;
    }


    std::vector<bool> InnerLayerOutline::find(const std::vector<Vec3>& positions, const std::vector<Vec3>& normals,
                                              const std::vector<const Triangles*>& occluders, const std::vector<const Triangles*>& targets) const {
        const std::size_t n = positions.size();
        const std::vector<bool> cover = covered(positions, normals, occluders, targets);

        // The vertical axis: the (x, z) centre of every vertex a target triangle uses
        double cx = 0.0, cz = 0.0;
        std::size_t count = 0;
        {
            std::vector<bool> used(n, false);
            for (const Triangles* list : targets) {
                for (const Tri& t : *list) {
                    for (unsigned long long v : t) {
                        if (v < n && !used[v]) {
                            used[v] = true;
                            cx += positions[v][0];
                            cz += positions[v][2];
                            ++count;
                        }
                    }
                }
            }
        }
        if (count > 0) {
            cx /= static_cast<double>(count);
            cz /= static_cast<double>(count);
        }

        std::vector<bool> result(n, false);
        for (const Triangles* list : targets) {
            for (const Tri& t : *list) {
                if (t[0] >= n || t[1] >= n || t[2] >= n) {
                    continue;
                }
                bool inner = (static_cast<int>(cover[t[0]]) + static_cast<int>(cover[t[1]]) + static_cast<int>(cover[t[2]])) >= 2;

                if (!inner && facingAxis) {
                    const Vec3& a = positions[t[0]];
                    Vec3 fn = cross(sub(positions[t[1]], a), sub(positions[t[2]], a));
                    const float fl = std::sqrt(dot(fn, fn));
                    const float mx = (a[0] + positions[t[1]][0] + positions[t[2]][0]) / 3.0f - static_cast<float>(cx);
                    const float mz = (a[2] + positions[t[1]][2] + positions[t[2]][2]) / 3.0f - static_cast<float>(cz);
                    const float ol = std::sqrt(mx * mx + mz * mz);
                    if (fl > 1e-20f && ol > 1e-9f) {
                        inner = (fn[0] * mx + fn[2] * mz) / (fl * ol) < -facingCos;
                    }
                }

                if (inner) {
                    result[t[0]] = result[t[1]] = result[t[2]] = true;
                }
            }
        }
        return result;
    }


    void InnerLayerOutline::readPositions(const ByteVec& buffer, std::size_t stride, std::vector<Vec3>& positions, std::vector<Vec3>& normals) {
        if (stride < 24) {
            throw std::invalid_argument("a Position.buf line of " + std::to_string(stride) + " bytes has no normal");
        }
        if (buffer.size() % stride != 0) {
            throw std::invalid_argument("a Position.buf of " + std::to_string(buffer.size()) + " bytes is not a whole number of "
                                        + std::to_string(stride) + "-byte lines");
        }
        const std::size_t n = buffer.size() / stride;
        positions.resize(n);
        normals.resize(n);
        for (std::size_t v = 0; v < n; ++v) {
            std::memcpy(positions[v].data(), buffer.data() + v * stride, 12);
            std::memcpy(normals[v].data(), buffer.data() + v * stride + 12, 12);
        }
    }
}
