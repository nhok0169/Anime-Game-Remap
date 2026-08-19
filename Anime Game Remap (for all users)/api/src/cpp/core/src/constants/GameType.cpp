#include "AGRemapCore/constants/GameType.h"

#include <vector>


namespace AGRemapCore {

    namespace {
        // Single source of truth for GameTypeId's names -- both 'names' and 'dfa' below are
        // derived from this, so the two can never drift out of sync with each other.
        const std::vector<std::pair<GameTypeId, std::string>>& gameTypeIdNames() {
            static const std::vector<std::pair<GameTypeId, std::string>> data = {
                {GameTypeId::GI, "GI"},
                {GameTypeId::WuWa, "WuWa"},
            };

            return data;
        }
    }

    GameType::GameType() {
        std::unordered_map<std::string, GameTypeId> dfaData;

        for (const auto& [id, name] : gameTypeIdNames()) {
            names[id] = name;
            dfaData[name] = id;
        }

        dfa.build(dfaData);
    }
}
