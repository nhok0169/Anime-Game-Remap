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

#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseRegEdit<K, V, KeyHash, KeyEqual>::ContentPart& BaseRegEdit<K, V, KeyHash, KeyEqual>::editFromIni(
            ContentPart& part, const std::string& sectionName, IniFile* ini, const ModType* modType,
            const std::string& modName, const OrderRanges* partRanges) {
        // 'ini' is deliberately unused -- see this method's doc comment.
        (void)ini;
        return edit(part, sectionName, modType, modName, partRanges);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseRegEdit<K, V, KeyHash, KeyEqual>::ContentPart& BaseRegEdit<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;
        (void)partRanges;
        return part;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<typename BaseRegEdit<K, V, KeyHash, KeyEqual>::RangeSpec> BaseRegEdit<K, V, KeyHash, KeyEqual>::toRangeSpec(
            const OrderRanges* partRanges) {
        if (partRanges == nullptr) {
            return std::nullopt;
        }

        return partRanges->ranges;
    }
}
