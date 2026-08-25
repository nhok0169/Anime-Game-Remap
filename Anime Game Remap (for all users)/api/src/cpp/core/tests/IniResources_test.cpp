// -----------------------------------------------------------------------------
// Standalone regression test for the AGRemapCore::iniresources port --
// model/stats/{FileStats,CachedFileStats,RemapStats}.h,
// tools/files/{FileService,FileDownload}.h,
// model/iniresources/{IniResourceModel,IniSrcResourceModel,IniFixResourceModel,
// IniTexModel,IniDownloadModel,IniResource,RemapIniResource,RemapBlendResource}.h.
//
// NOTE: model/iniresources/RemapTexResource.h (RemapTexAddResource) is NOT
// covered here -- it includes model/files/TextureFile.h, which pulls in
// Compressonator, a ~53MB dependency requiring its own CMake add_subdirectory
// build step (see AI Agent Help/Building/CLAUDE.md) -- too heavy for this
// throwaway standalone verification. It was reviewed by hand instead (it's a
// short, low-risk file reusing already-proven TextureFile/TexCreator
// machinery); a real CMake build is the way to confirm it actually compiles.
//
// Covers, against the maintainer's own step-by-step spec + explicit
// clarifications for this port (not a 1-1 translation of the deprecated
// pure-Python originals -- Mod is removed from RemapIniResource entirely, per
// the maintainer's direction, and FileDownload::download's real networking is
// deliberately stubbed):
//   * FileStats/CachedFileStats/RemapStats: add*/update*/clear semantics,
//     including skippedByMods auto-vivification and CachedFileStats::clear
//     cascading into the inherited FileStats::clear
//   * FileService::absPathOfRelPath: relative vs. already-absolute input
//   * FileDownload::get(): first call always downloads; a cached hit copies
//     instead of downloading; cache = false always re-downloads; a failed
//     copy (missing/deleted previous file) falls back to a fresh download
//   * IniSrcResourceModel/IniFixResourceModel: path -> absolute fullPath
//     resolution, items() flattening (including the orig-path
//     positional-alignment behavior), and clear()
//   * IniTexModel/IniDownloadModel: the added texEdits/downloads fields ride
//     along with their base class's path resolution correctly
//   * IniResource/IniFixResource: srcPath/fixedPath absolute resolution
//   * IniGroupedResource: addResource/isMissing, fix() dispatching to
//     fixFunc when set vs. the default no-op _fix() otherwise
//   * RemapIniResourceMixin: every method's true default (false), matching
//     the Python original's bare "pass" (implicit None/falsy) bodies
//   * RemapIniResource/RemapIniFixResource: hasRequired() == true,
//     fixExists() semantics (delegates to srcIsFixed for RemapIniResource;
//     real on-disk file check for RemapIniFixResource)
//   * RemapIniDownload: srcEncounteredError/srcIsFixed/fixEncounteredError/
//     fixIsFixed/fixExists against RemapStats.download; fix()/remapFix()
//     with a fake (non-networking) FileDownload subclass, confirming the
//     file is actually moved to srcPath, downloadStats.fixed/hit are updated
//     correctly, and remapFix's caller-supplied downloadHandler/
//     cacheHitHandler callbacks fire on the right branch (the direct
//     replacement for the removed Mod-based mod.print(...) logging)
//   * RemapBlendResource: srcEncounteredError/srcIsFixed/fixEncounteredError/
//     fixIsFixed against RemapStats.blend, and fix() dispatching to a custom
//     fixFunc override (BlendFile's own remap() binary-format correctness is
//     out of scope here -- it's a pre-existing, separately-verified class)
//
// Compile directly (BlendFile/VGRemap/BufElementType have no Compressonator
// dependency, so no extra setup beyond ordered-map is needed):
//
//   cl /std:c++latest /EHsc /nologo /I <core>/include /I <ordered-map>/include ^
//      IniResources_test.cpp ^
//      <core>/src/model/stats/FileStats.cpp <core>/src/model/stats/CachedFileStats.cpp ^
//      <core>/src/model/stats/RemapStats.cpp ^
//      <core>/src/tools/files/FileService.cpp <core>/src/tools/files/FileDownload.cpp ^
//      <core>/src/model/iniresources/IniSrcResourceModel.cpp ^
//      <core>/src/model/iniresources/IniFixResourceModel.cpp ^
//      <core>/src/model/iniresources/IniTexModel.cpp ^
//      <core>/src/model/iniresources/IniDownloadModel.cpp ^
//      <core>/src/model/iniresources/IniResource.cpp ^
//      <core>/src/model/iniresources/RemapIniResource.cpp ^
//      <core>/src/model/iniresources/RemapBlendResource.cpp ^
//      <core>/src/model/VGRemap.cpp <core>/src/model/files/BinaryFile.cpp ^
//      <core>/src/model/files/BufFile.cpp <core>/src/model/files/BufFileErrors.cpp ^
//      <core>/src/model/files/BlendFile.cpp ^
//      <core>/src/model/buffers/BufType.cpp <core>/src/model/buffers/BufDataType.cpp ^
//      <core>/src/model/buffers/BufInt.cpp <core>/src/model/buffers/BufFloat.cpp ^
//      <core>/src/model/buffers/BufUnorm.cpp <core>/src/model/buffers/BufElementType.cpp ^
//      /Fe:test.exe
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/iniresources/IniDownloadModel.h"
#include "AGRemapCore/model/iniresources/IniFixResourceModel.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/model/iniresources/IniSrcResourceModel.h"
#include "AGRemapCore/model/iniresources/IniTexModel.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/stats/CachedFileStats.h"
#include "AGRemapCore/model/stats/FileStats.h"
#include "AGRemapCore/model/stats/RemapStats.h"
#include "AGRemapCore/tools/files/FileDownload.h"
#include "AGRemapCore/tools/files/FileService.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace AGRemapCore;

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (condition) {
        std::printf("[PASS] %s\n", description);
    } else {
        std::printf("[FAIL] %s\n", description);
        failures++;
    }
}

