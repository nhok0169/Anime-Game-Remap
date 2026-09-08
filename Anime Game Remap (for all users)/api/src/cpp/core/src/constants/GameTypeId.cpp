#include "AGRemapCore/constants/GameTypeId.h"

#include <set>

#include "AGRemapCore/constants/GameType.h"
#include "AGRemapCore/tools/Heading.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

    namespace {
        // Function-local rather than a namespace-scope global: GameType builds an Aho-Corasick DFA
        // in its constructor, and a namespace-scope instance of it would be racing every other
        // translation unit's static initialization to do so. Lazily built on first use instead,
        // which is the pattern the rest of core's global state uses.
        GameType& gameTypes() {
            static GameType result;
            return result;
        }

        const std::vector<std::string>& noAliases() {
            static const std::vector<std::string> result;
            return result;
        }
    }

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
        const std::unordered_map<GameTypeId, std::string>& names = gameTypes().names;

        auto nameIt = names.find(value);
        if (nameIt == names.end()) {
            return "";
        }

        return nameIt->second;
    }

    const std::vector<std::string>& GameTypeIdTools::getAliases(GameTypeId value) {
        const std::unordered_map<GameTypeId, std::vector<std::string>>& aliases = gameTypes().aliases;

        auto aliasIt = aliases.find(value);
        if (aliasIt == aliases.end()) {
            return noAliases();
        }

        return aliasIt->second;
    }

    const std::vector<GameTypeId>& GameTypeIdTools::getAll() {
        return gameTypes().order;
    }

    std::optional<GameTypeId> GameTypeIdTools::findByName(const std::string& name) {
        // Lowercased and trimmed to match how GameType's constructor filed the keys, and to match
        // how ModTypeIdTools::findByName normalizes what it is asked.
        const std::string normalizedName = StringTools::toLower(StringTools::strip(name));

        auto [matchedNamePtr, matchedGameTypeIdPtr] = gameTypes().dfa.getMaximalPtr(normalizedName);

        if (matchedNamePtr == nullptr) {
            return std::nullopt;
        }

        return *matchedGameTypeIdPtr;
    }

    std::string GameTypeIdTools::getHelpStr(GameTypeId value) {
        const std::string name = getName(value);
        Heading gameTypeHeading(name, 8, "-");

        std::string result = gameTypeHeading.open();
        result += "\n\nname: " + name;

        const std::vector<std::string>& gameAliases = getAliases(value);
        if (!gameAliases.empty()) {
            // Sorted, matching ModType::getHelpStr -- the help text is user-facing, so a stable
            // order matters more than the order the aliases happen to be declared in.
            std::set<std::string> sortedAliases(gameAliases.begin(), gameAliases.end());

            std::string aliasStr;
            for (const std::string& alias : sortedAliases) {
                if (!aliasStr.empty()) {
                    aliasStr += ", ";
                }
                aliasStr += alias;
            }

            result += "\naliases: " + aliasStr;
        }

        result += "\n\n" + gameTypeHeading.close();
        return result;
    }
}
