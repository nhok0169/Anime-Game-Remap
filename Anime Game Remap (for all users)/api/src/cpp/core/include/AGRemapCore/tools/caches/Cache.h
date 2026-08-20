#ifndef AGRemapCore_Cache_H
#define AGRemapCore_Cache_H

#include <cstddef>
#include <functional>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A generic cache with no eviction policy of its own -- the C++ counterpart to the pure-Python
     ``Cache`` class (``tools/caches/Cache.py``). :cpp:class:`LruCache` extends this class the same
     way the pure-Python ``LruCache`` extends ``Cache`` :raw-html:`<br />` :raw-html:`<br />`

     #capacity is stored but never acted on here -- :cpp:func:`put` never evicts anything, matching
     the pure-Python original exactly (a subclass, e.g. :cpp:class:`LruCache`, is what actually
     enforces it)
     @endrst
     *
     * @tparam K The type for a key
     * @tparam V The type for a value
     * @tparam KeyHash The hash function for keys
     * @tparam KeyEqual The equality function for keys
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class Cache {
        public:

            /**
             * @brief Default capacity used when none is explicitly specified, matching the
             *      pure-Python ``Cache``'s own ``DefaultCacheSize``
             */
            static constexpr std::size_t DefaultCapacity = 128;

            /**
             * @brief Constructs a new, empty cache
             *
             * @param capacity
             @rst
             The capacity of the cache -- stored on :cpp:class:`Cache` itself, but only ever
             enforced by a subclass (see the class-level note)
             @endrst
             */
            explicit Cache(std::size_t capacity = DefaultCapacity);

            /**
             * @brief Getter for the cache's capacity
             */
            std::size_t getCapacity() const;

            /**
             * @brief Checks whether 'key' is in the cache
             *
             * @param key The key to check
             */
            bool contains(const K& key) const;

            /**
             * @brief Attempts to get the value for 'key' from the cache
             *
             * @param key The key to search for
             *
             * @return The value for 'key', or ``std::nullopt`` if 'key' is not in the cache
             */
            std::optional<V> get(const K& key) const;

            /**
             * @brief Inserts/updates 'key' with 'value'. Never evicts anything, regardless of
             *      #getCapacity -- see the class-level note
             *
             * @param key The key to insert/update
             * @param value The new value for 'key'
             */
            void put(const K& key, V value);

            /**
             * @brief Removes all entries from the cache
             */
            void clear();

            /**
             * @brief Retrieves the number of entries currently in the cache
             */
            std::size_t size() const;

        protected:
            using Entry = std::pair<K, V>;
            using EntryList = std::list<Entry>;
            using ListIt = typename EntryList::iterator;

            std::size_t capacity_;

            /**
             * @brief
             @rst
             The cache entries -- in plain insertion order here, but reinterpreted as recency
             order (front = most-recently-used) by :cpp:class:`LruCache`, which reuses this same
             member rather than keeping a separate structure of its own
             @endrst
             */
            EntryList order_;

            /**
             * @brief Index from a key to its node in #order_, for O(1) lookup
             */
            std::unordered_map<K, ListIt, KeyHash, KeyEqual> index_;
    };
}

#include "Cache.tpp"

#endif
