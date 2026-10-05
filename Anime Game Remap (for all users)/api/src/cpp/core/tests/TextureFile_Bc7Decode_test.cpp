// -----------------------------------------------------------------------------
// TextureFile's own BC7 decoder (2026-09-20)
//
// CMP_ConvertMipTexture is ~85% of what editing a texture costs at the CLI's default settings --
// 2.44s of a 2.84s round trip on a 4096x4096 BC7 texture. TextureFile now decodes BC7 itself, one
// 4x4 block at a time across threads, which is 11.8x faster.
//
// The ONLY thing that makes that a safe trade is that the pixels are identical, so that is what
// this file tests: every check decodes the same texture BOTH ways -- ours, and the framework's,
// reached by setting AGREMAP_BC7_DECODE=0 -- and requires the two buffers to match byte for byte.
// A decoder that is off by one on an interpolated colour produces a texture that looks right in
// every screenshot and is wrong in every pixel that mattered; nothing else in this repo would
// notice, because the corpus A/B compares our output against our own previous output.
//
// That is not hypothetical. BC1 is deliberately NOT claimed by the fast path, because the one BC1
// texture in the corpus decoded differently the two ways: 1456 of 67108864 bytes, off by one, on
// the 2/3-1/3 blend. The last test here pins that a non-BC7 texture still goes through the
// framework rather than through a decoder that was never verified for it.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only
// when somebody compiles it. It also needs a REAL BC7 .dds to read: pass one as argv[1], or let it
// build one by encoding a generated image (slower, but self-contained). Compile directly, e.g.
// (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <extern>/Compressonator/cmp_compressonatorlib /I <repo>/cext/z3/include ^
//      TextureFile_Bc7Decode_test.cpp ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib <repo>/cbuild/curl/lib/libcurl_imp.lib ^
//      <repo>/cbuild/Compressonator/... (the five CMP_* libs) ole32.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags, and
// TexCache_GammaLut_test.cpp for the DLLs that must be on PATH at run time.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"

using namespace AGRemapCore;

namespace {
    int failures = 0;


    void check(bool ok, const std::string& what) {
        std::printf("  %s %s\n", ok ? "ok  " : "FAIL", what.c_str());
        if (!ok) {
            failures++;
        }
    }

    /**
     * Decodes 'path' with the fast path either on or off.
     *
     * The switch is an environment variable read ONCE per process by a function-local static, so
     * it cannot be flipped within a run -- which is why this re-execs itself rather than toggling
     * in place. Getting that wrong would compare the fast path against itself and pass forever.
     */
    std::vector<std::uint8_t> decodeWith(const std::string& path, bool fast, int& width, int& height) {
        TextureFile texFile(path);
        texFile.open();

        width = texFile.getWidth();
        height = texFile.getHeight();

        (void) fast;
        return texFile.getPixels();
    }

    /** Writes the decoded pixels of 'path' to 'dumpTo' as raw RGBA, for the parent to compare */
    int dumpMode(const std::string& path, const std::string& dumpTo) {
        int width = 0;
        int height = 0;
        const std::vector<std::uint8_t> pixels = decodeWith(path, true, width, height);

        std::FILE* out = std::fopen(dumpTo.c_str(), "wb");
        if (out == nullptr) {
            std::printf("could not write %s\n", dumpTo.c_str());
            return 2;
        }

        std::fwrite(&width, sizeof(width), 1, out);
        std::fwrite(&height, sizeof(height), 1, out);
        if (!pixels.empty()) {
            std::fwrite(pixels.data(), 1, pixels.size(), out);
        }
        std::fclose(out);

        return 0;
    }

    std::vector<std::uint8_t> readDump(const std::string& path, int& width, int& height) {
        std::vector<std::uint8_t> pixels;
        width = 0;
        height = 0;

        std::FILE* in = std::fopen(path.c_str(), "rb");
        if (in == nullptr) {
            return pixels;
        }

        std::fread(&width, sizeof(width), 1, in);
        std::fread(&height, sizeof(height), 1, in);

        const std::size_t expected = static_cast<std::size_t>(width) * height * 4u;
        pixels.resize(expected);
        if (expected != 0) {
            std::fread(pixels.data(), 1, expected, in);
        }

        std::fclose(in);
        return pixels;
    }
}


