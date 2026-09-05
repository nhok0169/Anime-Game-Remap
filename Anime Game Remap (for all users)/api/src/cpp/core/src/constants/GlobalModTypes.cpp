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
