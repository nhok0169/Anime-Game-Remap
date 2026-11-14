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

#include "AGRemapCore/model/assets/ShapeKeyRemaps.h"

#include <utility>
#include <vector>

#include "AGRemapCore/data/ShapeKeyRemapData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {
    namespace {
        // Converted once and copied per instance -- see Hashes.cpp's identical note
        const ModAssets<std::string, std::string>& prototypeRepo() {
            static const ModAssets<std::string, std::string> repo = []() {
                std::vector<Row<std::string, std::string>> rows;
                for (const std::pair<std::vector<std::string>, std::string>& rawRow : Data::getShapeKeyRemapDataRows()) {
                    rows.push_back(Row<std::string, std::string>{rawRow.first, rawRow.second});
                }

                // 6 index columns: fromVersion, fromChar, fromComp, toVersion, toChar, toComp -- as VGRemaps
                return ModAssets<std::string, std::string>(
                    {true, false, false, true, false, false},
                    [](const std::string& raw) { return Version::parse(raw); },
                    std::move(rows));
            }();
            return repo;
        }
    }

    ShapeKeyRemaps::ShapeKeyRemaps(): ModAssets<std::string, std::string>(prototypeRepo()) {}
}
