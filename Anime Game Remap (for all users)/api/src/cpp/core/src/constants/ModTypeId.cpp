#include "AGRemapCore/constants/ModTypeId.h"

#include "AGRemapCore/tools/StringTools.h"


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
    unsigned long long ModTypeIdTools::_generation = 1;
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

    std::vector<ModTypeId> ModTypeIdTools::getHashRemapTargets(ModTypeId value) {
        // Transcribed mechanically from the pure-Python GIBuilder's 43 per-character
        // "Hashes(map = {...})" arguments, not retyped by hand -- see
        // core/tests/ModTypeRemaps_test.cpp, which pins every row.
        //
        // A switch rather than a static map so that adding a ModTypeId and forgetting its targets
        // is at least visible here in one place, next to getName's own switch over the same enum.
        switch (value) {
            case ModTypeId::Amber:
                return {ModTypeId::AmberCN};

            case ModTypeId::AmberCN:
                return {ModTypeId::Amber};

            case ModTypeId::Arlecchino:
                return {ModTypeId::ArlecchinoBoss};

            case ModTypeId::Ayaka:
                return {ModTypeId::AyakaSpringbloom};

            case ModTypeId::AyakaSpringbloom:
                return {ModTypeId::Ayaka};

            case ModTypeId::Barbara:
                return {ModTypeId::BarbaraSummertime};

            case ModTypeId::BarbaraSummertime:
                return {ModTypeId::Barbara};

            case ModTypeId::CherryHuTao:
                return {ModTypeId::HuTao};

            case ModTypeId::Diluc:
                return {ModTypeId::DilucFlamme};

            case ModTypeId::DilucFlamme:
                return {ModTypeId::Diluc};

            case ModTypeId::Fischl:
                return {ModTypeId::FischlHighness};

            case ModTypeId::FischlHighness:
                return {ModTypeId::Fischl};

            case ModTypeId::Ganyu:
                return {ModTypeId::GanyuTwilight};

            case ModTypeId::GanyuTwilight:
                return {ModTypeId::Ganyu};

            case ModTypeId::HuTao:
                return {ModTypeId::CherryHuTao};

            case ModTypeId::Jean:
                return {ModTypeId::JeanCN, ModTypeId::JeanSea};

            case ModTypeId::JeanCN:
                return {ModTypeId::Jean, ModTypeId::JeanSea};

            case ModTypeId::JeanSea:
                return {ModTypeId::Jean, ModTypeId::JeanCN};

            case ModTypeId::Kaeya:
                return {ModTypeId::KaeyaSailwind};

            case ModTypeId::KaeyaSailwind:
                return {ModTypeId::Kaeya};

            case ModTypeId::Keqing:
                return {ModTypeId::KeqingOpulent};

            case ModTypeId::KeqingOpulent:
                return {ModTypeId::Keqing};

            case ModTypeId::Kirara:
                return {ModTypeId::KiraraBoots};

            case ModTypeId::KiraraBoots:
                return {ModTypeId::Kirara};

            case ModTypeId::Klee:
                return {ModTypeId::KleeBlossomingStarlight};

            case ModTypeId::KleeBlossomingStarlight:
                return {ModTypeId::Klee};

            case ModTypeId::Lisa:
                return {ModTypeId::LisaStudent};

            case ModTypeId::LisaStudent:
                return {ModTypeId::Lisa};

            case ModTypeId::Mona:
                return {ModTypeId::MonaCN};

            case ModTypeId::MonaCN:
                return {ModTypeId::Mona};

            case ModTypeId::Nilou:
                return {ModTypeId::NilouBreeze};

            case ModTypeId::NilouBreeze:
                return {ModTypeId::Nilou};

            case ModTypeId::Ningguang:
                return {ModTypeId::NingguangOrchid};

            case ModTypeId::NingguangOrchid:
                return {ModTypeId::Ningguang};

            case ModTypeId::Raiden:
                return {ModTypeId::RaidenBoss};

            case ModTypeId::Rosaria:
                return {ModTypeId::RosariaCN};

            case ModTypeId::RosariaCN:
                return {ModTypeId::Rosaria};

            case ModTypeId::Shenhe:
                return {ModTypeId::ShenheFrostFlower};

            case ModTypeId::ShenheFrostFlower:
                return {ModTypeId::Shenhe};

            case ModTypeId::Xiangling:
                return {ModTypeId::XianglingCheer};

            case ModTypeId::XianglingCheer:
                return {ModTypeId::Xiangling};

            case ModTypeId::Xingqiu:
                return {ModTypeId::XingqiuBamboo};

            case ModTypeId::XingqiuBamboo:
                return {ModTypeId::Xingqiu};
            // Every remaining ModTypeId remaps onto nothing. That covers the two boss ids
            // (RaidenBoss, ArlecchinoBoss), which are only ever remap *targets* -- GIBuilder has
            // no factory for either.
            default:
                return {};
        }
    }


    std::vector<ModTypeId> ModTypeIdTools::getIndexRemapTargets(ModTypeId value) {
        // Identical to the hash targets for all but one mod type: Raiden remaps by hash only. Its
        // pure-Python factory passes a bare "Indices()" with no map at all, where every other
        // factory passes the same map to both -- verified against the live Python runtime rather
        // than read off the source, since a missing map and an empty one look alike in a diff.
        if (value == ModTypeId::Raiden) {
            return {};
        }

        return getHashRemapTargets(value);
    }


    std::vector<std::string> ModTypeIdTools::getSectionKeywords(ModTypeId value) {
        // Transcribed mechanically from the pure-Python IniClassifierBuilderOld's 43
        // addGIModType(...) calls -- specifically the KEYS of each call's keyword dict. The
        // regexes those keys map to are deliberately not carried over: they exist to disambiguate
        // overlapping names ("amber" must not match "ambercn"), and IniClassifier's own maximal
        // match already does that. See core/tests/IniClassifierPopulation_test.cpp.
        switch (value) {
            case ModTypeId::Amber:
                return {"amber"};

            case ModTypeId::AmberCN:
                return {"ambercn"};

            case ModTypeId::Arlecchino:
                return {"arlecchino"};

            case ModTypeId::Ayaka:
                return {"ayaka"};

            case ModTypeId::AyakaSpringbloom:
                return {"ayakaspringbloom"};

            case ModTypeId::Barbara:
                return {"barbara"};

            case ModTypeId::BarbaraSummertime:
                return {"barbarasummertime"};

            case ModTypeId::CherryHuTao:
                return {"cherryhutao", "hutaocherry"};

            case ModTypeId::Diluc:
                return {"diluc"};

            case ModTypeId::DilucFlamme:
                return {"dilucflamme"};

            case ModTypeId::Fischl:
                return {"fischl"};

            case ModTypeId::FischlHighness:
                return {"fischlhighness"};

            case ModTypeId::Ganyu:
                return {"ganyu"};

            case ModTypeId::GanyuTwilight:
                return {"ganyutwilight"};

            case ModTypeId::HuTao:
                return {"hutao"};

            case ModTypeId::Jean:
                return {"jean"};

            case ModTypeId::JeanCN:
                return {"jeancn"};

            case ModTypeId::JeanSea:
                return {"jeansea"};

            case ModTypeId::Kaeya:
                return {"kaeya"};

            case ModTypeId::KaeyaSailwind:
                return {"kaeyasailwind"};

            case ModTypeId::Keqing:
                return {"keqing"};

            case ModTypeId::KeqingOpulent:
                return {"keqingopulent"};

            case ModTypeId::Kirara:
                return {"kirara"};

            case ModTypeId::KiraraBoots:
                return {"kiraraboots"};

            case ModTypeId::Klee:
                return {"klee"};

            case ModTypeId::KleeBlossomingStarlight:
                return {"kleeblossomingstarlight"};

            case ModTypeId::Lisa:
                return {"lisa"};

            case ModTypeId::LisaStudent:
                return {"lisastudent"};

            case ModTypeId::Mona:
                return {"mona"};

            case ModTypeId::MonaCN:
                return {"monacn"};

            case ModTypeId::Nilou:
                return {"nilou"};

            case ModTypeId::NilouBreeze:
                return {"niloubreeze"};

            case ModTypeId::Ningguang:
                return {"ningguang"};

            case ModTypeId::NingguangOrchid:
                return {"ningguangorchid"};

            case ModTypeId::Raiden:
                return {"raiden", "shogun"};

            case ModTypeId::Rosaria:
                return {"rosaria"};

            case ModTypeId::RosariaCN:
                return {"rosariacn"};

            case ModTypeId::Shenhe:
                return {"shenhe"};

            case ModTypeId::ShenheFrostFlower:
                return {"shenhefrostflower"};

            case ModTypeId::Xiangling:
                return {"xiangling"};

            case ModTypeId::XianglingCheer:
                return {"xianglingcheer", "xianglingnewyear"};

            case ModTypeId::Xingqiu:
                return {"xingqiu"};

            case ModTypeId::XingqiuBamboo:
                return {"xingqiubamboo"};
            // The two target-only ids (RaidenBoss, ArlecchinoBoss) have no keywords: nothing
            // classifies a .ini file AS them, they are only ever what a mod is remapped ONTO.
            default:
                return {};
        }
    }


    void ModTypeIdTools::registerModType(const ModType &modType) {
        _modTypes.insert_or_assign(modType.modTypeId, modType);

        std::unordered_set<int> modTypeIdSet;
        modTypeIdSet.insert(modType.modTypeId);

        // Filed in lowercase, and findByName lowercases what it is asked, so a name or alias
        // resolves whatever case it is typed in. The pure-Python 'ModTypes' this mirrors does
        // exactly this -- it builds its DFA from 'modType.name.lower()' and searches with
        // 'txt.lower()' -- and its own --help text promises it: "The names/aliases for the mod
        // types are not case sensitive".
        //
        // Nothing loses the original casing by this: getName and getModType both answer out of
        // '_modTypes', which is keyed by id and holds the ModType as it was registered. The DFA is
        // only ever a lookup FROM text.
        const std::string lowerName = StringTools::toLower(modType.name);
        _nameDFA.add(lowerName, modTypeIdSet);
        _nameGameTypeIds[lowerName].insert(modType.gameTypeId);

        for (const std::string &alias : modType.aliases) {
            const std::string lowerAlias = StringTools::toLower(alias);
            _nameDFA.add(lowerAlias, modTypeIdSet);
            _nameGameTypeIds[lowerAlias].insert(modType.gameTypeId);
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

        // Lowercased and trimmed to match how registerModType filed the keys, and to match the
        // pure-Python 'ModTypes.search(txt.lower().strip())' this stands in for.
        const std::string normalizedName = StringTools::toLower(StringTools::strip(name));

        auto [matchedNamePtr, matchedModTypeIdsPtr] = _nameDFA.getMaximalPtr(normalizedName, pred);

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

        // Last, so a reader that sees the new generation also sees an already-empty registry.
        ++_generation;
    }


    unsigned long long ModTypeIdTools::generation() {
        return _generation;
    }

}