std::string readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return content;
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path, std::ios::binary);
    f << content;
}

// ============ FileStats/CachedFileStats/RemapStats ============

void testFileStats() {
    FileStats stats;
    stats.addFixed("a.txt");
    stats.addSkipped("b.txt", std::make_exception_ptr(std::runtime_error("boom")), std::string("modFolder1"));
    stats.addSkipped("c.txt", std::make_exception_ptr(std::runtime_error("boom2")));  // no modFolder -> derived from dirname

    check(stats.fixed.contains("a.txt"), "FileStats::addFixed adds to fixed");
    check(stats.skipped.contains("b.txt") && stats.skipped.contains("c.txt"), "FileStats::addSkipped adds to skipped");
    check(stats.skippedByMods["modFolder1"].contains("b.txt"), "FileStats::addSkipped groups by explicit modFolder");
    check(stats.skippedByMods[std::filesystem::path("c.txt").parent_path().string()].contains("c.txt"),
          "FileStats::addSkipped auto-vivifies modFolder from dirname when not given");

    stats.clear();
    check(stats.fixed.empty() && stats.skipped.empty() && stats.skippedByMods.empty(), "FileStats::clear empties everything");
}

void testCachedFileStats() {
    CachedFileStats stats;
    stats.addFixed("a.txt");
    stats.addHit("b.txt");
    check(stats.fixed.contains("a.txt") && stats.hit.contains("b.txt"), "CachedFileStats tracks both fixed (inherited) and hit");

    stats.clear();
    check(stats.fixed.empty() && stats.hit.empty(), "CachedFileStats::clear cascades into the inherited FileStats::clear AND clears hit");
}

void testRemapStats() {
    RemapStats stats;
    stats.blend.addFixed("blend.buf");
    stats.download.addHit("dl.zip");
    stats.clear();
    check(stats.blend.fixed.empty() && stats.download.hit.empty(), "RemapStats::clear cascades into every member's own clear()");
}

// ============ FileService ============

void testFileService() {
    std::string abs = FileService::absPathOfRelPath("foo.ini", "C:/some/folder");
    check(abs.find("foo.ini") != std::string::npos, "absPathOfRelPath: relative path resolves against relFolder");

    std::string alreadyAbs = FileService::absPathOfRelPath("C:/already/abs/foo.ini", "C:/some/other/folder");
    check(alreadyAbs.find("already") != std::string::npos && alreadyAbs.find("other") == std::string::npos,
          "absPathOfRelPath: an already-absolute dstPath ignores relFolder");
}

// ============ FileDownload ============

