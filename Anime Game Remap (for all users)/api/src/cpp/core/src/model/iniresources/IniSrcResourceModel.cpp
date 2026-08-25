#include "AGRemapCore/model/iniresources/IniSrcResourceModel.h"

#include <utility>

#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    IniSrcResourceModel::IniSrcResourceModel(std::string iniFolderPath, tsl::ordered_map<int, std::vector<std::string>> paths):
        IniResourceModel(std::move(iniFolderPath)), paths(std::move(paths)) {
        for (const auto& entry : this->paths) {
            std::vector<std::string> resolved;
            resolved.reserve(entry.second.size());

            for (const std::string& path : entry.second) {
                resolved.push_back(FileService::absPathOfRelPath(path, this->iniFolderPath));
            }

            fullPaths.emplace(entry.first, std::move(resolved));
        }
    }

    std::vector<std::pair<std::string, std::string>> IniSrcResourceModel::items() const {
        std::vector<std::pair<std::string, std::string>> result;

        for (const auto& entry : paths) {
            const std::vector<std::string>& partPaths = entry.second;
            const std::vector<std::string>& partFullPaths = fullPaths.at(entry.first);

            for (size_t i = 0; i < partPaths.size(); ++i) {
                result.emplace_back(partPaths[i], partFullPaths[i]);
            }
        }

        return result;
    }
}
