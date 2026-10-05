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
// Standalone test for AGRemapCore::TexFxLayout -- which TexFx per-layout sub-command a remapped part calls.
//
// Pinned: every family switches both ways; the unsuffixed aliases mean what TexFx's own Main.ini /
// ComponentFriendlyBuiltinShaders.ini make them mean (T/Transparency/C/Component -> .0, the Natlan families -> .1); a
// call already on the asked layout is left alone (nullopt); case and surrounding spaces do not matter; anything that is
// not one of these families is never touched.
//
// NOTE: nothing builds core/tests/*.cpp; see VGComponentMerge_test.cpp's header for the command line.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <optional>
#include <string>

#include "AGRemapCore/data/IniFixData/TexFxLayout.h"

using AGRemapCore::TexFxLayout;

namespace {
    int failures = 0;

    void expect(const std::optional<std::string>& got, const std::optional<std::string>& want, const char* what) {
        if (got != want) {
            std::printf("FAIL: %s -- got '%s', want '%s'\n", what, got.value_or("<none>").c_str(), want.value_or("<none>").c_str());
            ++failures;
        }
    }
}

int main() {
    const std::string F = "CommandList\\TexFx\\";

    expect(TexFxLayout::retarget(F + "T.0", true), F + "T.1", "T.0 onto a normal-map slot");
    expect(TexFxLayout::retarget(F + "T.1", false), F + "T.0", "T.1 onto a plain slot");
    expect(TexFxLayout::retarget(F + "T.1", true), std::nullopt, "T.1 already on a normal-map slot");
    expect(TexFxLayout::retarget(F + "T.0", false), std::nullopt, "T.0 already on a plain slot");

    // the unsuffixed aliases
    expect(TexFxLayout::retarget(F + "T", true), F + "T.1", "bare T means .0");
    expect(TexFxLayout::retarget(F + "T", false), std::nullopt, "bare T is already plain");
    expect(TexFxLayout::retarget(F + "TN", false), F + "TN.0", "bare TN means .1");
    expect(TexFxLayout::retarget(F + "TN", true), std::nullopt, "bare TN is already normal-map");
    expect(TexFxLayout::retarget(F + "Component", true), F + "Component.1", "bare Component means .0");
    expect(TexFxLayout::retarget(F + "ComponentNatlan", false), F + "ComponentNatlan.0", "bare ComponentNatlan means .1");

    // every family
    for (const char* family : {"Transparency", "C", "TNat", "TransparencyNatlan", "CN", "CNat"}) {
        expect(TexFxLayout::retarget(F + family + ".0", true), F + family + ".1", family);
        expect(TexFxLayout::retarget(F + family + ".1", false), F + family + ".0", family);
    }

    // case, spaces, and what is not ours
    expect(TexFxLayout::retarget("  commandlist\\TEXFX\\t.0 ", true), F + "T.1", "case and spaces do not matter");
    expect(TexFxLayout::retarget(F + "ClearInstanceValues", true), std::nullopt, "not a per-layout sub-command");
    expect(TexFxLayout::retarget(F + "T.2", true), std::nullopt, "not a layout suffix");
    expect(TexFxLayout::retarget("CommandList\\global\\ORFix\\ORFix", true), std::nullopt, "not TexFx at all");
    expect(TexFxLayout::retarget(F + "TNX.0", true), std::nullopt, "a prefix of no family");

    // the edits exist, one per family per direction
    if (TexFxLayout::switches(true).size() != 10 || TexFxLayout::switches(false).size() != 10) {
        std::printf("FAIL: switches() makes one edit per family\n");
        ++failures;
    }

    if (failures == 0) {
        std::printf("TexFxLayout_test: all passed\n");
        return 0;
    }
    std::printf("TexFxLayout_test: %d failed\n", failures);
    return 1;
}
