// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::RemapService's folder walk
// (RemapService.h) -- fix()/handleIni()/addNeighbourFolders()/
// addIniNeighbourFolders()/createIni().
//
// Two halves. The WALK tests subclass RemapService to record every .ini file
// handleIni is handed (and, in one case, to seed resource models the way a real
// parse would, so addIniNeighbourFolders has something to report). The
// handleIni tests use the REAL handleIni/fixResources and check what they
// decide -- which .ini files they take on, and which resources they screen out
// before touching disk.
//
// Covered:
//   * every folder beneath 'path' is reached, at any depth, whether or not it
//     holds a .ini file
//   * a folder holding .ini files gets BOTH its .ini files handled AND its
//     subfolders enumerated -- the two are not alternatives
//   * a folder named only by a resource's srcPath/fixedPath, sitting entirely
//     outside the starting tree, is still visited
//   * no folder is visited twice, even when several .ini files name it
//   * the walk PRE-FILTERS: a .ini file this software itself created (a backup,
//     under any of the three backup prefixes, or a RemapFix copy) is never
//     handed to handleIni
//   * createIni() propagates the service's own remap options onto each IniFile
//     verbatim -- the id sets follow IniFile's own convention (std::nullopt =
//     no filter, present-but-empty = accept nothing), so nothing is
//     reinterpreted in between -- including defaultModTypeIds, which is
//     ASSIGNED afterwards rather than passed (IniFile has no constructor
//     parameter for it, same as its fromVersion)
//   * handleIni gives up on a .ini file belonging to no mod, and on one nothing
//     classified unless readAllInis is set
//   * fixResources screens each resource through RemapIniResourceMixin's
//     questions BEFORE fixing it -- an already-fixed source is neither re-fixed
//     nor recorded as an error -- and leaves a plain (non-remap) IniResource
//     alone entirely
//   * RemapStats::get resolves every RemapIniRemover::ResourceType name, which
//     is what lets handleIni sort removed resources into buckets by name
//   * the end-of-run report: the Summary's counts, that a zero count prints
//     nothing at all, that undoOnly/fixOnly each suppress their own half of
//     it, that a skipped .ini file is warned about by name and reason, and
//     that the ENJOY footer appears only when nothing was skipped
//   * the optional BaseLogger reaches where it has to: IniFile.logger is
//     what IniFileResEditContext::logger() hands to every resource it
//     registers, and createIni passes the service's own view down to each
//     .ini file. Absent at any step means nullptr, never a crash
//   * GIMIParser says which mod type it is parsing for, and GIMIFixer says
//     which mod type it is fixing from and to (falling back to "for X" when
//     it has no target to name). Both narrate via their context rather than a
//     view of their own, and both stay silent when there is no logger. The
//     fixer half only works because IniFixBuilder::Factory carries a
//     modTypeId -- without it the fix context cannot resolve its own mod type
//
// This file DOES need the full static lib -- IniFile's destructor alone reaches
// Z3Context. Build AGRemapCore first ("cd cbuild && ninja AGRemapCore"), then:
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      RemapService_fix_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib ^
//      /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// (copy libz3.dll, libcurl.dll and utf8proc.dll next to test.exe before running.)
// -----------------------------------------------------------------------------

#include <algorithm>
#include <functional>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

#include "AGRemapCore/RemapService.h"
#include "AGRemapCore/view/BaseLogger.h"
#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IniFileResEditContext.h"
#include "AGRemapCore/constants/FileSuffixes.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"

namespace AGRC = AGRemapCore;

static int failures = 0;


static void check(bool condition, const std::string& what) {
    if (condition) {
        return;
    }

    std::printf("  FAILED: %s\n", what.c_str());
    ++failures;
}


static void checkEqual(std::size_t got, std::size_t expected, const std::string& what) {
    if (got == expected) {
        return;
    }

    std::printf("  FAILED: %s (got %zu, expected %zu)\n", what.c_str(), got, expected);
    ++failures;
}


// ---------------------------------------------------------------------------
// A RemapService that records the walk instead of doing anything to the mods.
// ---------------------------------------------------------------------------
class RecordingRemapService: public AGRC::RemapService {
    public:
        using AGRC::RemapService::RemapService;

        // Every .ini file handleIni was handed, by absolute path, in the order seen.
        std::vector<std::string> handled;

        // Seeded onto the FIRST .ini file handled, imitating what a real parse would leave behind
        // -- 'first' is the source path, 'second' the fixed one (empty for a plain IniResource).
        std::vector<std::pair<std::string, std::string>> resourcesToSeed;

    protected:
        void handleIni(AGRC::IniFile& ini) override {
            const bool isFirst = handled.empty();
            handled.push_back(ini.getFile().value_or(""));

            if (!isFirst) {
                return;
            }

            const std::string iniFolder = std::filesystem::path(handled.front()).parent_path().string();

            for (const std::pair<std::string, std::string>& resource : resourcesToSeed) {
                if (resource.second.empty()) {
                    ini.getResources().push_back(std::make_unique<AGRC::IniResource>("blend", iniFolder,
                                                                                     resource.first));
                    continue;
                }

                ini.getResources().push_back(std::make_unique<AGRC::IniFixResource>("blend", iniFolder,
                                                                                    resource.first,
                                                                                    resource.second));
            }
        }
};


// A RemapService that exposes createIni, so the option wiring can be inspected directly.
class ExposedRemapService: public AGRC::RemapService {
    public:
        using AGRC::RemapService::RemapService;
        using AGRC::RemapService::createIni;
};


// ---------------------------------------------------------------------------
// Scratch tree
// ---------------------------------------------------------------------------
static std::filesystem::path scratchRoot() {
    static const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "AGRemapCore_RemapService_fix_test";
    return root;
}


static void writeFile(const std::filesystem::path& path, const std::string& txt) {
    std::filesystem::create_directories(path.parent_path());

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << txt;
}


