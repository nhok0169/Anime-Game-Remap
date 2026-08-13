#include "AGRemapCore/tools/orderedMultiMap/OrderedMultiMap.h"


namespace AGRemapCore {

    // ------------------------------------------------------------------
    // detail::ListHandleHash<K, V>
    // ------------------------------------------------------------------

    template <typename K, typename V>
    size_t detail::ListHandleHash<K, V>::operator()(detail::ListHandle<K, V> h) const {
        return std::hash<const void*>()(&*h);
    }

    // ------------------------------------------------------------------
    // OrderedMultiMap<K, V, KeyHash, KeyEqual>
    // ------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMap<K, V, KeyHash, KeyEqual>::OrderedMultiMap(const OrderedMultiMap& other) {
        for (auto h = other.order_.begin(); h != other.order_.end(); ++h) {
            this->insert(h->key, h->value);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMap<K, V, KeyHash, KeyEqual>& OrderedMultiMap<K, V, KeyHash, KeyEqual>::operator=(const OrderedMultiMap& other) {
        if (this == &other) return *this;
        order_.clear();
        this->index_.clear();
        for (auto h = other.order_.begin(); h != other.order_.end(); ++h) {
            this->insert(h->key, h->value);
        }
        return *this;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMap<K, V, KeyHash, KeyEqual>::OrderedMultiMap(const std::vector<std::pair<K, V>>& items) : OrderedMultiMap() {
        for (const auto& p : items) this->insert(p.first, p.second);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMap<K, V, KeyHash, KeyEqual>::OrderedMultiMap(
        const std::unordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>& indexed)
        : OrderedMultiMap() {
        this->buildFromIndexed(indexed);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMap<K, V, KeyHash, KeyEqual>::OrderedMultiMap(
        const std::map<K, std::vector<std::pair<long long, V>>>& indexed)
        : OrderedMultiMap() {
        this->buildFromIndexed(indexed);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const typename OrderedMultiMap<K, V, KeyHash, KeyEqual>::Order& OrderedMultiMap<K, V, KeyHash, KeyEqual>::entries() const {
        return order_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<OrderedMultiMap<K, V, KeyHash, KeyEqual>> OrderedMultiMap<K, V, KeyHash, KeyEqual>::splitByInds(
        const tsl::ordered_set<long long>& inds,
        bool includeSplitKVP,
        bool includeEmptyParts,
        bool sortIndices) const {
        return groupsToParts(this->computeSplitGroups(
            std::vector<long long>(inds.begin(), inds.end()),
            includeSplitKVP, includeEmptyParts, sortIndices));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<OrderedMultiMap<K, V, KeyHash, KeyEqual>> OrderedMultiMap<K, V, KeyHash, KeyEqual>::splitByInds(
        const std::set<long long>& inds,
        bool includeSplitKVP,
        bool includeEmptyParts,
        bool sortIndices) const {
        return groupsToParts(this->computeSplitGroups(
            std::vector<long long>(inds.begin(), inds.end()),
            includeSplitKVP, includeEmptyParts, sortIndices));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<OrderedMultiMap<K, V, KeyHash, KeyEqual>> OrderedMultiMap<K, V, KeyHash, KeyEqual>::splitByInds(
        const std::unordered_set<long long>& inds,
        bool includeSplitKVP,
        bool includeEmptyParts) const {
        return groupsToParts(this->computeSplitGroups(
            std::vector<long long>(inds.begin(), inds.end()),
            includeSplitKVP, includeEmptyParts, /*sortIndices=*/true));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<OrderedMultiMap<K, V, KeyHash, KeyEqual>> OrderedMultiMap<K, V, KeyHash, KeyEqual>::groupsToParts(
        std::vector<std::vector<Handle>>&& groups) const {
        std::vector<OrderedMultiMap<K, V, KeyHash, KeyEqual>> parts;
        parts.reserve(groups.size());
        for (auto& group : groups) {
            OrderedMultiMap<K, V, KeyHash, KeyEqual> part;
            for (Handle h : group) part.insert(h->key, h->value);
            parts.push_back(std::move(part));
        }
        return parts;
    }

    // ------------------------------------------------------------------
    // primitives required by BaseOrderedMultiMap
    // ------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawSize() const { return order_.size(); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMap<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawBegin() const {
        return const_cast<Order&>(order_).begin();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMap<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawEnd() const {
        return const_cast<Order&>(order_).end();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMap<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawNext(Handle h) const {
        return std::next(h);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMap<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawPrev(Handle h) const {
        return std::prev(h);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMap<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawAtIndex(size_t idx) const {
        // The tactical fix: walk from whichever end is closer.
        const size_t n = order_.size();
        Order& mutableOrder = const_cast<Order&>(order_);
        if (idx <= n - 1 - idx) {
            return std::next(mutableOrder.begin(), static_cast<long>(idx));
        }
        return std::prev(mutableOrder.end(), static_cast<long>(n - idx));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMap<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawInsertBefore(
        Handle pos, const K& key, const V& value) {
        return order_.insert(pos, Entry{key, value, 0.0, {}});
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawErase(Handle h) { order_.erase(h); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const K& OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawKey(Handle h) const { return h->key; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const V& OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawValue(Handle h) const { return h->value; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawSetValue(Handle h, const V& value) { h->value = value; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    double OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawPos(Handle h) const { return h->pos; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawSetPos(Handle h, double p) { h->pos = p; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename std::list<typename OrderedMultiMap<K, V, KeyHash, KeyEqual>::Handle>::iterator
    OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawSelf(Handle h) const { return h->self; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawSetSelf(
        Handle h, typename std::list<Handle>::iterator self) { h->self = self; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawClear() { order_.clear(); }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMap<K, V, KeyHash, KeyEqual>::rawRelinkInOrder(const std::vector<Handle>& order) {
        for (Handle h : order) {
            order_.splice(order_.end(), order_, h);
        }
    }
}
