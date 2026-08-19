#include "AGRemapCore/constants/GameTypeId.h"


namespace AGRemapCore {

    std::optional<GameTypeId> GameTypeIdTools::getEnum(int value) {
        switch (value) {
            case static_cast<int>(GameTypeId::GI):
                return GameTypeId::GI;

            case static_cast<int>(GameTypeId::WuWa):
                return GameTypeId::WuWa;

            default:
                return std::nullopt;
        }
    }

}
