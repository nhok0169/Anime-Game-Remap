#include <algorithm>

namespace AGRemapCore {

    namespace {
        // Dedupes 'items' while preserving first-seen order -- the C++ equivalent of every
        // OrderedSet(...) use in the pure-Python original this class ports.
        template <typename T>
        std::vector<T> dedupeKeepOrder(const std::vector<T>& items) {
            std::vector<T> result;
            std::unordered_set<T> seen;
            result.reserve(items.size());
            for (const T& item : items) {
                if (seen.insert(item).second) {
                    result.push_back(item);
                }
            }
            return result;
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IniSectionGraph<K, V, KeyHash, KeyEqual>::IniSectionGraph(std::unordered_map<std::string, Section*> sections, std::vector<std::string> targetSectionNames,
                                                               IfTemplateRunConfig<K, V> runConfig, bool doBuild, bool copySections, Z3Context* z3Ctx):
        sections_(std::move(sections)), runConfig_(std::move(runConfig)), z3Ctx_(z3Ctx) {
        setTargetSectionNamesImpl(std::move(targetSectionNames));

        if (doBuild) {
            build(std::nullopt, std::nullopt, copySections);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniSectionGraph<K, V, KeyHash, KeyEqual>::setTargetSectionNamesImpl(std::vector<std::string> newTargetSections) {
        targetSectionNames_ = dedupeKeepOrder(newTargetSections);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::unordered_map<std::string, typename IniSectionGraph<K, V, KeyHash, KeyEqual>::Section*>& IniSectionGraph<K, V, KeyHash, KeyEqual>::sections() const {
        return sections_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::unordered_map<std::string, std::vector<std::string>>& IniSectionGraph<K, V, KeyHash, KeyEqual>::neighbours() const {
        return neighbours_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<std::string>& IniSectionGraph<K, V, KeyHash, KeyEqual>::roots() const {
        return roots_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<std::string>& IniSectionGraph<K, V, KeyHash, KeyEqual>::targetSectionNames() const {
        return targetSectionNames_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniSectionGraph<K, V, KeyHash, KeyEqual>::setTargetSectionNames(std::vector<std::string> newTargetSections) {
        setTargetSectionNamesImpl(std::move(newTargetSections));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const IfTemplateRunConfig<K, V>& IniSectionGraph<K, V, KeyHash, KeyEqual>::runConfig() const {
        return runConfig_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    Z3Context* IniSectionGraph<K, V, KeyHash, KeyEqual>::z3Ctx() const {
        return z3Ctx_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<std::string, typename IniSectionGraph<K, V, KeyHash, KeyEqual>::Section*> IniSectionGraph<K, V, KeyHash, KeyEqual>::deepCopySections(
            const std::unordered_map<std::string, Section*>& src, std::vector<std::unique_ptr<Section>>& storage,
            bool newPartIds) {
        std::unordered_map<std::string, Section*> result;
        for (const auto& entry : src) {
            storage.push_back(entry.second->deepcopy(newPartIds));
            result[entry.first] = storage.back().get();
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniSectionGraph<K, V, KeyHash, KeyEqual>::combine(const std::vector<IniSectionGraph<K, V, KeyHash, KeyEqual>*>& newGraphs) {
        std::vector<std::string> allTargets = targetSectionNames_;
        for (auto* graph : newGraphs) {
            for (const auto& name : graph->targetSectionNames()) {
                allTargets.push_back(name);
            }
        }
        setTargetSectionNamesImpl(std::move(allTargets));

        for (auto* graph : newGraphs) {
            for (const auto& entry : graph->sections()) {
                sections_[entry.first] = entry.second;
            }
        }

        build(sections_, targetSectionNames_, false);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniSectionGraph<K, V, KeyHash, KeyEqual>::build(std::optional<std::unordered_map<std::string, Section*>> sectionsArg,
                                                          std::optional<std::vector<std::string>> targetSectionNamesArg, bool copySections) {
        std::unordered_map<std::string, Section*> sections;
        if (sectionsArg.has_value()) {
            sections_ = std::move(*sectionsArg);
            sections = sections_;
        } else {
            sections = sections_;
        }

        std::vector<std::string> targets;
        if (targetSectionNamesArg.has_value()) {
            setTargetSectionNamesImpl(std::move(*targetSectionNamesArg));
        }
        targets = targetSectionNames_;

        roots_.clear();
        neighbours_.clear();

        std::unordered_set<std::string> visited;
        std::unordered_map<std::string, std::vector<std::string>> neighboursOrdered;
        std::unordered_map<std::string, std::unordered_set<std::string>> neighboursSeen;
        std::unordered_map<std::string, std::string> parents;
        std::unordered_map<std::string, Section*> resultSections;

        for (const std::string& sectionName : targets) {
            auto secIt = sections.find(sectionName);
            if (visited.count(sectionName) || secIt == sections.end()) {
                continue;
            }

            visited.insert(sectionName);
            std::deque<std::pair<std::string, Section*>> stack;
            stack.push_back({sectionName, secIt->second});
            resultSections[sectionName] = secIt->second;

            while (!stack.empty()) {
                auto [currentSectionName, currentSection] = stack.back();
                stack.pop_back();

                std::deque<std::pair<std::string, Section*>> currentToExplore;

                for (const auto& csEntry : currentSection->calledSubCommands()) {
                    const std::vector<std::string>& subSections = csEntry.second;

                    for (const std::string& subSectionName : subSections) {
                        auto neighbourIt = sections.find(subSectionName);
                        if (neighbourIt == sections.end()) {
                            continue;
                        }

                        if (neighboursSeen[currentSectionName].insert(subSectionName).second) {
                            neighboursOrdered[currentSectionName].push_back(subSectionName);
                        }

                        if (parents.find(subSectionName) == parents.end()) {
                            parents[subSectionName] = currentSectionName;
                        }

                        if (!visited.count(subSectionName)) {
                            visited.insert(subSectionName);
                            resultSections[subSectionName] = neighbourIt->second;
                            currentToExplore.push_back({subSectionName, neighbourIt->second});
                        }
                    }
                }

                for (auto& item : currentToExplore) {
                    stack.push_back(std::move(item));
                }
            }
        }

        neighbours_ = std::move(neighboursOrdered);

        // get the roots
        std::vector<std::string> rootsOrdered;
        std::unordered_set<std::string> rootsSeen;
        std::unordered_map<std::string, std::string> nodeRoots;

        for (const std::string& sectionName : targets) {
            std::string currentSectionName = sectionName;
            std::unordered_set<std::string> pathNodes{currentSectionName};
            bool addRoot = true;

            while (parents.find(currentSectionName) != parents.end()) {
                currentSectionName = parents.at(currentSectionName);

                auto nrIt = nodeRoots.find(currentSectionName);
                if (nrIt != nodeRoots.end()) {
                    currentSectionName = nrIt->second;
                    break;
                }

                pathNodes.insert(currentSectionName);

                if (rootsSeen.count(currentSectionName)) {
                    addRoot = false;
                    break;
                }

                if (currentSectionName == sectionName) {
                    break;
                }
            }

            if (addRoot) {
                if (rootsSeen.insert(currentSectionName).second) {
                    rootsOrdered.push_back(currentSectionName);
                }
                for (const std::string& node : pathNodes) {
                    nodeRoots[node] = currentSectionName;
                }
            }
        }

        roots_ = std::move(rootsOrdered);

        if (copySections) {
            ownedSections_.clear();
            resultSections = deepCopySections(resultSections, ownedSections_);
        }

        sections_ = std::move(resultSections);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IniSectionGraph<K, V, KeyHash, KeyEqual>::Section* IniSectionGraph<K, V, KeyHash, KeyEqual>::getSection(const std::string& sectionName, bool raiseException) const {
        auto it = sections_.find(sectionName);
        if (it == sections_.end()) {
            if (raiseException) {
                throw std::out_of_range("The section by the name '" + sectionName + "' does not exist");
            }
            return nullptr;
        }
        return it->second;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::Section*> IniSectionGraph<K, V, KeyHash, KeyEqual>::getRootSections() const {
        std::vector<Section*> result;
        result.reserve(roots_.size());
        for (const std::string& root : roots_) {
            result.push_back(getSection(root));
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool IniSectionGraph<K, V, KeyHash, KeyEqual>::isEmpty() const {
        return roots_.empty();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::string> IniSectionGraph<K, V, KeyHash, KeyEqual>::getNeighbourNames(const std::string& sectionName) const {
        auto it = neighbours_.find(sectionName);
        if (it == neighbours_.end()) {
            return {};
        }
        return it->second;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<std::string, typename IniSectionGraph<K, V, KeyHash, KeyEqual>::Section*> IniSectionGraph<K, V, KeyHash, KeyEqual>::getNeighbours(const std::string& sectionName) const {
        std::unordered_map<std::string, Section*> result;
        auto it = neighbours_.find(sectionName);
        if (it == neighbours_.end()) {
            return result;
        }

        for (const std::string& neighbourName : it->second) {
            Section* section = getSection(neighbourName, false);
            if (section != nullptr) {
                result[neighbourName] = section;
            }
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<std::string, std::vector<std::string>> IniSectionGraph<K, V, KeyHash, KeyEqual>::getChildren(
            const std::vector<std::string>& targetSections, bool getNeighbourChildren) const {
        std::unordered_map<std::string, std::vector<std::string>> result;
        std::vector<std::string> targets = dedupeKeepOrder(targetSections);

        const int exploreState = 0;
        const int addState = 1;

        for (const std::string& section : targets) {
            std::unordered_set<std::string> visited;
            std::deque<std::pair<int, std::string>> stack;
            stack.push_back({exploreState, section});

            while (!stack.empty()) {
                auto [state, currentSection] = stack.back();
                stack.pop_back();

                auto neighboursIt = neighbours_.find(currentSection);
                std::vector<std::string> neighboursHere = (neighboursIt != neighbours_.end()) ? neighboursIt->second : std::vector<std::string>{};

                if (state == exploreState) {
                    stack.push_back({addState, currentSection});

                    for (const std::string& neighbour : neighboursHere) {
                        if (!visited.count(neighbour)) {
                            stack.push_back({exploreState, neighbour});
                            visited.insert(neighbour);
                        }
                    }
                    continue;
                }

                std::vector<std::string> currentResultOrdered;
                std::unordered_set<std::string> currentResultSeen;

                for (const std::string& neighbour : neighboursHere) {
                    if (currentResultSeen.insert(neighbour).second) {
                        currentResultOrdered.push_back(neighbour);
                    }

                    auto childIt = result.find(neighbour);
                    if (childIt != result.end()) {
                        for (const std::string& n : childIt->second) {
                            if (currentResultSeen.insert(n).second) {
                                currentResultOrdered.push_back(n);
                            }
                        }
                    }
                }

                result[currentSection] = std::move(currentResultOrdered);
            }
        }

        for (const std::string& section : targets) {
            if (result.find(section) == result.end()) {
                result[section] = {};
            }
        }

        if (getNeighbourChildren) {
            return result;
        }

        std::unordered_set<std::string> targetSet(targets.begin(), targets.end());
        std::vector<std::string> toErase;
        for (const auto& entry : result) {
            if (!targetSet.count(entry.first)) {
                toErase.push_back(entry.first);
            }
        }
        for (const std::string& k : toErase) {
            result.erase(k);
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniSectionGraph<K, V, KeyHash, KeyEqual>::rename(const std::function<std::string(const std::string&)>& renameFunc) {
        std::deque<std::string> stack;
        std::unordered_map<std::string, Section*> visitedSections;
        std::unordered_map<std::string, std::string> renamedSections;
        std::vector<std::string> targetSectionNamesOrdered = targetSectionNames_;
        std::unordered_set<std::string> targetSectionNamesSet(targetSectionNamesOrdered.begin(), targetSectionNamesOrdered.end());
        bool hasRenamed = false;

        for (auto it = roots_.rbegin(); it != roots_.rend(); ++it) {
            stack.push_front(*it);
        }

        while (!stack.empty()) {
            std::string sectionName = stack.front();
            stack.pop_front();
            if (visitedSections.find(sectionName) != visitedSections.end()) {
                continue;
            }

            std::string newSectionName = renameFunc(sectionName);
            Section* section = nullptr;

            if (newSectionName != sectionName) {
                auto secIt = sections_.find(sectionName);
                section = (secIt != sections_.end()) ? secIt->second : nullptr;
                if (secIt != sections_.end()) {
                    sections_.erase(secIt);
                }
                if (section != nullptr) {
                    section->name = newSectionName;
                }
                renamedSections[sectionName] = newSectionName;

                if (section != nullptr && sections_.find(newSectionName) == sections_.end()) {
                    sections_[newSectionName] = section;
                }

                if (targetSectionNamesSet.count(sectionName)) {
                    auto eraseIt = std::find(targetSectionNamesOrdered.begin(), targetSectionNamesOrdered.end(), sectionName);
                    if (eraseIt != targetSectionNamesOrdered.end()) {
                        targetSectionNamesOrdered.erase(eraseIt);
                    }
                    targetSectionNamesSet.erase(sectionName);
                    if (targetSectionNamesSet.insert(newSectionName).second) {
                        targetSectionNamesOrdered.push_back(newSectionName);
                    }
                }

                hasRenamed = true;
            } else {
                section = getSection(sectionName);
            }

            visitedSections[sectionName] = section;

            auto neighboursIt = neighbours_.find(sectionName);
            if (neighboursIt != neighbours_.end()) {
                for (auto it = neighboursIt->second.rbegin(); it != neighboursIt->second.rend(); ++it) {
                    stack.push_back(*it);
                }
            }
        }

        // rename the calls to the section -- only touches parts that already have a calledSubCommands
        // entry (renaming never creates a new 'run =' reference), matching the pure-Python original
        // precisely (it iterates 'section.calledSubCommands', not every part).
        for (const auto& entry : visitedSections) {
            Section* section = entry.second;
            if (section == nullptr) {
                continue;
            }

            std::vector<size_t> partInds;
            for (const auto& csEntry : section->calledSubCommands()) {
                partInds.push_back(csEntry.first);
            }

            for (size_t partInd : partInds) {
                auto* contentPart = dynamic_cast<ContentPart*>(section->parts().at(partInd).get());
                if (contentPart == nullptr) {
                    continue;
                }

                auto runValsWithInds = contentPart->getValsWithInds(runConfig_.runKey);
                for (const auto& indexedVal : runValsWithInds) {
                    std::string runVal = runConfig_.sectionNameOf(indexedVal.second);
                    auto renameIt = renamedSections.find(runVal);
                    if (renameIt != renamedSections.end()) {
                        contentPart->setValByInd(indexedVal.first, runConfig_.valOfSectionName(renameIt->second));
                    }
                }

                section->refreshCalledSubCommand(partInd);
            }
        }

        if (hasRenamed) {
            setTargetSectionNamesImpl(std::move(targetSectionNamesOrdered));
            build(sections_, targetSectionNames_, false);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniSectionGraph<K, V, KeyHash, KeyEqual>::refreshPartIds(bool minimal) {
        if (minimal) {
            for (const auto& entry : sections_) {
                entry.second->refreshPartIds();
            }
            return;
        }

        // Non-minimal: every section this graph was originally constructed with, not just the
        // ones the DFS kept -- but this graph, unlike the pure-Python original, doesn't retain a
        // separate "all sections passed in" collection once build() has trimmed #sections_ down to
        // the reachable subset. Matches the practical effect for every real call site (none pass
        // minimal=false today) -- refresh whatever #sections_ currently holds either way.
        for (const auto& entry : sections_) {
            entry.second->refreshPartIds();
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<IniSectionGraph<K, V, KeyHash, KeyEqual>> IniSectionGraph<K, V, KeyHash, KeyEqual>::deepcopy(bool minimal, bool newPartIds) const {
        (void)minimal;  // this port's #sections_ is always already "minimal" (the DFS-trimmed set) -- see refreshPartIds's own note
        auto result = std::unique_ptr<IniSectionGraph<K, V, KeyHash, KeyEqual>>(
            new IniSectionGraph<K, V, KeyHash, KeyEqual>(sections_, targetSectionNames_, runConfig_, false, false, z3Ctx_));

        result->ownedSections_.clear();

        // BUGFIX: this used to clone every section with newPartIds = true unconditionally, so a
        // copy always got fresh part ids and 'newPartIds = false' only skipped the *second*
        // renumbering below -- ie. the flag could not do the one thing it is documented to do.
        // Anything correlating parts across a copy by id (ResGroupCollect's resource file keys,
        // ResRegCollect's rewrite of a remapped call site) silently matched nothing as a result.
        result->sections_ = IniSectionGraph<K, V, KeyHash, KeyEqual>::deepCopySections(sections_, result->ownedSections_,
                                                                                        newPartIds);

        if (newPartIds) {
            result->refreshPartIds(true);
        }

        result->neighbours_ = neighbours_;
        result->roots_ = roots_;
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<std::string, bool> IniSectionGraph<K, V, KeyHash, KeyEqual>::isKeyFullyCover(const K& key) const {
        std::unordered_set<std::string> visited;
        std::unordered_map<std::string, bool> sectionsKeyFullCover;

        for (const std::string& sectionName : roots_) {
            Section* section = getSection(sectionName);
            section->isKeyFullyCover(key, sections_, visited, sectionsKeyFullCover);
        }

        return sectionsKeyFullCover;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<std::string, bool> IniSectionGraph<K, V, KeyHash, KeyEqual>::rootsAreFullyCovered(const K& key) const {
        auto sectionsKeyFullCover = isKeyFullyCover(key);
        std::unordered_map<std::string, bool> result;
        for (const std::string& sectionName : roots_) {
            result[sectionName] = sectionsKeyFullCover.at(sectionName);
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<std::string, std::set<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::ContentPart*>> IniSectionGraph<K, V, KeyHash, KeyEqual>::getKeyMissingParts(const K& key) const {
        std::unordered_set<std::string> visited;
        std::unordered_map<std::string, std::set<ContentPart*>> sectionsMissingParts;
        std::unordered_map<std::string, bool> sectionAllBranchesMissing;

        for (const std::string& sectionName : roots_) {
            Section* section = getSection(sectionName);
            section->getKeyMissingParts(key, sections_, visited, sectionsMissingParts, sectionAllBranchesMissing);
        }

        return sectionsMissingParts;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::ContentPart*, std::vector<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::ContentPart*>>
    IniSectionGraph<K, V, KeyHash, KeyEqual>::computeSectionPredecessors(const std::vector<std::unique_ptr<IfTemplatePart>>& parts) {
        std::unordered_map<ContentPart*, std::vector<ContentPart*>> predecessors;
        std::vector<ContentPart*> current;

        struct Frame {
            std::vector<ContentPart*> preEntry;
            std::vector<std::vector<ContentPart*>> branchExits;
            bool hasElse = false;
        };
        std::vector<Frame> stack;

        for (const auto& partOwned : parts) {
            IfTemplatePart* partBase = partOwned.get();

            if (auto* contentPart = dynamic_cast<ContentPart*>(partBase)) {
                predecessors[contentPart] = current;
                current = {contentPart};
                continue;
            }

            auto* predPart = dynamic_cast<IfPredPart*>(partBase);
            IfPredPartType predType = predPart->type;

            if (predType == IfPredPartType::If) {
                Frame frame;
                frame.preEntry = current;
                stack.push_back(std::move(frame));
                current = stack.back().preEntry;
            } else if (predType == IfPredPartType::Elif) {
                stack.back().branchExits.push_back(current);
                current = stack.back().preEntry;
            } else if (predType == IfPredPartType::Else) {
                stack.back().branchExits.push_back(current);
                stack.back().hasElse = true;
                current = stack.back().preEntry;
            } else if (predType == IfPredPartType::EndIf) {
                Frame frame = std::move(stack.back());
                stack.pop_back();

                std::vector<std::vector<ContentPart*>> exits = frame.branchExits;
                exits.push_back(current);
                if (!frame.hasElse) {
                    exits.push_back(frame.preEntry);
                }

                std::vector<ContentPart*> merged;
                std::unordered_set<ContentPart*> mergedSeen;
                for (auto& group : exits) {
                    for (auto* pid : group) {
                        if (mergedSeen.insert(pid).second) {
                            merged.push_back(pid);
                        }
                    }
                }
                current = std::move(merged);
            }
        }

        return predecessors;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::ContentPart*, std::vector<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::ContentPart*>>
    IniSectionGraph<K, V, KeyHash, KeyEqual>::buildPartPredecessorGraph() const {
        std::unordered_map<ContentPart*, std::vector<ContentPart*>> predecessors;
        std::unordered_map<std::string, std::vector<ContentPart*>> entryPoints;

        for (const auto& entry : sections_) {
            auto sectionPredecessors = computeSectionPredecessors(entry.second->parts());
            std::vector<ContentPart*> eps;

            for (auto& p : sectionPredecessors) {
                predecessors[p.first] = p.second;
                if (p.second.empty()) {
                    eps.push_back(p.first);
                }
            }
            entryPoints[entry.first] = std::move(eps);
        }

        for (const auto& entry : sections_) {
            for (const auto& partOwned : entry.second->parts()) {
                auto* contentPart = dynamic_cast<ContentPart*>(partOwned.get());
                if (contentPart == nullptr) {
                    continue;
                }

                for (const V& targetVal : contentPart->getVals(runConfig_.runKey)) {
                    std::string target = runConfig_.sectionNameOf(targetVal);
                    auto epIt = entryPoints.find(target);
                    if (epIt == entryPoints.end()) {
                        continue;
                    }

                    for (auto* entryId : epIt->second) {
                        auto& entryPredecessors = predecessors[entryId];
                        if (std::find(entryPredecessors.begin(), entryPredecessors.end(), contentPart) == entryPredecessors.end()) {
                            entryPredecessors.push_back(contentPart);
                        }
                    }
                }
            }
        }

        return predecessors;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::CallGraphType> IniSectionGraph<K, V, KeyHash, KeyEqual>::buildCallGraph() const {
        using Node = typename CallGraphType::Node;

        std::unordered_set<ContentPart*> allParts;
        std::unordered_map<ContentPart*, std::vector<ContentPart*>> withinSuccessors;
        std::unordered_map<ContentPart*, std::string> sectionOfPart;
        std::unordered_map<ContentPart*, std::vector<std::string>> callTargetsOfPart;
        std::unordered_map<std::string, std::vector<ContentPart*>> callersOfSection;
        std::unordered_map<std::string, std::vector<ContentPart*>> entryPointsOfSection;

        for (const auto& entry : sections_) {
            const std::string& sectionName = entry.first;
            Section* section = entry.second;
            auto sectionPredecessors = computeSectionPredecessors(section->parts());

            std::vector<ContentPart*> entryPoints;
            for (auto& p : sectionPredecessors) {
                if (p.second.empty()) {
                    entryPoints.push_back(p.first);
                }
            }
            entryPointsOfSection[sectionName] = entryPoints;

            std::unordered_map<ContentPart*, std::vector<ContentPart*>> succ;
            for (auto& p : sectionPredecessors) {
                for (auto* predId : p.second) {
                    succ[predId].push_back(p.first);
                }
            }

            for (const auto& partOwned : section->parts()) {
                auto* contentPart = dynamic_cast<ContentPart*>(partOwned.get());
                if (contentPart == nullptr) {
                    continue;
                }

                allParts.insert(contentPart);
                sectionOfPart[contentPart] = sectionName;

                auto succIt = succ.find(contentPart);
                withinSuccessors[contentPart] = (succIt != succ.end()) ? succIt->second : std::vector<ContentPart*>{};

                std::vector<std::string> targets;
                for (const V& v : contentPart->getVals(runConfig_.runKey)) {
                    targets.push_back(runConfig_.sectionNameOf(v));
                }
                callTargetsOfPart[contentPart] = targets;
                for (const auto& target : targets) {
                    callersOfSection[target].push_back(contentPart);
                }
            }
        }

        typename CallGraphType::EdgeMap forwardEdges;

        auto exitNodeOfLocal = [&](ContentPart* pid) -> Node {
            auto it = callTargetsOfPart.find(pid);
            bool hasTargets = it != callTargetsOfPart.end() && !it->second.empty();
            return Node{pid, hasTargets};
        };

        for (auto* pid : allParts) {
            auto targetsIt = callTargetsOfPart.find(pid);
            std::vector<std::string> targets = (targetsIt != callTargetsOfPart.end()) ? targetsIt->second : std::vector<std::string>{};

            Node selfExit{pid, false};
            if (!targets.empty()) {
                for (const auto& target : targets) {
                    auto epIt = entryPointsOfSection.find(target);
                    if (epIt == entryPointsOfSection.end()) {
                        continue;
                    }
                    for (auto* entryId : epIt->second) {
                        forwardEdges[Node{pid, false}].push_back(Node{entryId, false});
                    }
                }
                selfExit = Node{pid, true};
            }

            auto succIt2 = withinSuccessors.find(pid);
            std::vector<ContentPart*> succs = (succIt2 != withinSuccessors.end()) ? succIt2->second : std::vector<ContentPart*>{};

            if (!succs.empty()) {
                for (auto* succ : succs) {
                    forwardEdges[selfExit].push_back(Node{succ, false});
                }
            } else {
                auto secIt = sectionOfPart.find(pid);
                if (secIt != sectionOfPart.end()) {
                    auto callersIt = callersOfSection.find(secIt->second);
                    if (callersIt != callersOfSection.end()) {
                        for (auto* callerId : callersIt->second) {
                            forwardEdges[selfExit].push_back(exitNodeOfLocal(callerId));
                        }
                    }
                }
            }
        }

        typename CallGraphType::EdgeMap backwardEdges;
        for (auto& srcEntry : forwardEdges) {
            for (auto& dst : srcEntry.second) {
                backwardEdges[dst].push_back(srcEntry.first);
            }
        }

        std::unordered_set<ContentPart*> rootNodeIds;
        for (const std::string& rootName : roots_) {
            auto epIt = entryPointsOfSection.find(rootName);
            if (epIt != entryPointsOfSection.end()) {
                for (auto* p : epIt->second) {
                    rootNodeIds.insert(p);
                }
            }
        }

        return std::make_unique<CallGraphType>(std::move(forwardEdges), std::move(backwardEdges), std::move(allParts), std::move(rootNodeIds), runConfig_.runKey);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniSectionGraph<K, V, KeyHash, KeyEqual>::normalize() {
        for (auto& entry : sections_) {
            entry.second->normalize();
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    Z3Predicate IniSectionGraph<K, V, KeyHash, KeyEqual>::trueQuery() const {
        if (z3Ctx_ == nullptr) {
            throw std::invalid_argument(
                "This IniSectionGraph has no associated Z3Context (z3Ctx was not provided at construction) -- "
                "cannot build a query for a part with no enclosing if/elif/else predicate.");
        }

        if (!trueQueryCache_.has_value()) {
            trueQueryCache_ = Z3Predicate::trueValue(*z3Ctx_);
        }

        return *trueQueryCache_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    Z3Predicate IniSectionGraph<K, V, KeyHash, KeyEqual>::getQuery(const std::vector<Z3Predicate>& queryPath, bool simplify) const {
        size_t queryPathLen = queryPath.size();
        Z3Predicate query = (queryPathLen == 0) ? trueQuery() : queryPath[0];

        for (size_t i = 1; i < queryPathLen; ++i) {
            query = query & queryPath[i];
        }

        if (simplify && queryPathLen >= 1) {
            query = query.simplify();
        }

        return query;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    Generator<std::pair<std::string, typename IniSectionGraph<K, V, KeyHash, KeyEqual>::Section*>> IniSectionGraph<K, V, KeyHash, KeyEqual>::iterSections() const {
        std::deque<std::string> stack;
        std::unordered_set<std::string> visitedSections;

        for (const auto& root : roots_) {
            stack.push_front(root);
        }

        while (!stack.empty()) {
            std::string sectionName = stack.back();
            stack.pop_back();
            if (visitedSections.count(sectionName)) {
                continue;
            }

            Section* section = getSection(sectionName);
            co_yield std::make_pair(sectionName, section);
            visitedSections.insert(sectionName);

            auto neighboursIt = neighbours_.find(sectionName);
            if (neighboursIt != neighbours_.end()) {
                for (auto it = neighboursIt->second.rbegin(); it != neighboursIt->second.rend(); ++it) {
                    stack.push_back(*it);
                }
            }
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    Generator<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::IterData> IniSectionGraph<K, V, KeyHash, KeyEqual>::iterSectsByContentPart(
            const std::unordered_map<std::string, Section*>& sections, const std::vector<std::string>& roots,
            const IfTemplateRunConfig<K, V>& runConfig, int states, bool colour, std::optional<std::unordered_set<K, KeyHash, KeyEqual>> colourKeys) {
        using PartVariant = std::variant<ContentPart*, NodeType*>;

        struct VisitKey {
            std::string sectionName;
            PartVariant part;
            bool operator==(const VisitKey& other) const {
                return sectionName == other.sectionName && part == other.part;
            }
        };
        struct VisitKeyHash {
            size_t operator()(const VisitKey& k) const {
                size_t h1 = std::hash<std::string>()(k.sectionName);
                size_t h2 = std::visit([](auto* p) { return std::hash<const void*>()(static_cast<const void*>(p)); }, k.part);
                return h1 ^ (h2 * 1000003u + 7u);
            }
        };
        struct StackFrame {
            std::string sectionName;
            Section* section;
            NodeType* node;
            PartVariant part;
            int state;
        };

        int processStates = states;
        if (colour && states % 2 == 1) {
            processStates += 1;
        }

        for (int currentState = 1; currentState <= processStates; currentState += 2) {
            std::vector<StackFrame> stack;
            std::unordered_set<VisitKey, VisitKeyHash> visitedParts;
            std::vector<VisitKey> visitedOrder;  // meaningful only when colour == true (LIFO undo on backtrack)

            Colouring colouring;
            std::unordered_map<VisitKey, std::vector<ColourChangeSet>, VisitKeyHash> colourChanges;

            for (auto it = roots.rbegin(); it != roots.rend(); ++it) {
                const std::string& rootName = *it;
                auto secIt = sections.find(rootName);
                if (secIt == sections.end()) {
                    continue;
                }

                Section* section = secIt->second;
                NodeType* rootNode = section->tree()->root();
                if (rootNode == nullptr) {
                    continue;
                }

                VisitKey key{rootName, PartVariant(rootNode)};
                if (visitedParts.count(key)) {
                    continue;
                }

                stack.push_back(StackFrame{rootName, section, rootNode, PartVariant(rootNode), currentState});
                visitedParts.insert(key);
                visitedOrder.push_back(key);
            }

            if (colour) {
                visitedParts.clear();
                visitedOrder.clear();
            }

            while (!stack.empty()) {
                StackFrame frame = std::move(stack.back());
                stack.pop_back();

                VisitKey nodeId{frame.sectionName, PartVariant(frame.node)};
                VisitKey partId{frame.sectionName, frame.part};
                visitedParts.insert(partId);
                if (colour) {
                    visitedOrder.push_back(partId);
                }

                bool isProcessState = frame.state == currentState;
                bool partIsIfContent = std::holds_alternative<ContentPart*>(frame.part);

                if (frame.state < processStates && isProcessState) {
                    if (colour && partIsIfContent) {
                        ColourChangeSet newColourChange = colouring.updateColouring(*std::get<ContentPart*>(frame.part), colourKeys);
                        colourChanges[nodeId].push_back(std::move(newColourChange));
                    }

                    StackFrame nextFrame = frame;
                    nextFrame.state = frame.state + 1;
                    stack.push_back(std::move(nextFrame));
                }

                if (partIsIfContent) {
                    if (frame.state <= states) {
                        co_yield IterData(frame.sectionName, frame.section, std::get<ContentPart*>(frame.part), frame.state, colour ? &colouring : nullptr);
                    }
                }

                if (!isProcessState) {
                    if (colour && !partIsIfContent) {
                        auto ccIt = colourChanges.find(nodeId);
                        if (ccIt != colourChanges.end()) {
                            for (auto it2 = ccIt->second.rbegin(); it2 != ccIt->second.rend(); ++it2) {
                                colouring.restore(*it2);
                            }
                            colourChanges.erase(ccIt);
                        }
                    }

                    if (colour && !visitedOrder.empty()) {
                        visitedParts.erase(visitedOrder.back());
                        visitedOrder.pop_back();
                    }

                    continue;
                }

                std::vector<StackFrame> neighbours;
                if (partIsIfContent) {
                    ContentPart* cp = std::get<ContentPart*>(frame.part);
                    if (cp->contains(runConfig.runKey)) {
                        for (const V& neighbourVal : cp->getVals(runConfig.runKey)) {
                            std::string neighbourName = runConfig.sectionNameOf(neighbourVal);
                            auto nsIt = sections.find(neighbourName);
                            if (nsIt == sections.end()) {
                                continue;
                            }

                            NodeType* neighbourRootNode = nsIt->second->tree()->root();
                            if (neighbourRootNode == nullptr) {
                                continue;
                            }

                            VisitKey nk{neighbourName, PartVariant(neighbourRootNode)};
                            if (visitedParts.count(nk)) {
                                continue;
                            }

                            neighbours.push_back(StackFrame{neighbourName, nsIt->second, neighbourRootNode, PartVariant(neighbourRootNode), currentState});
                        }
                    }
                } else {
                    NodeType* currentNodeAsPart = std::get<NodeType*>(frame.part);
                    for (const auto& partElement : currentNodeAsPart->parts()) {
                        VisitKey pk{frame.sectionName, partElement};
                        if (visitedParts.count(pk)) {
                            continue;
                        }

                        NodeType* neighbourNode = std::holds_alternative<NodeType*>(partElement) ? std::get<NodeType*>(partElement) : currentNodeAsPart;
                        neighbours.push_back(StackFrame{frame.sectionName, frame.section, neighbourNode, partElement, currentState});
                    }
                }

                for (auto it2 = neighbours.rbegin(); it2 != neighbours.rend(); ++it2) {
                    stack.push_back(*it2);
                }
            }
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    Generator<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::IterData> IniSectionGraph<K, V, KeyHash, KeyEqual>::iterByContentPart(
            int states, bool colour, std::optional<std::unordered_set<K, KeyHash, KeyEqual>> colourKeys) const {
        return iterSectsByContentPart(sections_, roots_, runConfig_, states, colour, std::move(colourKeys));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    Generator<typename IniSectionGraph<K, V, KeyHash, KeyEqual>::IterQueryData> IniSectionGraph<K, V, KeyHash, KeyEqual>::iterByQuery(
            std::vector<Z3Predicate> queryPathArg, bool simplify, int userStates, bool colour, std::optional<std::unordered_set<K, KeyHash, KeyEqual>> colourKeys) const {
        using PartVariant = std::variant<ContentPart*, NodeType*>;

        struct QStackFrame {
            int state;
            std::string sectionName;
            Section* section;
            NodeType* currentNode;
            PartVariant part;
            int depth;
            std::string rootSectionName;
            Section* rootSection;
        };
        struct NodeId {
            std::string sectionName;
            NodeType* node;
            bool operator==(const NodeId& other) const { return sectionName == other.sectionName && node == other.node; }
        };
        struct NodeIdHash {
            size_t operator()(const NodeId& n) const {
                return std::hash<std::string>()(n.sectionName) ^ (std::hash<const void*>()(static_cast<const void*>(n.node)) * 1000003u + 7u);
            }
        };

        bool colourKeysTruthy = colourKeys.has_value() && !colourKeys->empty();
        int states = userStates;
        if (colourKeysTruthy && states % 2 == 1) {
            states += 1;
        }

        const int exploreState = 0;
        const int cleanState = 1;

        for (int addState = 1; addState <= states; addState += 2) {
            std::vector<Z3Predicate> queryPath = queryPathArg;
            std::vector<int> queryCount{0};
            std::deque<QStackFrame> stack;
            std::unordered_set<std::string> visitedSections;
            const int startDepth = 0;

            Colouring colouring;
            std::unordered_map<NodeId, std::vector<ColourChangeSet>, NodeIdHash> colourChanges;

            for (const std::string& sectionName : roots_) {
                Section* section = getSection(sectionName);
                NodeType* rootNode = section->tree()->root();

                if (rootNode != nullptr) {
                    stack.push_front(QStackFrame{exploreState, sectionName, section, rootNode, PartVariant(rootNode), startDepth, sectionName, section});
                }
            }

            while (!stack.empty()) {
                QStackFrame frame = std::move(stack.back());
                stack.pop_back();

                NodeId nodeId{frame.sectionName, frame.currentNode};
                visitedSections.insert(frame.sectionName);

                if (frame.state == cleanState) {
                    if (std::holds_alternative<ContentPart*>(frame.part)) {
                        int clearState = addState + 1;
                        if (cleanState <= states && clearState <= userStates) {
                            Z3Predicate query = getQuery(queryPath, simplify);
                            co_yield IterQueryData(std::get<ContentPart*>(frame.part), query, frame.sectionName, frame.section,
                                                    frame.rootSectionName, frame.rootSection, addState + 1, &colouring);
                        }
                        continue;
                    }

                    bool isLastChild = stack.empty();
                    bool isEndIf = false;

                    if (!stack.empty()) {
                        QStackFrame& next = stack.back();
                        isLastChild = next.depth < frame.depth;

                        if (isLastChild) {
                            isEndIf = true;
                        } else {
                            bool nextIsContent = std::holds_alternative<ContentPart*>(next.part);
                            bool predNoneOrIf = false;
                            if (!nextIsContent) {
                                NodeType* nextNode = std::get<NodeType*>(next.part);
                                predNoneOrIf = (nextNode->ifPredPart == nullptr) || (nextNode->ifPredPart->type == IfPredPartType::If);
                            }
                            isEndIf = nextIsContent || predNoneOrIf;
                        }
                    }

                    if (isLastChild || isEndIf) {
                        int childrenQueryCount = queryCount.at(static_cast<size_t>(frame.depth));
                        for (int i = 0; i < childrenQueryCount; ++i) {
                            queryPath.pop_back();
                        }
                        queryCount[static_cast<size_t>(frame.depth)] = 0;
                    } else {
                        queryPath.back() = !queryPath.back();
                    }

                    if (isLastChild) {
                        queryCount.pop_back();
                    }

                    if (colour) {
                        auto ccIt = colourChanges.find(nodeId);
                        if (ccIt != colourChanges.end()) {
                            for (auto it2 = ccIt->second.rbegin(); it2 != ccIt->second.rend(); ++it2) {
                                colouring.restore(*it2);
                            }
                            colourChanges.erase(ccIt);
                        }
                    }

                    continue;
                }

                if (std::holds_alternative<ContentPart*>(frame.part)) {
                    ContentPart* contentPart = std::get<ContentPart*>(frame.part);
                    Z3Predicate query = getQuery(queryPath, simplify);

                    if (colour) {
                        ColourChangeSet newColourChange = colouring.updateColouring(*contentPart, colourKeys);
                        colourChanges[nodeId].push_back(std::move(newColourChange));
                    }

                    co_yield IterQueryData(contentPart, query, frame.sectionName, frame.section, frame.rootSectionName, frame.rootSection, addState, &colouring);

                    stack.push_back(QStackFrame{cleanState, frame.sectionName, frame.section, frame.currentNode, frame.part, frame.depth, frame.rootSectionName, frame.rootSection});

                    std::vector<std::string> childSectionNames;
                    if (contentPart->contains(runConfig_.runKey)) {
                        for (const V& v : contentPart->getVals(runConfig_.runKey)) {
                            childSectionNames.push_back(runConfig_.sectionNameOf(v));
                        }
                    }

                    for (auto it2 = childSectionNames.rbegin(); it2 != childSectionNames.rend(); ++it2) {
                        const std::string& childSectionName = *it2;
                        if (sections_.find(childSectionName) == sections_.end() || visitedSections.count(childSectionName)) {
                            continue;
                        }

                        Section* childSection = sections_.at(childSectionName);
                        NodeType* childRoot = childSection->tree()->root();
                        if (childRoot == nullptr) {
                            continue;
                        }

                        stack.push_back(QStackFrame{exploreState, childSectionName, childSection, childRoot, PartVariant(childRoot),
                                                     frame.depth + 1, frame.rootSectionName, frame.rootSection});
                    }

                    continue;
                }

                NodeType* nodeAsPart = std::get<NodeType*>(frame.part);
                IfPredPart* ifPredPart = nodeAsPart->ifPredPart;

                if (ifPredPart != nullptr && ifPredPart->query.has_value()) {
                    stack.push_back(QStackFrame{cleanState, frame.sectionName, frame.section, nodeAsPart, PartVariant(nodeAsPart),
                                                 frame.depth, frame.rootSectionName, frame.rootSection});
                    queryPath.push_back(*ifPredPart->query);
                    queryCount[static_cast<size_t>(frame.depth)] += 1;
                }

                const auto& children = nodeAsPart->parts();
                for (auto it2 = children.rbegin(); it2 != children.rend(); ++it2) {
                    stack.push_back(QStackFrame{exploreState, frame.sectionName, frame.section, nodeAsPart, *it2,
                                                 frame.depth + 1, frame.rootSectionName, frame.rootSection});
                }

                queryCount.push_back(0);
            }
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string IniSectionGraph<K, V, KeyHash, KeyEqual>::toStr(const std::function<std::string(Section&, const std::string&, bool)>& sectionToStr, bool autoindent) const {
        std::vector<std::string> result;
        std::deque<std::string> stack;
        std::unordered_set<std::string> visited;

        for (const auto& root : roots_) {
            stack.push_front(root);
        }

        while (!stack.empty()) {
            std::string sectionName = stack.back();
            stack.pop_back();

            Section* section = getSection(sectionName);
            visited.insert(sectionName);

            result.push_back(sectionToStr(*section, "", autoindent));

            auto neighboursIt = neighbours_.find(sectionName);
            if (neighboursIt != neighbours_.end()) {
                for (auto it = neighboursIt->second.rbegin(); it != neighboursIt->second.rend(); ++it) {
                    stack.push_back(*it);
                }
            }
        }

        std::string joined;
        for (size_t i = 0; i < result.size(); ++i) {
            if (i > 0) {
                joined += "\n\n";
            }
            joined += result[i];
        }
        return joined;
    }

}
