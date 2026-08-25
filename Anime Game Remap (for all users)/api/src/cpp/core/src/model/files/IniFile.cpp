#include "AGRemapCore/model/files/IniFile.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalIniClassifiers.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"


namespace AGRemapCore {
    IniFile::IniFile(std::optional<std::string> file, std::string txt, std::optional<int> gameTypeId,
                      std::optional<std::unordered_set<int>> filteredModTypeIds,
                      std::optional<std::unordered_set<int>> forcedModTypeIds,
                      std::optional<std::unordered_map<int, ModType>> overrideModTypes,
                      BaseIniClassifier* iniClassifier):
        file_(std::move(file)),
        gameTypeId_(gameTypeId),
        filteredModTypeIds_(std::move(filteredModTypeIds)),
        forcedModTypeIds_(std::move(forcedModTypeIds)),
        overrideModTypes_(overrideModTypes.has_value() ? std::move(*overrideModTypes) : std::unordered_map<int, ModType>()),
        iniClassifier_(iniClassifier != nullptr ? iniClassifier : &GlobalIniClassifiers::classifier()) {
        // Matches the pure-Python original's own "_setupFileLines" -- a file-backed IniFile isn't
        // read from disk until something actually asks for it (see #readFileLines); a file-less one
        // has no other source of data, so the constructor's 'txt' is used immediately.
        if (!file_.has_value()) {
            setFileTxt(std::move(txt));
        }
    }

    const std::optional<std::string>& IniFile::getFile() const {
        return file_;
    }

    const std::string& IniFile::getFileTxt() const {
        return fileTxt_;
    }

    const std::vector<std::string>& IniFile::getFileLines() const {
        return fileLines_;
    }

    bool IniFile::fileLinesRead() const {
        return fileLinesRead_;
    }

    void IniFile::setFileTxt(std::string txt) {
        fileTxt_ = std::move(txt);
        fileLines_.clear();

        // Split fileTxt_ into lines the same way Python's str.splitlines(keepends = True) does --
        // walk the text once, cutting a new line every time a '\n' is found and keeping it attached
        // to the line it ends. A trailing '\n' terminates the last line rather than starting a new,
        // empty one (eg. "abc\n" -> ["abc\n"], not ["abc\n", ""]) -- the loop below naturally does
        // this since it only ever starts a new line when there's still text left to put in it.
        size_t lineStart = 0;
        while (lineStart < fileTxt_.size()) {
            size_t newlinePos = fileTxt_.find('\n', lineStart);
            if (newlinePos == std::string::npos) {
                fileLines_.emplace_back(fileTxt_.substr(lineStart));
                break;
            }

            fileLines_.emplace_back(fileTxt_.substr(lineStart, newlinePos - lineStart + 1));
            lineStart = newlinePos + 1;
        }

        fileLinesRead_ = true;
    }

    void IniFile::readFromDisk(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + path);
        }

        std::ostringstream buf;
        buf << file.rdbuf();
        std::string txt = buf.str();

        // Normalize "\r\n"/lone "\r" line endings down to "\n", matching the universal-newline
        // translation Python's text-mode "open(path, 'r')" performs on read -- done manually (rather
        // than relying on the ifstream/OS text-mode behavior above) so this is portable and behaves
        // the same on every platform, not just Windows.
        std::string normalized;
        normalized.reserve(txt.size());
        for (size_t i = 0; i < txt.size(); ++i) {
            char c = txt[i];
            if (c == '\r') {
                normalized.push_back('\n');
                if (i + 1 < txt.size() && txt[i + 1] == '\n') {
                    ++i;
                }
            } else {
                normalized.push_back(c);
            }
        }

        setFileTxt(std::move(normalized));
    }

    const std::vector<std::string>& IniFile::readFileLines() {
        if (file_.has_value()) {
            readFromDisk(*file_);
        }
        return fileLines_;
    }

    std::optional<ModType> IniFile::getModType(int modTypeId) const {
        auto overrideIt = overrideModTypes_.find(modTypeId);
        if (overrideIt != overrideModTypes_.end()) {
            return overrideIt->second;
        }

        // ModTypeIdTools::getModType now takes the raw int directly (no more ModTypeId-enum-only
        // restriction), so a genuinely custom 'modTypeId' (one ModTypeIdTools::getEnum wouldn't
        // recognize) can be resolved via the global registry too now, not just overrideModTypes_
        // above -- as long as it was actually registered there via registerModType.
        return ModTypeIdTools::getModType(modTypeId);
    }

    void IniFile::classify() {
        if (!fileLinesRead_) {
            readFileLines();
        }

        // gameTypeId_ is kept as a plain int (see the constructor's own doc comment on why) -- the
        // classifier's API only understands a real GameTypeId, so an unrecognized custom id simply
        // can't be expressed there and falls back to "unfiltered" (std::nullopt) for classification
        // purposes.
        std::optional<GameTypeId> gameTypeIdEnum = gameTypeId_.has_value() ? GameTypeIdTools::getEnum(*gameTypeId_) : std::nullopt;

        modTypes.clear();

        if (forcedModTypeIds_.has_value()) {
            bool isFixedResult = false;
            bool isModResult = false;
            iniClassifier_->checkIsFixedMod(fileLines_, &isFixedResult, &isModResult, gameTypeIdEnum);
            isFixed = isFixedResult;
            isMod = isModResult;

            for (int modTypeId : *forcedModTypeIds_) {
                std::optional<ModType> modType = getModType(modTypeId);
                if (modType.has_value()) {
                    modTypes.emplace(modTypeId, *modType);
                }
            }

            return;
        }

        IniClassifyStats stats = iniClassifier_->classify(fileLines_, gameTypeIdEnum);
        isMod = stats.isMod;
        isFixed = stats.isFixed;

        for (const auto& entry : stats.modType) {
            int modTypeId = entry.first;
            if (filteredModTypeIds_.has_value() && !filteredModTypeIds_->contains(modTypeId)) {
                continue;
            }

            std::optional<ModType> modType = getModType(modTypeId);
            if (modType.has_value()) {
                modTypes.emplace(modTypeId, *modType);
            }
        }
    }
}
