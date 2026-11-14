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

#ifndef AGRemapCore_BufRestrideEdit_TPP
#define AGRemapCore_BufRestrideEdit_TPP

#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    BufRestrideReplace<K, V, KeyHash, KeyEqual>::BufRestrideReplace(
            GraphId resModObj, ResEditConfig config, std::string kind,
            std::vector<std::unique_ptr<BufElementType>> fromElements, std::vector<std::unique_ptr<BufElementType>> toElements,
            std::unordered_map<std::string, std::string> fillFrom, std::optional<std::string> resSubType):
        Base(std::move(resModObj), std::move(config), std::move(kind), std::move(resSubType)),
        fromElements(std::move(fromElements)), toElements(std::move(toElements)), fillFrom(std::move(fillFrom)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::unique_ptr<BufElementType>> BufRestrideReplace<K, V, KeyHash, KeyEqual>::cloneLayout(
            const std::vector<std::unique_ptr<BufElementType>>& elements) {
        std::vector<std::unique_ptr<BufElementType>> result;
        result.reserve(elements.size());
        for (const auto& element : elements) {
            result.push_back(std::make_unique<BufElementType>(*element));
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BufRestrideReplace<K, V, KeyHash, KeyEqual>::buildResModel(const std::string& resType, const std::string& srcPath,
                                                                     const std::string& fixedPath, const std::string& modName,
                                                                     const std::string& fileKey, Context& ctx) {
        (void)resType;
        (void)modName;

        // one layout copy per resource: an edit serves every .ini of the run
        auto resource = std::make_unique<RemapBufRestrideResource>(
            ctx.iniFolder(), srcPath, fixedPath, cloneLayout(fromElements), cloneLayout(toElements), fillFrom);
        resource->logger = ctx.logger();
        ctx.storeResource(fileKey, std::move(resource));
    }
}

#endif
