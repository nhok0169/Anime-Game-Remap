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

#include "AGRemapCore/model/buffers/VGComponentMerge.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

namespace AGRemapCore {
    VGComponentMerge::Indices VGComponentMerge::remapIndices(const Indices& indices, const Weights& weights,
                                                             const VGRemap& remap, std::vector<long long>& unmapped) {
        if (indices.size() != weights.size()) {
            throw std::invalid_argument("a Blend.buf has as many index lines as weight lines");
        }

        const std::unordered_map<long long, long long>& rows = remap.getRemap();
        std::set<long long> missing;
        Indices result = indices;

        for (std::size_t v = 0; v < indices.size(); ++v) {
            for (std::size_t k = 0; k < 4; ++k) {
                // a zero-weight slot contributes nothing and keeps its literal index
                if (weights[v][k] <= 0.0) {
                    continue;
                }

                long long group = indices[v][k];
                auto it = rows.find(group);
                if (it != rows.end()) {
                    result[v][k] = it->second;
                    continue;
                }

                // the sentinel the rest of the library writes for a group with nowhere to go: a
                // NEGATIVE bone, not an absent one, so the model kinks rather than losing a vertex
                result[v][k] = -group - 1;
                missing.insert(group);
            }
        }

        unmapped.insert(unmapped.end(), missing.begin(), missing.end());
        return result;
    }


    ByteVec VGComponentMerge::padLines(const ByteVec& src, std::size_t lines, std::size_t to) {
        if (lines == 0) {
            return ByteVec();
        }
        if (to == 0 || src.size() % lines != 0) {
            throw std::invalid_argument("a buffer of " + std::to_string(src.size()) + " bytes is not a whole number of "
                                        + std::to_string(lines) + " lines");
        }

        std::size_t from = src.size() / lines;
        if (from > to) {
            throw std::invalid_argument("a buffer of stride " + std::to_string(from) + " is wider than the merged stride "
                                        + std::to_string(to));
        }
        if (from == to) {
            return src;
        }

        ByteVec out(lines * to, static_cast<ByteVec::value_type>(0));
        for (std::size_t i = 0; i < lines; ++i) {
            std::copy(src.begin() + static_cast<std::ptrdiff_t>(i * from),
                      src.begin() + static_cast<std::ptrdiff_t>((i + 1) * from),
                      out.begin() + static_cast<std::ptrdiff_t>(i * to));
        }
        return out;
    }


    VGComponentMerge::VGComponentMerge(std::vector<Component> components) {
        if (components.empty()) {
            throw std::invalid_argument("a merge needs at least one component");
        }

        // the widest texcoord decides the merged stride: one buffer has one stride, and a component
        // carrying no second UV set is the short one
        std::size_t texcoordStride = 0;
        for (const Component& component : components) {
            std::size_t lines = component.weights.size();
            if (lines != 0 && component.texcoord.size() % lines == 0) {
                texcoordStride = std::max(texcoordStride, component.texcoord.size() / lines);
            }
        }

        Weights weights;
        Indices indices;
        std::size_t offset = 0;

        for (const Component& component : components) {
            if (offsets_.count(component.spec.name) != 0) {
                throw std::invalid_argument("two components are both named '" + component.spec.name + "'");
            }

            std::size_t lines = component.weights.size();
            if (component.indices.size() != lines) {
                throw std::invalid_argument("the '" + component.spec.name + "' component's Blend.buf has "
                                            + std::to_string(component.indices.size()) + " index lines against "
                                            + std::to_string(lines) + " weight lines");
            }

            std::vector<long long> missing;
            Indices remapped = remapIndices(component.indices, component.weights, component.spec.remap, missing);
            for (long long group : missing) {
                stats_.unmapped.emplace_back(component.spec.name, group);
            }

            weights.insert(weights.end(), component.weights.begin(), component.weights.end());
            indices.insert(indices.end(), remapped.begin(), remapped.end());

            position_.insert(position_.end(), component.position.begin(), component.position.end());

            // texcoordStride is 0 only when no component named a texcoord at all -- padding to 0
            // would throw, so the (empty) buffers are simply carried through
            if (texcoordStride != 0) {
                ByteVec texcoord = padLines(component.texcoord, lines, texcoordStride);
                if (lines != 0 && texcoord.size() != component.texcoord.size()) {
                    ++stats_.paddedComponents;
                }
                texcoord_.insert(texcoord_.end(), texcoord.begin(), texcoord.end());
            } else {
                texcoord_.insert(texcoord_.end(), component.texcoord.begin(), component.texcoord.end());
            }

            offsets_.emplace(component.spec.name, offset);
            stats_.order.push_back(component.spec.name);
            stats_.offsets.push_back(offset);
            stats_.vertices.push_back(lines);
            offset += lines;
        }

        blend_ = VGComponentSplit::encodeBlend(weights, indices);
        stats_.vertexCount = offset;
        stats_.texcoordStride = texcoordStride;
    }


    const ByteVec& VGComponentMerge::blend() const {
        return blend_;
    }


    const ByteVec& VGComponentMerge::position() const {
        return position_;
    }


    const ByteVec& VGComponentMerge::texcoord() const {
        return texcoord_;
    }


    VGComponentMerge::Triangles VGComponentMerge::mergeIbs(const std::vector<std::pair<std::string, Triangles>>& members) const {
        Triangles out;
        std::size_t total = 0;
        for (const auto& member : members) {
            total += member.second.size();
        }
        out.reserve(total);

        for (const auto& member : members) {
            auto offset = static_cast<unsigned long long>(offsetOf(member.first));
            for (const std::array<unsigned long long, 3>& triangle : member.second) {
                out.push_back({triangle[0] + offset, triangle[1] + offset, triangle[2] + offset});
            }
        }
        return out;
    }


    std::size_t VGComponentMerge::offsetOf(const std::string& component) const {
        auto it = offsets_.find(component);
        if (it == offsets_.end()) {
            throw std::invalid_argument("'" + component + "' is not one of the merged components");
        }
        return it->second;
    }


    std::size_t VGComponentMerge::vertexCount() const {
        return stats_.vertexCount;
    }


    const VGComponentMergeStats& VGComponentMerge::stats() const {
        return stats_;
    }
}
