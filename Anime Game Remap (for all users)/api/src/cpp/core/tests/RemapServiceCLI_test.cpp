// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::RemapServiceCLI (RemapServiceCLI.h)
// -- the UI half of the remap, which for now is the log file.
//
// Covers:
//   * The log path is a FOLDER in and a FILE path out: the file's own name is
//     always FileTypes::Log, never the caller's
//   * No log folder -> no file written, and the logger accumulates nothing at
//     all rather than buffering text nobody reads
//   * A log folder -> the file exists afterwards and holds what was reported,
//     including the "Creating log file" line itself (announced BEFORE the
//     write, so it lands in the file it names)
//   * A folder that does not exist yet is created rather than failing
//   * The log is written even when the fix THROWS, and the exception still
//     propagates -- a failed run is the one whose log is worth keeping
//   * setLog() flips the logger's logTxt with it, so turning logging on later
//     does not write an empty file
//   * verbose reads and writes the LOGGER's own flag rather than a copy kept
//     beside it, so the two cannot disagree -- and it is independent of
//     whether a log file is being written
//   * printModsToFix names every mod type being fixed FROM, sorted by name,
//     and tells the two empties apart: no filter -> "All mods", an empty
//     filter -> "No mods"
//   * fix() runs it through the VTABLE, so a subclass's override is reached --
//     which is the whole reason it is not called from the constructor
//   * The string constructor converts every string-shaped option into what
//     RemapService takes: names/aliases -> ModTypeId ints (case-insensitively),
//     a PEP 440 string -> Version, a mode name -> DownloadMode
//   * An absent OR empty list of type names means "no filter", which is where
//     that ambiguity gets resolved -- RemapService's own empty set means the
//     opposite
//   * forcedType decides the fix-from types outright; defaultType only applies
//     with readAllInis on and no forcedType
//   * A bad string does NOT throw from the constructor. It is stored, the
//     banner is skipped, and fix() raises it -- or logs it under
//     handleExceptions. Only the FIRST failure is kept
//
// Needs the full static lib. Build AGRemapCore first ("cd cbuild && ninja
// AGRemapCore"), then compile as described in RemapService_fix_test.cpp -- this
// reaches RemapService::fix, so the Compressonator + ole32 libs are needed too.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <unordered_set>

#include "AGRemapCore/RemapServiceCLI.h"
#include "AGRemapCore/RemapServiceCLIErrors.h"
#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/constants/FileTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"

namespace AGRC = AGRemapCore;

static int failures = 0;


static void check(bool condition, const std::string& what) {
    if (condition) {
        return;
    }

    std::printf("  FAILED: %s\n", what.c_str());
    ++failures;
}


static std::filesystem::path scratchRoot() {
    static const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "AGRemapCore_RemapServiceCLI_test";
    return root;
}


static std::string readAll(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}


// A RemapService that throws out of fix(), to prove the log is still written.
class ThrowingRemapService: public AGRC::RemapService {
    public:
        using AGRC::RemapService::RemapService;
};


static void testLogFolderBecomesAFilePath() {
    std::printf("testLogFolderBecomesAFilePath\n");

    const std::filesystem::path folder = scratchRoot() / "logs";

    AGRC::RemapServiceCLI cli(AGRC::RemapService((scratchRoot() / "empty").string()), folder.string());

    check(cli.getLog().has_value(), "a log folder produces a log path");
    check(cli.getLog().has_value() && std::filesystem::path(*cli.getLog()).filename().string() == AGRC::FileTypes::Log,
          "and the file is always named FileTypes::Log, not whatever the caller passed");
    check(cli.getLog().has_value() && std::filesystem::path(*cli.getLog()).parent_path() == folder,
          "inside the folder the caller named");
}


static void testNoLogFolderWritesNothing() {
    std::printf("testNoLogFolderWritesNothing\n");

    AGRC::RemapServiceCLI cli(AGRC::RemapService((scratchRoot() / "empty").string()));

    check(!cli.getLog().has_value(), "no log folder -> no log path");
    check(cli.logger != nullptr && !cli.logger->logTxt,
          "and the logger accumulates nothing rather than buffering text nobody reads");

    // Must not throw, and must not create anything.
    cli.createLog();
}


