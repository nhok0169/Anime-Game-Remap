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

    std::string GameTypeIdTools::getName(GameTypeId value) {
        switch (value) {
            case GameTypeId::GI:
                return "GI";

            case GameTypeId::WuWa:
                return "WuWa";

            default:
                return "";
        }
    }

}
