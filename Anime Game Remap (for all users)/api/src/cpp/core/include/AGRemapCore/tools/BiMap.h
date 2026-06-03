#ifndef BiMap_H
#define BiMap_H

#include <unordered_map>
#include <stdexcept>
#include <optional>


namespace AGRemapCore {
    template <typename K, typename V, typename KHash = std::hash<K>, typename KEqual = std::equal_to<K>, typename VHash = std::hash<V>, typename VEqual = std::equal_to<V>>
    class BiMap {
        protected:
            std::unordered_map<K, V, KHash, KEqual> forward;
            std::unordered_map<V, K, VHash, VEqual> backward;

        public:
            void clear();
            bool empty();
            void insert(const K& key, const V& val);
            const V& getValue(const K& key) const;
            const K& getKey(const V& val) const;
            size_t size() const;

            const V* findValuePtr(const K& key) const;
            std::optional<V> findValue(const K& key) const;
            const K* findKeyPtr(const V& val) const;
            std::optional<K> findKey(const V& val) const;

            bool containsKey(const K& key) const;
            bool containsValue(const V& val) const;

    };
}

#include "BiMap.tpp"

#endif