#include "AGRemapCore/model/iniresources/IniDownloadModel.h"

#include <utility>


namespace AGRemapCore {
    IniDownloadModel::IniDownloadModel(std::string iniFolderPath, tsl::ordered_map<int, std::vector<std::string>> paths,
                                        tsl::ordered_map<int, std::vector<std::unique_ptr<FileDownload>>> downloads):
        IniSrcResourceModel(std::move(iniFolderPath), std::move(paths)), downloads(std::move(downloads)) {}
}
