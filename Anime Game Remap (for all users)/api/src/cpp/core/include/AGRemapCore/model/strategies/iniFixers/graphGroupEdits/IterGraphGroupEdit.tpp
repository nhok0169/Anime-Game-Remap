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

#ifndef AGRemapCore_IterGraphGroupEdit_TPP
#define AGRemapCore_IterGraphGroupEdit_TPP

#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IterGraphGroupEdit<K, V, KeyHash, KeyEqual>::IterGraphGroupEdit(EditFactory makeEdits): makeEdits(std::move(makeEdits)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IterGraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphGroups& IterGraphGroupEdit<K, V, KeyHash, KeyEqual>::editFromIni(
            GraphGroups& graphGroups, IniFile* ini, const ModType* modType, const std::string& modName) {
        if (!makeEdits) {
            return graphGroups;
        }

        const std::size_t graphGroupsLen = graphGroups.size();
        for (std::size_t iniIndex = 0; iniIndex < graphGroupsLen; ++iniIndex) {
            GroupEdits groupEdits = makeEdits(iniIndex, graphGroups);
            for (const std::unique_ptr<Base>& edit : groupEdits.edits) {
                if (edit != nullptr) {
                    edit->editFromIni(graphGroups, ini, modType, modName);
                }
            }
        }

        return graphGroups;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IterGraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphGroups& IterGraphGroupEdit<K, V, KeyHash, KeyEqual>::edit(
            GraphGroups& graphGroups, const ModType* modType, const std::string& modName) {
        return editFromIni(graphGroups, nullptr, modType, modName);
    }
}

#endif
