#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"


namespace AGRemapCore {
    BaseIniGraphEdit::Graph& BaseIniGraphEdit::editFromIni(Graph& graph, IniFile& ini, const ModType& modType,
                                                            const std::string& modName, const PartFilter& partFilter) {
        // 'ini' is deliberately unused -- see this method's doc comment. The pure-Python original
        // drops it here too; only overriding subclasses actually read it.
        (void)ini;
        return edit(graph, modType, modName, partFilter);
    }

    BaseIniGraphEdit::Graph& BaseIniGraphEdit::edit(Graph& graph, const ModType& modType,
                                                     const std::string& modName, const PartFilter& partFilter) {
        (void)modType;
        (void)modName;
        (void)partFilter;
        return graph;
    }
}
