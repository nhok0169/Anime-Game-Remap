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

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/VarSectionsCreate.h"

#include <algorithm>
#include <unordered_map>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCreate.h"


namespace AGRemapCore {
    IfTemplateRunConfig<std::string, std::string> VarSectionsCreate::runCalls() {
        return IfTemplateRunConfig<std::string, std::string>{
            IniKeywords::Run,
            [](const std::string& val) { return val; },
            [](const std::string& section) { return section; }
        };
    }

    VarSectionsCreate::VarSectionsCreate(GraphEditVariables& variables, ModObj graph, VariablesOf variablesOf,
                                         IfTemplateRunConfig<std::string, std::string> runConfig):
        graph(std::move(graph)), variablesOf(std::move(variablesOf)), runConfig(std::move(runConfig)), variables_(variables) {}

    VarSectionsCreate::GraphGroups& VarSectionsCreate::edit(GraphGroups& groups, const ModType*, const std::string& modName) {
        // the sections of a group's variables, in order
        const auto sectionsOf = [this](std::size_t group) {
            std::vector<GraphEditVariables::Section*> result;
            if (!variablesOf) {
                return result;
            }

            for (const std::string& name : variablesOf(group)) {
                if (const GraphEditVariables::Sections* variable = variables_.findSections(name)) {
                    for (GraphEditVariables::Section* section : variable->list()) {
                        result.push_back(section);
                    }
                }
            }
            return result;
        };

        // ...and the first group's into a group of its own when there is none (every group removed, say)
        const std::size_t count = std::max<std::size_t>(groups.size(), groups.size() == 0 && !sectionsOf(0).empty() ? 1 : 0);
        for (std::size_t g = 0; g < count; ++g) {
            const std::vector<GraphEditVariables::Section*> sections = sectionsOf(g);
            if (sections.empty()) {
                continue;
            }

            // in order; a repeated name keeps its first section
            std::unordered_map<std::string, GraphEditVariables::Section*> byName;
            std::vector<std::string> names;
            for (GraphEditVariables::Section* section : sections) {
                if (byName.emplace(section->name, section).second) {
                    names.push_back(section->name);
                }
            }

            GraphCreate<std::string, std::string>::Graph created(std::move(byName), names, runConfig, true, false, z3);
            GraphCreate<std::string, std::string>(GraphId(g, graph.first, graph.second), &created).edit(groups, nullptr, modName);
        }

        return groups;
    }
}
