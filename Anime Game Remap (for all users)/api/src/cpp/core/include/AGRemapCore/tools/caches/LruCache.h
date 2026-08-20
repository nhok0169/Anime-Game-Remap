#ifndef AGRemapCore_LruCache_H
#define AGRemapCore_LruCache_H

#include <cstddef>
#include <functional>
#include <optional>
#include <utility>

#include "AGRemapCore/tools/caches/Cache.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A generic `LRU cache`_ -- the C++ counterpart to the pure-Python ``LruCache`` class
     (``tools/caches/LRUCache.py``), which extends ``Cache`` the same way this class extends
     :cpp:class:`Cache` :raw-html:`<br />` :raw-html:`<br />`

     Only :cpp:func:`get`/:cpp:func:`put` are overridden here, to add recency-promotion and
     capacity-based eviction on top of :cpp:class:`Cache`'s shared storage --
     :cpp:func:`Cache::contains`/:cpp:func:`Cache::clear`/:cpp:func:`Cache::size`/
     :cpp:func:`Cache::getCapacity` are inherited unchanged, mirroring exactly which methods the
     pure-Python ``LruCache`` does (and doesn't) override on ``Cache``
     @endrst
     *
     * @tparam K The type for a key
     * @tparam V The type for a value
     * @tparam KeyHash The hash function for keys
     * @tparam KeyEqual The equality function for keys
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class LruCache : public Cache<K, V, KeyHash, KeyEqual> {
        public:
            using Cache<K, V, KeyHash, KeyEqual>::DefaultCapacity;

            /**
             * @brief Constructs a new LRU cache
             *
             * @param capacity
             @rst
             The maximum number of entries the cache holds before evicting the
             least-recently-used entry on the next :cpp:func:`put` :raw-html:`<br />` :raw-html:`<br />`

             A capacity of ``0`` disables caching entirely -- :cpp:func:`put` becomes a no-op
             @endrst
             */
            explicit LruCache(std::size_t capacity = DefaultCapacity);

            /**
             * @brief Retrieves the value for 'key', marking it as most-recently-used
             *
             * @param key The key to search for
             *
             * @return The value for 'key', or ``std::nullopt`` if 'key' is not in the cache
             */
            std::optional<V> get(const K& key);

            /**
             * @brief
             @rst
             Inserts/updates 'key' with 'value', marking it as most-recently-used; evicts the
             least-recently-used entry first if the cache is already at capacity -- unlike
             :cpp:func:`Cache::put`, which never evicts anything
             @endrst
             *
             * @param key The key to insert/update
             * @param value The new value for 'key'
             */
            void put(const K& key, V value);
    };
}

#include "LruCache.tpp"

#endif
