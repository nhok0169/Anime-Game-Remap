#include "AGRemapCore/data/IniParseBuilderData.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {
    // Every generator is a stub for now -- see IniParseBuilderFuncs' own warning. They are
    // written out one-per-method rather than collapsed into a single shared stub so that each
    // can be filled in independently, and so the table below reads exactly like the
    // pure-Python original's.
    IniParseBuilder::Factory IniParseBuilderFuncs::amber4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::amberCN4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::ayaka4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::ayakaSpringbloom4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::barbara4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::barbaraSummertime4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::diluc4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::dilucFlamme4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::fischl4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::fischlHighness4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::ganyu4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::hutao4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::jean4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::jeanCN4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::jeanSea4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::kaeya4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::kaeyaSailwind4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::keqing4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::keqingOpulent4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::kirara4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::klee4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::kleeBlossomingStarlight4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::lisa4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::lisaStudent4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::mona4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::monaCN4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::nilou4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::ningguang4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::ningguangOrchid4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::giDefault() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::rosaria4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::rosariaCN4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::shenhe4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::xiangling4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::xingqiu4_0() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::ganyuTwilight4_4() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::shenheFrostFlower4_4() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::xingqiuBamboo4_4() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::kiraraBoots4_8() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::nilouBreeze4_8() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::cherryHutao5_3() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::xianglingCheer5_3() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::arlecchino5_4() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::jean5_5() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::jeanCN5_5() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::ayakaSpringbloom5_6() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::ayakaSpringbloom5_7() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::ganyuTwilight5_7() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::kirara5_7() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::kiraraBoots5_7() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::lisaStudent5_7() { return IniParseBuilder::defaultFactory(); }
    IniParseBuilder::Factory IniParseBuilderFuncs::nilou5_7() { return IniParseBuilder::defaultFactory(); }

    namespace {
        // The version index sits at position 0 and the mod name at position 1, matching the
        // pure-Python ModAssets' own ["version", "name"] index order.
        std::vector<Row<std::string, IniParseBuilder::Factory>> buildRows() {
            return {
                // ===== 4.0 =====
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Amber)}, IniParseBuilderFuncs::amber4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::AmberCN)}, IniParseBuilderFuncs::amberCN4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Ayaka)}, IniParseBuilderFuncs::ayaka4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom)}, IniParseBuilderFuncs::ayakaSpringbloom4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Barbara)}, IniParseBuilderFuncs::barbara4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::BarbaraSummertime)}, IniParseBuilderFuncs::barbaraSummertime4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Diluc)}, IniParseBuilderFuncs::diluc4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::DilucFlamme)}, IniParseBuilderFuncs::dilucFlamme4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Fischl)}, IniParseBuilderFuncs::fischl4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::FischlHighness)}, IniParseBuilderFuncs::fischlHighness4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Ganyu)}, IniParseBuilderFuncs::ganyu4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::HuTao)}, IniParseBuilderFuncs::hutao4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Jean)}, IniParseBuilderFuncs::jean4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::JeanCN)}, IniParseBuilderFuncs::jeanCN4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::JeanSea)}, IniParseBuilderFuncs::jeanSea4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Kaeya)}, IniParseBuilderFuncs::kaeya4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::KaeyaSailwind)}, IniParseBuilderFuncs::kaeyaSailwind4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Keqing)}, IniParseBuilderFuncs::keqing4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::KeqingOpulent)}, IniParseBuilderFuncs::keqingOpulent4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Kirara)}, IniParseBuilderFuncs::kirara4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Klee)}, IniParseBuilderFuncs::klee4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::KleeBlossomingStarlight)}, IniParseBuilderFuncs::kleeBlossomingStarlight4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Lisa)}, IniParseBuilderFuncs::lisa4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::LisaStudent)}, IniParseBuilderFuncs::lisaStudent4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Mona)}, IniParseBuilderFuncs::mona4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::MonaCN)}, IniParseBuilderFuncs::monaCN4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Nilou)}, IniParseBuilderFuncs::nilou4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Ningguang)}, IniParseBuilderFuncs::ningguang4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::NingguangOrchid)}, IniParseBuilderFuncs::ningguangOrchid4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Raiden)}, IniParseBuilderFuncs::giDefault()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Rosaria)}, IniParseBuilderFuncs::rosaria4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::RosariaCN)}, IniParseBuilderFuncs::rosariaCN4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Shenhe)}, IniParseBuilderFuncs::shenhe4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Xiangling)}, IniParseBuilderFuncs::xiangling4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Xingqiu)}, IniParseBuilderFuncs::xingqiu4_0()},

                // ===== 4.4 =====
                {{"4.4", ModTypeIdTools::getName(ModTypeId::GanyuTwilight)}, IniParseBuilderFuncs::ganyuTwilight4_4()},
                {{"4.4", ModTypeIdTools::getName(ModTypeId::ShenheFrostFlower)}, IniParseBuilderFuncs::shenheFrostFlower4_4()},
                {{"4.4", ModTypeIdTools::getName(ModTypeId::XingqiuBamboo)}, IniParseBuilderFuncs::xingqiuBamboo4_4()},

                // ===== 4.6 =====
                {{"4.6", ModTypeIdTools::getName(ModTypeId::Arlecchino)}, IniParseBuilderFuncs::giDefault()},

                // ===== 4.8 =====
                {{"4.8", ModTypeIdTools::getName(ModTypeId::KiraraBoots)}, IniParseBuilderFuncs::kiraraBoots4_8()},
                {{"4.8", ModTypeIdTools::getName(ModTypeId::NilouBreeze)}, IniParseBuilderFuncs::nilouBreeze4_8()},

                // ===== 5.3 =====
                {{"5.3", ModTypeIdTools::getName(ModTypeId::CherryHuTao)}, IniParseBuilderFuncs::cherryHutao5_3()},
                {{"5.3", ModTypeIdTools::getName(ModTypeId::XianglingCheer)}, IniParseBuilderFuncs::xianglingCheer5_3()},

                // ===== 5.4 =====
                {{"5.4", ModTypeIdTools::getName(ModTypeId::Arlecchino)}, IniParseBuilderFuncs::arlecchino5_4()},

                // ===== 5.5 =====
                {{"5.5", ModTypeIdTools::getName(ModTypeId::Jean)}, IniParseBuilderFuncs::jean5_5()},
                {{"5.5", ModTypeIdTools::getName(ModTypeId::JeanCN)}, IniParseBuilderFuncs::jeanCN5_5()},

                // ===== 5.6 =====
                {{"5.6", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom)}, IniParseBuilderFuncs::ayakaSpringbloom5_6()},

                // ===== 5.7 =====
                {{"5.7", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom)}, IniParseBuilderFuncs::ayakaSpringbloom5_7()},
                {{"5.7", ModTypeIdTools::getName(ModTypeId::GanyuTwilight)}, IniParseBuilderFuncs::ganyuTwilight5_7()},
                {{"5.7", ModTypeIdTools::getName(ModTypeId::Kirara)}, IniParseBuilderFuncs::kirara5_7()},
                {{"5.7", ModTypeIdTools::getName(ModTypeId::KiraraBoots)}, IniParseBuilderFuncs::kiraraBoots5_7()},
                {{"5.7", ModTypeIdTools::getName(ModTypeId::LisaStudent)}, IniParseBuilderFuncs::lisaStudent5_7()},
                {{"5.7", ModTypeIdTools::getName(ModTypeId::Nilou)}, IniParseBuilderFuncs::nilou5_7()},
            };
        }
    }

    const std::shared_ptr<const IniParseBuilder::ArgsRepo>& IniParseBuilderData::repo() {
        // Function-local static: built once, on first use, thread-safely -- and crucially after
        // ModTypeIdTools' own registry is ready, which the row keys depend on.
        static const std::shared_ptr<const IniParseBuilder::ArgsRepo> table =
            std::make_shared<const IniParseBuilder::ArgsRepo>(
                /*totalIndices*/ 2, /*versionIndexPos*/ 0,
                [](const std::string& raw) { return Version::parse(raw); },
                buildRows());

        return table;
    }
}
