#ifndef AGRemapCore_TexEdit_TPP
#define AGRemapCore_TexEdit_TPP

#include <utility>

#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/tools/TextTools.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    TexCreate<K, V, KeyHash, KeyEqual>::TexCreate(GraphId resModObj, std::string texName, ResEditConfig config, std::string resType):
        Base(std::move(resType), std::move(resModObj), std::move(config)), texName(std::move(texName)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void TexCreate<K, V, KeyHash, KeyEqual>::clear() {
        texInd_ = 0;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> TexCreate<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                       const std::string& modName) const {
        // 'resource' is deliberately discarded: a created texture has no original resource name to
        // build on, so the name comes entirely from the mod being fixed to and the texture type.
        (void)resource;

        std::string result = TextTools::capitalize(modName) + texName;

        // The first texture gets no suffix at all -- matching the pure-Python original's own
        // "if (self._texInd):" truthiness check on a zero-based counter.
        if (texInd_ != 0) {
            result += std::to_string(texInd_);
        }

        result = IniNamingTools::getRemapTexResourceName(result);
        ++texInd_;
        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string TexCreate<K, V, KeyHash, KeyEqual>::getFixFile(const std::string& file, const std::string& modName,
                                                                const std::string& graphId) const {
        (void)modName;

        std::string result = IniNamingTools::getFixedTexFile(file);
        if (graphId.empty()) {
            return result;
        }

        // 'file', not 'result' -- see this method's own note. Preserved from the pure-Python
        // original rather than corrected.
        return Base::fileAddGraphId(file, graphId);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    int TexCreate<K, V, KeyHash, KeyEqual>::texInd() const {
        return texInd_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    TexReplace<K, V, KeyHash, KeyEqual>::TexReplace(GraphId resModObj, ResEditConfig config, std::string resType,
                                                     std::optional<std::string> resSubType):
        Base(std::move(resType), std::move(resModObj), std::move(config)), resSubType(std::move(resSubType)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string TexReplace<K, V, KeyHash, KeyEqual>::subTypedModName(const std::string& modName) const {
        std::string result = TextTools::capitalize(modName);
        if (resSubType.has_value()) {
            result += TextTools::capitalize(*resSubType);
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> TexReplace<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                         const std::string& modName) const {
        return IniNamingTools::getRemapTexResourceName(resource, subTypedModName(modName));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string TexReplace<K, V, KeyHash, KeyEqual>::getFixFile(const std::string& file, const std::string& modName,
                                                                 const std::string& graphId) const {
        (void)modName;

        std::string result = IniNamingTools::getFixedTexFile(file);
        if (graphId.empty()) {
            return result;
        }

        return Base::fileAddGraphId(result, graphId);
    }
}

#endif
