#include "AGRemapCore/model/assets/Hashes.h"

#include <utility>

#include "AGRemapCore/data/HashData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModDictAssets.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {
    namespace {
        // Built once and shared by every Hashes instance: the rows are identical for all of them
        // (the data is a fixed table, not per-instance state), and re-flattening ~1000 rows into a
        // fresh ModDictAssets for every ModType would be pure waste.
        //
        // Copied out of, rather than handed out from, this cache -- see the Hashes constructor for
        // why each instance still needs its own table.
        const ModDictAssets<std::string, std::string>& prototypeRepo() {
            static const ModDictAssets<std::string, std::string> repo = []() {
                const std::vector<std::pair<std::vector<std::string>, std::string>>& rawRows = Data::getHashDataRows();

                std::vector<Row<std::string, std::string>> rows;
                rows.reserve(rawRows.size());
                for (const std::pair<std::vector<std::string>, std::string>& rawRow : rawRows) {
                    rows.push_back(Row<std::string, std::string>{rawRow.first, rawRow.second});
                }

                // 3 total indices (version, name, type), version at position 0 -- the same shape
                // the pybind layer's PyHashes builds, and the pure-Python HashData dict's own
                // nesting depth/order.
                return ModDictAssets<std::string, std::string>(
                    3, 0,
                    [](const std::string& raw) { return Version::parse(raw); },
                    std::move(rows));
            }();

            return repo;
        }
    }

    Hashes::Hashes(std::unordered_map<std::string, std::vector<std::string>> map):
        // A copy of the shared prototype rather than a reference to it: ModMappedAssets is mutable
        // (addRepoRows/addMap), and the pure-Python original gives every Hashes its own table, so
        // one mod type adding a hash must not be visible to every other one. Only the *parsing* of
        // the raw rows is shared.
        ModMappedAssets<std::string, std::string>(prototypeRepo(), std::move(map)) {}
}
