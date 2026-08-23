#ifndef AGRemapCore_IfTemplate_H
#define AGRemapCore_IfTemplate_H

#include <cstddef>
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

#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplateNode.h"
#include "AGRemapCore/model/iftemplate/IfTemplatePart.h"
#include "AGRemapCore/model/iftemplate/IfTemplateTree.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Domain-specific customization points shared by :cpp:class:`IfTemplate` and
     :cpp:class:`IniSectionGraph` -- kept generic/`Python`_-free the same way the rest of these
     classes are, by never hardcoding the ``.ini`` domain's ``run`` keyword or assuming ``V``
     converts to :cpp:type:`std::string` implicitly (it doesn't for ``V = py::object``, which the
     `pybind11`_ binding layer instantiates these classes with). The `Python`_ binding always
     constructs one of these with ``runKey = py::str("run")`` and ``sectionNameOf`` casting a
     ``py::object`` to :cpp:type:`std::string`.
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     */
    template <typename K, typename V>
    struct IfTemplateRunConfig {
        /**
         * @brief The `.ini`_ ``run`` keyword, as a ``K`` (eg. ``py::str("run")`` for the `Python`_ binding)
         */
        K runKey;

        /**
         * @brief
         @rst
         Converts a ``run =`` `KVP`_'s value (type ``V``) into the `section`_ name it references
         @endrst
         */
        std::function<std::string(const V&)> sectionNameOf;

        /**
         * @brief The inverse of #sectionNameOf -- builds a ``run =`` `KVP`_ value from a `section`_ name
         */
        std::function<V(const std::string&)> valOfSectionName;
    };

    /**
     * @brief
     @rst
     Data for storing information about a `section`_ in a ``.ini`` file -- the C++ port of
     ``IfTemplate.py`` :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This class owns every :cpp:class:`IfTemplatePart` reachable from #parts, and owns its own
        #tree (whose nodes hold non-owning references back into #parts -- see
        :cpp:class:`IfTemplateNode`'s own top-level note).
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyHash``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyEqual``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IfTemplate {
        public:
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;
            using NodeType = IfTemplateNode<K, V, KeyHash, KeyEqual>;
            using TreeType = IfTemplateTree<K, V, KeyHash, KeyEqual>;

            /**
             * @brief Which tree-construction algorithm #rebuildTree uses -- see :cpp:class:`IfTemplateTree`'s own three variants
             */
            enum class TreeKind {
                Basic,          ///< AGRemapCore::IfTemplateTree's own (base) algorithm
                NonEmptyNode,   ///< AGRemapCore::IfTemplateNonEmptyNodeTree's algorithm (the default, matching the pure-Python original's own default)
                Norm            ///< AGRemapCore::IfTemplateNormTree's algorithm
            };

            /**
             * @brief Constructs a new `IfTemplate`
             *
             * @param parts The individual parts of the `IfTemplate`, taken by ownership
             * @param runConfig The domain customization points this instance uses (see #IfTemplateRunConfig)
             * @param name The name of the `section`_ for this `IfTemplate`
             * @param treeKind Which tree-construction algorithm to use. **Default**: ``TreeKind::NonEmptyNode``
             * @param prefix Any prefix that precedes the content. **Default**: ``""``
             * @param suffix Any suffix that follows the content. **Default**: ``""``
             */
            explicit IfTemplate(std::vector<std::unique_ptr<IfTemplatePart>> parts, IfTemplateRunConfig<K, V> runConfig,
                                 std::string name = "", TreeKind treeKind = TreeKind::NonEmptyNode,
                                 std::string prefix = "", std::string suffix = "");

            // Explicitly deleted/defaulted for the same reason IfTemplateTree's own copy
            // operations are: #parts_/#tree_ make this class naturally move-only, and leaving that
            // implicit (rather than declared) trips a hard MSVC STL error the moment this class
            // template is instantiated at all -- see IfTemplateTree.h's own note on this.
            IfTemplate(const IfTemplate&) = delete;
            IfTemplate& operator=(const IfTemplate&) = delete;
            IfTemplate(IfTemplate&&) = default;
            IfTemplate& operator=(IfTemplate&&) = default;

            /**
             * @brief
             @rst
             Builds an `IfTemplate` by parsing raw parts (matching ``IfTemplate.py``'s own
             ``build`` classmethod)
             @endrst
             *
             * @param rawParts
             @rst
             Each entry is a starting line number paired with either a raw predicate string (parsed
             via :cpp:class:`IfPredPart`) or a fully-indexed ``src``-shaped description of an
             :cpp:type:`ContentPart` (see :cpp:class:`IfContentPart`'s own indexed constructor)
             @endrst
             * @param runConfig The domain customization points the new instance uses
             * @param name The name of the `section`_. **Default**: ``""``
             * @param ctx The context for parsing conditional predicates. A fresh one is used if ``nullptr``. **Default**: ``nullptr``
             * @param z3Ctx The `Z3`_ context every parsed :cpp:class:`IfPredPart` will share. A fresh one is used if ``nullptr``. **Default**: ``nullptr``
             *
             * @return The built `IfTemplate`
             */
            static std::unique_ptr<IfTemplate<K, V, KeyHash, KeyEqual>> build(
                const std::vector<std::pair<int, std::variant<std::string, tsl::ordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>>>>& rawParts,
                IfTemplateRunConfig<K, V> runConfig, std::string name = "", ParseContext* ctx = nullptr, Z3Context* z3Ctx = nullptr);

            /**
             * @brief The name of the `section`_ for this `IfTemplate`
             */
            std::string name;

            /**
             * @brief Any prefix that precedes the content
             */
            std::string prefix;

            /**
             * @brief Any suffix that follows the content
             */
            std::string suffix;

            /**
             * @brief The individual parts of the `IfTemplate`, owned by this instance
             */
            std::vector<std::unique_ptr<IfTemplatePart>>& parts();

            /**
             * @copydoc parts()
             */
            const std::vector<std::unique_ptr<IfTemplatePart>>& parts() const;

            /**
             * @brief The individual parts, by their id (not owned by this map)
             */
            const std::unordered_map<size_t, IfTemplatePart*>& partsById() const;

            /**
             * @brief
             @rst
             Any other `sections`_ this `IfTemplate` references, by ``run =`` :raw-html:`<br />`
             :raw-html:`<br />`

             The keys are the indices (into #parts) of the #ContentPart the `section`_ is called
             from; the values are the corresponding referenced `section`_ names for each part
             @endrst
             */
            const std::unordered_map<size_t, std::vector<std::string>>& calledSubCommands() const;

            /**
             * @brief The domain customization points this instance uses
             */
            const IfTemplateRunConfig<K, V>& runConfig() const;

            /**
             * @brief The parse tree for this `IfTemplate`
             */
            TreeType* tree() const;

            /**
             * @brief Updates the parse tree and the reference to other `sections`_ that this object calls
             */
            void rebuild();

            /**
             * @brief
             @rst
             Recomputes just #calledSubCommands()'s entry for a single part index, from that part's
             *current* ``run =`` values -- for a caller (eg. `IniSectionGraph::rename`) that just
             rewrote a ``run =`` value in place via :cpp:func:`IfContentPart::setValByInd` and needs
             #calledSubCommands() to reflect it, without paying for a full :cpp:func:`rebuild` (which
             would also needlessly reconstruct #tree()) :raw-html:`<br />` :raw-html:`<br />`

             Does nothing if 'partInd' has no existing #calledSubCommands() entry (matches the
             pure-Python original's own ``rename``, which only ever updates entries that already
             exist -- renaming never creates a *new* ``run =`` reference out of thin air).
             @endrst
             *
             * @param partInd The index (into #parts()) to refresh
             */
            void refreshCalledSubCommand(size_t partInd);

            /**
             * @brief Regenerates the ids for #parts
             */
            void refreshPartIds();

            /**
             * @brief Performs a deep copy of this `IfTemplate`
             *
             * @param newPartIds Whether to refresh the ids for each part. **Default**: ``true``
             * @return The copied object
             */
            std::unique_ptr<IfTemplate<K, V, KeyHash, KeyEqual>> deepcopy(bool newPartIds = true) const;

            /**
             * @brief
             @rst
             Normalizes the branching structure within this `IfTemplate` to follow
             :cpp:class:`IfTemplateNormTree`'s structure
             @endrst
             */
            void normalize();

            /**
             * @brief Adds a part to the `IfTemplate`, taking ownership of it
             *
             * @param part The part to add
             * @param updateTree Whether to update the parse tree. **Default**: ``false``
             * @return A non-owning pointer to the just-added part
             */
            IfTemplatePart* add(std::unique_ptr<IfTemplatePart> part, bool updateTree = false);

            /**
             * @brief Searches the `IfTemplate` for parts that meet a certain condition
             *
             * @tparam Result The postProcessor's return type
             * @param pred The predicate used to filter the parts. If empty, every part matches.
             * @param postProcessor A function that post-processes each matching part. If empty, defaults to a caller-supplied identity is required (there is no generic "return the part itself" for an arbitrary Result)
             * @return The filtered parts, keyed by their index in #parts
             */
            template <typename Result>
            std::unordered_map<size_t, Result> find(
                std::function<bool(IfTemplate<K, V, KeyHash, KeyEqual>&, size_t, IfTemplatePart&)> pred,
                std::function<Result(IfTemplate<K, V, KeyHash, KeyEqual>&, size_t, IfTemplatePart&)> postProcessor) const;

            /**
             * @brief Adds a new #ContentPart at the root of this `IfTemplate`, if needed
             *
             * @return The top part at the root of this `IfTemplate`
             */
            ContentPart* addTopContentPart();

            /**
             * @brief Adds some KVPs to the top of this `IfTemplate`
             */
            void addKVPsToFront(const std::vector<std::pair<K, V>>& kvps);

            /**
             * @brief Adds a KVP to the top of this `IfTemplate`
             */
            void addKVPToFront(const K& key, const V& val);

            /**
             * @brief Adds a new #ContentPart at the very end of this `IfTemplate`, if needed
             *
             * @return The bottom part at the end of this `IfTemplate`
             */
            ContentPart* addBottomContentPart();

            /**
             * @brief Adds some KVPs to the bottom of this `IfTemplate`
             */
            void addKVPsToBack(const std::vector<std::pair<K, V>>& kvps);

            /**
             * @brief Adds a KVP to the bottom of this `IfTemplate`
             */
            void addKVPToBack(const K& key, const V& val);

            // Deliberately no toStr() here, matching IfContentPart's own established precedent
            // (see IfContentPart.h's class doc comment): rendering a KVP's key/value to .ini text
            // needs K/V to be string-convertible, which isn't guaranteed for a generic K/V -- that
            // belongs at whichever layer already knows K/V concretely (the pybind11 binding, where
            // str() always works, matching PyIfContentPart.cpp's own toStr binding).

            /**
             * @brief
             @rst
             Checks whether 'key' appears in all branches of this `IfTemplate` -- see
             ``IfTemplate.py``'s own ``isKeyFullyCover`` for the full contract
             @endrst
             */
            bool isKeyFullyCover(const K& key, const std::unordered_map<std::string, IfTemplate<K, V, KeyHash, KeyEqual>*>& sections,
                                  std::unordered_set<std::string>& visited, std::unordered_map<std::string, bool>& sectionsKeyFullCover) const;

            /**
             * @brief
             @rst
             Finds every #ContentPart referenced by this `IfTemplate` that does not have 'key' --
             see ``IfTemplate.py``'s own ``getKeyMissingParts`` for the full contract
             @endrst
             */
            std::set<ContentPart*> getKeyMissingParts(const K& key, const std::unordered_map<std::string, IfTemplate<K, V, KeyHash, KeyEqual>*>& sections,
                                                        std::unordered_set<std::string>& visited,
                                                        std::unordered_map<std::string, std::set<ContentPart*>>& sectionsMissingParts,
                                                        std::unordered_map<std::string, bool>& sectionAllBranchesMissing) const;

        private:
            std::vector<std::unique_ptr<IfTemplatePart>> parts_;
            std::unordered_map<size_t, IfTemplatePart*> partsById_;
            std::unordered_map<size_t, std::vector<std::string>> calledSubCommands_;
            TreeKind treeKind_;
            std::unique_ptr<TreeType> tree_;
            IfTemplateRunConfig<K, V> runConfig_;

            std::unordered_map<size_t, IfTemplatePart*> setupPartsById() const;
            void rebuildTree();

            bool isKeyFullyCoverNode(NodeType& node, const K& key,
                const std::unordered_map<std::string, IfTemplate<K, V, KeyHash, KeyEqual>*>& sections,
                std::unordered_set<std::string>& visited, std::unordered_map<std::string, bool>& sectionsKeyFullCover) const;

            std::pair<std::set<ContentPart*>, bool> getKeyMissingPartsNode(NodeType& node, const K& key,
                const std::unordered_map<std::string, IfTemplate<K, V, KeyHash, KeyEqual>*>& sections,
                std::unordered_set<std::string>& visited,
                std::unordered_map<std::string, std::set<ContentPart*>>& sectionsMissingParts,
                std::unordered_map<std::string, bool>& sectionAllBranchesMissing) const;
    };

}

#include "IfTemplate.tpp"

#endif
