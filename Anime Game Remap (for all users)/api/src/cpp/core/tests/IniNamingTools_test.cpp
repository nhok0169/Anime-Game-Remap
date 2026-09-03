// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniNamingTools
// (model/IniNamingTools.h) and its two new dependencies, AGRemapCore::TextTools
// (tools/TextTools.h, partial port: capitalize/reverse only) and the
// AGRemapCore::IniKeywords/AGRemapCore::FileExt constants (constants/IniKeywords.h,
// partial; constants/FileExt.h, full).
//
// Covers, against the pure-Python original's documented/actual behavior
// (model/IniNamingTools.py, tools/TextTools.py) -- every non-trivial expected
// value below was cross-checked by actually running the equivalent Python
// snippet (see the chat transcript), not just read from the docstrings:
//   * getResourceName / removeResourceName: prefix add/remove, idempotent
//   * getRemapElementName / getRemapBlendName / getRemapPositionName /
//     getRemapTexcoordName / getRemapIbName: replaces the LAST occurrence of
//     the element keyword, or appends if not found
//   * getModSuffixedName / getRemapFixName / getRemapTexName / getRemapDLName:
//     the three-way already-suffixed / suffix-only / neither branches --
//     INCLUDING the confirmed Python-original bug in the suffix-only branch,
//     which this port fixes (per explicit maintainer direction) rather than
//     reproduces. testRemapFixNameBugFix documents the exact before/after.
//   * getRemapFixResourceName / getRemapTexResourceName / getRemapDLResourceName /
//     getRemapBlendResourceName / getRemapPositionResourceName: Resource-prefix
//     composition
//   * getFixedFile vs getFixedElementFile: the pathlib-parent "." fallback,
//     and the deliberate difference in whether a bare filename (no directory)
//     gets a "./" folder prefix in the result
//   * getFixedBlendFile / getFixedPositionFile: fileExt override to ".buf"
//   * getFixedTexFile: the os.path.dirname/basename-based (not pathlib-based)
//     implementation, including its own "rsplit on last '.', not
//     std::filesystem::path::stem()" extension-stripping rule
//   * getTextureOverrideRemapFix: capitalize + concatenate + getRemapFixName
//   * getObjRemapFixName: case-insensitive last-occurrence replace, and the
//     not-found fallback appending modName+newObjName
//   * TextTools::capitalize / TextTools::reverse: ASCII + a non-ASCII
//     (multi-byte UTF-8) case for each, confirming codepoint-level (not
//     byte-level) correctness
//
// This file has NO dependency on the project's build system (CMake/pybind11),
// Z3, ordered-map, or xxHash -- only utf8proc (for TextTools) and the standard
// library's <filesystem>. Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <utf8proc-src> ^
//      IniNamingTools_test.cpp <utf8proc-src>/utf8proc.c ^
//      <core>/src/tools/TextTools.cpp <core>/src/model/IniNamingTools.cpp ^
//      <core>/src/tools/StringTools.cpp <core>/src/tools/StringHash.cpp ^
//      <core>/src/tools/grapheme/GraphemeIterator.cpp <core>/src/tools/grapheme/GraphemeRange.cpp ^
//      /Fe:test.exe
//
// (IniNamingTools's case-insensitive matching goes through the grapheme-aware StringTools now,
//  hence the StringTools/StringHash/grapheme sources on that line -- see
//  StringTools_grapheme_test.cpp for the non-ASCII coverage of that path.)
//
// (g++/clang++ equivalent: swap /std:c++latest /EHsc /I /Fe: for
//  -std=c++23 -I ... -o test.exe, and -DUTF8PROC_STATIC likely isn't needed)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/tools/TextTools.h"

#include <cstdio>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

using AGRemapCore::IniNamingTools;
using AGRemapCore::TextTools;

