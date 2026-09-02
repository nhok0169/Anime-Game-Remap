#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IniFileResEditContext.h"

#include <filesystem>

#include "AGRemapCore/model/files/IniFile.h"


namespace AGRemapCore {
    IniFileResEditContext::IniFileResEditContext(IniFile* iniFile, Collected* collected):
        iniFile_(iniFile), collected_(collected) {}


    IniFile* IniFileResEditContext::getIniFile() const {
        return iniFile_;
    }


    IniFileResEditContext::Collected* IniFileResEditContext::getCollected() const {
        return collected_;
    }


    bool IniFileResEditContext::isCollecting() const {
        return collecting_;
    }


    bool IniFileResEditContext::hasIni() const {
        return iniFile_ != nullptr;
    }


    std::string IniFileResEditContext::iniFolder() const {
        if (!hasIni() || !iniFile_->getFile().has_value()) {
            return "";
        }

        // Derived rather than stored, the same way IniFileRemoveContext does it -- an
        // AGRemapCore::IniFile keeps only its path, with no FilePath object to ask for a folder.
        return std::filesystem::path(*iniFile_->getFile()).parent_path().string();
    }


    void IniFileResEditContext::storeResource(const std::string& fileKey, std::unique_ptr<IniResource> resource) {
        if (resource == nullptr) {
            return;
        }

        // Checked in the same order the pure-Python original checks them -- see this class's own
        // note. A capture pass wins over a collect map, which wins over the .ini file.
        if (collecting_) {
            IniResource* raw = resource.get();
            captureKeepAlive_.push_back(std::move(resource));
            buffer_.emplace_back(fileKey, raw);
            return;
        }

        if (collected_ != nullptr) {
            (*collected_)[fileKey].push_back(std::move(resource));
            return;
        }

        // No .ini file to hand it to and nowhere else to put it: dropped rather than leaked, which
        // is what the original does too (its own branch is guarded on "self._iniFile is not None").
        if (!hasIni()) {
            return;
        }

        // The file key goes unused here, exactly as the original's "ini.resources.append(...)"
        // ignores it -- this is a flat list, not a keyed one.
        iniFile_->getResources().push_back(std::move(resource));
    }


    void IniFileResEditContext::beginCollectingResources() {
        collecting_ = true;
        buffer_.clear();
    }


    std::vector<std::pair<std::string, IniResource*>> IniFileResEditContext::takeCollectedResources() {
        std::vector<std::pair<std::string, IniResource*>> result;
        result.swap(buffer_);

        // 'captureKeepAlive_' is deliberately NOT touched: the pointers just handed out have to stay
        // valid, and this context is what keeps them so -- see this class's own danger note.
        return result;
    }


    void IniFileResEditContext::endCollectingResources() {
        collecting_ = false;
        buffer_.clear();
    }


    std::unordered_map<std::string, IniFileResEditContext::Section*> IniFileResEditContext::sectionIfTemplates() const {
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


    Z3Context* IniFileResEditContext::z3Ctx() const {
        if (!hasIni()) {
            return nullptr;
        }

        return iniFile_->getZ3Ctx();
    }
}
