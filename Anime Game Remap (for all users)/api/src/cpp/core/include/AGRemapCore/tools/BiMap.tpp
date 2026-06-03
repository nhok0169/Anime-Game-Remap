#include "AGRemapCore/tools/BiMap.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    void BiMap<K, V, KHash, KEqual, VHash, VEqual>::insert(const K& key, const V& val) {
        if (forward.find(key) != forward.end() || backward.find(val) != backward.end()) {
            throw std::runtime_error("Duplicate key or value violation.");
        }

        forward[key] = val;
        backward[val] = key;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    void BiMap<K, V, KHash, KEqual, VHash, VEqual>::clear() {
        forward.clear();
        backward.clear();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    bool BiMap<K, V, KHash, KEqual, VHash, VEqual>::empty() {
        return forward.empty() && backward.empty();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    const V& BiMap<K, V, KHash, KEqual, VHash, VEqual>::getValue(const K& key) const {
        auto it = forward.find(key);
        if (it == forward.end()) throw std::out_of_range("Key not found.");
        return it->second;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    const K& BiMap<K, V, KHash, KEqual, VHash, VEqual>::getKey(const V& val) const {
        auto it = backward.find(val);
        if (it == backward.end()) throw std::out_of_range("Value not found.");
        return it->second;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    size_t BiMap<K, V, KHash, KEqual, VHash, VEqual>::size() const {
        return forward.size();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    const V* BiMap<K, V, KHash, KEqual, VHash, VEqual>::findValuePtr(const K& key) const {
        auto it = forward.find(key);
        if (it == forward.end()) return nullptr;
        return &(it->second);
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    std::optional<V> BiMap<K, V, KHash, KEqual, VHash, VEqual>::findValue(const K& key) const {
        const V* val = findValuePtr(key);
        if (val == nullptr) return std::nullopt;
        return *val;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    const K* BiMap<K, V, KHash, KEqual, VHash, VEqual>::findKeyPtr(const V& val) const {
        auto it = backward.find(val);
        if (it == backward.end()) return nullptr;
        return &(it->second);
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    std::optional<K> BiMap<K, V, KHash, KEqual, VHash, VEqual>::findKey(const V& val) const {
        const K* key = findKeyPtr(val);
        if (key == nullptr) return std::nullopt;
        return *key;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    bool BiMap<K, V, KHash, KEqual, VHash, VEqual>::containsKey(const K& key) const {
        return forward.find(key) != forward.end();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    bool BiMap<K, V, KHash, KEqual, VHash, VEqual>::containsValue(const V& val) const {
        return backward.find(val) != backward.end();
    }
}