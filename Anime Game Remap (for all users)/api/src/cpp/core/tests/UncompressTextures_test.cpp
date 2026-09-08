// -----------------------------------------------------------------------------
// Standalone regression test for RemapService::uncompressTextures -- the
// --uncompressTextures command line option, from the model flag down to the
// bytes on disk.
//
// WHY THIS FILE EXISTS: this option's entire failure mode is the one this repo
// specialises in -- a flag that is set, stored, and read by nobody.
//
// The happy path is NOT what this file is for. A real CLI run covers that, and
// should be the first thing you reach for: `-s <a copy of
// Testing/Integration Tester/.../inputs/multiFix/select/Jean> -d` with and
// without `-c` writes SmollerJeanRemapTex.dds at 2852 vs 10128 bytes and
// CuteJean/JeanHeadLightMapRemapDLRemapTex.dds at 1048724 vs 4194432 (exactly
// 4x -- RGBA8 against BC), because Jean's fixer carries a real texEdits row.
// See AI Agent Help/Testing/CLAUDE.md's "Real mod data".
//
// What that run cannot show is everything AROUND the happy path, which is what
// is left here:
//
//   * the SEAM -- RemapService::_applyUncompressTextures, reached through a
//     subclass (it is protected for exactly this reason). Pins that the flag
//     reaches both texture resources, that it does nothing when OFF, and that a
//     non-texture resource is left alone. A real run only ever exercises one of
//     those three at a time, and the off case looks identical to "no texture was
//     written" from the outside
//   * the ONE-WAY rule -- an edit that already asked for no compression must not
//     be switched back on. No shipped character asks for that today, so no run
//     over real data can catch a regression in it
//   * RemapTexAddResource/TexCreator -- the *add* half. Jean only exercises the
//     *edit* half, so without this the option would be silently half-wired for
//     the first character that creates a texture rather than editing one
//   * the BYTES, at the writer rather than through the fix -- TexEditor and
//     TexCreator each writing a .dds both ways, so a failure here says which of
//     the two broke rather than just "the file came out wrong"
//
// NOT wired into any build target (core/tests/*.cpp never is -- no CMake entry,
// no CTest, not run by CI or by main.py). Compile and run it by hand; the
// static-lib link line in AI Agent Help/Building/CLAUDE.md works for it.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "AGRemapCore/RemapService.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/textures/Colour.h"

using namespace AGRemapCore;