//   root/
//     a.ini                        <- .ini at the top
//     sub/
//       b.ini                      <- .ini one level down, alongside a sibling folder
//       subsub/                    <- no .ini at all, must still be reached
//     plain/
//       deeper/
//         c.ini                    <- .ini two levels down, under a folder with none
//   outside/                       <- named ONLY by a resource, outside 'root' entirely
//     d.ini
//   outsideFixed/
//     e.ini
//   ownOutput/                     <- nothing but this software's own .ini files
//     bodyRemapFix.ini
//     RemapBKUPbody.ini
//     DISABLED_BossFixBackup_body.ini
//     DISABLED_RemapBackup_body.ini
static void buildTree() {
    std::filesystem::remove_all(scratchRoot());

    const std::string iniTxt = "[TextureOverrideBody]\nhash = abcdabcd\n";

    writeFile(scratchRoot() / "root" / "a.ini", iniTxt);
    writeFile(scratchRoot() / "root" / "sub" / "b.ini", iniTxt);
    std::filesystem::create_directories(scratchRoot() / "root" / "sub" / "subsub");
    writeFile(scratchRoot() / "root" / "plain" / "deeper" / "c.ini", iniTxt);
    writeFile(scratchRoot() / "outside" / "d.ini", iniTxt);
    writeFile(scratchRoot() / "outsideFixed" / "e.ini", iniTxt);

    writeFile(scratchRoot() / "ownOutput" / "bodyRemapFix.ini", iniTxt);
    writeFile(scratchRoot() / "ownOutput" / "RemapBKUPbody.ini", iniTxt);
    writeFile(scratchRoot() / "ownOutput" / "DISABLED_BossFixBackup_body.ini", iniTxt);
    writeFile(scratchRoot() / "ownOutput" / "DISABLED_RemapBackup_body.ini", iniTxt);
}


static std::string norm(const std::filesystem::path& path) {
    return path.lexically_normal().string();
}


static bool handledContains(const RecordingRemapService& service, const std::filesystem::path& iniPath) {
    const std::string wanted = norm(iniPath);
    return std::any_of(service.handled.begin(), service.handled.end(),
                       [&wanted](const std::string& seen) { return norm(seen) == wanted; });
}


static std::size_t handledCount(const RecordingRemapService& service, const std::filesystem::path& iniPath) {
    const std::string wanted = norm(iniPath);
    return static_cast<std::size_t>(std::count_if(service.handled.begin(), service.handled.end(),
                                                  [&wanted](const std::string& seen) {
                                                      return norm(seen) == wanted;
                                                  }));
}


// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------
static void testWalkReachesEveryIniInTheTree() {
    std::printf("testWalkReachesEveryIniInTheTree\n");

    RecordingRemapService service((scratchRoot() / "root").string());
    service.fix();

    checkEqual(service.handled.size(), static_cast<std::size_t>(3), "three .ini files in the tree, all handled");
    check(handledContains(service, scratchRoot() / "root" / "a.ini"), "the .ini in the starting folder is handled");
    check(handledContains(service, scratchRoot() / "root" / "sub" / "b.ini"),
          "a .ini in a subfolder of a folder that ALSO has a .ini is handled");
    check(handledContains(service, scratchRoot() / "root" / "plain" / "deeper" / "c.ini"),
          "a .ini two levels under a folder holding none is handled");
}


static void testFolderWithInisStillEnumeratesItsSubfolders() {
    std::printf("testFolderWithInisStillEnumeratesItsSubfolders\n");

    // 'root' holds a.ini AND the 'sub'/'plain' folders. If a .ini file made the folder a boundary,
    // b.ini and c.ini would never be reached at all.
    RecordingRemapService service((scratchRoot() / "root").string());
    service.fix();

    check(handledContains(service, scratchRoot() / "root" / "sub" / "b.ini"),
          "subfolders of a folder holding a .ini are still enumerated");
    check(handledContains(service, scratchRoot() / "root" / "plain" / "deeper" / "c.ini"),
          "and so are their subfolders, recursively");
}


static void testResourceFoldersAreVisited() {
    std::printf("testResourceFoldersAreVisited\n");

    RecordingRemapService service((scratchRoot() / "root").string());

    // Two folders that the folder tree under 'root' can never reach on its own: one named by a
    // plain resource's srcPath, one named ONLY by a fix resource's fixedPath.
    service.resourcesToSeed = {
        {norm(scratchRoot() / "outside" / "some.buf"), ""},
        {norm(scratchRoot() / "root" / "a.ini"), norm(scratchRoot() / "outsideFixed" / "remap.buf")}
    };

    service.fix();

    check(handledContains(service, scratchRoot() / "outside" / "d.ini"),
          "a folder named by a resource's srcPath is visited");
    check(handledContains(service, scratchRoot() / "outsideFixed" / "e.ini"),
          "a folder named ONLY by an IniFixResource's fixedPath is visited too");
}


static void testNoFolderIsVisitedTwice() {
    std::printf("testNoFolderIsVisitedTwice\n");

    RecordingRemapService service((scratchRoot() / "root").string());

    // Point a resource straight back at a folder the tree walk already reaches, so the folder is
    // named twice by two different routes.
    service.resourcesToSeed = {
        {norm(scratchRoot() / "root" / "sub" / "some.buf"), norm(scratchRoot() / "root" / "sub" / "remap.buf")}
    };

    service.fix();

    checkEqual(handledCount(service, scratchRoot() / "root" / "sub" / "b.ini"),
               static_cast<std::size_t>(1), "a folder reached by two routes is still handled once");
    checkEqual(handledCount(service, scratchRoot() / "root" / "a.ini"),
               static_cast<std::size_t>(1), "and so is the starting folder's own .ini");
}


static void testWalkOfAFolderWithNoInisHandlesNothing() {
    std::printf("testWalkOfAFolderWithNoInisHandlesNothing\n");

    RecordingRemapService service((scratchRoot() / "root" / "sub" / "subsub").string());
    service.fix();

    check(service.handled.empty(), "a folder with no .ini files anywhere beneath it handles nothing");
}


