#ifndef AGRemapCore_ResGroupCollect_H
#define AGRemapCore_ResGroupCollect_H

#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"
#include "AGRemapCore/tools/Ranges.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Creates the :cpp:class:`IniSectionGraph` for a particular *group* of resources
     :raw-html:`<br />` :raw-html:`<br />`

     Where :cpp:class:`ResRegCollect` handles one resource at a time, this handles several that
     belong together -- a ``Blend.buf`` file and the textures that go with it, say -- and has to work out
     which combinations of them can actually co-occur. That is what makes it the one edit in this
     family that reasons about `Z3`_ predicates: two resources belong in the same group only if the
     conditional branches they live under are simultaneously satisfiable :raw-html:`<br />`
     :raw-html:`<br />`

     Roughly, per resource-group type: **collect** every reference (recording the query each one
     sits under), optionally **remap** the source graphs, **group** the referenced resources by
     satisfiability, **replicate** each resource's graph once per group that needs its own copy,
     then **connect** -- splicing ``if``/``endif`` blocks back into the source graphs so each call
     site reaches the right replica :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This is the one place in the codebase where predicates from two *different*
        :cpp:class:`Z3Context`\\s legitimately meet -- a source graph and a resource's own graph can
        come from different ``.ini`` files, each owning its own context. Every combination therefore
        goes through #combineQueries, which reparents whichever operand does not already belong to
        the target context. Combining raw would not reliably throw; it would silently misbehave
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class ResGroupCollect: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
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
             * @brief The type of `section`_ a graph is made of
             */
            using Section = typename Graph::Section;

            /**
             * @brief The type of part a resource reference lives in
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The per-part iteration data the predicates are handed
             */
            using IterData = SectionIterData<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The per-part iteration data carrying the query the part sits under
             */
            using IterQueryData = SectionIterQueryData<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ranges of valid `KVP`_ order indices a \ref PartPredicate returns
             */
            using OrderRanges = Ranges<long long>;

            /**
             * @brief The resource edit each resource is built with
             */
            using ResEdit = BaseResEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ``.ini`` file the resources are built for
             */
            using Context = typename ResEdit::Context;

            /**
             * @brief The graph remapper this uses when \ref remaps is set
             */
            using Remapper = GraphGroupRemap<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc GraphGroupRemap::RenameFunc
             */
            using RenameFunc = typename Remapper::RenameFunc;

            /**
             * @copydoc ResRegCollect::PartPredicate
             */
            using PartPredicate = std::function<OrderRanges(const IterQueryData&)>;

            /**
             * @copydoc ResRegCollect::ResPredicate
             */
            using ResPredicate = std::function<bool(const K&, const V&, const IterQueryData&)>;

            /**
             * @brief A map keyed by which graph something applies to
             */
            template <typename T>
            using ByGraph = tsl::ordered_map<GraphId, T, GraphIdHash>;

            /**
             * @brief
             @rst
             Builds one type of grouped resource :raw-html:`<br />` :raw-html:`<br />`

             The pure-Python original takes an ``IniGroupedResBuilder``, which wraps a *user-supplied
             class* to instantiate -- there is nothing for this class to construct itself. Behind an
             interface, the same holds for a plain C++ caller, and the `pybind11`_ layer's
             implementation simply calls the caller's own `Python`_ builder
             @endrst
             */
            class GroupedResBuilder {
                public:
                    virtual ~GroupedResBuilder() = default;

                    /**
                     * @brief
                     @rst
                     Builds a fresh grouped resource that is **not** yet marked built
                     :raw-html:`<br />` :raw-html:`<br />`

                     The implementation keeps ownership; the returned pointer must stay valid until
                     the edit finishes
                     @endrst
                     */
                    virtual IniGroupedResource* build() = 0;

                    /**
                     * @brief Hands a finished grouped resource to the ``.ini`` file it belongs to
                     *
                     * @param resource The grouped resource, already marked built and fully populated
                     */
                    virtual void store(IniGroupedResource& resource) = 0;

                    /**
                     * @brief
                     @rst
                     Files one built model into a grouped resource, under the resource type it
                     belongs to :raw-html:`<br />` :raw-html:`<br />`

                     Not :cpp:func:`IniGroupedResource::addResource` directly: that keys by a plain
                     ``std::string`` resource-type name, while a group here is keyed by a whole mod
                     object -- and the `Python`_-facing grouped resource stores these somewhere
                     entirely different from the C++ one (see ``PyIniGroupedResource``). Both are
                     the implementation's business
                     @endrst
                     *
                     * @param group The grouped resource to file into
                     * @param resType The mod object for the type of resource
                     * @param resource The built model. Ownership stays wherever it already was
                     */
                    virtual void addResource(IniGroupedResource& group, const GraphId& resType, IniResource& resource) = 0;
            };

            /**
             * @brief Where one reference to a resource was found
             */
            struct ResRootLocation {
                /**
                 * @brief The mod object for the type of resource being referenced
                 */
                GraphId resModObj;

                /**
                 * @brief The mod object of the graph the reference was found in
                 */
                GraphId srcModObj;

                /**
                 * @brief The name of the `section`_ the reference was found in
                 */
                std::string sectionName;

                /**
                 * @brief The id of the part the reference was found in
                 */
                std::size_t partId = 0;

                /**
                 * @brief The order index the reference was found at within that part
                 */
                long long orderInd = 0;

                bool operator<(const ResRootLocation& other) const;
                bool operator==(const ResRootLocation& other) const;
            };

            /**
             * @brief One collected reference to a resource, and the query it sits under
             */
            struct ResCall {
                /**
                 * @brief The name of the resource `section`_ being referenced
                 */
                V val;

                /**
                 * @brief The conditional predicate the reference sits under
                 */
                std::optional<Z3Predicate> query;
            };

            /**
             * @brief One resource that ended up in a group
             */
            struct ResGroupEntry {
                /**
                 * @brief The assigned id for the resource's source file
                 */
                std::string fileKey;

                /**
                 * @brief The name of the root `section`_ the resource's graph was reached through
                 */
                std::string rootSectionName;

                /**
                 * @brief Where the reference that pulled this resource in was found
                 */
                ResRootLocation rootLocation;

                /**
                 * @brief The depth of the part the resource's file was found at
                 */
                int partDepth = 0;

                /**
                 * @brief
                 @rst
                 The built resource model, filled in during the connect phase. ``nullptr`` until then
                 @endrst
                 */
                IniResource* resource = nullptr;
            };

            /**
             * @brief
             @rst
             One group of resources that can co-occur, plus the query under which they do
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The pure-Python original stored these entries **inside** the grouped resource's own
                ``resources`` dict, as placeholder tuples later overwritten with real resource
                objects, and ``copy.deepcopy``'d the whole thing to fork a group. That only works
                because the `Python`_-facing ``resources`` is an untyped ``dict``; the real
                :cpp:member:`IniGroupedResource::resources` is a typed map of owned
                :cpp:class:`IniResource`\\s and could never hold a placeholder. Keeping the working
                state here instead is both typed and cheaper to fork -- and the grouped resource
                itself is only built once a group actually survives (see #connectResGroups), rather
                than once per candidate
             @endrst
             */
            struct ResGroup {
                /**
                 * @brief The resources in this group, keyed by which type of resource each is
                 */
                ByGraph<ResGroupEntry> entries;

                /**
                 * @brief The query under which every resource in this group co-occurs
                 */
                std::optional<Z3Predicate> query;

                /**
                 * @brief
                 @rst
                 Whether this group is missing any of 'collected' -- the equivalent of
                 :cpp:func:`IniGroupedResource::isMissing`, over #entries
                 @endrst
                 *
                 * @param collected The resource types that have been collected so far
                 */
                bool isMissing(const std::unordered_set<GraphId, GraphIdHash>& collected) const;
            };

            /**
             * @brief The unique names for the type of resource groups
             */
            const std::vector<std::string>& resGroupTypes() const;

            /**
             * @brief
             @rst
             Sets the types of the resource groups -- duplicates are dropped, keeping the first
             occurrence of each, exactly as the pure-Python original's own property setter did
             @endrst
             *
             * @param newResGroupTypes The new types for the resource groups
             */
            void setResGroupTypes(std::vector<std::string> newResGroupTypes);

            /**
             * @brief
             @rst
             The registers that reference each resource, keyed first by the mod object for the type
             of resource, then by which :cpp:class:`IniSectionGraph` to search
             @endrst
             */
            ByGraph<ByGraph<K>> srcRegs;

            /**
             * @brief
             @rst
             How each resource in a group is built, keyed first by the mod object for the type of
             resource, then by the type of the resource group. Borrowed, not owned
             @endrst
             */
            ByGraph<tsl::ordered_map<std::string, ResEdit*>> resEdits;

            /**
             * @brief The builders used to construct each type of grouped resource. Borrowed, not owned
             */
            tsl::ordered_map<std::string, GroupedResBuilder*> groupedResBuilders;

            /**
             * @brief Which order indices to collect from, keyed by resource type then by source graph
             */
            ByGraph<ByGraph<PartPredicate>> partPredicates;

            /**
             * @brief Which references to collect, keyed by resource type then by source graph
             */
            ByGraph<ByGraph<ResPredicate>> resPredicates;

            /**
             * @brief
             @rst
             Whether to remap the graphs searched from \ref srcRegs, keyed by the resource's mod
             object then by the type of the resource group. Empty means no remapping at all
             @endrst
             */
            ByGraph<tsl::ordered_map<std::string, typename Remapper::RemapTarget>> remaps;

            /**
             * @brief Whether \ref trackKeysGlobal applies to every graph, instead of the per-graph \ref trackKeys entries
             */
            bool trackKeysIsGlobal = true;

            /**
             * @brief The key-tracking flag used for every graph when \ref trackKeysIsGlobal is ``true``
             */
            bool trackKeysGlobal = false;

            /**
             * @brief Whether to track `KVPs`_ while searching, keyed by resource type then by source graph
             */
            ByGraph<ByGraph<bool>> trackKeys;

            /**
             * @brief Which `KVP`_ keys to track while searching, keyed by resource type then by source graph
             */
            ByGraph<ByGraph<std::optional<std::unordered_set<K, KeyHash, KeyEqual>>>> keysToTrack;

            /**
             * @brief
             @rst
             Assumes every resource type has the same graph topology across all resource-group types,
             which lets the expensive satisfiability work be done once and reused :raw-html:`<br />`
             :raw-html:`<br />`

             Two graphs have "the same topology" when they hold exactly the same `sections`_, with
             the same structure and the same names. The state this is judged against is the graphs
             as :cpp:func:`BaseResEdit::getResGraph` returns them with ``rename = false``
             @endrst
             */
            bool resGroupTypesSameTopology = false;

            /**
             * @brief
             @rst
             The unique id for this object, which every replicated graph's own id is built from (see
             #getResGraphId)
             @endrst
             */
            long long id = 0;

            /**
             * @brief
             @rst
             The value a ``filename =`` `KVP`_ carries to mean "no resource at all" -- references to
             it are skipped. Defaults to :cpp:member:`IniKeywords::Null`
             @endrst
             */
            std::string nullValue;

            /**
             * @brief
             @rst
             Builds the ``V`` for a ``run =``-style reference to a `section`_, by name -- the same
             customization point :cpp:type:`IfTemplateRunConfig` carries, needed here because the
             new call sites this edit splices in name their target `sections`_
             @endrst
             */
            std::function<V(const std::string&)> valOfSectionName;

            /**
             * @brief
             @rst
             The calls to each resource, keyed by resource type, then source graph, then `section`_
             name, then part id, then order index :raw-html:`<br />` :raw-html:`<br />`

             Scratch state, rebuilt by every \ref edit
             @endrst
             */
            ByGraph<ByGraph<tsl::ordered_map<std::string, tsl::ordered_map<std::size_t, std::map<long long, ResCall>>>>> resCalls;

            /**
             * @brief Constructs a new resource-group-collecting edit
             *
             * @param resGroupTypes The unique names for the type of resource groups
             * @param srcRegs The registers that reference each resource
             * @param resEdits How each resource in a group is built. Borrowed, not owned
             * @param groupedResBuilders The builders for each type of grouped resource. Borrowed, not owned
             * @param valOfSectionName Builds the ``V`` for a reference to a `section`_, by name
             * @param id The unique id for this object
             */
            ResGroupCollect(std::vector<std::string> resGroupTypes = {}, ByGraph<ByGraph<K>> srcRegs = {},
                             ByGraph<tsl::ordered_map<std::string, ResEdit*>> resEdits = {},
                             tsl::ordered_map<std::string, GroupedResBuilder*> groupedResBuilders = {},
                             std::function<V(const std::string&)> valOfSectionName = {}, long long id = 0);

            /**
             * @brief Clears \ref resCalls and every resource edit's own saved state
             */
            void clear() override;

            /**
             * @brief
             @rst
             Combines two queries with a logical AND, tolerating them belonging to *different*
             :cpp:class:`Z3Context`\\s :raw-html:`<br />` :raw-html:`<br />`

             Whichever operand does not already belong to 'targetZ3Ctx' is reparented into it first
             -- the only way to make two `Z3`_ predicates combinable at all when they do not already
             share a context. The ``belongsTo`` check in front of that matters: it is a raw pointer
             comparison, while reparenting is a full render/re-parse/re-generate round trip, so the
             common same-context case stays free
             @endrst
             *
             * @param a The first query
             * @param b The second query
             * @param targetZ3Ctx
             @rst
             The context the result should belong to. ``nullptr`` skips reparenting entirely, which
             only produces a correct result when 'a'/'b' already share a context -- best-effort
             behaviour for a graph that was never threaded a context, rather than a hard failure
             @endrst
             *
             * @throw std::invalid_argument If reparenting either query into 'targetZ3Ctx' fails
             *
             * @return 'a' AND 'b'
             */
            static Z3Predicate combineQueries(const Z3Predicate& a, const Z3Predicate& b, Z3Context* targetZ3Ctx);

            /**
             * @brief Collects, groups, replicates and connects -- see this class's own description
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param ctx The .ini file the resources are built for
             * @param modType The type of mod to fix. Unused directly -- reaches the resource edits only
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& editWithContext(GraphGroups& graphGroups, Context& ctx, const ModType* modType,
                                          const std::string& modName = "");

            /**
             * @brief Collects and groups, but builds no resources -- there is no ``.ini`` file to build them for
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

        private:
            std::vector<std::string> resGroupTypes_;

            struct RemappedGraph {
                Graph* graph = nullptr;
                RenameFunc renameFunc;
                bool partIdRefreshRequired = false;
            };

            using RemappedGraphs = ByGraph<tsl::ordered_map<std::string, RemappedGraph>>;
            using CollectedSections = typename ResEdit::CollectedSections;
            using ResGroups = std::vector<ResGroup>;

            // One built resource, plus which replicated graph it came from -- the pure-Python
            // original's own (resource, graphId) pair.
            struct CollectedResource {
                IniResource* resource = nullptr;
                std::string graphId;
            };

            using CollectedResources = tsl::ordered_map<std::string, std::deque<CollectedResource>>;

            // The new "run =" call sites to splice into one source part, keyed by the order index
            // the original reference sat at. std::map (not insertion-ordered) deliberately: the
            // split below sorts its indices, so these have to be walked in the same ascending order.
            struct ResCallConnData {
                int partDepth = 0;
                std::vector<std::pair<std::string, std::optional<Z3Predicate>>> resCallers;
            };

            void collectFromGraphGroup(GraphGroups& graphGroups, const GraphId& resModObj, const GraphId& srcModObj, const K& srcReg);
            GraphGroups& remapGraphs(GraphGroups& graphGroups, RemappedGraphs* remappedGraphs);
            std::pair<bool, std::vector<GraphId>> isValidResGroupType(const std::string& resGroupType) const;
            CollectedSections getResCallNewNames(const GraphId& resModObj, const std::string& resGroupType,
                                                  tsl::ordered_map<std::string, std::optional<Z3Predicate>>& resRootQueries,
                                                  tsl::ordered_map<std::string, ResRootLocation>& resRootLocations,
                                                  const std::string& modName) const;
            Graph* getResGraph(GraphGroups& graphGroups, const std::string& resGroupType, const GraphId& resModObj,
                                ByGraph<CollectedSections>& resCallNewNames,
                                tsl::ordered_map<std::string, std::optional<Z3Predicate>>& resRootQueries,
                                tsl::ordered_map<std::string, ResRootLocation>& resRootLocations, Context* ctx,
                                const std::string& modName);
            Graph* collectAllResources(GraphGroups& graphGroups, const std::string& resGroupType, const GraphId& resModObj,
                                        ByGraph<CollectedSections>& resCallNewNames, GroupedResBuilder& builder,
                                        ResGroups& resGroups, std::unordered_set<GraphId, GraphIdHash>& collectedResTypes,
                                        Context* ctx, const std::string& modName);
            Graph* collectSatisfyingResources(GraphGroups& graphGroups, const std::string& resGroupType, const GraphId& resModObj,
                                               ByGraph<CollectedSections>& resCallNewNames, GroupedResBuilder& builder,
                                               ResGroups& resGroups, std::unordered_set<GraphId, GraphIdHash>& collectedResTypes,
                                               Context* ctx, const std::string& modName);
            void collectResGroups(GraphGroups& graphGroups, const std::string& resGroupType,
                                   ByGraph<CollectedSections>& resCallNewNames, const std::vector<GraphId>& commonResTypes,
                                   ResGroups& resGroups, ByGraph<Graph*>& resGraphs,
                                   std::unordered_set<GraphId, GraphIdHash>& collectedResTypes, Context* ctx,
                                   const std::string& modName);
            void collectResGraphNewNames(const std::string& resGroupType, const std::vector<GraphId>& commonResTypes,
                                          ByGraph<CollectedSections>& resCallNewNames, const std::string& modName);
            static bool fileKeyExists(const std::string& fileKey, tsl::ordered_map<std::string, long long>& fileFreqs);
            std::string getResGraphId(std::size_t resGroupTypeId, std::size_t resTypeId, std::size_t graphCopyId) const;
            void replicateResGraphs(const std::string& resGroupType, std::size_t resGroupTypeId,
                                     const ByGraph<tsl::ordered_map<std::string, long long>>& sectionFreqs,
                                     const ByGraph<tsl::ordered_map<std::string, long long>>& fileFreqs,
                                     const std::vector<GraphId>& commonResTypes, ByGraph<Graph*>& resGraphs,
                                     CollectedResources& collectedResources,
                                     ByGraph<std::vector<std::pair<Graph*, std::string>>>& combinedResGraphs,
                                     GraphGroups& graphGroups, Context* ctx, const std::string& modName);
            void countAndReplicateResGraphs(const std::string& resGroupType, std::size_t resGroupTypeId, const ResGroups& resGroups,
                                             ByGraph<tsl::ordered_map<std::string, long long>>& sectionFreqs,
                                             ByGraph<tsl::ordered_map<std::string, long long>>& fileFreqs,
                                             const std::vector<GraphId>& commonResTypes, ByGraph<Graph*>& resGraphs,
                                             CollectedResources& collectedResources,
                                             ByGraph<std::vector<std::pair<Graph*, std::string>>>& combinedResGraphs,
                                             GraphGroups& graphGroups, Context* ctx, const std::string& modName);
            static std::string getNewSectionName(const std::string& sectionName, const CollectedSections& resNewCalls,
                                                  const std::string& graphId);
            std::map<ResRootLocation, ResCallConnData> connectResGroups(ResGroups& resGroups, CollectedResources& collectedResources,
                                                                         const ByGraph<CollectedSections>& resCallNewNames,
                                                                         const std::string& resGroupType);
            std::vector<std::unique_ptr<IfTemplatePart>> buildResIfCalls(
                const std::vector<std::pair<std::string, std::optional<Z3Predicate>>>& resCallers, const K& srcReg,
                int depth, Z3Context* targetZ3Ctx) const;
            static std::vector<std::unique_ptr<IfTemplatePart>> splitIfContentPart(
                ContentPart& part, std::map<long long, std::vector<std::unique_ptr<IfTemplatePart>>>& ifPartsToAdd);
            static Graph* resolveToGraph(const std::string& resGroupType, const GraphId& srcModObj, GraphGroups& graphGroups,
                                          RemappedGraphs* remappedGraphs);
            void connectResCalls(const std::string& resGroupType, GraphGroups& graphGroups,
                                  const std::map<ResRootLocation, ResCallConnData>& resCallConnData,
                                  RemappedGraphs* remappedGraphs);
            static void cleanResCallGraphs(RemappedGraphs* remappedGraphs);
            void connectResGraphs(const std::string& resGroupType,
                                   ByGraph<std::vector<std::pair<Graph*, std::string>>>& combinedResGraphs,
                                   const ByGraph<CollectedSections>& resCallNewNames, GraphGroups& graphGroups);
            GraphGroups& editImpl(GraphGroups& graphGroups, Context* ctx, const ModType* modType, const std::string& modName);
    };
}

#include "ResGroupCollect.tpp"

#endif
