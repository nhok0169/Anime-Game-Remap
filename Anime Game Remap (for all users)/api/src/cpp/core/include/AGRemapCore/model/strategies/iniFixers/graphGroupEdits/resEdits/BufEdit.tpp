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

#ifndef AGRemapCore_BufEdit_TPP
#define AGRemapCore_BufEdit_TPP

#include <memory>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/tools/TextTools.h"

namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    BufReplace<K, V, KeyHash, KeyEqual>::BufReplace(GraphId resModObj, ResEditConfig config, std::string kind,
                                                     std::optional<std::string> resSubType):
        Base(resTypeOf(kind), std::move(resModObj), std::move(config)), kind(std::move(kind)), resSubType(std::move(resSubType)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string BufReplace<K, V, KeyHash, KeyEqual>::elementName(const std::string& kind) {
        if (kind == "blend") { return IniKeywords::Blend; }
        if (kind == "position") { return IniKeywords::Position; }
        if (kind == "texcoord") { return IniKeywords::Texcoord; }
        if (kind == "ib") { return "IB"; }
        throw std::invalid_argument("'" + kind + "' is not a kind of buffer (blend / position / texcoord / ib)");
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string BufReplace<K, V, KeyHash, KeyEqual>::resTypeOf(const std::string& kind) {
        elementName(kind);
        return kind == "ib" ? "buf" : kind;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string BufReplace<K, V, KeyHash, KeyEqual>::subTypedModName(const std::string& modName) const {
        std::string result = TextTools::capitalize(modName);
        if (resSubType.has_value()) {
            result += TextTools::capitalize(*resSubType);
        }
        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> BufReplace<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                        const std::string& modName) const {
        return IniNamingTools::getResourceName(IniNamingTools::getRemapElementName(resource, elementName(kind), subTypedModName(modName)));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string BufReplace<K, V, KeyHash, KeyEqual>::getFixFile(const std::string& file, const std::string& modName,
                                                                 const std::string& graphId) const {
        // A vertex buffer is always written as a .buf; an index buffer keeps whatever extension it came with
        std::optional<std::string> ext = (kind == "ib") ? std::nullopt : std::optional<std::string>(FileExt::Buf);
        std::string result = IniNamingTools::getFixedElementFile(file, elementName(kind), subTypedModName(modName), ext);
        if (graphId.empty()) {
            return result;
        }
        return Base::fileAddGraphId(result, graphId);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BufReplace<K, V, KeyHash, KeyEqual>::buildResModel(const std::string& resType, const std::string& srcPath,
                                                             const std::string& fixedPath, const std::string& modName,
                                                             const std::string& fileKey, Context& ctx) {
        (void)resType;
        (void)modName;

        // Typed by KIND, whatever the caller passed: the grouped fix tells its members apart by it
        auto resource = std::make_unique<IniFixResource>(this->resType, ctx.iniFolder(), srcPath, fixedPath);
        resource->logger = ctx.logger();
        ctx.storeResource(fileKey, std::move(resource));
    }
}

#endif
