#include <algorithm>
#include <tuple>

namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IfTemplate<K, V, KeyHash, KeyEqual>::IfTemplate(std::vector<std::unique_ptr<IfTemplatePart>> parts, IfTemplateRunConfig<K, V> runConfig,
                                                     std::string name, TreeKind treeKind, std::string prefix, std::string suffix):
        name(std::move(name)), prefix(std::move(prefix)), suffix(std::move(suffix)),
        parts_(std::move(parts)), treeKind_(treeKind), runConfig_(std::move(runConfig)) {
        rebuild();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<IfTemplate<K, V, KeyHash, KeyEqual>> IfTemplate<K, V, KeyHash, KeyEqual>::build(
            const std::vector<std::pair<int, std::variant<std::string, tsl::ordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>>>>& rawParts,
            IfTemplateRunConfig<K, V> runConfig, std::string name, ParseContext* ctx, Z3Context* z3Ctx) {
        ParseContext ownedCtx;
        Z3Context ownedZ3Ctx;
        if (ctx == nullptr) {
            ctx = &ownedCtx;
        }
        if (z3Ctx == nullptr) {
            z3Ctx = &ownedZ3Ctx;
        }

        std::vector<std::unique_ptr<IfTemplatePart>> parts;
        int depth = 0;

        for (const auto& rawPartEntry : rawParts) {
            int lineNo = rawPartEntry.first;
            const auto& rawPart = rawPartEntry.second;

            if (const std::string* rawPredStr = std::get_if<std::string>(&rawPart)) {
                std::optional<IfPredPartType> predType = IfPredPartTypeTools::getType(*rawPredStr);
                if (!predType.has_value()) {
                    continue;
                }

                if (*predType == IfPredPartType::If) {
                    ++depth;
                } else if (*predType == IfPredPartType::EndIf) {
                    --depth;
                }

                ctx->startLineNo = lineNo;
                parts.push_back(std::make_unique<IfPredPart>(*rawPredStr, *predType, *z3Ctx, ctx));
                continue;
            }

            const auto& src = std::get<tsl::ordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>>(rawPart);
            parts.push_back(std::make_unique<ContentPart>(src, depth));
        }

        return std::make_unique<IfTemplate<K, V, KeyHash, KeyEqual>>(std::move(parts), std::move(runConfig), std::move(name));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::unique_ptr<IfTemplatePart>>& IfTemplate<K, V, KeyHash, KeyEqual>::parts() {
        return parts_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<std::unique_ptr<IfTemplatePart>>& IfTemplate<K, V, KeyHash, KeyEqual>::parts() const {
        return parts_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::unordered_map<size_t, IfTemplatePart*>& IfTemplate<K, V, KeyHash, KeyEqual>::partsById() const {
        return partsById_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::unordered_map<size_t, std::vector<std::string>>& IfTemplate<K, V, KeyHash, KeyEqual>::calledSubCommands() const {
        return calledSubCommands_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const IfTemplateRunConfig<K, V>& IfTemplate<K, V, KeyHash, KeyEqual>::runConfig() const {
        return runConfig_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IfTemplate<K, V, KeyHash, KeyEqual>::TreeType* IfTemplate<K, V, KeyHash, KeyEqual>::tree() const {
        return tree_.get();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<size_t, IfTemplatePart*> IfTemplate<K, V, KeyHash, KeyEqual>::setupPartsById() const {
        std::unordered_map<size_t, IfTemplatePart*> result;
        for (const auto& part : parts_) {
            result[part->id()] = part.get();
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplate<K, V, KeyHash, KeyEqual>::rebuildTree() {
        switch (treeKind_) {
            case TreeKind::Basic:
                tree_ = TreeType::construct(parts_);
                break;
            case TreeKind::Norm:
                tree_ = IfTemplateNormTree<K, V, KeyHash, KeyEqual>::construct(parts_);
                break;
            case TreeKind::NonEmptyNode:
            default:
                tree_ = IfTemplateNonEmptyNodeTree<K, V, KeyHash, KeyEqual>::construct(parts_);
                break;
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplate<K, V, KeyHash, KeyEqual>::rebuild() {
        partsById_ = setupPartsById();
        rebuildTree();

        calledSubCommands_.clear();
        for (size_t i = 0; i < parts_.size(); ++i) {
            auto* contentPart = dynamic_cast<ContentPart*>(parts_[i].get());
            if (contentPart == nullptr || !contentPart->contains(runConfig_.runKey)) {
                continue;
            }

            std::vector<V> runVals = contentPart->getVals(runConfig_.runKey);
            std::vector<std::string> targets;
            targets.reserve(runVals.size());
            for (const V& val : runVals) {
                targets.push_back(runConfig_.sectionNameOf(val));
            }
            calledSubCommands_[i] = std::move(targets);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplate<K, V, KeyHash, KeyEqual>::refreshCalledSubCommand(size_t partInd) {
        if (calledSubCommands_.find(partInd) == calledSubCommands_.end() || partInd >= parts_.size()) {
            return;
        }

        auto* contentPart = dynamic_cast<ContentPart*>(parts_[partInd].get());
        if (contentPart == nullptr) {
            return;
        }

        std::vector<V> runVals = contentPart->getVals(runConfig_.runKey);
        std::vector<std::string> targets;
        targets.reserve(runVals.size());
        for (const V& val : runVals) {
            targets.push_back(runConfig_.sectionNameOf(val));
        }
        calledSubCommands_[partInd] = std::move(targets);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplate<K, V, KeyHash, KeyEqual>::refreshPartIds() {
        for (auto& part : parts_) {
            part->refreshId();
        }
        partsById_ = setupPartsById();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<IfTemplate<K, V, KeyHash, KeyEqual>> IfTemplate<K, V, KeyHash, KeyEqual>::deepcopy(bool newPartIds) const {
        std::vector<std::unique_ptr<IfTemplatePart>> clonedParts;
        clonedParts.reserve(parts_.size());

        for (const auto& part : parts_) {
            if (auto* contentPart = dynamic_cast<ContentPart*>(part.get())) {
                clonedParts.push_back(contentPart->clone(false));
            } else {
                auto* predPart = dynamic_cast<IfPredPart*>(part.get());
                clonedParts.push_back(std::make_unique<IfPredPart>(predPart->clone(false)));
            }
        }

        // Rebuilding fresh (rather than literally deep-copying #tree_ node-for-node) gives an
        // identical result, since the tree is a pure function of (parts_, treeKind_) -- see this
        // class's own #rebuildTree.
        auto result = std::make_unique<IfTemplate<K, V, KeyHash, KeyEqual>>(
            std::move(clonedParts), runConfig_, name, treeKind_, prefix, suffix);

        if (newPartIds) {
            result->refreshPartIds();
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplate<K, V, KeyHash, KeyEqual>::normalize() {
        // Deliberately bypasses treeKind_/rebuildTree() -- matches the pure-Python original's own
        // 'self.tree = IfTemplateNormTree.construct(self.parts)', a one-shot reassignment that
        // does NOT change what a future rebuild() call would use.
        tree_ = IfTemplateNormTree<K, V, KeyHash, KeyEqual>::construct(parts_);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IfTemplatePart* IfTemplate<K, V, KeyHash, KeyEqual>::add(std::unique_ptr<IfTemplatePart> part, bool updateTree) {
        IfTemplatePart* result = part.get();
        parts_.push_back(std::move(part));
        partsById_[result->id()] = result;

        if (updateTree) {
            // Deliberately the base algorithm always, matching the pure-Python original's own
            // hardcoded 'IfTemplateTree.construct(self.parts)' here (not 'self.treeCls.construct').
            tree_ = TreeType::construct(parts_);
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    template <typename Result>
    std::unordered_map<size_t, Result> IfTemplate<K, V, KeyHash, KeyEqual>::find(
            std::function<bool(IfTemplate<K, V, KeyHash, KeyEqual>&, size_t, IfTemplatePart&)> pred,
            std::function<Result(IfTemplate<K, V, KeyHash, KeyEqual>&, size_t, IfTemplatePart&)> postProcessor) const {
        std::unordered_map<size_t, Result> result;

        for (size_t i = 0; i < parts_.size(); ++i) {
            IfTemplatePart& part = *parts_[i];
            if (!pred || pred(const_cast<IfTemplate<K, V, KeyHash, KeyEqual>&>(*this), i, part)) {
                result[i] = postProcessor(const_cast<IfTemplate<K, V, KeyHash, KeyEqual>&>(*this), i, part);
            }
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IfTemplate<K, V, KeyHash, KeyEqual>::ContentPart* IfTemplate<K, V, KeyHash, KeyEqual>::addTopContentPart() {
        ContentPart* result = nullptr;

        if (parts_.empty() || dynamic_cast<ContentPart*>(parts_[0].get()) == nullptr) {
            auto owned = std::make_unique<ContentPart>(0);
            result = owned.get();
            parts_.insert(parts_.begin(), std::move(owned));
            partsById_[result->id()] = result;
        } else {
            result = static_cast<ContentPart*>(parts_[0].get());
        }

        if (tree_->root() == nullptr) {
            tree_->setRoot(tree_->makeNode(nullptr));
        }

        auto& rootParts = tree_->root()->parts();
        bool rootStartsWithContent = !rootParts.empty() && std::holds_alternative<ContentPart*>(rootParts.front())
            && std::get<ContentPart*>(rootParts.front()) != nullptr;
        if (!rootStartsWithContent) {
            rootParts.insert(rootParts.begin(), typename NodeType::PartsElement(result));
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplate<K, V, KeyHash, KeyEqual>::addKVPsToFront(const std::vector<std::pair<K, V>>& kvps) {
        ContentPart* part = addTopContentPart();
        part->addKVPsToFront(kvps);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplate<K, V, KeyHash, KeyEqual>::addKVPToFront(const K& key, const V& val) {
        addKVPsToFront({std::make_pair(key, val)});
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IfTemplate<K, V, KeyHash, KeyEqual>::ContentPart* IfTemplate<K, V, KeyHash, KeyEqual>::addBottomContentPart() {
        ContentPart* result = nullptr;

        if (parts_.empty() || dynamic_cast<ContentPart*>(parts_.back().get()) == nullptr) {
            auto owned = std::make_unique<ContentPart>(0);
            result = owned.get();
            parts_.push_back(std::move(owned));
            partsById_[result->id()] = result;
        } else {
            result = static_cast<ContentPart*>(parts_.back().get());
        }

        if (tree_->root() == nullptr) {
            tree_->setRoot(tree_->makeNode(nullptr));
        }

        auto& rootParts = tree_->root()->parts();
        bool rootEndsWithContent = !rootParts.empty() && std::holds_alternative<ContentPart*>(rootParts.back())
            && std::get<ContentPart*>(rootParts.back()) != nullptr;
        if (!rootEndsWithContent) {
            rootParts.push_back(typename NodeType::PartsElement(result));
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplate<K, V, KeyHash, KeyEqual>::addKVPsToBack(const std::vector<std::pair<K, V>>& kvps) {
        ContentPart* part = addBottomContentPart();
        part->addKVPs(kvps);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfTemplate<K, V, KeyHash, KeyEqual>::addKVPToBack(const K& key, const V& val) {
        addKVPsToBack({std::make_pair(key, val)});
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool IfTemplate<K, V, KeyHash, KeyEqual>::isKeyFullyCoverNode(
            NodeType& node, const K& key,
            const std::unordered_map<std::string, IfTemplate<K, V, KeyHash, KeyEqual>*>& sections,
            std::unordered_set<std::string>& visited, std::unordered_map<std::string, bool>& sectionsKeyFullCover) const {
        bool result = node.hasKey(key);
        if (result) {
            return result;
        }

        bool childrenResult = true;
        const auto& branchChildren = node.children();

        std::vector<std::vector<std::pair<long long, V>>> runValues = node.getKeyValues(runConfig_.runKey);
        std::unordered_set<std::string> subCommandsToCheck;
        std::unordered_set<std::string> subCommandsChecked;

        for (const auto& partValues : runValues) {
            for (const auto& valueData : partValues) {
                std::string subCommand = runConfig_.sectionNameOf(valueData.second);
                if (visited.find(subCommand) == visited.end()) {
                    subCommandsToCheck.insert(subCommand);
                } else if (sectionsKeyFullCover.find(subCommand) != sectionsKeyFullCover.end()) {
                    subCommandsChecked.insert(subCommand);
                }
            }
        }

        if (branchChildren.empty() && subCommandsToCheck.empty() && subCommandsChecked.empty()) {
            return result;
        }

        for (const std::string& subCommand : subCommandsChecked) {
            childrenResult = childrenResult && sectionsKeyFullCover.at(subCommand);
            if (!childrenResult) {
                return result;
            }
        }

        for (const auto& childEntry : branchChildren) {
            NodeType* child = childEntry.second;
            childrenResult = childrenResult && isKeyFullyCoverNode(*child, key, sections, visited, sectionsKeyFullCover);
            if (!childrenResult) {
                return result;
            }
        }

        for (const std::string& subCommand : subCommandsToCheck) {
            // Assume the .ini file has correct syntax and does not reference some command that
            // doesn't exist -- same assumption the pure-Python original makes.
            auto it = sections.find(subCommand);
            if (it == sections.end()) {
                continue;
            }

            childrenResult = childrenResult && it->second->isKeyFullyCover(key, sections, visited, sectionsKeyFullCover);
            if (!childrenResult) {
                return result;
            }
        }

        result = result || childrenResult;
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool IfTemplate<K, V, KeyHash, KeyEqual>::isKeyFullyCover(
            const K& key, const std::unordered_map<std::string, IfTemplate<K, V, KeyHash, KeyEqual>*>& sections,
            std::unordered_set<std::string>& visited, std::unordered_map<std::string, bool>& sectionsKeyFullCover) const {
        visited.insert(name);

        NodeType* node = tree_->root();
        bool result = isKeyFullyCoverNode(*node, key, sections, visited, sectionsKeyFullCover);
        sectionsKeyFullCover[name] = result;
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<std::set<typename IfTemplate<K, V, KeyHash, KeyEqual>::ContentPart*>, bool> IfTemplate<K, V, KeyHash, KeyEqual>::getKeyMissingPartsNode(
            NodeType& node, const K& key,
            const std::unordered_map<std::string, IfTemplate<K, V, KeyHash, KeyEqual>*>& sections,
            std::unordered_set<std::string>& visited,
            std::unordered_map<std::string, std::set<ContentPart*>>& sectionsMissingParts,
            std::unordered_map<std::string, bool>& sectionAllBranchesMissing) const {
        auto [nodeMissingPart, hasContentPart] = node.getKeyMissingPart(key);
        if (hasContentPart && nodeMissingPart == nullptr) {
            return std::make_pair(std::set<ContentPart*>(), false);
        }

        std::set<ContentPart*> result;
        if (nodeMissingPart != nullptr) {
            result.insert(nodeMissingPart);
        }

        std::set<ContentPart*> childrenResult;
        const auto& branchChildren = node.children();

        std::vector<std::vector<std::pair<long long, V>>> runValues = node.getKeyValues(runConfig_.runKey);
        std::unordered_set<std::string> subCommandsToCheck;
        std::unordered_set<std::string> subCommandsChecked;

        for (const auto& partValues : runValues) {
            for (const auto& valueData : partValues) {
                std::string subCommand = runConfig_.sectionNameOf(valueData.second);
                if (visited.find(subCommand) == visited.end()) {
                    subCommandsToCheck.insert(subCommand);
                } else if (sectionsMissingParts.find(subCommand) != sectionsMissingParts.end()) {
                    subCommandsChecked.insert(subCommand);
                }
            }
        }

        if (branchChildren.empty() && subCommandsToCheck.empty() && subCommandsChecked.empty()) {
            return std::make_pair(result, true);
        }

        size_t branchChildrenLen = branchChildren.size();
        size_t subCommandsToCheckLen = subCommandsToCheck.size();
        size_t subCommandsCheckedLen = subCommandsChecked.size();

        // Three separate counters, one per category -- mirroring 'branchChildrenLen'/
        // 'subCommandsToCheckLen'/'subCommandsCheckedLen' above, and (by the same relationship)
        // isKeyFullyCoverNode's own three loops just below in this same file. The pure-Python
        // original declared 'missingKeySubCommandsToCheck'/'missingKeySubCommandsChecked' but
        // every one of its three loops incremented only the first counter -- a real bug (not a
        // preserved-on-purpose quirk): since 'missingKeyChildrenTotal' is a plain sum of all
        // three, that alone is harmless (addition doesn't care which counter held which share).
        // The loop below for 'subCommandsToCheck' compounded it though -- it never refreshed
        // 'currentAllBranchesMissing' for the subCommand it had just recursed into, so it kept
        // reusing whatever the *previous* loop (branchChildren) last left that shared local at,
        // for every iteration -- which can make 'missingKeyChildrenTotal' land on the wrong side
        // of the '== childrenTotal' check below, returning the wrong shape of result (this node's
        // own missing part vs. the deeper aggregated set). Fixed here to look the freshly-computed
        // value up the same way the 'subCommandsChecked' loop already does for its own (cached)
        // entries.
        size_t missingKeyBranchChildren = 0;
        size_t missingKeySubCommandsToCheck = 0;
        size_t missingKeySubCommandsChecked = 0;

        std::set<ContentPart*> currentChildMissingKeys;
        bool currentAllBranchesMissing = false;

        for (const std::string& subCommand : subCommandsChecked) {
            currentChildMissingKeys = sectionsMissingParts.at(subCommand);
            auto abmIt = sectionAllBranchesMissing.find(subCommand);
            if (abmIt == sectionAllBranchesMissing.end()) {
                continue;
            }

            currentAllBranchesMissing = abmIt->second;
            if (!currentChildMissingKeys.empty()) {
                childrenResult.insert(currentChildMissingKeys.begin(), currentChildMissingKeys.end());
                if (currentAllBranchesMissing) {
                    ++missingKeySubCommandsChecked;
                }
            }
        }

        for (const auto& childEntry : branchChildren) {
            NodeType* child = childEntry.second;
            std::tie(currentChildMissingKeys, currentAllBranchesMissing) = getKeyMissingPartsNode(
                *child, key, sections, visited, sectionsMissingParts, sectionAllBranchesMissing);

            if (!currentChildMissingKeys.empty()) {
                childrenResult.insert(currentChildMissingKeys.begin(), currentChildMissingKeys.end());
                if (currentAllBranchesMissing) {
                    ++missingKeyBranchChildren;
                }
            }
        }

        for (const std::string& subCommand : subCommandsToCheck) {
            auto it = sections.find(subCommand);
            if (it == sections.end()) {
                continue;
            }

            // The public getKeyMissingParts only returns the set (not a tuple), but it stores the
            // subcommand's own "all branches missing" verdict into 'sectionAllBranchesMissing' as
            // a side effect -- look it back up from there, the same way the 'subCommandsChecked'
            // loop above reads an already-cached one, instead of reusing a stale value left over
            // from the previous loop.
            currentChildMissingKeys = it->second->getKeyMissingParts(key, sections, visited, sectionsMissingParts, sectionAllBranchesMissing);
            auto abmIt = sectionAllBranchesMissing.find(subCommand);
            currentAllBranchesMissing = (abmIt != sectionAllBranchesMissing.end()) && abmIt->second;

            if (!currentChildMissingKeys.empty()) {
                childrenResult.insert(currentChildMissingKeys.begin(), currentChildMissingKeys.end());
                if (currentAllBranchesMissing) {
                    ++missingKeySubCommandsToCheck;
                }
            }
        }

        size_t missingKeyChildrenTotal = missingKeyBranchChildren + missingKeySubCommandsToCheck + missingKeySubCommandsChecked;
        size_t childrenTotal = branchChildrenLen + subCommandsToCheckLen + subCommandsCheckedLen;

        if (!result.empty() && missingKeyChildrenTotal == childrenTotal) {
            return std::make_pair(result, true);
        }

        return std::make_pair(childrenResult, false);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::set<typename IfTemplate<K, V, KeyHash, KeyEqual>::ContentPart*> IfTemplate<K, V, KeyHash, KeyEqual>::getKeyMissingParts(
            const K& key, const std::unordered_map<std::string, IfTemplate<K, V, KeyHash, KeyEqual>*>& sections,
            std::unordered_set<std::string>& visited,
            std::unordered_map<std::string, std::set<ContentPart*>>& sectionsMissingParts,
            std::unordered_map<std::string, bool>& sectionAllBranchesMissing) const {
        visited.insert(name);

        NodeType* node = tree_->root();
        auto [result, allBranchesMissing] = getKeyMissingPartsNode(
            *node, key, sections, visited, sectionsMissingParts, sectionAllBranchesMissing);
        sectionsMissingParts[name] = result;
        sectionAllBranchesMissing[name] = allBranchesMissing;
        return result;
    }

}