namespace {

int failures = 0;

void check(const std::string& actual, const std::string& expected, const char* description) {
    if (actual == expected) {
        std::printf("[PASS] %s\n", description);
    } else {
        std::printf("[FAIL] %s -- got %s, expected %s\n", description, actual.c_str(), expected.c_str());
        failures++;
    }
}

// Builds an expected folder-joined path the same way std::filesystem::path::operator/ would --
// used so expected values in testFixedFilePaths carry the platform's own native separator
// (backslash on Windows, forward slash elsewhere) instead of a hardcoded guess, matching however
// IniNamingTools itself actually joined the path.
std::string join(const char* folder, const std::string& name) {
    return (std::filesystem::path(folder) / name).string();
}

void testResourceName() {
    check(IniNamingTools::getResourceName("CuteLittleEi"), "ResourceCuteLittleEi", "getResourceName adds prefix");
    check(IniNamingTools::getResourceName("ResourceCuteLittleEi"), "ResourceCuteLittleEi", "getResourceName is idempotent");
    check(IniNamingTools::removeResourceName("ResourceCuteLittleEi"), "CuteLittleEi", "removeResourceName strips prefix");
    check(IniNamingTools::removeResourceName("LittleMissGanyu"), "LittleMissGanyu", "removeResourceName leaves non-prefixed unchanged");
}

void testRemapElementName() {
    check(IniNamingTools::getRemapElementName("EiTriesToUseBlenderAndFails", "Blend", "Raiden"),
          "EiTriesToUseRaidenRemapBlenderAndFails", "getRemapElementName replaces last occurrence");
    check(IniNamingTools::getRemapElementName("EiTextsTheTexture", "Tex", "Yae"),
          "EiTextsTheYaeRemapTexture", "getRemapElementName replaces last occurrence #2");
    check(IniNamingTools::getRemapElementName("ResourceCuteLittleEi", "Position", "Raiden"),
          "ResourceCuteLittleEiRaidenRemapPosition", "getRemapElementName appends when not found");
    check(IniNamingTools::getRemapElementName("ResourceCuteLittleEiRemapDango", "Dango", "Raiden"),
          "ResourceCuteLittleEiRemapRaidenRemapDango", "getRemapElementName replaces last occurrence #3");

    check(IniNamingTools::getRemapBlendName("TextureOverrideEiBlend", "Raiden"),
          "TextureOverrideEiRaidenRemapBlend", "getRemapBlendName");
    check(IniNamingTools::getRemapPositionName("TextureOverrideEiPosition", "Raiden"),
          "TextureOverrideEiRaidenRemapPosition", "getRemapPositionName");
    check(IniNamingTools::getRemapTexcoordName("TextureOverrideEiTexcoord", "Raiden"),
          "TextureOverrideEiRaidenRemapTexcoord", "getRemapTexcoordName");
    check(IniNamingTools::getRemapIbName("TextureOverrideEiIB", "Raiden"),
          "TextureOverrideEiRaidenRemapIB", "getRemapIbName uses literal \"IB\"");
}

void testRemapFixNameBugFix() {
    // Cross-checked against the real Python original: it currently returns
    // "EiIsDoneRaidenRemapFix" here (a confirmed bug -- see the file header),
    // contradicting its own docstring example of "EiIsDoneWithRaidenRemapFix".
    // This C++ port implements the documented/intended behavior, per explicit
    // maintainer direction.
    check(IniNamingTools::getRemapFixName("EiIsDoneWithRemapFix", "Raiden"),
          "EiIsDoneWithRaidenRemapFix", "getRemapFixName: suffix-only branch uses the FIXED (not buggy) behavior");
    check(IniNamingTools::getRemapFixName("EiIsHappy", "Raiden"),
          "EiIsHappyRaidenRemapFix", "getRemapFixName: neither branch appends");
    check(IniNamingTools::getRemapFixName("EiIsDoneWithRaidenRemapFix", "Raiden"),
          "EiIsDoneWithRaidenRemapFix", "getRemapFixName: already-suffixed branch is unchanged");

    check(IniNamingTools::getRemapTexName("EiIsDoneWithRemapTex", "Raiden"),
          "EiIsDoneWithRaidenRemapTex", "getRemapTexName: suffix-only branch");
    check(IniNamingTools::getRemapTexName("EiIsHappy", "Raiden"),
          "EiIsHappyRaidenRemapTex", "getRemapTexName: neither branch");

    check(IniNamingTools::getRemapDLName("EiIsDoneWithRemapDL", "Raiden"),
          "EiIsDoneWithRaidenRemapDL", "getRemapDLName: suffix-only branch");
    check(IniNamingTools::getRemapDLName("EiIsHappy", "Raiden"),
          "EiIsHappyRaidenRemapDL", "getRemapDLName: neither branch");
}

void testResourceNameComposition() {
    check(IniNamingTools::getRemapFixResourceName("EiIsHappy", "Raiden"),
          "ResourceEiIsHappyRaidenRemapFix", "getRemapFixResourceName");
    check(IniNamingTools::getRemapTexResourceName("EiIsHappy", "Raiden"),
          "ResourceEiIsHappyRaidenRemapTex", "getRemapTexResourceName");
    check(IniNamingTools::getRemapDLResourceName("EiIsHappy", "Raiden"),
          "ResourceEiIsHappyRaidenRemapDL", "getRemapDLResourceName");
    check(IniNamingTools::getRemapBlendResourceName("TextureOverrideEiBlend", "Raiden"),
          "ResourceTextureOverrideEiRaidenRemapBlend", "getRemapBlendResourceName");
    check(IniNamingTools::getRemapPositionResourceName("TextureOverrideEiPosition", "Raiden"),
          "ResourceTextureOverrideEiRaidenRemapPosition", "getRemapPositionResourceName");
}

void testFixedFilePaths() {
    // All expected values here were cross-checked against the real Python original (running its
    // exact pathlib/os.path logic, with the confirmed getModSuffixedName bug fixed the same way
    // this port fixes it) rather than hand-derived, specifically because the "stem" transform
    // (getRemapFixName/getRemapElementName operate on the file's BASE NAME, not the whole path)
    // and the folder-prefix rules are easy to get wrong by inspection alone.

    // getFixedFile: bare filename -> "./"-prefixed (pathlib parent == ".", always joined).
    check(IniNamingTools::getFixedFile("foo.ini", "Raiden"), join(".", "fooRaidenRemapFix.ini"),
          "getFixedFile: bare filename gets a \"./\" folder prefix, base name kept + suffixed");
    check(IniNamingTools::getFixedFile("a/b/foo.ini", "Raiden"), join("a/b", "fooRaidenRemapFix.ini"),
          "getFixedFile: nested path keeps its folder");
    check(IniNamingTools::getFixedFile("foo.tar.gz", "Raiden"), join(".", "foo.tarRaidenRemapFix.gz"),
          "getFixedFile: only the LAST extension is treated as the extension (stem keeps \".tar\")");
    check(IniNamingTools::getFixedFile("foo.ini", "Raiden", std::string(".txt")), join(".", "fooRaidenRemapFix.txt"),
          "getFixedFile: explicit fileExt overrides the original extension");

    // getFixedElementFile: bare filename -> NO folder prefix (deliberately different from getFixedFile).
    check(IniNamingTools::getFixedElementFile("foo.buf", "Blend", "Raiden"), "fooRaidenRemapBlend.buf",
          "getFixedElementFile: bare filename gets NO folder prefix");
    check(IniNamingTools::getFixedElementFile("a/b/foo.buf", "Blend", "Raiden"), join("a/b", "fooRaidenRemapBlend.buf"),
          "getFixedElementFile: nested path keeps its folder");

    check(IniNamingTools::getFixedBlendFile("a/b/EiBlend.buf", "Raiden"), join("a/b", "EiRaidenRemapBlend.buf"),
          "getFixedBlendFile: forces .buf extension via getFixedElementFile");
    check(IniNamingTools::getFixedPositionFile("a/b/EiPosition.buf", "Raiden"), join("a/b", "EiRaidenRemapPosition.buf"),
          "getFixedPositionFile: forces .buf extension via getFixedElementFile");

    // getFixedTexFile: os.path.dirname/basename-based -- bare filename also gets no folder prefix,
    // but arrived at through a different code path than getFixedElementFile.
    check(IniNamingTools::getFixedTexFile("foo.dds", "Raiden"), "fooRaidenRemapTex.dds",
          "getFixedTexFile: bare filename gets NO folder prefix");
    check(IniNamingTools::getFixedTexFile("a/b/foo.tar.dds", "Raiden"), join("a/b", "foo.tarRaidenRemapTex.dds"),
          "getFixedTexFile: only the LAST '.' is the extension boundary (manual rsplit, not path::stem())");
    check(IniNamingTools::getFixedTexFile("noext", "Raiden"), "noextRaidenRemapTex.dds",
          "getFixedTexFile: a file with no extension at all is handled (no '.' found)");
}

void testTextureOverrideRemapFix() {
    // getTextureOverrideRemapFix calls getRemapFixName with NO modName argument (default ""), even
    // though 'modName' was already folded into the name itself -- so the result is suffixed with
    // plain "RemapFix", not "{modName}RemapFix". Confirmed against the real Python original.
    check(IniNamingTools::getTextureOverrideRemapFix("head", "ei", "raiden"),
          "TextureOverrideRaidenHeadEiRemapFix", "getTextureOverrideRemapFix: modName only capitalized+prefixed, RemapFix suffix has no modName");
}

void testObjRemapFixName() {
    // Cross-checked directly against the real Python original.
    check(IniNamingTools::getObjRemapFixName("TextureOverrideHeadEiBlend", "raiden", {"Head", "Ei"}, {"Body", "Ei"}),
          "TextureOverrideBodyEiBlendRaidenRemapFix", "getObjRemapFixName: found, case-sensitive-exact match replaced");
    check(IniNamingTools::getObjRemapFixName("TextureOverrideEiBlend", "raiden", {"Head", "Ei"}, {"Body", "Ei"}),
          "TextureOverrideEiBlendRaidenBodyEiRemapFix", "getObjRemapFixName: not found, falls back to modName+newObjName suffix");
    check(IniNamingTools::getObjRemapFixName("TextureOverrideheadEIBlend", "raiden", {"Head", "Ei"}, {"Body", "Ei"}),
          "TextureOverrideBodyEiBlendRaidenRemapFix", "getObjRemapFixName: match is case-INsensitive");
}

void testTextTools() {
    check(TextTools::capitalize(""), "", "capitalize: empty stays empty");
    check(TextTools::capitalize("a"), "A", "capitalize: single ASCII char");
    check(TextTools::capitalize("ei"), "Ei", "capitalize: ASCII word");
    check(TextTools::capitalize("Ei"), "Ei", "capitalize: already-capitalized ASCII word");
    // e9 (Latin Small Letter E with Acute, U+00E9, UTF-8 "\xC3\xA9") -> C9 (U+00C9, "\xC3\x89"),
    // a genuine multi-byte-UTF-8 case -- confirms this doesn't corrupt/truncate the character or
    // silently fall back to ASCII-only handling. Cross-checked: Python's "élan".upper() ==
    // "ÉLAN" (leading \xc9 is U+00C9, capital E-acute).
    check(TextTools::capitalize("\xC3\xA9lan"), "\xC3\x89lan", "capitalize: multi-byte UTF-8 leading character (e-acute)");

    check(TextTools::reverse(""), "", "reverse: empty stays empty");
    check(TextTools::reverse("abc"), "cba", "reverse: ASCII");
    // "\xC3\xA9cole" (e-acute + "cole") reversed by CODEPOINT is "elocé" ("e","l","o","c","é"), i.e.
    // "eloc" + the SAME 2-byte e-acute sequence at the end -- NOT a byte-reversal (which would
    // corrupt the 2-byte character into two separate invalid bytes at opposite ends of the string).
    check(TextTools::reverse("\xC3\xA9" "cole"), "eloc" "\xC3\xA9", "reverse: multi-byte UTF-8 char reverses as one codepoint, not as raw bytes");
}

}  // namespace

int main() {
    testResourceName();
    testRemapElementName();
    testRemapFixNameBugFix();
    testResourceNameComposition();
    testFixedFilePaths();
    testTextureOverrideRemapFix();
    testObjRemapFixName();
    testTextTools();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
