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
// Standalone test for AGRemapCore::VGComponentMerge -- the inverse of VGComponentSplit: a mod built
// for a skin of SEVERAL components merged onto a target of one.
//
// What is worth pinning here, because each was a real mistake before it was a rule:
//
//   * A bone slot is remapped only where it carries WEIGHT. [0,0,0,0] with weights [1,0,0,0] comes
//     out [64,0,0,0], NOT [64,64,64,64]. The first version of the Python prototype remapped all
//     four and disagreed with the rest of the library byte for byte while looking perfectly right
//     in game.
//   * A weighted group with no entry becomes the -index-1 SENTINEL and is reported. It is a bone,
//     not an absent one: the model kinks there and nothing else in the pipeline says a word.
//   * The texcoord stride is levelled up to the widest component's and short lines are padded at
//     the END -- where a missing TEXCOORD1 sits. YelanTranquil's Eye is stride 12 against her Body
//     and Bang's 20, and one buffer has one stride.
//   * Index buffers are offset by their component's first vertex, and several source objects on one
//     target object concatenate into a single draw (her Bang and Eye both land on Yelan's head).
//   * The first component takes offset 0, so its index buffers pass through untouched.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only when
// somebody compiles it. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      VGComponentMerge_test.cpp ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags.
// -----------------------------------------------------------------------------

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "AGRemapCore/model/buffers/VGComponentMerge.h"

namespace AGRC = AGRemapCore;

namespace {
    int failures = 0;

    void check(bool condition, const std::string& what) {
        if (condition) {
            std::cout << "  ok   " << what << "\n";
            return;
        }
        std::cout << "  FAIL " << what << "\n";
        ++failures;
    }

    // One blend line's four indices back out of the encoded bytes
    std::vector<long long> indicesOf(const AGRC::ByteVec& blend, std::size_t line) {
        std::vector<long long> out;
        for (std::size_t k = 0; k < 4; ++k) {
            std::int32_t value = 0;
            std::memcpy(&value, blend.data() + line * 32 + 16 + k * 4, sizeof(value));
            out.push_back(value);
        }
        return out;
    }

    AGRC::VGComponentMerge::Component makeComponent(const std::string& name,
                                                    std::unordered_map<long long, long long> remap,
                                                    AGRC::VGComponentMerge::Weights weights,
                                                    AGRC::VGComponentMerge::Indices indices,
                                                    std::size_t positionStride, std::size_t texcoordStride,
                                                    unsigned char fill) {
        AGRC::VGComponentMerge::Component component;
        component.spec.name = name;
        component.spec.remap = AGRC::VGRemap(std::move(remap));
        component.weights = std::move(weights);
        component.indices = std::move(indices);
        component.position.assign(component.weights.size() * positionStride, fill);
        component.texcoord.assign(component.weights.size() * texcoordStride, fill);
        return component;
    }
}


