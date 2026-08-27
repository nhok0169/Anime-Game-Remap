#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"

#include <stdexcept>
#include <utility>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"


namespace AGRemapCore {
    BaseIniGraphGroupEdit::GraphGroups& BaseIniGraphGroupEdit::editFromIni(GraphGroups& graphGroups, IniFile& ini,
                                                                            const ModType& modType, const std::string& modName) {
        // 'ini' is deliberately unused -- see this method's doc comment.
        (void)ini;
        return edit(graphGroups, modType, modName);
    }

    BaseIniGraphGroupEdit::GraphGroups& BaseIniGraphGroupEdit::edit(GraphGroups& graphGroups, const ModType& modType,
                                                                     const std::string& modName) {
        (void)modType;
        (void)modName;
        return graphGroups;
    }

    BaseIniGraphGroupEdit::Graph* BaseIniGraphGroupEdit::getGraph(GraphGroups& graphGroups, const GraphId& id, bool errorOnNotFound) {
        Graph* result = nullptr;

        // Matches the pure-Python original's "if (iniInd < len(graphGroups))" guard -- an
        // out-of-range .ini index is a miss, not an out-of-bounds access.
        if (id.iniIndex < graphGroups.size()) {
            result = graphGroups[id.iniIndex].getGraph(id.modObj);
        }

        if (result == nullptr && errorOnNotFound) {
            throw std::out_of_range("No .ini graph found by the key: (" + std::to_string(id.iniIndex) + ", " +
                                    id.modObj.first + ", " + id.modObj.second + ")");
        }

        return result;
    }

    bool BaseIniGraphGroupEdit::addGraph(GraphGroups& graphGroups, const GraphId& id, Graph graph) {
        if (id.iniIndex >= graphGroups.size()) {
            return false;
        }

        graphGroups[id.iniIndex].addGraph(id.modObj, std::move(graph));
        return true;
    }
}
