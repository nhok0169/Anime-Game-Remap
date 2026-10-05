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

#ifndef AGRemapCore_PositionEdit_TPP
#define AGRemapCore_PositionEdit_TPP

#include "PositionEdit.h"

#include <memory>
#include <utility>

#include "AGRemapCore/tools/TextTools.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RemapPositionReplace<K, V, KeyHash, KeyEqual>::RemapPositionReplace(GraphId resModObj, ResEditConfig config,
                                                                        std::string resType):
        Base(std::move(resType), std::move(resModObj), std::move(config)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> RemapPositionReplace<K, V, KeyHash, KeyEqual>::getFixResourceName(
            const std::string& resource, const std::string& modName) const {
        return IniNamingTools::getRemapPositionResourceName(resource, TextTools::capitalize(modName));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapPositionReplace<K, V, KeyHash, KeyEqual>::getFixFile(const std::string& file,
                                                                          const std::string& modName,
                                                                          const std::string& graphId) const {
        std::string result = IniNamingTools::getFixedPositionFile(file, TextTools::capitalize(modName));
        if (graphId.empty()) {
            return result;
        }

        return Base::fileAddGraphId(result, graphId);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    PositionEditReplace<K, V, KeyHash, KeyEqual>::PositionEditReplace(GraphId resModObj, ResEditConfig config,
                                                                      BufFile::Filter edit, std::string resType):
        Base(std::move(resModObj), std::move(config), std::move(resType)), edit(std::move(edit)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void PositionEditReplace<K, V, KeyHash, KeyEqual>::buildResModel(
            const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
            const std::string& modName, const std::string& fileKey, Context& ctx) {
        (void)resType;
        (void)modName;

        // No edit is a real answer, not a reason to write a plain copy under a remapped name -- the
        // same call VGRemapBlendReplace makes when a pair has no vertex group remap.
        if (!edit) {
            Base::buildResModel(this->resType, srcPath, fixedPath, modName, fileKey, ctx);
            return;
        }

        auto resource = std::make_unique<RemapPositionResource>(
            ctx.iniFolder(), srcPath, fixedPath, edit, this->resType,
            std::function<bool(RemapPositionResource&)>{});

        resource->logger = ctx.logger();
        ctx.storeResource(fileKey, std::move(resource));
    }
}

#endif
