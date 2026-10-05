// -----------------------------------------------------------------------------
// TexCache, the gamma lookup table, and the uncompressed .dds writer (2026-09-20)
//
// Three texture-path optimisations, all of which claim to be BYTE-IDENTICAL to what came before.
// That claim is the whole point of them, so it is what this file tests -- a faster path that
// changes a pixel is not an optimisation, it is a silent regression in every mod's textures.
//
// 1. GammaFilter's 256-entry lookup table. CorrectGamma::correctGamma is a static pure function
//    of an 8-bit channel value, so the table must agree with calling it directly for ALL 256
//    inputs, at several gammas. This is the one that would fail loudly against a table built with
//    a rounding shortcut or the wrong exponent -- the previous code called std::pow per pixel
//    (67 million times for a 4096x4096), and the table has to reproduce it exactly, not closely.
//
// 2. TexCache's write dedupe. Identical pixels + identical settings must come back as the path
//    already written; DIFFERENT pixels must not. The second half matters more: a cache that
//    returns a hit too eagerly hands one texture's bytes to another object.
//
// 3. The uncompressed .dds writer. Header shape and the RGBA -> BGRA swizzle, which is what the
//    file's own masks (R 0x00ff0000, B 0x000000ff) demand. Getting this backwards swaps red and
//    blue in every texture the fix writes, which no count and no file size can see.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only
// when somebody compiles it. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include /I <repo>/cext/Compressonator/include ^
//      TexCache_GammaLut_test.cpp ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags.
// -----------------------------------------------------------------------------

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "AGRemapCore/model/files/TexCache.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/CorrectGamma.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/GammaFilter.h"
#include "AGRemapCore/tools/hashing/Hash128.h"

using namespace AGRemapCore;

namespace {
    int failures = 0;


    void check(bool ok, const std::string& what) {
        std::printf("  %s %s\n", ok ? "ok  " : "FAIL", what.c_str());
        if (!ok) {
            ++failures;
        }
    }


    std::vector<std::uint8_t> solid(int width, int height, std::uint8_t r, std::uint8_t g,
                                     std::uint8_t b, std::uint8_t a) {
        std::vector<std::uint8_t> pixels;
        pixels.reserve(static_cast<std::size_t>(width) * height * 4);
        for (int i = 0; i < width * height; ++i) {
            pixels.push_back(r);
            pixels.push_back(g);
            pixels.push_back(b);
            pixels.push_back(a);
        }
        return pixels;
    }


    // ---- 1. the gamma table reproduces the per-pixel pow EXACTLY ----
    void testGammaTableMatchesPow() {
        std::printf("GammaFilter: the table agrees with std::pow for every input\n");

        // 1/2.2 is the sRGB pre-correction TextureFile::open sets; the others are there so a table
        // that happened to be right for one exponent cannot pass.
        const double gammas[] = {1.0 / 2.2, 2.2, 0.5, 1.0};

        for (double gamma : gammas) {
            // Every one of the 256 possible channel values, in a single row -- the filter has to
            // be right across the whole domain, not on the handful a sample texture happens to use.
            std::vector<std::uint8_t> pixels;
            for (int value = 0; value < 256; ++value) {
                const std::uint8_t v = static_cast<std::uint8_t>(value);
                pixels.push_back(v);
                pixels.push_back(v);
                pixels.push_back(v);
                pixels.push_back(static_cast<std::uint8_t>(255 - value));
            }

            TextureFile texFile("");
            texFile.setPixels(pixels, 256, 1);
            GammaFilter(gamma).transform(texFile);

            const std::vector<std::uint8_t> out = texFile.getPixels();

            bool allMatch = true;
            bool alphaUntouched = true;
            for (int value = 0; value < 256; ++value) {
                const int expected = CorrectGamma::correctGamma(value, gamma);
                const std::size_t at = static_cast<std::size_t>(value) * 4;

                if (out[at] != expected || out[at + 1] != expected || out[at + 2] != expected) {
                    allMatch = false;
                }

                // Alpha is a mask in these textures, not opacity -- the filter must leave it alone
                if (out[at + 3] != static_cast<std::uint8_t>(255 - value)) {
                    alphaUntouched = false;
                }
            }

            check(allMatch, "gamma " + std::to_string(gamma) + ": all 256 values match correctGamma");
            check(alphaUntouched, "gamma " + std::to_string(gamma) + ": alpha untouched");
        }
    }


