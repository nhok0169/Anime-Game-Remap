#include "AGRemapCore/model/iftemplate/IfContentPartColour.h"

#include <algorithm>


namespace AGRemapCore {

    // ==================== IfContentPartColourChange ====================

    template <typename V>
    IfContentPartColourChange<V>::IfContentPartColourChange(std::optional<StateValue> old) : old(std::move(old)) {}

    template <typename V>
    std::unique_ptr<IfContentPartColourChange<V>> IfContentPartColourChange<V>::clone() const {
        return std::make_unique<IfContentPartColourChange<V>>(*this);
    }

    template <typename V>
    template <typename K, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    void IfContentPartColourChange<V>::restore(IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>& colouring, const K& key) const {
        if (!colouring.contains(key)) {
            return;
        }

        if (!old.has_value()) {
            colouring.erase(key);
            return;
        }

        colouring.set(key, *old);
    }

    // ==================== IfContentPartColouring ====================

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::unique_ptr<IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>>
    IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::clone() const {
        return std::make_unique<IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>>(*this);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    bool IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::contains(const K& key) const {
        return data_.find(key) != data_.end();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    size_t IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::size() const {
        return data_.size();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    bool IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::empty() const {
        return data_.empty();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::optional<typename IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::StateValue>
    IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::get(const K& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    const typename IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::StateValue&
    IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::at(const K& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::out_of_range("IfContentPartColouring::at: key not found");
        }
        return it->second;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    void IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::set(const K& key, StateValue value) {
        data_[key] = std::move(value);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    bool IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::erase(const K& key) {
        return data_.erase(key) > 0;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    void IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::clear() {
        data_.clear();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::vector<K> IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::keys() const {
        std::vector<K> result;
        result.reserve(data_.size());
        for (const auto& entry : data_) {
            result.push_back(entry.first);
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::vector<std::pair<K, typename IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::StateValue>>
    IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::items() const {
        std::vector<std::pair<K, StateValue>> result;
        result.reserve(data_.size());
        for (const auto& entry : data_) {
            result.emplace_back(entry.first, entry.second);
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::unordered_map<K, typename IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::Change, KeyHash, KeyEqual>
    IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::updateColouring(
        const ContentPart& ifContentPart,
        const std::optional<std::unordered_set<K, KeyHash, KeyEqual>>& targetKeys,
        bool updatePreviousKVPs) {

        std::unordered_map<K, Change, KeyHash, KeyEqual> change;
        std::unordered_set<K, KeyHash, KeyEqual> keysChanged;

        // Update the values from previous parts. Snapshots the keys first (rather than mutating
        // 'data_' while iterating it directly) purely to keep this loop obviously safe -- only
        // existing keys' values are ever reassigned here, never inserted/erased, so iterating
        // 'data_' directly would also be safe, but the snapshot makes that invariant unconditional
        // rather than dependent on tsl::ordered_map's own iterator-stability guarantees.
        if (updatePreviousKVPs) {
            std::vector<K> currentKeys = keys();
            for (const K& key : currentKeys) {
                auto it = data_.find(key);
                if (it == data_.end() || !std::holds_alternative<std::vector<IndexedValue>>(it->second)) {
                    continue;
                }

                const auto& indexedVals = std::get<std::vector<IndexedValue>>(it->second);
                change.emplace(key, Change(it->second));

                V lastVal = indexedVals.back().second;
                data_[key] = StateValue(std::move(lastVal));
                keysChanged.insert(key);
            }
        }

        size_t ifContentSrcLen = ifContentPart.keySize();
        size_t targetKeysLen = targetKeys.has_value() ? targetKeys->size() : ifContentSrcLen;

        std::unordered_set<K, KeyHash, KeyEqual> allKeys;
        const std::unordered_set<K, KeyHash, KeyEqual>* shortIterKeys;
        if (targetKeys.has_value() && targetKeysLen < ifContentSrcLen) {
            shortIterKeys = &(*targetKeys);
        } else {
            allKeys = ifContentPart.getKeys();
            shortIterKeys = &allKeys;
        }

        // Update the values for the current part.
        for (const K& key : *shortIterKeys) {
            if (!ifContentPart.contains(key)) {
                continue;
            }

            if (targetKeys.has_value() && targetKeys->find(key) == targetKeys->end()) {
                continue;
            }

            std::vector<IndexedValue> newVal = ifContentPart.getValsWithInds(key);
            if (newVal.empty()) {
                continue;
            }

            if (keysChanged.find(key) == keysChanged.end()) {
                change.emplace(key, Change(get(key)));
            }

            data_[key] = StateValue(std::move(newVal));
        }

        return change;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    void IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::restore(
        const std::unordered_map<K, Change, KeyHash, KeyEqual>& colourChange) {
        for (const auto& entry : colourChange) {
            entry.second.restore(*this, entry.first);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::vector<std::pair<std::optional<long long>, V>>
    IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::rawIndVals(const K& key) const {
        std::vector<std::pair<std::optional<long long>, V>> result;

        auto it = data_.find(key);
        if (it == data_.end()) {
            return result;
        }

        const StateValue& val = it->second;
        if (std::holds_alternative<V>(val)) {
            result.emplace_back(std::nullopt, std::get<V>(val));
            return result;
        }

        const auto& indexedVals = std::get<std::vector<IndexedValue>>(val);
        result.reserve(indexedVals.size());
        for (const auto& indexedVal : indexedVals) {
            result.emplace_back(std::optional<long long>(indexedVal.first), indexedVal.second);
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::vector<std::pair<std::optional<long long>, V>>
    IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::getIndVals(const K& key, const std::optional<Filter>& filter) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return {};
        }

        const StateValue& val = it->second;

        // A carried-over flat value is always returned as-is, unfiltered -- see this method's
        // doc comment for why this deliberately doesn't match getVals()'s own treatment of the
        // same case.
        if (std::holds_alternative<V>(val)) {
            std::vector<std::pair<std::optional<long long>, V>> result;
            result.emplace_back(std::nullopt, std::get<V>(val));
            return result;
        }

        const auto& indexedVals = std::get<std::vector<IndexedValue>>(val);
        std::vector<std::pair<std::optional<long long>, V>> result;
        result.reserve(indexedVals.size());
        for (const auto& indexedVal : indexedVals) {
            std::optional<long long> ind(indexedVal.first);
            if (!filter.has_value() || (*filter)(ind, indexedVal.second)) {
                result.emplace_back(ind, indexedVal.second);
            }
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::vector<V> IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::getVals(const K& key, const std::optional<Filter>& filter) const {
        std::vector<V> result;
        for (auto& indVal : rawIndVals(key)) {
            if (!filter.has_value() || (*filter)(indVal.first, indVal.second)) {
                result.push_back(std::move(indVal.second));
            }
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    std::unordered_set<V, ValueHash, ValueEqual>
    IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::getUniqueVals(const K& key, const std::optional<Filter>& filter) const {
        std::unordered_set<V, ValueHash, ValueEqual> result;
        for (auto& indVal : rawIndVals(key)) {
            if (!filter.has_value() || (*filter)(indVal.first, indVal.second)) {
                result.insert(std::move(indVal.second));
            }
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    Ranges<long long> IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>::getRanges(
        const std::optional<std::unordered_map<K, bool, KeyHash, KeyEqual>>& keysExists,
        const std::optional<std::unordered_map<K, Filter, KeyHash, KeyEqual>>& keyFilters,
        bool existsRequireAll, bool filtersRequireAll, bool globalRequireAll, bool includeKeyDefs) const {

        using RangesLL = Ranges<long long>;

        RangesLL result = RangesLL::createFull();
        std::unordered_map<K, RangesLL, KeyHash, KeyEqual> filterRanges;
        std::unordered_map<K, RangesLL, KeyHash, KeyEqual> existRanges;
        std::unordered_set<long long> keyDefInds;

        if (keyFilters.has_value()) {
            for (const auto& entry : *keyFilters) {
                const K& reg = entry.first;
                const Filter& keyFilter = entry.second;

                auto it = data_.find(reg);
                if (it == data_.end()) {
                    continue;
                }
                const StateValue& regVals = it->second;

                if (std::holds_alternative<V>(regVals)) {
                    bool matches = keyFilter(std::nullopt, std::get<V>(regVals));
                    filterRanges.emplace(reg, matches ? RangesLL::createFull() : RangesLL::createEmpty());
                    continue;
                }

                const auto& indexedVals = std::get<std::vector<IndexedValue>>(regVals);
                size_t n = indexedVals.size();

                std::vector<typename RangesLL::Range> currentRanges;
                for (size_t i = 0; i < n; ++i) {
                    long long ind = indexedVals[i].first;
                    const V& val = indexedVals[i].second;

                    if (!includeKeyDefs) {
                        keyDefInds.insert(ind);
                    }

                    if (!keyFilter(std::optional<long long>(ind), val)) {
                        continue;
                    }

                    std::optional<long long> endInd = (i < n - 1) ? std::optional<long long>(indexedVals[i + 1].first) : std::nullopt;
                    currentRanges.push_back({std::optional<long long>(ind), endInd});
                }

                filterRanges.emplace(reg, RangesLL(currentRanges));
            }
        }

        if (keysExists.has_value()) {
            for (const auto& entry : *keysExists) {
                const K& reg = entry.first;
                bool keyExists = entry.second;

                auto it = data_.find(reg);
                if (it == data_.end()) {
                    existRanges.emplace(reg, keyExists ? RangesLL::createEmpty() : RangesLL::createFull());
                    continue;
                }
                const StateValue& regVals = it->second;

                if (std::holds_alternative<V>(regVals)) {
                    existRanges.emplace(reg, keyExists ? RangesLL::createFull() : RangesLL::createEmpty());
                    continue;
                }

                const auto& indexedVals = std::get<std::vector<IndexedValue>>(regVals);

                std::vector<typename RangesLL::Range> currentExistRanges;
                if (!indexedVals.empty()) {
                    currentExistRanges.push_back({std::optional<long long>(indexedVals[0].first), std::nullopt});
                }

                RangesLL existR(currentExistRanges);
                existRanges.emplace(reg, keyExists ? existR : existR.negate());

                if (!includeKeyDefs) {
                    for (const auto& indexedVal : indexedVals) {
                        keyDefInds.insert(indexedVal.first);
                    }
                }
            }
        }

        std::vector<RangesLL> filterList;
        filterList.reserve(filterRanges.size());
        for (const auto& entry : filterRanges) {
            filterList.push_back(entry.second);
        }

        std::vector<RangesLL> existList;
        existList.reserve(existRanges.size());
        for (const auto& entry : existRanges) {
            existList.push_back(entry.second);
        }

        RangesLL existResult = result.getOverlaps(existList, existsRequireAll);
        RangesLL filterResult = result.getOverlaps(filterList, filtersRequireAll);

        RangesLL finalResult = globalRequireAll ? existResult.intersect({filterResult}) : existResult.unionWith({filterResult});

        if (includeKeyDefs) {
            return finalResult;
        }

        RangesLL keyDefRanges = RangesLL::createFromSet(std::set<long long>(keyDefInds.begin(), keyDefInds.end()));
        return finalResult - keyDefRanges;
    }

}
