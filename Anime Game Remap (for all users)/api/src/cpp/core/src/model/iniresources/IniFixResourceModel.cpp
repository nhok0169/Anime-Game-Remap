#include "AGRemapCore/model/iniresources/IniFixResourceModel.h"

#include <utility>

#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    IniFixResourceModel::IniFixResourceModel(std::string iniFolderPath,
                                              tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fixedPaths,
                                              std::optional<tsl::ordered_map<int, std::vector<std::string>>> origPaths):
        IniResourceModel(std::move(iniFolderPath)), fixedPaths(std::move(fixedPaths)), origPaths(std::move(origPaths)) {
        for (const auto& partEntry : this->fixedPaths) {
            tsl::ordered_map<std::string, std::vector<std::string>> resolvedModPaths;

            for (const auto& modEntry : partEntry.second) {
                std::vector<std::string> resolved;
                resolved.reserve(modEntry.second.size());

                for (const std::string& path : modEntry.second) {
                    resolved.push_back(FileService::absPathOfRelPath(path, this->iniFolderPath));
                }

                resolvedModPaths.emplace(modEntry.first, std::move(resolved));
            }

            fullPaths.emplace(partEntry.first, std::move(resolvedModPaths));
        }

        if (this->origPaths.has_value()) {
            for (const auto& partEntry : *this->origPaths) {
                std::vector<std::string> resolved;
                resolved.reserve(partEntry.second.size());

                for (const std::string& path : partEntry.second) {
                    resolved.push_back(FileService::absPathOfRelPath(path, this->iniFolderPath));
                }

                origFullPaths.emplace(partEntry.first, std::move(resolved));
            }
        }
    }

    std::vector<IniFixResourceModel::Entry> IniFixResourceModel::items() const {
        std::vector<Entry> result;

        for (const auto& partEntry : fixedPaths) {
            int ifTemplateInd = partEntry.first;
            const tsl::ordered_map<std::string, std::vector<std::string>>& modPaths = partEntry.second;
            const tsl::ordered_map<std::string, std::vector<std::string>>& modFullPaths = fullPaths.at(ifTemplateInd);

            // Original (unfixed) paths are keyed only by IfContentPart index, not by mod name --
            // the i-th fixed path for a given mod name lines up positionally with the i-th entry
            // of this same IfContentPart's flat orig-path list. Resolved once per part below,
            // rather than re-looked-up per mod name.
            const std::vector<std::string>* origPartPaths = nullptr;
            const std::vector<std::string>* origPartFullPaths = nullptr;
            if (origPaths.has_value()) {
                auto origIt = origPaths->find(ifTemplateInd);
                if (origIt != origPaths->end()) {
                    origPartPaths = &origIt.value();
                    origPartFullPaths = &origFullPaths.at(ifTemplateInd);
                }
            }

            for (const auto& modEntry : modPaths) {
                const std::string& modName = modEntry.first;
                const std::vector<std::string>& partPaths = modEntry.second;
                const std::vector<std::string>& partFullPaths = modFullPaths.at(modName);

                for (size_t i = 0; i < partPaths.size(); ++i) {
                    Entry entry;
                    entry.fixedPath = partPaths[i];
                    entry.fullPath = partFullPaths[i];

                    // Matches the Python original's own bounds behavior loosely (it only guards
                    // against a missing IfContentPart index, via "except KeyError" -- an
                    // out-of-range 'i' there would actually raise an uncaught IndexError). Here,
                    // both the missing-key and out-of-range cases are treated the same way (no
                    // orig data for this entry), which is a defensive, not a behavior-preserving,
                    // choice -- there's no meaningful "expected" output to preserve for what would
                    // otherwise be a crash.
                    if (origPartPaths != nullptr && i < origPartPaths->size()) {
                        entry.origPath = (*origPartPaths)[i];
                        entry.origFullPath = (*origPartFullPaths)[i];
                    }

                    result.push_back(std::move(entry));
                }
            }
        }

        return result;
    }

    void IniFixResourceModel::clear() {
        fixedPaths.clear();
        fullPaths.clear();
        origFullPaths.clear();

        if (origPaths.has_value()) {
            origPaths->clear();
        }
    }
}
