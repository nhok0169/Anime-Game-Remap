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

#include "AGRemapCore/model/strategies/iniFixers/TemplateFixer.h"

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/tools/StringTools.h"

#include <algorithm>
#include <unordered_map>


namespace AGRemapCore {
    namespace {
        TemplateFixer::Fixer::FixerConfig templateFixerConfig() {
            TemplateFixer::Fixer::FixerConfig config{};
            config.sectionToStr = &renderIfTemplate;
            return config;
        }

        std::optional<std::vector<std::string>> onlyModToFix(const std::string& toModName) {
            return toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                     : std::optional<std::vector<std::string>>({toModName});
        }
    }


    TemplateFixer::TemplateFixer(BaseIniParser<std::string, std::string>* parser, const std::string& toModName,
                                 std::optional<int> modTypeId):
        Fixer(parser, nullptr, {}, onlyModToFix(toModName), nullptr, templateFixerConfig()),
        ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName) {
        this->setCtx(&ctx_);
    }

    bool TemplateFixer::hasGivenUp() const {
        return gaveUp_;
    }

    BaseResEdit<std::string, std::string>::ResEditConfig TemplateFixer::resEditConfig(KVPs extras) {
        BaseResEdit<std::string, std::string>::ResEditConfig config{
            IniKeywords::Filename,
            [](const std::string& value) { return value; },
            [](const std::string& file) { return file; }
        };
        config.extraKVPs = std::move(extras);
        return config;
    }

    IfTemplateRunConfig<std::string, std::string> TemplateFixer::sectionRunConfig() {
        return IfTemplateRunConfig<std::string, std::string>{
            IniKeywords::Run,
            [](const std::string& val) { return val; },
            [](const std::string& section) { return section; }
        };
    }

    std::unique_ptr<TemplateFixer::Section> TemplateFixer::makeSection(const std::string& name, const KVPs& kvps) {
        std::vector<std::unique_ptr<IfTemplatePart>> parts;
        parts.push_back(std::make_unique<IfContentPart<std::string, std::string>>(kvps, 0));
        return std::make_unique<Section>(std::move(parts), sectionRunConfig(), name);
    }

    std::string TemplateFixer::renderSection(const std::string& name, const KVPs& kvps) {
        std::unique_ptr<Section> section = makeSection(name, kvps);
        return renderIfTemplate(*section) + "\n";
    }

    void TemplateFixer::applyGraphGroupEdits(const std::string& modName) {
        if (this->graphGroups() == nullptr) {
            return;
        }

        beforeGraphGroupEdits(modName);

        for (Fixer::GroupEdit* edit : this->graphGroupEdits) {
            if (edit != nullptr) {
                edit->editFromIni(*this->graphGroups(), ctx_.getIniFile(), nullptr, modName);
            }
        }

        addFixSectionGraphs(modName);

        // The parser registers a download per object it filled, at parse time, and fixResources
        // fetches every one it finds whether or not anything references it -- so a fix that writes
        // nothing would still drop the game's files into the mod's folder.
        if (gaveUp_ && withdrawDownloadsOnGiveUp && ctx_.getIniFile() != nullptr) {
            ctx_.getIniFile()->getFileDownloads().clear();
        }

        afterGraphGroupEdits(modName);
    }

    void TemplateFixer::beforeGraphGroupEdits(const std::string&) {}

    void TemplateFixer::afterGraphGroupEdits(const std::string&) {}

    void TemplateFixer::giveUp() {
        gaveUp_ = true;
        this->graphGroupEdits = {&removeEveryGroup_};
        fixSections_.clear();
        groupFixSections_.clear();
    }

    // see TemplateFixer::takeFixSections
    class TemplateFixer::TakeFixSections: public BaseIniGraphGroupEdit<std::string, std::string> {
        public:
            TakeFixSections(TemplateFixer& fixer, ModObj graph, std::vector<std::string> sections, std::string comment):
                fixer_(fixer), graph_(std::move(graph)), sections_(std::move(sections)), comment_(std::move(comment)) {}

            GraphGroups& edit(GraphGroups& graphGroups, const ModType*, const std::string& = "") override {
                if (graphGroups.size() == 0) {
                    return graphGroups;
                }

                auto* graph = graphGroups.getGraph(0, graph_);
                if (graph == nullptr) {
                    return graphGroups;
                }

                for (const std::string& name : sections_) {
                    Section* section = graph->getSection(name, false);
                    if (section == nullptr) {
                        continue;
                    }

                    std::unique_ptr<Section> copy = section->deepcopy();
                    copy->prefix = comment_.empty() ? "" : "; " + comment_;
                    copy->rebuild();
                    fixer_.addFixSection(std::move(copy));
                }

                graphGroups.removeGraph(0, graph_);
                return graphGroups;
            }

        private:
            TemplateFixer& fixer_;
            ModObj graph_;
            std::vector<std::string> sections_;
            std::string comment_;
    };

    TemplateFixer::GroupEdit* TemplateFixer::takeFixSections(const ModObj& graph, std::vector<std::string> sections,
                                                                  const std::string& comment) {
        takeEdits_.push_back(std::make_unique<TakeFixSections>(*this, graph, std::move(sections), comment));
        return takeEdits_.back().get();
    }

    std::unique_ptr<TemplateFixer::Section> TemplateFixer::copyOnHash(const Section& section, const std::string& hash,
                                                                       const std::string& comment) const {
        std::unique_ptr<Section> copy = section.deepcopy();
        copy->name = remapFixName(section.name);
        copy->prefix = comment.empty() ? "" : "; " + comment;
        for (const std::unique_ptr<IfTemplatePart>& part : copy->parts()) {
            auto* content = dynamic_cast<IfContentPart<std::string, std::string>*>(part.get());
            if (content != nullptr) {
                RegNewVals<>({{std::string(IniKeywords::Hash), RegNewVals<>::NewVal(hash)}}).edit(*content, copy->name);
            }
        }

        copy->rebuild();
        return copy;
    }

    std::optional<Version> TemplateFixer::iniToVersion() const {
        const IniFile* ini = ctx_.getIniFile();
        return ini == nullptr ? std::nullopt : ini->toVersion;
    }

    std::string TemplateFixer::remapFixName(const std::string& name) const {
        return IniNamingTools::getRemapFixName(name, toModName_);
    }

    std::string TemplateFixer::remapName(RemapName kind, const std::string& name) const {
        switch (kind) {
            case RemapName::Blend:
                return IniNamingTools::getRemapBlendName(name, toModName_);
            case RemapName::Position:
                return IniNamingTools::getRemapPositionName(name, toModName_);
            case RemapName::Texcoord:
                return IniNamingTools::getRemapTexcoordName(name, toModName_);
            case RemapName::Ib:
                return IniNamingTools::getRemapIbName(name, toModName_);
            default:
                return IniNamingTools::getRemapFixName(name, toModName_);
        }
    }

    std::unique_ptr<GraphRename<std::string, std::string>> TemplateFixer::remapRename(RemapName kind, RenameWrap wrap) const {
        // the names are the target's, captured by value: the edit outlives no fixer, but runs after
        // the fixer's constructor has returned
        return std::make_unique<GraphRename<std::string, std::string>>(
            [toModName = toModName_, kind, wrap = std::move(wrap)](const std::string& name) {
                std::string renamed;
                switch (kind) {
                    case RemapName::Blend:
                        renamed = IniNamingTools::getRemapBlendName(name, toModName);
                        break;
                    case RemapName::Position:
                        renamed = IniNamingTools::getRemapPositionName(name, toModName);
                        break;
                    case RemapName::Texcoord:
                        renamed = IniNamingTools::getRemapTexcoordName(name, toModName);
                        break;
                    case RemapName::Ib:
                        renamed = IniNamingTools::getRemapIbName(name, toModName);
                        break;
                    default:
                        renamed = IniNamingTools::getRemapFixName(name, toModName);
                        break;
                }
                return wrap ? wrap(name, renamed) : renamed;
            });
    }

    std::unique_ptr<RegAssetRemap<std::string, std::string>> TemplateFixer::hashRemap(
            const std::string& fromName, const std::optional<Version>& toVersion,
            const std::optional<std::string>& notFound, AssetSpecs extraSpecs) const {
        AssetSpecs specs;
        specs.emplace_back(IniKeywords::Hash, RegAssetRemap<std::string, std::string>::AssetSpec(ctx_.modTypeHashes(), notFound));
        for (auto& spec : extraSpecs) {
            specs.push_back(std::move(spec));
        }

        return std::make_unique<RegAssetRemap<std::string, std::string>>(std::move(specs), toModName_, fromName, ctx_.version(), toVersion);
    }

    bool TemplateFixer::isFixLibraryCall(const std::string& value) {
        return StringTools::equalsIgnoreCase(StringTools::strip(value), IniKeywords::ORFixPath)
               || StringTools::equalsIgnoreCase(StringTools::strip(value), IniKeywords::NNFixPath);
    }

    std::string TemplateFixer::texFxGuardSection(const std::string& name, const std::string& hash, const std::string& firstIndex) {
        return renderSection(name, {{IniKeywords::Hash, hash}, {IniKeywords::MatchFirstIndex, firstIndex},
                                    {IniKeywords::TexFxUseDefaultShader, "-1"}});
    }

    void TemplateFixer::addFixSection(std::unique_ptr<Section> section, std::optional<std::size_t> group) {
        if (section == nullptr) {
            return;
        }

        if (group.has_value()) {
            groupFixSections_[*group].push_back(std::move(section));
        } else {
            fixSections_.push_back(std::move(section));
        }
    }

    GraphRemove<std::string, std::string>::GraphId TemplateFixer::fixSectionsGraphId(std::size_t group) {
        return GraphRemove<std::string, std::string>::GraphId(group, "", "fixSections");
    }

    void TemplateFixer::addFixSectionGraphs(const std::string& modName) {
        Fixer::GraphGroups& groups = *this->graphGroups();
        // ...and a group of their own when every group was removed (see giveUp)
        const std::size_t count = std::max<std::size_t>(groups.size(), fixSections_.empty() ? 0 : 1);
        for (std::size_t g = 0; g < count; ++g) {
            std::vector<Section*> sections;
            if (g == 0 || fixSectionsInCopies) {
                for (const auto& section : fixSections_) {
                    sections.push_back(section.get());
                }
            }
            const auto own = groupFixSections_.find(g);
            if (own != groupFixSections_.end()) {
                for (const auto& section : own->second) {
                    sections.push_back(section.get());
                }
            }
            if (sections.empty()) {
                continue;
            }

            // in the order they were added; a repeated name keeps its first section
            std::unordered_map<std::string, Section*> byName;
            std::vector<std::string> names;
            for (Section* section : sections) {
                if (byName.emplace(section->name, section).second) {
                    names.push_back(section->name);
                }
            }

            GraphCreate<std::string, std::string>::Graph graph(std::move(byName), names, sectionRunConfig(), true, false, fixSectionsZ3);
            GraphCreate<std::string, std::string>(fixSectionsGraphId(g), &graph).edit(groups, nullptr, modName);
        }
    }

    void TemplateFixer::giveUp(const std::string& logLine) {
        ctx_.log(logLine);
        giveUp();
    }

    bool TemplateFixer::iniCallsTexFx() const {
        const IniFile* ini = ctx_.getIniFile();
        return ini != nullptr && StringTools::containsIgnoreCase(ini->getFileTxt(), IniKeywords::TexFxFolder + "\\");
    }
}
