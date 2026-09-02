// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniFileResEditContext -- the plain-C++
// IniResEditContext, and the AGRemapCore::IniFile::getResources storage behind it.
//
// WHY THIS FILE EXISTS: the pybind11 layer has its own IniResEditContext
// (PyIniResEditContext, covered from Python through the ResRegCollect/ResGroupCollect
// tests), and nothing in the Python suite ever instantiates this one. It is reachable
// only from a caller with a real C++ AGRemapCore::IniFile, so it is covered here or
// nowhere -- the same reason GIMIFixer_test.cpp exists.
//
// What it pins is the part that is easy to get quietly wrong: which of the three
// destinations a stored model lands in, and that takeCollectedResources' raw pointers
// stay alive afterwards.
//
// NOT wired into any build target (core/tests/*.cpp never is). Compile and run it by hand:
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      IniFileResEditContext_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// (build AGRemapCore.lib first: `cd cbuild && ninja AGRemapCore`, and copy
//  cext/z3/bin/libz3.dll, cbuild/curl/lib/libcurl.dll and cbuild/utf8proc/utf8proc.dll
//  next to test.exe before running it)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IniFileResEditContext.h"

#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace AGRemapCore;

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (condition) {
        std::printf("[PASS] %s\n", description);
    } else {
        std::printf("[FAIL] %s\n", description);
        ++failures;
    }
}


std::unique_ptr<IniResource> makeResource(const std::string& name) {
    return std::make_unique<IniResource>(name, "C:/Mods/TestMod", name + ".buf");
}


// ---------------------------------------------------------------------------------------
// Where a stored model goes
// ---------------------------------------------------------------------------------------

void testStoreDestinations() {
    std::printf("\n--- IniFileResEditContext::storeResource ---\n");

    IniFile ini("C:/Mods/TestMod/CuteLittleEi.ini");
    IniFileResEditContext ctx(&ini);

    check(ini.getResources().empty(), "an .ini file starts with no resources");

    ctx.storeResource("someKey", makeResource("blend"));
    check(ini.getResources().size() == 1, "with no collect map and no capture pass, a model goes to the .ini file");
    check(ini.getResources()[0] != nullptr && ini.getResources()[0]->type == "blend",
          "and it is the model that was stored");

    // The file key is not part of that path at all -- ini.resources is a flat list.
    ctx.storeResource("anotherKey", makeResource("position"));
    check(ini.getResources().size() == 2, "a second model under a different key still just appends");

    IniFileResEditContext::Collected collected;
    IniFileResEditContext collectCtx(&ini, &collected);

    collectCtx.storeResource("vb0", makeResource("texcoord"));
    collectCtx.storeResource("vb0", makeResource("texcoord2"));
    collectCtx.storeResource("vb1", makeResource("draw"));

    check(ini.getResources().size() == 2, "a collect map takes the models instead of the .ini file");
    check(collected.size() == 2 && collected["vb0"].size() == 2 && collected["vb1"].size() == 1,
          "and keys them by file key, keeping every model stored under one");

    IniFileResEditContext nullCtx;
    nullCtx.storeResource("someKey", makeResource("orphan"));
    check(!nullCtx.hasIni(), "a context with no .ini file and no collect map has nowhere to store");
    check(ini.getResources().size() == 2, "...so the model is dropped rather than going anywhere else");
}


// ---------------------------------------------------------------------------------------
// The capture pass ResGroupCollect drives
// ---------------------------------------------------------------------------------------

void testCollecting() {
    std::printf("\n--- IniFileResEditContext::beginCollectingResources ---\n");

    IniFile ini("C:/Mods/TestMod/CuteLittleEi.ini");
    IniFileResEditContext::Collected collected;
    IniFileResEditContext ctx(&ini, &collected);

    check(!ctx.isCollecting(), "a context does not start out capturing");

    ctx.beginCollectingResources();
    check(ctx.isCollecting(), "beginCollectingResources opens a pass");

    ctx.storeResource("vb0", makeResource("first"));
    ctx.storeResource("vb1", makeResource("second"));

    check(ini.getResources().empty() && collected.empty(),
          "a capture pass wins over both the collect map and the .ini file");

    std::vector<std::pair<std::string, IniResource*>> taken = ctx.takeCollectedResources();
    check(taken.size() == 2, "takeCollectedResources hands back everything captured...");
    check(taken[0].first == "vb0" && taken[1].first == "vb1", "...in the order it was built, keyed by file key");

    // The whole point of the keep-alive store: these are bare pointers into models the context
    // still owns, and reading them after the hand-off has to be safe.
    check(taken[0].second != nullptr && taken[0].second->type == "first"
              && taken[1].second != nullptr && taken[1].second->type == "second",
          "and the models those pointers name are still alive afterwards");

    check(ctx.takeCollectedResources().empty(), "a second take finds the buffer emptied");

    ctx.storeResource("vb2", makeResource("third"));
    std::vector<std::pair<std::string, IniResource*>> second = ctx.takeCollectedResources();
    check(second.size() == 1 && second[0].second->type == "third", "the pass is still open, so capturing continues");
    check(taken[0].second->type == "first", "and the earlier take's models are *still* alive");

    ctx.endCollectingResources();
    check(!ctx.isCollecting(), "endCollectingResources closes the pass");

    ctx.storeResource("vb3", makeResource("afterwards"));
    check(collected.size() == 1 && collected["vb3"].size() == 1, "so later models go to the collect map again");
}


// ---------------------------------------------------------------------------------------
// What it reads off the .ini file
// ---------------------------------------------------------------------------------------

void testIniAccess() {
    std::printf("\n--- IniFileResEditContext (.ini file access) ---\n");

    IniFile ini("C:/Mods/TestMod/CuteLittleEi.ini");
    IniFileResEditContext ctx(&ini);

    check(ctx.hasIni() && ctx.getIniFile() == &ini, "a context remembers the .ini file it wraps");
    check(ctx.iniFolder() == "C:/Mods/TestMod", "the folder is derived from the file's own path");
    check(ctx.z3Ctx() == ini.getZ3Ctx(), "and the Z3 context is the .ini file's own, not a new one");

    IniFileResEditContext empty;
    check(!empty.hasIni() && empty.iniFolder().empty() && empty.z3Ctx() == nullptr,
          "a context with no .ini file answers as though there were none");
    check(empty.sectionIfTemplates().empty(), "including for its sections");

    // clear() empties ini.resources, which is exactly the danger getResources documents.
    ctx.storeResource("someKey", makeResource("blend"));
    check(ini.getResources().size() == 1, "a stored model is on the .ini file...");

    ini.clear();
    check(ini.getResources().empty(), "...and clear() drops it, as the pure-Python clearModels does");
    check(ctx.z3Ctx() == ini.getZ3Ctx(),
          "the Z3 context pointer survives a clear -- the value is replaced, not the member");
}

}


int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    testStoreDestinations();
    testCollecting();
    testIniAccess();

    if (failures == 0) {
        std::printf("\nALL PASSED\n");
        return 0;
    }

    std::printf("\n%d CHECK(S) FAILED\n", failures);
    return 1;
}
