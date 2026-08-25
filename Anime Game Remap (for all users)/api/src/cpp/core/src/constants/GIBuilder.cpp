#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/constants/GameTypeId.h"


namespace AGRemapCore {
    ModType GIBuilder::amber() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), ModTypeIdTools::getName(ModTypeId::Amber), {"BaronBunny", "ColleisBestie"});
    }

    ModType GIBuilder::amberCN() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::AmberCN), ModTypeIdTools::getName(ModTypeId::AmberCN), {"BaronBunnyCN", "ColleisBestieCN"});
    }

    ModType GIBuilder::ayaka() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Ayaka), ModTypeIdTools::getName(ModTypeId::Ayaka), {"Ayaya", "Yandere", "NewArchonOfEternity"});
    }

    ModType GIBuilder::ayakaSpringBloom() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::AyakaSpringbloom), ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom), {"AyayaFontaine", "YandereFontaine", "NewArchonOfEternityFontaine", "FontaineAyaya", "FontaineYandere", "NewFontaineArchonOfEternity", "MusketeerAyaka", "AyakaMusketeer", "AyayaMusketeer"});
    }

    ModType GIBuilder::arlecchino() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Arlecchino), ModTypeIdTools::getName(ModTypeId::Arlecchino), {"Father", "Knave", "Perrie", "Peruere", "Harlequin"});
    }

    ModType GIBuilder::barbara() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Barbara), ModTypeIdTools::getName(ModTypeId::Barbara), {"Idol", "Healer"});
    }

    ModType GIBuilder::barbaraSummerTime() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::BarbaraSummertime), ModTypeIdTools::getName(ModTypeId::BarbaraSummertime), {"IdolSummertime", "HealerSummertime", "BarbaraBikini"});
    }

    ModType GIBuilder::cherryHutao() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::CherryHuTao), ModTypeIdTools::getName(ModTypeId::CherryHuTao), {"HutaoCherry", "HutaoSnowLaden", "SnowLadenHutao", "LanternRiteHutao", "HutaoLanternRite", "Cherry77thDirectoroftheWangshengFuneralParlor", "CherryQiqiKidnapper", "77thDirectoroftheWangshengFuneralParlorCherry", "QiqiKidnapperCherry", "LanternRite77thDirectoroftheWangshengFuneralParlor", "LanternRiteQiqiKidnapper", "77thDirectoroftheWangshengFuneralParlorLanternRite", "QiqiKidnapperLanternRite"});
    }

    ModType GIBuilder::diluc() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Diluc), ModTypeIdTools::getName(ModTypeId::Diluc), {"KaeyasBrother", "DawnWineryMaster", "AngelShareOwner", "DarkNightBlaze"});
    }

    ModType GIBuilder::dilucFlamme() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::DilucFlamme), ModTypeIdTools::getName(ModTypeId::DilucFlamme), {"RedDeadOfTheNight", "DarkNightHero"});
    }

    ModType GIBuilder::fischl() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Fischl), ModTypeIdTools::getName(ModTypeId::Fischl), {"Amy", "Chunibyo", "8thGraderSyndrome", "Delusional", "PrinzessinderVerurteilung", "MeinFraulein", "FischlvonLuftschlossNarfidort", "PrincessofCondemnation", "TheCondemedPrincess", "OzsMiss"});
    }

    ModType GIBuilder::fischlHighness() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::FischlHighness), ModTypeIdTools::getName(ModTypeId::FischlHighness), {"PrincessAmy", "RealPrinzessinderVerurteilung", "Prinzessin", "PrincessFischlvonLuftschlossNarfidort", "PrinzessinFischlvonLuftschlossNarfidort", "ImmernachtreichPrincess", "PrinzessinderImmernachtreich", "PrincessoftheEverlastingNight", "OzsPrincess"});
    }

    ModType GIBuilder::ganyu() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Ganyu), ModTypeIdTools::getName(ModTypeId::Ganyu), {"Cocogoat"});
    }

    ModType GIBuilder::ganyuTwilight() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::GanyuTwilight), ModTypeIdTools::getName(ModTypeId::GanyuTwilight), {"GanyuLanternRite", "LanternRiteGanyu", "CocogoatTwilight", "CocogoatLanternRite", "LanternRiteCocogoat"});
    }

    ModType GIBuilder::huTao() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::HuTao), ModTypeIdTools::getName(ModTypeId::HuTao), {"77thDirectoroftheWangshengFuneralParlor", "QiqiKidnapper"});
    }

    ModType GIBuilder::jean() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Jean), ModTypeIdTools::getName(ModTypeId::Jean), {"ActingGrandMaster", "KleesBabySitter"});
    }

    ModType GIBuilder::jeanCN() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::JeanCN), ModTypeIdTools::getName(ModTypeId::JeanCN), {"ActingGrandMasterCN", "KleesBabySitterCN"});
    }

    ModType GIBuilder::jeanSea() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::JeanSea), ModTypeIdTools::getName(ModTypeId::JeanSea), {"ActingGrandMasterSea", "KleesBabySitterSea"});
    }

    ModType GIBuilder::kaeya() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Kaeya), ModTypeIdTools::getName(ModTypeId::Kaeya), {"DilucsBrother", "CavalryCaptain"});
    }

    ModType GIBuilder::kaeyaSailwind() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::KaeyaSailwind), ModTypeIdTools::getName(ModTypeId::KaeyaSailwind), {"DilucsBrotherSailwind", "CavalryCaptainSailwind", "TheftKaeya", "TheftDilucsBrother", "TheftCavalryCaptain", "KaeyaTheft", "DilucsBrotherTheft", "CavalryCaptainTheft"});
    }

    ModType GIBuilder::keqing() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Keqing), ModTypeIdTools::getName(ModTypeId::Keqing), {"Kequeen", "ZhongliSimp", "MoraxSimp"});
    }

    ModType GIBuilder::keqingOpulent() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::KeqingOpulent), ModTypeIdTools::getName(ModTypeId::KeqingOpulent), {"LanternRiteKeqing", "KeqingLaternRite", "CuterKequeen", "LanternRiteKequeen", "KequeenLanternRite", "KequeenOpulent", "CuterKeqing", "ZhongliSimpOpulent", "MoraxSimpOpulent", "ZhongliSimpLaternRite", "MoraxSimpLaternRite", "LaternRiteZhongliSimp", "LaternRiteMoraxSimp"});
    }

    ModType GIBuilder::kirara() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Kirara), ModTypeIdTools::getName(ModTypeId::Kirara), {"Nekomata", "KonomiyaExpress", "CatBox"});
    }

    ModType GIBuilder::kiraraBoots() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::KiraraBoots), ModTypeIdTools::getName(ModTypeId::KiraraBoots), {"NekomataInBoots", "KonomiyaExpressInBoots", "CatBoxWithBoots", "PussInBoots"});
    }

    ModType GIBuilder::klee() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Klee), ModTypeIdTools::getName(ModTypeId::Klee), {"SparkKnight", "DodocoBuddy", "DestroyerofWorlds"});
    }

    ModType GIBuilder::kleeBlossomingStarlight() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::KleeBlossomingStarlight), ModTypeIdTools::getName(ModTypeId::KleeBlossomingStarlight), {"RedVelvetMage", "DodocoLittleWitchBuddy", "MagicDestroyerofWorlds", "FlandreScarlet", "ScarletFlandre"});
    }

    ModType GIBuilder::lisa() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Lisa), ModTypeIdTools::getName(ModTypeId::Lisa), {"CutieLibrarian"});
    }

    ModType GIBuilder::lisaStudent() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::LisaStudent), ModTypeIdTools::getName(ModTypeId::LisaStudent), {"LisaSumeru", "SumeruLisa", "AkademiyaLisa", "LisaAkademiya"});
    }

    ModType GIBuilder::mona() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Mona), ModTypeIdTools::getName(ModTypeId::Mona), {"NoMora", "BigHat"});
    }

    ModType GIBuilder::monaCN() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::MonaCN), ModTypeIdTools::getName(ModTypeId::MonaCN), {"NoMoraCN", "BigHatCN"});
    }

    ModType GIBuilder::nilou() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Nilou), ModTypeIdTools::getName(ModTypeId::Nilou), {"Dancer", "Morgiana", "BloomGirl"});
    }

    ModType GIBuilder::nilouBreeze() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::NilouBreeze), ModTypeIdTools::getName(ModTypeId::NilouBreeze), {"ForestFairy", "NilouFairy", "DancerBreeze", "MorgianaBreeze", "BloomGirlBreeze", "DancerFairy", "MorgianaFairy", "BloomGirlFairy", "FairyNilou", "FairyDancer", "FairyMorgiana", "FairyBloomGirl"});
    }

    ModType GIBuilder::ningguang() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Ningguang), ModTypeIdTools::getName(ModTypeId::Ningguang), {"GeoMommy", "SugarMommy"});
    }

    ModType GIBuilder::ningguangOrchid() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::NingguangOrchid), ModTypeIdTools::getName(ModTypeId::NingguangOrchid), {"NingguangLanternRite", "LanternRiteNingguang", "GeoMommyOrchid", "SugarMommyOrchid", "GeoMommyLaternRite", "SugarMommyLanternRite", "LaternRiteGeoMommy", "LanternRiteSugarMommy"});
    }

    ModType GIBuilder::raiden() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Raiden), ModTypeIdTools::getName(ModTypeId::Raiden), {"Ei", "RaidenEi", "Shogun", "RaidenShogun", "RaidenShotgun", "Shotgun", "CrydenShogun", "Cryden", "SmolEi"});
    }

    ModType GIBuilder::rosaria() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Rosaria), ModTypeIdTools::getName(ModTypeId::Rosaria), {"GothGirl"});
    }

    ModType GIBuilder::rosariaCN() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::RosariaCN), ModTypeIdTools::getName(ModTypeId::RosariaCN), {"GothGirlCN"});
    }

    ModType GIBuilder::shenhe() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Shenhe), ModTypeIdTools::getName(ModTypeId::Shenhe), {"YelansBestie", "RedRopes"});
    }

    ModType GIBuilder::shenheFrostFlower() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::ShenheFrostFlower), ModTypeIdTools::getName(ModTypeId::ShenheFrostFlower), {"ShenheLanternRite", "LanternRiteShenhe", "YelansBestieFrostFlower", "YelansBestieLanternRite", "LanternRiteYelansBestie", "RedRopesFrostFlower", "RedRopesLanternRite", "LanternRiteRedRopes"});
    }

    ModType GIBuilder::xiangling() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Xiangling), ModTypeIdTools::getName(ModTypeId::Xiangling), {"CookingFanatic", "HeadChefoftheWanminRestaurant", "ChefMaosDaughter", "GuobasBuddy"});
    }

    ModType GIBuilder::xianglingCheer() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::XianglingCheer), ModTypeIdTools::getName(ModTypeId::XianglingCheer), {"XianglingLanternRite", "LanternRiteXiangling", "CookingFanaticLanternRite", "HeadChefoftheWanminRestaurantLanternRite", "ChefMaosDaughterLanternRite", "GuobasBuddyLanternRite", "LanternRiteCookingFanatic", "LanternRiteHeadChefoftheWanminRestaurant", "LanternRiteChefMaosDaughter", "LanternRiteGuobasBuddy"});
    }

    ModType GIBuilder::xingqiu() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Xingqiu), ModTypeIdTools::getName(ModTypeId::Xingqiu), {"GuhuaGeek", "Bookworm", "SecondSonofTheFeiyunCommerceGuild", "ChongyunsBestie"});
    }

    ModType GIBuilder::xingqiuBamboo() {
        return ModType(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::XingqiuBamboo), ModTypeIdTools::getName(ModTypeId::XingqiuBamboo), {"XingqiuLanternRite", "GuhuaGeekLanternRite", "BookwormLanternRite", "SecondSonofTheFeiyunCommerceGuildLanternRite", "ChongyunsBestieLanternRite", "LanternRiteXingqiu", "LanternRiteGuhuaGeek", "LanternRiteBookworm", "LanternRiteSecondSonofTheFeiyunCommerceGuild", "LanternRiteChongyunsBestie", "GuhuaGeekBamboo", "BookwormBamboo", "SecondSonofTheFeiyunCommerceGuildBamboo", "ChongyunsBestieBamboo"});
    }

}
