#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegRemove<K, V, KeyHash, KeyEqual>::RegRemove(std::vector<std::pair<K, std::optional<RemoveKeyCheck>>> removeKeys):
        removeKeys(std::move(removeKeys)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegRemove<K, V, KeyHash, KeyEqual>::ContentPart& RegRemove<K, V, KeyHash, KeyEqual>::edit(
            ContentPart& part, const std::string& sectionName, const ModType* modType, const std::string& modName,
            const OrderRanges* partRanges) {
        (void)sectionName;
        (void)modType;
        (void)modName;

        part.removeKeys(removeKeys, Base::toRangeSpec(partRanges));
        return part;
    }
}
