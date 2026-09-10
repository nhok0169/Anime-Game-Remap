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
// Standalone regression test for AGRemapCore::FileDownload::download()'s real
// libcurl-backed implementation (tools/files/FileDownload.h/.cpp) -- the
// piece that was a stub (always threw std::logic_error) until libcurl was
// wired into the build.
//
// Covers:
//   * A real curl_easy_perform() transfer via a "file://" URL (deterministic,
//     no network access needed -- exercises curl_easy_init, the
//     CURLOPT_WRITEFUNCTION callback actually receiving real bytes,
//     CURLE_OK handling, and the returned path actually existing with the
//     right content) -- proves the libcurl wiring itself (headers, link,
//     runtime .dll) works end to end, not just "compiles"
//   * An invalid/unreachable URL throws std::runtime_error (via
//     CURLOPT_FAILONERROR / a non-CURLE_OK result) rather than silently
//     "succeeding" with an empty/partial file, and does not leave a partial
//     file behind
//   * A REAL HTTPS download over the actual internet, against a real file
//     from the maintainer's own repo (FileDownloadData.py's
//     GithubDownloadFolder) -- confirms the full network+TLS stack works,
//     not just curl's local-file-copy code path. (This sandbox's own `curl`
//     CLI can't reach https:// at all -- SSL cert verify failure against an
//     intercepting proxy in this environment -- but this build links curl
//     configured for Schannel on Windows, which defers to the OS's own
//     certificate store instead of a bundled CA file, and that store already
//     trusts the intercepting proxy locally; confirmed empirically, not
//     assumed.)
//   * FileDownload::get()'s caching logic (already covered in
//     IniResources_test.cpp against a fake download) still behaves the same
//     way now that the real download() is live, confirming get() doesn't
//     depend on the stub in any way
//
// Needs a real curl build (this repo builds curl from source via
// core/CMakeLists.txt's add_subdirectory -- see AI Agent Help/Building/CLAUDE.md).
// This test links against an already-built libcurl_imp.lib/libcurl.dll rather
// than rebuilding curl from source for a throwaway verification (same
// "reuse an already-installed tree" posture as this doc's own z3 guidance).
// Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /I <core>/include /I <curl>/include ^
//      FileDownload_curl_test.cpp <core>/src/tools/files/FileDownload.cpp ^
//      /Fe:test.exe /link /LIBPATH:<cbuild>/curl/lib libcurl_imp.lib
//
// libcurl.dll must be copied alongside the built .exe (or on PATH) before
// running it -- same DLL-next-to-.exe requirement as libz3.dll in the Building
// doc's own z3 section.
// -----------------------------------------------------------------------------

#include "AGRemapCore/tools/files/FileDownload.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>

using AGRemapCore::FileDownload;

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
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

std::string toFileUrl(const std::filesystem::path& p) {
    // Minimal "file://" URL builder -- good enough for a plain absolute Windows path with no
    // characters needing percent-escaping (this test's own scratch paths qualify).
    std::string generic = p.generic_string();
    if (!generic.empty() && generic[0] != '/') {
        generic = "/" + generic;
    }
    return "file://" + generic;
}

void testRealFileUrlDownload(const std::string& scratchDir) {
    std::filesystem::path sourceFile = std::filesystem::path(scratchDir) / "curl_test_source.txt";
    std::filesystem::path destFolder = std::filesystem::path(scratchDir) / "curl_test_dest";
    std::filesystem::remove_all(destFolder);
    std::filesystem::create_directories(destFolder);

    const std::string content = "hello from libcurl file:// test\n";
    {
        std::ofstream out(sourceFile, std::ios::binary);
        out << content;
    }

    FileDownload dl(toFileUrl(std::filesystem::absolute(sourceFile)), "curl_test_source.txt");
    std::string downloadedPath = dl.download(destFolder.string());

    check(std::filesystem::exists(downloadedPath), "download(): real curl_easy_perform() produced a file on disk");
    check(readFile(downloadedPath) == content, "download(): downloaded content matches the source file exactly");
    check(std::filesystem::path(downloadedPath).parent_path() == destFolder, "download(): file landed in the requested folder");

    std::filesystem::remove_all(destFolder);
    std::filesystem::remove(sourceFile);
}