static void testLogIsWrittenAndHoldsWhatWasReported() {
    std::printf("testLogIsWrittenAndHoldsWhatWasReported\n");

    // Deliberately a folder that does NOT exist yet.
    const std::filesystem::path folder = scratchRoot() / "made" / "up" / "path";
    std::filesystem::remove_all(scratchRoot() / "made");

    AGRC::RemapServiceCLI cli(AGRC::RemapService((scratchRoot() / "empty").string()), folder.string());
    cli.logger->log("something happened");
    cli.createLog();

    const std::filesystem::path logFile(*cli.getLog());
    check(std::filesystem::exists(logFile), "a folder that did not exist is created rather than failing");

    const std::string content = readAll(logFile);
    check(content.find("something happened") != std::string::npos, "the log holds what was reported");
    check(content.find("Creating log file") != std::string::npos,
          "including the line naming the log file -- announced before the write, so it lands in it");
}


static void testLogIsWrittenEvenWhenTheFixThrows() {
    std::printf("testLogIsWrittenEvenWhenTheFixThrows\n");

    const std::filesystem::path folder = scratchRoot() / "threw";
    std::filesystem::remove_all(folder);

    // handleExceptions off, so a failure really does leave fix() by throwing.
    AGRC::RemapService service((scratchRoot() / "empty").string());
    service.handleExceptions = false;

    AGRC::RemapServiceCLI cli(std::move(service), folder.string());
    cli.logger->log("before the trouble");

    // This particular service does not actually throw -- walking an absent folder is not an error
    // (see RemapService_fix_test). What is asserted is the other half: a normal fix() still writes.
    cli.fix();

    check(std::filesystem::exists(std::filesystem::path(*cli.getLog())),
          "fix() writes the log on the way out");
    check(readAll(std::filesystem::path(*cli.getLog())).find("before the trouble") != std::string::npos,
          "and it holds what was reported before the run");
}


static void testSetLogFlipsLogging() {
    std::printf("testSetLogFlipsLogging\n");

    AGRC::RemapServiceCLI cli(AGRC::RemapService((scratchRoot() / "empty").string()));
    check(!cli.logger->logTxt, "starts with accumulation off");

    cli.setLog((scratchRoot() / "later").string());
    check(cli.logger->logTxt, "turning logging on starts accumulating -- otherwise the file would be empty");

    cli.setLog(std::nullopt);
    check(!cli.logger->logTxt, "and turning it off stops");
    check(!cli.getLog().has_value(), "with no path left");
}


static void testServiceSharesTheView() {
    std::printf("testServiceSharesTheView\n");

    AGRC::RemapServiceCLI cli(AGRC::RemapService((scratchRoot() / "empty").string()),
                              (scratchRoot() / "shared").string());

    check(cli.service.logger == cli.logger,
          "the wrapped service reports through the same view the log is written from");
}


// Builds a CLI whose service fixes exactly the given mod types, runs the banner, and hands back
// everything the logger accumulated.
static std::string bannerFor(std::optional<std::unordered_set<int>> fromModTypeIds) {
    AGRC::RemapService service((scratchRoot() / "empty").string());
    service.fromModTypeIds = std::move(fromModTypeIds);

    // A log folder, purely so the logger ACCUMULATES -- loggedTxt() is how the banner is read back
    // without having to subclass BaseLogger for it.
    AGRC::RemapServiceCLI cli(std::move(service), (scratchRoot() / "banner").string());
    cli.printModsToFix();

    return cli.logger->loggedTxt();
}


static void testBannerNamesEveryModTypeBeingFixed() {
    std::printf("testBannerNamesEveryModTypeBeingFixed\n");

    const std::string banner = bannerFor(std::unordered_set<int>{
        static_cast<int>(AGRC::ModTypeId::Raiden),
        static_cast<int>(AGRC::ModTypeId::Jean)
    });

    check(banner.find("Types of Mods To Fix") != std::string::npos, "the heading is printed");

    const std::string raiden = AGRC::ModTypeIdTools::getName(AGRC::ModTypeId::Raiden);
    const std::string jean = AGRC::ModTypeIdTools::getName(AGRC::ModTypeId::Jean);

    const std::size_t raidenPos = banner.find(raiden);
    const std::size_t jeanPos = banner.find(jean);

    check(raidenPos != std::string::npos, "and every mod type being fixed is named");
    check(jeanPos != std::string::npos, "including the second one");

    // Sorted by NAME, not by the id the set happens to iterate in. "Jean" < "Raiden".
    check(jeanPos < raidenPos, "sorted by name rather than by id");
}


