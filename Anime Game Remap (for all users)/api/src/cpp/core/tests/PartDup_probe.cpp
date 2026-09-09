// -----------------------------------------------------------------------------
// Diagnostic probe: does one section's ContentPart belong to more than one mod
// object's graph, and are those parts the SAME pointer?
//
// WHY: GanyuTwilight -> Ganyu draws the body 8 times instead of 6, because the
// downloaded `ib` is inserted TWICE into CommandListGanyuTwilightBody and the
// draw call is derived from `ib`. The suspicion is that the CommandList belongs
// to both ("", "body") and ("", "ib"), and that parseCommands builds those two
// graphs with copySections = false so they share the underlying Section.
//
// GIMIParser::getDownloads guards against exactly this with a `visitedParts` set
// shared across mod objects. Either the parts are not pointer-identical (so the
// guard cannot see the repeat), or this download takes the refToSection branch,
// which never consults the guard. This tells us which.
//
// NOT wired into any build target -- core/tests/*.cpp never is. Compile it with
// the same static-lib line the other standalone tests use.
// -----------------------------------------------------------------------------

#include <algorithm>
#include <cstdio>
#include <optional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"

namespace AGRC = AGRemapCore;

namespace {
    const char* SECTION_OF_INTEREST = "CommandListGanyuTwilightBody";

    std::string modObjStr(const std::pair<std::string, std::string>& modObj) {
        return "(\"" + modObj.first + "\", \"" + modObj.second + "\")";
    }
}


int main(int argc, char** argv) {
    if (argc < 2) {
        std::printf("usage: PartDup_probe <path to a GanyuTwilight .ini>\n");
        return 2;
    }

    AGRC::GlobalModTypes::registerAll();

    // Braces, not parens: AGRC::IniFile ini(std::string(argv[1])) is a most-vexing-parse and
    // declares a function.
    const std::string path = argv[1];
    AGRC::IniFile ini{std::optional<std::string>(path)};
    ini.classify();

    if (ini.getModTypes().empty()) {
        std::printf("FAIL: the .ini classified as no mod type at all\n");
        return 1;
    }

    for (const auto& entry : ini.getModTypes()) {
        std::printf("classified as: %s\n", entry.second.name.c_str());
    }

    AGRC::IniFile::ParseData& parsed = ini.parse();

    // section name -> mod object -> the parts that section has in THAT mod object's graph
    std::map<std::string, std::map<std::string, std::vector<const void*>>> bySection;

    for (auto& modTypeEntry : parsed) {
        for (auto& group : modTypeEntry.second) {
            for (const auto& modObj : group.modObjs()) {
                auto* graph = group.getGraph(modObj);
                if (graph == nullptr) {
                    continue;
                }

                auto walk = graph->iterByContentPart();
                while (walk.next()) {
                    auto& iterData = walk.value();
                    bySection[iterData.sectionName][modObjStr(modObj)].push_back(
                        static_cast<const void*>(iterData.part));
                }
            }
        }
    }

    // ---- FIRST: what each graph actually IS. The inversion below is only trustworthy if these
    // roots and section lists are, so print them and check by eye before believing anything else.
    std::printf("\n=== every graph: its roots, and the sections it holds ===\n");
    for (auto& modTypeEntry2 : parsed) {
        for (auto& group : modTypeEntry2.second) {
            for (const auto& modObj : group.modObjs()) {
                auto* graph = group.getGraph(modObj);
                if (graph == nullptr) {
                    std::printf("\n  %-18s <no graph>\n", modObjStr(modObj).c_str());
                    continue;
                }

                std::printf("\n  %-18s graph @ %p\n", modObjStr(modObj).c_str(),
                            static_cast<const void*>(graph));
                std::printf("      roots:");
                for (const std::string& root : graph->roots()) {
                    std::printf(" [%s]", root.c_str());
                }
                std::printf("\n      sections (%zu):", graph->sections().size());

                std::vector<std::string> names;
                for (const auto& sectionEntry : graph->sections()) {
                    names.push_back(sectionEntry.first);
                }
                std::sort(names.begin(), names.end());
                for (const std::string& name : names) {
                    std::printf(" [%s]", name.c_str());
                }
                std::printf("\n");
            }
        }
    }

    std::printf("\n=== sections belonging to MORE THAN ONE mod object ===\n");
    std::size_t shared = 0;

    for (const auto& sectionEntry : bySection) {
        if (sectionEntry.second.size() < 2) {
            continue;
        }

        ++shared;
        std::printf("\n  [%s] in %zu mod objects:\n",
                    sectionEntry.first.c_str(), sectionEntry.second.size());

        // are the parts the same pointers across those mod objects?
        std::set<const void*> allParts;
        std::size_t totalParts = 0;

        for (const auto& objEntry : sectionEntry.second) {
            std::printf("      %-18s %zu part(s):", objEntry.first.c_str(), objEntry.second.size());
            for (const void* part : objEntry.second) {
                std::printf(" %p", part);
                allParts.insert(part);
                ++totalParts;
            }
            std::printf("\n");
        }

        std::printf("      -> %zu part slot(s), %zu DISTINCT pointer(s) -- %s\n",
                    totalParts, allParts.size(),
                    allParts.size() * sectionEntry.second.size() == totalParts
                        ? "SHARED (same objects seen once per mod object)"
                        : "NOT shared (each graph has its own copies)");
    }

    if (shared == 0) {
        std::printf("  none -- every section belongs to exactly one mod object\n");
    }

    std::printf("\n=== the section under suspicion ===\n");
    auto found = bySection.find(SECTION_OF_INTEREST);
    if (found == bySection.end()) {
        std::printf("  [%s] not present in any graph\n", SECTION_OF_INTEREST);
    } else {
        std::printf("  [%s] belongs to %zu mod object(s)\n",
                    SECTION_OF_INTEREST, found->second.size());
        for (const auto& objEntry : found->second) {
            std::printf("      %s\n", objEntry.first.c_str());
        }
    }

    return 0;
}