void testInvalidUrlThrows(const std::string& scratchDir) {
    std::filesystem::path destFolder = std::filesystem::path(scratchDir) / "curl_test_invalid_dest";
    std::filesystem::remove_all(destFolder);

    FileDownload dl("file:///this/path/definitely/does/not/exist/anywhere.txt", "anywhere.txt");

    bool threw = false;
    try {
        dl.download(destFolder.string());
    } catch (const std::runtime_error&) {
        threw = true;
    }

    check(threw, "download(): a request that can't succeed throws std::runtime_error");

    std::string expectedLeftover = (destFolder / "anywhere.txt").string();
    check(!std::filesystem::exists(expectedLeftover), "download(): no partial file left behind after a failed transfer");

    std::filesystem::remove_all(destFolder);
}

// A real file from the maintainer's own repo (see
// api/src/py/FixRaidenBoss2/data/FileDownloadData.py's GithubDownloadFolder) -- exercises the
// actual network/TLS stack (this build uses Schannel on Windows, per core/CMakeLists.txt's
// CURL_USE_SCHANNEL, so it validates against the OS's own certificate store rather than a bundled
// CA file) rather than just the local "file://" transfers above.
const std::string RealHttpsUrl =
    "https://github.com/nhok0169/Anime-Game-Remap/raw/nhok0169/Data/Mod%20Downloads/GI/Amber/4_0/AmberHeadDiffuse.dds";

void testRealHttpsDownload(const std::string& scratchDir) {
    std::filesystem::path destFolder = std::filesystem::path(scratchDir) / "curl_test_https_dest";
    std::filesystem::remove_all(destFolder);
    std::filesystem::create_directories(destFolder);

    FileDownload dl(RealHttpsUrl, "AmberHeadDiffuse.dds");
    std::string downloadedPath = dl.download(destFolder.string());

    check(std::filesystem::exists(downloadedPath), "download(): a real HTTPS transfer over the actual internet produced a file");

    std::string content = readFile(downloadedPath);
    check(content.size() > 100000, "download(): the real .dds file is a plausible size (not an empty/error page)");
    check(content.size() >= 4 && content.substr(0, 4) == "DDS ", "download(): the file starts with the real \"DDS \" magic bytes -- genuine content, not an HTML error page");

    std::filesystem::remove_all(destFolder);
}

void testGetStillWorksWithRealDownload(const std::string& scratchDir) {
    std::filesystem::path sourceFile = std::filesystem::path(scratchDir) / "curl_test_get_source.txt";
    std::filesystem::path destFolder = std::filesystem::path(scratchDir) / "curl_test_get_dest";
    std::filesystem::remove_all(destFolder);
    std::filesystem::create_directories(destFolder);

    {
        std::ofstream out(sourceFile, std::ios::binary);
        out << "get() + real download()\n";
    }

    FileDownload dl(toFileUrl(std::filesystem::absolute(sourceFile)), "curl_test_get_source.txt");
    auto [path1, downloaded1, wasFirst1] = dl.get(destFolder.string());
    (void)wasFirst1;
    check(downloaded1 && std::filesystem::exists(path1), "get(): first call drives the real download() successfully");

    auto [path2, downloaded2, wasFirst2] = dl.get(destFolder.string());
    (void)wasFirst2;
    check(!downloaded2 && path2 == path1, "get(): repeat call in the same folder short-circuits without re-downloading");

    std::filesystem::remove_all(destFolder);
    std::filesystem::remove(sourceFile);
}

