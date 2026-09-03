#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifier.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/constants/IniKeywords.h"


namespace AGRemapCore {
    namespace {
        // Section-name keyword matching is case-insensitive, so both ends go through here: what
        // addGIModType registers, and what readSectionName looks up. Lowering only one side matches
        // nothing -- and does it silently, since an unmatched keyword just means "not this mod
        // type". Lowered by grapheme through StringTools, so a non-ASCII keyword or section name
        // is folded the same way on both ends too.
        std::string toLower(std::string_view text) {
            return StringTools::toLower(text);
        }
    }


    // "game:<GameTypeId>" DFA transition names, keyed off GameTypeId's underlying int value rather
    // than the game's name, so they stay in sync if GameTypeId's enumerators are ever renamed.
    static const std::string GITransition = "game:" + std::to_string(static_cast<int>(GameTypeId::GI));
    static const std::string WuWaTransition = "game:" + std::to_string(static_cast<int>(GameTypeId::WuWa));

    IniClassifier::IniClassifier(bool checkHasTextureOverride): checkHasTextureOverride(checkHasTextureOverride) {
        setupStateDFA();

        // Combines duplicate keyword registrations in sectionKeywordsDFA by unioning the two
        // GameTypeId sets, rather than the default behavior of letting the newer value overwrite
        // the older one -- a section-name keyword can legitimately identify more than one
        // GameTypeId (e.g. the same keyword registered for more than one game). Set once here
        // rather than in clear() too: BaseTrie::clear() never touches handleDuplicate, so this
        // survives a clear() call.
        BaseAhoCorasickDFA<std::unordered_set<int>>::DupHandler combineGameTypeIdSets =
            [](std::string_view keyword, const std::unordered_set<int>& existingSet, const std::unordered_set<int>& newSet) -> std::unordered_set<int> {
                std::unordered_set<int> combined = existingSet;
                for (int gameTypeId : newSet) {
                    combined.insert(gameTypeId);
                }
                return combined;
            };
        sectionKeywordsDFA.setHandleDuplicate(combineGameTypeIdSets);
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
        keywordGameTypeIds.clear();
        modTypeIdDistribution.clear();
        savedWuWaModTypeIds.clear();
        currentSectionIsRemap = false;
        acceptModTypeIds.clear();
    }

    IniClassifyStats IniClassifier::classify(const std::string& iniTxt, std::optional<GameTypeId> gameTypeId) {
        // Matches ParseContext's own str -> lines convention (StringTools::splitlines), rather
        // than the old pure-Python IniClassifierOld's 'readlines'-style keepends=True splitting --
        // see the chat writeup for why this is flagged as a to-confirm assumption, not a settled
        // behavior match.
        std::vector<std::string> lines;
        for (std::string_view line : StringTools::splitlines(iniTxt)) {
            lines.emplace_back(line);
        }

        return classify(lines, gameTypeId);
    }

    IniClassifyStats IniClassifier::classify(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId) {
        IniClassifyStats stats;

        // All three are transient, per-classification state (a running tally of ModTypeIds seen so
        // far this .ini file, hashes seen before their '$\WWMIv1' confirmation marker, and which
        // section is currently being read) -- none should leak into a later, unrelated classify()
        // call on the same IniClassifier instance.
        modTypeIdDistribution.clear();
        savedWuWaModTypeIds.clear();
        currentSectionIsRemap = false;

        for (const std::string& line : iniTxt) {
            // The hide-original marker comes off BEFORE the whitespace strip, so a section a
            // previous fix commented out still classifies. A hidden section reads
            // ";RemapFixHideOrig -->[TextureOverrideRaidenBody]"; leaving the marker on makes it
            // an ordinary comment line, and the .ini file it belongs to silently classifies as no
            // mod type at all on a re-run.
            //
            // This is what the pure-Python original does in its own classify loop
            // (IniClassifierOld.py: "line.replace(IniKeywords.HideOriginalComment.value, '')"),
            // and it is the counterpart of IniFile::stripHideOriginalComment on the read side.
            std::string cleanedLine(line);
            StringTools::eraseAll(cleanedLine, IniKeywords::HideOriginalComment);

            std::string strippedLine(StringTools::strip(cleanedLine));
            readLine(strippedLine, stats, gameTypeId);
        }

        // The most likely ModTypeId(s) are whichever have the highest count in
        // modTypeIdDistribution -- there may be ties, so collect all of them, then sort ascending
        // before adding to stats.
        int maxCount = 0;
        for (const auto& [modTypeId, count] : modTypeIdDistribution) {
            if (count > maxCount) {
                maxCount = count;
            }
        }

        std::vector<int> topModTypeIds;
        for (const auto& [modTypeId, count] : modTypeIdDistribution) {
            if (count == maxCount) {
                topModTypeIds.push_back(modTypeId);
            }
        }

        std::sort(topModTypeIds.begin(), topModTypeIds.end());

        for (int modTypeId : topModTypeIds) {
            stats.modType.emplace(modTypeId, modTypes.at(modTypeId));
        }

        return stats;
    }