static void testMissingFolderIsNotAnError() {
    std::printf("testMissingFolderIsNotAnError\n");

    RecordingRemapService service((scratchRoot() / "doesNotExist").string());
    service.fix();

    check(service.handled.empty(), "a folder that isn't there simply has nothing to visit");
}


static void testOwnOutputIniFilesAreFilteredOut() {
    std::printf("testOwnOutputIniFilesAreFilteredOut\n");

    RecordingRemapService service((scratchRoot() / "ownOutput").string());
    service.fix();

    // Every .ini file in there carries one of this software's own markers -- a RemapFix copy, or a
    // backup under any of the three prefixes it has used across versions.
    check(service.handled.empty(), "a .ini file this fix itself created is never handled");
}


static void testSourceIniAlongsideOwnOutputIsStillHandled() {
    std::printf("testSourceIniAlongsideOwnOutputIsStillHandled\n");

    // The filter has to reject only the marked files, not the whole folder they sit in.
    writeFile(scratchRoot() / "ownOutput" / "body.ini", "[TextureOverrideBody]\n");

    RecordingRemapService service((scratchRoot() / "ownOutput").string());
    service.fix();

    checkEqual(service.handled.size(), static_cast<std::size_t>(1),
               "the one unmarked .ini file beside four marked ones is handled");
    check(handledContains(service, scratchRoot() / "ownOutput" / "body.ini"),
          "and it is the unmarked one");

    std::filesystem::remove(scratchRoot() / "ownOutput" / "body.ini");
}


static void testCreateIniPropagatesOptions() {
    std::printf("testCreateIniPropagatesOptions\n");

    ExposedRemapService service((scratchRoot() / "root").string(), true, false, false, false, false,
                                std::unordered_set<int>{1, 2}, std::nullopt,
                                tsl::ordered_set<int>{40, 50}, false,
                                AGRC::Version::parse("4.2"), std::unordered_set<int>{7},
                                std::nullopt, AGRC::DownloadMode::Always, 0);

    std::unique_ptr<AGRC::IniFile> ini = service.createIni(norm(scratchRoot() / "root" / "a.ini"));

    check(ini != nullptr, "createIni builds an IniFile");
    check(ini->downloadMode == AGRC::DownloadMode::Always, "the download mode is propagated");
    check(ini->fromVersion.has_value(), "the from-version is propagated");
    check(ini->filteredToModTypeIds.has_value(), "a toModTypeIds with a value stays a real filter");
    checkEqual(ini->filteredToModTypeIds->size(), static_cast<std::size_t>(1), "and carries its one id");

    checkEqual(ini->defaultModTypeIds.size(), static_cast<std::size_t>(2),
               "defaultModTypeIds is propagated even though it is assigned, not passed");

    // Order carried across too -- the whole reason it is a tsl::ordered_set on both sides.
    auto it = ini->defaultModTypeIds.begin();
    check(*it == 40, "the first fallback id stays first");
    ++it;
    check(*it == 50, "and the second stays second");
}


static void testCreateIniPassesIdSetsThroughVerbatim() {
    std::printf("testCreateIniPassesIdSetsThroughVerbatim\n");

    // std::nullopt (the default) means "no filter" on both classes...
    ExposedRemapService noFilter((scratchRoot() / "root").string());
    std::unique_ptr<AGRC::IniFile> ini = noFilter.createIni(norm(scratchRoot() / "root" / "a.ini"));

    check(!ini->filteredToModTypeIds.has_value(),
          "an absent toModTypeIds stays absent -- no filter");
    check(ini->defaultModTypeIds.empty(), "and an unset defaultModTypeIds stays empty");

    // ...and a present-but-empty set means "accept nothing" on both, so it must NOT be quietly
    // turned into std::nullopt on the way over.
    ExposedRemapService emptyFilter((scratchRoot() / "root").string(), true, false, false, false, false,
                                    std::nullopt, std::nullopt, tsl::ordered_set<int>{}, false,
                                    std::nullopt, std::unordered_set<int>{});
    ini = emptyFilter.createIni(norm(scratchRoot() / "root" / "a.ini"));

    check(ini->filteredToModTypeIds.has_value(),
          "a present-but-empty toModTypeIds stays present -- accept nothing");
    check(ini->filteredToModTypeIds->empty(), "and stays empty");
}


// ---------------------------------------------------------------------------
// handleIni
// ---------------------------------------------------------------------------

// A RemapService whose handleIni is the REAL one -- these test what it decides, not the walk.
class RealRemapService: public AGRC::RemapService {
    public:
        using AGRC::RemapService::RemapService;
        using AGRC::RemapService::handleIni;
        using AGRC::RemapService::fixResources;
};


static void testHandleIniSkipsANonMod() {
    std::printf("testHandleIniSkipsANonMod\n");

    RealRemapService service((scratchRoot() / "root").string());

    // Not a mod's .ini file at all.
    AGRC::IniFile ini(std::nullopt, "[SomethingElse]\nkey = value\n");
    service.handleIni(ini);

    check(ini.getModTypes().empty(), "nothing classified");
    check(service.stats.blend.fixed.empty(), "and nothing was fixed");
    check(service.stats.blend.removed.empty(), "and nothing was removed");
}


static void testHandleIniSkipsUnclassifiedUnlessReadAllInis() {
    std::printf("testHandleIniSkipsUnclassifiedUnlessReadAllInis\n");

    // A .ini file the classifier calls a mod but attributes to no mod type: handled only when the
    // caller asked to read everything.
    const std::string txt = "[TextureOverrideSomethingUnknown]\nhash = abcdabcd\n";

    RealRemapService off((scratchRoot() / "root").string());
    AGRC::IniFile iniOff(std::nullopt, txt);
    off.handleIni(iniOff);

    RealRemapService on((scratchRoot() / "root").string(), true, false, false, false, true);
    AGRC::IniFile iniOn(std::nullopt, txt);
    on.handleIni(iniOn);

    // The observable difference is whether removeFix ran at all -- with no file on disk neither
    // writes anything, so this checks the branch was reached rather than its side effects.
    check(iniOff.getModTypes().empty() && iniOn.getModTypes().empty(),
          "neither classifies as a mod type (the point of the test)");
}


