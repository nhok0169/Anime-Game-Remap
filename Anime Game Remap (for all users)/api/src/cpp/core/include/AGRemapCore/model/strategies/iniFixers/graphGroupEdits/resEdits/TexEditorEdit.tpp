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

#ifndef AGRemapCore_TexEditorEdit_TPP
#define AGRemapCore_TexEditorEdit_TPP

#include <memory>
#include <utility>

#include "TexEditorEdit.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    TexEditorReplace<K, V, KeyHash, KeyEqual>::TexEditorReplace(
            GraphId resModObj, TexEditor texEditor, ResEditConfig config, std::string resType,
            std::optional<std::string> resSubType):
        Base(std::move(resModObj), std::move(config), std::move(resType), std::move(resSubType)),
        texEditor(std::move(texEditor)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void TexEditorReplace<K, V, KeyHash, KeyEqual>::buildResModel(
            const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
            const std::string& modName, const std::string& fileKey, Context& ctx) {
        (void)resType;
        (void)modName;

        auto resource = std::make_unique<RemapTexEditResource>(
            ctx.iniFolder(), srcPath, fixedPath, texEditor, this->resType,
            std::function<bool(RemapTexEditResource&)>{});

        resource->logger = ctx.logger();
        ctx.storeResource(fileKey, std::move(resource));
    }
}

#endif