    bool IniClassifier::checkIsMod(const std::string& iniTxt, std::optional<GameTypeId> gameTypeId) {
        std::vector<std::string> lines;
        for (std::string_view line : StringTools::splitlines(iniTxt)) {
            lines.emplace_back(line);
        }

        return checkIsMod(lines, gameTypeId);
    }

    bool IniClassifier::checkIsMod(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId) {
        IniClassifyStats stats;

        // Same transient-state reset as classify() -- see its own comment for why.
        modTypeIdDistribution.clear();
        savedWuWaModTypeIds.clear();
        currentSectionIsRemap = false;

        for (const std::string& line : iniTxt) {
            std::string strippedLine(StringTools::strip(line));
            readLine(strippedLine, stats, gameTypeId);

            if (stats.isMod) {
                return true;
            }
        }

        return false;
    }

    void IniClassifier::checkIsFixedMod(const std::string& iniTxt, bool* isFixed, bool* isMod, std::optional<GameTypeId> gameTypeId) {
        std::vector<std::string> lines;
        for (std::string_view line : StringTools::splitlines(iniTxt)) {
            lines.emplace_back(line);
        }

        checkIsFixedMod(lines, isFixed, isMod, gameTypeId);
    }

    void IniClassifier::checkIsFixedMod(const std::vector<std::string>& iniTxt, bool* isFixed, bool* isMod, std::optional<GameTypeId> gameTypeId) {
        IniClassifyStats stats;

        // Same transient-state reset as classify()/checkIsMod() -- see classify()'s own comment for why.
        modTypeIdDistribution.clear();
        savedWuWaModTypeIds.clear();
        currentSectionIsRemap = false;

        for (const std::string& line : iniTxt) {
            std::string strippedLine(StringTools::strip(line));
            readLine(strippedLine, stats, gameTypeId);

            if (stats.isMod && stats.isFixed) {
                break;
            }
        }

        *isFixed = stats.isFixed;
        *isMod = stats.isMod;
    }

