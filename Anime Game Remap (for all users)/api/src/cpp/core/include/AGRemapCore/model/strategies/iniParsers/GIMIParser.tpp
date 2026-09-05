#ifndef AGRemapCore_GIMIParser_TPP
#define AGRemapCore_GIMIParser_TPP

#include <algorithm>
#include <type_traits>

#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/TextTools.h"

#include "GIMIParser.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ParserConfig GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::defaultConfig() {
        ParserConfig result{};
        result.classifier = Classifier::defaultConfig();

        // Only a K/V that a .ini keyword or section name can literally be spelled as gets real
        // defaults -- see GIMISectionClassifier::defaultConfig's own note.
        if constexpr (std::is_constructible_v<K, const std::string&> && std::is_constructible_v<V, const std::string&>) {
            result.runConfig = IfTemplateRunConfig<K, V>{
                K(IniKeywords::Run),
                [](const V& val) { return std::string(val); },
                [](const std::string& name) { return V(name); }
            };
            result.valOfSectionName = [](const std::string& name) { return V(name); };
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::GIMIParser(Context* ctx, std::vector<ModObj> modObjs, std::vector<ObjTargetFunc> objTargetFuncs,
                                                     Downloads downloads, GroupEdit* commandEdits, bool makeGlobalGraph, bool disjointModObjs,
                                                     bool trackKeys, std::optional<std::unordered_set<K, KeyHash, KeyEqual>> keysToTrack,
                                                     ParserConfig config):
        Base(),
        objTargetFuncs(std::move(objTargetFuncs)), downloads(std::move(downloads)), makeGlobalGraph(makeGlobalGraph),
        disjointModObjs(disjointModObjs), trackKeys(trackKeys), keysToTrack(std::move(keysToTrack)),
        ctx_(ctx), commandEdits_(commandEdits), globalGraph_(nullptr), downloadsAdded_(false), config_(std::move(config)) {
        setModObjs(std::move(modObjs));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::Context* GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ctx() const {
        return ctx_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::setCtx(Context* ctx) {
        ctx_ = ctx;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    const std::vector<typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ModObj>& GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::modObjs() const {
        return modObjs_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::setModObjs(std::vector<ModObj> newModObjs) {
        modObjs_.clear();
        std::unordered_map<ModObj, bool, ModObjHash> seen;

        for (ModObj& modObj : newModObjs) {
            if (seen.emplace(modObj, true).second) {
                modObjs_.push_back(std::move(modObj));
            }
        }

        refreshComponents();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::refreshComponents() {
        components_.clear();
        std::unordered_set<std::string> seen;

        for (const ModObj& modObj : modObjs_) {
            if (seen.insert(modObj.first).second) {
                components_.push_back(modObj.first);
            }
        }

        // The cached name classifier was built for a different set of mod objects, so it no longer
        // answers the question this parser is going to ask it.
        nameClassifier_.reset();
        nameClassifierModObjs_.clear();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    const std::vector<std::string>& GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::components() const {
        return components_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::GroupEdit* GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::commandEdits() const {
        return commandEdits_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::setCommandEdits(GroupEdit* newCommandEdits) {
        commandEdits_ = newCommandEdits;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    std::vector<std::pair<typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ModObj, typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::Graph*>>
    GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::commandGraphs() const {
        std::vector<GraphMapEntry> result;
        if (ctx_ == nullptr) {
            return result;
        }

        typename Context::GraphGroups& groups = ctx_->graphGroups();
        if (groups.size() == 0) {
            return result;
        }

        for (const ModObj& modObj : groups.modObjs(0)) {
            result.emplace_back(modObj, groups.getGraph(0, modObj));
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::Graph* GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::getCommandGraph(const ModObj& modObj) const {
        if (ctx_ == nullptr || ctx_->graphGroups().size() == 0) {
            return nullptr;
        }

        return ctx_->graphGroups().getGraph(0, modObj);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    const typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ResourceGraphs& GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::downloadResourceGraphs() const {
        return downloadResourceGraphs_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    const tsl::ordered_map<typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ModObj, std::vector<std::string>, typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ModObjHash>&
    GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::sectionTargets() const {
        return sectionTargets_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::Graph* GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::globalGraph() const {
        return globalGraph_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::setGlobalGraph(Graph* newGlobalGraph) {
        globalGraph_ = newGlobalGraph;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    const typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ParserConfig& GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::config() const {
        return config_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::removeAddedIfTemplates() {
        if (ctx_ != nullptr) {
            for (const std::string& sectionName : addedIfTemplateNames) {
                ctx_->removeSection(sectionName);
            }
        }

        addedIfTemplateNames.clear();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::clear() {
        Base::clear();

        if (ctx_ != nullptr && ctx_->graphGroups().size() > 0) {
            typename Context::GraphGroups& groups = ctx_->graphGroups();
            for (const ModObj& modObj : groups.modObjs(0)) {
                groups.removeGraph(0, modObj);
            }
        }

        downloadResourceGraphs_.clear();
        createdDownloadResources_.clear();
        removeAddedIfTemplates();
        globalGraph_ = nullptr;
        defaultClassifier_.reset();
        downloadsAdded_ = false;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::Graph* GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::buildGlobalGraph() {
        return ctx_->graphGroups().createGraph(ctx_->sectionIfTemplates(), ctx_->sectionNames(), false, ctx_->z3Ctx());
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::NameClassifier& GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::nameClassifier(const std::vector<ModObj>& modObjs) {
        if (nameClassifier_ != nullptr && nameClassifierModObjs_ == modObjs) {
            return *nameClassifier_;
        }

        std::unordered_map<std::string, std::optional<ModObj>> data;
        for (const ModObj& modObj : modObjs) {
            data[StringTools::toLower(modObj.first + modObj.second)] = modObj;
        }

        // std::nullopt marks the "this section was written by us" keyword -- see NameClassifier.
        data[StringTools::toLower(IniKeywords::Remap)] = std::nullopt;

        nameClassifier_ = std::make_unique<NameClassifier>(data);
        nameClassifierModObjs_ = modObjs;
        return *nameClassifier_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    std::vector<typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ModObj> GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::classifyByTextureOverrideName(
            GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>& parser, const std::string& sectionName, bool disjoint,
            const std::vector<ModObj>* modObjs, bool fromRoots) {
        std::vector<ModObj> result;

        if (fromRoots && parser.globalGraph() == nullptr) {
            parser.setGlobalGraph(parser.buildGlobalGraph());
        }

        const std::vector<ModObj>& targetModObjs = (modObjs == nullptr) ? parser.modObjs() : *modObjs;
        NameClassifier& classifier = parser.nameClassifier(targetModObjs);

        static const std::string textureOverrideKey = StringTools::toLower(IniKeywords::TextureOverride);
        static const std::string remapKey = StringTools::toLower(IniKeywords::Remap);

        std::string cleanedSectionName = StringTools::toLower(StringTools::strip(sectionName));
        if (cleanedSectionName.rfind(textureOverrideKey, 0) != 0) {
            return result;
        }

        cleanedSectionName = cleanedSectionName.substr(textureOverrideKey.size());
        std::unordered_map<std::string, const std::optional<ModObj>*> found = classifier.getAll(cleanedSectionName);

        if (found.empty() || found.count(remapKey) != 0) {
            return result;
        }

        // Only keywords the name actually *ends* with classify it. The pure-Python original picks
        // between several such keywords by whatever order its Aho-Corasick implementation happened
        // to report them in; this sorts longest-first (then lexicographically) so that "the most
        // specific mod object wins" deterministically instead.
        std::vector<std::string> matched;
        for (const auto& entry : found) {
            if (StringTools::endsWith(cleanedSectionName, entry.first)) {
                matched.push_back(entry.first);
            }
        }

        std::sort(matched.begin(), matched.end(), [](const std::string& left, const std::string& right) {
            if (left.size() != right.size()) {
                return left.size() > right.size();
            }
            return left < right;
        });

        for (const std::string& keyword : matched) {
            const std::optional<ModObj>* modObj = found.at(keyword);
            if (modObj == nullptr || !modObj->has_value()) {
                continue;
            }

            result.push_back(**modObj);

            if (disjoint) {
                break;
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    std::vector<typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::ObjTargetFunc> GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::resolveObjTargetFuncs(bool byKVPs) {
        if (!objTargetFuncs.empty()) {
            return objTargetFuncs;
        }

        if (byKVPs && ctx_ != nullptr && ctx_->hasModType()) {
            // Built fresh here (and kept alive as a member) rather than handed to the caller by
            // value: GIMISectionClassifier is stateful, and the pure-Python original likewise
            // rebuilds it per call.
            defaultClassifier_ = Classifier::buildDefaultClassifierFromIni(*ctx_, config_.classifier);

            Classifier* classifier = defaultClassifier_.get();
            return {[classifier](GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>&, const std::string& sectionName, Section* section,
                                  bool, ContentPart*, const Colouring* colouring) {
                if (colouring == nullptr) {
                    return std::vector<ModObj>();
                }
                return classifier->classify(sectionName, section, *colouring);
            }};
        }

        return {[](GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>& parser, const std::string& sectionName, Section*, bool disjoint,
                    ContentPart*, const Colouring*) {
            return classifyByTextureOverrideName(parser, sectionName, disjoint, &parser.modObjs(), true);
        }};
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::getSectionTargetsBySectionNames(tsl::ordered_map<ModObj, std::vector<std::string>, ModObjHash>& result) {
        std::vector<ObjTargetFunc> funcs = resolveObjTargetFuncs(false);

        for (const std::string& sectionName : ctx_->sectionNames()) {
            Section* section = ctx_->getSection(sectionName);
            std::vector<ModObj> sectionResult;
            std::unordered_map<ModObj, bool, ModObjHash> seen;

            for (const ObjTargetFunc& func : funcs) {
                if (!func) {
                    continue;
                }

                for (ModObj& modObj : func(*this, sectionName, section, disjointModObjs, nullptr, nullptr)) {
                    if (seen.emplace(modObj, true).second) {
                        sectionResult.push_back(std::move(modObj));
                    }
                }

                if (!sectionResult.empty() && disjointModObjs) {
                    sectionResult.resize(1);
                    break;
                }
            }

            for (const ModObj& modObj : sectionResult) {
                // A mod object nothing asked for is dropped rather than added. The pure-Python
                // original raises KeyError here instead (its result dict is pre-seeded from
                // modObjs and never grown), which no caller could usefully catch -- and
                // parseCommands only ever looks up modObjs anyway, so an extra entry would be
                // dead either way.
                auto found = result.find(modObj);
                if (found != result.end()) {
                    result[modObj].push_back(sectionName);
                }
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::getSectionTargetsByKVPs(tsl::ordered_map<ModObj, std::vector<std::string>, ModObjHash>& result) {
        std::vector<ObjTargetFunc> funcs = resolveObjTargetFuncs(true);
        if (globalGraph_ == nullptr) {
            return;
        }

        tsl::ordered_map<std::string, std::vector<ModObj>> sectionResults;
        tsl::ordered_map<std::string, std::unordered_map<ModObj, bool, ModObjHash>> seen;

        auto walk = globalGraph_->iterByContentPart(1, trackKeys, keysToTrack);
        while (walk.next()) {
            typename Graph::IterData& iterData = walk.value();
            const std::string& sectionName = iterData.sectionName;

            if (sectionResults.find(sectionName) == sectionResults.end()) {
                sectionResults[sectionName] = {};
                seen[sectionName] = {};
            }

            for (const ObjTargetFunc& func : funcs) {
                if (!func) {
                    continue;
                }

                for (ModObj& modObj : func(*this, sectionName, iterData.section, disjointModObjs, iterData.part, iterData.colouring)) {
                    if (seen[sectionName].emplace(modObj, true).second) {
                        sectionResults[sectionName].push_back(std::move(modObj));
                    }
                }
            }
        }

        for (const auto& entry : sectionResults) {
            for (const ModObj& modObj : entry.second) {
                // Same "drop what nothing asked for" note as getSectionTargetsBySectionNames.
                auto found = result.find(modObj);
                if (found != result.end()) {
                    result[modObj].push_back(entry.first);
                }
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::getSectionTargets() {
        if (ctx_ == nullptr) {
            return;
        }

        if (makeGlobalGraph) {
            globalGraph_ = buildGlobalGraph();
        }

        sectionTargets_.clear();

        tsl::ordered_map<ModObj, std::vector<std::string>, ModObjHash> result;
        for (const ModObj& modObj : modObjs_) {
            result[modObj] = {};
        }

        if (!trackKeys || !makeGlobalGraph) {
            getSectionTargetsBySectionNames(result);
        } else {
            getSectionTargetsByKVPs(result);
        }

        sectionTargets_ = std::move(result);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::parseCommands() {
        if (ctx_ == nullptr) {
            return;
        }

        typename Context::GraphGroups& groups = ctx_->graphGroups();
        if (groups.size() == 0) {
            groups.insertGroup(0);
        }

        for (const ModObj& modObj : modObjs_) {
            auto found = sectionTargets_.find(modObj);
            if (found == sectionTargets_.end()) {
                continue;
            }

            Graph* commandGraph = groups.createGraph(ctx_->sectionIfTemplates(), found->second, false, ctx_->z3Ctx());
            groups.addGraph(0, modObj, commandGraph);
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::DownloadNeeds GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::getDownloads() {
        DownloadNeeds result;
        std::set<ContentPart*> visitedParts;

        if (ctx_ == nullptr) {
            return result;
        }

        for (const auto& modObjEntry : downloads) {
            const ModObj& modObj = modObjEntry.first;
            Graph* commandGraph = getCommandGraph(modObj);
            if (commandGraph == nullptr) {
                continue;
            }

            for (const auto& regEntry : modObjEntry.second) {
                const K& reg = regEntry.first;
                DownloadData* download = regEntry.second;
                if (download == nullptr) {
                    continue;
                }

                DownloadTargets targets;
                targets.refToSection = download->refToSection();

                if (!targets.refToSection) {
                    for (const auto& sectionEntry : commandGraph->getKeyMissingParts(reg)) {
                        for (ContentPart* part : sectionEntry.second) {
                            if (visitedParts.insert(part).second) {
                                targets.parts.insert(part);
                            }
                        }
                    }
                } else if (ctx_->downloadMode() != DownloadMode::Always) {
                    for (const auto& coverEntry : commandGraph->rootsAreFullyCovered(reg)) {
                        if (!coverEntry.second) {
                            targets.sections.insert(commandGraph->getSection(coverEntry.first));
                        }
                    }
                } else {
                    for (Section* section : commandGraph->getRootSections()) {
                        targets.sections.insert(section);
                    }
                }

                result[modObj][reg] = std::move(targets);
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    std::string GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::createDownloadResource(const std::string& modTypeName, const ModObj& modObj, const K& reg,
                                                                            DownloadData& downloadData, const std::string& iniFolder) {
        std::string resourceSectionName = IniNamingTools::getRemapDLResourceName(
            TextTools::capitalize(modTypeName) + TextTools::capitalize(downloadData.name()));

        auto& objGraphs = downloadResourceGraphs_[modObj];
        auto foundGraph = objGraphs.find(reg);
        Graph* resourceGraph = nullptr;

        if (foundGraph == objGraphs.end()) {
            resourceGraph = ctx_->graphGroups().createGraph({}, {}, false, nullptr);
            objGraphs[reg] = resourceGraph;
        } else {
            resourceGraph = foundGraph->second;
        }

        if (resourceGraph->sections().find(resourceSectionName) != resourceGraph->sections().end()) {
            return resourceSectionName;
        }

        Section* downloadSection = nullptr;

        if (createdDownloadResources_.count(resourceSectionName) != 0) {
            // Already built, for some earlier register or mod object -- reuse that one `section`_
            // rather than building (and downloading) a second copy of it. See this method's own
            // note on why the `section`_ name is the identity here.
            downloadSection = ctx_->getSection(resourceSectionName);
        } else {
            downloadSection = downloadData.createResSection(resourceSectionName, *ctx_);
            createdDownloadResources_.insert(resourceSectionName);
            downloadData.addFileDownload(*ctx_, iniFolder);
        }

        if (downloadSection == nullptr) {
            return resourceSectionName;
        }

        std::unordered_map<std::string, Section*> sections = resourceGraph->sections();
        sections[resourceSectionName] = downloadSection;

        std::vector<std::string> targets = resourceGraph->targetSectionNames();
        targets.push_back(resourceSectionName);

        // The pure-Python original assigns into 'resourceGraph.sections'/'targetSectionNames'
        // directly and defers the rebuild to the end of addDownloads. Both of those are read-only
        // views over the C++ graph, so a rebuild here is what actually lands the new section --
        // and it is what makes this work at all (writing through those views is silently discarded
        // in the pure-Python original, which is why its own download resources never appear).
        resourceGraph->build(std::move(sections), std::move(targets));

        return resourceSectionName;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::addDownloads(const DownloadNeeds& partsNeedDownload) {
        if (ctx_ == nullptr) {
            return;
        }

        std::string modTypeName = ctx_->modTypeName();
        std::string iniFolder = ctx_->iniFolder();

        for (const auto& modObjEntry : partsNeedDownload) {
            const ModObj& modObj = modObjEntry.first;

            auto objDownloads = downloads.find(modObj);
            if (objDownloads == downloads.end() || getCommandGraph(modObj) == nullptr) {
                continue;
            }

            for (const auto& regEntry : modObjEntry.second) {
                const K& reg = regEntry.first;
                auto foundDownload = objDownloads->second.find(reg);
                if (foundDownload == objDownloads->second.end() || foundDownload->second == nullptr) {
                    continue;
                }

                DownloadData& downloadData = *foundDownload->second;
                std::string resourceSectionName = createDownloadResource(modTypeName, modObj, reg, downloadData, iniFolder);
                V resourceSectionVal = config_.valOfSectionName(resourceSectionName);

                const DownloadTargets& needDownloadData = regEntry.second;

                if (!downloadData.refToSection()) {
                    for (ContentPart* part : needDownloadData.parts) {
                        downloadData.addToPart(*part, reg, resourceSectionVal);
                    }
                    continue;
                }

                for (Section* section : needDownloadData.sections) {
                    downloadData.addToSection(*section, reg, resourceSectionVal);
                }
            }
        }

        // add in the downloads for mod objects whose command graph turned out to be empty
        std::unordered_map<ModObj, bool, ModObjHash> commandGraphWasEmpty;

        for (const auto& modObjEntry : downloads) {
            const ModObj& modObj = modObjEntry.first;

            for (const auto& regEntry : modObjEntry.second) {
                const K& reg = regEntry.first;
                DownloadData* downloadPtr = regEntry.second;
                Graph* commandGraph = getCommandGraph(modObj);

                if (commandGraph == nullptr || downloadPtr == nullptr) {
                    continue;
                }

                DownloadData& downloadData = *downloadPtr;
                bool graphWasEmpty = false;
                bool graphIsEmpty = false;

                auto wasEmpty = commandGraphWasEmpty.find(modObj);
                if (wasEmpty == commandGraphWasEmpty.end()) {
                    graphWasEmpty = commandGraph->isEmpty();
                    commandGraphWasEmpty[modObj] = graphWasEmpty;
                    graphIsEmpty = graphWasEmpty;
                } else {
                    graphWasEmpty = wasEmpty->second;
                    graphIsEmpty = commandGraph->isEmpty();
                }

                if (!graphWasEmpty) {
                    continue;
                }

                std::string commandSectionName = IniNamingTools::getTextureOverrideRemapFix(modObj.first, modObj.second, modTypeName);
                std::string resourceSectionName = createDownloadResource(modTypeName, modObj, reg, downloadData, iniFolder);
                V resourceSectionVal = config_.valOfSectionName(resourceSectionName);

                if (graphIsEmpty) {
                    Section* commandIfTemplate = ctx_->addSection(commandSectionName,
                        std::make_unique<Section>(std::vector<std::unique_ptr<IfTemplatePart>>{}, config_.runConfig, commandSectionName));

                    commandGraph->build(std::unordered_map<std::string, Section*>{{commandSectionName, commandIfTemplate}},
                                         std::vector<std::string>{commandSectionName});

                    downloadData.addToSection(*commandIfTemplate, reg, resourceSectionVal);
                    continue;
                }

                Section* commandIfTemplate = ctx_->getSection(commandSectionName);
                if (commandIfTemplate == nullptr) {
                    continue;
                }

                if (!downloadData.refToSection()) {
                    auto* part = dynamic_cast<ContentPart*>(commandIfTemplate->parts().empty() ? nullptr : commandIfTemplate->parts()[0].get());
                    if (part != nullptr) {
                        downloadData.addToPart(*part, reg, resourceSectionVal);
                    }
                } else {
                    downloadData.addToSection(*commandIfTemplate, reg, resourceSectionVal);
                }
            }
        }

        // build the resource graphs
        for (const auto& modObjEntry : downloadResourceGraphs_) {
            for (const auto& regEntry : modObjEntry.second) {
                if (regEntry.second != nullptr) {
                    regEntry.second->build();
                }
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::setupDownloads() {
        if (ctx_ == nullptr) {
            return;
        }

        if (ctx_->downloadMode() == DownloadMode::Disabled && !downloadsAdded_) {
            downloadsAdded_ = true;
            return;
        }

        if (downloadsAdded_) {
            return;
        }

        if (ctx_->downloadMode() == DownloadMode::Always) {
            for (const GraphMapEntry& entry : commandGraphs()) {
                if (entry.second != nullptr) {
                    entry.second->normalize();
                }
            }
        }

        downloadsAdded_ = true;
        addDownloads(getDownloads());
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    void GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::editCommands() {
        if (commandEdits_ == nullptr || ctx_ == nullptr) {
            return;
        }

        // 'modType' is nullptr rather than the mod type the .ini file was classified as: there is
        // nothing castable to an AGRemapCore::ModType from a still-pure-Python one, the same
        // situation every graphGroupEdits/ caller is in. The pybind11 layer overrides this method
        // and passes the real Python objects instead.
        commandEdits_->editFromIni(ctx_->graphGroups(), this->iniFile_, nullptr, ctx_->modTypeName());
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    std::vector<typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::GraphGroup>
    GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::collectParseResult() const {
        std::vector<GraphGroup> result;
        GraphGroup group;

        for (const GraphMapEntry& entry : commandGraphs()) {
            if (entry.second != nullptr) {
                group.addGraph(entry.first, entry.second->deepcopy());
            }
        }

        for (const auto& modObjEntry : downloadResourceGraphs_) {
            auto objDownloads = downloads.find(modObjEntry.first);
            if (objDownloads == downloads.end()) {
                continue;
            }

            for (const auto& regEntry : modObjEntry.second) {
                auto foundDownload = objDownloads->second.find(regEntry.first);
                if (foundDownload == objDownloads->second.end() || foundDownload->second == nullptr
                        || regEntry.second == nullptr) {
                    continue;
                }

                // Keyed by the download's own name rather than the register's: one download
                // resource can be referenced from several registers, and the group should hold it
                // once. Matches what GIMIFixer builds by hand today.
                ModObj modObj(IniGraphModObjKeywords::Download, foundDownload->second->name());
                if (group.getGraph(modObj) != nullptr) {
                    continue;
                }

                group.addGraph(std::move(modObj), regEntry.second->deepcopy());
            }
        }

        result.push_back(std::move(group));
        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ParserBase>
    std::vector<typename GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::GraphGroup> GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>::parse() {
        // A parser is built for exactly one mod type (see IniFile::getParser), so naming it here
        // is what tells a reader which of a multi-type .ini file's passes is running.
        const std::string modTypeName = (ctx_ == nullptr) ? "" : ctx_->modTypeName();
        if (ctx_ != nullptr && !modTypeName.empty()) {
            ctx_->log("Parsing the .ini file for " + modTypeName);
        }

        getSectionTargets();
        parseCommands();
        setupDownloads();
        editCommands();

        return collectParseResult();
    }
}

#endif
