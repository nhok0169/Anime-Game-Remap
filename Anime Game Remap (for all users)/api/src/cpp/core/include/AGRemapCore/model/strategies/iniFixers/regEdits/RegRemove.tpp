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

#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<RegRemove<K, V, KeyHash, KeyEqual>> RegRemove<K, V, KeyHash, KeyEqual>::of(K key, std::optional<RemoveKeyCheck> check) {
        return std::make_unique<RegRemove>(std::vector<std::pair<K, std::optional<RemoveKeyCheck>>>{{std::move(key), std::move(check)}});
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegRemove<K, V, KeyHash, KeyEqual>::RemoveKeyCheck RegRemove<K, V, KeyHash, KeyEqual>::valueStartsWith(std::string prefix) {
        prefix = StringTools::toLower(prefix);
        return [prefix](long long, const V& value) {
            const std::string lowered = StringTools::toLower(StringTools::lstrip(value));
            const std::string ref = StringTools::toLower(IniKeywords::Ref);
            if (StringTools::startsWith(lowered, prefix)) {
                return true;
            }
            if (!StringTools::startsWith(lowered, ref + " ")) {
                return false;
            }
            return StringTools::startsWith(StringTools::lstrip(lowered.substr(ref.size())), prefix);
        };
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegRemove<K, V, KeyHash, KeyEqual>::RegRemove(std::vector<std::pair<K, std::optional<RemoveKeyCheck>>> removeKeys):
        removeKeys(std::move(removeKeys)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegRemove<K, V, KeyHash, KeyEqual>::ContentPart& RegRemove<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;

        part.removeKeys(removeKeys, Base::toRangeSpec(partRanges));
        return part;
    }
}
