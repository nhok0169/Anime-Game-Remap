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

#include "AGRemapCore/model/strategies/iniParsers/TemplateParser.h"

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCreate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"
#include "AGRemapCore/tools/StringTools.h"

#include <utility>


namespace AGRemapCore {
    TemplateParser::TemplateParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs):
        GIMIParser<>(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId) {
        // The context is a member, so it can only be handed to the base after that base is built
        this->setCtx(&ctx_);
        this->setIniFile(iniFile);
    }

    TemplateParser::Classifier& TemplateParser::ownClassifier(std::unique_ptr<Classifier> classifier) {
        classifiers_.push_back(std::move(classifier));
        return *classifiers_.back();
    }

    TemplateParser::GroupEdit& TemplateParser::ownStep(std::unique_ptr<GroupEdit> step) {
        steps_.push_back(std::move(step));
        return *steps_.back();
    }

    TemplateParser::GroupEdit& TemplateParser::addParseStep(std::unique_ptr<GroupEdit> step) {
        GroupEdit& owned = ownStep(std::move(step));
        this->parseSteps.push_back(&owned);
        return owned;
    }

    TemplateParser::GroupEdit& TemplateParser::addDeferredParseStep(std::unique_ptr<GroupEdit> step) {
        GroupEdit& owned = ownStep(std::move(step));
        this->deferredParseSteps.push_back(&owned);
        return owned;
    }

    std::string TemplateParser::resourceNamed(const std::string& value) const {
        const std::string name = IniNamingTools::removeRefPrefix(std::string(StringTools::strip(value)));
        const auto declared = declaredResources_.find(StringTools::toLower(name));
        return declared == declaredResources_.end() ? std::string() : declared->second;
    }

    // The global graph -- and, found as it is handed over, the resource sections the .ini declares and the keys of
    // the graph that ever name one
    TemplateParser::Graph* TemplateParser::globalGraphForResources() {
        resourceKeys_.clear();
        declaredResources_.clear();
        for (const std::string& name : ctx_.sectionNames()) {
            if (StringTools::startsWith(StringTools::toLower(name), "resource")) {
                declaredResources_.emplace(StringTools::toLower(name), name);
            }
        }

        Graph* graph = this->globalGraph();
        if (graph == nullptr) {
            return nullptr;
        }

        for (const auto& [name, section] : graph->sections()) {
            (void)name;
            if (section == nullptr) {
                continue;
            }

            for (const auto& part : section->parts()) {
                const auto* content = dynamic_cast<const ContentPart*>(part.get());
                if (content == nullptr) {
                    continue;
                }

                for (const auto& item : content->items()) {
                    if (!resourceNamed(item.value).empty()) {
                        resourceKeys_.insert(item.key);
                    }
                }
            }
        }
        return graph;
    }

    void TemplateParser::addRowSteps(const RowSteps& rows) {
        using Collect = GraphCollect<>;
        using GraphId = GroupEdit::GraphId;

        // the graph the resource collect builds of the resource sections the mod binds, and the copy of the parser's
        // global graph it reads them from: a mod binds buffers from sections no mod object reaches ([Present], a
        // callback's command list)
        static const GraphId ResourceGraph{0, "resource", ""};
        static const GraphId GlobalGraph{0, "global", ""};
        const auto anyKey = [](const std::string&) { return true; };

        auto global = std::make_unique<GraphCreate<>>(GlobalGraph, nullptr);
        global->graphOf = [this]() { return globalGraphForResources(); };
        addDeferredParseStep(std::move(global));

        if (rows.resources) {
            auto collect = std::make_unique<ResRegCollect<>>();
            collect->srcRegMatches.insert_or_assign(GlobalGraph, [this](const std::string& key) {
                return resourceKeys_.count(key) > 0;
            });
            collect->resPredicates.insert_or_assign(GlobalGraph, [this](const std::string&, const std::string& value,
                                                                        const ResRegCollect<>::IterData&) {
                return !resourceNamed(value).empty();
            });
            BaseResEdit<>::ResEditConfig resConfig{
                IniKeywords::Filename,
                [this](const std::string& value) { return resourceNamed(value); },
                [](const std::string& file) { return file; }
            };
            resourceIdentity_ = std::make_unique<ResIdentity<>>(ResourceGraph, std::move(resConfig), false);
            collect->resEdits.insert_or_assign("", resourceIdentity_.get());
            addDeferredParseStep(std::move(collect));
        }

        auto files = std::make_unique<Collect>(Collect::ByGraph<std::vector<std::string>>{{ResourceGraph, {IniKeywords::Filename}}});
        files->name = IniParseRows::Resources;

        // every section's declaration, bound or not
        std::vector<std::string> declaredKeys{IniKeywords::Filename, IniKeywords::Type, IniKeywords::Format, IniKeywords::Stride};
        declaredKeys.insert(declaredKeys.end(), rows.declaredKeys.begin(), rows.declaredKeys.end());
        auto declared = std::make_unique<Collect>(Collect::ByGraph<std::vector<std::string>>{{GlobalGraph, std::move(declaredKeys)}});
        declared->name = IniParseRows::Declared;

        auto objs = std::make_unique<Collect>();
        objs->name = IniParseRows::Objs;
        for (const ModObj& obj : rows.objs ? this->modObjs() : std::vector<ModObj>{}) {
            if (rows.objs(obj)) {
                objs->keyMatches.insert_or_assign(GraphId(0, obj.first, obj.second), anyKey);
            }
        }

        addDeferredParseStep(std::move(files));
        addDeferredParseStep(std::move(declared));
        if (rows.sections) {
            auto sections = std::make_unique<Collect>();
            sections->name = IniParseRows::Sections;
            sections->keyMatches.insert_or_assign(GlobalGraph, anyKey);
            addDeferredParseStep(std::move(sections));
        }
        addDeferredParseStep(std::make_unique<GraphRemove<>>(std::vector<GraphId>{ResourceGraph, GlobalGraph}));
        addDeferredParseStep(std::move(objs));
    }
}
