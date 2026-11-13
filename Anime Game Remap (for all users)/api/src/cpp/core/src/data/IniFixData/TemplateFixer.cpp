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

#include "AGRemapCore/data/IniFixData/TemplateFixer.h"

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphVarApply.h"
#include "AGRemapCore/tools/StringTools.h"

#include <algorithm>
#include <optional>
#include <stdexcept>
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

        // each step is handed the mod type being fixed: a step that is the same for every mod reads the mod's values
        // off its parse data (IniFile::parseDataOf)
        for (GroupEdit* edit : list()) {
            if (edit != nullptr) {
                edit->editFromIni(*this->graphGroups(), ctx_.getIniFile(), ctx_.modType(), modName);
            }
        }

        // The parser registers a download per object it filled, at parse time, and fixResources
        // fetches every one it finds whether or not anything references it -- so a fix that writes
        // nothing would still drop the game's files into the mod's folder.
        if (gaveUp_ && withdrawDownloadsOnGiveUp && ctx_.getIniFile() != nullptr) {
            ctx_.getIniFile()->getFileDownloads().clear();
        }
    }

    std::vector<TemplateFixer::GroupEdit*> TemplateFixer::list() {
        if (listRewritten_) {
            return this->graphGroupEdits;
        }

        std::vector<GroupEdit*> steps = this->graphGroupEdits;
        steps.push_back(fixSectionsStep());
        for (const std::string& kind : fixSectionKinds_) {
            steps.push_back(fixSectionsStep(kind));
        }
        steps.insert(steps.end(), fixSectionEdits.begin(), fixSectionEdits.end());
        return steps;
    }

    GraphEditVariables& TemplateFixer::fixVariables() {
        return fixVariables_;
    }

    void TemplateFixer::adoptStep(GroupEdit* step) {
        if (auto* apply = dynamic_cast<GraphVarApply*>(step); apply != nullptr && apply->variables == nullptr) {
            apply->variables = &fixVariables_;
        }
    }

    void TemplateFixer::setList(std::vector<GroupEdit*> steps) {
        if (gaveUp_) {
            return;
        }
        for (GroupEdit* step : steps) {
            adoptStep(step);
        }
        this->graphGroupEdits = std::move(steps);
        fixSectionEdits.clear();
        listRewritten_ = true;
    }

    IniFixBuilder::Factory TemplateFixer::withList(
            IniFixBuilder::Factory factory,
            std::function<std::vector<GroupEdit*>(TemplateFixer& fixer, std::vector<GroupEdit*> steps)> rewrite) {
        return [factory = std::move(factory), rewrite = std::move(rewrite)](BaseIniParser<>* parser, const std::string& toModName,
                                                                           std::optional<int> modTypeId) {
            std::shared_ptr<BaseIniFixer<>> fixer = factory(parser, toModName, modTypeId);
            auto* templateFixer = dynamic_cast<TemplateFixer*>(fixer.get());
            if (templateFixer == nullptr) {
                throw std::invalid_argument("withList: the factory does not build a template fixer, whose list could be rewritten");
            }
            if (templateFixer->hasGivenUp()) {
                return fixer;
            }

            templateFixer->setList(rewrite(*templateFixer, templateFixer->list()));
            return fixer;
        };
    }

    void TemplateFixer::giveUp() {
        gaveUp_ = true;
        this->graphGroupEdits = {&removeEveryGroup_};
        fixSectionEdits.clear();
        listRewritten_ = false;
        fixVariables_.clear();
    }

    TemplateFixer::GroupEdit* TemplateFixer::takeFixSections(const ModObj& graph, std::vector<std::string> sections,
                                                                  const std::string& comment, bool inPlace) {
        takeEdits_.push_back(std::make_unique<GraphVarTake>(fixVariables_, fixSectionsVariable(), graph, std::move(sections),
                                                               comment, inPlace));
        return takeEdits_.back().get();
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

    std::string TemplateFixer::fixSectionsVariable() {
        return "fixSections";
    }

    std::string TemplateFixer::fixSectionsVariable(std::size_t group) {
        return fixSectionsVariable() + "." + std::to_string(group);
    }

    void TemplateFixer::addFixSection(std::unique_ptr<Section> section, std::optional<std::size_t> group) {
        fixVariables_.sections(group.has_value() ? fixSectionsVariable(*group) : fixSectionsVariable()).add(std::move(section));
    }

    std::string TemplateFixer::fixSectionsVariable(const std::string& kind, std::optional<std::size_t> group) {
        if (kind.empty()) {
            return group.has_value() ? fixSectionsVariable(*group) : fixSectionsVariable();
        }
        return fixSectionsVariable() + "." + kind + (group.has_value() ? "." + std::to_string(*group) : std::string());
    }

    void TemplateFixer::addFixSection(const std::string& kind, std::unique_ptr<Section> section, std::optional<std::size_t> group) {
        if (!kind.empty() && std::find(fixSectionKinds_.begin(), fixSectionKinds_.end(), kind) == fixSectionKinds_.end()) {
            fixSectionKinds_.push_back(kind);
        }
        fixVariables_.sections(fixSectionsVariable(kind, group)).add(std::move(section));
    }

    void TemplateFixer::declareFixSectionKinds(const std::vector<std::string>& kinds) {
        std::vector<std::string> ordered;
        for (const std::string& kind : kinds) {
            if (!kind.empty() && std::find(ordered.begin(), ordered.end(), kind) == ordered.end()) {
                ordered.push_back(kind);
            }
        }
        for (const std::string& kind : fixSectionKinds_) {
            if (std::find(ordered.begin(), ordered.end(), kind) == ordered.end()) {
                ordered.push_back(kind);
            }
        }
        fixSectionKinds_ = std::move(ordered);
    }

    GraphRemove<std::string, std::string>::GraphId TemplateFixer::fixSectionsGraphId(std::size_t group) {
        return GraphRemove<std::string, std::string>::GraphId(group, "", "fixSections");
    }

    TemplateFixer::GroupEdit* TemplateFixer::fixSectionsStep(const std::string& kind) {
        // each group's graph: the kind's sections added with no group (into the first group, or every copy too), then the
        // group's own -- the sections of no kind making the graph, each kind's added to it
        std::unique_ptr<GraphCreate<std::string, std::string>>& edit = createEdits_[kind];
        if (edit == nullptr) {
            edit = std::make_unique<GraphCreate<std::string, std::string>>(fixSectionsGraphId(0), nullptr);
            edit->runConfig = sectionRunConfig();
            edit->name = kind.empty() ? "fixSections" : kind;
            edit->append = !kind.empty();
            edit->sectionsOf = [this, kind](std::size_t group) {
                std::vector<Section*> sections;
                for (const std::string& name : group == 0 || fixSectionsInCopies
                         ? std::vector<std::string>{fixSectionsVariable(kind, std::nullopt), fixSectionsVariable(kind, group)}
                         : std::vector<std::string>{fixSectionsVariable(kind, group)}) {
                    if (const GraphEditVariables::Sections* variable = fixVariables_.findSections(name)) {
                        for (Section* section : variable->list()) {
                            sections.push_back(section);
                        }
                    }
                }
                return sections;
            };
        }

        // fixSectionsZ3 is set by the time a list is asked for: in the fixer's constructor
        edit->z3 = fixSectionsZ3;
        return edit.get();
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