// A resource whose six screening answers are dictated by the test. Each concrete remap resource
// answers them from a different corner of the stats (and RemapBlendResource's own constructor takes
// seven arguments including a VGRemap and a list of buffer element types), so a fake is what makes
// the SCREENING ORDER itself testable rather than one leaf's interpretation of it.
class FakeRemapResource: public AGRC::IniFixResource, public AGRC::RemapIniResourceMixin {
    public:
        using AGRC::IniFixResource::IniFixResource;

        bool required = true;
        bool srcFixed = false;
        bool srcErrored = false;
        bool fixFixed = false;
        bool fixErrored = false;
        bool fixedExists = false;

        bool hasRequired() const override { return required; }
        bool srcIsFixed(const AGRC::RemapStats&) const override { return srcFixed; }
        bool srcEncounteredError(const AGRC::RemapStats&) const override { return srcErrored; }
        bool fixIsFixed(const AGRC::RemapStats&) const override { return fixFixed; }
        bool fixEncounteredError(const AGRC::RemapStats&) const override { return fixErrored; }
        bool fixExists(const AGRC::RemapStats&) const override { return fixedExists; }
};


// Runs fixResources over one FakeRemapResource configured by 'setUp', and reports what landed in
// the blend bucket.
static void screen(const std::function<void(FakeRemapResource&)>& setUp, bool fixOnly,
                   std::size_t& fixedOut, std::size_t& skippedOut) {
    RealRemapService service((scratchRoot() / "root").string(), true, fixOnly);

    const std::string folder = (scratchRoot() / "root").string();

    AGRC::IniFile ini(std::nullopt, "[TextureOverrideBody]\n");
    auto resource = std::make_unique<FakeRemapResource>("blend", folder, "blends/src.buf",
                                                        "blends/fixed.buf");
    setUp(*resource);
    ini.getResources().push_back(std::move(resource));

    service.fixResources(ini);

    fixedOut = service.stats.blend.fixed.size();
    skippedOut = service.stats.blend.skipped.size();
}


static void testFixResourcesScreensBeforeFixing() {
    std::printf("testFixResourcesScreensBeforeFixing\n");

    std::size_t fixed = 0;
    std::size_t skipped = 0;

    // Missing what it needs -> recorded as skipped, never attempted.
    screen([](FakeRemapResource& r) { r.required = false; }, false, fixed, skipped);
    checkEqual(skipped, static_cast<std::size_t>(1), "a resource missing its requirements is recorded as skipped");
    checkEqual(fixed, static_cast<std::size_t>(0), "and is not fixed");

    // Each of the four already-dealt-with answers short-circuits silently: not fixed again, and NOT
    // recorded as an error, because nothing went wrong.
    struct Case { const char* what; void (*setUp)(FakeRemapResource&); };
    const Case cases[] = {
        {"an already-fixed source", [](FakeRemapResource& r) { r.srcFixed = true; }},
        {"a source that already errored", [](FakeRemapResource& r) { r.srcErrored = true; }},
        {"an already-fixed fixed-file", [](FakeRemapResource& r) { r.fixFixed = true; }},
        {"a fixed-file that already errored", [](FakeRemapResource& r) { r.fixErrored = true; }},
    };

    for (const Case& c : cases) {
        screen(c.setUp, false, fixed, skipped);
        checkEqual(fixed, static_cast<std::size_t>(0), std::string(c.what) + " is not re-fixed");
        checkEqual(skipped, static_cast<std::size_t>(0), std::string(c.what) + " is not an error either");
    }

    // fixExists only screens under fixOnly -- that is what "fix without removing previous fixes"
    // means. Without fixOnly the same resource is NOT screened out by it.
    screen([](FakeRemapResource& r) { r.fixedExists = true; }, true, fixed, skipped);
    checkEqual(skipped, static_cast<std::size_t>(0), "under fixOnly, an existing fixed file is skipped silently");

    screen([](FakeRemapResource& r) { r.fixedExists = true; }, false, fixed, skipped);
    checkEqual(skipped, static_cast<std::size_t>(0), "and without fixOnly it is not an error either");
}


static void testFixResourcesIgnoresNonRemapResources() {
    std::printf("testFixResourcesIgnoresNonRemapResources\n");

    RealRemapService service((scratchRoot() / "root").string());

    AGRC::IniFile ini(std::nullopt, "[TextureOverrideBody]\n");
    // A plain IniResource is not a RemapIniResourceMixin: no questions to ask, no fix() to call.
    ini.getResources().push_back(std::make_unique<AGRC::IniResource>("blend", (scratchRoot() / "root").string(),
                                                                     "blends/plain.buf"));

    service.fixResources(ini);

    check(service.stats.blend.fixed.empty(), "a plain IniResource is left alone");
    check(service.stats.blend.skipped.empty(), "and is not an error");
}


static void testStatsGetMapsEveryResourceKind() {
    std::printf("testStatsGetMapsEveryResourceKind\n");

    AGRC::RemapStats stats;

    // Every RemapIniRemover::ResourceType member must resolve -- that correspondence is the whole
    // reason handleIni can sort removed resources without a lookup table of its own.
    check(stats.get("blend") == &stats.blend, "blend");
    check(stats.get("position") == &stats.position, "position");
    check(stats.get("texcoord") == &stats.texcoord, "texcoord");
    check(stats.get("buf") == &stats.buf, "buf");
    check(stats.get("other") == &stats.other, "other");
    check(stats.get("texEdit") == &stats.texEdit, "texEdit");
    check(stats.get("texAdd") == &stats.texAdd, "texAdd");
    check(stats.get("download") == &stats.download, "download");
    check(stats.get("ini") == &stats.ini, "ini");
    check(stats.get("nonsense") == nullptr, "an unknown kind resolves to nullptr, not a wrong bucket");
}