class FakeFileDownload: public FileDownload {
    public:
        using FileDownload::FileDownload;
        int downloadCallCount = 0;

        std::string download(const std::string& folder, std::optional<std::string> proxy) override {
            (void)proxy;
            downloadCallCount++;
            std::string path = (std::filesystem::path(folder) / std::filesystem::path(filename).filename()).string();
            writeFile(path, "fake-download-content");
            return path;
        }
};

void testFileDownload(const std::string& scratchDir) {
    std::string folder = scratchDir + "/dl";
    std::filesystem::create_directories(folder);

    // First get(): no previous download -> always downloads.
    FakeFileDownload dl("http://example.invalid/file.zip", "file.zip", true);
    auto [path1, downloaded1, wasFirst1] = dl.get(folder);
    check(dl.downloadCallCount == 1, "FileDownload::get(): first call downloads");
    check(downloaded1, "FileDownload::get(): first call reports downloaded == true");
    check(std::filesystem::exists(path1), "FileDownload::get(): downloaded file actually exists");

    // Second get(), SAME folder, cache == true -> the freshly-computed target path is exactly
    // equal to the previous download's own path (both are "folder/basename(filename)"), so get()
    // takes its early "nothing to do" branch -- correctly not re-downloading, but without actually
    // exercising the copy_file call at all (that only happens when the folder DIFFERS -- see below).
    auto [path2, downloaded2, wasFirst2] = dl.get(folder);
    (void)wasFirst2;
    check(dl.downloadCallCount == 1, "FileDownload::get(): repeat call in the SAME folder short-circuits, no re-download");
    check(!downloaded2, "FileDownload::get(): repeat call in the SAME folder reports downloaded == false");
    check(path2 == path1, "FileDownload::get(): repeat call in the SAME folder returns the identical path");

    // Third get(), a DIFFERENT folder, cache == true -> now the freshly-computed target genuinely
    // differs from the previous path, so get() actually copies the previous download there instead
    // of re-downloading.
    std::string otherFolder = scratchDir + "/dl-other";
    std::filesystem::create_directories(otherFolder);
    auto [path3, downloaded3, wasFirst3] = dl.get(otherFolder);
    (void)wasFirst3;
    check(dl.downloadCallCount == 1, "FileDownload::get(): a different folder copies from the cache, still no re-download");
    check(!downloaded3, "FileDownload::get(): a different-folder cache hit reports downloaded == false");
    check(path3 != path1 && readFile(path3) == "fake-download-content", "FileDownload::get(): the file was actually copied into the new folder");

    // cache == false -> always re-downloads, even with a previous path.
    FakeFileDownload dlNoCache("http://example.invalid/file2.zip", "file2.zip", false);
    dlNoCache.get(folder);
    dlNoCache.get(folder);
    check(dlNoCache.downloadCallCount == 2, "FileDownload::get(): cache == false always re-downloads");

    // A failed copy (source of the copy deleted) falls back to a fresh download -- needs a
    // different folder each time too, for the same reason as above (same-folder always
    // short-circuits before ever attempting a copy).
    FakeFileDownload dlBroken("http://example.invalid/file3.zip", "file3.zip", true);
    auto [brokenPath1, brokenDownloaded1, brokenFirst1] = dlBroken.get(folder);
    (void)brokenDownloaded1;
    (void)brokenFirst1;
    std::filesystem::remove(brokenPath1);  // delete the source the copy attempt would read from

    std::string brokenOtherFolder = scratchDir + "/dl-broken-other";
    std::filesystem::create_directories(brokenOtherFolder);
    auto [brokenPath2, brokenDownloaded2, brokenFirst2] = dlBroken.get(brokenOtherFolder);
    (void)brokenFirst2;
    check(dlBroken.downloadCallCount == 2, "FileDownload::get(): a failed copy (missing source) triggers a fresh download");
    check(brokenDownloaded2, "FileDownload::get(): the fallback re-download is reported as downloaded == true");
    check(std::filesystem::exists(brokenPath2), "FileDownload::get(): the fallback re-download actually lands on disk");
}

// ============ IniSrcResourceModel / IniFixResourceModel / IniTexModel / IniDownloadModel ============

