#ifndef AGRemapCore_RegBranchAdd_TPP
#define AGRemapCore_RegBranchAdd_TPP

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

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBranchAdd.h"


#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResGroupCollect.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegBranchAdd<K, V, KeyHash, KeyEqual>::RegBranchAdd(BranchOf branchOf): branchOf(std::move(branchOf)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<RegBranchAdd<K, V, KeyHash, KeyEqual>> RegBranchAdd<K, V, KeyHash, KeyEqual>::perBranchReplacements(
            Z3Context* z3Ctx, std::vector<GraphCollectRow<K, V, KeyHash, KeyEqual>> rows, std::string keyPrefix,
            ReplacementsOf replacementsOf) {
        return std::make_unique<RegBranchAdd>(
            [z3Ctx, rows = std::move(rows), keyPrefix = std::move(keyPrefix), replacementsOf = std::move(replacementsOf)](
                    const Z3Predicate& query, const IterData&) {
                Branch result;
                const std::optional<Z3Predicate> local = ResGroupCollect<K, V, KeyHash, KeyEqual>::combineQueries(
                    query, Z3Predicate::trueValue(*z3Ctx), z3Ctx);
                const long long branch = GraphCollect<K, V, KeyHash, KeyEqual>::onlyUnder(rows, local, z3Ctx);
                if (branch < 0) {
                    return result;
                }

                Additions replacements = replacementsOf(static_cast<std::size_t>(branch), local);
                if (replacements.empty()) {
                    return result;
                }

                result.key = keyPrefix + ";" + std::to_string(branch);
                result.replacements = std::move(replacements);
                return result;
            });
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<RegBranchAdd<K, V, KeyHash, KeyEqual>> RegBranchAdd<K, V, KeyHash, KeyEqual>::perBranchRemovals(
            Z3Context* z3Ctx, std::vector<GraphCollectRow<K, V, KeyHash, KeyEqual>> rows, std::string keyPrefix,
            RemovalsOf removalsOf) {
        return std::make_unique<RegBranchAdd>(
            [z3Ctx, rows = std::move(rows), keyPrefix = std::move(keyPrefix), removalsOf = std::move(removalsOf)](
                    const Z3Predicate& query, const IterData&) {
                Branch result;
                const std::optional<Z3Predicate> local = ResGroupCollect<K, V, KeyHash, KeyEqual>::combineQueries(
                    query, Z3Predicate::trueValue(*z3Ctx), z3Ctx);
                const long long branch = GraphCollect<K, V, KeyHash, KeyEqual>::onlyUnder(rows, local, z3Ctx);
                if (branch < 0) {
                    return result;
                }

                std::vector<K> removals = removalsOf(static_cast<std::size_t>(branch), local);
                if (removals.empty()) {
                    return result;
                }

                result.key = keyPrefix + ";" + std::to_string(branch);
                result.removals = std::move(removals);
                return result;
            });
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegBranchAdd<K, V, KeyHash, KeyEqual>::Graph& RegBranchAdd<K, V, KeyHash, KeyEqual>::edit(
        Graph& graph, const ModType* modType, const std::string& modName, const PartFilter& partFilter,
        bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        (void)modName;
        (void)trackKeys;
        (void)keysToTrack;

        if (!branchOf) {
            return graph;
        }

        // The LAST part each branch claims, in the graph's own iteration order, and what goes in it.
        // Iterated first and edited afterwards: adding KVPs to a part while iterating the graph that
        // produced it is the kind of thing that works until a part splits.
        struct Claim {
            ContentPart* part = nullptr;
            Additions additions;
            Additions replacements;
            std::vector<K> removals;
        };

        tsl::ordered_map<std::string, Claim> claims;

        auto parts = graph.iterByQuery();
        while (parts.next()) {
            auto& queryData = parts.value();
            if (queryData.part == nullptr) {
                continue;
            }

            // iterByQuery reports a SectionIterQueryData -- the same thing plus the predicate --
            // and everything downstream of here takes the plain one.
            IterData iterData(queryData.sectionName, queryData.section, queryData.part, queryData.state,
                               queryData.colouring);

            if (partFilter) {
                const auto accepted = partFilter(iterData, modType, nullptr);
                if (accepted.isEmpty()) {
                    continue;
                }
            }

            Branch branch = branchOf(queryData.query, iterData);
            if (branch.key.empty() || (branch.additions.empty() && branch.replacements.empty() && branch.removals.empty())) {
                continue;
            }

            claims[branch.key] = Claim{iterData.part, std::move(branch.additions), std::move(branch.replacements),
                                       std::move(branch.removals)};
        }

        for (const auto& entry : claims) {
            const Claim& claim = entry.second;
            if (claim.part == nullptr) {
                continue;
            }

            for (const K& key : claim.removals) {
                claim.part->removeKey(key);
            }

            if (!claim.replacements.empty()) {
                std::vector<std::pair<K, typename ContentPart::ReplaceSpec>> newVals;
                newVals.reserve(claim.replacements.size());
                for (const auto& kvp : claim.replacements) {
                    newVals.emplace_back(kvp.first, typename ContentPart::ReplaceSpec(kvp.second));
                }

                claim.part->replaceVals(newVals, /*addNew*/ true);
            }

            for (const auto& kvp : claim.additions) {
                claim.part->addKVP(kvp.first, kvp.second);
            }
        }

        return graph;
    }
}

#endif