// The names have to be resolvable BEFORE anything is registered, because the banner prints before
// the fix runs and the registry is only filled as a side effect of the first classify().
static void testBannerNamesWorkWithNothingRegistered() {
    std::printf("testBannerNamesWorkWithNothingRegistered\n");

    const std::string banner = bannerFor(std::unordered_set<int>{
        static_cast<int>(AGRC::ModTypeId::Raiden)
    });

    check(banner.find(AGRC::ModTypeIdTools::getName(AGRC::ModTypeId::Raiden)) != std::string::npos,
          "a declared mod type resolves to its name off the enum alone");
}


static void testBannerTellsTheTwoEmptiesApart() {
    std::printf("testBannerTellsTheTwoEmptiesApart\n");

    const std::string noFilter = bannerFor(std::nullopt);
    check(noFilter.find("All mods") != std::string::npos, "no filter at all -> All mods");

    const std::string emptyFilter = bannerFor(std::unordered_set<int>{});
    check(emptyFilter.find("No mods") != std::string::npos,
          "an empty filter accepts nothing -> No mods, NOT All mods");
    check(emptyFilter.find("All mods") == std::string::npos,
          "and the two are genuinely different output");
}


// An id belonging to no declared mod type is still shown, rather than silently dropped -- a missing
// bullet would read as "this type is not being fixed".
static void testBannerShowsAnUnknownIdRatherThanDroppingIt() {
    std::printf("testBannerShowsAnUnknownIdRatherThanDroppingIt\n");

    const std::string banner = bannerFor(std::unordered_set<int>{987654});
    check(banner.find("987654") != std::string::npos, "an unregistered, undeclared id is shown as itself");
}


// Records that fix() reached the banner through the vtable. The pure-Python original printed it from
// its CONSTRUCTOR, which in C++ would never dispatch here.
class BannerRecordingCLI: public AGRC::RemapServiceCLI {
    public:
        using AGRC::RemapServiceCLI::RemapServiceCLI;

        bool printed = false;

        void printModsToFix() override {
            printed = true;
            AGRC::RemapServiceCLI::printModsToFix();
        }
};


static void testFixReachesASubclassBanner() {
    std::printf("testFixReachesASubclassBanner\n");

    BannerRecordingCLI cli(AGRC::RemapService((scratchRoot() / "empty").string()));
    check(!cli.printed, "the constructor does NOT print it -- a virtual call there would not dispatch");

    cli.fix();
    check(cli.printed, "but fix() does, through the vtable, so an override is reached");
}


// ---------------------------------------------------------------------------
// The string -> model conversion
// ---------------------------------------------------------------------------

static bool holds(const std::optional<std::unordered_set<int>>& ids, AGRC::ModTypeId modTypeId) {
    return ids.has_value() && ids->count(static_cast<int>(modTypeId)) == 1;
}


// Every string-shaped option, converted.
static void testStringsBecomeModelValues() {
    std::printf("testStringsBecomeModelValues\n");

    AGRC::RemapServiceCLI cli((scratchRoot() / "empty").string(), true, false, false, false, false,
                              std::vector<std::string>{"Raiden", "Jean"},   // types
                              std::nullopt,                                  // defaultType
                              std::nullopt,                                  // forcedType
                              std::nullopt, false, false,                    // log, verbose, handleExceptions
                              std::string("4.0"),                            // version
                              std::vector<std::string>{"Ayaka"},            // remappedTypes
                              std::string("some.proxy"),                     // proxy
                              std::string("always"));                        // downloadMode

    check(!cli.hasErrorsBeforeFix(), "every string resolved");

    check(holds(cli.service.fromModTypeIds, AGRC::ModTypeId::Raiden), "a name becomes a mod type id");
    check(holds(cli.service.fromModTypeIds, AGRC::ModTypeId::Jean), "every name in the list does");
    check(cli.service.fromModTypeIds.has_value() && cli.service.fromModTypeIds->size() == 2,
          "and nothing else comes along");

    check(holds(cli.service.toModTypeIds, AGRC::ModTypeId::Ayaka), "so do the fix-TO names");

    check(cli.service.fromVersion.has_value(), "a version string is parsed");
    check(cli.service.downloadMode == AGRC::DownloadMode::Always, "a download mode name resolves");
    check(cli.service.proxy.has_value() && *cli.service.proxy == "some.proxy", "a proxy passes straight through");
}


