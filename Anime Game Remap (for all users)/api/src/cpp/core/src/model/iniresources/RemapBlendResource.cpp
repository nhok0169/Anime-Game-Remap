#include "AGRemapCore/model/iniresources/RemapBlendResource.h"

#include <utility>


namespace AGRemapCore {

    namespace {
        std::vector<std::unique_ptr<BufElementType>> cloneElements(const std::vector<std::unique_ptr<BufElementType>>& elements) {
            std::vector<std::unique_ptr<BufElementType>> result;
            result.reserve(elements.size());

            for (const auto& element : elements) {
                result.push_back(std::make_unique<BufElementType>(*element));
            }

            return result;
        }
    }

    RemapBlendResource::RemapBlendResource(const std::string& iniFolderPath, const std::string& srcPath, const std::string& fixedPath,
                                            VGRemap vgRemap, std::string type, std::function<bool(RemapBlendResource&)> fixFunc,
                                            std::vector<std::unique_ptr<BufElementType>> blendElements):
        RemapIniFixResource(std::move(type), iniFolderPath, srcPath, fixedPath), vgRemap(std::move(vgRemap)),
        blendElements(std::move(blendElements)), fixFunc(std::move(fixFunc)) {}

    bool RemapBlendResource::srcEncounteredError(const RemapStats& stats) const {
        return stats.blend.skipped.contains(srcPath);
    }

    bool RemapBlendResource::srcIsFixed(const RemapStats& stats) const {
        return stats.blend.fixed.contains(srcPath);
    }

    bool RemapBlendResource::fixEncounteredError(const RemapStats& stats) const {
        return stats.blend.skipped.contains(fixedPath);
    }

    bool RemapBlendResource::fixIsFixed(const RemapStats& stats) const {
        return stats.blend.fixed.contains(fixedPath);
    }

    BlendFile RemapBlendResource::createBlend() const {
        return BlendFile(srcPath, cloneElements(blendElements));
    }

    bool RemapBlendResource::_fix() {
        BlendFile blend = createBlend();
        blend.remap(vgRemap, fixedPath);
        return true;
    }

    bool RemapBlendResource::fix() {
        if (fixFunc) {
            return fixFunc(*this);
        }

        return _fix();
    }
}