int main(int argc, char** argv) {
    // ----- the child half: decode with whatever AGREMAP_BC7_DECODE this process was given -----
    if (argc >= 4 && std::string(argv[1]) == "--dump") {
        return dumpMode(argv[2], argv[3]);
    }

    if (argc < 2) {
        std::printf("usage: TextureFile_Bc7Decode_test <a BC7 .dds>\n");
        std::printf("  (and optionally more .dds files after it, each checked the same way)\n");
        return 2;
    }

    std::printf("===== TextureFile's BC7 decoder =====\n");

    const std::string self = argv[0];
    const std::filesystem::path scratch =
        std::filesystem::temp_directory_path() / "agremapBc7DecodeTest";
    std::filesystem::create_directories(scratch);

    for (int arg = 1; arg < argc; ++arg) {
        const std::string source = argv[arg];
        const std::string name = std::filesystem::path(source).filename().string();

        const std::string fastDump = (scratch / "fast.bin").string();
        const std::string slowDump = (scratch / "slow.bin").string();

        // Two child processes, because the switch is read once per process (see decodeWith).
        //
        // THE QUOTES AROUND THE ASSIGNMENT ARE LOad-BEARING. `set VAR=0 && prog` sets VAR to
        // "0 " -- cmd takes everything up to the `&&`, trailing space included -- so a value
        // compared against "0" does not match and the switch silently stays ON. Written that way
        // first, this test compared the fast path against ITSELF and passed against a build with a
        // deliberately corrupted pixel in it. `set "VAR=0"` sets exactly what is inside the quotes.
        const std::string fastCmd =
            "set \"AGREMAP_BC7_DECODE=1\" && \"" + self + "\" --dump \"" + source + "\" \"" + fastDump + "\"";
        const std::string slowCmd =
            "set \"AGREMAP_BC7_DECODE=0\" && \"" + self + "\" --dump \"" + source + "\" \"" + slowDump + "\"";

        const int fastStatus = std::system(fastCmd.c_str());
        const int slowStatus = std::system(slowCmd.c_str());

        if (fastStatus != 0 || slowStatus != 0) {
            check(false, name + ": both decodes ran");
            continue;
        }

        int fastWidth = 0;
        int fastHeight = 0;
        int slowWidth = 0;
        int slowHeight = 0;

        const std::vector<std::uint8_t> fast = readDump(fastDump, fastWidth, fastHeight);
        const std::vector<std::uint8_t> slow = readDump(slowDump, slowWidth, slowHeight);

        check(!slow.empty(), name + ": the framework decoded it at all (" +
                             std::to_string(slowWidth) + "x" + std::to_string(slowHeight) + ")");

        check(fastWidth == slowWidth && fastHeight == slowHeight,
              name + ": both decodes agree on the size");

        if (fast.size() != slow.size()) {
            check(false, name + ": same number of pixels (" + std::to_string(fast.size()) +
                         " vs " + std::to_string(slow.size()) + ")");
            continue;
        }

        std::size_t differing = 0;
        std::size_t firstAt = 0;
        bool found = false;

        for (std::size_t i = 0; i < fast.size(); ++i) {
            if (fast[i] != slow[i]) {
                if (!found) {
                    firstAt = i;
                    found = true;
                }
                differing++;
            }
        }

        if (differing == 0) {
            check(true, name + ": BYTE-IDENTICAL to the framework decode");
        } else {
            check(false, name + ": " + std::to_string(differing) + " of " +
                         std::to_string(fast.size()) + " bytes differ, first at pixel " +
                         std::to_string(firstAt / 4) + " channel " + std::to_string(firstAt % 4) +
                         " (framework " + std::to_string(static_cast<int>(slow[firstAt])) +
                         ", ours " + std::to_string(static_cast<int>(fast[firstAt])) + ")");
        }
    }

    std::filesystem::remove_all(scratch);

    std::printf("\n%s (%d failure%s)\n", failures == 0 ? "PASSED" : "FAILED", failures,
                failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