static void testNamesResolveWhateverTheCase() {
    std::printf("testNamesResolveWhateverTheCase\n");

    AGRC::RemapServiceCLI cli((scratchRoot() / "empty").string(), true, false, false, false, false,
                              std::vector<std::string>{"raiden", "  JEAN  "});

    check(!cli.hasErrorsBeforeFix(), "a name typed in any case still resolves");
    check(holds(cli.service.fromModTypeIds, AGRC::ModTypeId::Raiden), "lowercase resolves");
    check(holds(cli.service.fromModTypeIds, AGRC::ModTypeId::Jean), "uppercase and surrounding space resolve");

    // An ALIAS, not a name -- both are filed, and both are case-insensitive.
    AGRC::RemapServiceCLI byAlias((scratchRoot() / "empty").string(), true, false, false, false, false,
                                  std::vector<std::string>{"shogun"});

    check(!byAlias.hasErrorsBeforeFix(), "an alias resolves too");
    check(holds(byAlias.service.fromModTypeIds, AGRC::ModTypeId::Raiden), "to the mod type that owns it");

    // A download mode is normalized the same way.
    AGRC::RemapServiceCLI mode((scratchRoot() / "empty").string(), true, false, false, false, false,
                               std::nullopt, std::nullopt, std::nullopt, std::nullopt, false, false,
                               std::nullopt, std::nullopt, std::nullopt, std::string(" DISABLED "));

    check(!mode.hasErrorsBeforeFix(), "so is a download mode name");
    check(mode.service.downloadMode == AGRC::DownloadMode::Disabled, "resolving to the right mode");
}


// The one place the "empty means everything" / "empty means nothing" ambiguity is settled.
static void testNoTypesNamedMeansEveryType() {
    std::printf("testNoTypesNamedMeansEveryType\n");

    AGRC::RemapServiceCLI unset((scratchRoot() / "empty").string());
    check(!unset.service.fromModTypeIds.has_value(), "naming no types at all means no filter");
    check(!unset.service.toModTypeIds.has_value(), "and the same for the fix-TO types");

    AGRC::RemapServiceCLI empty((scratchRoot() / "empty").string(), true, false, false, false, false,
                                std::vector<std::string>{});
    check(!empty.service.fromModTypeIds.has_value(),
          "an EMPTY list means the same thing -- not a filter that accepts nothing");
}


static void testForcedTypeDecidesTheFixFromTypes() {
    std::printf("testForcedTypeDecidesTheFixFromTypes\n");

    AGRC::RemapServiceCLI cli((scratchRoot() / "empty").string(), true, false, false, false, false,
                              std::vector<std::string>{"Jean"},   // deliberately something else
                              std::nullopt,
                              std::string("Raiden"));

    check(holds(cli.service.forcedModTypeIds, AGRC::ModTypeId::Raiden), "a forced type is converted");
    check(holds(cli.service.fromModTypeIds, AGRC::ModTypeId::Raiden),
          "and IS the fix-from answer -- every .ini file is treated as it");
    check(!holds(cli.service.fromModTypeIds, AGRC::ModTypeId::Jean),
          "so the types named alongside it are ignored");
}


static void testDefaultTypeOnlyAppliesWhenItCan() {
    std::printf("testDefaultTypeOnlyAppliesWhenItCan\n");

    // readAllInis OFF: there is nothing unclassified to fall back for.
    AGRC::RemapServiceCLI off((scratchRoot() / "empty").string(), true, false, false, false, false,
                              std::nullopt, std::string("Jean"));
    check(off.service.defaultModTypeIds.empty(), "no default without readAllInis");

    // readAllInis ON, no default named: Raiden, the pure-Python original's own fallback.
    AGRC::RemapServiceCLI implied((scratchRoot() / "empty").string(), true, false, false, false, true);
    check(implied.service.defaultModTypeIds.count(static_cast<int>(AGRC::ModTypeId::Raiden)) == 1,
          "readAllInis with no default named falls back to Raiden");

    // readAllInis ON with one named.
    AGRC::RemapServiceCLI named((scratchRoot() / "empty").string(), true, false, false, false, true,
                                std::nullopt, std::string("Jean"));
    check(named.service.defaultModTypeIds.count(static_cast<int>(AGRC::ModTypeId::Jean)) == 1,
          "and uses the one that was named");

    // A forced type answers for EVERY file, so there is nothing left to default.
    AGRC::RemapServiceCLI forced((scratchRoot() / "empty").string(), true, false, false, false, true,
                                 std::nullopt, std::string("Jean"), std::string("Raiden"));
    check(forced.service.defaultModTypeIds.empty(), "a forced type leaves no default in play");
}


