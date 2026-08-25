#include "AGRemapCore/model/iniresources/RemapTexResource.h"

#include <filesystem>
#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"


namespace AGRemapCore {
    RemapTexAddResource::RemapTexAddResource(const std::string& iniFolderPath, const std::string& srcPath, TexCreator texCreator,
                                              std::string type, std::function<bool(RemapTexAddResource&)> fixFunc):
        RemapIniResource(std::move(type), iniFolderPath, srcPath), texCreator(std::move(texCreator)), fixFunc(std::move(fixFunc)) {}

    bool RemapTexAddResource::srcEncounteredError(const RemapStats& stats) const {
        return stats.texAdd.skipped.contains(srcPath);
    }

    bool RemapTexAddResource::srcIsFixed(const RemapStats& stats) const {
        return stats.texAdd.fixed.contains(srcPath);
    }

    bool RemapTexAddResource::fixEncounteredError(const RemapStats& stats) const {
        return srcEncounteredError(stats);
    }

    bool RemapTexAddResource::fixIsFixed(const RemapStats& stats) const {
        return srcIsFixed(stats);
    }

    bool RemapTexAddResource::fixExists(const RemapStats& stats) const {
        (void)stats;
        return std::filesystem::exists(srcPath);
    }

    bool RemapTexAddResource::_fix() {
        TextureFile texture(srcPath);
        texCreator.fix(texture, srcPath);
        return true;
    }

    bool RemapTexAddResource::fix() {
        if (fixFunc) {
            return fixFunc(*this);
        }

        return _fix();
    }
}
