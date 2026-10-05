// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    Cache<K, V, KeyHash, KeyEqual>::Cache(std::size_t capacity): capacity_(capacity) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::size_t Cache<K, V, KeyHash, KeyEqual>::getCapacity() const {
        return capacity_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool Cache<K, V, KeyHash, KeyEqual>::contains(const K& key) const {
        return index_.find(key) != index_.end();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<V> Cache<K, V, KeyHash, KeyEqual>::get(const K& key) const {
        auto it = index_.find(key);
        if (it == index_.end()) {
            return std::nullopt;
        }
        return it->second->second;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void Cache<K, V, KeyHash, KeyEqual>::put(const K& key, V value) {
        auto it = index_.find(key);
        if (it != index_.end()) {
            it->second->second = std::move(value);
            return;
        }

        order_.emplace_back(key, std::move(value));
        ListIt last = order_.end();
        --last;
        index_.emplace(key, last);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void Cache<K, V, KeyHash, KeyEqual>::clear() {
        order_.clear();
        index_.clear();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::size_t Cache<K, V, KeyHash, KeyEqual>::size() const {
        return index_.size();
    }
}
