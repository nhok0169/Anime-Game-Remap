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

#include "AGRemapCore/model/files/TexcoordFile.h"

#include <cmath>
#include <utility>
#include <variant>

#include "AGRemapCore/model/buffers/BufFloat.h"


namespace AGRemapCore {
    TexcoordFile::TexcoordFile(BinarySrc src, std::vector<std::unique_ptr<BufElementType>> elements, std::string uvElement):
        BufFile(std::move(src), elements.empty() ? halfElements(4) : std::move(elements), "Texcoord.buf"),
        uvElement_(std::move(uvElement)) {}

    std::vector<std::unique_ptr<BufElementType>> TexcoordFile::halfElements(std::size_t stride) {
        std::vector<std::unique_ptr<BufElementType>> elements;
        elements.push_back(BufElementType::repeated(
            UVKey, stride / 2, [] { return std::make_unique<BufFloat16>(false, BufFloat16::Rounding::NearestEven); }));
        return elements;
    }

    const std::string& TexcoordFile::uvElement() const {
        return uvElement_;
    }

    std::vector<float> TexcoordFile::us() const {
        std::vector<float> u;
        for (const double value : column(uvElement_, 0)) {
            u.push_back(std::isnan(value) ? 0.0f : static_cast<float>(value));
        }
        return u;
    }

    std::vector<bool> TexcoordFile::straddling(const std::vector<long long>& indices) const {
        const std::vector<float> u = us();
        const std::size_t vertices = u.size();
        std::vector<bool> keep(vertices, false);
        for (std::size_t t = 0; t + 2 < indices.size(); t += 3) {
            bool inRange = true;
            std::size_t tri[3] = {0, 0, 0};
            for (std::size_t i = 0; i < 3; ++i) {
                inRange = inRange && indices[t + i] >= 0 && static_cast<std::size_t>(indices[t + i]) < vertices;
                tri[i] = inRange ? static_cast<std::size_t>(indices[t + i]) : 0;
            }
            if (!inRange) {
                continue;
            }

            const float a = std::floor(u[tri[0]]);
            const float b = std::floor(u[tri[1]]);
            const float c = std::floor(u[tri[2]]);
            if (a != b || b != c) {
                keep[tri[0]] = true;
                keep[tri[1]] = true;
                keep[tri[2]] = true;
            }
        }
        return keep;
    }

    TexcoordFile::CleanStats TexcoordFile::clean(const std::vector<bool>& keep) {
        CleanStats stats;
        const std::string& key = uvElement_;
        const Filter cleaner = [&stats, &keep, &key](const BufLineData& line, long long, double index, long long) {
            BufLineData out = line;
            const auto at = out.find(key);
            if (at == out.end() || at->second.empty()) {
                return out;
            }

            for (BufValue& value : at->second) {
                if (std::holds_alternative<double>(value) && std::isnan(std::get<double>(value))) {
                    value = 0.0;
                    ++stats.cleared;
                }
            }

            if (!std::holds_alternative<double>(at->second.front())) {
                return out;
            }

            const auto vertex = static_cast<std::size_t>(index);
            const auto first = static_cast<float>(std::get<double>(at->second.front()));
            if (first >= 1.0f && (vertex >= keep.size() || !keep[vertex])) {
                at->second.front() = static_cast<double>(std::fmod(first, 1.0f));
                ++stats.folded;
            }
            return out;
        };

        FixResult result = fix(std::nullopt, {cleaner});
        if (stats.cleared > 0 || stats.folded > 0) {
            setSrc(std::get<ByteVec>(std::move(result)));
            read();
        }
        return stats;
    }
}