namespace {

int failures = 0;

void check(bool condition, const std::string& description) {
    if (condition) {
        std::printf("[PASS] %s\n", description.c_str());
    } else {
        std::printf("[FAIL] %s\n", description.c_str());
        failures++;
    }
}

std::filesystem::path scratchRoot() {
    std::filesystem::path root = std::filesystem::temp_directory_path() / "AGRemap_UncompressTextures_test";
    std::error_code ec;
    std::filesystem::create_directories(root, ec);
    return root;
}

// Reaches the protected seam. RemapServiceCLI_test.cpp subclasses the CLI the same way.
class TestableRemapService: public RemapService {
    public:
        using RemapService::RemapService;
        using RemapService::_applyUncompressTextures;
};

// Something that is emphatically not a texture, to prove the seam is selective rather than
// reaching for a member on whatever it is handed.
class PlainResource: public IniResource {
    public:
        PlainResource(): IniResource("plain", ".", "plain.txt") {}
};

std::uintmax_t fileSize(const std::filesystem::path& p) {
    std::error_code ec;
    std::uintmax_t size = std::filesystem::file_size(p, ec);
    return ec ? 0 : size;
}


// ----- the seam -----

void testFlagOnReachesBothTextureWriters() {
    TestableRemapService service;
    service.uncompressTextures = true;

    RemapTexEditResource edit(".", "src.dds", "fixed.dds", TexEditor({}, true));
    check(edit.texEditor.getCompress(), "edit resource starts out compressing (the state worth changing)");
    service._applyUncompressTextures(edit);
    check(!edit.texEditor.getCompress(), "uncompressTextures=true turns a texture EDIT's compression off");

    RemapTexAddResource add(".", "made.dds", TexCreator(4, 4, Colour(), true));
    check(add.texCreator.compress, "add resource starts out compressing");
    service._applyUncompressTextures(add);
    check(!add.texCreator.compress, "uncompressTextures=true turns a texture ADD's compression off");
}

void testFlagOffChangesNothing() {
    TestableRemapService service;
    check(!service.uncompressTextures, "uncompressTextures defaults to false");

    RemapTexEditResource edit(".", "src.dds", "fixed.dds", TexEditor({}, true));
    service._applyUncompressTextures(edit);
    check(edit.texEditor.getCompress(), "uncompressTextures=false leaves a texture edit's own answer alone");

    RemapTexAddResource add(".", "made.dds", TexCreator(4, 4, Colour(), true));
    service._applyUncompressTextures(add);
    check(add.texCreator.compress, "uncompressTextures=false leaves a texture add's own answer alone");
}

// The override is ONE-WAY on purpose -- see RemapService::uncompressTextures. An edit that
// deliberately asked for no compression must not be turned back on by a run that did not ask for it.
void testOverrideIsOneWay() {
    TestableRemapService service;
    service.uncompressTextures = true;

    RemapTexEditResource edit(".", "src.dds", "fixed.dds", TexEditor({}, false));
    service._applyUncompressTextures(edit);
    check(!edit.texEditor.getCompress(), "an edit that already asked for no compression stays that way");
}

void testNonTextureResourceIsUntouched() {
    TestableRemapService service;
    service.uncompressTextures = true;

    PlainResource plain;
    service._applyUncompressTextures(plain);
    check(true, "a resource that writes no texture is a no-op rather than a crash");
}

void testConstructorArgumentReachesTheMember() {
    RemapService on(std::nullopt, true, false, false, false, false, std::nullopt, std::nullopt, {},
                    false, std::nullopt, std::nullopt, std::nullopt, DownloadMode::Normal,
                    std::nullopt, true);
    check(on.uncompressTextures, "the constructor argument lands on the member");
}


// ----- the bytes -----

// Everything above would pass with a writer that ignored the flag entirely, so this half checks the
// only thing that actually matters to a user: the file on disk is different.
void testWrittenFileActuallyDiffers() {
    const std::filesystem::path root = scratchRoot();
    const std::filesystem::path compressed = root / "compressed.dds";
    const std::filesystem::path uncompressed = root / "uncompressed.dds";

    std::error_code ec;
    std::filesystem::remove(compressed, ec);
    std::filesystem::remove(uncompressed, ec);

    // 64x64 rather than anything realistic -- BCn encoding is the slow part, and this is about
    // which branch ran, not about throughput.
    const int width = 64;
    const int height = 64;

    TexCreator compressing(width, height, Colour(), true);
    TextureFile compressedFile(compressed.string());
    compressing.fix(compressedFile, compressed.string());

    TexCreator notCompressing(width, height, Colour(), false);
    TextureFile uncompressedFile(uncompressed.string());
    notCompressing.fix(uncompressedFile, uncompressed.string());

    const std::uintmax_t compressedSize = fileSize(compressed);
    const std::uintmax_t uncompressedSize = fileSize(uncompressed);

    check(compressedSize > 0, "TexCreator(compress=true) wrote a file");
    check(uncompressedSize > 0, "TexCreator(compress=false) wrote a file");

    // RGBA8 is 4 bytes/pixel; BC7 is 1. The exact ratio is not the point -- that the two branches
    // produce measurably different files is.
    check(uncompressedSize > compressedSize,
          "TexCreator(compress=false) writes a BIGGER file than compress=true -- the flag reaches the bytes");

    std::printf("       compressed=%llu bytes, uncompressed=%llu bytes\n",
                static_cast<unsigned long long>(compressedSize),
                static_cast<unsigned long long>(uncompressedSize));
}

// The same check for TexEditor, whose fix() takes a different route (it opens and re-encodes an
// existing file rather than inventing one).
void testTexEditorHonoursCompress() {
    const std::filesystem::path root = scratchRoot();
    const std::filesystem::path source = root / "editSrc.dds";
    const std::filesystem::path editedCompressed = root / "editedCompressed.dds";
    const std::filesystem::path editedUncompressed = root / "editedUncompressed.dds";

    std::error_code ec;
    for (const std::filesystem::path& p : {source, editedCompressed, editedUncompressed}) {
        std::filesystem::remove(p, ec);
    }

    // A compressed source to read back, built the same way as above.
    TextureFile sourceFile(source.string());
    TexCreator(32, 32, Colour(), true).fix(sourceFile, source.string());

    if (fileSize(source) == 0) {
        check(false, "could not build a source texture to edit -- the rest of this test is moot");
        return;
    }

    // A filter that changes nothing: TexEditor::fix returns early on an EMPTY filter list, so a
    // no-op filter is what makes it reach the save at all.
    const TexEditor::Filter noOp = [](TextureFile&) {};

    TextureFile compressedTarget(source.string());
    TexEditor({noOp}, true).fix(compressedTarget, editedCompressed.string());

    TextureFile uncompressedTarget(source.string());
    TexEditor({noOp}, false).fix(uncompressedTarget, editedUncompressed.string());

    const std::uintmax_t compressedSize = fileSize(editedCompressed);
    const std::uintmax_t uncompressedSize = fileSize(editedUncompressed);

    check(compressedSize > 0, "TexEditor(compress=true) wrote a file");
    check(uncompressedSize > 0, "TexEditor(compress=false) wrote a file");
    check(uncompressedSize > compressedSize,
          "TexEditor(compress=false) writes a BIGGER file than compress=true -- the flag reaches the bytes");

    std::printf("       edited compressed=%llu bytes, edited uncompressed=%llu bytes\n",
                static_cast<unsigned long long>(compressedSize),
                static_cast<unsigned long long>(uncompressedSize));
}

}


int main() {
    // Unbuffered, so a crash mid-run still shows which check it got to.
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    testFlagOnReachesBothTextureWriters();
    testFlagOffChangesNothing();
    testOverrideIsOneWay();
    testNonTextureResourceIsUntouched();
    testConstructorArgumentReachesTheMember();
    testWrittenFileActuallyDiffers();
    testTexEditorHonoursCompress();

    if (failures == 0) {
        std::printf("\nAll uncompressTextures tests passed.\n");
        return 0;
    }

    std::printf("\n%d uncompressTextures test(s) FAILED.\n", failures);
    return 1;
}