int main() {
    std::cout << "VGComponentMerge\n";

    // ---- a zero-weight slot keeps its literal index ----
    {
        AGRC::VGComponentMerge::Weights weights = {{1.0, 0.0, 0.0, 0.0}, {0.5, 0.5, 0.0, 0.0}};
        AGRC::VGComponentMerge::Indices indices = {{0, 0, 0, 0}, {0, 3, 0, 0}};
        std::vector<long long> unmapped;
        AGRC::VGComponentMerge::Indices out =
            AGRC::VGComponentMerge::remapIndices(indices, weights, AGRC::VGRemap({{0, 64}, {3, 7}}), unmapped);

        check(out[0] == (std::array<long long, 4>{64, 0, 0, 0}),
              "[0,0,0,0] weighted [1,0,0,0] -> [64,0,0,0], not [64,64,64,64]");
        check(out[1] == (std::array<long long, 4>{64, 7, 0, 0}), "both weighted slots remap");
        check(unmapped.empty(), "nothing unmapped when every weighted group has an entry");
    }

    // ---- an unmapped weighted group becomes the sentinel, and is reported ----
    {
        AGRC::VGComponentMerge::Weights weights = {{1.0, 0.0, 0.0, 0.0}, {1.0, 0.0, 0.0, 0.0}};
        AGRC::VGComponentMerge::Indices indices = {{5, 0, 0, 0}, {5, 0, 0, 0}};
        std::vector<long long> unmapped;
        AGRC::VGComponentMerge::Indices out =
            AGRC::VGComponentMerge::remapIndices(indices, weights, AGRC::VGRemap({{0, 64}}), unmapped);

        check(out[0][0] == -6, "an unmapped group 5 becomes the -index-1 sentinel, -6");
        check(unmapped == std::vector<long long>{5}, "and is reported once, not once per vertex");
    }

    // ---- padding goes at the END, where a missing TEXCOORD1 sits ----
    {
        AGRC::ByteVec src = {1, 2, 3, 4, 5, 6};                 // 2 lines of 3
        AGRC::ByteVec out = AGRC::VGComponentMerge::padLines(src, 2, 5);
        check(out == AGRC::ByteVec({1, 2, 3, 0, 0, 4, 5, 6, 0, 0}), "each line is zero-padded at its end");
        check(AGRC::VGComponentMerge::padLines(src, 2, 3) == src, "a buffer already at the stride is returned as is");
    }

    // ---- the merge itself ----
    {
        std::vector<AGRC::VGComponentMerge::Component> components;
        // Body: 3 vertices, texcoord stride 4 (the wide one)
        components.push_back(makeComponent("Body", {{0, 10}, {1, 11}},
                                           {{1.0, 0.0, 0.0, 0.0}, {1.0, 0.0, 0.0, 0.0}, {1.0, 0.0, 0.0, 0.0}},
                                           {{0, 0, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}}, 40, 4, 0xAA));
        // Bang: 2 vertices, texcoord stride 2 (the short one -- padded up)
        components.push_back(makeComponent("Bang", {{0, 64}},
                                           {{1.0, 0.0, 0.0, 0.0}, {1.0, 0.0, 0.0, 0.0}},
                                           {{0, 0, 0, 0}, {0, 0, 0, 0}}, 40, 2, 0xBB));
        AGRC::VGComponentMerge merge(std::move(components));

        check(merge.vertexCount() == 5, "the merged vertex count is every component's, summed");
        check(merge.offsetOf("Body") == 0, "the first component takes offset 0");
        check(merge.offsetOf("Bang") == 3, "the second starts after the first's vertices");
        check(merge.stats().texcoordStride == 4, "the merged texcoord stride is the widest component's");
        check(merge.stats().paddedComponents == 1, "only the short component was padded");
        check(merge.texcoord().size() == 5 * 4, "the merged texcoord is every vertex at the merged stride");
        check(merge.position().size() == 5 * 40, "the merged position is every vertex at 40");
        check(merge.blend().size() == 5 * 32, "the merged blend is every vertex at 32");

        check(indicesOf(merge.blend(), 1) == std::vector<long long>({11, 0, 0, 0}),
              "each component's blend went through its OWN remap (Body 1 -> 11)");
        check(indicesOf(merge.blend(), 3) == std::vector<long long>({64, 0, 0, 0}),
              "and the Bang's 0 -> 64, not the Body's 0 -> 10");

        // the short component's own bytes survive the padding, at the front of each line
        check(merge.texcoord()[3 * 4] == 0xBB && merge.texcoord()[3 * 4 + 2] == 0x00,
              "a padded line keeps its own bytes and zeroes the rest");

        // ---- index buffers: offset, and concatenated when two objects share a target ----
        AGRC::VGComponentMerge::Triangles bodyIb = {{0, 1, 2}};
        AGRC::VGComponentMerge::Triangles bangIb = {{0, 1, 0}};
        check(merge.mergeIbs({{"Body", bodyIb}}) == AGRC::VGComponentMerge::Triangles({{0, 1, 2}}),
              "the first component's index buffer passes through unchanged");
        check(merge.mergeIbs({{"Bang", bangIb}}) == AGRC::VGComponentMerge::Triangles({{3, 4, 3}}),
              "a later component's indices shift by its offset");
        check(merge.mergeIbs({{"Body", bodyIb}, {"Bang", bangIb}})
                  == AGRC::VGComponentMerge::Triangles({{0, 1, 2}, {3, 4, 3}}),
              "two objects on one target object concatenate into a single draw");

        bool threw = false;
        try {
            merge.mergeIbs({{"Eye", bangIb}});
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        check(threw, "a member naming a component that was not merged is an error, not a silent 0 offset");
    }

    // ---- the unmapped report names its component ----
    {
        std::vector<AGRC::VGComponentMerge::Component> components;
        components.push_back(makeComponent("Body", {{0, 10}}, {{1.0, 0.0, 0.0, 0.0}}, {{0, 0, 0, 0}}, 40, 4, 0));
        components.push_back(makeComponent("Bang", {}, {{1.0, 0.0, 0.0, 0.0}}, {{2, 0, 0, 0}}, 40, 4, 0));
        AGRC::VGComponentMerge merge(std::move(components));

        check(merge.stats().unmapped.size() == 1 && merge.stats().unmapped[0].first == "Bang"
                  && merge.stats().unmapped[0].second == 2,
              "an unmapped group is reported against the component it came from");
    }

    std::cout << (failures == 0 ? "\nALL PASSED\n" : "\n" + std::to_string(failures) + " FAILED\n");
    return failures == 0 ? 0 : 1;
}