// TWO SEPARATE FileDownload OBJECTS, ONE RUN. This is the shape the real fix has: a parser is
// built per IniFile, so a mod whose 36 .ini files all want the same file reaches each of them
// with a FRESH FileDownload whose own prevPath_ is empty. Before DownloadCache existed that
// meant 36 requests to github for one texture -- measured, and it is what this covers.
//
// The source file is CHANGED between the two calls, which is what makes the two outcomes
// tell themselves apart by content: a copy from the cache still carries the first bytes, while
// a real download would carry the second. Trusting get()'s own 'downloaded' flag alone would
// only prove the flag agrees with itself.
void testSharedCacheAcrossFileDownloads(const std::string& scratchDir) {
    std::filesystem::path sourceFile = std::filesystem::path(scratchDir) / "curl_test_shared_source.txt";
    std::filesystem::path firstFolder = std::filesystem::path(scratchDir) / "curl_test_shared_a";
    std::filesystem::path secondFolder = std::filesystem::path(scratchDir) / "curl_test_shared_b";
    std::filesystem::remove_all(firstFolder);
    std::filesystem::remove_all(secondFolder);
    std::filesystem::create_directories(firstFolder);
    std::filesystem::create_directories(secondFolder);

    {
        std::ofstream out(sourceFile, std::ios::binary);
        out << "first";
    }

    const std::string url = toFileUrl(std::filesystem::absolute(sourceFile));
    AGRemapCore::DownloadCache shared;

    FileDownload first(url, "curl_test_shared_source.txt");
    check(!first.cachedPath(&shared).has_value(), "cachedPath(): nothing fetched yet, so the first download really has to go and get it");

    auto [pathA, downloadedA, wasFirstA] = first.get(firstFolder.string(), std::nullopt, &shared);
    (void)wasFirstA;
    check(downloadedA, "get(): the first FileDownload downloads");
    check(readFile(pathA) == "first", "get(): ...and gets the source's content");

    // Whatever the URL serves has moved on. Only a real download can see this.
    {
        std::ofstream out(sourceFile, std::ios::binary);
        out << "second";
    }

    FileDownload second(url, "curl_test_shared_source.txt");
    check(second.cachedPath(&shared).has_value(), "cachedPath(): a DIFFERENT FileDownload for the same url now knows it can copy");

    auto [pathB, downloadedB, wasFirstB] = second.get(secondFolder.string(), std::nullopt, &shared);
    (void)wasFirstB;
    check(!downloadedB, "get(): the second FileDownload does NOT download");
    check(readFile(pathB) == "first", "get(): ...it copied the first one's file, rather than re-fetching the changed source");
    check(pathA != pathB && std::filesystem::exists(pathB), "get(): the copy landed in its OWN folder");

    // ...and without the shared cache it is the old behaviour, which is the regression this
    // guards: a fresh object has nothing of its own to copy from and goes back to the network.
    std::filesystem::path thirdFolder = std::filesystem::path(scratchDir) / "curl_test_shared_c";
    std::filesystem::remove_all(thirdFolder);
    std::filesystem::create_directories(thirdFolder);

    FileDownload third(url, "curl_test_shared_source.txt");
    check(!third.cachedPath(nullptr).has_value(), "cachedPath(): with no shared cache a fresh FileDownload has nothing to copy");

    auto [pathC, downloadedC, wasFirstC] = third.get(thirdFolder.string());
    (void)wasFirstC;
    check(downloadedC && readFile(pathC) == "second", "get(): ...so it re-downloads, and sees the CHANGED source");

    // A remembered file that has since been deleted must not wedge every later use. get() falls
    // back to a real download when the copy fails.
    std::filesystem::remove(pathA);
    std::filesystem::path fourthFolder = std::filesystem::path(scratchDir) / "curl_test_shared_d";
    std::filesystem::remove_all(fourthFolder);
    std::filesystem::create_directories(fourthFolder);

    FileDownload fourth(url, "curl_test_shared_source.txt");
    auto [pathD, downloadedD, wasFirstD] = fourth.get(fourthFolder.string(), std::nullopt, &shared);
    (void)wasFirstD;
    check(downloadedD && readFile(pathD) == "second", "get(): a remembered file that is gone falls back to downloading");

    std::filesystem::remove_all(firstFolder);
    std::filesystem::remove_all(secondFolder);
    std::filesystem::remove_all(thirdFolder);
    std::filesystem::remove_all(fourthFolder);
    std::filesystem::remove(sourceFile);
}

}  // namespace

int main(int argc, char** argv) {
    std::string scratchDir = ".";
    if (argc > 1) {
        scratchDir = argv[1];
    }

    testRealFileUrlDownload(scratchDir);
    testInvalidUrlThrows(scratchDir);
    testRealHttpsDownload(scratchDir);
    testGetStillWorksWithRealDownload(scratchDir);
    testSharedCacheAcrossFileDownloads(scratchDir);

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
