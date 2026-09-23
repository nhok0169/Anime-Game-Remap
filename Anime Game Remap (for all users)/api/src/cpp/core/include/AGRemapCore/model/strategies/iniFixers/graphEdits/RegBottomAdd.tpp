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

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBottomAdd.h"

#include <memory>
#include <optional>

#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/tools/z3/Z3Context.h"

#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegBottomAdd<K, V, KeyHash, KeyEqual>::RegBottomAdd(Additions additions, std::string condition):
        additions(std::move(additions)), condition(std::move(condition)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegBottomAdd<K, V, KeyHash, KeyEqual>::Graph&
    RegBottomAdd<K, V, KeyHash, KeyEqual>::edit(Graph& graph, const ModType* modType, const std::string& modName,
                                                  const PartFilter& partFilter, bool trackKeys,
                                                  const std::optional<KeySet>& keysToTrack) {
        (void)modName;
        (void)trackKeys;
        (void)keysToTrack;

        if (additions.empty()) {
            return graph;
        }

        for (const std::string& rootName : graph.roots()) {
            Section* section = graph.getSection(rootName, false);
            if (section == nullptr) {
                continue;
            }

            // The filter is asked about the root's CURRENT last content part -- the one a new part
            // is appended after -- because that is the only existing position the caller can
            // reasonably discriminate on. Same choice, and the same reason, as
            // RegFillMissing::addBottomCover's.
            if (partFilter) {
                ContentPart* lastPart = nullptr;
                for (const auto& part : section->parts()) {
                    auto* contentPart = dynamic_cast<ContentPart*>(part.get());
                    if (contentPart != nullptr) {
                        lastPart = contentPart;
                    }
                }

                if (lastPart != nullptr) {
                    typename Graph::Colouring colouring;
                    IterData iterData(rootName, section, lastPart, 1, trackKeys ? &colouring : nullptr);

                    const OrderRanges accepted = partFilter(iterData, modType, nullptr);
                    if (accepted.isEmpty()) {
                        continue;
                    }
                }
            }

            // A CONDITION: a new `if` ... `endif` block of its own, appended at the section's depth
            // and built the way ResGroupCollect builds its `if 1` blocks -- parts, then rebuild().
            if (!condition.empty()) {
                std::optional<Z3Context> fallbackZ3Ctx;
                Z3Context* z3Ctx = graph.z3Ctx();
                if (z3Ctx == nullptr) {
                    fallbackZ3Ctx.emplace();
                    z3Ctx = &(*fallbackZ3Ctx);
                }

                auto& parts = section->parts();
                parts.push_back(std::make_unique<IfPredPart>(
                    IfPredPartTypeTools::getName(IfPredPartType::If) + " " + condition, IfPredPartType::If, *z3Ctx));
                parts.push_back(std::make_unique<ContentPart>(additions, 1));
                parts.push_back(std::make_unique<IfPredPart>(
                    IfPredPartTypeTools::getName(IfPredPartType::EndIf), IfPredPartType::EndIf, *z3Ctx));
                section->rebuild();
                continue;
            }

            // A FRESH part at the section's own depth, which is what makes this unconditional: the
            // section's last EXISTING part may well be inside an `if` block.
            ContentPart* bottomPart = section->addBottomContentPart();
            if (bottomPart == nullptr) {
                continue;
            }

            for (const auto& kvp : additions) {
                bottomPart->addKVP(kvp.first, kvp.second);
            }
        }

        return graph;
    }
}
