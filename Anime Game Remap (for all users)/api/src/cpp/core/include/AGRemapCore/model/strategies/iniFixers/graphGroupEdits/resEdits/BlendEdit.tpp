#ifndef AGRemapCore_BlendEdit_TPP
#define AGRemapCore_BlendEdit_TPP

#include <utility>

#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/tools/TextTools.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RemapBlendReplace<K, V, KeyHash, KeyEqual>::RemapBlendReplace(GraphId resModObj, ResEditConfig config, std::string resType,
                                                                   std::optional<std::string> resSubType,
                                                                   std::optional<std::string> fromComp,
                                                                   std::optional<std::string> toComp):
        Base(std::move(resType), std::move(resModObj), std::move(config)),
        resSubType(std::move(resSubType)), fromComp(std::move(fromComp)), toComp(std::move(toComp)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapBlendReplace<K, V, KeyHash, KeyEqual>::subTypedModName(const std::string& modName) const {
        std::string result = TextTools::capitalize(modName);
        if (resSubType.has_value()) {
            result += TextTools::capitalize(*resSubType);
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> RemapBlendReplace<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                               const std::string& modName) const {
        return IniNamingTools::getRemapBlendResourceName(resource, subTypedModName(modName));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string RemapBlendReplace<K, V, KeyHash, KeyEqual>::getFixFile(const std::string& file, const std::string& modName,
                                                                        const std::string& graphId) const {
        std::string result = IniNamingTools::getFixedBlendFile(file, subTypedModName(modName));
        if (graphId.empty()) {
            return result;
        }

        return Base::fileAddGraphId(result, graphId);
    }
}

#endif
