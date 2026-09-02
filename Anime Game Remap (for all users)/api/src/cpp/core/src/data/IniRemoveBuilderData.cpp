#include "AGRemapCore/data/IniRemoveBuilderData.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {
    // Every generator resolves the same way -- to RemapIniRemover, the only concrete remover there
    // is, which is also the only one the pure-Python side has. They are written out one-per-method
    // rather than collapsed into a single shared function so that any one of them can later be
    // pointed somewhere else independently, and so the table below reads exactly like the
    // pure-Python original's.
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::amber4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::amberCN4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::ayaka4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::ayakaSpringbloom4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::arlecchino4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::barbara4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::barbaraSummertime4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::cherryHuTao4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::diluc4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::dilucFlamme4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::fischl4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::fischlHighness4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::ganyu4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::ganyuTwilight4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::huTao4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::jean4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::jeanCN4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::jeanSea4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::kaeya4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::kaeyaSailwind4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::keqing4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::keqingOpulent4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::kirara4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::kiraraBoots4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::klee4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::kleeBlossomingStarlight4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::lisa4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::lisaStudent4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::mona4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::monaCN4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::nilou4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::nilouBreeze4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::ningguang4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::ningguangOrchid4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::raiden4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::rosaria4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::rosariaCN4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::shenhe4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::shenheFrostFlower4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::xiangling4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::xianglingCheer4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::xingqiu4_0() { return IniRemoveBuilder::defaultFactory(); }
    IniRemoveBuilder::Factory IniRemoveBuilderFuncs::xingqiuBamboo4_0() { return IniRemoveBuilder::defaultFactory(); }

    namespace {
        // The version index sits at position 0 and the mod name at position 1, matching the
        // pure-Python ModAssets' own ["version", "name"] index order.
        std::vector<Row<std::string, IniRemoveBuilder::Factory>> buildRows() {
            return {
                // ===== 4.0 =====
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Amber)}, IniRemoveBuilderFuncs::amber4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::AmberCN)}, IniRemoveBuilderFuncs::amberCN4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Ayaka)}, IniRemoveBuilderFuncs::ayaka4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom)}, IniRemoveBuilderFuncs::ayakaSpringbloom4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Arlecchino)}, IniRemoveBuilderFuncs::arlecchino4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Barbara)}, IniRemoveBuilderFuncs::barbara4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::BarbaraSummertime)}, IniRemoveBuilderFuncs::barbaraSummertime4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::CherryHuTao)}, IniRemoveBuilderFuncs::cherryHuTao4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Diluc)}, IniRemoveBuilderFuncs::diluc4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::DilucFlamme)}, IniRemoveBuilderFuncs::dilucFlamme4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Fischl)}, IniRemoveBuilderFuncs::fischl4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::FischlHighness)}, IniRemoveBuilderFuncs::fischlHighness4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Ganyu)}, IniRemoveBuilderFuncs::ganyu4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::GanyuTwilight)}, IniRemoveBuilderFuncs::ganyuTwilight4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::HuTao)}, IniRemoveBuilderFuncs::huTao4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Jean)}, IniRemoveBuilderFuncs::jean4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::JeanCN)}, IniRemoveBuilderFuncs::jeanCN4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::JeanSea)}, IniRemoveBuilderFuncs::jeanSea4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Kaeya)}, IniRemoveBuilderFuncs::kaeya4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::KaeyaSailwind)}, IniRemoveBuilderFuncs::kaeyaSailwind4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Keqing)}, IniRemoveBuilderFuncs::keqing4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::KeqingOpulent)}, IniRemoveBuilderFuncs::keqingOpulent4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Kirara)}, IniRemoveBuilderFuncs::kirara4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::KiraraBoots)}, IniRemoveBuilderFuncs::kiraraBoots4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Klee)}, IniRemoveBuilderFuncs::klee4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::KleeBlossomingStarlight)}, IniRemoveBuilderFuncs::kleeBlossomingStarlight4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Lisa)}, IniRemoveBuilderFuncs::lisa4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::LisaStudent)}, IniRemoveBuilderFuncs::lisaStudent4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Mona)}, IniRemoveBuilderFuncs::mona4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::MonaCN)}, IniRemoveBuilderFuncs::monaCN4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Nilou)}, IniRemoveBuilderFuncs::nilou4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::NilouBreeze)}, IniRemoveBuilderFuncs::nilouBreeze4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Ningguang)}, IniRemoveBuilderFuncs::ningguang4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::NingguangOrchid)}, IniRemoveBuilderFuncs::ningguangOrchid4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Raiden)}, IniRemoveBuilderFuncs::raiden4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Rosaria)}, IniRemoveBuilderFuncs::rosaria4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::RosariaCN)}, IniRemoveBuilderFuncs::rosariaCN4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Shenhe)}, IniRemoveBuilderFuncs::shenhe4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::ShenheFrostFlower)}, IniRemoveBuilderFuncs::shenheFrostFlower4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Xiangling)}, IniRemoveBuilderFuncs::xiangling4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::XianglingCheer)}, IniRemoveBuilderFuncs::xianglingCheer4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::Xingqiu)}, IniRemoveBuilderFuncs::xingqiu4_0()},
                {{"4.0", ModTypeIdTools::getName(ModTypeId::XingqiuBamboo)}, IniRemoveBuilderFuncs::xingqiuBamboo4_0()},
            };
        }
    }

    const std::shared_ptr<const IniRemoveBuilder::ArgsRepo>& IniRemoveBuilderData::repo() {
        // Function-local static: built once, on first use, thread-safely -- and crucially after
        // ModTypeIdTools' own registry is ready, which the row keys depend on.
        static const std::shared_ptr<const IniRemoveBuilder::ArgsRepo> table =
            std::make_shared<const IniRemoveBuilder::ArgsRepo>(
                /*totalIndices*/ 2, /*versionIndexPos*/ 0,
                [](const std::string& raw) { return Version::parse(raw); },
                buildRows());

        return table;
    }
}
