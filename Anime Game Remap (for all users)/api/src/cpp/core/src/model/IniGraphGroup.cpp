#include "AGRemapCore/model/IniGraphGroup.h"

#include <utility>


namespace AGRemapCore {
    IniGraphGroup::IniGraphGroup(GraphMap graphs): graphs_(std::move(graphs)) {}

    const IniGraphGroup::GraphMap& IniGraphGroup::graphs() const {
        return graphs_;
    }

    IniGraphGroup::GraphMap& IniGraphGroup::graphs() {
        return graphs_;
    }

    void IniGraphGroup::addGraph(ModObj modObj, Graph graph) {
        // insert_or_assign rather than emplace: the pure-Python original's "self.graphs[modObj] =
        // graph" overwrites an existing entry, whereas emplace would silently keep the old one.
        graphs_.insert_or_assign(std::move(modObj), std::move(graph));
    }

    bool IniGraphGroup::removeGraph(const ModObj& modObj) {
        auto it = graphs_.find(modObj);
        if (it == graphs_.end()) {
            return false;
        }

        graphs_.erase(it);
        return true;
    }

    IniGraphGroup::Graph* IniGraphGroup::getGraph(const ModObj& modObj) {
        auto it = graphs_.find(modObj);
        if (it == graphs_.end()) {
            return nullptr;
        }

        return &(it.value());
    }

    std::size_t IniGraphGroup::size() const {
        return graphs_.size();
    }

    bool IniGraphGroup::empty() const {
        return graphs_.empty();
    }
}
