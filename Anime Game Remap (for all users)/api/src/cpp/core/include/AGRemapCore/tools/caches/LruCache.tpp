namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    LruCache<K, V, KeyHash, KeyEqual>::LruCache(std::size_t capacity): Cache<K, V, KeyHash, KeyEqual>(capacity) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<V> LruCache<K, V, KeyHash, KeyEqual>::get(const K& key) {
        auto it = this->index_.find(key);
        if (it == this->index_.end()) {
            return std::nullopt;
        }

        this->order_.splice(this->order_.begin(), this->order_, it->second);
        return it->second->second;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void LruCache<K, V, KeyHash, KeyEqual>::put(const K& key, V value) {
        if (this->capacity_ == 0) {
            return;
        }

        auto it = this->index_.find(key);
        if (it != this->index_.end()) {
            it->second->second = std::move(value);
            this->order_.splice(this->order_.begin(), this->order_, it->second);
            return;
        }

        if (this->index_.size() >= this->capacity_) {
            this->index_.erase(this->order_.back().first);
            this->order_.pop_back();
        }

        this->order_.emplace_front(key, std::move(value));
        this->index_.emplace(key, this->order_.begin());
    }
}