    // ---- 2. the write dedupe answers on CONTENT, and only on identical content ----
    void testWriteDedupe() {
        std::printf("\nTexCache: write dedupe keys on content\n");

        TexCache cache;
        const Hash128 keyA = Hash128::hash(std::string_view("imageA"));
        const Hash128 keyB = Hash128::hash(std::string_view("imageB"));

        check(!cache.writtenAs(keyA).has_value(), "an unseen image is a miss");

        cache.rememberWritten(keyA, "first.dds");
        check(cache.writtenAs(keyA).value_or("") == "first.dds", "the same image comes back as a hit");
        check(!cache.writtenAs(keyB).has_value(), "a DIFFERENT image is still a miss");

        // The destination was rewritten with something else, so the entry naming it for the old
        // image describes bytes that file no longer holds
        cache.rememberWritten(keyB, "first.dds");
        check(!cache.writtenAs(keyA).has_value(),
               "rewriting a destination invalidates the entry that claimed it");
        check(cache.writtenAs(keyB).value_or("") == "first.dds", "and the new owner is a hit");

        cache.forgetWritten(keyB);
        check(!cache.writtenAs(keyB).has_value(), "forgetWritten drops it");
    }


    // ---- 2b. the decode cache holds, promotes and evicts by BYTES ----
    void testDecodeCacheBudget() {
        std::printf("\nTexCache: the decode half is bounded by bytes\n");

        TexCache::Decoded one;
        one.pixels.assign(600, 0);
        one.width = 10;
        one.height = 15;

        TexCache small(1000);
        const Hash128 a = Hash128::hash(std::string_view("a"));
        const Hash128 b = Hash128::hash(std::string_view("b"));

        small.rememberDecoded(a, one);
        check(small.decoded(a) != nullptr, "a decoded texture comes back");

        small.rememberDecoded(b, one);
        check(small.getBytes() <= 1000, "the budget is respected");
        check(small.decoded(a) == nullptr && small.decoded(b) != nullptr,
               "the least recently used one was evicted, not the newest");

        TexCache off(0);
        off.rememberDecoded(a, one);
        check(off.decoded(a) == nullptr, "a zero budget disables the decode half");

        // A single texture larger than the whole budget is skipped rather than evicting everything
        TexCache tiny(100);
        tiny.rememberDecoded(a, one);
        check(tiny.getBytes() == 0, "a texture bigger than the budget is not cached at all");
    }


    // ---- 3. the uncompressed writer: header shape and the BGRA swizzle ----
    void testUncompressedWriter() {
        std::printf("\nTextureFile: the uncompressed .dds writer\n");

        const std::filesystem::path dest =
            std::filesystem::temp_directory_path() / "AGRemapTexCacheTest.dds";

        TextureFile texFile(dest.string());
        // A colour whose channels are all different, so a swizzle mistake cannot hide
        texFile.setPixels(solid(4, 2, 10, 20, 30, 40), 4, 2);
        texFile.save(false, false);

        std::ifstream in(dest, std::ios::binary);
        const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),
                                               std::istreambuf_iterator<char>());
        in.close();

        auto read32 = [&bytes](std::size_t at) {
            return static_cast<std::uint32_t>(bytes[at])
                    | (static_cast<std::uint32_t>(bytes[at + 1]) << 8)
                    | (static_cast<std::uint32_t>(bytes[at + 2]) << 16)
                    | (static_cast<std::uint32_t>(bytes[at + 3]) << 24);
        };

        check(bytes.size() == 128 + (4 * 2 * 4), "size is the 128 byte header plus the pixels");
        check(bytes[0] == 'D' && bytes[1] == 'D' && bytes[2] == 'S' && bytes[3] == ' ', "magic");
        check(read32(4) == 124, "dwSize is 124");
        check(read32(12) == 2 && read32(16) == 4, "height and width, in that order");
        check(read32(28) == 1, "a single mip level");
        check(read32(88) == 32, "32 bits per pixel");
        check(read32(92) == 0x00FF0000u && read32(100) == 0x000000FFu,
               "the masks say BGRA (red high, blue low)");

        // ... and the payload has to actually BE in that order
        check(bytes[128] == 30 && bytes[129] == 20 && bytes[130] == 10 && bytes[131] == 40,
               "the first pixel is written B,G,R,A");

        std::error_code ec;
        std::filesystem::remove(dest, ec);
    }
}


int main() {
    testGammaTableMatchesPow();
    testWriteDedupe();
    testDecodeCacheBudget();
    testUncompressedWriter();

    std::printf("\n%s (%d failure%s)\n", failures == 0 ? "PASSED" : "FAILED", failures,
                 failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