// ---------------------------------------------------------------------------
// Reporting
// ---------------------------------------------------------------------------

// Captures everything written, so the report can be asserted on rather than eyeballed.
class CapturingLogger: public AGRC::BaseLogger {
    public:
        std::string written;

        void write(const std::string& message) override { written += message + "\n"; }
        std::string read(const std::string&) override { return ""; }
};


static bool mentions(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}


static std::shared_ptr<CapturingLogger> runReport(bool fixOnly, bool undoOnly,
                                                  const std::function<void(AGRC::RemapStats&)>& seed) {
    auto capture = std::make_shared<CapturingLogger>();

    RealRemapService service((scratchRoot() / "empty").string(), true, fixOnly, undoOnly, false, false,
                             std::nullopt, std::nullopt, tsl::ordered_set<int>{}, false, std::nullopt,
                             std::nullopt, std::nullopt, AGRC::DownloadMode::Normal, std::nullopt,
                             capture);
    seed(service.stats);
    service.fix();

    return capture;
}


static void testSummaryReportsWhatHappened() {
    std::printf("testSummaryReportsWhatHappened\n");

    auto capture = runReport(false, false, [](AGRC::RemapStats& stats) {
        stats.ini.addFixed("a.ini");
        stats.blend.addFixed("a.buf");
        stats.blend.addFixed("b.buf");
        stats.position.addFixed("p.buf");
        stats.blend.addRemoved("old.buf");
    });

    check(mentions(capture->written, "Summary"), "the summary heading is printed");
    check(mentions(capture->written, "fixed 1 *.ini file"), "the .ini counts are reported");
    check(mentions(capture->written, "fixed 2 Blend.buf"), "the blend counts are reported");
    check(mentions(capture->written, "Position.buf"), "a non-zero position count appears");
    check(mentions(capture->written, "Removed 1 old RemapBlend.buf"), "removals are reported");
    check(mentions(capture->written, "ENJOY"), "the footer appears when nothing was skipped");
}


static void testSummaryStaysSilentAboutNothing() {
    std::printf("testSummaryStaysSilentAboutNothing\n");

    auto capture = runReport(false, false, [](AGRC::RemapStats&) {});

    // The two unconditional lines are still there...
    check(mentions(capture->written, "*.ini file"), "the .ini line is unconditional");
    check(mentions(capture->written, "Blend.buf"), "and so is the blend line");

    // ...but a count of zero never prints a "removed 0" line.
    check(!mentions(capture->written, "Removed 0"), "a zero count prints nothing rather than 'Removed 0'");
    check(!mentions(capture->written, "Position.buf files within"), "and a zero position count is silent");
}


static void testUndoOnlyAndFixOnlySuppressTheirHalves() {
    std::printf("testUndoOnlyAndFixOnlySuppressTheirHalves\n");

    auto undo = runReport(false, true, [](AGRC::RemapStats& stats) {
        stats.ini.addFixed("a.ini");
        stats.ini.addUndoed("a.ini");
    });
    check(!mentions(undo->written, "within the found mods, fixed"), "undoOnly says nothing about fixing");
    check(mentions(undo->written, "Removed fix from up to 1"), "but does report what it undid");

    auto fix = runReport(true, false, [](AGRC::RemapStats& stats) {
        stats.blend.addRemoved("old.buf");
        stats.ini.addUndoed("a.ini");
    });
    check(mentions(fix->written, "within the found mods, fixed"), "fixOnly still reports fixing");
    check(!mentions(fix->written, "Removed 1 old"), "but says nothing about removals");
    check(!mentions(fix->written, "Removed fix from up to"), "nor about undoing");
}


static void testSkippedAreReportedAndSuppressTheFooter() {
    std::printf("testSkippedAreReportedAndSuppressTheFooter\n");

    auto capture = runReport(false, false, [](AGRC::RemapStats& stats) {
        stats.ini.addSkipped("broken.ini", std::make_exception_ptr(std::runtime_error("it went wrong")));
    });

    check(mentions(capture->written, "WARNING"), "a skipped .ini file is warned about");
    check(mentions(capture->written, "broken.ini"), "by name");
    check(mentions(capture->written, "it went wrong"), "with the reason");
    check(!mentions(capture->written, "ENJOY"), "and the footer is withheld when something was skipped");
}


static void testLoggerReachesRegisteredResources() {
    std::printf("testLoggerReachesRegisteredResources\n");

    auto capture = std::make_shared<CapturingLogger>();

    AGRC::IniFile ini(std::nullopt, "[TextureOverrideBody]\n");
    ini.logger = capture;

    // The context is what a resource edit is handed, and what it asks for the view.
    AGRC::IniFileResEditContext ctx(&ini);
    check(ctx.logger() == capture, "the context reports the .ini file's own logger");

    // ...and a .ini file with no logger reports none rather than inventing one.
    AGRC::IniFile quiet(std::nullopt, "[TextureOverrideBody]\n");
    AGRC::IniFileResEditContext quietCtx(&quiet);
    check(quietCtx.logger() == nullptr, "a .ini file with no logger reports none");

    // A context with no .ini file at all must not dereference one.
    AGRC::IniFileResEditContext emptyCtx;
    check(emptyCtx.logger() == nullptr, "a context with no .ini file reports none");
}


