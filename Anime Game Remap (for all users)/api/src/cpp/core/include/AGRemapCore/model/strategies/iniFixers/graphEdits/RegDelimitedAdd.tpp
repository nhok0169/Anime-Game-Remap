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

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegDelimitedAdd.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegDelimitedAdd<K, V, KeyHash, KeyEqual>::RegDelimitedAdd(Additions additions, RegMap delimiterRegs,
                                                                 bool pathEndOnlyWhenUndelimited,
                                                                 RegDelimitedAddMode mode,
                                                                 RegMap invalidatorRegs, RegMap coveredRegs):
        additions(std::move(additions)), delimiterRegs(std::move(delimiterRegs)),
        invalidatorRegs(std::move(invalidatorRegs)), coveredRegs(std::move(coveredRegs)),
        pathEndOnlyWhenUndelimited(pathEndOnlyWhenUndelimited), mode(mode) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<long long> RegDelimitedAdd<K, V, KeyHash, KeyEqual>::getDelimiterInds(const ContentPart& part) const {
        std::vector<long long> result;

        for (const auto& [reg, pred] : delimiterRegs) {
            for (const auto& indVal : part.getValsWithInds(reg)) {
                if (!pred || pred(indVal.second)) {
                    result.push_back(indVal.first);
                }
            }
        }

        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool RegDelimitedAdd<K, V, KeyHash, KeyEqual>::isPathEnd(const CallGraphType& callGraph, ContentPart* part) {
        // A part that makes a 'run =' call continues at its exit node once the call returns; a part
        // that makes none continues at itself -- exitNodeOf picks the right one. Whichever it is,
        // "nothing more executes" is exactly "that node has no outgoing edge": buildCallGraph gives
        // a section's last parts an edge back to every caller's exit, so only the end of a section
        // nobody runs is left without one
        const auto& forwardEdges = callGraph.forwardEdges();
        auto it = forwardEdges.find(callGraph.exitNodeOf(part));
        return it == forwardEdges.end() || it->second.empty();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void RegDelimitedAdd<K, V, KeyHash, KeyEqual>::addAdditionsAt(ContentPart& part, long long index) const {
        // Front to back at index, index+1, ...: each insertion only shifts what comes after it, so
        // the list ends up in its own order and nothing already in the part moves relative to it
        long long ind = index;
        for (const auto& [key, val] : additions) {
            part.addKVPAt(ind, key, val);
            ++ind;
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegDelimitedAdd<K, V, KeyHash, KeyEqual>::Graph&
    RegDelimitedAdd<K, V, KeyHash, KeyEqual>::edit(Graph& graph, const ModType* modType,
                                                    const std::string& modName, const PartFilter& partFilter,
                                                    bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        (void)modName;
        (void)trackKeys;
        (void)keysToTrack;

        if (additions.empty()) {
            return graph;
        }

        std::unique_ptr<CallGraphType> callGraph = graph.buildCallGraph();

        // Which sections delimit, directly or through a section they run. Only needed for
        // pathEndOnlyWhenUndelimited, and built BEFORE the loop below starts inserting so it
        // describes the graph as it arrived. A part can end a path having delimited nothing itself
        // while the section it ran delimited before returning, which is the whole reason the callee
        // has to be consulted rather than just this part.
        std::unordered_map<std::string, bool> sectionDelimits;
        std::unordered_map<std::string, std::unordered_set<std::string>> sectionCalls;

        if (pathEndOnlyWhenUndelimited) {
            auto scan = graph.iterByContentPart();
            while (scan.next()) {
                IterData& scanData = scan.value();
                if (!getDelimiterInds(*scanData.part).empty()) {
                    sectionDelimits[scanData.sectionName] = true;
                }

                for (const V& runVal : scanData.part->getVals(graph.runConfig().runKey)) {
                    sectionCalls[scanData.sectionName].insert(graph.runConfig().sectionNameOf(runVal));
                }
            }

            // Transitive closure. One section per mod object, so iterating to a fixed point costs
            // nothing and handles a call cycle without special-casing it.
            bool changed = true;
            while (changed) {
                changed = false;
                for (const auto& callEntry : sectionCalls) {
                    auto delimitsIt = sectionDelimits.find(callEntry.first);
                    if (delimitsIt != sectionDelimits.end() && delimitsIt->second) {
                        continue;
                    }

                    for (const std::string& callee : callEntry.second) {
                        auto calleeIt = sectionDelimits.find(callee);
                        if (calleeIt != sectionDelimits.end() && calleeIt->second) {
                            sectionDelimits[callEntry.first] = true;
                            changed = true;
                            break;
                        }
                    }
                }
            }
        }

        if (mode == RegDelimitedAddMode::PerPath) {
            editPerPath(graph, *callGraph, modType, partFilter);
            return graph;
        }

        if (mode == RegDelimitedAddMode::PerBindingGeneration) {
            editPerGeneration(graph, *callGraph, modType, partFilter);
            return graph;
        }

        // The walk starts from the roots and follows 'run =' calls, so a part nobody can reach is
        // never touched; a part reachable more than one way is edited once
        std::unordered_set<ContentPart*> visited;

        auto walk = graph.iterByContentPart();
        while (walk.next()) {
            IterData& iterData = walk.value();
            ContentPart* part = iterData.part;
            if (!visited.insert(part).second) {
                continue;
            }

            std::vector<long long> insertInds = getDelimiterInds(*part);
            if (isPathEnd(*callGraph, part)) {
                bool addAtEnd = true;

                if (pathEndOnlyWhenUndelimited) {
                    // This part delimits, or something it ran did: the path has already had its
                    // additions placed before those delimiters, and one more after the last of them
                    // would be a surplus call.
                    addAtEnd = insertInds.empty();

                    if (addAtEnd) {
                        auto callsIt = sectionCalls.find(iterData.sectionName);
                        if (callsIt != sectionCalls.end()) {
                            for (const std::string& callee : callsIt->second) {
                                auto calleeIt = sectionDelimits.find(callee);
                                if (calleeIt != sectionDelimits.end() && calleeIt->second) {
                                    addAtEnd = false;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (addAtEnd) {
                    insertInds.push_back(static_cast<long long>(part->size()));
                }
            }
            if (insertInds.empty()) {
                continue;
            }

            std::optional<OrderRanges> allowed;
            if (partFilter) {
                allowed = partFilter(iterData, modType, nullptr);
            }

            // Back to front, so an insertion never shifts the indices of the ones still to come
            for (auto it = insertInds.rbegin(); it != insertInds.rend(); ++it) {
                if (allowed.has_value() && !allowed->has(*it)) {
                    continue;
                }
                addAdditionsAt(*part, *it);
            }
        }

        return graph;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void RegDelimitedAdd<K, V, KeyHash, KeyEqual>::editPerPath(Graph& graph, const CallGraphType& callGraph,
                                                                 const ModType* modType, const PartFilter& partFilter) {
        // ONCE PER PATH, at the frontier of the region nothing has delimited yet.
        //
        // Two must-analyses over the call graph, both to a fixed point so a 'run =' cycle needs no
        // special case, and both optimistic (start true, only ever cleared) which is what makes a
        // cycle converge to "clean"/"covered" rather than to nothing:
        //
        //   clean(n)   -- on EVERY path reaching n, no accepted delimiter has executed yet
        //   covered(n) -- on EVERY path reaching n, the addition has already been placed
        //
        // The candidates are the clean nodes; they form a prefix of the graph, so every path leaves
        // them exactly once, which is what makes "once per path" exact rather than approximate.
        const auto& forwardEdges = callGraph.forwardEdges();
        const auto& backwardEdges = callGraph.backwardEdges();

        // Which parts delimit, and which are reachable at all. iterByContentPart walks from the
        // roots through 'run =' calls, so a part nobody can reach never enters the analysis.
        std::unordered_map<ContentPart*, std::vector<long long>> delimiterInds;
        std::vector<ContentPart*> reachable;
        std::unordered_map<ContentPart*, IterData> iterDataOf;

        auto scan = graph.iterByContentPart();
        while (scan.next()) {
            IterData& scanData = scan.value();
            if (iterDataOf.find(scanData.part) != iterDataOf.end()) {
                continue;
            }

            iterDataOf.emplace(scanData.part, scanData);
            reachable.push_back(scanData.part);
            delimiterInds[scanData.part] = getDelimiterInds(*scanData.part);
        }

        // A node's own "does anything delimit while executing it" -- the entry node carries the
        // part's own delimiters. The exit node is the continuation after a 'run =' returns, and
        // whatever the callee delimited is already accounted for by the callee's own nodes being on
        // the path, so it contributes nothing of its own.
        auto nodeDelimits = [&](const Node& node) {
            if (node.isExit) {
                return false;
            }

            auto it = delimiterInds.find(node.part);
            return it != delimiterInds.end() && !it->second.empty();
        };

        auto nodesOf = [&](ContentPart* part) {
            std::vector<Node> result;
            result.push_back(Node{part, false});

            const Node exit = callGraph.exitNodeOf(part);
            if (exit.isExit) {
                result.push_back(exit);
            }
            return result;
        };

        std::vector<Node> nodes;
        for (ContentPart* part : reachable) {
            for (const Node& node : nodesOf(part)) {
                nodes.push_back(node);
            }
        }

        const std::unordered_set<ContentPart*>& roots = callGraph.rootNodes();
        auto isRoot = [&](const Node& node) {
            return !node.isExit && roots.count(node.part) != 0;
        };

        std::unordered_map<Node, bool, NodeHash> clean;
        for (const Node& node : nodes) {
            clean[node] = true;
        }

        bool changed = true;
        while (changed) {
            changed = false;
            for (const Node& node : nodes) {
                if (!clean[node] || isRoot(node)) {
                    continue;
                }

                auto backIt = backwardEdges.find(node);
                if (backIt == backwardEdges.end() || backIt->second.empty()) {
                    continue;
                }

                for (const Node& pred : backIt->second) {
                    // A predecessor outside this analysis is the return edge of a `run =` whose
                    // target is not in the graph -- an EXTERNAL command list. It delimits nothing
                    // visible, so it cannot be what makes this node dirty; reading it as unknown
                    // and therefore dirty stopped the draw AFTER a TexFx block from taking the
                    // addition at all.
                    auto predIt = clean.find(pred);
                    if (predIt == clean.end()) {
                        continue;
                    }

                    if (!predIt->second || nodeDelimits(pred)) {
                        clean[node] = false;
                        changed = true;
                        break;
                    }
                }
            }
        }

        // Where each clean node WOULD insert, if it is the one that has to. A clean node holding a
        // delimiter inserts before its first; one holding none inserts at its end, and only when it
        // must -- it is a path end, or a successor of it is not clean, so no position further along
        // serves the path that leaves there.
        auto mustInsert = [&](const Node& node) {
            auto cleanIt = clean.find(node);
            if (cleanIt == clean.end() || !cleanIt->second) {
                return false;
            }

            if (nodeDelimits(node)) {
                return true;
            }

            // WHAT COMES NEXT IS BOTH OF A CALLING PART'S EDGES, not one of them.
            //
            // A part holding a `run =` has two: its entry node goes to the CALLEE, and its exit
            // node -- where the call returns -- goes to whatever follows in its own section. Asking
            // only the entry makes a part whose callee is external (TexFx, and every shared list a
            // mod does not define, none of which are in the graph) look like the end of its path;
            // asking only the exit hides the callee, so a fix whose draw lives in a called section
            // took the addition in the CALLER, after the call that already drew.
            std::vector<Node> successors;
            auto appendSuccessors = [&](const Node& from) {
                auto it = forwardEdges.find(from);
                if (it != forwardEdges.end()) {
                    successors.insert(successors.end(), it->second.begin(), it->second.end());
                }
            };

            appendSuccessors(node);
            if (!node.isExit) {
                const Node exit = callGraph.exitNodeOf(node.part);
                if (exit.isExit) {
                    appendSuccessors(exit);
                }
            }

            if (successors.empty()) {
                return true;        // a real path end
            }

            for (const Node& succ : successors) {
                // A successor this analysis has never seen is that external callee again: it
                // delimits nothing visible and it returns, so it is a continuation rather than a
                // way out of the region nothing has delimited yet.
                auto succIt = clean.find(succ);
                if (succIt != clean.end() && !succIt->second) {
                    return true;
                }
            }

            return false;
        };

        // covered: the addition has already landed on every path reaching this node. A node that
        // must insert covers everything downstream of it, which is what stops the header content of
        // a section whose draws are all in independent 'if' blocks from ALSO inserting inside each
        // of them.
        std::unordered_map<Node, bool, NodeHash> covered;
        for (const Node& node : nodes) {
            covered[node] = true;
        }

        changed = true;
        while (changed) {
            changed = false;
            for (const Node& node : nodes) {
                if (!covered[node]) {
                    continue;
                }

                if (isRoot(node)) {
                    covered[node] = false;
                    changed = true;
                    continue;
                }

                auto backIt = backwardEdges.find(node);
                if (backIt == backwardEdges.end() || backIt->second.empty()) {
                    covered[node] = false;
                    changed = true;
                    continue;
                }

                for (const Node& pred : backIt->second) {
                    auto coveredIt = covered.find(pred);
                    const bool predCovered = (coveredIt != covered.end()) && coveredIt->second;
                    if (!predCovered && !mustInsert(pred)) {
                        covered[node] = false;
                        changed = true;
                        break;
                    }
                }
            }
        }

        for (ContentPart* part : reachable) {
            const Node entry{part, false};
            if (covered[entry] || !mustInsert(entry)) {
                continue;
            }

            long long index = static_cast<long long>(part->size());
            auto indsIt = delimiterInds.find(part);
            if (indsIt != delimiterInds.end() && !indsIt->second.empty()) {
                index = indsIt->second.front();
            }

            if (partFilter) {
                auto iterIt = iterDataOf.find(part);
                if (iterIt != iterDataOf.end()) {
                    const OrderRanges allowed = partFilter(iterIt->second, modType, nullptr);
                    if (!allowed.has(index)) {
                        continue;
                    }
                }
            }

            addAdditionsAt(*part, index);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void RegDelimitedAdd<K, V, KeyHash, KeyEqual>::editPerGeneration(Graph& graph, const CallGraphType& callGraph,
                                                                       const ModType* modType,
                                                                       const PartFilter& partFilter) {
        // ONCE PER BINDING GENERATION.
        //
        // A generation opens at an invalidator, is served by the addition (or by a coveredRegs
        // occurence the mod wrote itself) and is consumed by a delimiter. Two of them can sit in one
        // part -- bind, draw, bind, draw -- so unlike editPerPath, whose unit is the NODE, the unit
        // here is a position within a part, and the call graph only carries the state BETWEEN parts.
        //
        // Live state is a single bool: "a generation has opened and nothing has served it yet".
        const auto& forwardEdges = callGraph.forwardEdges();
        const auto& backwardEdges = callGraph.backwardEdges();

        // Every accepted occurence of one RegMap in a part, by insertion order index.
        auto indsOf = [&](const ContentPart& part, const RegMap& regs) {
            std::vector<long long> result;
            for (const auto& regEntry : regs) {
                for (const auto& indVal : part.getValsWithInds(regEntry.first)) {
                    if (!regEntry.second || regEntry.second(indVal.second)) {
                        result.push_back(indVal.first);
                    }
                }
            }

            std::sort(result.begin(), result.end());
            result.erase(std::unique(result.begin(), result.end()), result.end());
            return result;
        };

        std::vector<ContentPart*> reachable;
        std::unordered_map<ContentPart*, IterData> iterDataOf;
        std::unordered_map<ContentPart*, std::vector<long long>> opens;      // invalidators
        std::unordered_map<ContentPart*, std::vector<long long>> serves;     // coveredRegs
        std::unordered_map<ContentPart*, std::vector<long long>> consumes;   // delimiters

        auto scan = graph.iterByContentPart();
        while (scan.next()) {
            IterData& scanData = scan.value();
            if (iterDataOf.find(scanData.part) != iterDataOf.end()) {
                continue;
            }

            iterDataOf.emplace(scanData.part, scanData);
            reachable.push_back(scanData.part);
            opens[scanData.part] = indsOf(*scanData.part, invalidatorRegs);
            serves[scanData.part] = indsOf(*scanData.part, coveredRegs);
            consumes[scanData.part] = getDelimiterInds(*scanData.part);
        }

        // Walking one part from an incoming state: where the addition has to go, and what state
        // leaves. Shared by the analysis and the insertion pass so the two cannot disagree.
        //
        // No invalidator anywhere in the graph means the whole path is one generation, which is
        // PerPath -- entering live is then the right start, and the first delimiter takes the
        // addition.
        auto walkPart = [&](ContentPart* part, bool liveIn, std::vector<long long>* inserts) {
            std::vector<std::pair<long long, int>> events;   // (index, kind): 0 opens, 1 serves, 2 consumes
            for (long long ind : opens[part]) {
                events.emplace_back(ind, 0);
            }
            for (long long ind : serves[part]) {
                events.emplace_back(ind, 1);
            }
            for (long long ind : consumes[part]) {
                events.emplace_back(ind, 2);
            }
            std::sort(events.begin(), events.end());

            bool live = liveIn;
            for (const auto& event : events) {
                if (event.second == 0) {
                    live = true;
                } else if (event.second == 1) {
                    live = false;
                } else if (live) {
                    // as late as possible: immediately before the delimiter that consumes it
                    if (inserts != nullptr) {
                        inserts->push_back(event.first);
                    }
                    live = false;
                }
            }

            return live;
        };

        // The MUST analysis: live on entry to a node only when every path in says so. Optimistic
        // start (everything live, roots live) with a fixpoint, the same shape editPerPath's two
        // analyses use, so a `run =` cycle converges instead of needing a special case.
        std::unordered_map<ContentPart*, bool> liveIn;
        for (ContentPart* part : reachable) {
            liveIn[part] = true;
        }

        // A ROOT ENTERS LIVE, deliberately: with no invalidatorRegs that is what makes the whole
        // path one generation, which is the documented "this mode is PerPath then". It also keeps
        // the two modes agreeing on a path that DRAWS before it binds anything -- PerPath puts its
        // one call before that first draw, and so does this.
        const std::unordered_set<ContentPart*>& roots = callGraph.rootNodes();

        bool changed = true;
        while (changed) {
            changed = false;
            for (ContentPart* part : reachable) {
                if (!liveIn[part] || roots.count(part) != 0) {
                    continue;
                }

                const Node entry{part, false};
                auto backIt = backwardEdges.find(entry);
                if (backIt == backwardEdges.end() || backIt->second.empty()) {
                    continue;
                }

                for (const Node& pred : backIt->second) {
                    // An exit node is the continuation after a `run =`: whatever the callee left
                    // live is already this predecessor's own outgoing state, so both kinds of node
                    // are read the same way -- through the part they belong to.
                    auto predIt = liveIn.find(pred.part);
                    if (predIt == liveIn.end()) {
                        continue;   // an external command list -- see editPerPath's note
                    }

                    if (!walkPart(pred.part, predIt->second, nullptr)) {
                        liveIn[part] = false;
                        changed = true;
                        break;
                    }
                }
            }
        }

        // A generation still live where a path ENDS was never consumed by a delimiter. It still
        // needs its addition -- the draw may be in a section outside this graph -- unless the
        // caller asked for the addition only on an undelimited path.
        for (ContentPart* part : reachable) {
            std::vector<long long> inserts;
            const bool liveOut = walkPart(part, liveIn[part], &inserts);

            if (liveOut && isPathEnd(callGraph, part)
                    && !(pathEndOnlyWhenUndelimited && !consumes[part].empty())) {
                inserts.push_back(static_cast<long long>(part->size()));
            }

            if (inserts.empty()) {
                continue;
            }

            std::optional<OrderRanges> allowed;
            if (partFilter) {
                auto iterIt = iterDataOf.find(part);
                if (iterIt != iterDataOf.end()) {
                    allowed = partFilter(iterIt->second, modType, nullptr);
                }
            }

            // Back to front, so an insertion never shifts the indices of the ones still to come
            for (auto it = inserts.rbegin(); it != inserts.rend(); ++it) {
                if (allowed.has_value() && !allowed->has(*it)) {
                    continue;
                }
                addAdditionsAt(*part, *it);
            }
        }
    }
}
