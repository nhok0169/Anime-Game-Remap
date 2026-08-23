#ifndef AGRemapCore_IniSectionGraph_H
#define AGRemapCore_IniSectionGraph_H

#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "AGRemapCore/model/CallGraph.h"
#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfContentPartColour.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/tools/Generator.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class for constructing a directed subgraph for how the `sections`_ in the .ini file are ran --
     the C++ port of ``IniSectionGraph.py`` :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The nodes are the `sections`_ of the .ini file; the directed edges are the command calls
        from the `sections`_ (a ``run =`` `KVP`_), source -> callee.

     .. note::
        Unlike this class's `IfTemplate`/`IfTemplateNode`/`IfTemplateTree` dependencies (which
        always own everything reachable from them), this class's #sections are, by default,
        **borrowed, non-owning references** -- matching the pure-Python original's own
        ``self.sections = sections`` (a plain reference to whatever dict was passed in, shared with
        the caller, eg. an ``IniFile``'s own ``sectionIfTemplates``). Only when constructed with
        ``copySections = true`` does this graph deep-copy and genuinely own its own sections (see
        #ownedSections_).
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyHash``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyEqual``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IniSectionGraph {
        public:
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;
            using NodeType = typename Section::NodeType;
            using Colouring = IfContentPartColouring<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;
            // What one updateColouring() call returns/restore() consumes -- every key that
            // changed, mapped to its previous state. Not a single Colouring::Change (that's one
            // key's own change record, the *value* type inside this map).
            using ColourChangeSet = std::unordered_map<K, typename Colouring::Change, KeyHash, KeyEqual>;
            using IterData = SectionIterData<K, V, KeyHash, KeyEqual>;
            using IterQueryData = SectionIterQueryData<K, V, KeyHash, KeyEqual>;
            using CallGraphType = CallGraph<K, V, KeyHash, KeyEqual>;

            /**
             * @brief Constructs a new graph
             *
             * @param sections All the `sections`_ of the .ini file, borrowed (not owned) unless ``copySections`` is ``true``
             * @param targetSectionNames Names of the desired `sections`_ we want our subgraph to have
             * @param runConfig The domain customization points this instance uses (see :cpp:class:`IfTemplateRunConfig`)
             * @param build Whether to build the graph. **Default**: ``true``
             * @param copySections Whether to deep-copy the referenced `sections`_. **Default**: ``false``
             * @param z3Ctx The `Z3`_ context every :cpp:class:`Z3Predicate` produced by this graph belongs to. **Default**: ``nullptr``
             */
            explicit IniSectionGraph(std::unordered_map<std::string, Section*> sections, std::vector<std::string> targetSectionNames,
                                      IfTemplateRunConfig<K, V> runConfig, bool build = true, bool copySections = false, Z3Context* z3Ctx = nullptr);

            IniSectionGraph(const IniSectionGraph&) = delete;
            IniSectionGraph& operator=(const IniSectionGraph&) = delete;
            IniSectionGraph(IniSectionGraph&&) = default;
            IniSectionGraph& operator=(IniSectionGraph&&) = default;

            /**
             * @brief All the `sections`_ of the constructed subgraph
             */
            const std::unordered_map<std::string, Section*>& sections() const;

            /**
             * @brief The out-neighbours of the subgraph (the adjacency list)
             */
            const std::unordered_map<std::string, std::vector<std::string>>& neighbours() const;

            /**
             * @brief The root nodes of the subgraph
             */
            const std::vector<std::string>& roots() const;

            /**
             * @brief Names of the desired `sections`_ we want our subgraph to have
             */
            const std::vector<std::string>& targetSectionNames() const;

            /**
             * @copydoc targetSectionNames() const
             */
            void setTargetSectionNames(std::vector<std::string> newTargetSections);

            /**
             * @brief The domain customization points this instance uses
             */
            const IfTemplateRunConfig<K, V>& runConfig() const;

            /**
             * @brief The `Z3`_ context this graph's queries belong to, if any
             */
            Z3Context* z3Ctx() const;

            /**
             * @brief Combines this graph with other graphs
             */
            void combine(const std::vector<IniSectionGraph<K, V, KeyHash, KeyEqual>*>& newGraphs);

            /**
             * @brief
             @rst
             Constructs the subgraph for the `sections`_ using `DFS`_
             @endrst
             *
             * @param sections If provided, replaces #sections() first. **Default**: ``std::nullopt``
             * @param targetSectionNames If provided, replaces #targetSectionNames() first. **Default**: ``std::nullopt``
             * @param copySections Whether to deep-copy the `sections`_ traversed. **Default**: ``false``
             */
            void build(std::optional<std::unordered_map<std::string, Section*>> sections = std::nullopt,
                       std::optional<std::vector<std::string>> targetSectionNames = std::nullopt, bool copySections = false);

            /**
             * @brief Retrieves the :cpp:class:`IfTemplate` for a certain `section`_
             *
             * @param sectionName The name of the `section`_
             * @param raiseException Whether to throw when not found. **Default**: ``true``
             * @throw std::out_of_range If not found and 'raiseException' is ``true``
             * @return The corresponding section, or ``nullptr`` if not found and 'raiseException' is ``false``
             */
            Section* getSection(const std::string& sectionName, bool raiseException = true) const;

            /**
             * @brief Retrieves the `sections`_ corresponding to the roots of the graph
             */
            std::vector<Section*> getRootSections() const;

            /**
             * @brief Determines whether the graph is empty
             */
            bool isEmpty() const;

            /**
             * @brief Retrieves the names of the out-neighbour `sections`_
             */
            std::vector<std::string> getNeighbourNames(const std::string& sectionName) const;

            /**
             * @brief
             @rst
             Retrieves the out-neighbours of some `section`_ :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The pure-Python original's own ``getNeighbours`` has an empty body (a documented,
                never-implemented stub -- confirmed via grep that nothing anywhere calls it), so
                there's no existing behavior to preserve here; this implements the contract its own
                docstring already describes rather than porting "always returns ``None``" forward.
             @endrst
             */
            std::unordered_map<std::string, Section*> getNeighbours(const std::string& sectionName) const;

            /**
             * @brief Retrieves the children `sections`_ of the given `sections`_, read in a `DFS`_ manner
             *
             * @param targetSections The names of the `sections`_ to retrieve the children of
             * @param getNeighbourChildren Whether to also retrieve the children for the neighbours. **Default**: ``true``
             * @return The names of the children `sections`_, per given `section`_ -- preserves first-seen order (matching the `OrderedSet`_ the pure-Python original uses)
             */
            std::unordered_map<std::string, std::vector<std::string>> getChildren(const std::vector<std::string>& targetSections, bool getNeighbourChildren = true) const;

            /**
             * @brief Renames the `sections`_ and reconstructs the graph
             *
             * @param renameFunc Function to rename a `section`_, given its old name
             */
            void rename(const std::function<std::string(const std::string&)>& renameFunc);

            /**
             * @brief Regenerates the ids of the parts
             *
             * @param minimal Only refresh `sections`_ that are part of the graph. **Default**: ``true``
             */
            void refreshPartIds(bool minimal = true);

            /**
             * @brief
             @rst
             Performs a deep copy on the object. Deliberately does **not** deep-copy #z3Ctx_ -- the
             copy keeps a reference to the exact same :cpp:class:`Z3Context`, matching the
             pure-Python original's own custom ``__deepcopy__`` (see that class's own note on why:
             every predicate reachable from a deep copy still needs to be combinable with the
             original's).
             @endrst
             *
             * @param minimal Only copy `sections`_ that are part of the graph. **Default**: ``true``
             * @param newPartIds Whether to refresh the ids for the parts. **Default**: ``true``
             */
            std::unique_ptr<IniSectionGraph<K, V, KeyHash, KeyEqual>> deepcopy(bool minimal = true, bool newPartIds = true) const;

            /**
             * @brief
             @rst
             Determines whether a key fully covers all the conditional branches of a `section`_, for
             every `section`_ in #roots()
             @endrst
             */
            std::unordered_map<std::string, bool> isKeyFullyCover(const K& key) const;

            /**
             * @brief Convenience over #isKeyFullyCover, filtered to just #roots()
             */
            std::unordered_map<std::string, bool> rootsAreFullyCovered(const K& key) const;

            /**
             * @brief Retrieves the parts in the `sections`_ that are not covered by 'key'
             */
            std::unordered_map<std::string, std::set<ContentPart*>> getKeyMissingParts(const K& key) const;

            /**
             * @brief
             @rst
             Computes, for every #ContentPart in 'parts' (a `section`_'s flat, textually-ordered
             list), the pointer of every #ContentPart that must run immediately before it on some
             path through this `section`_ alone (ie. ignoring any ``run =`` call)
             @endrst
             */
            static std::unordered_map<ContentPart*, std::vector<ContentPart*>> computeSectionPredecessors(const std::vector<std::unique_ptr<IfTemplatePart>>& parts);

            /**
             * @brief
             @rst
             Builds a graph-wide version of #computeSectionPredecessors, additionally linking a
             ``run =`` call's own part as a predecessor of whatever `section`_ it calls into
             @endrst
             */
            std::unordered_map<ContentPart*, std::vector<ContentPart*>> buildPartPredecessorGraph() const;

            /**
             * @brief
             @rst
             Builds a `call graph`_ over the #ContentPart\\s of this graph, modeling ``run =`` as a
             `call-with-return`_
             @endrst
             */
            std::unique_ptr<CallGraphType> buildCallGraph() const;

            /**
             * @brief Normalizes the branching structure of all `sections`_ in #sections()
             */
            void normalize();

            /**
             * @brief Retrieves the common mods to fix to, based off every :cpp:class:`IfTemplate` in the graph
             *
             @rst
             .. note::
                Deliberately not ported -- see this port's own migration notes: the pure-Python
                original (``IfTemplate.getMods``/``IniSectionGraph.getCommonMods``) references
                attributes (``self.hashes``/``self.indices``) that are never actually set anywhere
                in the live codebase, so calling either always raises ``AttributeError`` today, and
                the only real (non-deprecated) caller of ``getCommonMods`` no longer exists. Omitted
                by explicit user decision rather than guessed at.
             @endrst
             */
            // std::unordered_set<std::string> getCommonMods(...) const;  -- intentionally omitted, see above

            /**
             * @brief An iterator that iterates through all #ContentPart of the given `sections`_ using `DFS`_
             */
            static Generator<IterData> iterSectsByContentPart(const std::unordered_map<std::string, Section*>& sections, const std::vector<std::string>& roots,
                                                                const IfTemplateRunConfig<K, V>& runConfig, int states = 1, bool colour = false,
                                                                std::optional<std::unordered_set<K, KeyHash, KeyEqual>> colourKeys = std::nullopt);

            /**
             * @brief An iterator that iterates through all #ContentPart of the `sections`_ of this graph using `DFS`_
             */
            Generator<IterData> iterByContentPart(int states = 1, bool colour = false,
                                                   std::optional<std::unordered_set<K, KeyHash, KeyEqual>> colourKeys = std::nullopt) const;

            /**
             * @brief
             @rst
             Iterates through all the `sections`_ of the graph using `DFS`_, yielding ``(sectionName, section)`` pairs
             @endrst
             */
            Generator<std::pair<std::string, Section*>> iterSections() const;

            /**
             * @brief
             @rst
             An iterator that iterates through all the #ContentPart\\s of the graph and also
             retrieves the conditional logical predicate that each #ContentPart resides in
             @endrst
             */
            Generator<IterQueryData> iterByQuery(std::vector<Z3Predicate> queryPath = {}, bool simplify = false, int states = 1,
                                                  bool colour = false, std::optional<std::unordered_set<K, KeyHash, KeyEqual>> colourKeys = std::nullopt) const;

            /**
             * @brief Converts all `sections`_ to a string, using a caller-supplied per-part renderer (see :cpp:class:`IfTemplate`'s own note on why this needs one)
             */
            std::string toStr(const std::function<std::string(Section&, const std::string&, bool)>& sectionToStr, bool autoindent = true) const;

        private:
            std::unordered_map<std::string, Section*> sections_;
            std::vector<std::unique_ptr<Section>> ownedSections_;  // populated only when copySections is used
            std::unordered_map<std::string, std::vector<std::string>> neighbours_;
            std::vector<std::string> roots_;
            std::vector<std::string> targetSectionNames_;
            IfTemplateRunConfig<K, V> runConfig_;
            Z3Context* z3Ctx_;
            mutable std::optional<Z3Predicate> trueQueryCache_;

            void setTargetSectionNamesImpl(std::vector<std::string> newTargetSections);
            static std::unordered_map<std::string, Section*> deepCopySections(const std::unordered_map<std::string, Section*>& src, std::vector<std::unique_ptr<Section>>& storage);

            Z3Predicate trueQuery() const;
            Z3Predicate getQuery(const std::vector<Z3Predicate>& queryPath, bool simplify) const;
    };

}

#include "IniSectionGraph.tpp"

#endif
