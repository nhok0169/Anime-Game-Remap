#include "AGRemapCore/model/assets/VertexCounts.h"

#include <utility>
#include <vector>

#include "AGRemapCore/data/VertexCountData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {
    namespace {
        // Converted once and reused -- see Hashes.cpp's identical note. Each VertexCounts still gets
        // its own copy, since ModDictAssets is mutable (addRows).
        const ModDictAssets<std::string, int>& prototypeRepo() {
            static const ModDictAssets<std::string, int> repo = []() {
                const std::vector<std::pair<std::vector<std::string>, int>>& rawRows = Data::getVertexCountDataRows();

                std::vector<Row<std::string, int>> rows;
                rows.reserve(rawRows.size());
                for (const std::pair<std::vector<std::string>, int>& rawRow : rawRows) {
                    rows.push_back(Row<std::string, int>{rawRow.first, rawRow.second});
                }

                // 3 total indices (version, name, component), version at position 0. The
                // component column is currently "" on every row -- see getVertexCountDataRows.
                return ModDictAssets<std::string, int>(
                    3, 0,
                    [](const std::string& raw) { return Version::parse(raw); },
                    std::move(rows));
            }();

            return repo;
        }
    }

    VertexCounts::VertexCounts(): ModDictAssets<std::string, int>(prototypeRepo()) {}
}
