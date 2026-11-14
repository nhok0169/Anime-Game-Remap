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

#ifndef AGRemapCore_GraphVarRename_TPP
#define AGRemapCore_GraphVarRename_TPP

#include <memory>
#include <utility>
#include <vector>

#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphVarRename<K, V, KeyHash, KeyEqual>::GraphVarRename(std::map<std::string, std::string> renames, Z3Context* z3):
        renames(std::move(renames)), z3(z3) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GraphVarRename<K, V, KeyHash, KeyEqual>::renameIn(Section& section, const std::map<std::string, std::string>& renames,
                                                           Z3Context* z3) {
        if (renames.empty() && z3 == nullptr) {
            return;
        }

        bool reparsed = false;
        for (auto& part : section.parts()) {
            if (auto* content = dynamic_cast<typename Section::ContentPart*>(part.get())) {
                const std::vector<std::pair<K, V>> entries = content->entries();
                for (std::size_t i = 0; i < entries.size(); ++i) {
                    const std::string key = IniNamingTools::renameVars(entries[i].first, renames);
                    const std::string value = IniNamingTools::renameVars(entries[i].second, renames);
                    if (key != entries[i].first) {
                        content->removeKVPAt(i);
                        content->addKVPAt(static_cast<long long>(i), key, value);
                    } else if (value != entries[i].second) {
                        content->setValByInd(static_cast<long long>(i), value);
                    }
                }
                continue;
            }

            auto* pred = dynamic_cast<IfPredPart*>(part.get());
            if (pred == nullptr) {
                continue;
            }

            const std::string src = IniNamingTools::renameVars(pred->src, renames);
            if (z3 == nullptr) {
                pred->src = src;
                continue;
            }
            if (!pred->query.has_value() && src == pred->src) {
                continue;                   // an `else` / `endif`: nothing to parse
            }

            part = std::make_unique<IfPredPart>(src, pred->type, *z3, nullptr, std::nullopt, pred->id());
            reparsed = true;
        }

        if (reparsed) {
            section.rebuild();
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphVarRename<K, V, KeyHash, KeyEqual>::Graph& GraphVarRename<K, V, KeyHash, KeyEqual>::edit(
            Graph& graph, const ModType* modType, const std::string& modName, const PartFilter& partFilter,
            bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        (void)modType;
        (void)modName;
        (void)partFilter;
        (void)trackKeys;
        (void)keysToTrack;

        for (const auto& entry : graph.sections()) {
            if (entry.second != nullptr) {
                renameIn(*entry.second, renames, z3);
            }
        }
        return graph;
    }
}

#endif
