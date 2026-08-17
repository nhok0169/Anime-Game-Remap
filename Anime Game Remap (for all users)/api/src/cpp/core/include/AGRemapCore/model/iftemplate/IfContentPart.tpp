#include "AGRemapCore/model/iftemplate/IfContentPart.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<IOrderedMultiMap<K, V>> IfContentPart<K, V, KeyHash, KeyEqual>::makeDefaultContent() {
        if constexpr (hasDefaultHashableContent) {
            return std::make_unique<OrderedMultiMapListAdapter<K, V, KeyHash, KeyEqual>>();
        } else {
            // Unreachable from the pybind11 binding layer (PyIfContentPart's Python-facing
            // constructor always supplies an explicit 'content' of its own, built with
            // PyObjectHash/PyObjectEqual as this instantiation's KeyHash/KeyEqual, before ever
            // reaching this constructor) -- this only guards a direct C++ caller constructing
            // IfContentPart<K, V, KeyHash, KeyEqual> with no 'content' while KeyHash is left at
            // its default (std::hash<K>) for a K that has no usable specialization.
            throw std::logic_error(
                "IfContentPart<K, V, KeyHash, KeyEqual>: no 'content' was given and KeyHash has no usable "
                "default construction for this K, so no default implementation could be constructed -- an "
                "implementation must be supplied explicitly for this K, V combination");
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IfContentPart<K, V, KeyHash, KeyEqual>::IfContentPart(int depth, std::unique_ptr<IOrderedMultiMap<K, V>> content,
                                                           const std::optional<size_t>& id)
        : IfTemplatePart(id), content_(content ? std::move(content) : makeDefaultContent()), depth_(depth) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IfContentPart<K, V, KeyHash, KeyEqual>::IfContentPart(
        const tsl::ordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>& src,
        int depth, std::unique_ptr<IOrderedMultiMap<K, V>> content, const std::optional<size_t>& id)
        : IfTemplatePart(id), content_(content ? std::move(content) : makeDefaultContent()), depth_(depth) {
        if (!src.empty()) {
            populateFromIndexed(src);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IfContentPart<K, V, KeyHash, KeyEqual>::IfContentPart(
        const std::unordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>& src,
        int depth, std::unique_ptr<IOrderedMultiMap<K, V>> content, const std::optional<size_t>& id)
        : IfTemplatePart(id), content_(content ? std::move(content) : makeDefaultContent()), depth_(depth) {
        if (!src.empty()) {
            populateFromIndexed(src);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IfContentPart<K, V, KeyHash, KeyEqual>::IfContentPart(
        const std::vector<std::pair<K, V>>& src, int depth, std::unique_ptr<IOrderedMultiMap<K, V>> content,
        const std::optional<size_t>& id)
        : IfTemplatePart(id), content_(content ? std::move(content) : makeDefaultContent()), depth_(depth) {
        if (!src.empty()) {
            content_->insertAllEnd(src);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    template <typename IndexedMap>
    void IfContentPart<K, V, KeyHash, KeyEqual>::populateFromIndexed(const IndexedMap& src) {
        std::vector<std::pair<long long, std::pair<K, V>>> triples;

        size_t total = 0;
        for (const auto& entry : src) total += entry.second.size();
        triples.reserve(total);

        for (const auto& entry : src) {
            for (const auto& indexedVal : entry.second) {
                triples.emplace_back(indexedVal.first, std::make_pair(entry.first, indexedVal.second));
            }
        }

        std::stable_sort(triples.begin(), triples.end(),
                          [](const auto& a, const auto& b) { return a.first < b.first; });

        std::vector<std::pair<K, V>> ordered;
        ordered.reserve(triples.size());
        for (auto& triple : triples) {
            ordered.push_back(std::move(triple.second));
        }

        content_->insertAllEnd(ordered);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<IfContentPart<K, V, KeyHash, KeyEqual>> IfContentPart<K, V, KeyHash, KeyEqual>::clone(bool newId) const {
        return std::make_unique<IfContentPart<K, V, KeyHash, KeyEqual>>(
            depth_, content_->clone(), newId ? std::nullopt : std::optional<size_t>(id()));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    int IfContentPart<K, V, KeyHash, KeyEqual>::depth() const { return depth_; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::setDepth(int depth) { depth_ = depth; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IOrderedMultiMap<K, V>& IfContentPart<K, V, KeyHash, KeyEqual>::content() { return *content_; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const IOrderedMultiMap<K, V>& IfContentPart<K, V, KeyHash, KeyEqual>::content() const { return *content_; }

    // ---- insert family ----

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::addKVP(const K& key, const V& value) {
        content_->insert(key, value);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::addKVPToFront(const K& key, const V& value) {
        content_->insertStart(key, value);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::addKVPAt(long long index, const K& key, const V& value) {
        content_->insertAt(index, key, value);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::addKVPs(const std::vector<std::pair<K, V>>& kvps) {
        content_->insertAllEnd(kvps);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::addKVPsToFront(const std::vector<std::pair<K, V>>& kvps) {
        content_->insertAllStart(kvps);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t IfContentPart<K, V, KeyHash, KeyEqual>::addKVPsByInds(const std::vector<std::pair<long long, std::pair<K, V>>>& kvps,
                                               bool sortIndices,
                                               const std::optional<RangeSpec>& ranges) {
        return content_->insertAllAt(kvps, sortIndices, ranges);
    }

    // ---- removal ----

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool IfContentPart<K, V, KeyHash, KeyEqual>::removeKVPAt(size_t pos, const std::optional<RangeSpec>& ranges) {
        return content_->removeAt(pos, ranges);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t IfContentPart<K, V, KeyHash, KeyEqual>::removeKey(const K& key, const std::optional<RangeSpec>& ranges,
                                           const std::optional<RemoveKeyCheck>& check) {
        return content_->removeKey(key, ranges, check);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t IfContentPart<K, V, KeyHash, KeyEqual>::removeKeys(const std::vector<std::pair<K, std::optional<RemoveKeyCheck>>>& keys,
                                            const std::optional<RangeSpec>& ranges) {
        size_t total = 0;
        for (const auto& entry : keys) {
            total += content_->removeKey(entry.first, ranges, entry.second);
        }
        return total;
    }

    // ---- bulk edits ----

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::reorder(const std::vector<std::pair<long long, long long>>& orderMap,
                                       const std::optional<RangeSpec>& ranges) {
        content_->reorder(orderMap, ranges);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::remapKeys(const std::vector<std::pair<K, KeyRemapValue>>& keyRemap,
                                         const std::optional<RangeSpec>& ranges) {
        content_->remapKeys(keyRemap, ranges);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::replaceVals(const std::vector<std::pair<K, ReplaceSpec>>& newVals, bool addNew,
                                           const std::optional<RangeSpec>& ranges) {
        content_->replaceVals(newVals, addNew, ranges);
    }

    // ---- lookups ----

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool IfContentPart<K, V, KeyHash, KeyEqual>::contains(const K& key) const { return content_->contains(key); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool IfContentPart<K, V, KeyHash, KeyEqual>::containsKey(const K& key) const { return content_->containsKey(key); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t IfContentPart<K, V, KeyHash, KeyEqual>::count(const K& key) const { return content_->count(key); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t IfContentPart<K, V, KeyHash, KeyEqual>::size() const { return content_->size(); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t IfContentPart<K, V, KeyHash, KeyEqual>::length() const { return content_->length(); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t IfContentPart<K, V, KeyHash, KeyEqual>::keySize() const { return content_->keySize(); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool IfContentPart<K, V, KeyHash, KeyEqual>::empty() const { return content_->empty(); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<V> IfContentPart<K, V, KeyHash, KeyEqual>::getVals(const K& key, bool ordered, const std::optional<RangeSpec>& ranges) const {
        return content_->getAll(key, ordered, ranges);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::pair<long long, V>> IfContentPart<K, V, KeyHash, KeyEqual>::getValsWithInds(const K& key, bool ordered, const std::optional<RangeSpec>& ranges) const {
        return content_->getAllWithInds(key, ordered, ranges);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_set<K, KeyHash, KeyEqual> IfContentPart<K, V, KeyHash, KeyEqual>::getKeys() const {
        auto keys = content_->getKeys();
        return std::unordered_set<K, KeyHash, KeyEqual>(keys.begin(), keys.end());
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<K, V> IfContentPart<K, V, KeyHash, KeyEqual>::getByInd(long long index) const {
        return content_->getByInd(index);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<long long, V> IfContentPart<K, V, KeyHash, KeyEqual>::getByIndWithOccurrence(long long index) const {
        return content_->getByIndWithOccurrence(index);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IfContentPart<K, V, KeyHash, KeyEqual>::setValByInd(long long index, const V& value) {
        content_->setValByInd(index, value);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::pair<K, V>> IfContentPart<K, V, KeyHash, KeyEqual>::entries() const {
        return content_->entries();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<typename IfContentPart<K, V, KeyHash, KeyEqual>::Item> IfContentPart<K, V, KeyHash, KeyEqual>::items() const {
        return content_->items();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::unique_ptr<IfContentPart<K, V, KeyHash, KeyEqual>>> IfContentPart<K, V, KeyHash, KeyEqual>::splitByInds(
        const std::vector<long long>& inds, bool includeSplitKVP,
        bool includeEmptyParts, bool sortIndices) const {
        std::vector<std::unique_ptr<IOrderedMultiMap<K, V>>> parts =
            content_->splitByInds(inds, includeSplitKVP, includeEmptyParts, sortIndices);

        std::vector<std::unique_ptr<IfContentPart<K, V, KeyHash, KeyEqual>>> result;
        result.reserve(parts.size());
        for (auto& part : parts) {
            result.push_back(std::make_unique<IfContentPart<K, V, KeyHash, KeyEqual>>(depth_, std::move(part)));
        }
        return result;
    }

}
