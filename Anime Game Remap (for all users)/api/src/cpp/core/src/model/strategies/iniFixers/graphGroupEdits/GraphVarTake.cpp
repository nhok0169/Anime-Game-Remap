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

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphVarTake.h"

#include <memory>


namespace AGRemapCore {
    GraphVarTake::GraphVarTake(GraphEditVariables& variables, std::string variable, ModObj graph,
                                     std::vector<std::string> sections, std::string comment, bool inPlace):
        variables_(variables), variable_(std::move(variable)), graph_(std::move(graph)), sections_(std::move(sections)),
        comment_(std::move(comment)) {
        if (inPlace) {
            place_ = variables_.sections(variable_).reserve();
        }
    }

    GraphVarTake::GraphGroups& GraphVarTake::edit(GraphGroups& graphGroups, const ModType*, const std::string&) {
        if (graphGroups.size() == 0) {
            return graphGroups;
        }

        auto* graph = graphGroups.getGraph(0, graph_);
        if (graph == nullptr) {
            return graphGroups;
        }

        GraphEditVariables::Sections& into = variables_.sections(variable_);
        for (const std::string& name : sections_) {
            GraphEditVariables::Section* section = graph->getSection(name, false);
            if (section == nullptr) {
                continue;
            }

            std::unique_ptr<GraphEditVariables::Section> copy = section->deepcopy();
            copy->prefix = comment_.empty() ? "" : "; " + comment_;
            copy->rebuild();
            if (place_.has_value()) {
                into.addAt(std::move(copy), *place_);
            } else {
                into.add(std::move(copy));
            }
        }

        graphGroups.removeGraph(0, graph_);
        return graphGroups;
    }
}
