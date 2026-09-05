#ifndef AGRemapCore_ResEdit_TPP
#define AGRemapCore_ResEdit_TPP

#include <memory>
#include <unordered_set>
#include <utility>

#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/tools/hashing/HashTools.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    BaseResEdit<K, V, KeyHash, KeyEqual>::BaseResEdit(std::string resType, GraphId resModObj, ResEditConfig config,
                                                       IniGraphReplaceMode graphReplaceMode):
        resType(std::move(resType)), config(std::move(config)), resModObj(std::move(resModObj)),
        graphReplaceMode(graphReplaceMode) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BaseResEdit<K, V, KeyHash, KeyEqual>::clear() {
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string BaseResEdit<K, V, KeyHash, KeyEqual>::getFileId(const GraphId& modObj, const std::string& sectionName,
                                                                 std::size_t partId, long long orderInd, const std::string& file) const {
        // Every component is length-prefixed so no two different tuples can serialize to the same
        // bytes (a plain separator character could collide with one appearing inside a section name
        // or file path). See this method's own note on why byte-compatibility with the pure-Python
        // original isn't required.
        auto append = [](std::string& out, const std::string& part) {
            out += std::to_string(part.size());
            out += ':';
            out += part;
        };

        std::string data;
        append(data, std::to_string(modObj.iniIndex));
        append(data, modObj.modObj.first);
        append(data, modObj.modObj.second);
        append(data, sectionName);
        append(data, std::to_string(partId));
        append(data, std::to_string(orderInd));
        append(data, file);

        return HashTools::getDeterministicHashStr(data);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<std::string, std::string> BaseResEdit<K, V, KeyHash, KeyEqual>::collectResourceName(const std::string& oldResourceName,
                                                                                                   const std::string& newResourceName) const {
        return {oldResourceName, newResourceName};
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> BaseResEdit<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                         const std::string& modName) const {
        return IniNamingTools::getRemapFixName(resource, modName);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string BaseResEdit<K, V, KeyHash, KeyEqual>::fileAddGraphId(const std::string& file, const std::string& graphId) {
        // Python's file.rsplit(".", 1): with no dot at all, the "extension" half simply doesn't
        // exist and the id is appended to the whole name.
        std::size_t dot = file.rfind('.');
        if (dot == std::string::npos) {
            return file + "_" + graphId;
        }

        return file.substr(0, dot) + "_" + graphId + file.substr(dot);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string BaseResEdit<K, V, KeyHash, KeyEqual>::getFixFile(const std::string& file, const std::string& modName,
                                                                  const std::string& graphId) const {
        (void)graphId;
        return IniNamingTools::getFixedFile(file, modName);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BaseResEdit<K, V, KeyHash, KeyEqual>::buildResModel(const std::string& resType, const std::string& srcPath,
                                                              const std::string& fixedPath, const std::string& modName,
                                                              const std::string& fileKey, Context& ctx) {
        // 'fixedPath'/'modName' are deliberately unused: a plain IniResource carries only the
        // source path. ResReplace's own override is the one that uses both.
        (void)fixedPath;
        (void)modName;
        // The view is attached at registration, which is the one moment both the resource and the
        // .ini file it belongs to are in scope -- see IniResource::logger for why it is an
        // attribute rather than an argument to the fix.
        std::unique_ptr<IniResource> resource = std::make_unique<IniResource>(resType, ctx.iniFolder(), srcPath);
        resource->logger = ctx.logger();

        ctx.storeResource(fileKey, std::move(resource));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BaseResEdit<K, V, KeyHash, KeyEqual>::buildResModels(Graph& graph, Context& ctx, const std::string& modName,
                                                               const ResourceFilter& resourceFilter, const std::string& graphId,
                                                               const GraphId* resModObj) {
        (void)modName;

        std::string shortGraphHash = graphId.empty() ? std::string() : HashTools::getShortDeterministicHashStr(graphId);
        const GraphId& currentResModObj = (resModObj == nullptr) ? this->resModObj : *resModObj;

        auto parts = graph.iterByContentPart();
        while (parts.next()) {
            auto& iterData = parts.value();
            const std::string& sectionName = iterData.sectionName;
            ContentPart* part = iterData.part;

            std::vector<std::pair<long long, V>> regVals = part->getValsWithInds(config.filenameKey);
            std::unordered_set<long long> indsToRemove;

            for (const auto& regVal : regVals) {
                long long ind = regVal.first;
                std::string val = config.fileOf(regVal.second);
                std::string fileKey = getFileId(currentResModObj, sectionName, part->id(), ind, val);

                if (resourceFilter && !resourceFilter(val, fileKey)) {
                    indsToRemove.insert(ind);
                    continue;
                }

                std::string newVal = val;
                if (!graphId.empty()) {
                    newVal = fileAddGraphId(val, shortGraphHash);
                    part->setValByInd(ind, config.valOfFile(newVal));
                }

                buildResModel(resType, newVal, "", modName, fileKey, ctx);
            }

            if (!indsToRemove.empty()) {
                part->removeKey(config.filenameKey, std::nullopt,
                                 typename ContentPart::RemoveKeyCheck([&indsToRemove](long long ind, const V&) {
                                     return indsToRemove.count(ind) > 0;
                                 }));
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string BaseResEdit<K, V, KeyHash, KeyEqual>::renameUncollectedSection(const std::string& sectionName,
                                                                                const std::string& modName) const {
        std::optional<std::string> result = getFixResourceName(sectionName, modName);
        if (!result.has_value()) {
            return sectionName;
        }

        return *result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseResEdit<K, V, KeyHash, KeyEqual>::Graph* BaseResEdit<K, V, KeyHash, KeyEqual>::getResGraph(const CollectedSections& collectedSections,
                                                                                                            Context& ctx, GraphGroups& graphGroups,
                                                                                                            const std::string& modName, bool rename,
                                                                                                            bool copySections) {
        if (resModObj.iniIndex >= graphGroups.size()) {
            return nullptr;
        }

        Graph* graph = graphGroups.getGraph(resModObj.iniIndex, resModObj.modObj);

        auto renameCollected = [&collectedSections](const std::string& sectionName) {
            auto it = collectedSections.find(sectionName);
            return (it == collectedSections.end()) ? sectionName : it->second;
        };

        if (graph != nullptr && graphReplaceMode == IniGraphReplaceMode::Ignore) {
            if (rename) {
                graph->rename(renameCollected);
            }

            return graph;
        }

        std::vector<std::string> targetSectionNames;
        targetSectionNames.reserve(collectedSections.size());
        for (const auto& entry : collectedSections) {
            targetSectionNames.push_back(entry.first);
        }

        if (graph == nullptr || graphReplaceMode == IniGraphReplaceMode::Replace) {
            graph = graphGroups.createGraph(ctx.sectionIfTemplates(), targetSectionNames, copySections, ctx.z3Ctx());

            if (rename && graph != nullptr) {
                graph->rename([this, &collectedSections, &modName](const std::string& sectionName) {
                    auto it = collectedSections.find(sectionName);
                    return (it == collectedSections.end()) ? renameUncollectedSection(sectionName, modName) : it->second;
                });
            }

            return graph;
        }

        if (graphReplaceMode == IniGraphReplaceMode::Combine) {
            std::vector<std::string> newTargets = graph->targetSectionNames();
            newTargets.insert(newTargets.end(), targetSectionNames.begin(), targetSectionNames.end());
            graph->setTargetSectionNames(std::move(newTargets));

            // DictTools.update(graph.sections, ini.sectionIfTemplates) -- the .ini file's own
            // sections win over whatever the graph already had under the same name.
            std::unordered_map<std::string, Section*> sections = graph->sections();
            for (const auto& entry : ctx.sectionIfTemplates()) {
                sections[entry.first] = entry.second;
            }

            graph->build(std::move(sections), std::nullopt, copySections);

            if (rename) {
                graph->rename(renameCollected);
            }
        }

        return graph;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseResEdit<K, V, KeyHash, KeyEqual>::GraphGroups& BaseResEdit<K, V, KeyHash, KeyEqual>::buildResources(const CollectedSections& collectedSections,
                                                                                                                     Context& ctx, GraphGroups& graphGroups,
                                                                                                                     const std::string& modName,
                                                                                                                     const ResourceFilter& resourceFilter,
                                                                                                                     bool copySections) {
        Graph* graph = getResGraph(collectedSections, ctx, graphGroups, modName, true, copySections);
        if (graph == nullptr) {
            return graphGroups;
        }

        if (resModObj.iniIndex >= graphGroups.size()) {
            return graphGroups;
        }

        graphGroups.addGraph(resModObj.iniIndex, resModObj.modObj, graph);
        buildResModels(*graph, ctx, modName, resourceFilter);
        return graphGroups;
    }


    // ---------------------------------------------------------------------------------------
    // ResIdentity
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    ResIdentity<K, V, KeyHash, KeyEqual>::ResIdentity(GraphId resModObj, typename Base::ResEditConfig config, bool createResModel):
        Base("", std::move(resModObj), std::move(config)), createResModel(createResModel) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> ResIdentity<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                         const std::string& modName) const {
        (void)resource;
        (void)modName;
        return std::nullopt;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResIdentity<K, V, KeyHash, KeyEqual>::buildResModels(Graph& graph, Context& ctx, const std::string& modName,
                                                               const ResourceFilter& resourceFilter, const std::string& graphId,
                                                               const GraphId* resModObj) {
        if (!createResModel) {
            return;
        }

        Base::buildResModels(graph, ctx, modName, resourceFilter, graphId, resModObj);
    }


    // ---------------------------------------------------------------------------------------
    // ResReplace
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResReplace<K, V, KeyHash, KeyEqual>::buildResModels(Graph& graph, Context& ctx, const std::string& modName,
                                                              const ResourceFilter& resourceFilter, const std::string& graphId,
                                                              const GraphId* resModObj) {
        std::string shortGraphHash = graphId.empty() ? std::string() : HashTools::getShortDeterministicHashStr(graphId);
        const GraphId& currentResModObj = (resModObj == nullptr) ? this->resModObj : *resModObj;

        auto parts = graph.iterByContentPart();
        while (parts.next()) {
            auto& iterData = parts.value();
            const std::string& sectionName = iterData.sectionName;
            auto* part = iterData.part;

            std::vector<std::pair<long long, V>> regVals = part->getValsWithInds(this->config.filenameKey);
            std::unordered_set<long long> indsToRemove;

            for (const auto& regVal : regVals) {
                long long ind = regVal.first;
                std::string val = this->config.fileOf(regVal.second);
                std::string fileKey = this->getFileId(currentResModObj, sectionName, part->id(), ind, val);

                if (resourceFilter && !resourceFilter(val, fileKey)) {
                    indsToRemove.insert(ind);
                    continue;
                }

                // Unlike the base implementation, the value is rewritten to the *fixed* path (with
                // the graph id folded in), and the model is built from both paths.
                std::string newVal = this->getFixFile(val, modName, shortGraphHash);
                part->setValByInd(ind, this->config.valOfFile(newVal));
                this->buildResModel(this->resType, val, newVal, modName, fileKey, ctx);
            }

            if (!indsToRemove.empty()) {
                part->removeKey(this->config.filenameKey, std::nullopt,
                                 typename Base::ContentPart::RemoveKeyCheck([&indsToRemove](long long ind, const V&) {
                                     return indsToRemove.count(ind) > 0;
                                 }));
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResReplace<K, V, KeyHash, KeyEqual>::buildResModel(const std::string& resType, const std::string& srcPath,
                                                             const std::string& fixedPath, const std::string& modName,
                                                             const std::string& fileKey, Context& ctx) {
        (void)modName;
        // See this file's other storeResource call for why the view is attached here.
        std::unique_ptr<IniFixResource> resource =
            std::make_unique<IniFixResource>(resType, ctx.iniFolder(), srcPath, fixedPath);
        resource->logger = ctx.logger();

        ctx.storeResource(fileKey, std::move(resource));
    }


    // ---------------------------------------------------------------------------------------
    // ResCreate
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<std::string, std::string> ResCreate<K, V, KeyHash, KeyEqual>::collectResourceName(const std::string& oldResourceName,
                                                                                                 const std::string& newResourceName) const {
        (void)oldResourceName;
        return {newResourceName, newResourceName};
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResCreate<K, V, KeyHash, KeyEqual>::Graph* ResCreate<K, V, KeyHash, KeyEqual>::getResGraph(const CollectedSections& collectedSections,
                                                                                                        Context& ctx, GraphGroups& graphGroups,
                                                                                                        const std::string& modName, bool rename,
                                                                                                        bool copySections) {
        // 'rename' is deliberately unused: a created graph's sections are built directly under
        // their fixed names, so there is nothing left to rename -- exactly as in the pure-Python
        // original, whose ResCreate.getResGraph also ignores it.
        (void)rename;

        if (this->resModObj.iniIndex >= graphGroups.size()) {
            return nullptr;
        }

        Graph* graph = graphGroups.getGraph(this->resModObj.iniIndex, this->resModObj.modObj);
        bool graphExists = graph != nullptr;

        if (graphExists && this->graphReplaceMode == IniGraphReplaceMode::Ignore) {
            return graph;
        }

        std::unordered_map<std::string, Section*> sections;
        std::vector<std::string> targetSectionNames;
        targetSectionNames.reserve(collectedSections.size());

        for (const auto& entry : collectedSections) {
            const std::string& newSectionName = entry.second;
            targetSectionNames.push_back(newSectionName);

            if (sections.count(newSectionName) > 0) {
                continue;
            }

            Section* section = buildSection(newSectionName, modName);
            if (section != nullptr) {
                sections[newSectionName] = section;
            }
        }

        if (graphExists && this->graphReplaceMode == IniGraphReplaceMode::Combine) {
            std::unordered_map<std::string, Section*> combined = graph->sections();
            for (const auto& entry : sections) {
                combined[entry.first] = entry.second;
            }

            std::vector<std::string> newTargets = graph->targetSectionNames();
            newTargets.insert(newTargets.end(), targetSectionNames.begin(), targetSectionNames.end());
            graph->setTargetSectionNames(std::move(newTargets));
            graph->build(std::move(combined), std::nullopt, copySections);

            return graph;
        }

        if (!graphExists || this->graphReplaceMode == IniGraphReplaceMode::Replace) {
            graph = graphGroups.createGraph(std::move(sections), std::move(targetSectionNames), copySections, ctx.z3Ctx());
        }

        return graph;
    }
}

#endif
