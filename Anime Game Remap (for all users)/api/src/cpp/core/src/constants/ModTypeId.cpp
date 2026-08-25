#include "AGRemapCore/constants/ModTypeId.h"


namespace AGRemapCore {

    std::optional<ModTypeId> ModTypeIdTools::getEnum(int value) {
        switch (value) {
            case static_cast<int>(ModTypeId::Amber):
                return ModTypeId::Amber;

            case static_cast<int>(ModTypeId::AmberCN):
                return ModTypeId::AmberCN;

            case static_cast<int>(ModTypeId::Ayaka):
                return ModTypeId::Ayaka;

            case static_cast<int>(ModTypeId::AyakaSpringbloom):
                return ModTypeId::AyakaSpringbloom;

            case static_cast<int>(ModTypeId::Arlecchino):
                return ModTypeId::Arlecchino;

            case static_cast<int>(ModTypeId::ArlecchinoBoss):
                return ModTypeId::ArlecchinoBoss;

            case static_cast<int>(ModTypeId::Barbara):
                return ModTypeId::Barbara;

            case static_cast<int>(ModTypeId::BarbaraSummertime):
                return ModTypeId::BarbaraSummertime;

            case static_cast<int>(ModTypeId::CherryHuTao):
                return ModTypeId::CherryHuTao;

            case static_cast<int>(ModTypeId::Diluc):
                return ModTypeId::Diluc;

            case static_cast<int>(ModTypeId::DilucFlamme):
                return ModTypeId::DilucFlamme;

            case static_cast<int>(ModTypeId::Fischl):
                return ModTypeId::Fischl;

            case static_cast<int>(ModTypeId::FischlHighness):
                return ModTypeId::FischlHighness;

            case static_cast<int>(ModTypeId::Ganyu):
                return ModTypeId::Ganyu;

            case static_cast<int>(ModTypeId::GanyuTwilight):
                return ModTypeId::GanyuTwilight;

            case static_cast<int>(ModTypeId::HuTao):
                return ModTypeId::HuTao;

            case static_cast<int>(ModTypeId::Jean):
                return ModTypeId::Jean;

            case static_cast<int>(ModTypeId::JeanCN):
                return ModTypeId::JeanCN;

            case static_cast<int>(ModTypeId::JeanSea):
                return ModTypeId::JeanSea;

            case static_cast<int>(ModTypeId::Kaeya):
                return ModTypeId::Kaeya;

            case static_cast<int>(ModTypeId::KaeyaSailwind):
                return ModTypeId::KaeyaSailwind;

            case static_cast<int>(ModTypeId::Keqing):
                return ModTypeId::Keqing;

            case static_cast<int>(ModTypeId::KeqingOpulent):
                return ModTypeId::KeqingOpulent;

            case static_cast<int>(ModTypeId::Kirara):
                return ModTypeId::Kirara;

            case static_cast<int>(ModTypeId::KiraraBoots):
                return ModTypeId::KiraraBoots;

            case static_cast<int>(ModTypeId::Klee):
                return ModTypeId::Klee;

            case static_cast<int>(ModTypeId::KleeBlossomingStarlight):
                return ModTypeId::KleeBlossomingStarlight;

            case static_cast<int>(ModTypeId::Lisa):
                return ModTypeId::Lisa;

            case static_cast<int>(ModTypeId::LisaStudent):
                return ModTypeId::LisaStudent;

            case static_cast<int>(ModTypeId::Mona):
                return ModTypeId::Mona;

            case static_cast<int>(ModTypeId::MonaCN):
                return ModTypeId::MonaCN;

            case static_cast<int>(ModTypeId::Nilou):
                return ModTypeId::Nilou;

            case static_cast<int>(ModTypeId::NilouBreeze):
                return ModTypeId::NilouBreeze;

            case static_cast<int>(ModTypeId::Ningguang):
                return ModTypeId::Ningguang;

            case static_cast<int>(ModTypeId::NingguangOrchid):
                return ModTypeId::NingguangOrchid;

            case static_cast<int>(ModTypeId::Raiden):
                return ModTypeId::Raiden;

            case static_cast<int>(ModTypeId::RaidenBoss):
                return ModTypeId::RaidenBoss;

            case static_cast<int>(ModTypeId::Rosaria):
                return ModTypeId::Rosaria;

            case static_cast<int>(ModTypeId::RosariaCN):
                return ModTypeId::RosariaCN;

            case static_cast<int>(ModTypeId::Shenhe):
                return ModTypeId::Shenhe;

            case static_cast<int>(ModTypeId::ShenheFrostFlower):
                return ModTypeId::ShenheFrostFlower;

            case static_cast<int>(ModTypeId::Xiangling):
                return ModTypeId::Xiangling;

            case static_cast<int>(ModTypeId::XianglingCheer):
                return ModTypeId::XianglingCheer;

            case static_cast<int>(ModTypeId::Xingqiu):
                return ModTypeId::Xingqiu;

            case static_cast<int>(ModTypeId::XingqiuBamboo):
                return ModTypeId::XingqiuBamboo;

            default:
                return std::nullopt;
        }
    }

