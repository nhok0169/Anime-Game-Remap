// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "AGRemapCore/data/IndexCountData.h"

#include "AGRemapCore/constants/ModTypeId.h"

// Generated from WWMI-Assets/PlayerCharacterData/<Name>/Metadata.json by the script that
// registered Sanhua and SanhuaExorcist (2026-09-19). A WWMI character's draw slots are its
// 'components' entries in order; the value is the entry's field named in the header.


namespace AGRemapCore {
namespace Data {

const std::vector<std::pair<std::vector<std::string>, std::string>>& getIndexCountDataRows() {
    static const std::vector<std::pair<std::vector<std::string>, std::string>> rows = {
        // Sanhua (2.5): 7 draw slots, from WWMI-Assets' Metadata.json
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component0"}, "8199"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component1"}, "7158"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component2"}, "9684"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component3"}, "11328"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component4"}, "60678"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component5"}, "45822"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component6"}, "1380"},
        // SanhuaExorcist (2.5): 6 draw slots, from WWMI-Assets' Metadata.json
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component0"}, "9495"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component1"}, "13620"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component2"}, "9684"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component3"}, "98160"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component4"}, "43116"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component5"}, "1380"},

        // Chisa (2.8): 7 draw slots, from the frame dump's Metadata.json
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component0"}, "13566"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component1"}, "36834"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component2"}, "13494"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component3"}, "108702"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component4"}, "105798"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component5"}, "7602"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component6"}, "1362"},
        // ChisaParfait (3.5): 8 draw slots, from the frame dump's Metadata.json
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component0"}, "13602"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component1"}, "64224"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component2"}, "13494"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component3"}, "87435"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component4"}, "71643"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component5"}, "28176"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component6"}, "1362"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component7"}, "540"},
    };
    return rows;
}

}
}
