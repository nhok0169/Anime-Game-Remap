#ifndef AGRemapCore_ResEdit_H
#define AGRemapCore_ResEdit_H

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/constants/IniGraphReplaceMode.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IniResEditContext.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class to construct the necessary parts for a particular resource in a ``.ini`` file
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class BaseResEdit {
        public:

            /**
             * @brief The group of graphs a resource's graph is added to
             */
            using GraphGroups = IIniGraphGroups<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniGraphGroupEdit::Graph
             */
            using Graph = typename GraphGroups::Graph;

            /**
             * @copydoc IIniGraphGroups::Section
             */
            using Section = typename GraphGroups::Section;

            /**
             * @brief The type of part a resource reference lives in
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniGraphGroupEdit::GraphId
             */
            using GraphId = typename BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphId;

            /**
             * @brief The ``.ini`` file this builds resources for
             */
            using Context = IniResEditContext<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The target `sections`_ that reference the resource -- old `section`_ name mapped to its
             fixed name. Insertion-ordered, because the pure-Python original's own
             ``list(collectedSections.keys())`` order decides the built graph's target order
             @endrst
             */
            using CollectedSections = tsl::ordered_map<std::string, std::string>;

            /**
             * @brief
             @rst
             A predicate deciding which files to build a resource for -- takes the source file and
             its assigned id (see \ref getFileId). An empty ``std::function`` accepts everything
             @endrst
             */
            using ResourceFilter = std::function<bool(const std::string&, const std::string&)>;

            /**
             * @brief
             @rst
             The domain customization points a resource edit needs, in the same spirit as
             :cpp:class:`IfTemplateRunConfig` :raw-html:`<br />` :raw-html:`<br />`

             A ``.ini`` file's ``filename =`` `KVP`_ is a plain string in a plain C++ caller and a
             ``py::str`` in the `pybind11`_ layer, so neither the key nor the ``V`` <-> file-path
             conversion can be hardcoded here
             @endrst
             */
            struct ResEditConfig {
                /**
                 * @brief The ``.ini`` ``filename`` register key, as a ``K``
                 */
                K filenameKey;

                /**
                 * @brief Converts a ``filename =`` `KVP`_'s value into the file path it names
                 */
                std::function<std::string(const V&)> fileOf;

                /**
                 * @brief The inverse of #fileOf -- builds a ``filename =`` `KVP`_ value from a file path
                 */
                std::function<V(const std::string&)> valOfFile;
            };

            /**
             * @brief The name of the type of resource
             */
            std::string resType;

            /**
             * @brief The domain customization points this instance uses
             */
            ResEditConfig config;

            /**
             * @brief
             @rst
             The mod object to hold the newly created :cpp:class:`IniSectionGraph` for the resource
             @endrst
             */
            GraphId resModObj;

            /**
             * @brief
             @rst
             What to do when the corresponding :cpp:class:`IniSectionGraph` to construct already
             exists
             @endrst
             */
            IniGraphReplaceMode graphReplaceMode;

            /**
             * @brief Constructs a new resource edit
             *
             * @param resType The name of the type of resource
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param config The domain customization points this instance uses (see #ResEditConfig)
             * @param graphReplaceMode What to do when that graph already exists. **Default**: ``Ignore``
             */
            BaseResEdit(std::string resType, GraphId resModObj, ResEditConfig config,
                         IniGraphReplaceMode graphReplaceMode = IniGraphReplaceMode::Ignore);

            virtual ~BaseResEdit() = default;

            /**
             * @brief Clears any saved state information. No-op by default
             */
            virtual void clear();

            /**
             * @brief
             @rst
             Retrieves a unique id for a file within a single ``.ini`` file :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The value is **not** byte-identical to the pure-Python original's, which hashed a
                `Python`_ tuple through ``CyHashTools.hashLibSerialize``. It is an opaque
                within-one-run dictionary key -- never written to a file, never persisted, never
                compared against a previously stored value (the hash that *does* reach a filename is
                the separate ``graphId`` one, which hashes a plain string and is unchanged) -- so a
                different canonical serialization is safe here, and avoids two disagreeing
                implementations of the same id inside one run
             @endrst
             *
             * @param modObj The mod object holding the graph for the resource
             * @param sectionName The name of the `section`_
             * @param partId The id of the part where the file belongs to
             * @param orderInd The specific order index where the file occurs in the part
             * @param file The path for the file
             *
             * @return The unique id for the file
             */
            virtual std::string getFileId(const GraphId& modObj, const std::string& sectionName, std::size_t partId,
                                           long long orderInd, const std::string& file) const;

            /**
             * @brief
             @rst
             Collects the name of the fixed resource `section`_ (used for the ``collectedSections``
             argument of \ref buildResGraph)
             @endrst
             *
             * @param oldResourceName The old name of the resource `section`_
             * @param newResourceName The fixed name for the resource `section`_
             *
             * @return ``(old resource name, new resource name)``
             */
            virtual std::pair<std::string, std::string> collectResourceName(const std::string& oldResourceName,
                                                                            const std::string& newResourceName) const;

            /**
             * @brief Retrieves the name of the fixed resource `section`_
             *
             * @param resource The name of the original resource `section`_
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return
             @rst
             The `section`_ name of the fixed resource, or ``std::nullopt`` (the pure-Python
             original's ``None``) to indicate there is no name change between the original resource
             and the fixed resource
             @endrst
             */
            virtual std::optional<std::string> getFixResourceName(const std::string& resource, const std::string& modName = "") const;

            /**
             * @brief
             @rst
             Adds the unique id for the :cpp:class:`IniSectionGraph` of the resource to the name of
             the file
             @endrst
             *
             * @param file The path to the file to add the id to
             * @param graphId The id to add. **Default**: ``""``
             *
             * @return The file with the id added
             */
            static std::string fileAddGraphId(const std::string& file, const std::string& graphId = "");

            /**
             * @brief Retrieves the file path to the fixed resource
             *
             * @param file The file path to the original resource
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param graphId The unique id for the graph of the resource. **Default**: ``""``
             *
             * @return The file path to the fixed resource
             */
            virtual std::string getFixFile(const std::string& file, const std::string& modName = "",
                                            const std::string& graphId = "") const;

            /**
             * @brief
             @rst
             Builds the model for one resource **and stores it** :raw-html:`<br />`
             :raw-html:`<br />`

             The one place the concrete resource-edit classes genuinely differ: each builds a
             different kind of :cpp:class:`IniResource`. Storing is part of the same call rather than
             a separate step, because a built model is opaque to this class -- it is never read back
             :raw-html:`<br />` :raw-html:`<br />`

             The default builds a plain :cpp:class:`IniResource` from 'srcPath', ignoring 'fixedPath'
             @endrst
             *
             * @param resType The name for the type of resource
             * @param srcPath The file path to the original resource
             * @param fixedPath The file path to the fixed resource. Empty for an edit that has no separate fixed path
             * @param modName The name of the mod to fix to
             * @param fileKey The assigned id for the source file (see \ref getFileId)
             * @param ctx The .ini file the resource is being built for
             */
            virtual void buildResModel(const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
                                        const std::string& modName, const std::string& fileKey, Context& ctx);

            /**
             * @brief
             @rst
             Builds and saves the resources, given the :cpp:class:`IniSectionGraph` for a resource
             @endrst
             *
             * @param graph The graph for the particular resource
             * @param ctx The .ini file to build the resource for
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param resourceFilter Which files to build the resource for -- empty accepts everything. **Default**: empty
             * @param graphId The unique id for the graph of the resource. **Default**: ``""``
             * @param resModObj The mod object used to create the unique id for the resources, or ``nullptr`` for \ref resModObj. **Default**: ``nullptr``
             */
            virtual void buildResModels(Graph& graph, Context& ctx, const std::string& modName = "",
                                         const ResourceFilter& resourceFilter = {}, const std::string& graphId = "",
                                         const GraphId* resModObj = nullptr);

            /**
             * @brief
             @rst
             The name a `section`_ that was *not* collected gets renamed to -- \ref
             getFixResourceName, or the `section`_'s own name when that reports no change
             @endrst
             *
             * @param sectionName The name of the `section`_
             * @param modName The name of the mod to fix to. **Default**: ``""``
             */
            virtual std::string renameUncollectedSection(const std::string& sectionName, const std::string& modName = "") const;

            /**
             * @brief
             @rst
             Retrieves the particular :cpp:class:`IniSectionGraph` for the resource
             @endrst
             *
             * @param collectedSections The target `sections`_ that reference the resource
             * @param ctx The associated original .ini file being fixed
             * @param graphGroups The group of graphs for each .ini file
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param rename Whether to rename the `sections`_ for the graph. **Default**: ``true``
             * @param copySections Whether to make a deep copy of the `sections`_ referenced by the graph. **Default**: ``false``
             *
             * @return The retrieved graph, or ``nullptr``
             */
            virtual Graph* getResGraph(const CollectedSections& collectedSections, Context& ctx, GraphGroups& graphGroups,
                                        const std::string& modName = "", bool rename = true, bool copySections = false);

            /**
             * @brief
             @rst
             Builds the :cpp:class:`IniSectionGraph` and the corresponding models for the resources
             @endrst
             *
             * @param collectedSections The target `sections`_ that reference the resource
             * @param ctx The associated original .ini file being fixed
             * @param graphGroups The group of graphs to edit for each .ini file
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param resourceFilter Which files to build the resource for. **Default**: empty
             * @param copySections Whether to deep-copy the `sections`_ referenced by the graph. **Default**: ``false``
             *
             * @return
             @rst
             The same groups that were passed in, now including the newly created graph for the
             resource (reachable via \ref resModObj)
             @endrst
             */
            virtual GraphGroups& buildResources(const CollectedSections& collectedSections, Context& ctx, GraphGroups& graphGroups,
                                                 const std::string& modName = "", const ResourceFilter& resourceFilter = {},
                                                 bool copySections = false);
    };


    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseResEdit`

     Class to only build the :cpp:class:`IniSectionGraph` for the original collected resource
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class ResIdentity: public BaseResEdit<K, V, KeyHash, KeyEqual> {
        public:
            using Base = BaseResEdit<K, V, KeyHash, KeyEqual>;
            using Graph = typename Base::Graph;
            using GraphId = typename Base::GraphId;
            using Context = typename Base::Context;
            using ResourceFilter = typename Base::ResourceFilter;

            /**
             * @brief Whether to build the models for the resources
             */
            bool createResModel;

            /**
             * @brief Constructs a new identity resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param config The domain customization points this instance uses
             * @param createResModel Whether to build the models for the resources. **Default**: ``true``
             */
            ResIdentity(GraphId resModObj, typename Base::ResEditConfig config, bool createResModel = true);

            /**
             * @brief
             @rst
             Always ``std::nullopt`` -- an identity edit never renames the resource `section`_
             @endrst
             *
             * @param resource The name of the original resource `section`_. Unused
             * @param modName The name of the mod to fix to. Unused. **Default**: ``""``
             */
            std::optional<std::string> getFixResourceName(const std::string& resource, const std::string& modName = "") const override;

            /**
             * @brief Builds the resource models, unless \ref createResModel is ``false``
             *
             * @param graph The graph for the particular resource
             * @param ctx The .ini file to build the resource for
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param resourceFilter Which files to build the resource for. **Default**: empty
             * @param graphId The unique id for the graph of the resource. **Default**: ``""``
             * @param resModObj The mod object used to create the unique id for the resources. **Default**: ``nullptr``
             */
            void buildResModels(Graph& graph, Context& ctx, const std::string& modName = "",
                                 const ResourceFilter& resourceFilter = {}, const std::string& graphId = "",
                                 const GraphId* resModObj = nullptr) override;
    };


    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseResEdit`

     Class that creates the necessary parts for a fixed resource by building upon the existing
     :cpp:class:`IniSectionGraph` of the original resource :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The pure-Python original also overrode ``buildResources`` here, identically to the base's
        except for one out-of-range guard reading ``iniInd > len(graphGroups)`` where every sibling
        (including its own base) reads ``>=``. That difference is not a feature: at exactly
        ``iniInd == len(graphGroups)`` the ``>`` version fell straight through into an
        ``IndexError``. There is no override here, so the base's ``>=`` guard applies and that case
        is skipped, like everywhere else in this family
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class ResReplace: public BaseResEdit<K, V, KeyHash, KeyEqual> {
        public:
            using Base = BaseResEdit<K, V, KeyHash, KeyEqual>;
            using Graph = typename Base::Graph;
            using GraphId = typename Base::GraphId;
            using Context = typename Base::Context;
            using ResourceFilter = typename Base::ResourceFilter;
            using CollectedSections = typename Base::CollectedSections;
            using GraphGroups = typename Base::GraphGroups;

            using Base::Base;

            /**
             * @brief
             @rst
             Builds and saves the resources, renaming every referenced file to its fixed path along
             the way (unlike :cpp:func:`BaseResEdit::buildResModels`, which only ever appends a
             graph id)
             @endrst
             *
             * @param graph The graph for the particular resource
             * @param ctx The .ini file to build the resource for
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param resourceFilter Which files to build the resource for. **Default**: empty
             * @param graphId The unique id for the graph of the resource. **Default**: ``""``
             * @param resModObj The mod object used to create the unique id for the resources. **Default**: ``nullptr``
             */
            void buildResModels(Graph& graph, Context& ctx, const std::string& modName = "",
                                 const ResourceFilter& resourceFilter = {}, const std::string& graphId = "",
                                 const GraphId* resModObj = nullptr) override;

            /**
             * @brief Builds an :cpp:class:`IniFixResource` (which carries both paths), and stores it
             *
             * @param resType The name for the type of resource
             * @param srcPath The file path to the original resource
             * @param fixedPath The file path to the fixed resource
             * @param modName The name of the mod to fix to. Unused by this edit
             * @param fileKey The assigned id for the source file
             * @param ctx The .ini file the resource is being built for
             */
            void buildResModel(const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
                                const std::string& modName, const std::string& fileKey, Context& ctx) override;
    };


    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseResEdit`

     Class that creates the necessary parts for a brand-new fixed resource, building its
     `sections`_ from scratch rather than from the ``.ini`` file's existing ones
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class ResCreate: public BaseResEdit<K, V, KeyHash, KeyEqual> {
        public:
            using Base = BaseResEdit<K, V, KeyHash, KeyEqual>;
            using Graph = typename Base::Graph;
            using Section = typename Base::Section;
            using GraphId = typename Base::GraphId;
            using Context = typename Base::Context;
            using CollectedSections = typename Base::CollectedSections;
            using GraphGroups = typename Base::GraphGroups;

            using Base::Base;

            /**
             * @brief
             @rst
             Builds a `section`_ for the resource :raw-html:`<br />` :raw-html:`<br />`

             **The implementation keeps ownership** of whatever it returns, for as long as the graph
             built from it lives -- the graph only borrows its `sections`_ unless it was asked to
             copy them (see :cpp:class:`IniSectionGraph`'s own note)
             @endrst
             *
             * @param sectionName The name for the `section`_
             * @param modName The name of the mod to fix to
             *
             * @return The generated `section`_, or ``nullptr`` to skip it
             */
            virtual Section* buildSection(const std::string& sectionName, const std::string& modName) = 0;

            /**
             * @brief
             @rst
             ``(new name, new name)`` -- a created resource has no "old" name to collect, so both
             halves are the fixed one
             @endrst
             *
             * @param oldResourceName The old name of the resource `section`_. Unused
             * @param newResourceName The fixed name for the resource `section`_
             */
            std::pair<std::string, std::string> collectResourceName(const std::string& oldResourceName,
                                                                     const std::string& newResourceName) const override;

            /**
             * @brief
             @rst
             Retrieves the resource's graph, building its `sections`_ from scratch via \ref
             buildSection rather than from the ``.ini`` file's existing ones
             @endrst
             *
             * @param collectedSections The target `sections`_ that reference the resource
             * @param ctx The associated original .ini file being fixed
             * @param graphGroups The group of graphs for each .ini file
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param rename Ignored -- a created graph's `sections`_ are already built under their fixed names. **Default**: ``true``
             * @param copySections Whether to deep-copy the built `sections`_. **Default**: ``false``
             *
             * @return The retrieved graph, or ``nullptr``
             */
            Graph* getResGraph(const CollectedSections& collectedSections, Context& ctx, GraphGroups& graphGroups,
                                const std::string& modName = "", bool rename = true, bool copySections = false) override;
    };
}

#include "ResEdit.tpp"

#endif
