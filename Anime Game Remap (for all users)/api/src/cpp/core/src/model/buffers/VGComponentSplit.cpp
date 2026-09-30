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

#include "AGRemapCore/model/buffers/VGComponentSplit.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/model/files/BlendFile.h"
#include "AGRemapCore/model/files/IbFile.h"

namespace AGRemapCore {
    namespace {

        // The most common value, ties going to the one seen first -- Counter.most_common(1)
        std::optional<long long> mostCommon(const std::vector<long long>& values) {
            std::unordered_map<long long, std::size_t> counts;
            std::vector<long long> order;
            for (long long value : values) {
                if (counts[value]++ == 0) {
                    order.push_back(value);
                }
            }

            std::optional<long long> best;
            std::size_t bestCount = 0;
            for (long long value : order) {
                if (counts[value] > bestCount) {
                    bestCount = counts[value];
                    best = value;
                }
            }
            return best;
        }

        void appendLittleEndian(ByteVec& out, std::uint32_t value) {
            out.push_back(static_cast<std::uint8_t>(value & 0xFFu));
            out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
            out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFFu));
            out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFFu));
        }
    }


    VGComponentSplit::VGComponentSplit(Weights weights, Indices indices, std::vector<Triangles> ibs, std::vector<VGComponentSpec> specs):
        weights_(std::move(weights)), indices_(std::move(indices)), ibs_(std::move(ibs)), specs_(std::move(specs)) {
        if (weights_.size() != indices_.size()) {
            throw std::invalid_argument("a Blend.buf has as many weight lines as index lines");
        }

        totals_.reserve(weights_.size());
        for (const auto& w : weights_) {
            // The same float32 accumulation numpy does over a 4-wide row
            float total = 0.0f;
            for (double v : w) {
                total += static_cast<float>(v);
            }
            totals_.push_back(static_cast<double>(total));
        }
    }


    std::pair<VGComponentSplit::Weights, VGComponentSplit::Indices> VGComponentSplit::readBlend(BlendFile& blend) {
        blend.read();
        std::vector<BufFile::BufColumnData> columns = blend.decodeAll();

        std::size_t lines = 0;
        for (const auto& column : columns) {
            lines = std::max(lines, std::visit([](auto&& values) { return values.size(); }, column.values));
        }

        Weights weights(lines, std::array<double, 4>{0.0, 0.0, 0.0, 0.0});
        Indices indices(lines, std::array<long long, 4>{0, 0, 0, 0});
        for (const auto& column : columns) {
            if (column.valueInd >= 4) {
                continue;
            }

            std::visit([&](auto&& values) {
                for (std::size_t i = 0; i < values.size(); ++i) {
                    if (column.elementKey == BlendFile::BlendWeightKey) {
                        weights[i][column.valueInd] = static_cast<double>(values[i]);
                    } else if (column.elementKey == BlendFile::BlendIndicesKey) {
                        indices[i][column.valueInd] = static_cast<long long>(values[i]);
                    }
                }
            }, column.values);
        }

        return {std::move(weights), std::move(indices)};
    }


    VGComponentSplit::Triangles VGComponentSplit::readIb(IbFile& ib) {
        ib.read();
        std::vector<BufFile::BufColumnData> columns = ib.decodeAll();

        std::size_t lines = 0;
        for (const auto& column : columns) {
            lines = std::max(lines, std::visit([](auto&& values) { return values.size(); }, column.values));
        }

        Triangles triangles(lines, std::array<unsigned long long, 3>{0, 0, 0});
        for (const auto& column : columns) {
            if (column.elementKey != IbFile::TriangleBufElementKey || column.valueInd >= IbFile::VerticesPerTriangle) {
                continue;
            }

            std::visit([&](auto&& values) {
                for (std::size_t i = 0; i < values.size(); ++i) {
                    triangles[i][column.valueInd] = static_cast<unsigned long long>(values[i]);
                }
            }, column.values);
        }

        return triangles;
    }


    ByteVec VGComponentSplit::encodeBlend(const Weights& weights, const Indices& indices) {
        if (weights.size() != indices.size()) {
            throw std::invalid_argument("a Blend.buf has as many weight lines as index lines");
        }

        ByteVec out;
        out.reserve(weights.size() * 32);
        for (std::size_t i = 0; i < weights.size(); ++i) {
            for (double value : weights[i]) {
                float f = static_cast<float>(value);
                std::uint32_t bits = 0;
                std::memcpy(&bits, &f, sizeof(bits));
                appendLittleEndian(out, bits);
            }
            for (long long value : indices[i]) {
                appendLittleEndian(out, static_cast<std::uint32_t>(static_cast<std::int32_t>(value)));
            }
        }
        return out;
    }


    ByteVec VGComponentSplit::encodeIb(const Triangles& triangles) {
        ByteVec out;
        out.reserve(triangles.size() * 12);
        for (const auto& triangle : triangles) {
            for (unsigned long long value : triangle) {
                appendLittleEndian(out, static_cast<std::uint32_t>(value));
            }
        }
        return out;
    }


    ByteVec VGComponentSplit::keepLines(const ByteVec& src, std::size_t bytesPerLine, const std::vector<std::size_t>& lines) {
        if (bytesPerLine == 0) {
            throw std::invalid_argument("a buffer line cannot be 0 bytes");
        }

        ByteVec out;
        out.reserve(lines.size() * bytesPerLine);
        for (std::size_t line : lines) {
            std::size_t start = line * bytesPerLine;
            if (start + bytesPerLine > src.size()) {
                throw std::invalid_argument("line " + std::to_string(line) + " is past the end of a buffer of "
                                            + std::to_string(src.size() / bytesPerLine) + " lines");
            }
            out.insert(out.end(), src.begin() + static_cast<std::ptrdiff_t>(start),
                       src.begin() + static_cast<std::ptrdiff_t>(start + bytesPerLine));
        }
        return out;
    }


    std::size_t VGComponentSplit::vertexCount() const {
        return weights_.size();
    }


    const std::vector<VGComponentSpec>& VGComponentSplit::specs() const {
        return specs_;
    }


    // Per vertex slot: the group's bone in this component (or -1), and whether the slot counts
    // as the component's -- ComponentSplit.py's 'mapped' / 'inComponent'
    VGComponentSplit::Membership VGComponentSplit::membership(const VGComponentSpec& spec, bool withSecondary) const {
        const std::unordered_map<long long, long long>& forward = spec.remap.getRemap();
        Membership result;
        result.mapped.assign(weights_.size(), std::array<long long, 4>{-1, -1, -1, -1});
        result.inComponent.assign(weights_.size(), std::array<bool, 4>{false, false, false, false});

        for (std::size_t v = 0; v < weights_.size(); ++v) {
            for (std::size_t k = 0; k < 4; ++k) {
                auto it = forward.find(indices_[v][k]);
                if (it != forward.end()) {
                    result.mapped[v][k] = it->second;
                    result.inComponent[v][k] = weights_[v][k] > 0.0;
                }
            }
        }

        if (!withSecondary || spec.secondary.empty()) {
            return result;
        }

        // Honoured only where the vertex already carries one of the component's own bones, and
        // never for a group the forward remap already covers
        for (std::size_t v = 0; v < weights_.size(); ++v) {
            bool anchored = false;
            for (bool in : result.inComponent[v]) {
                anchored = anchored || in;
            }
            if (!anchored) {
                continue;
            }

            for (std::size_t k = 0; k < 4; ++k) {
                if (result.inComponent[v][k] || weights_[v][k] <= 0.0 || forward.contains(indices_[v][k])) {
                    continue;
                }
                auto it = spec.secondary.find(indices_[v][k]);
                if (it != spec.secondary.end()) {
                    result.mapped[v][k] = it->second;
                    result.inComponent[v][k] = true;
                }
            }
        }

        return result;
    }


    // Per vertex, its total weight on the component's own (forward) bones -- '_componentShares'
    std::vector<double> VGComponentSplit::forwardShares(const VGComponentSpec& spec) const {
        const std::unordered_map<long long, long long>& forward = spec.remap.getRemap();
        std::vector<double> shares(weights_.size(), 0.0);
        for (std::size_t v = 0; v < weights_.size(); ++v) {
            float share = 0.0f;
            for (std::size_t k = 0; k < 4; ++k) {
                if (weights_[v][k] > 0.0 && forward.contains(indices_[v][k])) {
                    share += static_cast<float>(weights_[v][k]);
                }
            }
            shares[v] = static_cast<double>(share);
        }
        return shares;
    }


    // Negative index: a vertex is live when none of its used slots became a sentinel
    std::vector<bool> VGComponentSplit::liveVertices(const VGComponentSpec& spec) const {
        Membership member = membership(spec, true);
        std::vector<bool> live(weights_.size(), false);
        for (std::size_t v = 0; v < weights_.size(); ++v) {
            bool ok = totals_[v] > 0.0;
            for (std::size_t k = 0; k < 4 && ok; ++k) {
                if (weights_[v][k] > 0.0 && !member.inComponent[v][k]) {
                    ok = false;
                }
            }
            live[v] = ok;
        }
        return live;
    }


    VGComponentBuffers VGComponentSplit::split(const std::string& component) const {
        for (std::size_t i = 0; i < specs_.size(); ++i) {
            if (specs_[i].name == component) {
                if (specs_[i].negativeIndex) {
                    return splitNegative(specs_[i]);
                }

                VGComponentBuffers result = splitCut(i);
                addMirroredLayer(result, specs_[i].mirroredIbs);
                return result;
            }
        }
        throw std::invalid_argument("no component named '" + component + "'");
    }


    // The inner layer -- see VGComponentSpec::mirroredIbs. Each corner of a mirrored buffer's triangles is copied
    // once (a vertex two mirrored buffers share gets one copy), and every triangle is followed by its copy wound
    // the other way, under the same source id.
    void VGComponentSplit::addMirroredLayer(VGComponentBuffers& result, const std::vector<std::size_t>& ibs) {
        if (ibs.empty()) {
            return;
        }

        result.mirrored.assign(result.vertices.size(), false);
        std::unordered_map<unsigned long long, unsigned long long> copyOf;
        const auto copy = [&](unsigned long long corner) {
            auto found = copyOf.find(corner);
            if (found != copyOf.end()) {
                return found->second;
            }

            const unsigned long long made = static_cast<unsigned long long>(result.vertices.size());
            result.vertices.push_back(result.vertices[static_cast<std::size_t>(corner)]);
            result.weights.push_back(result.weights[static_cast<std::size_t>(corner)]);
            result.indices.push_back(result.indices[static_cast<std::size_t>(corner)]);
            if (!result.live.empty()) {
                result.live.push_back(result.live[static_cast<std::size_t>(corner)]);
            }
            result.mirrored.push_back(true);
            copyOf.emplace(corner, made);
            ++result.stats.mirroredVertices;
            return made;
        };

        std::vector<bool> done(result.ibs.size(), false);
        for (std::size_t which : ibs) {
            if (which >= result.ibs.size() || done[which]) {
                continue;
            }
            done[which] = true;

            Triangles layered;
            std::vector<std::size_t> layeredIds;
            layered.reserve(result.ibs[which].size() * 2);
            layeredIds.reserve(result.ibs[which].size() * 2);
            const std::vector<std::size_t>& ids = result.keptTriangleIds[which];
            for (std::size_t t = 0; t < result.ibs[which].size(); ++t) {
                const auto& triangle = result.ibs[which][t];
                const std::size_t id = t < ids.size() ? ids[t] : 0;
                layered.push_back(triangle);
                layeredIds.push_back(id);

                const unsigned long long a = copy(triangle[0]);
                const unsigned long long b = copy(triangle[1]);
                const unsigned long long c = copy(triangle[2]);
                layered.push_back({a, c, b});
                layeredIds.push_back(id);
                ++result.stats.mirroredTriangles;
            }

            result.ibs[which] = std::move(layered);
            result.keptTriangleIds[which] = std::move(layeredIds);
            if (which < result.stats.trianglesKept.size()) {
                result.stats.trianglesKept[which] = result.ibs[which].size();
            }
        }

        result.stats.keptVertices = result.vertices.size();
    }


    ByteVec VGComponentSplit::mirrorPositionLine(const ByteVec& line, float offset) {
        constexpr std::size_t NormalAt = 12;
        if (line.size() < NormalAt + 3 * sizeof(float)) {
            return line;
        }

        ByteVec out = line;
        float position[3];
        float normal[3];
        std::memcpy(position, out.data(), sizeof(position));
        std::memcpy(normal, out.data() + NormalAt, sizeof(normal));
        for (std::size_t k = 0; k < 3; ++k) {
            position[k] -= normal[k] * offset;
            normal[k] = -normal[k];
        }
        std::memcpy(out.data(), position, sizeof(position));
        std::memcpy(out.data() + NormalAt, normal, sizeof(normal));
        return out;
    }


    VGComponentBuffers VGComponentSplit::splitNegative(const VGComponentSpec& spec) const {
        Membership member = membership(spec, true);
        VGComponentBuffers result;
        result.stats.vertexCount = weights_.size();
        result.vertices.resize(weights_.size());
        result.weights = weights_;
        result.indices.resize(weights_.size());
        result.live.assign(weights_.size(), false);

        for (std::size_t v = 0; v < weights_.size(); ++v) {
            result.vertices[v] = v;
            bool live = totals_[v] > 0.0;
            for (std::size_t k = 0; k < 4; ++k) {
                long long source = indices_[v][k];
                if (weights_[v][k] <= 0.0) {
                    result.indices[v][k] = source;               // a zero-weight slot is left alone
                } else if (member.inComponent[v][k]) {
                    result.indices[v][k] = member.mapped[v][k];
                } else {
                    result.indices[v][k] = -source - 1;
                    ++result.stats.sentinels;
                    live = false;
                }
            }
            result.live[v] = live;
        }
        result.stats.keptVertices = weights_.size();

        // Trimmed to the triangles all of whose corners are live: a sentinel corner is not
        // invisible, it lands near the origin
        for (const Triangles& ib : ibs_) {
            Triangles kept;
            std::vector<std::size_t> keptIds;
            std::size_t dropped = 0;
            for (std::size_t t = 0; t < ib.size(); ++t) {
                const auto& triangle = ib[t];
                bool ok = true;
                for (unsigned long long corner : triangle) {
                    if (corner >= result.live.size() || !result.live[corner]) {
                        ok = false;
                        break;
                    }
                }
                if (ok) {
                    kept.push_back(triangle);
                    keptIds.push_back(t);
                } else {
                    ++dropped;
                }
            }
            result.stats.trianglesKept.push_back(kept.size());
            result.stats.trianglesDropped.push_back(dropped);
            result.ibs.push_back(std::move(kept));
            result.keptTriangleIds.push_back(std::move(keptIds));
        }

        return result;
    }


    VGComponentBuffers VGComponentSplit::splitCut(std::size_t column) const {
        const std::size_t n = weights_.size();
        const std::size_t componentCount = specs_.size();

        // Ownership among the cut ("fill") components: the one holding most of a vertex's forward
        // weight, when it holds any. Argmax takes the FIRST column on a tie, as numpy does
        std::vector<std::vector<double>> shares(componentCount);
        std::vector<std::size_t> fillColumns;
        for (std::size_t c = 0; c < componentCount; ++c) {
            shares[c] = forwardShares(specs_[c]);
            if (!specs_[c].negativeIndex) {
                fillColumns.push_back(c);
            }
        }

        // A component with a claimShare takes a vertex only when that much of its weight is the
        // component's own -- see VGComponentSpec::claimShare; a vertex nothing can claim falls back
        // to the plain majority.
        auto claims = [&](std::size_t c, std::size_t v) {
            const double need = specs_[c].claimShare;
            return need <= 0.0 || shares[c][v] >= need * totals_[v] - 1e-9;
        };
        std::vector<long long> owner(n, -1);
        for (std::size_t v = 0; v < n; ++v) {
            double best = 0.0;
            for (std::size_t c : fillColumns) {
                if (shares[c][v] > best && claims(c, v)) {
                    best = shares[c][v];
                    owner[v] = static_cast<long long>(c);
                }
            }
            if (owner[v] >= 0) {
                continue;
            }
            best = 0.0;
            for (std::size_t c : fillColumns) {
                if (shares[c][v] > best) {
                    best = shares[c][v];
                    owner[v] = static_cast<long long>(c);
                }
            }
        }

        // The triangles the negative-index components already draw: all corners live in one
        std::vector<std::vector<bool>> excluded(ibs_.size());
        for (std::size_t i = 0; i < ibs_.size(); ++i) {
            excluded[i].assign(ibs_[i].size(), false);
        }
        for (const VGComponentSpec& spec : specs_) {
            if (!spec.negativeIndex) {
                continue;
            }
            std::vector<bool> live = liveVertices(spec);
            for (std::size_t i = 0; i < ibs_.size(); ++i) {
                for (std::size_t t = 0; t < ibs_[i].size(); ++t) {
                    bool all = true;
                    for (unsigned long long corner : ibs_[i][t]) {
                        if (corner >= live.size() || !live[corner]) {
                            all = false;
                            break;
                        }
                    }
                    if (all) {
                        excluded[i][t] = true;
                    }
                }
            }
        }

        VGComponentBuffers result;
        result.stats.vertexCount = n;

        // Per index buffer: a triangle goes to the fill component most of its corners belong to,
        // ties broken by the corners' summed share (scaled by that buffer's largest sum)
        std::vector<Triangles> keptTriangles;
        std::vector<std::vector<bool>> keepMask(ibs_.size());
        std::vector<std::vector<bool>> drawnByCut(ibs_.size());
        for (std::size_t i = 0; i < ibs_.size(); ++i) {
            const Triangles& ib = ibs_[i];
            keepMask[i].assign(ib.size(), false);
            drawnByCut[i].assign(ib.size(), false);

            // float, not double: the summed share is a float32 sum in the numpy original, and the
            // tie-break it feeds has to round the same way
            std::vector<std::vector<float>> weightSum(ib.size(), std::vector<float>(componentCount, 0.0f));
            std::vector<std::vector<int>> counts(ib.size(), std::vector<int>(componentCount, 0));
            float maxWeightSum = 0.0f;
            for (std::size_t t = 0; t < ib.size(); ++t) {
                for (unsigned long long corner : ib[t]) {
                    if (corner >= n) {
                        continue;
                    }
                    for (std::size_t c = 0; c < componentCount; ++c) {
                        weightSum[t][c] += static_cast<float>(shares[c][corner]);
                        if (owner[corner] == static_cast<long long>(c)) {
                            ++counts[t][c];
                        }
                    }
                }
                for (std::size_t c = 0; c < componentCount; ++c) {
                    maxWeightSum = std::max(maxWeightSum, weightSum[t][c]);
                }
            }
            const float scale = std::max(1.0f, ib.empty() ? 1.0f : maxWeightSum);

            for (std::size_t t = 0; t < ib.size(); ++t) {
                double bestScore = -std::numeric_limits<double>::infinity();
                long long bestColumn = -1;
                for (std::size_t c = 0; c < componentCount; ++c) {
                    double score = specs_[c].negativeIndex ? -1.0 : counts[t][c] * 10.0 + static_cast<double>(weightSum[t][c] / scale);
                    if (score > bestScore) {
                        bestScore = score;
                        bestColumn = static_cast<long long>(c);
                    }
                }
                keepMask[i][t] = bestColumn == static_cast<long long>(column) && bestScore > 0.0 && !excluded[i][t];
                drawnByCut[i][t] = bestScore > 0.0 && !excluded[i][t];
            }
        }

        // The overlap band -- see VGComponentSpec::overlapRings: each ring adds every triangle another
        // cut component draws that touches a vertex this component already draws. The rings are
        // grown across every index buffer at once, since a seam can run between two objects.
        const std::size_t rings = specs_[column].overlapRings;
        if (rings > 0) {
            std::vector<bool> near(n, false);
            for (std::size_t i = 0; i < ibs_.size(); ++i) {
                for (std::size_t t = 0; t < ibs_[i].size(); ++t) {
                    if (keepMask[i][t]) {
                        for (unsigned long long corner : ibs_[i][t]) {
                            if (corner < n) {
                                near[corner] = true;
                            }
                        }
                    }
                }
            }

            for (std::size_t ring = 0; ring < rings; ++ring) {
                std::vector<std::pair<std::size_t, std::size_t>> added;
                for (std::size_t i = 0; i < ibs_.size(); ++i) {
                    for (std::size_t t = 0; t < ibs_[i].size(); ++t) {
                        if (keepMask[i][t] || !drawnByCut[i][t]) {
                            continue;
                        }
                        for (unsigned long long corner : ibs_[i][t]) {
                            if (corner < n && near[corner]) {
                                added.emplace_back(i, t);
                                break;
                            }
                        }
                    }
                }
                if (added.empty()) {
                    break;
                }

                // marked after the pass, so each pass is exactly one ring
                for (const auto& [i, t] : added) {
                    keepMask[i][t] = true;
                    ++result.stats.overlapTriangles;
                    for (unsigned long long corner : ibs_[i][t]) {
                        if (corner < n) {
                            near[corner] = true;
                        }
                    }
                }
            }
        }

        for (std::size_t i = 0; i < ibs_.size(); ++i) {
            Triangles kept;
            std::vector<std::size_t> keptIds;
            std::size_t dropped = 0;
            for (std::size_t t = 0; t < ibs_[i].size(); ++t) {
                if (keepMask[i][t]) {
                    kept.push_back(ibs_[i][t]);
                    keptIds.push_back(t);
                } else {
                    ++dropped;
                }
            }
            result.stats.trianglesKept.push_back(kept.size());
            result.stats.trianglesDropped.push_back(dropped);
            keptTriangles.push_back(std::move(kept));
            result.keptTriangleIds.push_back(std::move(keptIds));
        }

        // The vertices those triangles use, ascending, and the renumbering into them
        std::vector<bool> referenced(n, false);
        for (const Triangles& kept : keptTriangles) {
            for (const auto& triangle : kept) {
                for (unsigned long long corner : triangle) {
                    if (corner < n) {
                        referenced[corner] = true;
                    }
                }
            }
        }
        std::vector<long long> renumber(n, -1);
        for (std::size_t v = 0; v < n; ++v) {
            if (referenced[v]) {
                renumber[v] = static_cast<long long>(result.vertices.size());
                result.vertices.push_back(v);
            }
        }
        result.stats.keptVertices = result.vertices.size();

        // The blend in the component's bones: forward remap (and the component's stand-ins, when it
        // has any -- VGComponentSpec::secondary), foreign weight dropped and the rest renormalised; a
        // vertex a triangle dragged in with no bone here is skinned to the bone its triangle
        // neighbours mostly use (fallback: the component's commonest bone)
        Membership member = membership(specs_[column], !specs_[column].secondary.empty());
        const std::size_t kept = result.vertices.size();
        result.weights.assign(kept, std::array<double, 4>{0.0, 0.0, 0.0, 0.0});
        result.indices.assign(kept, std::array<long long, 4>{0, 0, 0, 0});
        std::vector<float> rowTotals(kept, 0.0f);
        std::vector<std::size_t> orphans;
        for (std::size_t r = 0; r < kept; ++r) {
            std::size_t v = result.vertices[r];
            bool renormalised = false;
            float total = 0.0f;
            for (std::size_t k = 0; k < 4; ++k) {
                if (member.inComponent[v][k]) {
                    result.weights[r][k] = weights_[v][k];
                    result.indices[r][k] = member.mapped[v][k];
                } else if (weights_[v][k] > 0.0) {
                    renormalised = true;
                }
                total += static_cast<float>(result.weights[r][k]);
            }
            rowTotals[r] = total;
            if (renormalised) {
                ++result.stats.renormalised;
            }
            if (total <= 0.0f) {
                orphans.push_back(r);
            }
        }

        if (!orphans.empty()) {
            std::vector<long long> dominant(kept, 0);
            std::vector<bool> valid(kept, false);
            std::vector<long long> validDominants;
            for (std::size_t r = 0; r < kept; ++r) {
                std::size_t best = 0;
                for (std::size_t k = 1; k < 4; ++k) {
                    if (result.weights[r][k] > result.weights[r][best]) {
                        best = k;
                    }
                }
                dominant[r] = result.indices[r][best];
                valid[r] = rowTotals[r] > 0.0f;
                if (valid[r]) {
                    validDominants.push_back(dominant[r]);
                }
            }
            long long fallback = mostCommon(validDominants).value_or(0);

            std::unordered_map<std::size_t, std::vector<long long>> neighbours;
            for (std::size_t r : orphans) {
                neighbours[r];
            }
            for (const Triangles& keptIb : keptTriangles) {
                for (const auto& triangle : keptIb) {
                    std::array<long long, 3> local{};
                    for (std::size_t j = 0; j < 3; ++j) {
                        local[j] = triangle[j] < n ? renumber[triangle[j]] : -1;
                    }
                    for (std::size_t j = 0; j < 3; ++j) {
                        if (local[j] < 0) {
                            continue;
                        }
                        auto it = neighbours.find(static_cast<std::size_t>(local[j]));
                        if (it == neighbours.end()) {
                            continue;
                        }
                        for (std::size_t o = 0; o < 3; ++o) {
                            if (o != j && local[o] >= 0 && valid[static_cast<std::size_t>(local[o])]) {
                                it->second.push_back(dominant[static_cast<std::size_t>(local[o])]);
                            }
                        }
                    }
                }
            }

            for (std::size_t r : orphans) {
                long long bone = mostCommon(neighbours[r]).value_or(fallback);
                result.weights[r] = {1.0, 0.0, 0.0, 0.0};
                result.indices[r] = {bone, 0, 0, 0};
                rowTotals[r] = 1.0f;
            }
            result.stats.neighbourSkinned = orphans.size();
        }

        for (std::size_t r = 0; r < kept; ++r) {
            for (std::size_t k = 0; k < 4; ++k) {
                float w = static_cast<float>(result.weights[r][k]) / rowTotals[r];
                result.weights[r][k] = static_cast<double>(w);
            }
        }

        // Shared groups -- see VGComponentSpec::splitGroups. Over the final weights: each influence whose
        // SOURCE group is shared becomes one per bone of the share, equal bones merged, the 4 largest kept.
        const auto& splitGroups = specs_[column].splitGroups;
        if (!splitGroups.empty()) {
            for (std::size_t r = 0; r < kept; ++r) {
                const std::size_t v = result.vertices[r];
                bool touched = false;
                std::vector<std::pair<long long, double>> influences;
                const auto add = [&influences](long long bone, double weight) {
                    for (auto& entry : influences) {
                        if (entry.first == bone) {
                            entry.second += weight;
                            return;
                        }
                    }
                    influences.emplace_back(bone, weight);
                };

                for (std::size_t k = 0; k < 4; ++k) {
                    const double weight = result.weights[r][k];
                    if (weight <= 0.0) {
                        continue;
                    }
                    auto shared = member.inComponent[v][k] ? splitGroups.find(indices_[v][k]) : splitGroups.end();
                    if (shared == splitGroups.end()) {
                        add(result.indices[r][k], weight);
                        continue;
                    }
                    touched = true;
                    for (const auto& [bone, share] : shared->second) {
                        add(bone, weight * share);
                    }
                }

                if (!touched) {
                    continue;
                }

                std::stable_sort(influences.begin(), influences.end(),
                                 [](const auto& a, const auto& b) { return a.second > b.second; });
                influences.resize(std::min<std::size_t>(influences.size(), 4));
                double total = 0.0;
                for (const auto& entry : influences) {
                    total += entry.second;
                }
                for (std::size_t k = 0; k < 4; ++k) {
                    const bool has = k < influences.size() && total > 0.0;
                    result.indices[r][k] = has ? influences[k].first : 0;
                    result.weights[r][k] = has ? static_cast<double>(static_cast<float>(influences[k].second / total)) : 0.0;
                }
                ++result.stats.splitVertices;
            }
        }

        // The index buffers, renumbered into the kept vertices
        for (const Triangles& keptIb : keptTriangles) {
            Triangles renumbered;
            renumbered.reserve(keptIb.size());
            for (const auto& triangle : keptIb) {
                std::array<unsigned long long, 3> local{};
                for (std::size_t j = 0; j < 3; ++j) {
                    local[j] = static_cast<unsigned long long>(renumber[triangle[j]]);
                }
                renumbered.push_back(local);
            }
            result.ibs.push_back(std::move(renumbered));
        }

        return result;
    }
}
