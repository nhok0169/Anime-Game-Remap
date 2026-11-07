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

#include "AGRemapCore/data/IniFixData/Lynae/LynaeSkinCodes.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <utility>
#include <string>
#include <vector>



namespace AGRemapCore {
    namespace {
        // Measured over the two skins' game textures by the diffuse under each texel: flesh is
        // material 0 on 98% of Lynae's upper and lower body, material 4 on 94% / 82% of the skin's
        constexpr std::uint8_t LynaeSkin = 0;
        constexpr std::uint8_t PeppermintSkin = 4;

        template <typename Map>
        std::vector<std::uint8_t> tableOf(Map map) {
            std::vector<std::uint8_t> table(256);
            for (int v = 0; v < 256; ++v) {
                table[v] = map(static_cast<std::uint8_t>(v));
            }
            return table;
        }

        // Per alpha, the multiplier (in linear colour) that makes LynaePeppermint land where Lynae does: a flat
        // grey 120 under code 1 on both cards, the normal map flat at each alpha (menu, Lynae14, 2026-10-12). From
        // 128 up she renders as the skin does; around 102 (Lynae14's dress) a third as bright (0.39 at grey 120;
        // 0.46 after one closed-loop round on the dress's own red, darker inputs dipping less)
        double shadeBandMultiplier(int alpha) {
            static const std::pair<double, double> Points[] = {
                {0, 0.88}, {51, 0.78}, {72, 0.77}, {84, 0.80}, {96, 0.87}, {102, 0.46}, {108, 0.57},
                {114, 0.77}, {120, 0.93}, {128, 1.0}, {255, 1.0}};
            for (std::size_t i = 1; i < std::size(Points); ++i) {
                if (alpha <= Points[i].first) {
                    const double t = (alpha - Points[i - 1].first) / (Points[i].first - Points[i - 1].first);
                    return Points[i - 1].second + t * (Points[i].second - Points[i - 1].second);
                }
            }
            return 1.0;
        }
    }

    std::vector<std::uint8_t> lynaeSkinCodeTable() {
        return tableOf([](std::uint8_t v) {
            const std::uint8_t material = v & 0x0F;
            const std::uint8_t flags = v & 0xF0;
            if (material == LynaeSkin) {
                return static_cast<std::uint8_t>(flags | PeppermintSkin);
            }
            if (material == PeppermintSkin) {
                return static_cast<std::uint8_t>(flags | LynaeSkin);
            }
            return v;
        });
    }

    std::vector<std::uint8_t> lynaeFilmCodeTable() {
        // LynaePeppermint's coat film: material 1, flags 3
        return tableOf([](std::uint8_t v) { return (v & 0x0F) == 4 ? std::uint8_t{49} : v; });
    }

    std::vector<std::uint8_t> lynaePropsPlainCodeTable() {
        // her film (flag 0x10: her own sweater is 16) keeps its code, which the pouch draws as its foil;
        // everything else takes the pouch's one plain material, 15, with its flags kept
        return tableOf([](std::uint8_t v) { return (v & 0x10) ? v : static_cast<std::uint8_t>((v & 0xF0) | 0x0F); });
    }

    std::vector<std::array<double, 3>> lynaeShadeBandFactors() {
        // No per-material colour ratio: her stocking materials (codes 1-4) share every non-colour input
        // with ordinary cloth -- Lynae14 paints 88% of its atlas code 1, fur, gold and dresses alike, on
        // the same normal-alpha band and mask values as her legs -- so a ratio could only be aimed by
        // the diffuse's colour, which a mod is free to paint any way it likes. Her legs stay a little
        // warmer than on Lynae (the maintainer's call, 2026-10-12)
        std::vector<std::array<double, 3>> factors(256);
        for (int a = 0; a < 256; ++a) {
            const double m = shadeBandMultiplier(a);
            factors[a] = {m, m, m};
        }
        return factors;
    }

    std::vector<std::array<double, 3>> lynaeFilmFactors() {
        // The coat lining, Lynae code 4: her render of it (a pink-red film) over LynaePeppermint's with the
        // code moved to 49 by lynaeFilmCodeTable (menu, Lynae11 -- whose coat texture is Lynae's own, 2026-10-12)
        std::vector<std::array<double, 3>> factors(256);
        for (int v = 0; v < 256; ++v) {
            factors[v] = (v & 0x0F) == 4 ? std::array<double, 3>{2.0, 0.89, 0.88} : std::array<double, 3>{1.0, 1.0, 1.0};
        }
        return factors;
    }
}
