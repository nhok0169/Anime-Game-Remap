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

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
