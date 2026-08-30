#ifndef AGRemapCore_BaseIniGraphGroupEdit_TPP
#define AGRemapCore_BaseIniGraphGroupEdit_TPP

#include <stdexcept>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphGroups& BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>::editFromIni(GraphGroups& graphGroups, IniFile* ini,
                                                                                                                                      const ModType* modType, const std::string& modName) {
        // 'ini' is deliberately unused -- see this method's doc comment.
        (void)ini;
        return edit(graphGroups, modType, modName);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphGroups& BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>::edit(GraphGroups& graphGroups, const ModType* modType,
                                                                                                                               const std::string& modName) {
        (void)modType;
        (void)modName;
        return graphGroups;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>::Graph* BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>::getGraph(const GraphGroups& graphGroups, const GraphId& id,
                                                                                                                             bool errorOnNotFound) {
        // Matches the pure-Python original's "if (iniInd < len(graphGroups))" guard -- an
        // out-of-range .ini index is a miss, not an out-of-bounds access.
        Graph* result = graphGroups.getGraph(id.iniIndex, id.modObj);

        if (result == nullptr && errorOnNotFound) {
            throw std::out_of_range("No .ini graph found by the key: (" + std::to_string(id.iniIndex) + ", " +
                                    id.modObj.first + ", " + id.modObj.second + ")");
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>::addGraph(GraphGroups& graphGroups, const GraphId& id, Graph* graph) {
        if (id.iniIndex >= graphGroups.size()) {
            return false;
        }

        graphGroups.addGraph(id.iniIndex, id.modObj, graph);
        return true;
    }
}

#endif
