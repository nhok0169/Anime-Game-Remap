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
        // "Creating", not "Editting": this class makes a texture from nothing, and has no fixedPath
        // at all -- srcPath is both source and destination (see this class's own note). Naming the
        // file it writes keeps it consistent with its editing sibling.
        if (logger != nullptr) {
            logger->log("Creating texture for " + std::filesystem::path(srcPath).filename().string());
        }

        if (fixFunc) {
            return fixFunc(*this);
        }

        return _fix();
    }

    RemapTexEditResource::RemapTexEditResource(const std::string& iniFolderPath, const std::string& srcPath,
                                                const std::string& fixedPath, TexEditor texEditor, std::string type,
                                                std::function<bool(RemapTexEditResource&)> fixFunc):
        RemapIniFixResource(std::move(type), iniFolderPath, srcPath, fixedPath), texEditor(std::move(texEditor)),
        fixFunc(std::move(fixFunc)) {}

    bool RemapTexEditResource::srcEncounteredError(const RemapStats& stats) const {
        return stats.texEdit.skipped.contains(srcPath);
    }

    bool RemapTexEditResource::srcIsFixed(const RemapStats& stats) const {
        return stats.texEdit.fixed.contains(srcPath);
    }

    bool RemapTexEditResource::fixEncounteredError(const RemapStats& stats) const {
        return stats.texEdit.skipped.contains(fixedPath);
    }

    bool RemapTexEditResource::fixIsFixed(const RemapStats& stats) const {
        return stats.texEdit.fixed.contains(fixedPath);
    }

    bool RemapTexEditResource::_fix() {
        TextureFile texture(srcPath);
        texEditor.fix(texture, fixedPath);
        return true;
    }

    bool RemapTexEditResource::fix() {
        // See RemapBlendResource::fix for why this goes before the work rather than after.
        if (logger != nullptr) {
            logger->log("Editting texture for " + std::filesystem::path(fixedPath).filename().string());
        }

        if (fixFunc) {
            return fixFunc(*this);
        }

        return _fix();
    }
}
