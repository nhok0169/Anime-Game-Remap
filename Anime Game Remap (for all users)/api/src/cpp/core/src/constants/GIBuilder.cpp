#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalIniRemoveBuilders.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniRemoveBuilderData.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/assets/Indices.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"
#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"


namespace AGRemapCore {
    namespace {
        /*
         * The one IniParseBuilder every GI ModType shares, mirroring how the pure-Python GIBuilder
         * hands the same "IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value)" to all 43 of
         * its own ModType(...) calls.
         *
         * This is the *version-dependent* flavour, built over IniParseBuilderData's table -- the
         * C++ counterpart to the original passing "ModDataAssets.IniParseBuilderArgs.value". Every
         * generator in that table is still a stub returning a plain BaseIniParser (no concrete C++
         * GIMIParser/GIMIObjParser exists yet), so today every row resolves to the same thing; the
         * version *selection* around them is real, and filling a stub in immediately takes effect
         * for the mod types whose rows point at it.
         *
         * Function-local static rather than a namespace-scope one so it is constructed on first
         * use, after ModTypeIdTools' own registry is ready -- see PyGIBuilder's init-order note.
         * IniParseBuilderData::repo() depends on that registry for its row keys.
         */
        const std::shared_ptr<IniParseBuilder>& giIniParseBuilder() {
            static const std::shared_ptr<IniParseBuilder> builder =
                std::make_shared<IniParseBuilder>(IniParseBuilderData::repo());
            return builder;
        }

        /*
         * The fix-side counterpart of giIniParseBuilder -- same reasoning throughout, mirroring the
         * single shared "IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value)" the pure-Python
         * GIBuilder hands to all 43 of its own ModType(...) calls.
         */
        const std::shared_ptr<IniFixBuilder>& giIniFixBuilder() {
            static const std::shared_ptr<IniFixBuilder> builder =
                std::make_shared<IniFixBuilder>(IniFixBuilderData::repo());
            return builder;
        }

        /*
         * The remove-side counterpart, over IniRemoveBuilderData's table.
         *
         * Unlike the other two this has no pure-Python equivalent -- the original hands every mod
         * type the single global IniRemoveBuilder(RemapIniRemover) instead (which is still what
         * ModType's own null-fallback supplies, see GlobalIniRemoveBuilders). Using a table here is
         * a deliberate extension so a per-mod remover can be expressed when one exists; every row
         * is a stub today, so behaviour is identical to the global builder's.
         *
         * Note this is a *different* builder instance from GlobalIniRemoveBuilders::removeBuilder():
         * that one is the fixed-factory flavour, this one is table-backed. Neither caches, so the
         * only difference between them is which factory a given mod name resolves to.
         */
        const std::shared_ptr<IniRemoveBuilder>& giIniRemoveBuilder() {
            static const std::shared_ptr<IniRemoveBuilder> builder =
                std::make_shared<IniRemoveBuilder>(IniRemoveBuilderData::repo());
            return builder;
        }

        /*
         * Builds one GI ModType from the three shared builders.
         *
         * The pure-Python GIBuilder passes a real "IniParseBuilder(...)"/"IniFixBuilder(...)" to
         * every one of its own 43 ModType(...) calls, so those are passed explicitly here too
         * rather than leaning on ModType's own null-fallback.
         *
         * Nothing is constructed per mod type any more: IniFile builds a parser and fixer per file
         * from the first two builders, and asks the third for a remover per call. See
         * ModType::iniParseBuilder / ModType::iniRemoveBuilder.
         */
        /*
         * The "fromName -> [toName, ...]" map a ModMappedAssets is keyed by. Empty when this mod
         * type remaps onto nothing: ModMappedAssets::resolveToAssetNames returns nullopt for a
         * from-name that isn't a key, so an absent entry means "no targets", NOT "all targets".
         */
        std::unordered_map<std::string, std::vector<std::string>> makeRemapMap(const std::string& name,
                                                                               const std::vector<ModTypeId>& targets) {
            if (targets.empty()) {
                return {};
            }

            std::vector<std::string> targetNames;
            targetNames.reserve(targets.size());
            for (ModTypeId target : targets) {
                targetNames.push_back(ModTypeIdTools::getName(target));
            }

            return {{name, std::move(targetNames)}};
        }

