#include "AGRemapCore/constants/GameType.h"

#include <utility>
#include <vector>

#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

    namespace {
        // Single source of truth for GameTypeId's names AND aliases -- 'names', 'aliases', 'order'
        // and 'dfa' below are all derived from this, so none of the four can drift out of sync with
        // each other. GameTypeIdTools::getName reads it through here too, rather than carrying its
        // own switch: it used to, and a second table is a second thing to forget to update.
        const std::vector<std::pair<GameTypeId, std::vector<std::string>>>& gameTypeIdNames() {
            // The FIRST entry of each name list is the game's name; the rest are its aliases.
            static const std::vector<std::pair<GameTypeId, std::vector<std::string>>> data = {
                {GameTypeId::GI, {"GI", "Genshin", "GenshinImpact"}},
                {GameTypeId::WuWa, {"WuWa", "WutheringWaves"}},
            };

            return data;
        }
    }

    GameType::GameType() {
        std::unordered_map<std::string, GameTypeId> dfaData;

        for (const auto& [id, gameNames] : gameTypeIdNames()) {
            order.push_back(id);
            names[id] = gameNames.front();
            aliases[id] = std::vector<std::string>(gameNames.begin() + 1, gameNames.end());

            // Filed lowercased, and findByName lowercases what it is asked, so a name or alias
            // resolves whatever case it is typed in -- the same arrangement
            // ModTypeIdTools::registerModType uses for a ModType's names, and what the CLI's own
            // help text promises ("The names/aliases for the game types are not case sensitive").
            for (const std::string& gameName : gameNames) {
                dfaData[StringTools::toLower(gameName)] = id;
            }
        }

        dfa.build(dfaData);
    }
}
