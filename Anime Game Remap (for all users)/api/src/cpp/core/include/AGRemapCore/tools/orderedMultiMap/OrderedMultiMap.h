#ifndef AGRemapCore_OrderedMultiMap_H
#define AGRemapCore_OrderedMultiMap_H

#include <functional>
#include <list>
#include <set>
#include <unordered_set>
#include <utility>

#include <tsl/ordered_set.h>

#include "AGRemapCore/tools/orderedMultiMap/BaseOrderedMultiMap.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class OrderedMultiMap;

    // Internal storage details for OrderedMultiMap -- not part of the
    // public API, kept in a `detail` namespace with plain (non-Doxygen)
    // comments.
    namespace detail {
        // Forward declaration: see BaseOrderedMultiMap's note on why the
        // per-entry storage can't live in the base class; this is the
        // same self-referential-type trick used there, relocated here.
        template <typename K, typename V>
        struct ListEntry;

        template <typename K, typename V>
        using ListOrder = std::list<ListEntry<K, V>>;

        template <typename K, typename V>
        using ListHandle = typename ListOrder<K, V>::iterator;

        template <typename K, typename V>
        struct ListEntry {
            K key;
            V value;
            double pos;
            typename std::list<ListHandle<K, V>>::iterator self;
        };

        // Defined here (not nested in OrderedMultiMap) for the same reason
        // Handle itself is a template parameter of Base rather than a
        // nested `typename Derived::Handle`: any type referenced in a
        // member function's SIGNATURE (not just its body) is resolved
        // eagerly when the class template is instantiated, and Derived is
        // still incomplete at that point since Base is instantiated as
        // part of processing Derived's own definition.
        template <typename K, typename V>
        struct ListHandleHash {
            size_t operator()(ListHandle<K, V> h) const;
        };
    }

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseOrderedMultiMap`

     The "tactical" backing structure: a plain ``std::list`` of entries, giving
     :math:`O(1)` amortized insert/erase given a handle and permanently stable pointers.
     Positional access (:cpp:func:`getByInd`, :cpp:func:`insertAt`, :cpp:func:`removeAt`,
     etc.) walks from whichever end -- front or back -- is closer to the requested index,
     rather than always walking from the front.
     @endrst
     *
     * @tparam K The type of the keys stored in the map
     * @tparam V The type of the values stored in the map
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    class OrderedMultiMap
        : public BaseOrderedMultiMap<OrderedMultiMap<K, V, KeyHash, KeyEqual>, K, V,
                                      detail::ListHandle<K, V>, detail::ListHandleHash<K, V>,
                                      KeyHash, KeyEqual> {
        public:
            using Handle = detail::ListHandle<K, V>;
            using HandleHash = detail::ListHandleHash<K, V>;

        private:
            using Entry = detail::ListEntry<K, V>;
            using Order = detail::ListOrder<K, V>;
            using Base = BaseOrderedMultiMap<OrderedMultiMap<K, V, KeyHash, KeyEqual>, K, V, Handle, HandleHash, KeyHash, KeyEqual>;
            friend Base;

            Order order_;

        public:
            /**
             * @brief Constructs an empty OrderedMultiMap
             */
            OrderedMultiMap() = default;

            /**
             * @brief Move-constructs an OrderedMultiMap
             */
            OrderedMultiMap(OrderedMultiMap&&) = default;

            /**
             * @brief Move-assigns an OrderedMultiMap
             *
             * @return This map, after the assignment
             */
            OrderedMultiMap& operator=(OrderedMultiMap&&) = default;

            /**
             * @brief
             @rst
             Copy-constructs an OrderedMultiMap, rebuilding via :cpp:func:`insert` (which
             correctly wires up every cross-referencing handle fresh) rather than a naive
             member-wise copy, which would leave handles pointing at the original object's
             nodes.
             @endrst
             *
             * @param other The map to copy
             */
            OrderedMultiMap(const OrderedMultiMap& other);

            /**
             * @copydoc OrderedMultiMap(const OrderedMultiMap&)
             *
             * @return This map, after the assignment
             */
            OrderedMultiMap& operator=(const OrderedMultiMap& other);

            /**
             * @brief Constructs from a list of key-value pairs, inserted at the end in the order given
             *
             * @param items The key-value pairs to insert, in order
             */
            explicit OrderedMultiMap(const std::vector<std::pair<K, V>>& items);

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
            explicit OrderedMultiMap(const std::unordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>& indexed);

            /**
             * @copydoc OrderedMultiMap(const std::unordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>&)
             *
             @rst
             This overload's outer container iterates ordered by key (ascending) instead of
             unspecified, so cross-key tie-breaking follows that order instead.
             @endrst
             */
            explicit OrderedMultiMap(const std::map<K, std::vector<std::pair<long long, V>>>& indexed);

            /**
             * @brief
             @rst
             Retrieves read-only access to the full ordered sequence. The map itself is also
             directly iterable (see :cpp:class:`BaseOrderedMultiMap::Iterator`), yielding
             key/value/occurrenceIndex/orderIndex.
             @endrst
             *
             * @return The full ordered sequence of entries
             */
            const Order& entries() const;

            /**
             * @brief
             @rst
             Splits this map into several smaller OrderedMultiMaps. See
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
            std::vector<OrderedMultiMap<K, V, KeyHash, KeyEqual>> splitByInds(
                const tsl::ordered_set<long long>& inds,
                bool includeSplitKVP = true,
                bool includeEmptyParts = false,
                bool sortIndices = true) const;

            /**
             * @copydoc splitByInds(const tsl::ordered_set<long long>&, bool, bool, bool)
             */
            std::vector<OrderedMultiMap<K, V, KeyHash, KeyEqual>> splitByInds(
                const std::set<long long>& inds,
                bool includeSplitKVP = true,
                bool includeEmptyParts = false,
                bool sortIndices = true) const;

            /**
             * @copydoc splitByInds(const tsl::ordered_set<long long>&, bool, bool, bool)
             *
             @rst
             This overload's iteration order is unspecified, since ``std::unordered_set``'s
             own iteration order is unspecified. There is no ``sortIndices`` flag here:
             skipping the sort would depend on knowing that iteration order, which isn't
             available.
             @endrst
             */
            std::vector<OrderedMultiMap<K, V, KeyHash, KeyEqual>> splitByInds(
                const std::unordered_set<long long>& inds,
                bool includeSplitKVP = true,
                bool includeEmptyParts = false) const;

        private:
            // Turns computeSplitGroups()'s handle groups into actual new
            // OrderedMultiMap instances -- this is the piece that couldn't
            // live in Base (Derived is still incomplete at the point Base's
            // class template is instantiated, so a method defined directly
            // in Base can never return anything naming Derived).
            std::vector<OrderedMultiMap<K, V, KeyHash, KeyEqual>> groupsToParts(std::vector<std::vector<Handle>>&& groups) const;

            // --- primitives required by BaseOrderedMultiMap ------------------------

            size_t rawSize() const;
            Handle rawBegin() const;
            Handle rawEnd() const;
            Handle rawNext(Handle h) const;
            Handle rawPrev(Handle h) const;

            // The tactical fix: walk from whichever end is closer.
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

            // Wipes everything (used by remapKeys(), which rebuilds from scratch).
            void rawClear();

            // Physically relinks every currently-existing entry to match
            // `order` exactly (order.size() == rawSize(), a permutation of
            // all current handles). Used by reorder(). For a plain list,
            // splicing each handle to the end in the desired sequence is
            // already O(1) per splice with no rebalancing concerns.
            void rawRelinkInOrder(const std::vector<Handle>& order);
    };
}

#include "OrderedMultiMap.tpp"

#endif
