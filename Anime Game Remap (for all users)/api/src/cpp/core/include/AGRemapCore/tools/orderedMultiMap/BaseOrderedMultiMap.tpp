#include "AGRemapCore/tools/orderedMultiMap/BaseOrderedMultiMap.h"


namespace AGRemapCore {

    // ------------------------------------------------------------------
    // RemappedKeyData<K, V>
    // ------------------------------------------------------------------

    template <typename K, typename V>
    RemappedKeyData<K, V>::RemappedKeyData(K key,
                                            std::optional<CheckPredicate> check,
                                            std::optional<long long> toInd)
        : key_(std::move(key)), check_(std::move(check)), toInd_(toInd) {}

    template <typename K, typename V>
    const K& RemappedKeyData<K, V>::key() const { return key_; }

    template <typename K, typename V>
    const std::optional<typename RemappedKeyData<K, V>::CheckPredicate>& RemappedKeyData<K, V>::check() const {
        return check_;
    }

    template <typename K, typename V>
    const std::optional<long long>& RemappedKeyData<K, V>::toInd() const { return toInd_; }

    // ------------------------------------------------------------------
    // KeyRemapData<K, V>
    // ------------------------------------------------------------------

    template <typename K, typename V>
    KeyRemapData<K, V>::KeyRemapData(RemapList<K, V> remappedKeys, bool keepKeyWithoutRemap)
        : remappedKeys_(std::move(remappedKeys)), keepKeyWithoutRemap_(keepKeyWithoutRemap) {}

    template <typename K, typename V>
    const RemapList<K, V>& KeyRemapData<K, V>::remappedKeys() const { return remappedKeys_; }

    template <typename K, typename V>
    bool KeyRemapData<K, V>::keepKeyWithoutRemap() const { return keepKeyWithoutRemap_; }