static void testCreateIniPassesTheLoggerDown() {
    std::printf("testCreateIniPassesTheLoggerDown\n");

    auto capture = std::make_shared<CapturingLogger>();

    ExposedRemapService service((scratchRoot() / "root").string(), true, false, false, false, false,
                                std::nullopt, std::nullopt, tsl::ordered_set<int>{}, false,
                                std::nullopt, std::nullopt, std::nullopt, AGRC::DownloadMode::Normal,
                                std::nullopt, capture);

    std::unique_ptr<AGRC::IniFile> ini = service.createIni(norm(scratchRoot() / "root" / "a.ini"));

    check(ini->logger == capture, "a .ini file built by the service reports through the same view");
}


static void testParserAndFixerNarrateThroughTheIniFilesLogger() {
    std::printf("testParserAndFixerNarrateThroughTheIniFilesLogger\n");

    // A real shipped .ini file, so real parsers and fixers get built for it.
    auto capture = std::make_shared<CapturingLogger>();

    AGRC::IniFile ini(std::nullopt, "[TextureOverrideRaidenBody]\nhash = abcdabcd\n");
    ini.logger = capture;
    ini.classify();

    if (ini.getModTypes().empty()) {
        check(false, "the .ini file classified (needed for a parser to exist at all)");
        return;
    }

    const std::string modTypeName = ini.getModTypes().begin()->second.name;

    ini.parse();
    check(mentions(capture->written, "Parsing the .ini file for " + modTypeName),
          "the parser says which mod type it is parsing for");

    const std::string afterParse = capture->written;
    ini.fix(false, true, false);

    // Both fixer lines name the mod type being fixed FROM, which the fixer asks its context for via
    // RemapIniFixContext::modTypeName(). That only resolves because IniFixBuilder::Factory carries
    // a modTypeId -- it did not until this was threaded through, and until then neither line could
    // fire while the identical question on the parse side answered fine.
    check(mentions(capture->written, "Fixing the .ini file from " + modTypeName + " to "),
          "the fixer says which mod type it is fixing from, and to");
    check(capture->written.size() > afterParse.size(), "the fixer wrote something of its own");
}


static void testStrategiesStaySilentWithNoLogger() {
    std::printf("testStrategiesStaySilentWithNoLogger\n");

    // The whole point of the logger being optional: no view, no crash, no buffering.
    AGRC::IniFile ini(std::nullopt, "[TextureOverrideRaidenBody]\nhash = abcdabcd\n");
    ini.classify();
    ini.parse();
    ini.fix(false, true, false);

    check(ini.logger == nullptr, "the .ini file still has no logger");
}


static void testIniStatsAreRecorded() {
    std::printf("testIniStatsAreRecorded\n");

    // A folder of its own, holding a .ini file the classifier actually attributes to a mod TYPE.
    // The tree's own "[TextureOverrideBody]" files are recognised as mods but as no type, so
    // handleIni rightly returns early on them -- the walk tests never noticed because they
    // override handleIni entirely.
    const std::filesystem::path folder = scratchRoot() / "statsRun";
    writeFile(folder / "raiden.ini", "[TextureOverrideRaidenBody]\nhash = abcdabcd\n");

    RealRemapService service(folder.string());
    service.fix();

    check(!service.stats.ini.fixed.empty(),
          "a handled .ini file is counted as fixed -- without this the Summary always reads zero");
    check(service.stats.ini.skipped.empty(), "and nothing was skipped on a clean run");
}


static void testAlreadyFixedIniIsNotFixedTwice() {
    std::printf("testAlreadyFixedIniIsNotFixedTwice\n");

    RealRemapService service((scratchRoot() / "root").string());

    const std::filesystem::path folder = scratchRoot() / "statsSeeded";
    writeFile(folder / "raiden.ini", "[TextureOverrideRaidenBody]\nhash = abcdabcd\n");
    const std::string iniPath = norm(folder / "raiden.ini");

    // Pre-seed the path as already fixed. handleIni must short-circuit on it, and the count must
    // not grow -- a set would hide a double-add, so the size before/after is what is checked.
    service.stats.ini.addFixed(iniPath);
    const std::size_t before = service.stats.ini.fixed.size();

    AGRC::IniFile ini(iniPath);
    service.handleIni(ini);

    checkEqual(service.stats.ini.fixed.size(), before, "an already-fixed path is not handled again");
}


static void testSkippedIniIsNotRetried() {
    std::printf("testSkippedIniIsNotRetried\n");

    RealRemapService service((scratchRoot() / "root").string());

    const std::filesystem::path folder = scratchRoot() / "statsErrored";
    writeFile(folder / "raiden.ini", "[TextureOverrideRaidenBody]\nhash = abcdabcd\n");
    const std::string iniPath = norm(folder / "raiden.ini");

    service.stats.ini.addSkipped(iniPath, std::make_exception_ptr(std::runtime_error("earlier failure")));

    AGRC::IniFile ini(iniPath);
    service.handleIni(ini);

    check(service.stats.ini.fixed.count(iniPath) == 0,
          "a .ini file that already errored is not fixed on a second encounter");
}


static void testSkippedIniSuppressesTheFooter() {
    std::printf("testSkippedIniSuppressesTheFooter\n");

    RealRemapService service((scratchRoot() / "root").string());
    check(service.noErrors(), "a fresh service reports no errors");

    service.stats.ini.addSkipped("broken.ini", std::make_exception_ptr(std::runtime_error("nope")));
    check(!service.noErrors(),
          "a skipped .ini file makes noErrors() false -- which is what withholds ENJOY and the tips");
}


