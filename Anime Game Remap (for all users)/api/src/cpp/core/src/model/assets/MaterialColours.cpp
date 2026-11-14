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

#include "AGRemapCore/model/assets/MaterialColours.h"

#include <stdexcept>
#include <string>
#include <utility>

#include "AGRemapCore/data/MaterialColourData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModDictAssets.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {
    namespace {
        // Built once and shared by every instance -- see Hashes.cpp's identical note for why this is a prototype that
        // gets copied rather than a table that gets handed out.
        const ModDictAssets<std::string, std::string>& prototypeRepo() {
            static const ModDictAssets<std::string, std::string> repo = []() {
                const std::vector<std::pair<std::vector<std::string>, std::string>>& rawRows = Data::getMaterialColourDataRows();
                std::vector<Row<std::string, std::string>> rows;
                rows.reserve(rawRows.size());
                for (const std::pair<std::vector<std::string>, std::string>& rawRow : rawRows) {
                    rows.push_back(Row<std::string, std::string>{rawRow.first, rawRow.second});
                }

                // 4 total indices (version, name, component, type), version at position 0 -- the same shape as Indices
                return ModDictAssets<std::string, std::string>(
                    4, 0,
                    [](const std::string& raw) { return Version::parse(raw); },
                    std::move(rows));
            }();
            return repo;
        }
    }


    MaterialColours::MaterialColours(std::unordered_map<std::string, std::vector<std::string>> map):
        ModMappedAssets<std::string, std::string>(prototypeRepo(), std::move(map)) {
        nonVersionIndexNames = std::vector<std::string>{"name", "component", "type"};
    }


    std::string MaterialColours::valueOf(const Colour& colour) {
        const auto [red, green, blue, alpha] = colour.getTuple();
        return std::to_string(red) + "," + std::to_string(green) + "," + std::to_string(blue) + "," + std::to_string(alpha);
    }


    Colour MaterialColours::colourOf(const std::string& value) {
        std::vector<int> channels;
        std::size_t start = 0;
        while (start <= value.size()) {
            const std::size_t comma = value.find(',', start);
            const std::string part = value.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
            std::size_t used = 0;
            const int channel = std::stoi(part, &used);
            if (used != part.size()) {
                throw std::invalid_argument("not a colour channel: '" + part + "' in '" + value + "'");
            }
            channels.push_back(channel);
            if (comma == std::string::npos) {
                break;
            }
            start = comma + 1;
        }
        if (channels.size() != 4) {
            throw std::invalid_argument("a colour is four channels, r,g,b,a: '" + value + "'");
        }
        return Colour(channels[0], channels[1], channels[2], channels[3]);
    }
}
