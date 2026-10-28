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
    }

    std::optional<Version> TemplateFixer::iniToVersion() const {
        const IniFile* ini = ctx_.getIniFile();
        return ini == nullptr ? std::nullopt : ini->toVersion;
    }

    std::string TemplateFixer::remapFixName(const std::string& name) const {
        return IniNamingTools::getRemapFixName(name, toModName_);
    }
}