    // ------------------------------------------------------------------
    // BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>
    // ------------------------------------------------------------------

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    Derived& BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::self() {
        return static_cast<Derived&>(*this);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    const Derived& BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::self() const {
        return static_cast<const Derived&>(*this);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::insert(const K& key, const V& value) {
        insertBefore(self().rawEnd(), key, value);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::insertStart(const K& key, const V& value) {
        insertBefore(self().rawBegin(), key, value);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::insertAt(long long index, const K& key, const V& value) {
        const size_t pos = normalizeIndex(index, self().rawSize());
        insertBefore(handleAtInsertionSlot(pos), key, value);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::insertAllEnd(const std::vector<std::pair<K, V>>& items) {
        for (const auto& kv : items) {
            insertBefore(self().rawEnd(), kv.first, kv.second);
        }
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::insertAllStart(const std::vector<std::pair<K, V>>& items) {
        // Naively calling insertStart() once per item in a loop would REVERSE
        // the given order, since each new front-insert lands before the
        // previous one as rawBegin() keeps moving. This avoids that with a
        // fixed anchor. The anchor is re-derived from the just-inserted
        // entry's own handle after each insertion (via rawNext) rather than
        // reused as-is: a backing structure that rebalances on insert (e.g.
        // splitting a block) can change what an OLD handle means without
        // invalidating it outright, so a stale cached handle is not safe to
        // keep reusing across multiple mutating calls even when it's still
        // technically dereferenceable.
        Handle anchor = self().rawBegin();
        for (const auto& kv : items) {
            Handle newH = insertBefore(anchor, kv.first, kv.second);
            anchor = self().rawNext(newH);
        }
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    size_t BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::insertAllAt(
        const tsl::ordered_map<long long, std::pair<K, V>>& items,
        bool sortIndices,
        const std::optional<Ranges<long long>>& ranges) {
        return insertAllAtImpl(std::vector<std::pair<long long, std::pair<K, V>>>(
            items.begin(), items.end()), sortIndices, ranges);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    size_t BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::insertAllAt(
        const std::unordered_map<long long, std::pair<K, V>>& items,
        const std::optional<Ranges<long long>>& ranges) {
        return insertAllAtImpl(std::vector<std::pair<long long, std::pair<K, V>>>(
            items.begin(), items.end()), /*sortIndices=*/true, ranges);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::reorder(
        const tsl::ordered_map<long long, long long>& orderMap,
        const std::optional<Ranges<long long>>& ranges) {
        reorderImpl(std::vector<std::pair<long long, long long>>(
            orderMap.begin(), orderMap.end()), ranges);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::reorder(
        const std::unordered_map<long long, long long>& orderMap,
        const std::optional<Ranges<long long>>& ranges) {
        reorderImpl(std::vector<std::pair<long long, long long>>(
            orderMap.begin(), orderMap.end()), ranges);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    bool BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::removeAt(
        size_t pos, const std::optional<Ranges<long long>>& ranges) {
        if (pos >= self().rawSize()) return false;
        if (ranges.has_value() && !ranges->has(static_cast<long long>(pos))) return false;
        eraseHandle(self().rawAtIndex(pos));
        return true;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    size_t BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::removeKey(
        const K& key,
        const std::optional<Ranges<long long>>& ranges,
        const std::optional<RemoveKeyCheck>& check) {
        auto found = index_.find(key);
        if (found == index_.end()) return 0;

        // Fast path: unconditional removal, straight from the bucket, no
        // per-entry positional bookkeeping needed.
        if (!ranges.has_value() && !check.has_value()) {
            size_t count = 0;
            for (Handle h : found->second.items) {
                self().rawErase(h);
                ++count;
            }
            index_.erase(found);
            // Bypasses eraseHandle() (the usual place this gets set) since
            // there's no per-entry bucket bookkeeping to do here either --
            // still has to be marked, since every remaining handle's true
            // positional index shifted.
            indexMapDirty_ = true;
            return count;
        }

        const auto& indexOf = buildIndexMap();

        std::vector<Handle> toRemove;
        for (Handle h : found->second.items) {
            const long long pos = indexOf.at(h);
            const bool inRanges = !ranges.has_value() || ranges->has(pos);
            const bool passesCheck = !check.has_value() || (*check)(pos, self().rawValue(h));
            if (inRanges && passesCheck) {
                toRemove.push_back(h);
            }
        }

        for (Handle h : toRemove) {
            eraseHandle(h);
        }
        return toRemove.size();
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::remapKeys(
        const std::unordered_map<K, KeyRemapValue, KeyHash, KeyEqual>& keyRemap,
        const std::optional<Ranges<long long>>& ranges) {
        std::vector<std::pair<K, V>> rebuilt;
        std::vector<std::pair<long long, long long>> moves; // (intermediate index -> toInd)
        rebuilt.reserve(self().rawSize());

        long long idx = 0;
        for (Handle h = self().rawBegin(); h != self().rawEnd(); h = self().rawNext(h)) {
            const K key = self().rawKey(h);     // copies: rawClear() below invalidates h partway
            const V value = self().rawValue(h); // through this loop
            const bool inRanges = !ranges.has_value() || ranges->has(idx);
            ++idx;

            auto found = inRanges ? keyRemap.find(key) : keyRemap.end();
            if (found == keyRemap.end()) {
                rebuilt.emplace_back(key, value);
                continue;
            }

            const KeyRemapList* list;
            bool keepIfNoneFire;
            if (std::holds_alternative<KeyRemapList>(found->second)) {
                list = &std::get<KeyRemapList>(found->second);
                keepIfNoneFire = false;
            } else {
                const auto& krd = std::get<KeyRemapData<K, V>>(found->second);
                list = &krd.remappedKeys();
                keepIfNoneFire = krd.keepKeyWithoutRemap();
            }

            const size_t before = rebuilt.size();
            for (const RemapListItem& item : *list) {
                if (std::holds_alternative<K>(item)) {
                    rebuilt.emplace_back(std::get<K>(item), value);
                    continue;
                }
                const auto& rkd = std::get<RemappedKeyData<K, V>>(item);
                if (rkd.check().has_value() && !(*rkd.check())(key, value)) {
                    continue; // rule didn't fire for this occurrence
                }
                const long long newIndex = static_cast<long long>(rebuilt.size());
                rebuilt.emplace_back(rkd.key(), value);
                if (rkd.toInd().has_value()) {
                    moves.emplace_back(newIndex, *rkd.toInd());
                }
            }

            if (rebuilt.size() == before && keepIfNoneFire) {
                rebuilt.emplace_back(key, value); // nothing fired -- retain the original
            }
        }

        // Rebuild from scratch via the normal insert() path -- pos labels,
        // per-key buckets, and sorted caches all end up freshly and
        // correctly built in the new (intermediate) order.
        self().rawClear();
        index_.clear();
        indexMapDirty_ = true;
        for (auto& p : rebuilt) {
            insert(p.first, p.second);
        }

        // Reposition every toInd-tagged entry using reorder()'s own
        // machinery directly: `moves`' old-indices are exactly this
        // intermediate structure's own positions (always valid, never
        // colliding, so reorder()'s rule 1 never triggers here), and its
        // arrival order already matches "the order entries were
        // encountered" since `moves` was built by walking the original
        // order and each rule list in their own fixed order.
        if (!moves.empty()) {
            reorderImpl(moves);
        }
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::replaceVals(
        const tsl::ordered_map<K, ReplaceSpec, KeyHash, KeyEqual>& newVals, bool addNew,
        const std::optional<Ranges<long long>>& ranges) {
        replaceValsImpl(std::vector<std::pair<K, ReplaceSpec>>(
            newVals.begin(), newVals.end()), addNew, ranges);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::replaceVals(
        const std::unordered_map<K, ReplaceSpec, KeyHash, KeyEqual>& newVals, bool addNew,
        const std::optional<Ranges<long long>>& ranges) {
        replaceValsImpl(std::vector<std::pair<K, ReplaceSpec>>(
            newVals.begin(), newVals.end()), addNew, ranges);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    bool BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::contains(const K& key) const {
        return index_.find(key) != index_.end();
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    bool BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::containsKey(const K& key) const {
        return contains(key);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    size_t BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::count(const K& key) const {
        auto found = index_.find(key);
        return found == index_.end() ? 0 : found->second.items.size();
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    size_t BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::size() const {
        return self().rawSize();
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    size_t BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::length() const {
        return self().rawSize();
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    size_t BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::keySize() const {
        return index_.size();
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    bool BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::empty() const {
        return self().rawSize() == 0;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    std::vector<V> BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::getAll(
        const K& key, bool ordered, const std::optional<Ranges<long long>>& ranges) const {
        std::vector<V> result;
        auto found = index_.find(key);
        if (found == index_.end()) return result;
        const KeyBucket& bucket = found->second;
        result.reserve(bucket.items.size());

        // No ranges -- skip buildIndexMap() entirely, no true positional index needed here.
        if (!ranges.has_value()) {
            if (!ordered) {
                for (Handle h : bucket.items) result.push_back(self().rawValue(h));
                return result;
            }
            for (Handle h : ensureSortedCache(bucket)) result.push_back(self().rawValue(h));
            return result;
        }

        const auto& indexOf = buildIndexMap();
        if (!ordered) {
            for (Handle h : bucket.items) {
                if (ranges->has(indexOf.at(h))) result.push_back(self().rawValue(h));
            }
            return result;
        }
        for (Handle h : ensureSortedCache(bucket)) {
            if (ranges->has(indexOf.at(h))) result.push_back(self().rawValue(h));
        }
        return result;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    std::vector<std::pair<long long, V>> BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::getAll(
        const K& key, bool ordered, bool withInds, const std::optional<Ranges<long long>>& ranges) const {
        (void)withInds;
        std::vector<std::pair<long long, V>> result;
        auto found = index_.find(key);
        if (found == index_.end()) return result;
        const KeyBucket& bucket = found->second;
        result.reserve(bucket.items.size());

        const auto& indexOf = buildIndexMap();

        if (!ordered) {
            for (Handle h : bucket.items) {
                const long long idx = indexOf.at(h);
                if (!ranges.has_value() || ranges->has(idx)) result.emplace_back(idx, self().rawValue(h));
            }
            return result;
        }
        for (Handle h : ensureSortedCache(bucket)) {
            const long long idx = indexOf.at(h);
            if (!ranges.has_value() || ranges->has(idx)) result.emplace_back(idx, self().rawValue(h));
        }
        return result;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    std::unordered_set<K, KeyHash, KeyEqual> BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::getKeys() const {
        std::unordered_set<K, KeyHash, KeyEqual> result;
        result.reserve(index_.size());
        for (const auto& kv : index_) {
            result.insert(kv.first);
        }
        return result;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    std::pair<const K&, const V&> BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::getByInd(long long index) const {
        Handle h = handleAtQueryIndex(index);
        return {self().rawKey(h), self().rawValue(h)};
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    std::pair<long long, V> BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::getByInd(long long index, bool withInds) const {
        (void)withInds;
        Handle h = handleAtQueryIndex(index);
        const KeyBucket& bucket = index_.at(self().rawKey(h));
        const auto& sorted = ensureSortedCache(bucket);
        long long occ = 0;
        for (Handle sh : sorted) {
            if (sh == h) break;
            ++occ;
        }
        return {occ, self().rawValue(h)};
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::setValByInd(long long index, const V& value) {
        Handle h = handleAtQueryIndex(index);
        self().rawSetValue(h, value);
    }

    // ------------------------------------------------------------------
    // BaseOrderedMultiMap<...>::Iterator
    // ------------------------------------------------------------------

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    typename BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator::Item
    BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator::operator*() const {
        auto found = seenCount_.find(owner_->iterKey(h_));
        const size_t occ = (found != seenCount_.end()) ? found->second : 0;
        return Item{owner_->iterKey(h_), owner_->iterValue(h_), occ, orderIndex_};
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    typename BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator&
    BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator::operator++() {
        ++seenCount_[owner_->iterKey(h_)]; // record this occurrence before moving on
        h_ = owner_->iterNext(h_);
        ++orderIndex_;
        return *this;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    typename BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator
    BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator::operator++(int) {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    bool BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator::operator==(const Iterator& other) const {
        return h_ == other.h_;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    bool BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator::operator!=(const Iterator& other) const {
        return h_ != other.h_;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator::Iterator(
        const BaseOrderedMultiMap* owner, Handle h, size_t orderIndex)
        : owner_(owner), h_(h), orderIndex_(orderIndex) {}

    // ------------------------------------------------------------------
    // BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual> (continued)
    // ------------------------------------------------------------------

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    typename BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator
    BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::begin() const {
        return Iterator(this, self().rawBegin(), 0);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    typename BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::Iterator
    BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::end() const {
        return Iterator(this, self().rawEnd(), self().rawSize());
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    std::vector<std::vector<Handle>> BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::computeSplitGroups(
        const std::vector<long long>& inds,
        bool includeSplitKVP,
        bool includeEmptyParts,
        bool sortIndices) const {
        const size_t n = self().rawSize();
        const long long nn = static_cast<long long>(n);

        std::vector<long long> splitPoints;
        splitPoints.reserve(inds.size());

        auto normalize = [&](long long r) -> long long {
            long long idx = r;
            if (idx < 0) idx += nn;
            if (n == 0 || idx < 0 || idx >= nn) {
                throw std::out_of_range(
                    "OrderedMultiMap::splitByInds: index out of range [-size(), size()-1]");
            }
            return idx;
        };

        if (sortIndices) {
            std::set<long long> dedupSorted;
            for (long long r : inds) dedupSorted.insert(normalize(r));
            splitPoints.assign(dedupSorted.begin(), dedupSorted.end());
        } else {
            for (long long r : inds) splitPoints.push_back(normalize(r));
        }

        std::vector<std::vector<Handle>> groups;
        Handle it = self().rawBegin();
        long long pos = 0;

        auto flushGroup = [&](long long endExclusive) {
            std::vector<Handle> group;
            while (pos < endExclusive) {
                group.push_back(it);
                it = self().rawNext(it);
                ++pos;
            }
            if (!group.empty() || includeEmptyParts) {
                groups.push_back(std::move(group));
            }
        };

        for (long long split : splitPoints) {
            flushGroup(split); // everything up to (not including) the split point
            if (!includeSplitKVP) {
                // Drop the entry at the split point entirely.
                it = self().rawNext(it);
                ++pos;
            }
        }
        flushGroup(nn); // remainder after the last split point

        return groups;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    const K& BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::iterKey(Handle h) const {
        return self().rawKey(h);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    const V& BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::iterValue(Handle h) const {
        return self().rawValue(h);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    Handle BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::iterNext(Handle h) const {
        return self().rawNext(h);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    template <typename IndexedMap>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::buildFromIndexed(const IndexedMap& indexed) {
        std::vector<std::tuple<long long, K, V>> flattened;
        for (const auto& kv : indexed) {
            for (const auto& iv : kv.second) {
                flattened.emplace_back(iv.first, kv.first, iv.second);
            }
        }
        std::stable_sort(flattened.begin(), flattened.end(),
                          [](const auto& a, const auto& b) {
                              return std::get<0>(a) < std::get<0>(b);
                          });
        for (auto& t : flattened) {
            insert(std::get<1>(t), std::get<2>(t));
        }
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    size_t BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::normalizeIndex(long long index, size_t n) {
        const long long nn = static_cast<long long>(n);
        if (index < 0) {
            index += nn + 1;
            if (index < 0) index = 0;
        } else if (index > nn) {
            index = nn;
        }
        return static_cast<size_t>(index);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    const std::vector<Handle>& BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::ensureSortedCache(
        const KeyBucket& bucket) const {
        if (bucket.dirty) {
            bucket.sortedCache.assign(bucket.items.begin(), bucket.items.end());
            std::sort(bucket.sortedCache.begin(), bucket.sortedCache.end(),
                      [this](Handle a, Handle b) { return self().rawPos(a) < self().rawPos(b); });
            bucket.dirty = false;
        }
        return bucket.sortedCache;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    const std::unordered_map<Handle, long long, HandleHash>&
    BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::buildIndexMap() const {
        if (indexMapDirty_) {
            indexMapCache_.clear();
            long long idx = 0;
            for (Handle h = self().rawBegin(); h != self().rawEnd(); h = self().rawNext(h)) {
                indexMapCache_[h] = idx++;
            }
            indexMapDirty_ = false;
        }
        return indexMapCache_;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    Handle BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::insertBefore(Handle pos, const K& key, const V& value) {
        const bool atGlobalEnd = (pos == self().rawEnd());
        const double label = computeLabel(pos);
        Handle h = self().rawInsertBefore(pos, key, value);
        self().rawSetPos(h, label);
        indexMapDirty_ = true;

        KeyBucket& bucket = index_[key];
        bucket.items.push_back(h);
        self().rawSetSelf(h, std::prev(bucket.items.end()));

        if (bucket.items.size() == 1) {
            bucket.sortedCache.assign(1, h);
            bucket.dirty = false;
        } else if (atGlobalEnd && !bucket.dirty) {
            bucket.sortedCache.push_back(h);
        } else {
            bucket.dirty = true;
        }
        return h;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    size_t BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::insertAllAtImpl(
        const std::vector<std::pair<long long, std::pair<K, V>>>& raw,
        bool sortIndices,
        const std::optional<Ranges<long long>>& ranges) {
        if (raw.empty()) return 0;
        const size_t n = self().rawSize();

        // Normalize every raw index against the ORIGINAL size (frozen once,
        // up front) -- matching insertAt()'s clamping rules and numpy-style
        // "position in the original sequence" semantics. Entries outside
        // `ranges` (if given) are dropped here.
        std::vector<std::pair<size_t, std::pair<K, V>>> normalized;
        normalized.reserve(raw.size());
        for (const auto& entry : raw) {
            size_t idx = normalizeIndex(entry.first, n);
            if (ranges.has_value() && !ranges->has(static_cast<long long>(idx))) {
                continue;
            }
            normalized.emplace_back(idx, entry.second);
        }
        if (normalized.empty()) return 0;

        // Stable sort so the cursor below only ever advances forward;
        // entries with equal normalized index keep the relative order they
        // arrived in `raw`. Skippable via sortIndices=false when the caller
        // already guarantees ascending order.
        if (sortIndices) {
            std::stable_sort(normalized.begin(), normalized.end(),
                              [](const auto& a, const auto& b) { return a.first < b.first; });
        }

        Handle h = self().rawBegin();
        size_t cursor = 0;
        for (const auto& entry : normalized) {
            while (cursor < entry.first) {
                h = self().rawNext(h);
                ++cursor;
            }
            // Re-derive h from the just-inserted entry (via rawNext) rather
            // than reusing the pre-insertion h directly -- see
            // insertAllStart()'s comment for why a cached handle isn't safe
            // to keep reusing across multiple mutating calls for a
            // rebalancing backing structure. This matters here specifically
            // for TIED target indices (multiple entries at the same cursor
            // position, where the while loop above doesn't run and the
            // previous h would otherwise get reused as-is).
            Handle newH = insertBefore(h, entry.second.first, entry.second.second);
            h = self().rawNext(newH);
        }
        return normalized.size();
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::reorderImpl(
        const std::vector<std::pair<long long, long long>>& raw,
        const std::optional<Ranges<long long>>& ranges) {
        const size_t n = self().rawSize();
        if (n == 0 || raw.empty()) return;
        const long long nn = static_cast<long long>(n);

        // --- Rule 1: resolve old-index (key) conflicts, and apply `ranges`
        // as a per-entry eligibility filter (an ineligible entry is treated
        // exactly like an unmentioned one -- it never gets pinned, so it
        // ends up floating).
        std::vector<char> pinned(n, 0);
        std::vector<long long> pinnedRawV(n);
        std::vector<long long> pinnedArrival(n);

        long long arrival = 0;
        for (const auto& kv : raw) {
            const long long rawOld = kv.first;
            if (rawOld < -nn || rawOld > nn - 1) {
                throw std::out_of_range(
                    "OrderedMultiMap::reorder: old index out of range [-size(), size()-1]");
            }
            const size_t oldPos = static_cast<size_t>(rawOld < 0 ? rawOld + nn : rawOld);
            const bool eligible = !ranges.has_value() || ranges->has(static_cast<long long>(oldPos));
            if (eligible && !pinned[oldPos]) {
                pinned[oldPos] = 1;
                pinnedRawV[oldPos] = kv.second;
                pinnedArrival[oldPos] = arrival;
            }
            ++arrival;
        }

        // --- Classify every element (pinned or floating) in one pass over
        // the current order, grabbing its handle.
        struct Clustered { Handle h; long long v; long long arrival; };
        struct Normal     { Handle h; long long target; long long arrival; };

        std::vector<Clustered> front, back;
        std::vector<Normal> normalPinned;
        std::vector<Handle> floating; // unmentioned entries, original relative order

        size_t idx = 0;
        for (Handle h = self().rawBegin(); h != self().rawEnd(); h = self().rawNext(h), ++idx) {
            if (pinned[idx]) {
                const long long v = pinnedRawV[idx];
                if (v < -nn) {
                    front.push_back({h, v, pinnedArrival[idx]});
                } else if (v >= nn) {
                    back.push_back({h, v, pinnedArrival[idx]});
                } else {
                    const long long target = (v < 0) ? v + nn : v;
                    normalPinned.push_back({h, target, pinnedArrival[idx]});
                }
            } else {
                floating.push_back(h);
            }
        }

        // Within each cluster: smaller raw value first, ties (rule 2) broken
        // by insertion/arrival order. Same for tied normal targets.
        auto byValThenArrival = [](const Clustered& a, const Clustered& b) {
            if (a.v != b.v) return a.v < b.v;
            return a.arrival < b.arrival;
        };
        std::stable_sort(front.begin(), front.end(), byValThenArrival);
        std::stable_sort(back.begin(), back.end(), byValThenArrival);
        std::stable_sort(normalPinned.begin(), normalPinned.end(),
                          [](const Normal& a, const Normal& b) {
                              if (a.target != b.target) return a.target < b.target;
                              return a.arrival < b.arrival;
                          });

        const size_t f = front.size();
        const size_t m = normalPinned.size() + floating.size(); // == n - f - back.size()

        // --- Build the desired final order: front cluster, then a merge of
        // normal-pinned targets with floating fill-ins, then the back
        // cluster. The merge places a pinned item as soon as the walk
        // reaches (or passes) its target -- which naturally handles tied
        // targets (they land consecutively, in arrival order) and targets
        // that spill into a cluster's territory without needing a separate
        // clamping step.
        std::vector<Handle> finalOrder;
        finalOrder.reserve(n);
        for (const auto& e : front) finalOrder.push_back(e.h);

        size_t normalPtr = 0, floatPtr = 0;
        for (size_t outPos = 0; outPos < m; ++outPos) {
            bool takeNormal;
            if (normalPtr >= normalPinned.size()) {
                takeNormal = false;
            } else if (floatPtr >= floating.size()) {
                takeNormal = true;
            } else {
                takeNormal = (normalPinned[normalPtr].target <=
                              static_cast<long long>(f + outPos));
            }
            if (takeNormal) {
                finalOrder.push_back(normalPinned[normalPtr].h);
                ++normalPtr;
            } else {
                finalOrder.push_back(floating[floatPtr]);
                ++floatPtr;
            }
        }

        for (const auto& e : back) finalOrder.push_back(e.h);

        // --- Physically relink every entry to match finalOrder. O(n),
        // implemented per-backing-structure.
        self().rawRelinkInOrder(finalOrder);

        // Position labels no longer reflect the new arrangement, and any
        // key's sorted cache may now be stale (its entries could have moved
        // arbitrarily far apart) -- relabel once and mark every key dirty.
        // Call/insertion order (bucket.items, used by getAll(key,
        // ordered=false)) is untouched by a reorder, by design: reorder
        // only changes *positional* order.
        renumberAll();
        for (auto& kv : index_) kv.second.dirty = true;

        // rawRelinkInOrder() just changed the physical walk order without
        // going through insertBefore()/eraseHandle() (the two places that
        // otherwise dirty this on every mutation), so it has to be marked
        // here explicitly too.
        indexMapDirty_ = true;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::replaceValsImpl(
        const std::vector<std::pair<K, ReplaceSpec>>& raw, bool addNew,
        const std::optional<Ranges<long long>>& ranges) {
        // Built lazily (only when ranges is given) via buildIndexMap(): an
        // unavoidable one-time O(n) pass, paid once per call rather than
        // once per entry.
        std::unordered_map<Handle, long long, HandleHash> indexOf;
        if (ranges.has_value()) {
            indexOf = buildIndexMap();
        }
        auto inRanges = [&](Handle h) {
            if (!ranges.has_value()) return true;
            return ranges->has(indexOf.at(h));
        };

        for (const auto& entry : raw) {
            const K& key = entry.first;
            const ReplaceSpec& spec = entry.second;

            auto found = index_.find(key);
            if (found == index_.end()) {
                if (!addNew) continue;
                // unaffected by ranges -- see replaceVals()'s doc comment
                std::visit([&](const auto& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, V>) {
                        insert(key, val);
                    } else if constexpr (std::is_same_v<T, std::vector<V>>) {
                        for (const V& v : val) insert(key, v);
                    } else { // std::pair<V, Predicate> -- predicate has
                             // nothing to test yet, so just add the value.
                        insert(key, val.first);
                    }
                }, spec);
                continue;
            }

            KeyBucket& bucket = found->second;
            std::visit([&](const auto& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, V>) {
                    for (Handle h : bucket.items) {
                        if (inRanges(h)) self().rawSetValue(h, val);
                    }
                } else if constexpr (std::is_same_v<T, std::vector<V>>) {
                    // Positional order: the i-th entry (true left-to-right
                    // order) gets val[i]. Extra list entries beyond
                    // count(key) are simply unused, not appended.
                    auto& sorted = ensureSortedCache(bucket);
                    const size_t limit = std::min(sorted.size(), val.size());
                    for (size_t i = 0; i < limit; ++i) {
                        if (inRanges(sorted[i])) self().rawSetValue(sorted[i], val[i]);
                    }
                } else { // std::pair<V, Predicate>
                    const V& newValue = val.first;
                    const Predicate& pred = val.second;
                    for (Handle h : bucket.items) {
                        if (pred(self().rawValue(h)) && inRanges(h)) self().rawSetValue(h, newValue);
                    }
                }
            }, spec);
        }
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::eraseHandle(Handle h) {
        auto found = index_.find(self().rawKey(h));
        KeyBucket& bucket = found->second;
        bucket.items.erase(self().rawSelf(h));
        bucket.dirty = true;
        indexMapDirty_ = true;
        if (bucket.items.empty()) index_.erase(found);
        self().rawErase(h);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    double BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::computeLabel(Handle pos) {
        const bool hasPrev = (pos != self().rawBegin());
        const bool hasNext = (pos != self().rawEnd());

        if (!hasPrev && !hasNext) return 0.0;

        if (hasPrev && hasNext) {
            Handle prevH = self().rawPrev(pos);
            double lo = self().rawPos(prevH);
            double hi = self().rawPos(pos);
            double mid = lo + (hi - lo) / 2.0;
            if (!(mid > lo && mid < hi)) {
                renumberAll();
                lo = self().rawPos(prevH);
                hi = self().rawPos(pos);
                mid = lo + (hi - lo) / 2.0;
            }
            return mid;
        }
        if (hasPrev) return self().rawPos(self().rawPrev(pos)) + 1.0;
        return self().rawPos(pos) - 1.0;
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    void BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::renumberAll() {
        double label = 0.0;
        for (Handle h = self().rawBegin(); h != self().rawEnd(); h = self().rawNext(h), label += 1.0) {
            self().rawSetPos(h, label);
        }
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    Handle BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::handleAtInsertionSlot(size_t pos) const {
        if (pos == self().rawSize()) return self().rawEnd();
        return self().rawAtIndex(pos);
    }

    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash, typename KeyHash, typename KeyEqual>
    Handle BaseOrderedMultiMap<Derived, K, V, Handle, HandleHash, KeyHash, KeyEqual>::handleAtQueryIndex(long long index) const {
        const size_t n = self().rawSize();
        if (n == 0) {
            throw std::out_of_range("OrderedMultiMap::getByInd: map is empty");
        }
        const long long nn = static_cast<long long>(n);
        long long idx = index;
        if (idx < 0) idx += nn;
        if (idx < 0 || idx >= nn) {
            throw std::out_of_range(
                "OrderedMultiMap::getByInd: index out of range [-size(), size()-1]");
        }
        return self().rawAtIndex(static_cast<size_t>(idx));
    }
}
