#ifndef AGRemapCore_GraphCollect_H
#define AGRemapCore_GraphCollect_H

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

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollectRow.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Collects the `KVPs`_ of some keys from :cpp:class:`IniSectionGraph`\ s, editing nothing :raw-html:`<br />`
     :raw-html:`<br />`

     For each graph named in \ref keys or \ref keyMatches, every :cpp:class:`IfContentPart` the graph reaches
     (through ``run =``, depth first) is searched for each key, and every occurrence is recorded as a \ref Row:
     where it is, its value, and optionally

     * \ref contextKeys: what other keys hold at that point, as the part's
       :cpp:class:`IfContentPartColouring` tracks them along the walk -- a key's occurrences in the part with
       their lines, or else the value it carries in from earlier on the same path (a caller before its
       ``run =``, or an earlier part). A key the part sets replaces the value carried in, and a value set on a
       sibling ``if`` branch is never carried
     * \ref withConditions: the condition of the ``if`` blocks the part sits in

     :raw-html:`<br />`

     The rows stay until the next \ref edit or \ref clear. Run as one of :cpp:member:`GIMIParser::parseSteps` (or
     :cpp:member:`GIMIParser::deferredParseSteps`), a collect with a \ref name files its rows in the parser's
     :cpp:class:`IniParseData` (:cpp:func:`IniParseData::collected`) -- see :cpp:class:`IParseResult`

     .. warning::
        The rows are a snapshot of the graphs as they were when the collect ran, and nothing keeps them up to
        date. An edit run afterwards that renames, copies, moves or removes a section, or changes a value, leaves
        every row it touched describing what is no longer there -- and such a row still reads like a good one.
        Read the rows before any edit that changes the graphs they came from, or run the collect again
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphCollect: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>, public IParseResult {
        public:
            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniGraphGroupEdit::GraphGroups
             */
            using GraphGroups = typename Base::GraphGroups;

            /**
             * @copydoc BaseIniGraphGroupEdit::Graph
             */
            using Graph = typename Base::Graph;

            /**
             * @copydoc BaseIniGraphGroupEdit::GraphId
             */
            using GraphId = typename Base::GraphId;

            /**
             * @copydoc BaseIniGraphGroupEdit::GraphIdHash
             */
            using GraphIdHash = typename Base::GraphIdHash;

            /**
             * @brief The per-part iteration data the predicates are handed
             */
            using IterData = SectionIterData<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ranges of valid `KVP`_ order indices a \ref PartPredicate returns
             */
            using OrderRanges = Ranges<long long>;

            /**
             * @brief A set of keys
             */
            using KeySet = std::unordered_set<K, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             Restricts collection to specific order indices of a :cpp:class:`IfContentPart` -- an empty
             ``std::function`` means the whole part
             @endrst
             */
            using PartPredicate = std::function<OrderRanges(const IterData&)>;

            /**
             * @brief
             @rst
             Decides whether one occurrence is collected -- takes its key, its value and the part it was found
             in. An empty ``std::function`` accepts everything
             @endrst
             */
            using ValPredicate = std::function<bool(const K&, const V&, const IterData&)>;

            /**
             * @brief A test of a key's name -- see \ref keyMatches
             */
            using KeyMatch = std::function<bool(const K&)>;

            /**
             * @brief A map keyed by which graph something applies to
             */
            template <typename T>
            using ByGraph = tsl::ordered_map<GraphId, T, GraphIdHash>;

            /**
             * @brief One collected occurrence of a key
             */
            using Row = GraphCollectRow<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc GraphCollectRow::KeyState
             */
            using KeyState = typename Row::KeyState;

            /**
             * @brief The keys to collect, keyed by which graph to search -- searched in the order given
             */
            ByGraph<std::vector<K>> keys;

            /**
             * @brief
             @rst
             Keys to collect by a test of their NAME: every key of the graph's `KVPs`_ the test accepts is
             collected as if \ref keys named it, in the order the keys first appear in the graph's `sections`_.
             A key \ref keys already names for the graph is searched once. The graphs of \ref keys are searched
             first, then the graphs of this
             @endrst
             */
            ByGraph<KeyMatch> keyMatches;

            /**
             * @brief The keys whose state is recorded with each row, keyed by which graph -- see \ref GraphCollectRow::context
             */
            ByGraph<std::vector<K>> contextKeys;

            /**
             * @brief Which order indices to collect from, keyed by which graph the predicate applies to
             */
            ByGraph<PartPredicate> partPredicates;

            /**
             * @brief Which occurrences to collect, keyed by which graph the predicate applies to
             */
            ByGraph<ValPredicate> valPredicates;

            /**
             * @brief Whether \ref trackKeysGlobal applies to every graph, instead of the per-graph \ref trackKeys entries
             */
            bool trackKeysIsGlobal = true;

            /**
             * @brief The key-tracking flag used for every graph when \ref trackKeysIsGlobal is ``true``
             */
            bool trackKeysGlobal = false;

            /**
             * @brief Whether to track `KVPs`_ while searching each graph, keyed by which graph
             */
            ByGraph<bool> trackKeys;

            /**
             * @brief
             @rst
             Which `KVP`_ keys to track while searching each graph. A missing entry (or ``std::nullopt``) tracks
             every key encountered :raw-html:`<br />` :raw-html:`<br />`

             A graph with \ref contextKeys is always tracked, its context keys included
             @endrst
             */
            ByGraph<std::optional<KeySet>> keysToTrack;

            /**
             * @brief
             @rst
             Whether each row records the condition its part sits in (\ref GraphCollectRow::condition) -- the walk then asks
             :cpp:func:`IniSectionGraph::iterByQuery`, which solves a predicate per part. **Default**: ``false``
             @endrst
             */
            bool withConditions = false;

            /**
             * @brief
             @rst
             Whether every setting keyed by a graph applies in EVERY group that holds that graph, rather than
             only in the group its ``iniIndex`` names -- as :cpp:member:`ResRegCollect::everyGroup`.
             **Default**: ``false``
             @endrst
             */
            bool everyGroup = false;

            /**
             * @brief
             @rst
             The name the rows are filed under in :cpp:class:`IniParseData` when this runs as one of
             :cpp:member:`GIMIParser::parseSteps` -- empty files nothing. **Default**: empty
             @endrst
             */
            std::string name;

            /**
             * @brief Constructs a new collect
             *
             * @param keys The keys to collect, keyed by which graph to search. **Default**: none
             * @param contextKeys The keys whose state is recorded with each row, keyed by graph. **Default**: none
             * @param name The name the rows are filed under -- see \ref name. **Default**: empty
             */
            GraphCollect(ByGraph<std::vector<K>> keys = {}, ByGraph<std::vector<K>> contextKeys = {}, std::string name = "");

            /**
             * @brief The rows the last \ref edit collected, in the order they were found
             */
            const std::vector<Row>& rows() const;

            /**
             * @brief Empties \ref rows
             */
            void clear() override;

            /**
             * @brief
             @rst
             Files \ref rows under \ref name with :cpp:func:`IniParseData::setCollected` -- nothing when \ref name is
             empty, or for an instantiation whose ``K``/``V`` are not ``std::string``
             @endrst
             *
             * @param data The parser's data
             */
            void fileInto(IniParseData& data) const override;

            /**
             * @brief Collects the rows, replacing those of a previous run -- the groups are not changed
             *
             * @param graphGroups The group of graphs to search for each .ini file
             * @param modType The type of mod. Unused
             * @param modName The name of the mod to fix to. Unused. **Default**: ``""``
             *
             * @return The same groups that were passed in
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

            /**
             * @brief
             @rst
             One search: appends to \ref rows every occurrence of 'key' in the graph 'graphId', with the graph's
             settings. \ref edit is these searches in order; a caller that orders its searches differently
             runs them itself
             @endrst
             *
             * @param graphGroups The group of graphs to search
             * @param graphId The graph
             * @param key The key
             */
            void collectKey(GraphGroups& graphGroups, const GraphId& graphId, const K& key);

            /**
             * @brief
             @rst
             The keys of the graph's `KVPs`_ 'match' accepts, in the order they first appear in its `sections`_,
             leaving out the keys in 'searched' -- what \ref keyMatches searches
             @endrst
             *
             * @param graphGroups The group of graphs
             * @param graphId The graph
             * @param match The test of a key's name
             * @param searched The keys to leave out. **Default**: none
             */
            std::vector<K> matchedKeys(GraphGroups& graphGroups, const GraphId& graphId, const KeyMatch& match,
                                       KeySet searched = {}) const;

        private:
            // The settings everyGroup copies per group
            struct GraphSettings {
                ByGraph<std::vector<K>> keys;
                ByGraph<KeyMatch> keyMatches;
                ByGraph<std::vector<K>> contextKeys;
                ByGraph<PartPredicate> partPredicates;
                ByGraph<ValPredicate> valPredicates;
                ByGraph<bool> trackKeys;
                ByGraph<std::optional<KeySet>> keysToTrack;
            };

            GraphSettings takeGraphSettings();
            void putGraphSettings(GraphSettings settings);
            GraphSettings inEveryGroup(const GraphSettings& configured, const GraphGroups& graphGroups) const;

            void collectAll(GraphGroups& graphGroups);
            void addRows(const GraphId& graphId, const K& key, const IterData& iterData, const std::string& rootSection,
                         const std::optional<Z3Predicate>& condition);

            std::vector<Row> rows_;
    };
}

#include "GraphCollect.tpp"

namespace AGRemapCore {
    // note: every translation unit that uses these with <std::string, std::string> would otherwise
    //   instantiate all of their member bodies itself; they are compiled once, in
    //   src/model/strategies/iniFixers/graphGroupEdits/GraphCollectInstantiation.cpp, instead. Any other instantiation still happens implicitly.
    extern template class GraphCollect<std::string, std::string>;
}

#endif