        ModType makeGIModType(ModTypeId modTypeId, std::vector<std::string> aliases = {}) {
            const std::string name = ModTypeIdTools::getName(modTypeId);

            return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(modTypeId),
                           name, std::move(aliases),
                           // The remap graph -- which mod types this one can be fixed onto. The
                           // pure-Python GIBuilder spells this out per character as
                           // "Hashes(map = {...})" / "Indices(map = {...})"; here the same data
                           // lives in one table next to the ModTypeId enum, so a target is named by
                           // enumerator rather than by a bare string. Passing nullptr instead (as
                           // this used to) lands on ModType's bare Hashes()/Indices() defaults,
                           // whose empty map means the mod type can remap onto nothing at all.
                           std::make_shared<Hashes>(makeRemapMap(name, ModTypeIdTools::getHashRemapTargets(modTypeId))),
                           std::make_shared<Indices>(makeRemapMap(name, ModTypeIdTools::getIndexRemapTargets(modTypeId))),
                           // nullptr vertexCounts -> each GI mod type gets its own fully-populated
                           // table, matching the pure-Python GIBuilder (which passes none and so
                           // lands on ModType's own VertexCounts() default).
                           //
                           // nullptr vgRemaps is NOT the same thing: ModType's fallback there is the
                           // single shared ModDataAssets::vgRemaps, so all 43 GI mod types share one
                           // remap table. That too matches the original -- see ModType::vgRemaps.
                           nullptr, nullptr,
                           giIniParseBuilder(), giIniFixBuilder(), giIniRemoveBuilder());
        }
    }

    ModType GIBuilder::amber() {
        return makeGIModType(ModTypeId::Amber, {"BaronBunny", "ColleisBestie"});
    }

    ModType GIBuilder::amberCN() {
        return makeGIModType(ModTypeId::AmberCN, {"BaronBunnyCN", "ColleisBestieCN"});
    }

    ModType GIBuilder::ayaka() {
        return makeGIModType(ModTypeId::Ayaka, {"Ayaya", "Yandere", "NewArchonOfEternity"});
    }

    ModType GIBuilder::ayakaSpringBloom() {
        return makeGIModType(ModTypeId::AyakaSpringbloom, {"AyayaFontaine", "YandereFontaine", "NewArchonOfEternityFontaine", "FontaineAyaya", "FontaineYandere", "NewFontaineArchonOfEternity", "MusketeerAyaka", "AyakaMusketeer", "AyayaMusketeer"});
    }

    ModType GIBuilder::arlecchino() {
        return makeGIModType(ModTypeId::Arlecchino, {"Father", "Knave", "Perrie", "Peruere", "Harlequin"});
    }

    ModType GIBuilder::barbara() {
        return makeGIModType(ModTypeId::Barbara, {"Idol", "Healer"});
    }

    ModType GIBuilder::barbaraSummerTime() {
        return makeGIModType(ModTypeId::BarbaraSummertime, {"IdolSummertime", "HealerSummertime", "BarbaraBikini"});
    }

    ModType GIBuilder::cherryHutao() {
        return makeGIModType(ModTypeId::CherryHuTao, {"HutaoCherry", "HutaoSnowLaden", "SnowLadenHutao", "LanternRiteHutao", "HutaoLanternRite", "Cherry77thDirectoroftheWangshengFuneralParlor", "CherryQiqiKidnapper", "77thDirectoroftheWangshengFuneralParlorCherry", "QiqiKidnapperCherry", "LanternRite77thDirectoroftheWangshengFuneralParlor", "LanternRiteQiqiKidnapper", "77thDirectoroftheWangshengFuneralParlorLanternRite", "QiqiKidnapperLanternRite"});
    }

    ModType GIBuilder::diluc() {
        return makeGIModType(ModTypeId::Diluc, {"KaeyasBrother", "DawnWineryMaster", "AngelShareOwner", "DarkNightBlaze"});
    }

    ModType GIBuilder::dilucFlamme() {
        return makeGIModType(ModTypeId::DilucFlamme, {"RedDeadOfTheNight", "DarkNightHero"});
    }

    ModType GIBuilder::fischl() {
        return makeGIModType(ModTypeId::Fischl, {"Amy", "Chunibyo", "8thGraderSyndrome", "Delusional", "PrinzessinderVerurteilung", "MeinFraulein", "FischlvonLuftschlossNarfidort", "PrincessofCondemnation", "TheCondemedPrincess", "OzsMiss"});
    }

    ModType GIBuilder::fischlHighness() {
        return makeGIModType(ModTypeId::FischlHighness, {"PrincessAmy", "RealPrinzessinderVerurteilung", "Prinzessin", "PrincessFischlvonLuftschlossNarfidort", "PrinzessinFischlvonLuftschlossNarfidort", "ImmernachtreichPrincess", "PrinzessinderImmernachtreich", "PrincessoftheEverlastingNight", "OzsPrincess"});
    }

    ModType GIBuilder::ganyu() {
        return makeGIModType(ModTypeId::Ganyu, {"Cocogoat"});
    }

    ModType GIBuilder::ganyuTwilight() {
        return makeGIModType(ModTypeId::GanyuTwilight, {"GanyuLanternRite", "LanternRiteGanyu", "CocogoatTwilight", "CocogoatLanternRite", "LanternRiteCocogoat"});
    }

    ModType GIBuilder::huTao() {
        return makeGIModType(ModTypeId::HuTao, {"77thDirectoroftheWangshengFuneralParlor", "QiqiKidnapper"});
    }

    ModType GIBuilder::jean() {
        return makeGIModType(ModTypeId::Jean, {"ActingGrandMaster", "KleesBabySitter"});
    }

    ModType GIBuilder::jeanCN() {
        return makeGIModType(ModTypeId::JeanCN, {"ActingGrandMasterCN", "KleesBabySitterCN"});
    }

    ModType GIBuilder::jeanSea() {
        return makeGIModType(ModTypeId::JeanSea, {"ActingGrandMasterSea", "KleesBabySitterSea"});
    }

    ModType GIBuilder::kaeya() {
        return makeGIModType(ModTypeId::Kaeya, {"DilucsBrother", "CavalryCaptain"});
    }

    ModType GIBuilder::kaeyaSailwind() {
        return makeGIModType(ModTypeId::KaeyaSailwind, {"DilucsBrotherSailwind", "CavalryCaptainSailwind", "TheftKaeya", "TheftDilucsBrother", "TheftCavalryCaptain", "KaeyaTheft", "DilucsBrotherTheft", "CavalryCaptainTheft"});
    }

    ModType GIBuilder::keqing() {
        return makeGIModType(ModTypeId::Keqing, {"Kequeen", "ZhongliSimp", "MoraxSimp"});
    }

    ModType GIBuilder::keqingOpulent() {
        return makeGIModType(ModTypeId::KeqingOpulent, {"LanternRiteKeqing", "KeqingLaternRite", "CuterKequeen", "LanternRiteKequeen", "KequeenLanternRite", "KequeenOpulent", "CuterKeqing", "ZhongliSimpOpulent", "MoraxSimpOpulent", "ZhongliSimpLaternRite", "MoraxSimpLaternRite", "LaternRiteZhongliSimp", "LaternRiteMoraxSimp"});
    }

    ModType GIBuilder::kirara() {
        return makeGIModType(ModTypeId::Kirara, {"Nekomata", "KonomiyaExpress", "CatBox"});
    }

    ModType GIBuilder::kiraraBoots() {
        return makeGIModType(ModTypeId::KiraraBoots, {"NekomataInBoots", "KonomiyaExpressInBoots", "CatBoxWithBoots", "PussInBoots"});
    }

    ModType GIBuilder::klee() {
        return makeGIModType(ModTypeId::Klee, {"SparkKnight", "DodocoBuddy", "DestroyerofWorlds"});
    }

    ModType GIBuilder::kleeBlossomingStarlight() {
        return makeGIModType(ModTypeId::KleeBlossomingStarlight, {"RedVelvetMage", "DodocoLittleWitchBuddy", "MagicDestroyerofWorlds", "FlandreScarlet", "ScarletFlandre"});
    }

    ModType GIBuilder::lisa() {
        return makeGIModType(ModTypeId::Lisa, {"CutieLibrarian"});
    }

    ModType GIBuilder::lisaStudent() {
        return makeGIModType(ModTypeId::LisaStudent, {"LisaSumeru", "SumeruLisa", "AkademiyaLisa", "LisaAkademiya"});
    }

    ModType GIBuilder::mona() {
        return makeGIModType(ModTypeId::Mona, {"NoMora", "BigHat"});
    }

    ModType GIBuilder::monaCN() {
        return makeGIModType(ModTypeId::MonaCN, {"NoMoraCN", "BigHatCN"});
    }

    ModType GIBuilder::nilou() {
        return makeGIModType(ModTypeId::Nilou, {"Dancer", "Morgiana", "BloomGirl"});
    }

    ModType GIBuilder::nilouBreeze() {
        return makeGIModType(ModTypeId::NilouBreeze, {"ForestFairy", "NilouFairy", "DancerBreeze", "MorgianaBreeze", "BloomGirlBreeze", "DancerFairy", "MorgianaFairy", "BloomGirlFairy", "FairyNilou", "FairyDancer", "FairyMorgiana", "FairyBloomGirl"});
    }

    ModType GIBuilder::ningguang() {
        return makeGIModType(ModTypeId::Ningguang, {"GeoMommy", "SugarMommy"});
    }

    ModType GIBuilder::ningguangOrchid() {
        return makeGIModType(ModTypeId::NingguangOrchid, {"NingguangLanternRite", "LanternRiteNingguang", "GeoMommyOrchid", "SugarMommyOrchid", "GeoMommyLaternRite", "SugarMommyLanternRite", "LaternRiteGeoMommy", "LanternRiteSugarMommy"});
    }

    ModType GIBuilder::raiden() {
        return makeGIModType(ModTypeId::Raiden, {"Ei", "RaidenEi", "Shogun", "RaidenShogun", "RaidenShotgun", "Shotgun", "CrydenShogun", "Cryden", "SmolEi"});
    }

    ModType GIBuilder::rosaria() {
        return makeGIModType(ModTypeId::Rosaria, {"GothGirl"});
    }

    ModType GIBuilder::rosariaCN() {
        return makeGIModType(ModTypeId::RosariaCN, {"GothGirlCN"});
    }

    ModType GIBuilder::shenhe() {
        return makeGIModType(ModTypeId::Shenhe, {"YelansBestie", "RedRopes"});
    }

    ModType GIBuilder::shenheFrostFlower() {
        return makeGIModType(ModTypeId::ShenheFrostFlower, {"ShenheLanternRite", "LanternRiteShenhe", "YelansBestieFrostFlower", "YelansBestieLanternRite", "LanternRiteYelansBestie", "RedRopesFrostFlower", "RedRopesLanternRite", "LanternRiteRedRopes"});
    }

    ModType GIBuilder::xiangling() {
        return makeGIModType(ModTypeId::Xiangling, {"CookingFanatic", "HeadChefoftheWanminRestaurant", "ChefMaosDaughter", "GuobasBuddy"});
    }

    ModType GIBuilder::xianglingCheer() {
        return makeGIModType(ModTypeId::XianglingCheer, {"XianglingLanternRite", "LanternRiteXiangling", "CookingFanaticLanternRite", "HeadChefoftheWanminRestaurantLanternRite", "ChefMaosDaughterLanternRite", "GuobasBuddyLanternRite", "LanternRiteCookingFanatic", "LanternRiteHeadChefoftheWanminRestaurant", "LanternRiteChefMaosDaughter", "LanternRiteGuobasBuddy"});
    }

    ModType GIBuilder::xingqiu() {
        return makeGIModType(ModTypeId::Xingqiu, {"GuhuaGeek", "Bookworm", "SecondSonofTheFeiyunCommerceGuild", "ChongyunsBestie"});
    }

    ModType GIBuilder::xingqiuBamboo() {
        return makeGIModType(ModTypeId::XingqiuBamboo, {"XingqiuLanternRite", "GuhuaGeekLanternRite", "BookwormLanternRite", "SecondSonofTheFeiyunCommerceGuildLanternRite", "ChongyunsBestieLanternRite", "LanternRiteXingqiu", "LanternRiteGuhuaGeek", "LanternRiteBookworm", "LanternRiteSecondSonofTheFeiyunCommerceGuild", "LanternRiteChongyunsBestie", "GuhuaGeekBamboo", "BookwormBamboo", "SecondSonofTheFeiyunCommerceGuildBamboo", "ChongyunsBestieBamboo"});
    }

    std::vector<ModType> GIBuilder::all() {
        // Listed explicitly rather than derived from ModTypeId's range: two of that enum's members
        // (RaidenBoss, ArlecchinoBoss) are remap *targets* only and have no factory here, so
        // iterating the enum would try to build mod types that do not exist.
        return {
            amber(),
            amberCN(),
            ayaka(),
            ayakaSpringBloom(),
            arlecchino(),
            barbara(),
            barbaraSummerTime(),
            cherryHutao(),
            diluc(),
            dilucFlamme(),
            fischl(),
            fischlHighness(),
            ganyu(),
            ganyuTwilight(),
            huTao(),
            jean(),
            jeanCN(),
            jeanSea(),
            kaeya(),
            kaeyaSailwind(),
            keqing(),
            keqingOpulent(),
            kirara(),
            kiraraBoots(),
            klee(),
            kleeBlossomingStarlight(),
            lisa(),
            lisaStudent(),
            mona(),
            monaCN(),
            nilou(),
            nilouBreeze(),
            ningguang(),
            ningguangOrchid(),
            raiden(),
            rosaria(),
            rosariaCN(),
            shenhe(),
            shenheFrostFlower(),
            xiangling(),
            xianglingCheer(),
            xingqiu(),
            xingqiuBamboo()
        };
    }
}
