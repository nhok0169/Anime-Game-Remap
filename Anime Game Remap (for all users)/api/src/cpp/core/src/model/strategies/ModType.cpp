#include "AGRemapCore/model/strategies/ModType.h"

#include "AGRemapCore/constants/ModTypeId.h"


namespace AGRemapCore {
    ModType::ModType(int gameTypeId, int modTypeId): gameTypeId(gameTypeId) {
        setModTypeId(modTypeId);
    }

    int ModType::getModTypeId() const {
        return modTypeId;
    }

    void ModType::setModTypeId(int newModTypeId) {
        modTypeId = newModTypeId;
        std::optional<ModTypeId> modTypeIdEnum = ModTypeIdTools::getEnum(modTypeId);
        name = modTypeIdEnum.has_value() ? ModTypeIdTools::getName(*modTypeIdEnum) : "";
    }
}
