#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifier.h"

#include <cctype>
#include <string>

#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

    // "game:<GameTypeId>" DFA transition names, keyed off GameTypeId's underlying int value rather
    // than the game's name, so they stay in sync if GameTypeId's enumerators are ever renamed.
    static const std::string GITransition = "game:" + std::to_string(static_cast<int>(GameTypeId::GI));
    static const std::string WuWaTransition = "game:" + std::to_string(static_cast<int>(GameTypeId::WuWa));

    IniClassifier::IniClassifier() {
        setupStateDFA();

        // Combines duplicate keyword registrations in sectionKeywordsDFA by unioning the two
        // ModTypeId sets, rather than the default behavior of letting the newer value overwrite
        // the older one -- a section-name keyword can legitimately identify more than one
        // ModTypeId (e.g. two mod types sharing a keyword). Set once here rather than in clear()
        // too: BaseTrie::clear() never touches handleDuplicate, so this survives a clear() call.
        BaseAhoCorasickDFA<std::unordered_set<int>>::DupHandler combineModTypeIdSets =
            [](std::string_view keyword, const std::unordered_set<int>& existingSet, const std::unordered_set<int>& newSet) -> std::unordered_set<int> {
                std::unordered_set<int> combined = existingSet;
                for (int modTypeId : newSet) {
                    combined.insert(modTypeId);
                }
                return combined;
            };
        sectionKeywordsDFA.setHandleDuplicate(combineModTypeIdSets);
    }

    void IniClassifier::setupStateDFA() {
        stateDFA.addState("start");
        stateDFA.addState("isWuwa");
        stateDFA.addKeywordTransition("start", WuWaTransition, "isWuwa");
    }

    void IniClassifier::clear() {
        stateDFA.clear();
        setupStateDFA();

        hashGameTypeIds.clear();
        modTypes.clear();
        sectionKeywordsDFA.clear();
        modTypeIdDistribution.clear();
        savedWuWaModTypeIds.clear();
        acceptModTypeIds.clear();
    }

    IniClassifyStats IniClassifier::classify(const std::string& iniTxt, bool checkIsMod, bool checkIsFixed, std::optional<GameTypeId> gameTypeId) {
        // Matches ParseContext's own str -> lines convention (StringTools::splitlines), rather
        // than the old pure-Python IniClassifierOld's 'readlines'-style keepends=True splitting --
        // see the chat writeup for why this is flagged as a to-confirm assumption, not a settled
        // behavior match.
        std::vector<std::string> lines;
        for (std::string_view line : StringTools::splitlines(iniTxt)) {
            lines.emplace_back(line);
        }

        return classify(lines, checkIsMod, checkIsFixed, gameTypeId);
    }

    IniClassifyStats IniClassifier::classify(const std::vector<std::string>& iniTxt, bool checkIsMod, bool checkIsFixed, std::optional<GameTypeId> gameTypeId) {
        IniClassifyStats stats;

        for (const std::string& line : iniTxt) {
            std::string strippedLine(StringTools::strip(line));
            readLine(strippedLine, stats);
        }

        return stats;
    }

    void IniClassifier::readLine(const std::string& line, IniClassifyStats& stats) {
        static constexpr std::string_view hashPrefix = "hash";
        static constexpr std::string_view wwmiPrefix = "$\\WWMIv1";

        // "hash[whitespace*]=[value]"
        if (line.starts_with(hashPrefix)) {
            size_t pos = hashPrefix.size();
            while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) {
                pos++;
            }

            if (pos < line.size() && line[pos] == '=') {
                pos++;

                // The value runs up to (but excludes) a trailing .ini comment, if any -- e.g.
                // "hash = deadbeef ; some comment" should only capture "deadbeef", not the comment.
                size_t end = pos;
                while (end < line.size() && line[end] != ';' && line[end] != '#') {
                    end++;
                }

                std::string_view hashValue = StringTools::strip(std::string_view(line).substr(pos, end - pos));
                readHash(hashValue, stats);
                return;
            }
        }

        // "$\WWMIv1..."
        if (line.starts_with(wwmiPrefix)) {
            markWuWa(stats);
            return;
        }

        // "[X]...", where X contains none of '[', ']', or a .ini comment character
        if (!line.empty() && line.front() == '[') {
            size_t i = 1;
            bool validSectionName = true;

            while (i < line.size() && line[i] != ']') {
                char c = line[i];
                if (c == '[' || c == ';' || c == '#') {
                    validSectionName = false;
                    break;
                }

                i++;
            }

            if (validSectionName && i < line.size() && line[i] == ']') {
                readSectionName(std::string_view(line).substr(1, i - 1), stats);
                return;
            }
        }
    }

    void IniClassifier::readHash(std::string_view hash, IniClassifyStats& stats) {
        std::string hashStr(hash);
        std::string prevStateId = stateDFA.getCurrentStateId();

        std::string newState;
        bool isAccept;
        bool taken;
        stateDFA.transition("hash:" + hashStr, &newState, &isAccept, &taken);

        if (!taken) {
            return;
        }

        // Direct hit: '$\WWMIv1' was already seen (stateDFA on "isWuwa") and this hash is
        // registered for a WuWa mod type -- "isWuwa" transitions straight to the accepting
        // "accept<ModTypeId>" state via this hash, so the match is already confirmed.
        if (isAccept && prevStateId == "isWuwa") {
            bool addedAny = false;
            auto acceptIt = acceptModTypeIds.find(newState);
            if (acceptIt != acceptModTypeIds.end()) {
                for (int modTypeId : acceptIt->second) {
                    stats.modType.emplace(modTypeId, modTypes.at(modTypeId));
                    addedAny = true;
                }
            }

            if (addedAny) {
                stats.isMod = true;
            }

            stateDFA.transition("reset", &newState, &isAccept, &taken);
            return;
        }

        // Otherwise, this hash led to a "foundHash:<hash>" state -- try each GameTypeId this hash
        // is registered under (normally just one, but a set to account for the theoretical case
        // where the same hash is registered for more than one game type).
        auto hashGameTypeIdsIt = hashGameTypeIds.find(hashStr);
        if (hashGameTypeIdsIt != hashGameTypeIds.end()) {
            const std::unordered_set<int>& gameTypeIds = hashGameTypeIdsIt->second;
            bool addedToStats = false;
            size_t i = 0;
            size_t count = gameTypeIds.size();

            for (int gameTypeId : gameTypeIds) {
                i++;

                stateDFA.transition("game:" + std::to_string(gameTypeId), &newState, &isAccept, &taken);

                if (taken && isAccept) {
                    auto acceptIt = acceptModTypeIds.find(newState);
                    if (acceptIt != acceptModTypeIds.end()) {
                        for (int modTypeId : acceptIt->second) {
                            if (gameTypeId == static_cast<int>(GameTypeId::WuWa)) {
                                savedWuWaModTypeIds.insert(modTypeId);
                            } else {
                                stats.modType.emplace(modTypeId, modTypes.at(modTypeId));
                                addedToStats = true;
                            }
                        }
                    }
                }

                if (i != count) {
                    stateDFA.transition("prev:hash:" + hashStr, &newState, &isAccept, &taken);
                } else {
                    stateDFA.transition("reset", &newState, &isAccept, &taken);
                }
            }

            if (addedToStats) {
                stats.isMod = true;
            }
        }
    }

    void IniClassifier::markWuWa(IniClassifyStats& stats) {
        if (stateDFA.getCurrentStateId() != "start") {
            return;
        }

        std::string newState;
        bool isAccept;
        bool transitionTaken;
        stateDFA.transition(WuWaTransition, &newState, &isAccept, &transitionTaken);

        if (!savedWuWaModTypeIds.empty()) {
            for (int modTypeId : savedWuWaModTypeIds) {
                incModTypeCountByHash(modTypeId);
            }

            stats.isMod = true;
            savedWuWaModTypeIds.clear();
        }
    }

    void IniClassifier::readSectionName(std::string_view sectionName, IniClassifyStats& stats) {
        // TODO: filled in later
    }

    bool IniClassifier::addGIModType(const ModType& modType, const std::unordered_set<std::string>& hashes, const std::unordered_set<std::string>& sectionKeywords) {
        // ----- checks -----
        if (modTypes.find(modType.modTypeId) != modTypes.end()) {
            return false;
        }

        if (modType.gameTypeId != static_cast<int>(GameTypeId::GI)) {
            return false;
        }

        // ----- setup -----
        modTypes.emplace(modType.modTypeId, modType);

        std::string acceptStateId = "accept" + std::to_string(modType.modTypeId);
        stateDFA.addState(acceptStateId, true);
        acceptModTypeIds[acceptStateId].insert(modType.modTypeId);

        for (const std::string& hash : hashes) {
            hashGameTypeIds[hash].insert(static_cast<int>(GameTypeId::GI));

            std::string foundHashStateId = "foundHash:" + hash;
            stateDFA.addKeywordTransition("start", "hash:" + hash, foundHashStateId);
            stateDFA.addKeywordTransition(foundHashStateId, GITransition, acceptStateId);
            stateDFA.addKeywordTransition(acceptStateId, "prev:hash:" + hash, foundHashStateId);
        }

        stateDFA.addKeywordTransition(acceptStateId, "reset", "start");

        std::unordered_set<int> modTypeIdSet;
        modTypeIdSet.insert(modType.modTypeId);

        for (const std::string& keyword : sectionKeywords) {
            sectionKeywordsDFA.add(keyword, modTypeIdSet);

            std::string sectionNameStateId = "sectionName:" + keyword;
            stateDFA.addKeywordTransition("start", sectionNameStateId, sectionNameStateId);
            stateDFA.addKeywordTransition(sectionNameStateId, GITransition, acceptStateId);
            stateDFA.addKeywordTransition(acceptStateId, "prev:sectionName:" + keyword, sectionNameStateId);
        }

        return true;
    }

    bool IniClassifier::addWuWaModType(const ModType& modType, const std::unordered_set<std::string>& hashes) {
        // ----- checks -----
        if (modTypes.find(modType.modTypeId) != modTypes.end()) {
            return false;
        }

        if (modType.gameTypeId != static_cast<int>(GameTypeId::WuWa)) {
            return false;
        }

        // ----- setup -----
        modTypes.emplace(modType.modTypeId, modType);

        std::string acceptStateId = "accept" + std::to_string(modType.modTypeId);
        std::string saveStateId = "save" + std::to_string(modType.modTypeId);
        stateDFA.addState(acceptStateId, true);
        stateDFA.addState(saveStateId, true);
        acceptModTypeIds[acceptStateId].insert(modType.modTypeId);
        acceptModTypeIds[saveStateId].insert(modType.modTypeId);

        for (const std::string& hash : hashes) {
            hashGameTypeIds[hash].insert(static_cast<int>(GameTypeId::WuWa));

            std::string foundHashStateId = "foundHash:" + hash;
            stateDFA.addKeywordTransition("start", "hash:" + hash, foundHashStateId);
            stateDFA.addKeywordTransition("isWuwa", "hash:" + hash, acceptStateId);
            stateDFA.addKeywordTransition(foundHashStateId, WuWaTransition, saveStateId);
            stateDFA.addKeywordTransition(saveStateId, "prev:hash:" + hash, foundHashStateId);
        }

        stateDFA.addKeywordTransition(acceptStateId, "prev", "isWuwa");
        stateDFA.addKeywordTransition(acceptStateId, "reset", "isWuwa");
        stateDFA.addKeywordTransition(saveStateId, "reset", "start");

        return true;
    }

    void IniClassifier::incModTypeCountBySectionName(int modTypeId) {
        modTypeIdDistribution[modTypeId] += 1;
    }

    void IniClassifier::incModTypeCountByHash(int modTypeId) {
        modTypeIdDistribution[modTypeId] += 2;
    }

}
