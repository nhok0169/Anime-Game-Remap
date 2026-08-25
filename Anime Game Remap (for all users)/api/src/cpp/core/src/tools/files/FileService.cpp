#include "AGRemapCore/tools/files/FileService.h"

#include <filesystem>


namespace AGRemapCore {
    std::string FileService::absPathOfRelPath(const std::string& dstPath, const std::string& relFolder) {
        std::filesystem::path path(dstPath);

        if (path.is_absolute()) {
            return path.lexically_normal().string();
        }

        std::filesystem::path absFolder = std::filesystem::absolute(std::filesystem::path(relFolder));
        return (absFolder / path).lexically_normal().string();
    }
}
