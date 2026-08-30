#include "AGRemapCore/model/assets/VGRemaps.h"

#include <utility>
#include <vector>

#include "AGRemapCore/data/VGRemapData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {
    namespace {
        // Converted once and reused -- see Hashes.cpp's identical note. Each VGRemaps still gets its
        // own copy, since ModAssets is mutable. Copying matters more here than for the other tables:
        // this one carries 5229 remap pairs, which is exactly why ModType::vgRemaps falls back to a
        // single shared instance (ModDataAssets::vgRemaps) rather than building one per mod type.
        const ModAssets<std::string, VGRemap>& prototypeRepo() {
            static const ModAssets<std::string, VGRemap> repo = []() {
                const std::vector<std::pair<std::vector<std::string>, VGRemap>>& rawRows = Data::getVGRemapDataRows();

                std::vector<Row<std::string, VGRemap>> rows;
                rows.reserve(rawRows.size());
                for (const std::pair<std::vector<std::string>, VGRemap>& rawRow : rawRows) {
                    rows.push_back(Row<std::string, VGRemap>{rawRow.first, rawRow.second});
                }

                // 6 index columns: fromVersion, fromChar, fromComp, toVersion, toChar, toComp --
                // with the version flags at positions 0 and 3. That two-version-column shape is why
                // this is a ModAssets rather than a ModDictAssets; see VGRemaps' own note.
                return ModAssets<std::string, VGRemap>(
                    {true, false, false, true, false, false},
                    [](const std::string& raw) { return Version::parse(raw); },
                    std::move(rows));
            }();

            return repo;
        }
    }

    VGRemaps::VGRemaps(): ModAssets<std::string, VGRemap>(prototypeRepo()) {}
}