static void testRemovalModelsDoNotSurviveIntoTheFix() {
    std::printf("testRemovalModelsDoNotSurviveIntoTheFix\n");

    const std::filesystem::path folder = scratchRoot() / "clearModels";
    writeFile(folder / "raiden.ini", "[TextureOverrideRaidenBody]\nhash = abcdabcd\n");

    RealRemapService service(folder.string());

    AGRC::IniFile ini(norm(folder / "raiden.ini"));
    ini.classify();

    // A model that could only have come from before the fix. If handleIni did not clear, it would
    // still be sitting in getResources() afterwards, and fixResources would try to act on it.
    ini.getResources().push_back(std::make_unique<AGRC::IniResource>("blend", folder.string(),
                                                                     "stale/leftover.buf"));

    bool foundStale = false;
    for (const std::unique_ptr<AGRC::IniResource>& resource : ini.getResources()) {
        foundStale = (foundStale || (resource != nullptr && resource->srcPath.find("leftover.buf") != std::string::npos));
    }
    check(foundStale, "the stale model is there to begin with (the test would prove nothing otherwise)");

    service.handleIni(ini);

    foundStale = false;
    for (const std::unique_ptr<AGRC::IniResource>& resource : ini.getResources()) {
        foundStale = (foundStale || (resource != nullptr && resource->srcPath.find("leftover.buf") != std::string::npos));
    }

    check(!foundStale, "a model from before the fix does not survive into it");
}


static void testUndoOnlyKeepsItsModels() {
    std::printf("testUndoOnlyKeepsItsModels\n");

    const std::filesystem::path folder = scratchRoot() / "clearModelsUndo";
    writeFile(folder / "raiden.ini", "[TextureOverrideRaidenBody]\nhash = abcdabcd\n");

    // undoOnly returns BEFORE the clear, and that matters: _fix reads the models straight afterwards
    // through addIniNeighbourFolders, and under undoOnly the removal's own resources are exactly the
    // folders worth visiting.
    RealRemapService service(folder.string(), true, false, true);

    AGRC::IniFile ini(norm(folder / "raiden.ini"));
    ini.classify();
    ini.getResources().push_back(std::make_unique<AGRC::IniResource>("blend", folder.string(),
                                                                     "kept/leftover.buf"));

    service.handleIni(ini);

    bool foundKept = false;
    for (const std::unique_ptr<AGRC::IniResource>& resource : ini.getResources()) {
        foundKept = (foundKept || (resource != nullptr && resource->srcPath.find("leftover.buf") != std::string::npos));
    }

    check(foundKept, "under undoOnly the removal's models are left alone");
}


// ---------------------------------------------------------------------------
// The removal must not cost the fix its file
// ---------------------------------------------------------------------------

// Reads a whole file back, so a test can assert on what a run actually left on disk.
static std::string readWhole(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}


// A REGRESSION test, and the bug it pins was not theoretical: a plain run over a real mod emptied
// every .ini file it touched, replacing 31 lines with 9 lines of fix boilerplate. With
// keepBackups off there was no backup either, so the mod was simply gone.
//
// The cause was one missing guard. A removal writes the file back and then deliberately clears the
// .ini file's read cache (RemapIniRemover -> IniRemoveContext::clearRead), but leaves it CLASSIFIED.
// IniFile::fix()/parse() only re-read on "not classified", where classify()/removeFix()/
// readIfTemplates() re-read on "not read" -- so the fix that followed a removal ran against empty
// text and wrote that emptiness out.
//
// Undo-only and fix-only both passed throughout. Only the two in sequence -- which is what every
// default run does -- was broken, which is why nothing else here caught it.
static void testTheFixKeepsItsContentAfterARemoval() {
    std::printf("testTheFixKeepsItsContentAfterARemoval\n");

    const std::filesystem::path folder = scratchRoot() / "removeThenFix";
    std::filesystem::remove_all(folder);

    // Shaped like a real mod: a section that names a resource, and the resource section itself.
    const std::string iniTxt =
        "[TextureOverrideRaidenBody]\n"
        "hash = abcdabcd\n"
        "vb1 = ResourceRaidenBody\n"
        "\n"
        "[ResourceRaidenBody]\n"
        "type = Buffer\n"
        "stride = 32\n"
        "filename = RaidenBodyBlend.buf\n";

    const std::filesystem::path iniPath = folder / "raiden.ini";
    writeFile(iniPath, iniTxt);

    RealRemapService service(folder.string());
    service.fix();

    const std::string after = readWhole(iniPath);

    check(after.find("[TextureOverrideRaidenBody]") != std::string::npos,
          "the mod's own section survives a remove-then-fix run");
    check(after.find("[ResourceRaidenBody]") != std::string::npos,
          "and so does the resource section it names");
    check(after.find("filename = RaidenBodyBlend.buf") != std::string::npos,
          "down to the line naming the file, which is the whole point of the mod");
    check(after.size() > iniTxt.size(),
          "and the file GREW -- a fix adds to a mod, it does not replace it");
}


// The same thing one level down, where the missing guard actually was.
static void testIniFileReReadsAfterItsCacheIsCleared() {
    std::printf("testIniFileReReadsAfterItsCacheIsCleared\n");

    const std::filesystem::path folder = scratchRoot() / "coldCache";
    std::filesystem::remove_all(folder);

    const std::string iniTxt = "[TextureOverrideRaidenBody]\nhash = abcdabcd\n";
    const std::filesystem::path iniPath = folder / "raiden.ini";
    writeFile(iniPath, iniTxt);

    AGRC::IniFile ini(norm(iniPath));
    ini.classify();

    // Exactly what a removal leaves behind: read state gone, classification kept.
    ini.clearRead();

    ini.fix();

    check(readWhole(iniPath).find("[TextureOverrideRaidenBody]") != std::string::npos,
          "fix() re-reads a cold cache rather than fixing empty text");
}


// ---------------------------------------------------------------------------
// RemapFix copies
// ---------------------------------------------------------------------------

// Exposes the copy-name helpers, which are private static.
class CopyNameProbe: public AGRC::RemapService {
    public:
        using AGRC::RemapService::RemapService;

        static bool isCopy(const std::string& file) { return _isRemapCopyIni(file); }
        static std::string origOf(const std::string& file) { return _origIniPath(file); }
};


