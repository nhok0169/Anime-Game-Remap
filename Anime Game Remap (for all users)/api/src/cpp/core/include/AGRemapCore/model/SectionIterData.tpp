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