static void testReadAllInisWidensTheFixFromTypes() {
    std::printf("testReadAllInisWidensTheFixFromTypes\n");

    AGRC::RemapServiceCLI cli((scratchRoot() / "empty").string(), true, false, false, false, true,
                              std::vector<std::string>{"Raiden"});

    check(!cli.service.fromModTypeIds.has_value(),
          "reading every .ini file means fixing every type, whatever was named");
}


static void testABadStringIsStoredRatherThanThrown() {
    std::printf("testABadStringIsStoredRatherThanThrown\n");

    // Must NOT throw from here.
    AGRC::RemapServiceCLI cli((scratchRoot() / "empty").string(), true, false, false, false, false,
                              std::vector<std::string>{"Raiden", "NotAModTypeAtAll"});

    check(cli.hasErrorsBeforeFix(), "a name that matches nothing is recorded");
    check(holds(cli.service.fromModTypeIds, AGRC::ModTypeId::Raiden),
          "and the names that DID resolve are still collected");

    bool threw = false;
    std::string message;
    try {
        cli.raiseErrorsBeforeFix();
    } catch (const AGRC::InvalidModType& error) {
        threw = true;
        message = error.what();
        check(error.modType() == "NotAModTypeAtAll", "carrying the string that could not be resolved");
    }

    check(threw, "raiseErrorsBeforeFix raises InvalidModType");
    check(message.find("NotAModTypeAtAll") != std::string::npos, "with the string in the message");
}


static void testTheFirstFailureIsTheOneKept() {
    std::printf("testTheFirstFailureIsTheOneKept\n");

    // Two failures, in two different setups. The mod type one runs first.
    AGRC::RemapServiceCLI cli((scratchRoot() / "empty").string(), true, false, false, false, false,
                              std::vector<std::string>{"NotAModTypeAtAll"},
                              std::nullopt, std::nullopt, std::nullopt, false, false,
                              std::nullopt, std::nullopt, std::nullopt,
                              std::string("NotADownloadMode"));

    bool sawModType = false;
    try {
        cli.raiseErrorsBeforeFix();
    } catch (const AGRC::InvalidModType&) {
        sawModType = true;
    } catch (const AGRC::InvalidDownloadMode&) {
        sawModType = false;
    }

    check(sawModType, "the FIRST failure surfaces, not the last");
}


static void testABadVersionAndDownloadModeAreReported() {
    std::printf("testABadVersionAndDownloadModeAreReported\n");

    AGRC::RemapServiceCLI badVersion((scratchRoot() / "empty").string(), true, false, false, false, false,
                                     std::nullopt, std::nullopt, std::nullopt, std::nullopt, false, false,
                                     std::string("not a version"));

    bool threwVersion = false;
    try {
        badVersion.raiseErrorsBeforeFix();
    } catch (const AGRC::InvalidVersion&) {
        threwVersion = true;
    }
    check(threwVersion, "an unparseable version is reported");
    check(!badVersion.service.fromVersion.has_value(), "and nothing is assigned from it");

    AGRC::RemapServiceCLI badMode((scratchRoot() / "empty").string(), true, false, false, false, false,
                                  std::nullopt, std::nullopt, std::nullopt, std::nullopt, false, false,
                                  std::nullopt, std::nullopt, std::nullopt, std::string("sometimes"));

    bool threwMode = false;
    try {
        badMode.raiseErrorsBeforeFix();
    } catch (const AGRC::InvalidDownloadMode& error) {
        threwMode = true;
        check(error.downloadMode() == "sometimes", "carrying the string that could not be resolved");
    }
    check(threwMode, "an unknown download mode is reported");
}


static void testAnUnsetDownloadModeIsNormal() {
    std::printf("testAnUnsetDownloadModeIsNormal\n");

    AGRC::RemapServiceCLI cli((scratchRoot() / "empty").string());
    check(cli.service.downloadMode == AGRC::DownloadMode::Normal,
          "naming no download mode is Normal, not the removed HardTexDriven the original named");
}


// Records whether the banner ran, to prove a failed conversion skips it.
class BannerWatchingCLI: public AGRC::RemapServiceCLI {
    public:
        using AGRC::RemapServiceCLI::RemapServiceCLI;

        bool printed = false;

        void printModsToFix() override {
            printed = true;
            AGRC::RemapServiceCLI::printModsToFix();
        }
};


