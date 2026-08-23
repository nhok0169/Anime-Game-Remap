#ifndef AGRemapCore_OrderedMultiMapSqrt_H
#define AGRemapCore_OrderedMultiMapSqrt_H

#include <cmath>
#include <functional>
#include <list>
#include <set>
#include <unordered_set>
#include <utility>

#include <tsl/ordered_set.h>

#include "AGRemapCore/tools/orderedMultiMap/BaseOrderedMultiMap.h"


namespace AGRemapCore {

    // Internal storage details for OrderedMultiMapSqrt -- not part of the
    // public API, kept in a `sqrt_detail` namespace with plain
    // (non-Doxygen) comments.
    namespace sqrt_detail {
        template <typename K, typename V> struct SqrtEntry;
        template <typename K, typename V> struct SqrtBlock;

        template <typename K, typename V>
        using SqrtEntryList = std::list<SqrtEntry<K, V>>;

        template <typename K, typename V>
        using SqrtHandle = typename SqrtEntryList<K, V>::iterator;

        template <typename K, typename V>
        using SqrtBlockContainer = std::list<SqrtBlock<K, V>>;

        template <typename K, typename V>
        using SqrtBlockIt = typename SqrtBlockContainer<K, V>::iterator;

        template <typename K, typename V>
        struct SqrtEntry {
            K key;
            V value;
            double pos;
            typename std::list<SqrtHandle<K, V>>::iterator self; // KeyBucket backlink, same role as the list version
            SqrtBlockIt<K, V> ownerBlock;                          // which block currently owns this entry
        };

        template <typename K, typename V>
        struct SqrtBlock {
            SqrtEntryList<K, V> items;
        };

