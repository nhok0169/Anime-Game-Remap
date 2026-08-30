#include "AGRemapCore/model/assets/Indices.h"

#include <utility>

#include "AGRemapCore/data/IndexData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModDictAssets.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {
    namespace {
        // Built once and shared by every Indices instance -- see Hashes.cpp's identical note for
        // why this is a prototype that gets copied rather than a table that gets handed out.
        const ModDictAssets<std::string, std::string>& prototypeRepo() {
            static const ModDictAssets<std::string, std::string> repo = []() {
                const std::vector<std::pair<std::vector<std::string>, std::string>>& rawRows = Data::getIndexDataRows();

                std::vector<Row<std::string, std::string>> rows;
                rows.reserve(rawRows.size());
                for (const std::pair<std::vector<std::string>, std::string>& rawRow : rawRows) {
                    rows.push_back(Row<std::string, std::string>{rawRow.first, rawRow.second});
                }

                // 4 total indices (version, name, component, type), version at position 0 -- the
                // same shape the pybind layer's PyIndices builds, and the pure-Python IndexData
                // dict's own nesting depth/order. Note this is one deeper than Hashes.
                return ModDictAssets<std::string, std::string>(
                    4, 0,
                    [](const std::string& raw) { return Version::parse(raw); },
                    std::move(rows));
            }();

            return repo;
        }
    }

    Indices::Indices(std::unordered_map<std::string, std::vector<std::string>> map):
        ModMappedAssets<std::string, std::string>(prototypeRepo(), std::move(map)) {}
}