    std::string ModTypeIdTools::getName(ModTypeId value) {
        switch (value) {
            case ModTypeId::Amber:
                return "Amber";

            case ModTypeId::AmberCN:
                return "AmberCN";

            case ModTypeId::Ayaka:
                return "Ayaka";

            // note: the value differs from the enumerator's own name ("AyakaSpringbloom") --
            // mirrors ModTypeNames.py's AyakaSpringbloom = "AyakaSpringBloom" exactly
            case ModTypeId::AyakaSpringbloom:
                return "AyakaSpringBloom";

            case ModTypeId::Arlecchino:
                return "Arlecchino";

            case ModTypeId::ArlecchinoBoss:
                return "ArlecchinoBoss";

            case ModTypeId::Barbara:
                return "Barbara";

            case ModTypeId::BarbaraSummertime:
                return "BarbaraSummertime";

            case ModTypeId::CherryHuTao:
                return "CherryHuTao";

            case ModTypeId::Diluc:
                return "Diluc";

            case ModTypeId::DilucFlamme:
                return "DilucFlamme";

            case ModTypeId::Fischl:
                return "Fischl";

            case ModTypeId::FischlHighness:
                return "FischlHighness";

            case ModTypeId::Ganyu:
                return "Ganyu";

            case ModTypeId::GanyuTwilight:
                return "GanyuTwilight";

            case ModTypeId::HuTao:
                return "HuTao";

            case ModTypeId::Jean:
                return "Jean";

            case ModTypeId::JeanCN:
                return "JeanCN";

            case ModTypeId::JeanSea:
                return "JeanSea";

            case ModTypeId::Kaeya:
                return "Kaeya";

            case ModTypeId::KaeyaSailwind:
                return "KaeyaSailwind";

            case ModTypeId::Keqing:
                return "Keqing";

            case ModTypeId::KeqingOpulent:
                return "KeqingOpulent";

            case ModTypeId::Kirara:
                return "Kirara";

            case ModTypeId::KiraraBoots:
                return "KiraraBoots";

            case ModTypeId::Klee:
                return "Klee";

            case ModTypeId::KleeBlossomingStarlight:
                return "KleeBlossomingStarlight";

            case ModTypeId::Lisa:
                return "Lisa";

            case ModTypeId::LisaStudent:
                return "LisaStudent";

            case ModTypeId::Mona:
                return "Mona";

            case ModTypeId::MonaCN:
                return "MonaCN";

            case ModTypeId::Nilou:
                return "Nilou";

            case ModTypeId::NilouBreeze:
                return "NilouBreeze";

            case ModTypeId::Ningguang:
                return "Ningguang";

            case ModTypeId::NingguangOrchid:
                return "NingguangOrchid";

            case ModTypeId::Raiden:
                return "Raiden";

            case ModTypeId::RaidenBoss:
                return "RaidenBoss";

            case ModTypeId::Rosaria:
                return "Rosaria";

            case ModTypeId::RosariaCN:
                return "RosariaCN";

            case ModTypeId::Shenhe:
                return "Shenhe";

            case ModTypeId::ShenheFrostFlower:
                return "ShenheFrostFlower";

            case ModTypeId::Xiangling:
                return "Xiangling";

            case ModTypeId::XianglingCheer:
                return "XianglingCheer";

            case ModTypeId::Xingqiu:
                return "Xingqiu";

            case ModTypeId::XingqiuBamboo:
                return "XingqiuBamboo";

            default:
                return "";
        }
    }

