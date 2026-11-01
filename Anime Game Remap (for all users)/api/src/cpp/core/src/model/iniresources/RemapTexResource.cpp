// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/model/iniresources/RemapTexResource.h"

#include <filesystem>
#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"


namespace AGRemapCore {
    namespace {
        // Runs 'fix' with a TexEditor's or TexCreator's compression off for its duration when
        // 'compressAllowed' is false, and puts the editor's own setting back however 'fix' ends
        template <typename Fn>
        void withCompression(BaseTexEditor* editor, bool compressAllowed, Fn&& fix) {
            TexEditor* texEditor = compressAllowed ? nullptr : dynamic_cast<TexEditor*>(editor);
            TexCreator* texCreator = compressAllowed ? nullptr : dynamic_cast<TexCreator*>(editor);
            const bool was = texEditor != nullptr ? texEditor->getCompress() : (texCreator != nullptr && texCreator->compress);
            const auto set = [&](bool compress) {
                if (texEditor != nullptr) {
                    texEditor->setCompress(compress);
                } else if (texCreator != nullptr) {
                    texCreator->compress = compress;
                }
            };

            set(false);
            try {
                fix();
            } catch (...) {
                set(was);
                throw;
            }
            set(was);
        }
    }

    RemapTexAddResource::RemapTexAddResource(const std::string& iniFolderPath, const std::string& srcPath, std::shared_ptr<BaseTexEditor> texCreator,
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
        return std::filesystem::exists(FileService::strToPath(srcPath));
    }

    bool RemapTexAddResource::_fix() {
        if (texCreator == nullptr) {
            return true;
        }

        TextureFile texture(srcPath);
        texture.setCache(texCache);
        withCompression(texCreator.get(), compressAllowed, [&] { texCreator->fix(texture, srcPath); });
        return true;
    }

    bool RemapTexAddResource::fix() {
        // "Creating", not "Editting": this class makes a texture from nothing, and has no fixedPath
        // at all -- srcPath is both source and destination (see this class's own note). Naming the
        // file it writes keeps it consistent with its editing sibling.
        if (logger != nullptr) {
            logger->log("Creating texture for " + FileService::pathToStr(FileService::strToPath(srcPath).filename()));
        }

        if (fixFunc) {
            return fixFunc(*this);
        }

        return _fix();
    }

    RemapTexEditResource::RemapTexEditResource(const std::string& iniFolderPath, const std::string& srcPath,
                                                const std::string& fixedPath, std::shared_ptr<BaseTexEditor> texEditor, std::string type,
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
        if (texEditor == nullptr) {
            return true;
        }

        TextureFile texture(srcPath);
        texture.setCache(texCache);
        withCompression(texEditor.get(), compressAllowed, [&] { texEditor->fix(texture, fixedPath); });
        return true;
    }

    bool RemapTexEditResource::fix() {
        // See RemapBlendResource::fix for why this goes before the work rather than after.
        if (logger != nullptr) {
            logger->log("Editting texture for " + FileService::pathToStr(FileService::strToPath(fixedPath).filename()));
        }

        if (fixFunc) {
            return fixFunc(*this);
        }

        return _fix();
    }
}