void testIniSrcResourceModel() {
    tsl::ordered_map<int, std::vector<std::string>> paths;
    paths.emplace(0, std::vector<std::string>{"a.buf", "b.buf"});
    paths.emplace(1, std::vector<std::string>{"c.buf"});

    IniSrcResourceModel model("C:/mods/EiRemap", paths);
    check(model.fullPaths.at(0)[0].find("a.buf") != std::string::npos, "IniSrcResourceModel: fullPaths resolved for part 0");
    check(model.fullPaths.at(1)[0].find("c.buf") != std::string::npos, "IniSrcResourceModel: fullPaths resolved for part 1");

    auto items = model.items();
    check(items.size() == 3, "IniSrcResourceModel::items(): flattens across all parts");
    check(items[0].first == "a.buf" && items[2].first == "c.buf", "IniSrcResourceModel::items(): preserves insertion order");
}

void testIniFixResourceModel() {
    tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fixedPaths;
    tsl::ordered_map<std::string, std::vector<std::string>> modPaths;
    modPaths.emplace("Raiden", std::vector<std::string>{"RaidenRemapBlend.buf"});
    fixedPaths.emplace(0, modPaths);

    tsl::ordered_map<int, std::vector<std::string>> origPaths;
    origPaths.emplace(0, std::vector<std::string>{"EiBlend.buf"});

    IniFixResourceModel model("C:/mods/EiRemap", fixedPaths, origPaths);

    auto items = model.items();
    check(items.size() == 1, "IniFixResourceModel::items(): one entry for one fixed path");
    check(items[0].fixedPath == "RaidenRemapBlend.buf", "IniFixResourceModel::items(): fixedPath preserved");
    check(items[0].origPath.has_value() && *items[0].origPath == "EiBlend.buf", "IniFixResourceModel::items(): origPath aligned positionally");
    check(items[0].origFullPath.has_value() && items[0].origFullPath->find("EiBlend.buf") != std::string::npos,
          "IniFixResourceModel::items(): origFullPath resolved");

    model.clear();
    check(model.fixedPaths.empty() && model.origPaths.has_value() && model.origPaths->empty(),
          "IniFixResourceModel::clear(): empties fixedPaths and origPaths (but origPaths stays present, not nullopt)");
}

void testIniFixResourceModelNoOrigPaths() {
    tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fixedPaths;
    tsl::ordered_map<std::string, std::vector<std::string>> modPaths;
    modPaths.emplace("Raiden", std::vector<std::string>{"RaidenRemapBlend.buf"});
    fixedPaths.emplace(0, modPaths);

    IniFixResourceModel model("C:/mods/EiRemap", fixedPaths);
    auto items = model.items();
    check(!items[0].origPath.has_value(), "IniFixResourceModel::items(): origPath is nullopt when origPaths wasn't given at all");
}

void testIniTexModel() {
    tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fixedPaths;
    tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::unique_ptr<BaseTexEditor>>>> texEdits;
    tsl::ordered_map<std::string, std::vector<std::unique_ptr<BaseTexEditor>>> modEdits;
    modEdits.emplace("Raiden", std::vector<std::unique_ptr<BaseTexEditor>>());
    modEdits.at("Raiden").push_back(std::make_unique<BaseTexEditor>());
    texEdits.emplace(0, std::move(modEdits));

    IniTexModel model("C:/mods/EiRemap", fixedPaths, std::move(texEdits));
    check(model.texEdits.at(0).at("Raiden").size() == 1, "IniTexModel: texEdits stored correctly");

    model.clear();
    check(model.texEdits.empty(), "IniTexModel::clear(): also clears texEdits");
}

void testIniDownloadModel() {
    tsl::ordered_map<int, std::vector<std::string>> paths;
    paths.emplace(0, std::vector<std::string>{"dl.zip"});

    tsl::ordered_map<int, std::vector<std::unique_ptr<FileDownload>>> downloads;
    std::vector<std::unique_ptr<FileDownload>> partDownloads;
    partDownloads.push_back(std::make_unique<FileDownload>("http://example.invalid/dl.zip", "dl.zip"));
    downloads.emplace(0, std::move(partDownloads));

    IniDownloadModel model("C:/mods/EiRemap", paths, std::move(downloads));
    check(model.downloads.at(0).size() == 1, "IniDownloadModel: downloads stored correctly");
    check(model.fullPaths.at(0)[0].find("dl.zip") != std::string::npos, "IniDownloadModel: inherited path resolution still works");
}

