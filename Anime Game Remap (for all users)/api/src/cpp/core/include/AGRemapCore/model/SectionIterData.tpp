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

namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    SectionIterData<K, V, KeyHash, KeyEqual>::SectionIterData(std::string sectionName, Section* section, ContentPart* part, int state, Colouring* colouring):
        sectionName(std::move(sectionName)), section(section), part(part), state(state), colouring(colouring) {

    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    SectionIterQueryData<K, V, KeyHash, KeyEqual>::SectionIterQueryData(ContentPart* part, Z3Predicate query, std::string sectionName, Section* section,
                                                                         std::string rootSectionName, Section* rootSection, int state, Colouring* colouring):
        part(part), query(std::move(query)), sectionName(std::move(sectionName)), section(section),
        rootSectionName(std::move(rootSectionName)), rootSection(rootSection), state(state), colouring(colouring) {

    }

}
