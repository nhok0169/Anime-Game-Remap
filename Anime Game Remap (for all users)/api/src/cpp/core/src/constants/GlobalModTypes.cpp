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

#include "AGRemapCore/constants/GlobalModTypes.h"

#include <utility>

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/ModTypeId.h"


namespace AGRemapCore {

    std::vector<ModType> GlobalModTypes::all() {
        return GIBuilder::all();
    }

    void GlobalModTypes::registerMissing() {
        for (const ModType& modType : all()) {
            // Only the gap-filling half of registerAll -- see this function's own doc comment for
            // why the implicit path must not overwrite.
            if (!ModTypeIdTools::getModType(modType.modTypeId).has_value()) {
                ModTypeIdTools::registerModType(modType);
            }
        }
    }


    void GlobalModTypes::registerAll() {
        for (const ModType& modType : all()) {
            ModTypeIdTools::registerModType(modType);
        }
    }
}