    std::unordered_map<int, ModType> ModTypeIdTools::_modTypes;
    BaseAhoCorasickDFA<std::unordered_set<int>> ModTypeIdTools::_nameDFA;
    std::unordered_map<std::string, std::unordered_set<int>> ModTypeIdTools::_nameGameTypeIds;

    // '_nameDFA' needs the same duplicate-merging behavior IniClassifier sets up for its own
    // 'sectionKeywordsDFA' (see IniClassifier's constructor) -- two different registered ModTypes
    // can legitimately share the same name/alias, and the default add() behavior would otherwise
    // let the second registration's ModTypeId set silently overwrite the first's instead of
    // merging. There's no constructor to do this setup in for a static-method-only "Tools" class,
    // so '_nameDFAInitialized's own initializer triggers it exactly once, right after '_nameDFA'
    // itself is constructed above (namespace-scope variables in one translation unit are
    // initialized in declaration order, so this ordering is well-defined).
    bool ModTypeIdTools::_setupNameDFA() {
        BaseAhoCorasickDFA<std::unordered_set<int>>::DupHandler combineModTypeIdSets =
            [](std::string_view keyword, const std::unordered_set<int>& existingSet, const std::unordered_set<int>& newSet) -> std::unordered_set<int> {
                std::unordered_set<int> combined = existingSet;
                for (int modTypeId : newSet) {
                    combined.insert(modTypeId);
                }
                return combined;
            };
        _nameDFA.setHandleDuplicate(combineModTypeIdSets);
        return true;
    }

    bool ModTypeIdTools::_nameDFAInitialized = ModTypeIdTools::_setupNameDFA();

    std::optional<ModType> ModTypeIdTools::getModType(int modTypeId) {
        auto it = _modTypes.find(modTypeId);
        if (it == _modTypes.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    void ModTypeIdTools::registerModType(const ModType &modType) {
        _modTypes.insert_or_assign(modType.modTypeId, modType);

        std::unordered_set<int> modTypeIdSet;
        modTypeIdSet.insert(modType.modTypeId);

        _nameDFA.add(modType.name, modTypeIdSet);
        _nameGameTypeIds[modType.name].insert(modType.gameTypeId);

        for (const std::string &alias : modType.aliases) {
            _nameDFA.add(alias, modTypeIdSet);
            _nameGameTypeIds[alias].insert(modType.gameTypeId);
        }
    }

    std::optional<ModTypeId> ModTypeIdTools::findByName(const std::string &name, std::optional<GameTypeId> gameTypeId) {
        std::optional<BaseAhoCorasickDFA<std::unordered_set<int>>::KeywordPredicate> pred = std::nullopt;

        if (gameTypeId.has_value()) {
            int gameTypeIdInt = static_cast<int>(*gameTypeId);
            pred = [gameTypeIdInt](const std::string& keyword) -> bool {
                auto it = _nameGameTypeIds.find(keyword);
                return it != _nameGameTypeIds.end() && it->second.count(gameTypeIdInt) == 1;
            };
        }

        auto [matchedNamePtr, matchedModTypeIdsPtr] = _nameDFA.getMaximalPtr(name, pred);

        if (matchedNamePtr == nullptr) {
            return std::nullopt;
        }

        // The DFA's own value is a flat, game-agnostic set of every ModTypeId sharing this
        // name/alias -- narrow it down to the ones whose own registered gameTypeId actually
        // matches (when a filter was given) by cross-referencing '_modTypes'. If more than one
        // ModTypeId still remains, the match is ambiguous -- don't guess.
        std::optional<int> resultModTypeId = std::nullopt;

        for (int candidateModTypeId : *matchedModTypeIdsPtr) {
            if (gameTypeId.has_value()) {
                auto modTypeIt = _modTypes.find(candidateModTypeId);
                if (modTypeIt == _modTypes.end() || modTypeIt->second.gameTypeId != static_cast<int>(*gameTypeId)) {
                    continue;
                }
            }

            if (resultModTypeId.has_value()) {
                return std::nullopt;
            }

            resultModTypeId = candidateModTypeId;
        }

        if (!resultModTypeId.has_value()) {
            return std::nullopt;
        }

        return getEnum(*resultModTypeId);
    }

    void ModTypeIdTools::clear() {
        _modTypes.clear();
        _nameDFA.clear();
        _nameGameTypeIds.clear();
    }

}
