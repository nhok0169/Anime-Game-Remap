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

#include "AGRemapCore/data/IniFixData/Lynae/LynaePropsGrade.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {
        // Lynae's props: the glass layer the grade compensates for
        constexpr int PropsComponent = 6;

        std::vector<std::uint8_t> readFile(const std::string& path) {
            std::ifstream in(FileService::strToPath(path), std::ios::binary);
            if (!in) {
                return {};
            }

            return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(in)),
                                             std::istreambuf_iterator<char>());
        }

        float halfToFloat(std::uint16_t bits) {
            const int sign = (bits >> 15) & 0x1;
            const int exponent = (bits >> 10) & 0x1F;
            const int mantissa = bits & 0x3FF;
            float value = 0.0f;
            if (exponent == 0) {
                value = std::ldexp(static_cast<float>(mantissa), -24);
            } else if (exponent != 0x1F) {
                value = std::ldexp(static_cast<float>(mantissa + 1024), exponent - 25);
            }

            return sign ? -value : value;
        }

        // The texels component 'comp' covers, rasterised from the MOD's own mesh (float16 UVs at the
        // start of each texcoord vertex, as every WWMI mod here has them), grown two texels into the
        // gutter so a bilinear sample at an island's edge reads graded colour too. Chisa's fixer has
        // the same raster split by height (her islandMask); this one takes the whole component.
        std::vector<bool> componentIsland(const WWMIFixerConfig::TexEditContext& ctx, int comp, int width, int height) {
            const auto ranges = ctx.drawRanges.find(comp);
            if (ranges == ctx.drawRanges.end() || ranges->second.empty() || width <= 0 || height <= 0) {
                return {};
            }

            const std::vector<std::uint8_t> uvRaw = readFile(ctx.texcoordFile);
            const std::vector<std::uint8_t> idxRaw = readFile(ctx.indexFile);
            const std::vector<std::uint8_t> posRaw = readFile(ctx.positionFile);
            const std::size_t vertices = posRaw.size() / 12;
            if (uvRaw.empty() || idxRaw.empty() || vertices == 0 || uvRaw.size() % vertices != 0) {
                return {};
            }

            const std::size_t stride = uvRaw.size() / vertices;
            if (stride < 4) {
                return {};
            }

            auto uvOf = [&uvRaw, stride](std::size_t v, int axis) {
                std::uint16_t bits = 0;
                std::memcpy(&bits, uvRaw.data() + v * stride + static_cast<std::size_t>(axis) * 2, 2);
                return halfToFloat(bits);
            };

            const std::uint32_t* index = reinterpret_cast<const std::uint32_t*>(idxRaw.data());
            const std::size_t indexCount = idxRaw.size() / 4;
            const std::size_t texels = static_cast<std::size_t>(width) * height;
            std::vector<bool> island(texels, false);
            std::size_t covered = 0;

            for (const auto& range : ranges->second) {
                for (long long k = 0; k + 2 < range.first; k += 3) {
                    const std::size_t at = static_cast<std::size_t>(range.second + k);
                    if (at + 2 >= indexCount) {
                        break;
                    }

                    const std::uint32_t tri[3] = {index[at], index[at + 1], index[at + 2]};
                    if (tri[0] >= vertices || tri[1] >= vertices || tri[2] >= vertices) {
                        continue;
                    }

                    float xs[3];
                    float ys[3];
                    for (int c = 0; c < 3; ++c) {
                        xs[c] = uvOf(tri[c], 0) * width;
                        ys[c] = uvOf(tri[c], 1) * height;
                    }

                    const int x0 = std::max(0, static_cast<int>(std::floor(std::min({xs[0], xs[1], xs[2]}))));
                    const int x1 = std::min(width - 1, static_cast<int>(std::ceil(std::max({xs[0], xs[1], xs[2]}))));
                    const int y0 = std::max(0, static_cast<int>(std::floor(std::min({ys[0], ys[1], ys[2]}))));
                    const int y1 = std::min(height - 1, static_cast<int>(std::ceil(std::max({ys[0], ys[1], ys[2]}))));
                    const float det = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2]);
                    if (std::fabs(det) < 1e-9f) {
                        continue;
                    }

                    for (int y = y0; y <= y1; ++y) {
                        for (int x = x0; x <= x1; ++x) {
                            const float px = x + 0.5f;
                            const float py = y + 0.5f;
                            const float w0 = ((ys[1] - ys[2]) * (px - xs[2]) + (xs[2] - xs[1]) * (py - ys[2])) / det;
                            const float w1 = ((ys[2] - ys[0]) * (px - xs[2]) + (xs[0] - xs[2]) * (py - ys[2])) / det;
                            if (w0 >= -1e-6f && w1 >= -1e-6f && (w0 + w1) <= 1.0f + 1e-6f) {
                                const std::size_t i = static_cast<std::size_t>(y) * width + x;
                                covered += island[i] ? 0 : 1;
                                island[i] = true;
                            }
                        }
                    }
                }
            }

            if (covered == 0) {
                return {};
            }

            for (int pass = 0; pass < 2; ++pass) {
                std::vector<bool> grown = island;
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        const std::size_t i = static_cast<std::size_t>(y) * width + x;
                        if (!island[i]
                                && ((x > 0 && island[i - 1]) || (x + 1 < width && island[i + 1])
                                    || (y > 0 && island[i - width]) || (y + 1 < height && island[i + width]))) {
                            grown[i] = true;
                        }
                    }
                }

                island.swap(grown);
            }

            return island;
        }
    }

    TexEditor::Filter lynaePropsGrade(const WWMIFixerConfig::TexEditContext& ctx, const std::string& role, double strength) {
        const auto own = ctx.gameFileOfRole.find(role);
        const std::string ownPath = own == ctx.gameFileOfRole.end() ? std::string() : own->second;

        return [ctx, ownPath, strength](TextureFile& tex) {
            if (ownPath.empty()) {
                return;
            }

            TextureFile reference(ownPath);
            reference.open();
            if (!reference.hasImage()) {
                return;
            }

            const int w = tex.getWidth();
            const int h = tex.getHeight();
            const std::vector<bool> island = componentIsland(ctx, PropsComponent, w, h);
            if (island.empty()) {
                return;
            }

            // Both are colour, compared as stored (sRGB) bytes: the gamma is kept, so the written
            // copy samples the way the mod's sRGB texture did
            std::vector<std::uint8_t> px = tex.getPixels();
            const std::vector<std::uint8_t>& ref = reference.getPixels();
            const int rw = reference.getWidth();
            const int rh = reference.getHeight();

            for (int y = 0; y < h; ++y) {
                // the character's texture at the same UV, whatever its resolution
                const int ry = std::min(rh - 1, static_cast<int>((static_cast<long long>(y) * rh) / h));
                for (int x = 0; x < w; ++x) {
                    const std::size_t i = static_cast<std::size_t>(y) * w + x;
                    if (!island[i] || i * 4 + 3 >= px.size()) {
                        continue;
                    }

                    const int rx = std::min(rw - 1, static_cast<int>((static_cast<long long>(x) * rw) / w));
                    const std::size_t r = (static_cast<std::size_t>(ry) * rw + rx) * 4;
                    if (r + 3 >= ref.size()) {
                        continue;
                    }

                    for (int c = 0; c < 3; ++c) {
                        const double pushed = ref[r + c] + strength * (static_cast<double>(px[i * 4 + c]) - ref[r + c]);
                        px[i * 4 + c] = static_cast<std::uint8_t>(std::lround(std::min(255.0, std::max(0.0, pushed))));
                    }
                }
            }

            tex.setPixels(std::move(px), w, h);
        };
    }
}
