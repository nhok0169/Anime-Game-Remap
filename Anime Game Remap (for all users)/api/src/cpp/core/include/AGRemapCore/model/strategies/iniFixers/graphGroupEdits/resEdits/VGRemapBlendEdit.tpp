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

#ifndef AGRemapCore_VGRemapBlendEdit_TPP
#define AGRemapCore_VGRemapBlendEdit_TPP

#include "VGRemapBlendEdit.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    VGRemapBlendReplace<K, V, KeyHash, KeyEqual>::VGRemapBlendReplace(
            GraphId resModObj, ResEditConfig config, const ModType* modType,
            std::optional<Version> fromVersion, std::optional<Version> toVersion, std::string resType,
            std::optional<std::string> resSubType, std::optional<std::string> fromComp, std::optional<std::string> toComp):
        Base(std::move(resModObj), std::move(config), std::move(resType), std::move(resSubType),
              std::move(fromComp), std::move(toComp)),
        modType(modType), fromVersion(std::move(fromVersion)), toVersion(std::move(toVersion)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void VGRemapBlendReplace<K, V, KeyHash, KeyEqual>::buildResModel(
            const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
            const std::string& modName, const std::string& fileKey, Context& ctx) {
        (void)resType;

        std::optional<VGRemap> vgRemap;
        if (modType != nullptr) {
            vgRemap = modType->getVGRemap(modName, fromVersion, toVersion, this->fromComp, this->toComp);
        }

        // No remap is a real answer, not a reason to write a plain copy under a remapped name --
        // see this class's own note.
        if (!vgRemap.has_value()) {
            Base::buildResModel(this->resType, srcPath, fixedPath, modName, fileKey, ctx);
            return;
        }

        auto resource = std::make_unique<RemapBlendResource>(
            ctx.iniFolder(), srcPath, fixedPath, std::move(*vgRemap), this->resType,
            std::function<bool(RemapBlendResource&)>{},
            std::vector<std::unique_ptr<BufElementType>>{});

        resource->logger = ctx.logger();
        ctx.storeResource(fileKey, std::move(resource));
    }
}

#endif