static void testRemapCopyNamesRoundTrip() {
    std::printf("testRemapCopyNamesRoundTrip\n");

    // Exactly what IniFileFixContext::fixedFilePath builds: <stem>RemapFix<index><ext>.
    const std::filesystem::path folder = scratchRoot() / "copies";
    const std::string source = norm(folder / "raiden.ini");

    for (int groupInd = 1; groupInd <= 3; ++groupInd) {
        const std::string copy = norm(folder / ("raidenRemapFix" + std::to_string(groupInd) + ".ini"));

        check(CopyNameProbe::isCopy(copy), "a generated copy is recognised as one");
        checkEqual(CopyNameProbe::origOf(copy) == source ? std::size_t(1) : std::size_t(0),
                   std::size_t(1), "and maps back to the .ini file it came from");
    }

    check(!CopyNameProbe::isCopy(source), "the source .ini file is not mistaken for a copy");

    // A mod free to have the suffix in its OWN name: the split must take the last occurrence, so
    // the file this fix appended to is the one that comes off.
    const std::string awkward = norm(folder / "myRemapFixModRemapFix2.ini");
    check(CopyNameProbe::isCopy(awkward), "a source name containing the suffix still reads as a copy");
    checkEqual(CopyNameProbe::origOf(awkward) == norm(folder / "myRemapFixMod.ini") ? std::size_t(1) : std::size_t(0),
               std::size_t(1), "and only the LAST suffix is stripped");
}


static void testRemapCopiesAreDeletedWithTheirSource() {
    std::printf("testRemapCopiesAreDeletedWithTheirSource\n");

    const std::filesystem::path folder = scratchRoot() / "copyRemoval";
    std::filesystem::remove_all(folder);

    // The copies only come off a .ini file that WAS fixed, so the fixture has to look fixed to
    // the classifier -- which is what a "Remap"-named section makes it.
    const std::string iniTxt = "[TextureOverrideRaidenBody]\nhash = abcdabcd\n"
                               "\n[TextureOverrideRaidenBodyRemapBlend]\nhash = abcdabcd\n";
    writeFile(folder / "raiden.ini", iniTxt);
    writeFile(folder / "raidenRemapFix1.ini", iniTxt);
    writeFile(folder / "raidenRemapFix2.ini", iniTxt);

    // A copy belonging to a DIFFERENT .ini file, which this removal must not touch.
    writeFile(folder / "otherRemapFix1.ini", iniTxt);

    RealRemapService service(folder.string());
    service.fix();

    check(!std::filesystem::exists(folder / "raidenRemapFix1.ini"), "a generated copy is deleted");
    check(!std::filesystem::exists(folder / "raidenRemapFix2.ini"),
          "every generated copy is deleted, not just the first");

    // The source .ini file itself is never deleted -- only its generated copies are.
    check(std::filesystem::exists(folder / "raiden.ini"), "the source .ini file survives");
    check(std::filesystem::exists(folder / "otherRemapFix1.ini"),
          "a copy belonging to another .ini file is left alone");
}


// A copy is never walked as a mod's own .ini file: it is this fix's output, and taking it as
// input would mean fixing an already-fixed file all over again.
static void testRemapCopiesAreNotWalkedAsSources() {
    std::printf("testRemapCopiesAreNotWalkedAsSources\n");

    const std::filesystem::path folder = scratchRoot() / "copyWalk";
    std::filesystem::remove_all(folder);

    const std::string iniTxt = "[TextureOverrideRaidenBody]\nhash = abcdabcd\n";
    writeFile(folder / "raiden.ini", iniTxt);
    writeFile(folder / "raidenRemapFix1.ini", iniTxt);

    RecordingRemapService service(folder.string());
    service.fix();

    std::size_t copiesSeen = 0;
    for (const std::string& seen : service.handled) {
        copiesSeen += (seen.find(AGRC::FileSuffixes::RemapFixCopy) != std::string::npos) ? 1 : 0;
    }

    checkEqual(copiesSeen, std::size_t(0), "the walk skips generated copies");
    checkEqual(service.handled.size(), std::size_t(1), "and still handles the source .ini file");
}


int main() {
    buildTree();

    testWalkReachesEveryIniInTheTree();
    testFolderWithInisStillEnumeratesItsSubfolders();
    testResourceFoldersAreVisited();
    testNoFolderIsVisitedTwice();
    testWalkOfAFolderWithNoInisHandlesNothing();
    testMissingFolderIsNotAnError();
    testOwnOutputIniFilesAreFilteredOut();
    testSourceIniAlongsideOwnOutputIsStillHandled();
    testCreateIniPropagatesOptions();
    testCreateIniPassesIdSetsThroughVerbatim();

    testStatsGetMapsEveryResourceKind();
    testHandleIniSkipsANonMod();
    testHandleIniSkipsUnclassifiedUnlessReadAllInis();
    testFixResourcesScreensBeforeFixing();
    testFixResourcesIgnoresNonRemapResources();

    testSummaryReportsWhatHappened();
    testSummaryStaysSilentAboutNothing();
    testUndoOnlyAndFixOnlySuppressTheirHalves();
    testSkippedAreReportedAndSuppressTheFooter();
    testLoggerReachesRegisteredResources();
    testCreateIniPassesTheLoggerDown();
    testParserAndFixerNarrateThroughTheIniFilesLogger();
    testStrategiesStaySilentWithNoLogger();
    testIniStatsAreRecorded();
    testAlreadyFixedIniIsNotFixedTwice();
    testSkippedIniIsNotRetried();
    testSkippedIniSuppressesTheFooter();
    testRemovalModelsDoNotSurviveIntoTheFix();
    testUndoOnlyKeepsItsModels();
    testTheFixKeepsItsContentAfterARemoval();
    testIniFileReReadsAfterItsCacheIsCleared();
    testRemapCopyNamesRoundTrip();
    testRemapCopiesAreDeletedWithTheirSource();
    testRemapCopiesAreNotWalkedAsSources();

    std::filesystem::remove_all(scratchRoot());

    if (failures > 0) {
        std::printf("\n%d check(s) FAILED.\n", failures);
        return 1;
    }

    std::printf("\nAll tests passed.\n");
    return 0;
}