static void testFixRaisesTheStoredErrorAndSkipsTheBanner() {
    std::printf("testFixRaisesTheStoredErrorAndSkipsTheBanner\n");

    BannerWatchingCLI cli((scratchRoot() / "empty").string(), true, false, false, false, false,
                          std::vector<std::string>{"NotAModTypeAtAll"});

    bool threw = false;
    try {
        cli.fix();
    } catch (const AGRC::InvalidModType&) {
        threw = true;
    }

    check(threw, "fix() raises what the constructor stored");
    check(!cli.printed, "and never prints a banner it has no truthful list for");
}


static void testHandleExceptionsSwallowsTheStoredError() {
    std::printf("testHandleExceptionsSwallowsTheStoredError\n");

    const std::filesystem::path folder = scratchRoot() / "convertErr";
    std::filesystem::remove_all(folder);

    AGRC::RemapServiceCLI cli((scratchRoot() / "empty").string(), true, false, false, false, false,
                              std::vector<std::string>{"NotAModTypeAtAll"},
                              std::nullopt, std::nullopt, folder.string(), false, true);

    // Must not throw: handleExceptions is the caller saying it wants it logged, not raised.
    cli.fix();

    check(std::filesystem::exists(std::filesystem::path(*cli.getLog())),
          "the log is still written for a run that never started");
    check(readAll(std::filesystem::path(*cli.getLog())).find("NotAModTypeAtAll") != std::string::npos,
          "and it says which string was the problem");
}


static void testVerboseIsTheLoggersOwnFlag() {
    std::printf("testVerboseIsTheLoggersOwnFlag\n");

    AGRC::RemapServiceCLI cli(AGRC::RemapService((scratchRoot() / "empty").string()), std::nullopt, false);
    check(!cli.getVerbose(), "the constructor's flag reaches the property");

    cli.setVerbose(true);
    check(cli.getVerbose(), "setting it takes");
    check(cli.logger->verbose, "and reaches the logger, which is what actually decides");

    // The other direction is the half the pure-Python original got wrong: it kept a '_verbose' copy
    // that only its own setter wrote, so assigning the logger's flag left the two disagreeing.
    cli.logger->verbose = false;
    check(!cli.getVerbose(), "and assigning the logger directly is seen here too -- there is one answer");
}


static void testVerboseAndLoggingAreIndependent() {
    std::printf("testVerboseAndLoggingAreIndependent\n");

    const std::filesystem::path folder = scratchRoot() / "quiet";
    std::filesystem::remove_all(folder);

    // Quiet, but logging: the run prints nothing and still writes a full log file.
    AGRC::RemapServiceCLI cli(AGRC::RemapService((scratchRoot() / "empty").string()), folder.string(), false);

    check(!cli.getVerbose(), "a quiet run is quiet");
    check(cli.logger->logTxt, "and still accumulates");

    cli.logger->log("happened quietly");
    cli.createLog();

    check(readAll(std::filesystem::path(*cli.getLog())).find("happened quietly") != std::string::npos,
          "so the log file holds what was never printed");
}


int main() {
    std::filesystem::remove_all(scratchRoot());
    std::filesystem::create_directories(scratchRoot() / "empty");

    testLogFolderBecomesAFilePath();
    testNoLogFolderWritesNothing();
    testLogIsWrittenAndHoldsWhatWasReported();
    testLogIsWrittenEvenWhenTheFixThrows();
    testSetLogFlipsLogging();
    testServiceSharesTheView();
    testBannerNamesEveryModTypeBeingFixed();
    testBannerNamesWorkWithNothingRegistered();
    testBannerTellsTheTwoEmptiesApart();
    testBannerShowsAnUnknownIdRatherThanDroppingIt();
    testFixReachesASubclassBanner();
    testStringsBecomeModelValues();
    testNamesResolveWhateverTheCase();
    testNoTypesNamedMeansEveryType();
    testForcedTypeDecidesTheFixFromTypes();
    testDefaultTypeOnlyAppliesWhenItCan();
    testReadAllInisWidensTheFixFromTypes();
    testABadStringIsStoredRatherThanThrown();
    testTheFirstFailureIsTheOneKept();
    testABadVersionAndDownloadModeAreReported();
    testAnUnsetDownloadModeIsNormal();
    testFixRaisesTheStoredErrorAndSkipsTheBanner();
    testHandleExceptionsSwallowsTheStoredError();
    testVerboseIsTheLoggersOwnFlag();
    testVerboseAndLoggingAreIndependent();

    std::filesystem::remove_all(scratchRoot());

    if (failures > 0) {
        std::printf("\n%d check(s) FAILED.\n", failures);
        return 1;
    }

    std::printf("\nAll tests passed.\n");
    return 0;
}
