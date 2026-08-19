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

}
