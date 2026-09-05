#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"


namespace AGRemapCore {
    namespace {

        // Taken from the *parser's* defaults, not the fixer's: GIMIFixer::FixerConfig carries only a
        // section renderer, and the graphs a fixer's groups hold are copies of ones a parser built,
        // so they have to follow the same "run =" edges those did.
        IfTemplateRunConfig<std::string, std::string> fixRunConfig() {
            return GIMIParser<>::defaultConfig().runConfig;
        }
    }


    IniFileFixContext::IniFileFixContext(IniFile* iniFile, std::optional<int> modTypeId,
                                          std::optional<std::string> header, std::optional<std::string> footer):
        RemapIniFixContext<std::string, std::string>(std::move(header), std::move(footer)),
        iniFile_(iniFile), modTypeId_(std::move(modTypeId)) {}


    IniFile* IniFileFixContext::getIniFile() const {
        return iniFile_;
    }


    const std::optional<int>& IniFileFixContext::getModTypeId() const {
        return modTypeId_;
    }


    void IniFileFixContext::setModTypeId(std::optional<int> modTypeId) {
        modTypeId_ = std::move(modTypeId);
    }


    const std::vector<std::string>& IniFileFixContext::getLogs() const {
        return logs_;
    }


    const ModType* IniFileFixContext::modType() const {
        if (!hasIni() || !modTypeId_.has_value()) {
            return nullptr;
        }

        const tsl::ordered_map<int, ModType>& modTypes = iniFile_->getModTypes();
        auto it = modTypes.find(*modTypeId_);

        if (it == modTypes.end()) {
            return nullptr;
        }

        return &it->second;
    }


    std::optional<std::string> IniFileFixContext::modTypeName() const {
        const ModType* type = modType();
        if (type == nullptr) {
            return std::nullopt;
        }

        return type->name;
    }


    bool IniFileFixContext::hasIni() const {
        return iniFile_ != nullptr;
    }


    std::vector<std::string> IniFileFixContext::modsToFix() const {
        // Empty, and not a stub: this is the equivalent of the original's
        // "ini.availableType.getModsToFix()", which reads ModMappedAssets.fixTo -- a set the
        // pure-Python original declares and never populates anywhere, so it is always empty there
        // too (see PyModMappedAssets' own note on it). A caller that knows better sets
        // GIMIFixer::modsToFix explicitly, which is what IniFile::fix's own fixers do.
        return {};
    }


    std::optional<std::string> IniFileFixContext::fixedFilePath(std::size_t groupInd) const {
        if (!hasIni() || !iniFile_->getFile().has_value()) {
            return std::nullopt;
        }

        std::filesystem::path path(*iniFile_->getFile());
        if (groupInd == 0) {
            return path.string();
        }

        // Group 0 is the .ini file's own path; every later group is a copy, named by appending the
        // RemapFix suffix and the index to the base name -- the equivalent of the pure-Python
        // original mutating a deep copy of its FilePath's baseName as it walks the groups.
        std::filesystem::path copy = path.parent_path() /
            (path.stem().string() + IniKeywords::RemapFix + std::to_string(groupInd) + path.extension().string());

        return copy.string();
    }


    bool IniFileFixContext::fixedFileExists() const {
        if (!hasIni() || !iniFile_->getFile().has_value()) {
            return false;
        }

        std::error_code err;
        bool result = std::filesystem::exists(*iniFile_->getFile(), err);
        return result && !err;
    }


    std::string IniFileFixContext::fileTxt() const {
        if (!hasIni()) {
            return "";
        }

        return iniFile_->getFileTxt();
    }


    void IniFileFixContext::setFileTxt(std::string txt) {
        if (!hasIni()) {
            return;
        }

        iniFile_->setFileTxt(std::move(txt));
    }


    void IniFileFixContext::disableIni() {
        if (!hasIni()) {
            return;
        }

        iniFile_->disableIni();
    }


    void IniFileFixContext::log(const std::string& message) {
        // Still kept -- MultiModFixer reads these back through getLogs, so the buffer is not just a
        // fallback. But an AGRemapCore::IniFile DOES have somewhere to print to now (its own
        // optional logger), so a line goes to both rather than only being stored.
        logs_.push_back(message);

        if (hasIni() && iniFile_->logger != nullptr) {
            iniFile_->logger->log(message);
        }
    }


    void IniFileFixContext::writeFixedFile(const std::string& path, const std::string& content) {
        // Binary mode and an explicit truncate, matching IniFile::write's own: the newline
        // normalization this codebase does is its own, so letting the OS re-translate a written
        // newline would make a written-then-read round trip lossy on Windows and not on Linux.
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Unable to open file for writing: " + path);
        }

        out.write(content.data(), static_cast<std::streamsize>(content.size()));
    }


    void IniFileFixContext::setIsFixed(bool isFixed) {
        // Deliberately nothing, exactly as IniFileRemoveContext::setIsFixed does and for the same
        // reason: IniFile::isFixed is protected and owned by IniFile::classify. See
        // IniFixContext::setIsFixed's own note, which says an implementation is free to do nothing.
        (void)isFixed;
    }


    std::unique_ptr<IniFileFixContext::GraphGroups> IniFileFixContext::makeGraphGroups() {
        groupStorages_.push_back(std::make_unique<std::vector<IniGraphGroup<std::string, std::string>>>());
        return std::make_unique<Groups>(*groupStorages_.back(), fixRunConfig());
    }
}
