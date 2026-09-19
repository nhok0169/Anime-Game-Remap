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

#include "AGRemapCore/data/ShapeKeyChecksumData.h"

#include "AGRemapCore/constants/ModTypeId.h"

// Generated from WWMI-Assets/PlayerCharacterData/<Name>/Metadata.json by the script that
// registered Sanhua and SanhuaExorcist (2026-09-19). A WWMI character's draw slots are its
// 'components' entries in order; the value is the entry's field named in the header.


namespace AGRemapCore {
namespace Data {

const std::vector<std::pair<std::vector<std::string>, std::string>>& getShapeKeyChecksumDataRows() {
    static const std::vector<std::pair<std::vector<std::string>, std::string>> rows = {
        // Sanhua (2.5): the sum of the first four shape-key offsets, Metadata.json's 'checksum'
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), "", "shapekeys"}, "3175"},
        // SanhuaExorcist (2.5): the sum of the first four shape-key offsets, Metadata.json's 'checksum'
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), "", "shapekeys"}, "2376"},
    };
    return rows;
}

}
}