// ============ IniResource / IniFixResource / IniGroupedResource ============

void testIniResource() {
    IniResource res("blend", "C:/mods/EiRemap", "EiBlend.buf");
    check(res.type == "blend", "IniResource: type stored");
    check(res.srcPath.find("EiBlend.buf") != std::string::npos && res.srcPath.find("mods") != std::string::npos,
          "IniResource: srcPath resolved to an absolute path");

    IniFixResource fixRes("blend", "C:/mods/EiRemap", "EiBlend.buf", "RaidenRemapBlend.buf");
    check(fixRes.fixedPath.find("RaidenRemapBlend.buf") != std::string::npos, "IniFixResource: fixedPath resolved to an absolute path");
}

void testIniGroupedResource() {
    IniGroupedResource group("blendGroup");
    check(!group.fix(), "IniGroupedResource::fix(): default _fix() returns false when no fixFunc is set");

    group.addResource("blend", std::make_unique<IniResource>("blend", "C:/mods", "a.buf"));
    check(group.resources.contains("blend"), "IniGroupedResource::addResource(): adds the resource");
    check(!group.isMissing({"blend"}), "IniGroupedResource::isMissing(): false when the collected subset is fully present");
    check(group.isMissing({"blend", "position"}), "IniGroupedResource::isMissing(): true when something's missing from the subset");

    IniGroupedResource withFunc("g2", {}, [](IniGroupedResource& self) {
        (void)self;
        return true;
    });
    check(withFunc.fix(), "IniGroupedResource::fix(): dispatches to fixFunc when set, ignoring the default _fix()");
}

// ============ RemapIniResourceMixin / RemapIniResource / RemapIniFixResource ============

void testRemapIniResourceMixinDefaults() {
    RemapIniResourceMixin mixin;
    RemapStats stats;
    check(!mixin.srcEncounteredError(stats) && !mixin.srcIsFixed(stats) && !mixin.fixEncounteredError(stats) &&
              !mixin.fixIsFixed(stats) && !mixin.fixExists(stats) && !mixin.hasRequired(),
          "RemapIniResourceMixin: every method defaults to false, matching the Python original's bare \"pass\" bodies");
}

void testRemapIniResource() {
    RemapIniResource res("blend", "C:/mods", "a.buf");
    check(res.hasRequired(), "RemapIniResource::hasRequired() is always true");

    RemapStats stats;
    check(!res.fixExists(stats), "RemapIniResource::fixExists() == srcIsFixed() (false, nothing tracked yet)");
}

void testRemapIniFixResource(const std::string& scratchDir) {
    std::string fixedPath = scratchDir + "/RemapIniFixResource_test_fixed.buf";
    std::filesystem::remove(fixedPath);

    RemapIniFixResource res("blend", scratchDir, "a.buf", "RemapIniFixResource_test_fixed.buf");
    check(res.hasRequired(), "RemapIniFixResource::hasRequired() is always true");

    RemapStats stats;
    check(!res.fixExists(stats), "RemapIniFixResource::fixExists(): false before the fixed file exists on disk");

    writeFile(fixedPath, "x");
    check(res.fixExists(stats), "RemapIniFixResource::fixExists(): true once the fixed file actually exists on disk");

    std::filesystem::remove(fixedPath);
}

// ============ RemapIniDownload ============

