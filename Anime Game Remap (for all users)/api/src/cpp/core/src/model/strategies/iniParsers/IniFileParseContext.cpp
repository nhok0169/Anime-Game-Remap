#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"

#include <filesystem>
#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"


namespace AGRemapCore {
    namespace {

        // The same domain customization points GIMIParser::defaultConfig builds for a plain
        // std::string parser -- taken from there rather than spelled again, so a graph this context
        // creates follows exactly the edges the parser that asked for it would have followed.
        IfTemplateRunConfig<std::string, std::string> parseRunConfig() {
            return GIMIParser<>::defaultConfig().runConfig;
        }
    }


    IniFileParseContext::IniFileParseContext(IniFile* iniFile, std::optional<int> modTypeId):
        iniFile_(iniFile), modTypeId_(std::move(modTypeId)) {
        rebuildGroups();
    }


    void IniFileParseContext::rebuildGroups() {
        // A view over groupStorage_, so it is rebuilt rather than reused whenever that vector is
        // replaced wholesale (see takeGroups).
        groups_ = std::make_unique<Groups>(groupStorage_, parseRunConfig());
    }


    IniFile* IniFileParseContext::getIniFile() const {
        return iniFile_;
    }


    const std::optional<int>& IniFileParseContext::getModTypeId() const {
        return modTypeId_;
    }


    void IniFileParseContext::setModTypeId(std::optional<int> modTypeId) {
        modTypeId_ = std::move(modTypeId);
    }


    std::vector<IniGraphGroup<std::string, std::string>> IniFileParseContext::takeGroups() {
        std::vector<IniGraphGroup<std::string, std::string>> result;
        result.swap(groupStorage_);

        rebuildGroups();
        return result;
    }


    bool IniFileParseContext::hasIni() const {
        return iniFile_ != nullptr;
    }


    std::string IniFileParseContext::iniFolder() const {
        if (!hasIni() || !iniFile_->getFile().has_value()) {
            return "";
        }

        // Derived rather than stored, the same way IniFileRemoveContext does it.
        return std::filesystem::path(*iniFile_->getFile()).parent_path().string();
    }


    std::optional<Version> IniFileParseContext::version() const {
        if (!hasIni()) {
            return std::nullopt;
        }

        return iniFile_->fromVersion;
    }


    DownloadMode IniFileParseContext::downloadMode() const {
        if (!hasIni()) {
            return DownloadMode::Normal;
        }

        return iniFile_->downloadMode;
    }


    Z3Context* IniFileParseContext::z3Ctx() const {
        if (!hasIni()) {
            return nullptr;
        }

        return iniFile_->getZ3Ctx();
    }


    std::unordered_map<std::string, IniFileParseContext::Section*> IniFileParseContext::sectionIfTemplates() const {
        std::unordered_map<std::string, Section*> result;
        if (!hasIni()) {
            return result;
        }

        for (const auto& entry : iniFile_->getIfTemplates()) {
            if (entry.second != nullptr) {
                result.emplace(entry.first, entry.second.get());
            }
        }

        return result;
    }


    std::vector<std::string> IniFileParseContext::sectionNames() const {
        if (!hasIni()) {
            return {};
        }

        // Declaration-ordered, which is the whole reason this is separate from sectionIfTemplates
        // -- see IniParseContext::sectionNames and IniFile::getSectionNames.
        return iniFile_->getSectionNames();
    }


    IniFileParseContext::Section* IniFileParseContext::getSection(const std::string& name) const {
        if (!hasIni()) {
            return nullptr;
        }

        return iniFile_->getSection(name);
    }


    IniFileParseContext::Section* IniFileParseContext::addSection(const std::string& name, std::unique_ptr<Section> section) {
        // Dropped rather than leaked when there is no .ini file to own it -- there is nowhere for a
        // synthesized section to live, and handing back a pointer into a destroyed object would be
        // worse than handing back nothing.
        if (!hasIni()) {
            return nullptr;
        }

        return iniFile_->addSection(name, std::move(section));
    }


    void IniFileParseContext::removeSection(const std::string& name) {
        if (!hasIni()) {
            return;
        }

        iniFile_->removeSection(name);
    }


    void IniFileParseContext::addFileDownload(std::unique_ptr<IniResource> download) {
        if (!hasIni() || download == nullptr) {
            return;
        }

        // The equivalent of the pure-Python original's "ini.fileDownloads.append(...)".
        iniFile_->getFileDownloads().push_back(std::move(download));
    }


    const ModType* IniFileParseContext::modType() const {
        if (!hasIni() || !modTypeId_.has_value()) {
            return nullptr;
        }

        // getModTypes() rather than IniFile::getModType, which is private -- and this wants the
        // .ini file's own ModType anyway, not a copy of it.
        const tsl::ordered_map<int, ModType>& modTypes = iniFile_->getModTypes();
        auto it = modTypes.find(*modTypeId_);

        if (it == modTypes.end()) {
            return nullptr;
        }

        return &it->second;
    }


    bool IniFileParseContext::hasModType() const {
        return modType() != nullptr;
    }


    void IniFileParseContext::log(const std::string& message) {
        // Straight through to the .ini file's own view, and dropped when it has none. Unlike
        // IniFileFixContext::log this buffers nothing: a parser's narration is progress, only
        // meaningful as it happens, whereas the fixer's lines are also read back by MultiModFixer.
        if (!hasIni() || iniFile_->logger == nullptr) {
            return;
        }

        iniFile_->logger->log(message);
    }


    std::string IniFileParseContext::modTypeName() const {
        const ModType* type = modType();
        if (type == nullptr) {
            return "";
        }

        return type->name;
    }


    IniFileParseContext::Assets* IniFileParseContext::modTypeHashes() const {
        const ModType* type = modType();
        if (type == nullptr) {
            return nullptr;
        }

        return type->hashes.get();
    }


    IniFileParseContext::Assets* IniFileParseContext::modTypeIndices() const {
        const ModType* type = modType();
        if (type == nullptr) {
            return nullptr;
        }

        return type->indices.get();
    }


    IniFileParseContext::GraphGroups& IniFileParseContext::graphGroups() {
        return *groups_;
    }
}
