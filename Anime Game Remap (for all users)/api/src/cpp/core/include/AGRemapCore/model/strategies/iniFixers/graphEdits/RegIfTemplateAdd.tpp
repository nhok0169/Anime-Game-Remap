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

#ifndef AGRemapCore_RegIfTemplateAdd_TPP
#define AGRemapCore_RegIfTemplateAdd_TPP

#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegFillMissing.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegIfTemplateAdd<K, V, KeyHash, KeyEqual>::RegIfTemplateAdd(const Section* ifTemplate, Position position, K key):
        ifTemplate(ifTemplate), position(position), key(std::move(key)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegIfTemplateAdd<K, V, KeyHash, KeyEqual>::Graph&
    RegIfTemplateAdd<K, V, KeyHash, KeyEqual>::edit(Graph& graph, const ModType* modType, const std::string& modName,
                                                      const PartFilter& partFilter, bool trackKeys,
                                                      const std::optional<KeySet>& keysToTrack) {
        (void)modName;
        (void)keysToTrack;

        if (ifTemplate == nullptr || ifTemplate->parts().empty()) {
            return graph;
        }

        // the template's `if` lines are re-parsed under the GRAPH's context: predicates of two
        // contexts must never meet in one section (Z3Predicate_MemberOrder_test)
        std::optional<Z3Context> fallbackZ3Ctx;
        Z3Context* z3Ctx = graph.z3Ctx();
        if (z3Ctx == nullptr) {
            fallbackZ3Ctx.emplace();
            z3Ctx = &(*fallbackZ3Ctx);
        }

        for (const std::string& rootName : graph.roots()) {
            Section* section = graph.getSection(rootName, false);
            if (section == nullptr) {
                continue;
            }
            auto& parts = section->parts();

            if (partFilter) {
                ContentPart* firstPart = nullptr;
                for (const auto& part : parts) {
                    firstPart = dynamic_cast<ContentPart*>(part.get());
                    if (firstPart != nullptr) {
                        break;
                    }
                }
                if (firstPart != nullptr) {
                    typename Graph::Colouring colouring;
                    typename Base::IterData iterData(rootName, section, firstPart, 1, trackKeys ? &colouring : nullptr);
                    if (partFilter(iterData, modType, nullptr).isEmpty()) {
                        continue;
                    }
                }
            }

            // where the parts go, and the depth they are shifted to
            std::size_t at = 0;
            int depth = 0;
            if (position == Position::Bottom) {
                const std::optional<std::size_t> guardEnd = RegFillMissing<K, V, KeyHash, KeyEqual>::guardEndIndex(*section);
                at = guardEnd.has_value() ? *guardEnd : parts.size();
                depth = guardEnd.has_value() ? 1 : 0;
            } else if (position == Position::AfterKey) {
                std::optional<std::size_t> partInd, kvpInd;
                for (std::size_t i = 0; i < parts.size() && !partInd.has_value(); ++i) {
                    auto* part = dynamic_cast<ContentPart*>(parts[i].get());
                    if (part == nullptr) {
                        continue;
                    }
                    const std::size_t len = part->size();
                    for (std::size_t k = 0; k < len; ++k) {
                        if (KeyEqual()(part->getByInd(static_cast<long long>(k)).first, key)) {
                            partInd = i;
                            kvpInd = k;
                            break;
                        }
                    }
                }
                if (!partInd.has_value()) {
                    continue;
                }

                // the part is cut after the key: its head stays, its tail follows the template's parts
                auto* part = dynamic_cast<ContentPart*>(parts[*partInd].get());
                depth = part->depth();
                const std::vector<std::pair<K, V>> entries = part->entries();
                std::vector<std::pair<K, V>> head(entries.begin(), entries.begin() + static_cast<std::ptrdiff_t>(*kvpInd + 1));
                std::vector<std::pair<K, V>> tail(entries.begin() + static_cast<std::ptrdiff_t>(*kvpInd + 1), entries.end());
                parts[*partInd] = std::make_unique<ContentPart>(head, depth);
                at = *partInd + 1;
                if (!tail.empty()) {
                    parts.insert(parts.begin() + static_cast<std::ptrdiff_t>(at), std::make_unique<ContentPart>(tail, depth));
                }
            }

            std::vector<std::unique_ptr<IfTemplatePart>> block;
            for (const auto& part : ifTemplate->parts()) {
                if (const auto* content = dynamic_cast<const ContentPart*>(part.get())) {
                    std::unique_ptr<ContentPart> copy = content->clone(true);
                    copy->setDepth(content->depth() + depth);
                    block.push_back(std::move(copy));
                } else if (const auto* pred = dynamic_cast<const IfPredPart*>(part.get())) {
                    block.push_back(std::make_unique<IfPredPart>(pred->src, pred->type, *z3Ctx));
                }
            }
            parts.insert(parts.begin() + static_cast<std::ptrdiff_t>(at),
                         std::make_move_iterator(block.begin()), std::make_move_iterator(block.end()));
            section->rebuild();
        }

        return graph;
    }
}

#endif
