#include "AGRemapCore/tools/BiMap.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    bool BiMap<K, V, KHash, KEqual, VHash, VEqual>::add(const K& key, const V& val) {
        if (forward.find(key) != forward.end() || backward.find(val) != backward.end()) return false;

        forward[key] = val;
        backward[val] = key;
        return true;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    void BiMap<K, V, KHash, KEqual, VHash, VEqual>::insert(const K& key, const V& val) {
        bool success = add(key, val);
        if (success) return;

        throw std::runtime_error("Duplicate key or value for the BiMap");
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
    template <typename KeyLike>
    const V& BiMap<K, V, KHash, KEqual, VHash, VEqual>::getValue(const KeyLike& key) const {
        auto it = forward.find(key);
        if (it == forward.end()) throw std::out_of_range("Key not found.");
        return it->second;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    template <typename ValueLike>
    const K& BiMap<K, V, KHash, KEqual, VHash, VEqual>::getKey(const ValueLike& val) const {
        auto it = backward.find(val);
        if (it == backward.end()) throw std::out_of_range("Value not found.");
        return it->second;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    size_t BiMap<K, V, KHash, KEqual, VHash, VEqual>::size() const {
        return forward.size();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    typename BiMap<K, V, KHash, KEqual, VHash, VEqual>::iterator BiMap<K, V, KHash, KEqual, VHash, VEqual>::begin() {
        return forward.begin();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    typename BiMap<K, V, KHash, KEqual, VHash, VEqual>::iterator BiMap<K, V, KHash, KEqual, VHash, VEqual>::end() {
        return forward.end();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    typename BiMap<K, V, KHash, KEqual, VHash, VEqual>::const_iterator BiMap<K, V, KHash, KEqual, VHash, VEqual>::begin() const {
        return forward.begin();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    typename BiMap<K, V, KHash, KEqual, VHash, VEqual>::const_iterator BiMap<K, V, KHash, KEqual, VHash, VEqual>::end() const {
        return forward.end();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    typename BiMap<K, V, KHash, KEqual, VHash, VEqual>::const_iterator BiMap<K, V, KHash, KEqual, VHash, VEqual>::cbegin() const {
        return forward.cbegin();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    typename BiMap<K, V, KHash, KEqual, VHash, VEqual>::const_iterator BiMap<K, V, KHash, KEqual, VHash, VEqual>::cend() const {
        return forward.cend();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    template <typename KeyLike>
    const V* BiMap<K, V, KHash, KEqual, VHash, VEqual>::findValuePtr(const KeyLike& key) const {
        auto it = forward.find(key);
        if (it == forward.end()) return nullptr;
        return &(it->second);
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    template <typename KeyLike>
    std::tuple<const K*, const V*> BiMap<K, V, KHash, KEqual, VHash, VEqual>::findKVPPtrByKey(const KeyLike& key) const {
        const V* valPtr = findValuePtr(key);
        if (valPtr == nullptr) {
            return std::make_tuple(nullptr, nullptr);
        }

        const K* keyPtr = findKeyPtr(*valPtr);
        return std::tuple(keyPtr, valPtr);
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    template <typename KeyLike>
    std::optional<V> BiMap<K, V, KHash, KEqual, VHash, VEqual>::findValue(const KeyLike& key) const {
        const V* val = findValuePtr(key);
        if (val == nullptr) return std::nullopt;
        return *val;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    template <typename ValueLike>
    const K* BiMap<K, V, KHash, KEqual, VHash, VEqual>::findKeyPtr(const ValueLike& val) const {
        auto it = backward.find(val);
        if (it == backward.end()) return nullptr;
        return &(it->second);
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    template <typename ValueLike>
    std::tuple<const K*, const V*> BiMap<K, V, KHash, KEqual, VHash, VEqual>::findKVPPtrByVal(const ValueLike& val) const {
        const K* keyPtr = findKeyPtr(val);
        if (keyPtr == nullptr) {
            return std::make_tuple(nullptr, nullptr);
        }

        const V* valPtr = findValuePtr(*keyPtr);
        return std::tuple(keyPtr, valPtr);
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    template <typename ValueLike>
    std::optional<K> BiMap<K, V, KHash, KEqual, VHash, VEqual>::findKey(const ValueLike& val) const {
        const K* key = findKeyPtr(val);
        if (key == nullptr) return std::nullopt;
        return *key;
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    template <typename KeyLike>
    bool BiMap<K, V, KHash, KEqual, VHash, VEqual>::containsKey(const KeyLike& key) const {
        return forward.find(key) != forward.end();
    }

    template <typename K, typename V, typename KHash, typename KEqual, typename VHash, typename VEqual>
    template <typename ValueLike>
    bool BiMap<K, V, KHash, KEqual, VHash, VEqual>::containsValue(const ValueLike& val) const {
        return backward.find(val) != backward.end();
    }
}