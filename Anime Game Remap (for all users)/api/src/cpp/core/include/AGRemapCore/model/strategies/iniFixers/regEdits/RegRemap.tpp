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

#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegRemap<K, V, KeyHash, KeyEqual>::RegRemap(std::vector<std::pair<K, KeyRemapValue>> keyRemap):
        keyRemap(std::move(keyRemap)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegRemap<K, V, KeyHash, KeyEqual>::ContentPart& RegRemap<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;

        part.remapKeys(keyRemap, Base::toRangeSpec(partRanges));
        return part;
    }
}