    void IniClassifier::readLine(const std::string& line, IniClassifyStats& stats, std::optional<GameTypeId> gameTypeId) {
        static constexpr std::string_view hashPrefix = "hash";
        static constexpr std::string_view wwmiPrefix = "$\\WWMIv1";

        // A previously-fixed .ini file has its original section commented out this way -- every
        // line of it (including the section header) individually prefixed, glued directly onto the
        // front with no separator (see IniFile._commentSection's f"{comment}{fileLines[i]}"). See
        // it through to the real line underneath and read that normally, rather than letting it
        // fall through as an ordinary, ignored ".ini comment".
        if (line.starts_with(IniKeywords::HideOriginalComment)) {
            readLine(line.substr(IniKeywords::HideOriginalComment.size()), stats, gameTypeId);
            return;
        }

        // "hash[whitespace*]=[value]"
        if (line.starts_with(hashPrefix)) {
            // Whitespace is skipped by grapheme through StringTools (so Unicode whitespace between
            // "hash" and "=" is tolerated, the way the pure-Python original's regex \s* was).
            std::string_view rest = StringTools::lstrip(std::string_view(line).substr(hashPrefix.size()));

            if (rest.starts_with('=')) {
                rest.remove_prefix(1);

                // The value runs up to (but excludes) a trailing .ini comment, if any -- e.g.
                // "hash = deadbeef ; some comment" should only capture "deadbeef", not the comment.
                std::string_view value = rest.substr(0, rest.find_first_of(";#"));

                // A hash inside a "Remap"-named section is a fix-added redirect to some other mod
                // type this .ini was already fixed to (e.g. a JeanCN/JeanSea RemapFix section
                // inside an already-fixed Jean .ini) -- it shouldn't count toward classification,
                // or the result would wrongly include those other mod types too.
                if (!currentSectionIsRemap) {
                    readHash(StringTools::strip(value), stats);
                }

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
                readSectionName(std::string_view(line).substr(1, i - 1), stats, gameTypeId);
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
                    incModTypeCountByHash(modTypeId);
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
                                incModTypeCountByHash(modTypeId);
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

    void IniClassifier::readSectionName(std::string_view sectionName, IniClassifyStats& stats, std::optional<GameTypeId> gameTypeId) {
        // Every comparison in this function is case-insensitive, matching the pure-Python original,
        // which lowercases each whole line before reading it. A .ini section is conventionally
        // written "[TextureOverrideRaidenBody]", but nothing requires that casing and real mods do
        // write "[textureoverride...]" -- so folding only the keyword lookup below and leaving
        // these prefix checks exact would accept a mod type's keyword while rejecting the very
        // section it appeared in.
        std::string lowerSectionName = toLower(sectionName);
        static const std::string lowerTextureOverride = toLower(IniKeywords::TextureOverride);
        static const std::string lowerShaderOverride = toLower(IniKeywords::ShaderOverride);
        static const std::string lowerRemap = toLower(IniKeywords::Remap);

        // Tracks which section readLine is currently inside, for its own hash-skipping purposes --
        // updated unconditionally, before anything below (including #checkHasTextureOverride) has a
        // chance to return early, so it always reflects the actual most recent section.
        bool isRemapSection = lowerSectionName.find(lowerRemap) != std::string::npos;
        currentSectionIsRemap = isRemapSection;

        bool hasTextureOverride = lowerSectionName.starts_with(lowerTextureOverride);
        if (hasTextureOverride || lowerSectionName.starts_with(lowerShaderOverride)) {
            stats.isMod = true;
        }

        if (checkHasTextureOverride && !hasTextureOverride) {
            return;
        }

        if (isRemapSection) {
            stats.isFixed = true;

            // Same reasoning as readLine's hash-skip for a Remap-named section: a section-keyword
            // match inside one is a fix-added redirect to some other mod type this .ini was already
            // fixed to (e.g. a JeanCN/JeanSea RemapFix section inside an already-fixed Jean .ini),
            // and shouldn't count toward #modTypeIdDistribution either -- or the same
            // misclassification could happen via this section-keyword path instead of the hash path.
            return;
        }

        std::optional<BaseAhoCorasickDFA<std::unordered_set<int>>::KeywordPredicate> pred = std::nullopt;

        if (gameTypeId.has_value()) {
            int gameTypeIdInt = static_cast<int>(*gameTypeId);
            pred = [this, gameTypeIdInt](const std::string& keyword) -> bool {
                auto keywordGameTypeIdsIt = keywordGameTypeIds.find(keyword);
                return keywordGameTypeIdsIt != keywordGameTypeIds.end() && keywordGameTypeIdsIt->second.count(gameTypeIdInt) == 1;
            };
        }

        // addGIModType lowered the keyword when it registered it, so 'lowerSectionName' is what it
        // has to be matched against.
        auto [matchedKeywordPtr, matchedGameTypeIdsPtr] = sectionKeywordsDFA.getMaximalPtr(lowerSectionName, pred);

        if (matchedKeywordPtr == nullptr) {
            return;
        }

        // Position stateDFA on the matched keyword's "sectionName:<keyword>" state, mirroring
        // readHash's initial "hash:<hash>" transition -- required for the "game:<GameTypeId>"
        // transitions below to have a chance of landing on an accept state (see addGIModType's
        // wiring: "start" --sectionName:<keyword>--> sectionName:<keyword> --game:<GameTypeId>-->
        // accept<ModTypeId>).
        std::string newState;
        bool isAccept;
        bool taken;
        stateDFA.transition("sectionName:" + *matchedKeywordPtr, &newState, &isAccept, &taken);

        if (!taken) {
            return;
        }

        const std::unordered_set<int>& gameTypeIds = *matchedGameTypeIdsPtr;
        size_t i = 0;
        size_t count = gameTypeIds.size();

        for (int matchedGameTypeId : gameTypeIds) {
            i++;

            stateDFA.transition("game:" + std::to_string(matchedGameTypeId), &newState, &isAccept, &taken);

            if (taken && isAccept) {
                auto acceptIt = acceptModTypeIds.find(newState);
                if (acceptIt != acceptModTypeIds.end()) {
                    for (int modTypeId : acceptIt->second) {
                        incModTypeCountBySectionName(modTypeId);
                    }
                }
            }

            if (i != count) {
                stateDFA.transition("prev:sectionName:" + *matchedKeywordPtr, &newState, &isAccept, &taken);
            } else {
                stateDFA.transition("reset", &newState, &isAccept, &taken);
            }
        }
    }

    bool IniClassifier::addGIModType(const ModTypeIdData& modType, const std::unordered_set<std::string>& hashes, const std::unordered_set<std::string>& sectionKeywords) {
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

            // Another ModTypeIdData may have already registered this same hash under the same
            // GameTypeId -- foundHashStateId --GITransition--> already exists in that case, and
            // overwriting it would orphan that ModTypeIdData's accept state for this hash. Instead,
            // leave the existing transition alone and just also associate this ModTypeId with
            // whichever accept state it already leads to.
            if (!stateDFA.hasKeywordTransition(foundHashStateId, GITransition)) {
                stateDFA.addKeywordTransition(foundHashStateId, GITransition, acceptStateId);
                stateDFA.addKeywordTransition(acceptStateId, "prev:hash:" + hash, foundHashStateId);
            } else {
                std::string existingAcceptStateId = *stateDFA.getKeywordToState(foundHashStateId, GITransition);
                acceptModTypeIds[existingAcceptStateId].insert(modType.modTypeId);
            }
        }

        stateDFA.addKeywordTransition(acceptStateId, "reset", "start");

        std::unordered_set<int> gameTypeIdSet;
        gameTypeIdSet.insert(static_cast<int>(GameTypeId::GI));

        for (const std::string& rawKeyword : sectionKeywords) {
            // Lowered once, here, so every downstream use of it -- the match DFA, the
            // GameTypeId predicate's lookup table, and the state id -- agrees on one spelling.
            std::string keyword = toLower(rawKeyword);

            sectionKeywordsDFA.add(keyword, gameTypeIdSet);
            keywordGameTypeIds[keyword].insert(static_cast<int>(GameTypeId::GI));

            std::string sectionNameStateId = "sectionName:" + keyword;
            stateDFA.addKeywordTransition("start", sectionNameStateId, sectionNameStateId);

            // Same collision as above, but for 2 ModTypeIdDatas sharing the same section keyword.
            if (!stateDFA.hasKeywordTransition(sectionNameStateId, GITransition)) {
                stateDFA.addKeywordTransition(sectionNameStateId, GITransition, acceptStateId);
                stateDFA.addKeywordTransition(acceptStateId, "prev:sectionName:" + keyword, sectionNameStateId);
            } else {
                std::string existingAcceptStateId = *stateDFA.getKeywordToState(sectionNameStateId, GITransition);
                acceptModTypeIds[existingAcceptStateId].insert(modType.modTypeId);
            }
        }

        return true;
    }

    bool IniClassifier::addWuWaModType(const ModTypeIdData& modType, const std::unordered_set<std::string>& hashes) {
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

            // Same collision as below, but for the "isWuwa" direct-hit edge (used by readHash's
            // direct-hit branch when '$\WWMIv1' was already seen).
            if (!stateDFA.hasKeywordTransition("isWuwa", "hash:" + hash)) {
                stateDFA.addKeywordTransition("isWuwa", "hash:" + hash, acceptStateId);
            } else {
                std::string existingAcceptStateId = *stateDFA.getKeywordToState("isWuwa", "hash:" + hash);
                acceptModTypeIds[existingAcceptStateId].insert(modType.modTypeId);
            }

            // Same collision as addGIModType's hash loop, but landing on "save<ModTypeId>" instead.
            if (!stateDFA.hasKeywordTransition(foundHashStateId, WuWaTransition)) {
                stateDFA.addKeywordTransition(foundHashStateId, WuWaTransition, saveStateId);
                stateDFA.addKeywordTransition(saveStateId, "prev:hash:" + hash, foundHashStateId);
            } else {
                std::string existingSaveStateId = *stateDFA.getKeywordToState(foundHashStateId, WuWaTransition);
                acceptModTypeIds[existingSaveStateId].insert(modType.modTypeId);
            }
        }

        stateDFA.addKeywordTransition(acceptStateId, "prev", "isWuwa");
        stateDFA.addKeywordTransition(acceptStateId, "reset", "isWuwa");
        stateDFA.addKeywordTransition(saveStateId, "reset", "start");

        return true;
    }

    ModTypeIdData IniClassifier::getModType(int modTypeId) {
        return modTypes.at(modTypeId);
    }

    void IniClassifier::incModTypeCountBySectionName(int modTypeId) {
        modTypeIdDistribution[modTypeId] += 1;
    }

    void IniClassifier::incModTypeCountByHash(int modTypeId) {
        modTypeIdDistribution[modTypeId] += 2;
    }

}