        // Defined here (not nested in OrderedMultiMapSqrt) for the same
        // reason as detail::ListHandleHash in OrderedMultiMap.h.
        template <typename K, typename V>
        struct SqrtHandleHash {
            size_t operator()(SqrtHandle<K, V> h) const;
        };
    }

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseOrderedMultiMap`

     The :math:`O(\\sqrt{n})` sqrt-decomposition backing structure: the sequence is split
     into roughly :math:`\\sqrt{n}` blocks. Positional access walks the block list
     accumulating sizes (:math:`O(\\text{number of blocks})`), then walks within the target
     block (:math:`O(\\text{block size})`) -- :math:`O(\\sqrt{n})` total, versus a plain
     list's :math:`O(n)` worst case for a middle index. :raw-html:`<br />` :raw-html:`<br />`

     Each block is itself a ``std::list`` (not a ``std::vector``), and the top-level
     sequence of blocks is also a ``std::list`` rather than a ``std::vector`` -- both
     needed to keep every entry's address permanently stable, since ``KeyBucket::items``
     and each entry's own backlink hold handles/iterators into each other that must
     survive insertions/erasures anywhere else in the structure. Each entry additionally
     stores an iterator back to its owning block, updated whenever rebalancing moves it to
     a different block. :raw-html:`<br />` :raw-html:`<br />`

     Rebalancing targets a block size of :math:`B = \\max(1, \\lfloor\\sqrt{n}\\rfloor)`,
     recomputed on the fly: after an insert, a block exceeding :math:`2B` is split in half;
     after an erase, a block dropping below :math:`B/2` (when it isn't the only block) is
     merged into a neighbor, re-splitting immediately if that overshoots :math:`2B`. This
     keeps amortized rebalancing cost proportional to :math:`O(1)` on top of the
     :math:`O(\\sqrt{n})` search cost that already dominates.
     @endrst
     *
     * @tparam K The type of the keys stored in the map
     * @tparam V The type of the values stored in the map
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class OrderedMultiMapSqrt
        : public BaseOrderedMultiMap<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>, K, V,
                                      sqrt_detail::SqrtHandle<K, V>, sqrt_detail::SqrtHandleHash<K, V>,
                                      KeyHash, KeyEqual> {
        public:
            using Handle = sqrt_detail::SqrtHandle<K, V>;
            using HandleHash = sqrt_detail::SqrtHandleHash<K, V>;

        private:
            using Entry = sqrt_detail::SqrtEntry<K, V>;
            using Block = sqrt_detail::SqrtBlock<K, V>;
            using BlockContainer = sqrt_detail::SqrtBlockContainer<K, V>;
            using BlockIt = sqrt_detail::SqrtBlockIt<K, V>;
            using Base = BaseOrderedMultiMap<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>, K, V, Handle, HandleHash, KeyHash, KeyEqual>;
            friend Base;

            // mutable: rawBegin() etc. need non-const iterators even when
            // called const (see the same note in OrderedMultiMap.h).
            mutable BlockContainer blocks_;
            size_t totalSize_ = 0;

        public:
            /**
             * @brief
             @rst
             Constructs an empty OrderedMultiMapSqrt, with the "always at least one block"
             invariant already established
             @endrst
             */
            OrderedMultiMapSqrt();

            /**
             * @brief Move-constructs an OrderedMultiMapSqrt
             */
            OrderedMultiMapSqrt(OrderedMultiMapSqrt&&) = default;

            /**
             * @brief Move-assigns an OrderedMultiMapSqrt
             *
             * @return This map, after the assignment
             */
            OrderedMultiMapSqrt& operator=(OrderedMultiMapSqrt&&) = default;

            /**
             * @brief
             @rst
             Copy-constructs an OrderedMultiMapSqrt, rebuilding via :cpp:func:`insert` (which
             correctly wires up every cross-referencing handle fresh) rather than a naive
             member-wise copy, which would leave handles pointing at the original object's
             nodes.
             @endrst
             *
             * @param other The map to copy
             */
            OrderedMultiMapSqrt(const OrderedMultiMapSqrt& other);

            /**
             * @copydoc OrderedMultiMapSqrt(const OrderedMultiMapSqrt&)
             *
             * @return This map, after the assignment
             */
            OrderedMultiMapSqrt& operator=(const OrderedMultiMapSqrt& other);

            /**
             * @brief Constructs from a list of key-value pairs, inserted at the end in the order given
             *
             * @param items The key-value pairs to insert, in order
             */
            explicit OrderedMultiMapSqrt(const std::vector<std::pair<K, V>>& items);

            /**
             * @brief
             @rst
             Constructs from a fully-indexed description: for each key, a list of
             ``(index, value)`` pairs. See
             :cpp:func:`BaseOrderedMultiMap::buildFromIndexed`'s doc comment for the full
             semantics (index treated as a sort key, not a strict required position).
             @endrst
             *
             * @param indexed The key -> list of (index, value) pairs to build from
             */
            explicit OrderedMultiMapSqrt(const std::unordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>& indexed);

            /**
             * @copydoc OrderedMultiMapSqrt(const std::unordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>&)
             *
             @rst
             This overload's outer container iterates ordered by key (ascending) instead of
             unspecified, so cross-key tie-breaking follows that order instead.
             @endrst
             */
            explicit OrderedMultiMapSqrt(const std::map<K, std::vector<std::pair<long long, V>>>& indexed);

            /**
             * @brief
             @rst
             Retrieves the full ordered sequence as a flat vector. Unlike
             :cpp:func:`OrderedMultiMap::entries`, this always copies -- the block structure
             isn't a single container that a reference could be returned into. The map itself
             is also directly iterable (see :cpp:class:`BaseOrderedMultiMap::Iterator`),
             yielding key/value/occurrenceIndex/orderIndex without the copy.
             @endrst
             *
             * @return The full ordered sequence of (key, value) pairs
             */
            std::vector<std::pair<K, V>> entries() const;

            /**
             * @brief
             @rst
             Splits this map into several smaller OrderedMultiMapSqrt instances. See
             :cpp:func:`BaseOrderedMultiMap::computeSplitGroups`'s doc comment for the full
             semantics (index convention, ``includeSplitKVP``, ``includeEmptyParts``,
             ``sortIndices``). Each resulting part is a genuinely independent new instance,
             built via the normal :cpp:func:`insert` path.
             @endrst
             *
             * @param inds The indices at which to split
             * @param includeSplitKVP @copybrief BaseOrderedMultiMap::computeSplitGroups
             * @param includeEmptyParts @copybrief BaseOrderedMultiMap::computeSplitGroups
             * @param sortIndices
             @rst
             If ``true`` (the default), ``inds`` is normalized, deduplicated, and sorted
             ascending first. If you already know ``inds`` iterates in that exact order, pass
             ``false`` to skip that pass -- **this precondition is unchecked**, and violating
             it produces a silently wrong (not crashing) result.
             @endrst
             *
             * @return The resulting parts, left to right
             */
            std::vector<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>> splitByInds(
                const tsl::ordered_set<long long>& inds,
                bool includeSplitKVP = true,
                bool includeEmptyParts = false,
                bool sortIndices = true) const;

            /**
             * @brief
             @rst
             Same as the ``tsl::ordered_set``-keyed overload above, for callers with a plain
             ``std::set`` instead
             @endrst
             *
             * @param inds The indices at which to split
             * @param includeSplitKVP @copybrief BaseOrderedMultiMap::computeSplitGroups
             * @param includeEmptyParts @copybrief BaseOrderedMultiMap::computeSplitGroups
             * @param sortIndices Same meaning/default as the ``tsl::ordered_set``-keyed overload's ``sortIndices``
             *
             * @return The resulting parts, left to right
             */
            std::vector<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>> splitByInds(
                const std::set<long long>& inds,
                bool includeSplitKVP = true,
                bool includeEmptyParts = false,
                bool sortIndices = true) const;

            /**
             * @brief
             @rst
             Same as the ``tsl::ordered_set``-keyed overload above, for callers with a plain
             ``std::unordered_set`` instead. This overload's iteration order is unspecified,
             since ``std::unordered_set``'s own iteration order is unspecified. There is no
             ``sortIndices`` flag here: skipping the sort would depend on knowing that
             iteration order, which isn't available.
             @endrst
             *
             * @param inds The indices at which to split
             * @param includeSplitKVP @copybrief BaseOrderedMultiMap::computeSplitGroups
             * @param includeEmptyParts @copybrief BaseOrderedMultiMap::computeSplitGroups
             *
             * @return The resulting parts, left to right
             */
            std::vector<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>> splitByInds(
                const std::unordered_set<long long>& inds,
                bool includeSplitKVP = true,
                bool includeEmptyParts = false) const;

        private:
            // Turns computeSplitGroups()'s handle groups into actual new
            // OrderedMultiMapSqrt instances -- this is the piece that
            // couldn't live in Base (Derived is still incomplete at the
            // point Base's class template is instantiated, so a method
            // defined directly in Base can never return anything naming
            // Derived).
            std::vector<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>> groupsToParts(std::vector<std::vector<Handle>>&& groups) const;

            // --- primitives required by BaseOrderedMultiMap ------------------------

            size_t rawSize() const;

            Handle rawBegin() const;
            Handle rawEnd() const;

            Handle rawNext(Handle h) const;
            Handle rawPrev(Handle h) const;

            Handle rawAtIndex(size_t idx) const;

            Handle rawInsertBefore(Handle pos, const K& key, const V& value);
            void rawErase(Handle h);

            const K& rawKey(Handle h) const;
            const V& rawValue(Handle h) const;
            void rawSetValue(Handle h, const V& value);
            double rawPos(Handle h) const;
            void rawSetPos(Handle h, double p);
            typename std::list<Handle>::iterator rawSelf(Handle h) const;
            void rawSetSelf(Handle h, typename std::list<Handle>::iterator self);

            // Wipes everything (used by remapKeys(), which rebuilds from
            // scratch), restoring the "always >= 1 block" invariant.
            void rawClear();

            // Physically relinks every currently-existing entry to match
            // `order` exactly (order.size() == rawSize(), a permutation of
            // all current handles). Used by reorder(). Deliberately NOT
            // implemented as splice-one-at-a-time-with-incremental-
            // rebalancing: since reorder() is already O(n) overall
            // regardless of backing structure, a full rebuild into fresh,
            // evenly-sized blocks costs nothing extra in complexity class
            // and sidesteps the whole class of bug that incremental
            // rebalancing produced elsewhere in this codebase (see
            // BaseOrderedMultiMap.tpp's insertAllAtImpl for that story).
            void rawRelinkInOrder(const std::vector<Handle>& order);

            // --- rebalancing ---------------------------------------------------------

            size_t targetBlockSize() const;
            void relinkOwner(Block& blk, BlockIt blkIt);
            void rebalanceAfterInsert(BlockIt blk);
            void rebalanceAfterErase(BlockIt blk);
    };
}

#include "OrderedMultiMapSqrt.tpp"

#endif
