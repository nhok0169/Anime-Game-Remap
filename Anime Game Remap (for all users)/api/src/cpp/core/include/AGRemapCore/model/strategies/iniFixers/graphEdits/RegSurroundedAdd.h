#ifndef AGRemapCore_RegSurroundedAdd_H
#define AGRemapCore_RegSurroundedAdd_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"
#include "AGRemapCore/tools/GraphTools.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphEdit`

     Adds a `KVP`_ into some caller/callee graph of :cpp:class:`IniSectionGraph`, at every location
     that is `surrounded` by a particular set of registers: after every register specified at
     \ref beforeRegs has been seen at least once (and accepted by its predicate) and before every
     register specified at \ref afterRegs has been seen at least once (and accepted by its
     predicate) :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        ``run =`` is modeled as `call-with-return`_, not a plain `goto`_ -- a `section`_ can call
        itself (directly, or through a cycle of several `section`_\\s), and this edit is built to
        remain sound for that case via `fixpoint iteration`_ (:cpp:class:`GraphTools`) over
        :cpp:func:`IniSectionGraph::buildCallGraph`, rather than a single forward pass
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegSurroundedAdd: public BaseIniGraphEdit<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseIniGraphEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniGraphEdit::Graph
             */
            using Graph = typename Base::Graph;

            /**
             * @copydoc BaseIniGraphEdit::PartFilter
             */
            using PartFilter = typename Base::PartFilter;

            /**
             * @copydoc BaseIniGraphEdit::IterData
             */
            using IterData = typename Base::IterData;

            /**
             * @copydoc BaseIniGraphEdit::OrderRanges
             */
            using OrderRanges = typename Base::OrderRanges;

            /**
             * @copydoc BaseIniGraphEdit::KeySet
             */
            using KeySet = typename Base::KeySet;

            /**
             * @brief The parts this edit searches/inserts into
             */
            using ContentPart = typename Graph::ContentPart;

            /**
             * @brief The running `KVP`_ state used to evaluate a single :cpp:class:`IfContentPart` in isolation
             */
            using Colouring = typename Graph::Colouring;

            /**
             * @brief The `call graph`_ type this edit builds via :cpp:func:`IniSectionGraph::buildCallGraph`
             */
            using CallGraphType = typename Graph::CallGraphType;

            /**
             * @brief A node of \ref CallGraphType -- either a real #ContentPart, or its "after the call returns" continuation
             */
            using Node = typename CallGraphType::Node;

            /**
             * @brief Hasher for \ref Node
             */
            using NodeHash = typename CallGraphType::NodeHash;

            /**
             * @brief
             @rst
             The predicate for which particular occurence of a register to accept, taking in the
             value of the occurence -- an **empty** function accepts any value (mirrors the
             pure-Python original's ``None`` meaning "any occurence accepted")
             @endrst
             */
            using Predicate = std::function<bool(const V&)>;

            /**
             * @brief
             @rst
             The registers a `surrounded` window is defined by -- the keys are register names, the
             values are their \ref Predicate (empty accepts any value)
             @endrst
             */
            using RegMap = std::unordered_map<K, Predicate, KeyHash, KeyEqual>;

            /**
             * @brief The `KVP`_ to add
             */
            std::pair<K, V> addition;

            /**
             * @brief
             @rst
             The registers that must come before \ref addition (ie. \ref addition gets added after
             these registers) :raw-html:`<br />` :raw-html:`<br />`

             This condition is only satisfied once at least one accepted occurence has been seen
             for **every** key specified here
             @endrst
             */
            RegMap beforeRegs;

            /**
             * @brief
             @rst
             The registers that must come after \ref addition (ie. \ref addition gets added before
             these registers) -- same format/semantics as \ref beforeRegs, except the condition
             applies for coming after \ref addition instead of before it
             @endrst
             */
            RegMap afterRegs;

            /**
             * @brief
             @rst
             Whether to add \ref addition at the latest valid location within the `surrounded`
             window, instead of the earliest one :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             */
            bool latest = false;

            /**
             * @brief Constructs a new `surrounded`-window-adding edit
             *
             * @param addition The `KVP`_ to add
             * @param beforeRegs The registers that must come before 'addition'. **Default**: empty
             * @param afterRegs The registers that must come after 'addition'. **Default**: empty
             * @param latest Whether to add 'addition' at the latest valid location instead of the earliest. **Default**: ``false``
             */
            explicit RegSurroundedAdd(std::pair<K, V> addition = {}, RegMap beforeRegs = {}, RegMap afterRegs = {}, bool latest = false);

            /**
             * @brief
             @rst
             Checks whether every register in 'keys' is defined by at least one #ContentPart
             somewhere in 'graph' :raw-html:`<br />` :raw-html:`<br />`

             Used to rule out an \ref afterRegs register that can provably never be satisfied
             anywhere in the graph -- \ref getValidRangeForPart alone can't tell that case apart
             from "not reached yet, but still will be"
             @endrst
             *
             * @param graph The graph to search
             * @param keys The registers that all need to be found somewhere in 'graph'
             *
             * @return Whether every register in 'keys' was found
             */
            static bool keysExistSomewhere(Graph& graph, const KeySet& keys);

            /**
             * @brief
             @rst
             Fills the parts of 'graph' with a `surrounded` window insertion of \ref addition,
             honouring \ref latest for which valid location within each window is chosen
             @endrst
             *
             * @param graph The graph to edit, modified in place
             * @param modType The type of mod to fix. Unused by this edit -- only forwarded to 'partFilter'
             * @param modName The name of the mod to fix to. Unused by this edit
             * @param partFilter Which order indices may be used within a part -- empty accepts every index. **Default**: empty
             * @param trackKeys Unused by this edit -- it builds its own colourings from \ref beforeRegs/\ref afterRegs. **Default**: ``false``
             * @param keysToTrack Unused by this edit, for the same reason. **Default**: ``std::nullopt``
             *
             * @return The same graph that was passed in, after editing
             */
            Graph& edit(Graph& graph, const ModType* modType,
                         const std::string& modName = "", const PartFilter& partFilter = {},
                         bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt) override;

        private:

            std::unordered_map<K, typename Colouring::Filter, KeyHash, KeyEqual> _beforeFilters;
            std::unordered_map<K, typename Colouring::Filter, KeyHash, KeyEqual> _afterFilters;
            KeySet _trackedKeys;

            static std::unordered_map<K, typename Colouring::Filter, KeyHash, KeyEqual> buildKeyFilters(const RegMap& regs);

            static OrderRanges getSatisfiedRange(Colouring& colouring, const KeySet& regNames,
                                                  const std::unordered_map<K, typename Colouring::Filter, KeyHash, KeyEqual>& filters,
                                                  bool includeKeyDefs);

            long long pickInsertInd(const OrderRanges& validRange, const ContentPart& part) const;

            OrderRanges getForwardValidRangeForPart(const ContentPart& part, const K& runKey, const std::unordered_map<K, bool, KeyHash, KeyEqual>& beforeEntryFacts,
                                                     const std::unordered_map<K, bool, KeyHash, KeyEqual>& beforeReturnFacts) const;

            OrderRanges getBackwardValidRangeForPart(const ContentPart& part, const K& runKey, const std::unordered_map<K, bool, KeyHash, KeyEqual>& afterExitFacts,
                                                      const std::unordered_map<K, bool, KeyHash, KeyEqual>& afterReturnFacts) const;

            OrderRanges getValidRangeForPart(const ContentPart& part, const K& runKey, const std::unordered_map<K, bool, KeyHash, KeyEqual>& beforeEntryFacts,
                                              const std::unordered_map<K, bool, KeyHash, KeyEqual>& beforeReturnFacts,
                                              const std::unordered_map<K, bool, KeyHash, KeyEqual>& afterExitFacts,
                                              const std::unordered_map<K, bool, KeyHash, KeyEqual>& afterReturnFacts) const;

            OrderRanges getClippedValidRangeForPart(const IterData& iterData, const ModType* modType, const K& runKey, const PartFilter& partFilter,
                                                     const std::unordered_map<K, std::unordered_map<Node, bool, NodeHash>, KeyHash, KeyEqual>& beforeEntryFacts,
                                                     const std::unordered_map<K, std::unordered_map<Node, bool, NodeHash>, KeyHash, KeyEqual>& afterExitFacts) const;

            static std::pair<bool, bool> computeLocalForwardFact(const ContentPart& part, const K& reg, const Predicate& pred);

            static bool computeLocalBackwardFact(const ContentPart& part, const K& reg, const Predicate& pred);

            std::pair<std::unordered_map<K, std::unordered_map<Node, bool, NodeHash>, KeyHash, KeyEqual>,
                      std::unordered_map<K, std::unordered_map<Node, bool, NodeHash>, KeyHash, KeyEqual>>
            computeKeyFacts(Graph& graph) const;

            Graph& editEarliest(Graph& graph, const ModType* modType, const PartFilter& partFilter);

            Graph& editLatest(Graph& graph, const ModType* modType, const PartFilter& partFilter);
    };
}

#include "RegSurroundedAdd.tpp"

#endif
