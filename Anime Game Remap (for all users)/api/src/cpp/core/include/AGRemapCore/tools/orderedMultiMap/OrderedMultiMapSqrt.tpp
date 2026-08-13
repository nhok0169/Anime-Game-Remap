#include "AGRemapCore/tools/orderedMultiMap/OrderedMultiMapSqrt.h"


namespace AGRemapCore {

    // ------------------------------------------------------------------
    // sqrt_detail::SqrtHandleHash<K, V>
    // ------------------------------------------------------------------

    template <typename K, typename V>
    size_t sqrt_detail::SqrtHandleHash<K, V>::operator()(sqrt_detail::SqrtHandle<K, V> h) const {
        return std::hash<const void*>()(&*h);
    }

    // ------------------------------------------------------------------
    // OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>
    // ------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::OrderedMultiMapSqrt() {
        blocks_.emplace_back(); // invariant: always >= 1 block
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::OrderedMultiMapSqrt(const OrderedMultiMapSqrt& other) : OrderedMultiMapSqrt() {
        for (const Block& b : other.blocks_) {
            for (const Entry& e : b.items) this->insert(e.key, e.value);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>& OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::operator=(const OrderedMultiMapSqrt& other) {
        if (this == &other) return *this;
        blocks_.clear();
        blocks_.emplace_back();
        totalSize_ = 0;
        this->index_.clear();
        for (const Block& b : other.blocks_) {
            for (const Entry& e : b.items) this->insert(e.key, e.value);
        }
        return *this;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::OrderedMultiMapSqrt(const std::vector<std::pair<K, V>>& items)
        : OrderedMultiMapSqrt() {
        for (const auto& p : items) this->insert(p.first, p.second);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::OrderedMultiMapSqrt(
        const std::unordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>& indexed)
        : OrderedMultiMapSqrt() {
        this->buildFromIndexed(indexed);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::OrderedMultiMapSqrt(
        const std::map<K, std::vector<std::pair<long long, V>>>& indexed)
        : OrderedMultiMapSqrt() {
        this->buildFromIndexed(indexed);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::pair<K, V>> OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::entries() const {
        std::vector<std::pair<K, V>> result;
        result.reserve(totalSize_);
        for (const Block& b : blocks_) {
            for (const Entry& e : b.items) result.emplace_back(e.key, e.value);
        }
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>> OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::splitByInds(
        const tsl::ordered_set<long long>& inds,
        bool includeSplitKVP,
        bool includeEmptyParts,
        bool sortIndices) const {
        return groupsToParts(this->computeSplitGroups(
            std::vector<long long>(inds.begin(), inds.end()),
            includeSplitKVP, includeEmptyParts, sortIndices));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>> OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::splitByInds(
        const std::set<long long>& inds,
        bool includeSplitKVP,
        bool includeEmptyParts,
        bool sortIndices) const {
        return groupsToParts(this->computeSplitGroups(
            std::vector<long long>(inds.begin(), inds.end()),
            includeSplitKVP, includeEmptyParts, sortIndices));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>> OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::splitByInds(
        const std::unordered_set<long long>& inds,
        bool includeSplitKVP,
        bool includeEmptyParts) const {
        return groupsToParts(this->computeSplitGroups(
            std::vector<long long>(inds.begin(), inds.end()),
            includeSplitKVP, includeEmptyParts, /*sortIndices=*/true));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>> OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::groupsToParts(
        std::vector<std::vector<Handle>>&& groups) const {
        std::vector<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>> parts;
        parts.reserve(groups.size());
        for (auto& group : groups) {
            OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual> part;
            for (Handle h : group) part.insert(h->key, h->value);
            parts.push_back(std::move(part));
        }
        return parts;
    }

    // ------------------------------------------------------------------
    // primitives required by BaseOrderedMultiMap
    // ------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawSize() const { return totalSize_; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawBegin() const {
        return blocks_.front().items.begin();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawEnd() const {
        return blocks_.back().items.end();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawNext(Handle h) const {
        BlockIt blk = h->ownerBlock;
        Handle nextInBlock = std::next(h);
        if (nextInBlock != blk->items.end()) return nextInBlock;
        // Advance to the next block that has anything in it; if we run off
        // the end of blocks_, that means `blk` was the last block, so
        // nextInBlock (== blk->items.end() == rawEnd()) is correct.
        BlockIt nb = std::next(blk);
        while (nb != blocks_.end() && nb->items.empty()) ++nb;
        if (nb == blocks_.end()) return nextInBlock; // == rawEnd()
        return nb->items.begin();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawPrev(Handle h) const {
        // computeLabel() can legitimately call this with h == rawEnd(): for
        // a plain std::list, std::prev(end()) is well-defined for free (the
        // sentinel node has a valid prev pointer to the tail). Our block
        // design doesn't get that automatically -- h == rawEnd() isn't a
        // real, dereferenceable entry, so it must be handled before ever
        // touching h->ownerBlock.
        if (h == rawEnd()) {
            for (BlockIt b = std::prev(blocks_.end());; ) {
                if (!b->items.empty()) return std::prev(b->items.end());
                if (b == blocks_.begin()) break; // unreachable if rawSize() > 0
                --b;
            }
            return h; // unreachable if rawSize() > 0
        }
        BlockIt blk = h->ownerBlock;
        if (h != blk->items.begin()) return std::prev(h);
        BlockIt pb = blk;
        while (pb != blocks_.begin()) {
            --pb;
            if (!pb->items.empty()) return std::prev(pb->items.end());
        }
        return blk->items.begin(); // shouldn't happen if h != rawBegin()
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawAtIndex(size_t idx) const {
        size_t accumulated = 0;
        for (BlockIt b = blocks_.begin(); b != blocks_.end(); ++b) {
            const size_t sz = b->items.size();
            if (idx < accumulated + sz) {
                const size_t offset = idx - accumulated;
                // walk from whichever end of the BLOCK is closer -- same
                // tactical idea as the plain list version, applied locally.
                if (offset <= sz - 1 - offset) {
                    return std::next(b->items.begin(), static_cast<long>(offset));
                }
                return std::prev(b->items.end(), static_cast<long>(sz - offset));
            }
            accumulated += sz;
        }
        throw std::out_of_range("OrderedMultiMapSqrt::rawAtIndex: index out of range");
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::Handle OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawInsertBefore(
        Handle pos, const K& key, const V& value) {
        BlockIt blk = (pos == rawEnd()) ? std::prev(blocks_.end()) : pos->ownerBlock;
        Handle h = blk->items.insert(pos, Entry{key, value, 0.0, {}, blk});
        ++totalSize_;
        rebalanceAfterInsert(blk);
        return h;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawErase(Handle h) {
        BlockIt blk = h->ownerBlock;
        blk->items.erase(h);
        --totalSize_;
        rebalanceAfterErase(blk);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const K& OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawKey(Handle h) const { return h->key; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const V& OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawValue(Handle h) const { return h->value; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawSetValue(Handle h, const V& value) { h->value = value; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    double OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawPos(Handle h) const { return h->pos; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawSetPos(Handle h, double p) { h->pos = p; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename std::list<typename OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::Handle>::iterator
    OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawSelf(Handle h) const { return h->self; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawSetSelf(
        Handle h, typename std::list<Handle>::iterator self) { h->self = self; }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawClear() {
        blocks_.clear();
        blocks_.emplace_back();
        totalSize_ = 0;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rawRelinkInOrder(const std::vector<Handle>& order) {
        BlockContainer newBlocks;
        if (order.empty()) {
            newBlocks.emplace_back();
        } else {
            const size_t B = targetBlockSize();
            size_t i = 0;
            while (i < order.size()) {
                BlockIt newBlk = newBlocks.emplace(newBlocks.end());
                const size_t end = std::min(order.size(), i + B);
                for (; i < end; ++i) {
                    Handle h = order[i];
                    BlockIt srcBlk = h->ownerBlock;
                    newBlk->items.splice(newBlk->items.end(), srcBlk->items, h);
                    h->ownerBlock = newBlk;
                }
            }
        }
        blocks_ = std::move(newBlocks);
    }

    // ------------------------------------------------------------------
    // rebalancing
    // ------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    size_t OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::targetBlockSize() const {
        return std::max<size_t>(1, static_cast<size_t>(
            std::sqrt(static_cast<double>(std::max<size_t>(1, totalSize_)))));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::relinkOwner(Block& blk, BlockIt blkIt) {
        for (Entry& e : blk.items) e.ownerBlock = blkIt;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rebalanceAfterInsert(BlockIt blk) {
        const size_t B = targetBlockSize();
        if (blk->items.size() <= 2 * B) return;

        // Split blk in half: new block gets the second half, inserted
        // right after blk in the block list.
        BlockIt newBlk = blocks_.insert(std::next(blk), Block{});
        const size_t half = blk->items.size() / 2;
        auto splitPoint = blk->items.begin();
        std::advance(splitPoint, half);
        newBlk->items.splice(newBlk->items.begin(), blk->items, splitPoint, blk->items.end());
        relinkOwner(*newBlk, newBlk);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>::rebalanceAfterErase(BlockIt blk) {
        const size_t B = targetBlockSize();
        if (blk->items.size() >= (B + 1) / 2) return; // still at/above B/2 (rounded up), fine
        if (blocks_.size() == 1) return; // nothing to merge with, and that's fine

        // Merge into a neighbor -- prefer the previous block if it exists,
        // else the next one.
        BlockIt other;
        bool mergeIntoPrev;
        if (blk != blocks_.begin()) {
            other = std::prev(blk);
            mergeIntoPrev = true;
        } else {
            other = std::next(blk);
            mergeIntoPrev = false;
        }

        if (mergeIntoPrev) {
            other->items.splice(other->items.end(), blk->items);
            relinkOwner(*other, other);
            blocks_.erase(blk);
            rebalanceAfterInsert(other); // in case the merge overshot 2B
        } else {
            blk->items.splice(blk->items.end(), other->items);
            relinkOwner(*blk, blk);
            blocks_.erase(other);
            rebalanceAfterInsert(blk);
        }
    }
}
