#include "AGRemapCore/data/HashToModObjData.h"

#include "AGRemapCore/data/HashData.h"
#include "AGRemapCore/data/IndexData.h"


namespace AGRemapCore {
namespace Data {

namespace {

// The 'ib' half of the "hash alone / hash plus match_first_index" split. Tested against the object
// name rather than the whole key, so a component-qualified key ("someComp;ib") lands on the same
// side as its unqualified form.
bool isIndexObjName(const std::string& objName) {
    static const std::string suffix = "ib";
    return objName.size() >= suffix.size() && objName.compare(objName.size() - suffix.size(), suffix.size(), suffix) == 0;
}


// The last two index columns of a data row -- (component, object) for an index row.
ModObjKey rowTail(const std::vector<std::string>& indexVals) {
    if (indexVals.size() < 2) {
        return ModObjKey();
    }

    return ModObjKey(indexVals[indexVals.size() - 2], indexVals.back());
}

}


ModObjKey parseModObjKey(const std::string& key) {
    std::size_t sepInd = key.find(';');

    if (sepInd == std::string::npos) {
        return ModObjKey("", key);
    }

    return ModObjKey(key.substr(0, sepInd), key.substr(sepInd + 1));
}


const std::unordered_map<std::string, ModObjKey>& getHashKeyOnlyToModObj() {
    static const std::unordered_map<std::string, ModObjKey> result = []() {
        std::unordered_map<std::string, ModObjKey> built;

        for (const std::pair<std::vector<std::string>, std::string>& row : getHashDataRows()) {
            if (row.first.empty()) {
                continue;
            }

            const std::string& hashKey = row.first.back();
            ModObjKey modObj = parseModObjKey(hashKey);

            if (isIndexObjName(modObj.second)) {
                continue;
            }

            built.emplace(hashKey, std::move(modObj));
        }

        return built;
    }();

    return result;
}


const std::unordered_map<std::string, IndexKeyToModObj>& getIndexKeyToModObj() {
    static const std::unordered_map<std::string, IndexKeyToModObj> result = []() {
        // Every mod object an index row can name, shared by every 'ib'-suffixed hash type: the row
        // already ends in a (component, object) pair, so it *is* the mod object it maps to.
        IndexKeyToModObj indexModObjs;

        for (const std::pair<std::vector<std::string>, std::string>& row : getIndexDataRows()) {
            ModObjKey modObj = rowTail(row.first);

            if (modObj.second.empty()) {
                continue;
            }

            indexModObjs.emplace(modObj, modObj);
        }

        std::unordered_map<std::string, IndexKeyToModObj> built;

        for (const std::pair<std::vector<std::string>, std::string>& row : getHashDataRows()) {
            if (row.first.empty()) {
                continue;
            }

            const std::string& hashKey = row.first.back();

            if (!isIndexObjName(parseModObjKey(hashKey).second)) {
                continue;
            }

            built.emplace(hashKey, indexModObjs);
        }

        return built;
    }();

    return result;
}

}
}
