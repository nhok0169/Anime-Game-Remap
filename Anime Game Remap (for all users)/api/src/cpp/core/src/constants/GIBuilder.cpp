#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/constants/GameTypeId.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {
    namespace {
        /*
         * Builds one GI ModType, giving it the plain BaseIniParser/BaseIniFixer.
         *
         * The pure-Python GIBuilder passes a real "IniParseBuilder(...)"/"IniFixBuilder(...)" to
         * every one of its own 43 ModType(...) calls, so these are passed explicitly here too
         * rather than leaning on ModType's own null-fallback. They're the *base* classes for now
         * simply because no concrete C++ GIMIParser/GIMIFixer equivalent has been ported yet --
         * swap the two make_shared calls below when one lands, and all 43 mod types pick it up.
         *
         * Both are constructed unbound (no IniFile), since a ModType describes a kind of mod
         * rather than one specific .ini file -- see ModType::iniParser. The fixer holds a
         * non-owning pointer into the parser, and the ModType owns both shared_ptrs together, so
         * they share a lifetime.
         */
        ModType makeGIModType(ModTypeId modTypeId, std::vector<std::string> aliases = {}) {
            auto parser = std::make_shared<BaseIniParser>();
            auto fixer = std::make_shared<BaseIniFixer>(parser.get());

            return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(modTypeId),
                           ModTypeIdTools::getName(modTypeId), std::move(aliases),
                           std::move(parser), std::move(fixer));
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

}
