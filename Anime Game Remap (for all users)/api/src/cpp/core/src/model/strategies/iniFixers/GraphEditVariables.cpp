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

#include "AGRemapCore/model/strategies/iniFixers/GraphEditVariables.h"

#include <algorithm>


namespace AGRemapCore {
    void GraphEditVariables::Sections::add(std::unique_ptr<Section> section) {
        if (section != nullptr) {
            places_.emplace_back().push_back(std::move(section));
        }
    }

    GraphEditVariables::Sections::Place GraphEditVariables::Sections::reserve() {
        places_.emplace_back();
        return {places_.size() - 1, cleared_};
    }

    void GraphEditVariables::Sections::addAt(std::unique_ptr<Section> section, const Place& place) {
        if (section == nullptr) {
            return;
        }

        if (place.second != cleared_ || place.first >= places_.size()) {
            add(std::move(section));
            return;
        }

        places_[place.first].push_back(std::move(section));
    }

    std::vector<GraphEditVariables::Section*> GraphEditVariables::Sections::list() const {
        std::vector<Section*> result;
        for (const auto& place : places_) {
            for (const auto& section : place) {
                result.push_back(section.get());
            }
        }
        return result;
    }

    bool GraphEditVariables::Sections::empty() const {
        return std::all_of(places_.begin(), places_.end(), [](const auto& place) { return place.empty(); });
    }

    void GraphEditVariables::Sections::clear() {
        places_.clear();
        ++cleared_;
    }

    bool GraphEditVariables::Value::empty() const {
        return object_ == nullptr;
    }

    std::type_index GraphEditVariables::Value::type() const {
        return type_;
    }

    void GraphEditVariables::set(const std::string& name, Value value) {
        values_.insert_or_assign(name, std::move(value));
    }

    const GraphEditVariables::Value* GraphEditVariables::value(const std::string& name) const {
        const auto found = values_.find(name);
        return found == values_.end() ? nullptr : &found->second;
    }

    bool GraphEditVariables::has(const std::string& name) const {
        return values_.find(name) != values_.end();
    }

    void GraphEditVariables::erase(const std::string& name) {
        values_.erase(name);
    }

    std::vector<std::string> GraphEditVariables::names() const {
        std::vector<std::string> result;
        for (const auto& entry : values_) {
            result.push_back(entry.first);
        }
        return result;
    }

    void GraphEditVariables::apply(const std::vector<std::string>& inputs, const Apply& fn, const std::string& into) {
        if (!fn) {
            return;
        }

        // the inputs' values are copies sharing their objects, so 'fn' may write over any of them
        std::vector<Value> held;
        held.reserve(inputs.size());
        std::vector<const Value*> values;
        for (const std::string& name : inputs) {
            const Value* found = value(name);
            held.push_back(found == nullptr ? Value() : *found);
            values.push_back(found == nullptr ? nullptr : &held.back());
        }

        std::optional<Value> result = fn(values);
        if (result.has_value()) {
            set(into, std::move(*result));
        }
    }

    GraphEditVariables GraphEditVariables::of(const std::vector<std::string>& names, const std::vector<const Value*>& values) {
        GraphEditVariables result;
        for (std::size_t i = 0; i < names.size() && i < values.size(); ++i) {
            if (values[i] != nullptr) {
                result.set(names[i], *values[i]);
            }
        }
        return result;
    }

    GraphEditVariables::Sections& GraphEditVariables::sections(const std::string& name) {
        auto found = values_.find(name);
        if (found == values_.end()) {
            found = values_.emplace(name, Value::make<Sections>()).first;
        }

        Sections* result = found->second.as<Sections>();
        if (result == nullptr) {
            throw std::invalid_argument("the variable '" + name + "' holds a value other than sections");
        }
        return *result;
    }

    const GraphEditVariables::Sections* GraphEditVariables::findSections(const std::string& name) const {
        return find<Sections>(name);
    }

    std::vector<std::string> GraphEditVariables::sectionsNames() const {
        std::vector<std::string> result;
        for (const auto& entry : values_) {
            if (entry.second.as<Sections>() != nullptr) {
                result.push_back(entry.first);
            }
        }
        return result;
    }

    void GraphEditVariables::clear() {
        std::vector<std::string> others;
        for (const auto& entry : values_) {
            if (Sections* held = entry.second.as<Sections>()) {
                held->clear();
            } else {
                others.push_back(entry.first);
            }
        }
        for (const std::string& name : others) {
            values_.erase(name);
        }
    }
}