void testRemapIniDownload(const std::string& scratchDir) {
    std::string modFolder = scratchDir + "/RemapIniDownload_mod";
    std::filesystem::create_directories(modFolder);
    std::string srcPath = modFolder + "/downloaded.zip";
    std::filesystem::remove(srcPath);

    auto fakeDownload = std::make_unique<FakeFileDownload>("http://example.invalid/downloaded.zip", "downloaded.zip", true);
    FakeFileDownload* fakeDownloadPtr = fakeDownload.get();

    RemapIniDownload res(modFolder, "downloaded.zip", std::move(fakeDownload));
    check(res.type == "download", "RemapIniDownload: default type is \"download\"");

    RemapStats stats;
    check(!res.srcEncounteredError(stats) && !res.srcIsFixed(stats), "RemapIniDownload: nothing tracked yet -- both false");

    std::string downloadedHandlerPath;
    std::string cacheHitHandlerPath;
    bool downloaded = res.remapFix(
        stats, std::nullopt, [&](const std::string& p) { downloadedHandlerPath = p; }, [&](const std::string& p) { cacheHitHandlerPath = p; });

    check(downloaded, "RemapIniDownload::remapFix(): first call reports a fresh download");
    check(fakeDownloadPtr->downloadCallCount == 1, "RemapIniDownload::remapFix(): actually calls FileDownload::download()");
    check(std::filesystem::exists(res.srcPath), "RemapIniDownload::remapFix(): the downloaded file ends up moved to srcPath");
    check(downloadedHandlerPath == res.srcPath, "RemapIniDownload::remapFix(): downloadHandler fires with srcPath on a fresh download");
    check(cacheHitHandlerPath.empty(), "RemapIniDownload::remapFix(): cacheHitHandler does NOT fire on a fresh download");
    check(stats.download.fixed.contains(res.srcPath), "RemapIniDownload::remapFix(): downloadStats.fixed is updated");
    check(res.srcIsFixed(stats), "RemapIniDownload::srcIsFixed(): reflects the updated stats");
    check(res.fixExists(stats), "RemapIniDownload::fixExists(): same as srcIsFixed() for this class");

    // Second call: FakeFileDownload's own get() caching kicks in (same folder, cache == true) --
    // no fresh download, so cacheHitHandler should fire instead.
    downloadedHandlerPath.clear();
    cacheHitHandlerPath.clear();
    bool downloadedAgain = res.remapFix(
        stats, std::nullopt, [&](const std::string& p) { downloadedHandlerPath = p; }, [&](const std::string& p) { cacheHitHandlerPath = p; });

    check(!downloadedAgain, "RemapIniDownload::remapFix(): second call is a cache hit, not a fresh download");
    check(cacheHitHandlerPath == res.srcPath, "RemapIniDownload::remapFix(): cacheHitHandler fires with srcPath on a cache hit");
    check(downloadedHandlerPath.empty(), "RemapIniDownload::remapFix(): downloadHandler does NOT fire on a cache hit");

    std::filesystem::remove_all(modFolder);
}

// ============ RemapBlendResource ============

void testRemapBlendResource() {
    RemapBlendResource res("C:/mods/EiRemap", "EiBlend.buf", "RaidenRemapBlend.buf", VGRemap());

    RemapStats stats;
    check(!res.srcEncounteredError(stats) && !res.srcIsFixed(stats) && !res.fixEncounteredError(stats) && !res.fixIsFixed(stats),
          "RemapBlendResource: nothing tracked yet -- all false");

    stats.blend.addFixed(res.srcPath);
    check(res.srcIsFixed(stats), "RemapBlendResource::srcIsFixed(): reflects stats.blend.fixed");

    stats.blend.addSkipped(res.fixedPath, std::make_exception_ptr(std::runtime_error("boom")));
    check(res.fixEncounteredError(stats), "RemapBlendResource::fixEncounteredError(): reflects stats.blend.skipped (keyed by fixedPath)");

    bool fixFuncCalled = false;
    RemapBlendResource resWithFunc("C:/mods/EiRemap", "EiBlend.buf", "RaidenRemapBlend.buf", VGRemap(), "resourceRemapBlend",
                                    [&](RemapBlendResource& self) {
                                        (void)self;
                                        fixFuncCalled = true;
                                        return true;
                                    });
    check(resWithFunc.fix(), "RemapBlendResource::fix(): dispatches to fixFunc when set");
    check(fixFuncCalled, "RemapBlendResource::fix(): fixFunc actually invoked, real BlendFile::remap() NOT called");
}

}  // namespace

int main(int argc, char** argv) {
    std::string scratchDir = ".";
    if (argc > 1) {
        scratchDir = argv[1];
    }

    testFileStats();
    testCachedFileStats();
    testRemapStats();
    testFileService();
    testFileDownload(scratchDir);
    testIniSrcResourceModel();
    testIniFixResourceModel();
    testIniFixResourceModelNoOrigPaths();
    testIniTexModel();
    testIniDownloadModel();
    testIniResource();
    testIniGroupedResource();
    testRemapIniResourceMixinDefaults();
    testRemapIniResource();
    testRemapIniFixResource(scratchDir);
    testRemapIniDownload(scratchDir);
    testRemapBlendResource();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
