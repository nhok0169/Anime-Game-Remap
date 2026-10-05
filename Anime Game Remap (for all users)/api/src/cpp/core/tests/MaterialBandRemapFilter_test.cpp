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

// -----------------------------------------------------------------------------
// Standalone test for AGRemapCore::MaterialBandRemapFilter -- moving a light map's MATERIAL BANDS
// from one skin's legend onto another's.
//
// A light map's alpha is a material band and the legend differs per skin, so a remap that copies a
// light map across unchanged shades the wrong material. Two directions of one character pair had a
// hand-written closure each, ~90% identical, and the next character's two are already stubbed in
// Tools/Misc/Prototypes/ waiting for measured legends -- which is what this class is for.
//
// THE PROPERTY THAT MATTERS MOST IS CASE 3: the moves are applied SIMULTANEOUSLY, every decision
// read from the ORIGINAL alpha. A real legend is typically a PERMUTATION of the bands, and applied
// in sequence a permutation chases itself -- 0 -> 255, then 255 -> 121, and band 0 has landed on
// 121 having passed through a band it was never meant to occupy. That bug is invisible in every
// count, and in game it is a character shaded as the wrong material.
//
// The diffuse-gated paths are exercised here only through the "no diffuse readable => the gate
// PASSES" rule; a gate actually reading a diffuse needs a real .dds and is covered by the A/B
// against the previous build over real mods.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only when
// somebody compiles it. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      MaterialBandRemapFilter_test.cpp ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags.
// -----------------------------------------------------------------------------

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"

using namespace AGRemapCore;

namespace {
    int failures = 0;


    void check(bool ok, const std::string& what) {
        std::printf("  %s %s\n", ok ? "ok  " : "FAIL", what.c_str());
        if (!ok) {
            ++failures;
        }
    }


    // One pixel per band, in a 1-pixel-tall strip. The colour is mid-grey throughout, so a gated
    // band behaves purely on its gate rather than on anything the colour happens to look like.
    std::vector<std::uint8_t> strip(const std::vector<std::uint8_t>& alphas) {
        std::vector<std::uint8_t> pixels;
        pixels.reserve(alphas.size() * 4);
        for (std::uint8_t alpha : alphas) {
            pixels.push_back(128);
            pixels.push_back(128);
            pixels.push_back(128);
            pixels.push_back(alpha);
        }
        return pixels;
    }


    std::vector<std::uint8_t> run(const std::vector<MaterialBandRemapFilter::Band>& bands,
                                   const std::vector<std::uint8_t>& alphas,
                                   const std::string& diffusePath = "") {
        TextureFile texFile("");
        texFile.setPixels(strip(alphas), static_cast<int>(alphas.size()), 1);

        MaterialBandRemapFilter filter(bands, diffusePath);
        filter.transform(texFile);

        std::vector<std::uint8_t> out;
        const std::vector<std::uint8_t> pixels = texFile.getPixels();
        for (std::size_t i = 3; i < pixels.size(); i += 4) {
            out.push_back(pixels[i]);
        }
        return out;
    }


    std::string show(const std::vector<std::uint8_t>& values) {
        std::string out;
        for (std::uint8_t value : values) {
            if (!out.empty()) {
                out += ", ";
            }
            out += std::to_string(value);
        }
        return "[" + out + "]";
    }
}


int main() {
    std::printf("MaterialBandRemapFilter\n");

    // ---- 1. a single exact band moves, and nothing else is touched ----
    {
        const std::vector<std::uint8_t> got = run({{255, 0}}, {0, 64, 127, 180, 255});
        const std::vector<std::uint8_t> want = {0, 64, 127, 180, 0};
        check(got == want, "an exact band moves and every other band is left alone -- got " + show(got));
    }

    // ---- 2. a RANGE moves, inclusive at both ends ----
    {
        const std::vector<std::uint8_t> got = run({{115, 127, 255}}, {114, 115, 121, 127, 128});
        const std::vector<std::uint8_t> want = {114, 255, 255, 255, 128};
        check(got == want, "a range moves inclusively at both ends -- got " + show(got));
    }

    // ---- 3. THE ONE THAT MATTERS: the moves are SIMULTANEOUS, not sequential ----
    {
        // The real YelanTranquil -> Yelan legend, which is a permutation:
        //   255 -> 121   (skin)
        //     0 -> 255   (white fur)
        //   115-128 -> 0 (hair)
        // Applied in SEQUENCE this chases itself: 0 becomes 255, which the first rule would then
        // have taken to 121 -- and 115-128 -> 0 would feed the fur rule. Every value below must be
        // decided from the ORIGINAL alpha.
        const std::vector<MaterialBandRemapFilter::Band> permutation = {
            {255, 121},
            {0, 255},
            {115, 128, 0},
        };

        const std::vector<std::uint8_t> got = run(permutation, {0, 115, 121, 128, 255, 64});
        const std::vector<std::uint8_t> want = {255, 0, 0, 0, 121, 64};
        check(got == want,
              "a PERMUTATION of bands is applied simultaneously, every decision from the ORIGINAL "
              "alpha -- got " + show(got) + ", want " + show(want));

        // and specifically: band 0 must land on the fur band and STOP there
        check(got[0] == 255, "band 0 lands on 255 and is not then carried on to the skin band");
    }

    // ---- 4. the FIRST band containing a pixel decides ----
    {
        const std::vector<std::uint8_t> got = run({{100, 150, 10}, {120, 130, 20}}, {125});
        check(got == std::vector<std::uint8_t>{10},
              "where two ranges overlap the FIRST one wins -- got " + show(got));
    }

    // ---- 5. an unreadable diffuse makes every gate PASS ----
    {
        // The band legend is the better of the two guesses when there is nothing to check against,
        // so a gate with no diffuse must not silently block the move.
        const std::vector<MaterialBandRemapFilter::Band> gated = {
            {255, 121, &MaterialBandRemapFilter::skinColoured},
        };

        check(run(gated, {255}) == std::vector<std::uint8_t>{121},
              "a gated band still moves when NO diffuse path is given");
        check(run(gated, {255}, "no-such-file-anywhere.dds") == std::vector<std::uint8_t>{121},
              "and when the diffuse path names a file that cannot be read");
    }

    // ---- 6. an empty table is a no-op, and a negated gate is still a gate ----
    {
        check(run({}, {0, 115, 255}) == (std::vector<std::uint8_t>{0, 115, 255}),
              "an empty band table leaves the light map untouched");

        const std::vector<MaterialBandRemapFilter::Band> negated = {
            {115, 128, 0, &MaterialBandRemapFilter::skinColoured, true},
        };
        check(run(negated, {120}) == std::vector<std::uint8_t>{0},
              "a NEGATED gate with no diffuse also passes -- the rule is about the diffuse being "
              "unknown, not about which way the test points");
    }

    // ---- 7. the shared colour predicates say what they are documented to say ----
    {
        check(MaterialBandRemapFilter::skinColoured(243, 216, 197), "skinColoured accepts a measured skin tone");
        check(!MaterialBandRemapFilter::skinColoured(55, 65, 105), "and rejects the measured dark-navy hair");
        check(MaterialBandRemapFilter::whiteFurColoured(187, 204, 221), "whiteFurColoured accepts measured white fur");
        check(!MaterialBandRemapFilter::whiteFurColoured(45, 64, 88), "and rejects a dark navy cloth");
    }

    std::printf(failures ? "\n%d FAILURE(S)\n" : "\nall passed\n", failures);
    return failures ? 1 : 0;
}
