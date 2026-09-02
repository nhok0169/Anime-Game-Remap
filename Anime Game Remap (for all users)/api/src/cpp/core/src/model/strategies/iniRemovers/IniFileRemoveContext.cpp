#include "AGRemapCore/model/strategies/iniRemovers/IniFileRemoveContext.h"

#include <filesystem>
#include <utility>

#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"


namespace AGRemapCore {
    IniFileRemoveContext::IniFileRemoveContext(IniFile* iniFile): iniFile_(iniFile) {}


    IniFile* IniFileRemoveContext::getIniFile() const {
        return iniFile_;
    }


    bool IniFileRemoveContext::hasIni() const {
        return iniFile_ != nullptr;
    }


    std::string IniFileRemoveContext::iniFolder() const {
        if (!hasIni() || !iniFile_->getFile().has_value()) {
            return "";
        }

        return std::filesystem::path(*iniFile_->getFile()).parent_path().string();
    }


    std::optional<Version> IniFileRemoveContext::version() const {
        if (!hasIni()) {
            return std::nullopt;
        }

        return iniFile_->fromVersion;
    }


    std::vector<IniFileRemoveContext::Assets*> IniFileRemoveContext::modTypeHashes() const {
        std::vector<Assets*> result;
        if (!hasIni()) {
            return result;
        }

        // Classified lazily here rather than by the remover: see IniRemoveContext's own note on why
        // there is no classify() on that interface. IniFile::removeFix has already done this by the
        // time it reaches a remover, so this only fires for a caller driving one directly.
        if (!iniFile_->isClassified()) {
            iniFile_->classify();
        }

        for (const std::pair<const int, ModType>& entry : iniFile_->getModTypes()) {
            if (entry.second.hashes != nullptr) {
                result.push_back(entry.second.hashes.get());
            }
        }

        return result;
    }


    std::vector<std::string> IniFileRemoveContext::readFileLines() {
        if (!hasIni()) {
            return {};
        }

        if (!iniFile_->fileLinesRead()) {
            iniFile_->readFileLines();
        }

        return iniFile_->getFileLines();
    }


    std::unordered_map<std::string, IniFileRemoveContext::Section*> IniFileRemoveContext::sectionIfTemplates() const {
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


    std::string IniFileRemoveContext::fileTxt() const {
        if (!hasIni()) {
            return "";
        }

        return iniFile_->getFileTxt();
    }


    void IniFileRemoveContext::setFileTxt(std::string txt) {
        if (!hasIni()) {
            return;
        }

        iniFile_->setFileTxt(std::move(txt));
    }


    std::string IniFileRemoveContext::write() {
        if (!hasIni()) {
            return "";
        }

        return iniFile_->write();
    }


    void IniFileRemoveContext::clearRead() {
        if (!hasIni()) {
            return;
        }

        iniFile_->clearRead();
    }


    void IniFileRemoveContext::setIsFixed(bool isFixed) {
        // Deliberately nothing -- see IniRemoveContext::setIsFixed's own note. IniFile::isFixed is
        // protected and owned by IniFile::classify; re-classify the file if the flag matters after a
        // removal.
        (void)isFixed;
    }
}
