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

    GraphEditVariables::Sections& GraphEditVariables::sections(const std::string& name) {
        auto found = sections_.find(name);
        if (found == sections_.end()) {
            found = sections_.emplace(name, std::make_unique<Sections>()).first;
        }
        return *found->second;
    }

    const GraphEditVariables::Sections* GraphEditVariables::findSections(const std::string& name) const {
        const auto found = sections_.find(name);
        return found == sections_.end() ? nullptr : found->second.get();
    }

    std::vector<std::string> GraphEditVariables::sectionsNames() const {
        std::vector<std::string> names;
        for (const auto& entry : sections_) {
            names.push_back(entry.first);
        }
        return names;
    }

    void GraphEditVariables::clear() {
        for (auto& entry : sections_) {
            entry.second->clear();
        }
    }
}
