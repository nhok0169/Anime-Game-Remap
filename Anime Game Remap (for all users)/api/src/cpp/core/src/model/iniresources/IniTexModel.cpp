#include "AGRemapCore/model/iniresources/IniTexModel.h"

#include <utility>


namespace AGRemapCore {
    IniTexModel::IniTexModel(std::string iniFolderPath,
                              tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fixedPaths,
                              tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::unique_ptr<BaseTexEditor>>>> texEdits,
                              std::optional<tsl::ordered_map<int, std::vector<std::string>>> origPaths):
        IniFixResourceModel(std::move(iniFolderPath), std::move(fixedPaths), std::move(origPaths)),
        texEdits(std::move(texEdits)) {}

    void IniTexModel::clear() {
        IniFixResourceModel::clear();
        texEdits.clear();
    }
}
