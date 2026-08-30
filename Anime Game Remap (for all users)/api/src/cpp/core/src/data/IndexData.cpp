#include "AGRemapCore/data/IndexData.h"

// See IndexData.h's class-level note: mechanically generated from the real, live
// pure-Python IndexData dict, verified row-for-row identical before being committed --
// not hand-transcribed. Grouped/commented by version, then mod name, then component,
// mirroring the pre-migration Python dict's own visual structure. Future index updates
// edit the literal below directly.

namespace AGRemapCore {
namespace Data {

const std::vector<std::pair<std::vector<std::string>, std::string>>& getIndexDataRows() {
    static const std::vector<std::pair<std::vector<std::string>, std::string>> rows = {
        // ===== version 4.0 =====
        // Amber
        {{"4.0", "Amber", "", "head"}, "0"},
        {{"4.0", "Amber", "", "body"}, "5670"},
        // AmberCN
        {{"4.0", "AmberCN", "", "head"}, "0"},
        {{"4.0", "AmberCN", "", "body"}, "5670"},
        // Ayaka
        {{"4.0", "Ayaka", "", "head"}, "0"},
        {{"4.0", "Ayaka", "", "body"}, "11565"},
        {{"4.0", "Ayaka", "", "dress"}, "58209"},
        // AyakaSpringBloom
        {{"4.0", "AyakaSpringBloom", "", "head"}, "0"},
        {{"4.0", "AyakaSpringBloom", "", "body"}, "56223"},
        {{"4.0", "AyakaSpringBloom", "", "dress"}, "69603"},
        // Barbara
        {{"4.0", "Barbara", "", "head"}, "0"},
        {{"4.0", "Barbara", "", "body"}, "12015"},
        {{"4.0", "Barbara", "", "dress"}, "46248"},
        // BarbaraSummertime
        {{"4.0", "BarbaraSummertime", "", "head"}, "0"},
        {{"4.0", "BarbaraSummertime", "", "body"}, "11943"},
        {{"4.0", "BarbaraSummertime", "", "dress"}, "45333"},
        // Diluc
        {{"4.0", "Diluc", "", "head"}, "0"},
        {{"4.0", "Diluc", "", "body"}, "10896"},
        // DilucFlamme
        {{"4.0", "DilucFlamme", "", "head"}, "0"},
        {{"4.0", "DilucFlamme", "", "body"}, "38061"},
        {{"4.0", "DilucFlamme", "", "dress"}, "56010"},
        // Fischl
        {{"4.0", "Fischl", "", "head"}, "0"},
        {{"4.0", "Fischl", "", "body"}, "11535"},
        {{"4.0", "Fischl", "", "dress"}, "42471"},
        // FischlHighness
        {{"4.0", "FischlHighness", "", "head"}, "0"},
        {{"4.0", "FischlHighness", "", "body"}, "23091"},
        // Ganyu
        {{"4.0", "Ganyu", "", "head"}, "0"},
        {{"4.0", "Ganyu", "", "body"}, "12822"},
        {{"4.0", "Ganyu", "", "dress"}, "47160"},
        // HuTao
        {{"4.0", "HuTao", "", "head"}, "0"},
        {{"4.0", "HuTao", "", "body"}, "16509"},
        // Jean
        {{"4.0", "Jean", "", "head"}, "0"},
        {{"4.0", "Jean", "", "body"}, "7779"},
        // JeanCN
        {{"4.0", "JeanCN", "", "head"}, "0"},
        {{"4.0", "JeanCN", "", "body"}, "7779"},
        // JeanSea
        {{"4.0", "JeanSea", "", "head"}, "0"},
        {{"4.0", "JeanSea", "", "body"}, "7662"},
        {{"4.0", "JeanSea", "", "dress"}, "52542"},
        // Kaeya
        {{"4.0", "Kaeya", "", "head"}, "0"},
        {{"4.0", "Kaeya", "", "body"}, "7596"},
        {{"4.0", "Kaeya", "", "dress"}, "47349"},
        {{"4.0", "Kaeya", "", "extra"}, "47727"},
        // KaeyaSailwind
        {{"4.0", "KaeyaSailwind", "", "head"}, "0"},
        {{"4.0", "KaeyaSailwind", "", "body"}, "23109"},
        {{"4.0", "KaeyaSailwind", "", "dress"}, "76839"},
        // Keqing
        {{"4.0", "Keqing", "", "head"}, "0"},
        {{"4.0", "Keqing", "", "body"}, "10824"},
        {{"4.0", "Keqing", "", "dress"}, "48216"},
        // KeqingOpulent
        {{"4.0", "KeqingOpulent", "", "head"}, "0"},
        {{"4.0", "KeqingOpulent", "", "body"}, "19623"},
        // Kirara
        {{"4.0", "Kirara", "", "head"}, "0"},
        {{"4.0", "Kirara", "", "body"}, "37128"},
        {{"4.0", "Kirara", "", "dress"}, "75234"},
        // Klee
        {{"4.0", "Klee", "", "head"}, "0"},
        {{"4.0", "Klee", "", "body"}, "8436"},
        // KleeBlossomingStarlight
        {{"4.0", "KleeBlossomingStarlight", "", "head"}, "0"},
        {{"4.0", "KleeBlossomingStarlight", "", "body"}, "32553"},
        {{"4.0", "KleeBlossomingStarlight", "", "dress"}, "82101"},
        // Lisa
        {{"4.0", "Lisa", "", "head"}, "0"},
        {{"4.0", "Lisa", "", "body"}, "16815"},
        {{"4.0", "Lisa", "", "dress"}, "45873"},
        // LisaStudent
        {{"4.0", "LisaStudent", "", "head"}, "0"},
        {{"4.0", "LisaStudent", "", "body"}, "29730"},
        // Mona
        {{"4.0", "Mona", "", "head"}, "0"},
        {{"4.0", "Mona", "", "body"}, "17688"},
        // MonaCN
        {{"4.0", "MonaCN", "", "head"}, "0"},
        {{"4.0", "MonaCN", "", "body"}, "17688"},
        // Nilou
        {{"4.0", "Nilou", "", "head"}, "0"},
        {{"4.0", "Nilou", "", "body"}, "44844"},
        {{"4.0", "Nilou", "", "dress"}, "64080"},
        // Ningguang
        {{"4.0", "Ningguang", "", "head"}, "0"},
        {{"4.0", "Ningguang", "", "body"}, "12384"},
        {{"4.0", "Ningguang", "", "dress"}, "47157"},
        // NingguangOrchid
        {{"4.0", "NingguangOrchid", "", "head"}, "0"},
        {{"4.0", "NingguangOrchid", "", "body"}, "43539"},
        {{"4.0", "NingguangOrchid", "", "dress"}, "56124"},
        // Rosaria
        {{"4.0", "Rosaria", "", "head"}, "0"},
        {{"4.0", "Rosaria", "", "body"}, "11139"},
        {{"4.0", "Rosaria", "", "dress"}, "44088"},
        {{"4.0", "Rosaria", "", "extra"}, "45990"},
        // RosariaCN
        {{"4.0", "RosariaCN", "", "head"}, "0"},
        {{"4.0", "RosariaCN", "", "body"}, "11025"},
        {{"4.0", "RosariaCN", "", "dress"}, "46539"},
        {{"4.0", "RosariaCN", "", "extra"}, "48441"},
        // Shenhe
        {{"4.0", "Shenhe", "", "head"}, "0"},
        {{"4.0", "Shenhe", "", "body"}, "14385"},
        {{"4.0", "Shenhe", "", "dress"}, "48753"},
        // Xiangling
        {{"4.0", "Xiangling", "", "head"}, "0"},
        {{"4.0", "Xiangling", "", "body"}, "11964"},
        {{"4.0", "Xiangling", "", "dress"}, "48120"},
        // Xingqiu
        {{"4.0", "Xingqiu", "", "head"}, "0"},
        {{"4.0", "Xingqiu", "", "body"}, "6132"},

        // ===== version 4.4 =====
        // ShenheFrostFlower
        {{"4.4", "ShenheFrostFlower", "", "head"}, "0"},
        {{"4.4", "ShenheFrostFlower", "", "body"}, "31326"},
        {{"4.4", "ShenheFrostFlower", "", "dress"}, "66588"},
        {{"4.4", "ShenheFrostFlower", "", "extra"}, "70068"},
        // GanyuTwilight
        {{"4.4", "GanyuTwilight", "", "head"}, "0"},
        {{"4.4", "GanyuTwilight", "", "body"}, "50817"},
        {{"4.4", "GanyuTwilight", "", "dress"}, "74235"},
        // XingqiuBamboo
        {{"4.4", "XingqiuBamboo", "", "head"}, "0"},
        {{"4.4", "XingqiuBamboo", "", "body"}, "32508"},
        {{"4.4", "XingqiuBamboo", "", "dress"}, "62103"},

        // ===== version 4.6 =====
        // Arlecchino
        {{"4.6", "Arlecchino", "", "head"}, "0"},
        {{"4.6", "Arlecchino", "", "body"}, "40179"},
        {{"4.6", "Arlecchino", "", "dress"}, "74412"},
        // ArlecchinoBoss
        {{"4.6", "ArlecchinoBoss", "", "head"}, "0"},
        {{"4.6", "ArlecchinoBoss", "", "body"}, "40179"},
        {{"4.6", "ArlecchinoBoss", "", "dress"}, "74412"},

        // ===== version 4.8 =====
        // NilouBreeze
        {{"4.8", "NilouBreeze", "", "head"}, "0"},
        {{"4.8", "NilouBreeze", "", "body"}, "44538"},
        {{"4.8", "NilouBreeze", "", "dress"}, "73644"},
        // KiraraBoots
        {{"4.8", "KiraraBoots", "", "head"}, "0"},
        {{"4.8", "KiraraBoots", "", "body"}, "36804"},
        {{"4.8", "KiraraBoots", "", "dress"}, "80295"},

        // ===== version 5.3 =====
        // CherryHuTao
        {{"5.3", "CherryHuTao", "", "head"}, "0"},
        {{"5.3", "CherryHuTao", "", "body"}, "43968"},
        {{"5.3", "CherryHuTao", "", "dress"}, "77301"},
        {{"5.3", "CherryHuTao", "", "extra"}, "86808"},
        // XianglingCheer
        {{"5.3", "XianglingCheer", "", "head"}, "0"},
        {{"5.3", "XianglingCheer", "", "body"}, "46374"},

    };
    return rows;
}

}
}
