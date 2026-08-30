#ifndef AGRemapCore_IniGraphGroup_TPP
#define AGRemapCore_IniGraphGroup_TPP

#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const typename IniGraphGroup<K, V, KeyHash, KeyEqual>::GraphMap& IniGraphGroup<K, V, KeyHash, KeyEqual>::graphs() const {
        return graphs_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IniGraphGroup<K, V, KeyHash, KeyEqual>::GraphMap& IniGraphGroup<K, V, KeyHash, KeyEqual>::graphs() {
        return graphs_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<typename IniGraphGroup<K, V, KeyHash, KeyEqual>::ModObj> IniGraphGroup<K, V, KeyHash, KeyEqual>::modObjs() const {
        std::vector<ModObj> result;
        result.reserve(graphs_.size());

        for (const auto& entry : graphs_) {
            result.push_back(entry.first);
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniGraphGroup<K, V, KeyHash, KeyEqual>::addGraph(ModObj modObj, std::unique_ptr<Graph> graph) {
        // insert_or_assign rather than emplace: the pure-Python original's "self.graphs[modObj] =
        // graph" overwrites an existing entry, whereas emplace would silently keep the old one.
        graphs_.insert_or_assign(std::move(modObj), std::move(graph));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<typename IniGraphGroup<K, V, KeyHash, KeyEqual>::Graph> IniGraphGroup<K, V, KeyHash, KeyEqual>::removeGraph(const ModObj& modObj) {
        auto it = graphs_.find(modObj);
        if (it == graphs_.end()) {
            return nullptr;
        }

        // tsl::ordered_map's iterator always yields a const value (see this project's own
        // Architecture notes), so the owned pointer has to be moved out through a real mutable
        // reference (.at) rather than through the iterator itself.
        std::unique_ptr<Graph> result = std::move(graphs_.at(modObj));
        graphs_.erase(it);
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IniGraphGroup<K, V, KeyHash, KeyEqual>::Graph* IniGraphGroup<K, V, KeyHash, KeyEqual>::getGraph(const ModObj& modObj) const {
        auto it = graphs_.find(modObj);
        if (it == graphs_.end()) {
            return nullptr;
        }

        return it->second.get();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::size_t IniGraphGroup<K, V, KeyHash, KeyEqual>::size() const {
        return graphs_.size();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool IniGraphGroup<K, V, KeyHash, KeyEqual>::empty() const {
        return graphs_.empty();
    }
}

#endif
