#include "AGRemapCore/constants/GlobalModTypes.h"

#include <utility>

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/ModTypeId.h"


namespace AGRemapCore {

    std::vector<ModType> GlobalModTypes::all() {
        return GIBuilder::all();
    }

    void GlobalModTypes::registerAll() {
        for (const ModType& modType : all()) {
            ModTypeIdTools::registerModType(modType);
        }
    }
}
