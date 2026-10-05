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

#include "AGRemapCore/data/VGOffsetData.h"

#include "AGRemapCore/constants/ModTypeId.h"

// Generated from WWMI-Assets/PlayerCharacterData/<Name>/Metadata.json by the script that
// registered Sanhua and SanhuaExorcist (2026-09-19). A WWMI character's draw slots are its
// 'components' entries in order; the value is the entry's field named in the header.


namespace AGRemapCore {
namespace Data {

const std::vector<std::pair<std::vector<std::string>, std::string>>& getVGOffsetDataRows() {
    static const std::vector<std::pair<std::vector<std::string>, std::string>> rows = {
        // Sanhua (2.5): 7 draw slots, from WWMI-Assets' Metadata.json
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component0"}, "0"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component1"}, "19"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component2"}, "23"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component3"}, "24"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component4"}, "51"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component5"}, "123"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component6"}, "208"},
        // SanhuaExorcist (2.5): 6 draw slots, from WWMI-Assets' Metadata.json
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component0"}, "0"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component1"}, "19"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component2"}, "21"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component3"}, "22"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component4"}, "127"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component5"}, "190"},

        // Chisa (2.8): 7 draw slots, from the frame dump's Metadata.json. Her merged skeleton runs
        // to 420 slots, past the 256 an 8-bit blend index can name, so a mod of hers carries WWMI's
        // blend remap (Data/Mod Downloads/WuWa/README.md)
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component0"}, "0"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component1"}, "27"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component2"}, "141"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component3"}, "142"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component4"}, "270"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component5"}, "391"},
        {{"2.8", ModTypeIdTools::getName(ModTypeId::Chisa), "", "component6"}, "419"},
        // ChisaParfait (3.5): 8 draw slots, from the frame dump's Metadata.json
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component0"}, "0"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component1"}, "27"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component2"}, "71"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component3"}, "72"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component4"}, "162"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component5"}, "212"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component6"}, "258"},
        {{"3.5", ModTypeIdTools::getName(ModTypeId::ChisaParfait), "", "component7"}, "259"},
    };
    return rows;
}

}
}
