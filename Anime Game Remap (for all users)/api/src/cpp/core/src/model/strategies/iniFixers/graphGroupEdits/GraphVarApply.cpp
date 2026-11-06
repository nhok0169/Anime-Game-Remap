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

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphVarApply.h"

#include <utility>

#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"


namespace AGRemapCore {
    GraphVarApply::GraphVarApply(std::vector<std::string> inputs, GraphEditVariables::Apply fn, std::string into):
        inputs(std::move(inputs)), fn(std::move(fn)), into(std::move(into)) {}

    GraphVarApply::GraphGroups& GraphVarApply::edit(GraphGroups& graphGroups, const ModType*, const std::string&) {
        return graphGroups;
    }

    void GraphVarApply::fileInto(IniParseData& data) const {
        data.variables().apply(inputs, fn, into);
    }
}
