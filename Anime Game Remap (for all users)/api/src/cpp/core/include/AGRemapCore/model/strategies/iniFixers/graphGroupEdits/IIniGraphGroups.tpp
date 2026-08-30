#ifndef AGRemapCore_IIniGraphGroups_TPP
#define AGRemapCore_IIniGraphGroups_TPP

#include <algorithm>
#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::IniGraphGroupsVec(std::vector<Group>& groups, IfTemplateRunConfig<K, V> runConfig):
        groups_(groups), runConfig_(std::move(runConfig)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::size_t IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::size() const {
        return groups_.size();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::insertGroup(std::size_t groupInd) {
        std::size_t at = std::min(groupInd, groups_.size());
        groups_.insert(groups_.begin() + static_cast<std::ptrdiff_t>(at), Group());
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::removeGroup(std::size_t groupInd) {
        if (groupInd >= groups_.size()) {
            return;
        }

        // Whatever the group still holds is about to be destroyed with it, so hand any surviving
        // graphs over to owned_ first -- a caller can legitimately still be holding a borrowed
        // pointer into a group it is in the middle of dissolving (GraphGroupRemap does exactly
        // this), and IIniGraphGroups promises every pointer stays valid for this object's lifetime.
        Group& group = groups_[groupInd];
        for (const auto& modObj : group.modObjs()) {
            owned_.push_back(group.removeGraph(modObj));
        }

        groups_.erase(groups_.begin() + static_cast<std::ptrdiff_t>(groupInd));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<typename IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::ModObj> IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::modObjs(std::size_t groupInd) const {
        if (groupInd >= groups_.size()) {
            return {};
        }

        return groups_[groupInd].modObjs();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::size_t IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::graphCount(std::size_t groupInd) const {
        if (groupInd >= groups_.size()) {
            return 0;
        }

        return groups_[groupInd].size();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::Graph* IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::getGraph(std::size_t groupInd, const ModObj& modObj) const {
        if (groupInd >= groups_.size()) {
            return nullptr;
        }

        return groups_[groupInd].getGraph(modObj);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::addGraph(std::size_t groupInd, const ModObj& modObj, Graph* graph) {
        if (groupInd >= groups_.size() || graph == nullptr) {
            return;
        }

        // Every graph reaching here is already owned by this view (IIniGraphGroups's ownership
        // contract), so "adding" it is really *moving* its unique_ptr out of wherever it currently
        // lives -- owned_ (a fresh deepcopyGraph/createGraph result, or a previously removed graph)
        // or some other group. The scan is over a handful of groups holding a handful of graphs
        // each; there is no index worth maintaining for it.
        std::unique_ptr<Graph> taken;

        for (std::size_t i = 0; i < owned_.size(); ++i) {
            if (owned_[i].get() != graph) {
                continue;
            }

            taken = std::move(owned_[i]);
            owned_.erase(owned_.begin() + static_cast<std::ptrdiff_t>(i));
            break;
        }

        if (taken == nullptr) {
            for (Group& group : groups_) {
                bool found = false;

                for (const auto& currentModObj : group.modObjs()) {
                    if (group.getGraph(currentModObj) != graph) {
                        continue;
                    }

                    taken = group.removeGraph(currentModObj);
                    found = true;
                    break;
                }

                if (found) {
                    break;
                }
            }
        }

        if (taken == nullptr) {
            return;
        }

        groups_[groupInd].addGraph(modObj, std::move(taken));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::Graph* IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::removeGraph(std::size_t groupInd, const ModObj& modObj) {
        if (groupInd >= groups_.size()) {
            return nullptr;
        }

        std::unique_ptr<Graph> removed = groups_[groupInd].removeGraph(modObj);
        if (removed == nullptr) {
            return nullptr;
        }

        Graph* result = removed.get();
        owned_.push_back(std::move(removed));
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::Graph* IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::deepcopyGraph(const Graph& src, bool minimal, bool newPartIds) {
        std::unique_ptr<Graph> result = src.deepcopy(minimal, newPartIds);
        Graph* raw = result.get();
        owned_.push_back(std::move(result));
        return raw;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::Graph* IniGraphGroupsVec<K, V, KeyHash, KeyEqual>::createGraph(std::unordered_map<std::string, Section*> sections,
                                                                                                                        std::vector<std::string> targetSectionNames,
                                                                                                                        bool copySections, Z3Context* z3Ctx) {
        auto result = std::make_unique<Graph>(std::move(sections), std::move(targetSectionNames), runConfig_, true, copySections, z3Ctx);
        Graph* raw = result.get();
        owned_.push_back(std::move(result));
        return raw;
    }
}

#endif
