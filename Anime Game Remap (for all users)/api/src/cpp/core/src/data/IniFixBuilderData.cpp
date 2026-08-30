#include "AGRemapCore/data/IniFixBuilderData.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {
    // Every generator is a stub for now -- see IniFixBuilderFuncs' own warning. They are
    // written out one-per-method rather than collapsed into a single shared stub so that each
    // can be filled in independently.
    IniFixBuilder::Factory IniFixBuilderFuncs::amber4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::amberCN4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ayaka4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ayakaSpringbloom4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::barbara4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::barbaraSummertime4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::diluc4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::dilucFlamme4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::fischl4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::fischlHighness4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ganyu4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::hutao4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::jean4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::jeanCN4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::jeanSea4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::kaeya4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::kaeyaSailwind4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::keqing4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::keqingOpulent4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::kirara4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::klee4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::kleeBlossomingStarlight4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::lisa4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::lisaStudent4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::mona4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::monaCN4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::nilou4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ningguang4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ningguangOrchid4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::giDefault() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::rosaria4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::rosariaCN4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::shenhe4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::xiangling4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::xingqiu4_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ganyuTwilight4_4() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::shenheFrostFlower4_4() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::xingqiuBamboo4_4() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::kiraraBoots4_8() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::nilouBreeze4_8() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::kaeya5_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::kaeyaSailwind5_0() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::cherryHuTao5_3() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::xianglingCheer5_3() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ayaka5_4() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::arlecchino5_4() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::nilouBreeze5_4() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::lisa5_4() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::jean5_5() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::jeanCN5_5() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::hutao5_6() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ayaka5_6() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ayakaSpringbloom5_6() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::amber5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::amberCN5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ayaka5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ayakaSpringbloom5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::arlecchino5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::barbara5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::barbaraSummertime5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::diluc5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::dilucFlamme5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::fischl5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::fischlHighness5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ganyu5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::ganyuTwilight5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::kirara5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::kiraraBoots5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::lisa5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::nilou5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::nilouBreeze5_7() { return IniFixBuilder::defaultFactory(); }
    IniFixBuilder::Factory IniFixBuilderFuncs::shenheFrostFlower5_7() { return IniFixBuilder::defaultFactory(); }

    namespace {
        // Four index columns: fromVersion, fromModName, toVersion, toModName -- with the two
        // VERSION columns at positions 0 and 2. A fixer is chosen for a (source mod at a
        // source version) -> (target mod at a target version) pair, so both ends carry a
        // version, which is why this table is a ModAssets rather than a ModDictAssets.
        std::vector<Row<std::string, IniFixBuilder::Factory>> buildRows() {
            return {
                // ===== Amber @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Amber),
                  "4.0", ModTypeIdTools::getName(ModTypeId::AmberCN)}, IniFixBuilderFuncs::amber4_0()},

                // ===== AmberCN @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::AmberCN),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Amber)}, IniFixBuilderFuncs::amberCN4_0()},

                // ===== Ayaka @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Ayaka),
                  "4.0", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom)}, IniFixBuilderFuncs::ayaka4_0()},

                // ===== AyakaSpringBloom @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Ayaka)}, IniFixBuilderFuncs::ayakaSpringbloom4_0()},

                // ===== Barbara @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Barbara),
                  "4.0", ModTypeIdTools::getName(ModTypeId::BarbaraSummertime)}, IniFixBuilderFuncs::barbara4_0()},

                // ===== BarbaraSummertime @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::BarbaraSummertime),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Barbara)}, IniFixBuilderFuncs::barbaraSummertime4_0()},

                // ===== Diluc @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Diluc),
                  "4.0", ModTypeIdTools::getName(ModTypeId::DilucFlamme)}, IniFixBuilderFuncs::diluc4_0()},

                // ===== DilucFlamme @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::DilucFlamme),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Diluc)}, IniFixBuilderFuncs::dilucFlamme4_0()},

                // ===== Fischl @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Fischl),
                  "4.0", ModTypeIdTools::getName(ModTypeId::FischlHighness)}, IniFixBuilderFuncs::fischl4_0()},

                // ===== FischlHighness @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::FischlHighness),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Fischl)}, IniFixBuilderFuncs::fischlHighness4_0()},

                // ===== Ganyu @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Ganyu),
                  "4.0", ModTypeIdTools::getName(ModTypeId::GanyuTwilight)}, IniFixBuilderFuncs::ganyu4_0()},

                // ===== HuTao @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::HuTao),
                  "4.0", ModTypeIdTools::getName(ModTypeId::CherryHuTao)}, IniFixBuilderFuncs::hutao4_0()},

                // ===== Jean @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Jean),
                  "4.0", ModTypeIdTools::getName(ModTypeId::JeanCN)}, IniFixBuilderFuncs::jean4_0()},
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Jean),
                  "4.0", ModTypeIdTools::getName(ModTypeId::JeanSea)}, IniFixBuilderFuncs::jean4_0()},

                // ===== JeanCN @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::JeanCN),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Jean)}, IniFixBuilderFuncs::jeanCN4_0()},
                {{"1.0", ModTypeIdTools::getName(ModTypeId::JeanCN),
                  "4.0", ModTypeIdTools::getName(ModTypeId::JeanSea)}, IniFixBuilderFuncs::jeanCN4_0()},

                // ===== JeanSea @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::JeanSea),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Jean)}, IniFixBuilderFuncs::jeanSea4_0()},
                {{"1.0", ModTypeIdTools::getName(ModTypeId::JeanSea),
                  "4.0", ModTypeIdTools::getName(ModTypeId::JeanCN)}, IniFixBuilderFuncs::jeanSea4_0()},

                // ===== Kaeya @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Kaeya),
                  "4.0", ModTypeIdTools::getName(ModTypeId::KaeyaSailwind)}, IniFixBuilderFuncs::kaeya4_0()},

                // ===== KaeyaSailwind @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::KaeyaSailwind),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Kaeya)}, IniFixBuilderFuncs::kaeyaSailwind4_0()},

                // ===== Keqing @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Keqing),
                  "4.0", ModTypeIdTools::getName(ModTypeId::KeqingOpulent)}, IniFixBuilderFuncs::keqing4_0()},

                // ===== KeqingOpulent @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::KeqingOpulent),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Keqing)}, IniFixBuilderFuncs::keqingOpulent4_0()},

                // ===== Kirara @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Kirara),
                  "4.0", ModTypeIdTools::getName(ModTypeId::KiraraBoots)}, IniFixBuilderFuncs::kirara4_0()},

                // ===== Klee @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Klee),
                  "4.0", ModTypeIdTools::getName(ModTypeId::KleeBlossomingStarlight)}, IniFixBuilderFuncs::klee4_0()},

                // ===== KleeBlossomingStarlight @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::KleeBlossomingStarlight),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Klee)}, IniFixBuilderFuncs::kleeBlossomingStarlight4_0()},

                // ===== Lisa @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Lisa),
                  "4.0", ModTypeIdTools::getName(ModTypeId::LisaStudent)}, IniFixBuilderFuncs::lisa4_0()},

                // ===== LisaStudent @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::LisaStudent),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Lisa)}, IniFixBuilderFuncs::lisaStudent4_0()},

                // ===== Mona @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Mona),
                  "4.0", ModTypeIdTools::getName(ModTypeId::MonaCN)}, IniFixBuilderFuncs::mona4_0()},

                // ===== MonaCN @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::MonaCN),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Mona)}, IniFixBuilderFuncs::monaCN4_0()},

                // ===== Nilou @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Nilou),
                  "4.0", ModTypeIdTools::getName(ModTypeId::NilouBreeze)}, IniFixBuilderFuncs::nilou4_0()},

                // ===== Ningguang @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Ningguang),
                  "4.0", ModTypeIdTools::getName(ModTypeId::NingguangOrchid)}, IniFixBuilderFuncs::ningguang4_0()},

                // ===== NingguangOrchid @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::NingguangOrchid),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Ningguang)}, IniFixBuilderFuncs::ningguangOrchid4_0()},

                // ===== Raiden @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Raiden),
                  "4.0", ModTypeIdTools::getName(ModTypeId::RaidenBoss)}, IniFixBuilderFuncs::giDefault()},

                // ===== Rosaria @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Rosaria),
                  "4.0", ModTypeIdTools::getName(ModTypeId::RosariaCN)}, IniFixBuilderFuncs::rosaria4_0()},

                // ===== RosariaCN @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::RosariaCN),
                  "4.0", ModTypeIdTools::getName(ModTypeId::Rosaria)}, IniFixBuilderFuncs::rosariaCN4_0()},

                // ===== Shenhe @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Shenhe),
                  "4.0", ModTypeIdTools::getName(ModTypeId::ShenheFrostFlower)}, IniFixBuilderFuncs::shenhe4_0()},

                // ===== Xiangling @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Xiangling),
                  "4.0", ModTypeIdTools::getName(ModTypeId::XianglingCheer)}, IniFixBuilderFuncs::xiangling4_0()},

                // ===== Xingqiu @ toVersion 4.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Xingqiu),
                  "4.0", ModTypeIdTools::getName(ModTypeId::XingqiuBamboo)}, IniFixBuilderFuncs::xingqiu4_0()},

                // ===== GanyuTwilight @ toVersion 4.4 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::GanyuTwilight),
                  "4.4", ModTypeIdTools::getName(ModTypeId::Ganyu)}, IniFixBuilderFuncs::ganyuTwilight4_4()},

                // ===== ShenheFrostFlower @ toVersion 4.4 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::ShenheFrostFlower),
                  "4.4", ModTypeIdTools::getName(ModTypeId::Shenhe)}, IniFixBuilderFuncs::shenheFrostFlower4_4()},

                // ===== XingqiuBamboo @ toVersion 4.4 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::XingqiuBamboo),
                  "4.4", ModTypeIdTools::getName(ModTypeId::Xingqiu)}, IniFixBuilderFuncs::xingqiuBamboo4_4()},

                // ===== Arlecchino @ toVersion 4.6 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Arlecchino),
                  "4.6", ModTypeIdTools::getName(ModTypeId::ArlecchinoBoss)}, IniFixBuilderFuncs::giDefault()},

                // ===== KiraraBoots @ toVersion 4.8 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::KiraraBoots),
                  "4.8", ModTypeIdTools::getName(ModTypeId::Kirara)}, IniFixBuilderFuncs::kiraraBoots4_8()},

                // ===== NilouBreeze @ toVersion 4.8 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::NilouBreeze),
                  "4.8", ModTypeIdTools::getName(ModTypeId::Nilou)}, IniFixBuilderFuncs::nilouBreeze4_8()},

                // ===== Kaeya @ toVersion 5.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Kaeya),
                  "5.0", ModTypeIdTools::getName(ModTypeId::KaeyaSailwind)}, IniFixBuilderFuncs::kaeya5_0()},

                // ===== KaeyaSailwind @ toVersion 5.0 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::KaeyaSailwind),
                  "5.0", ModTypeIdTools::getName(ModTypeId::Kaeya)}, IniFixBuilderFuncs::kaeyaSailwind5_0()},

                // ===== CherryHuTao @ toVersion 5.3 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::CherryHuTao),
                  "5.3", ModTypeIdTools::getName(ModTypeId::HuTao)}, IniFixBuilderFuncs::cherryHuTao5_3()},

                // ===== XianglingCheer @ toVersion 5.3 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::XianglingCheer),
                  "5.3", ModTypeIdTools::getName(ModTypeId::Xiangling)}, IniFixBuilderFuncs::xianglingCheer5_3()},

                // ===== Ayaka @ toVersion 5.4 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Ayaka),
                  "5.4", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom)}, IniFixBuilderFuncs::ayaka5_4()},

                // ===== Arlecchino @ toVersion 5.4 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Arlecchino),
                  "5.4", ModTypeIdTools::getName(ModTypeId::ArlecchinoBoss)}, IniFixBuilderFuncs::arlecchino5_4()},

                // ===== NilouBreeze @ toVersion 5.4 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::NilouBreeze),
                  "5.4", ModTypeIdTools::getName(ModTypeId::Nilou)}, IniFixBuilderFuncs::nilouBreeze5_4()},

                // ===== Lisa @ toVersion 5.4 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Lisa),
                  "5.4", ModTypeIdTools::getName(ModTypeId::LisaStudent)}, IniFixBuilderFuncs::lisa5_4()},

                // ===== Jean @ toVersion 5.5 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Jean),
                  "5.5", ModTypeIdTools::getName(ModTypeId::JeanCN)}, IniFixBuilderFuncs::jean5_5()},
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Jean),
                  "5.5", ModTypeIdTools::getName(ModTypeId::JeanSea)}, IniFixBuilderFuncs::jean5_5()},

                // ===== JeanCN @ toVersion 5.5 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::JeanCN),
                  "5.5", ModTypeIdTools::getName(ModTypeId::Jean)}, IniFixBuilderFuncs::jeanCN5_5()},
                {{"1.0", ModTypeIdTools::getName(ModTypeId::JeanCN),
                  "5.5", ModTypeIdTools::getName(ModTypeId::JeanSea)}, IniFixBuilderFuncs::jeanCN5_5()},

                // ===== HuTao @ toVersion 5.6 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::HuTao),
                  "5.6", ModTypeIdTools::getName(ModTypeId::CherryHuTao)}, IniFixBuilderFuncs::hutao5_6()},

                // ===== Ayaka @ toVersion 5.6 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Ayaka),
                  "5.6", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom)}, IniFixBuilderFuncs::ayaka5_6()},

                // ===== AyakaSpringBloom @ toVersion 5.6 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom),
                  "5.6", ModTypeIdTools::getName(ModTypeId::Ayaka)}, IniFixBuilderFuncs::ayakaSpringbloom5_6()},

                // ===== Amber @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Amber),
                  "5.7", ModTypeIdTools::getName(ModTypeId::AmberCN)}, IniFixBuilderFuncs::amber5_7()},

                // ===== AmberCN @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::AmberCN),
                  "5.7", ModTypeIdTools::getName(ModTypeId::Amber)}, IniFixBuilderFuncs::amberCN5_7()},

                // ===== Ayaka @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Ayaka),
                  "5.7", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom)}, IniFixBuilderFuncs::ayaka5_7()},

                // ===== AyakaSpringBloom @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom),
                  "5.7", ModTypeIdTools::getName(ModTypeId::Ayaka)}, IniFixBuilderFuncs::ayakaSpringbloom5_7()},

                // ===== Arlecchino @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Arlecchino),
                  "5.7", ModTypeIdTools::getName(ModTypeId::ArlecchinoBoss)}, IniFixBuilderFuncs::arlecchino5_7()},

                // ===== Barbara @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Barbara),
                  "5.7", ModTypeIdTools::getName(ModTypeId::BarbaraSummertime)}, IniFixBuilderFuncs::barbara5_7()},

                // ===== BarbaraSummertime @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::BarbaraSummertime),
                  "5.7", ModTypeIdTools::getName(ModTypeId::Barbara)}, IniFixBuilderFuncs::barbaraSummertime5_7()},

                // ===== Diluc @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Diluc),
                  "5.7", ModTypeIdTools::getName(ModTypeId::DilucFlamme)}, IniFixBuilderFuncs::diluc5_7()},

                // ===== DilucFlamme @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::DilucFlamme),
                  "5.7", ModTypeIdTools::getName(ModTypeId::Diluc)}, IniFixBuilderFuncs::dilucFlamme5_7()},

                // ===== Fischl @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Fischl),
                  "5.7", ModTypeIdTools::getName(ModTypeId::FischlHighness)}, IniFixBuilderFuncs::fischl5_7()},

                // ===== FischlHighness @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::FischlHighness),
                  "5.7", ModTypeIdTools::getName(ModTypeId::Fischl)}, IniFixBuilderFuncs::fischlHighness5_7()},

                // ===== Ganyu @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Ganyu),
                  "5.7", ModTypeIdTools::getName(ModTypeId::GanyuTwilight)}, IniFixBuilderFuncs::ganyu5_7()},

                // ===== GanyuTwilight @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::GanyuTwilight),
                  "5.7", ModTypeIdTools::getName(ModTypeId::Ganyu)}, IniFixBuilderFuncs::ganyuTwilight5_7()},

                // ===== Kirara @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Kirara),
                  "5.7", ModTypeIdTools::getName(ModTypeId::KiraraBoots)}, IniFixBuilderFuncs::kirara5_7()},

                // ===== KiraraBoots @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::KiraraBoots),
                  "5.7", ModTypeIdTools::getName(ModTypeId::Kirara)}, IniFixBuilderFuncs::kiraraBoots5_7()},

                // ===== Lisa @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Lisa),
                  "5.7", ModTypeIdTools::getName(ModTypeId::LisaStudent)}, IniFixBuilderFuncs::lisa5_7()},

                // ===== Nilou @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::Nilou),
                  "5.7", ModTypeIdTools::getName(ModTypeId::NilouBreeze)}, IniFixBuilderFuncs::nilou5_7()},

                // ===== NilouBreeze @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::NilouBreeze),
                  "5.7", ModTypeIdTools::getName(ModTypeId::Nilou)}, IniFixBuilderFuncs::nilouBreeze5_7()},

                // ===== ShenheFrostFlower @ toVersion 5.7 =====
                {{"1.0", ModTypeIdTools::getName(ModTypeId::ShenheFrostFlower),
                  "5.7", ModTypeIdTools::getName(ModTypeId::Shenhe)}, IniFixBuilderFuncs::shenheFrostFlower5_7()},
            };
        }
    }

    const std::shared_ptr<const IniFixBuilder::ArgsRepo>& IniFixBuilderData::repo() {
        // Function-local static: built once, on first use, thread-safely -- and crucially after
        // ModTypeIdTools' own registry is ready, which the row keys depend on.
        static const std::shared_ptr<const IniFixBuilder::ArgsRepo> table =
            std::make_shared<const IniFixBuilder::ArgsRepo>(
                // isVersionColumn: fromVersion and toVersion are versions; the two mod names are not.
                std::vector<bool>{true, false, true, false},
                [](const std::string& raw) { return Version::parse(raw); },
                buildRows());

        return table;
    }
}
