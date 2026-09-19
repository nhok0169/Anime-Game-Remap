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

#include "AGRemapCore/data/VGCountData.h"

#include "AGRemapCore/constants/ModTypeId.h"

// Generated from WWMI-Assets/PlayerCharacterData/<Name>/Metadata.json by the script that
// registered Sanhua and SanhuaExorcist (2026-09-19). A WWMI character's draw slots are its
// 'components' entries in order; the value is the entry's field named in the header.


namespace AGRemapCore {
namespace Data {

const std::vector<std::pair<std::vector<std::string>, std::string>>& getVGCountDataRows() {
    static const std::vector<std::pair<std::vector<std::string>, std::string>> rows = {
        // Sanhua (2.5): 7 draw slots, from WWMI-Assets' Metadata.json
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component0"}, "19"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component1"}, "4"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component2"}, "1"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component3"}, "27"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component4"}, "72"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component5"}, "85"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "component6"}, "1"},
        // SanhuaExorcist (2.5): 6 draw slots, from WWMI-Assets' Metadata.json
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component0"}, "19"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component1"}, "2"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component2"}, "1"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component3"}, "105"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component4"}, "63"},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "component5"}, "1"},
    };
    return rows;
}

}
}
