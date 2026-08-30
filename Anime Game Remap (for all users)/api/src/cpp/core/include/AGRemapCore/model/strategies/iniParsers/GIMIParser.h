#ifndef AGRemapCore_GIMIParser_H
#define AGRemapCore_GIMIParser_H

#include <cstddef>
#include <type_traits>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfContentPartColour.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseContext.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseDownloadData.h"
#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Parses a ``.ini`` file used by a ``GIMI``-style importer :raw-html:`<br />`
     :raw-html:`<br />`

     What it produces, per mod object (a ``(component, object)`` pair such as ``("", "body")`` or
     ``("bang", "B")``):

     #. **A command graph** -- the ``TextureOverride``-rooted caller/callee subgraph of `sections`_
        that mod object is drawn by. These live in group ``0`` of the
        :cpp:func:`IniParseContext::graphGroups`, see #commandGraphs
     #. **Download resource graphs** -- one tiny graph per register whose file the mod is missing,
        holding the ``Resource...RemapDL`` `section`_ this parser synthesized for it, see
        #downloadResourceGraphs

     :raw-html:`<br />`

     **How a `section`_ gets attributed to a mod object** is the interesting half, and there are two
     strategies:

     * **By name** (:cpp:func:`classifyByTextureOverrideName`) -- used when #trackKeys or
       #makeGlobalGraph is off. An `Aho-Corasick`_ automaton over ``"{component}{object}"`` matches
       against the tail of each ``TextureOverride...`` `section`_ name
     * **By `KVP`_** (:cpp:class:`GIMISectionClassifier`) -- used otherwise. Every
       :cpp:class:`IfContentPart` in the file is walked with its `KVP`_ colouring, and its ``hash``/
       ``match_first_index`` values are resolved against the mod type's real asset tables

     :raw-html:`<br />`

     .. note::
        Divergences from the pure-Python original, all deliberate:

        * **The ``.ini`` file is reached through** :cpp:class:`IniParseContext`, not through
          :cpp:member:`BaseIniParser::iniFile_` -- see that interface's own note on why. The
          inherited ``iniFile_`` stays ``nullptr`` for every real caller
        * **Every** :cpp:type:`ObjTargetFunc` **takes 6 arguments**
          ``(parser, sectionName, section, disjoint, part, kvps)``, which is exactly what the
          original *documents*. The original's own by-name default is written with a **7th**
          ``partInd`` parameter and called with 7 arguments, so any user-supplied function written
          to the documented 5-plus-self shape raises ``TypeError`` the moment the by-name strategy
          runs. Reported rather than reproduced
        * **The by-`KVP`_ strategy passes the real** `section`_ **in the ``section`` slot.** The
          original passes the :cpp:class:`IfContentPart` there instead (so ``section`` and ``part``
          are the same object), contradicting its own documented signature. Nothing in the codebase
          reads that argument, so this is a strictly-better fix rather than an observable change
        * **#getDownloads returns a typed** :cpp:type:`DownloadNeeds` (a
          :cpp:class:`DownloadTargets` struct holding *either* parts *or* `sections`_) rather than
          the original's ``Union[Set[IfContentPart], Set[IfTemplate]]`` -- there is no such union
          type in C++, and which side is populated is already decided by
          :cpp:func:`IniParseDownloadData::refToSection`
        * **#removeAddedIfTemplates iterates its own name set** rather than the ``.ini`` file's
          `section`_ map. The original iterates the map while popping from it, which raises
          ``RuntimeError: dictionary changed size during iteration`` -- unreachable today only
          because nothing ever adds to ``_addedIfTemplateNames``, this class included. #parse still
          adds nothing to #addedIfTemplateNames (faithful), but the method now works if a subclass
          or fixer does
        * **The ``textureOverrideClassifier`` cache is a real typed member**, not an entry in the
          free-form ``tempKwargs`` dict the original stashes it in. ``tempKwargs`` itself survives
          on the `Python`_ binding as the user-facing scratch space it's documented to be
        * **#parse returns a real result**, where the pure-Python original returns ``None`` and
          leaves everything on the parser. It hands back the single :cpp:class:`IniGraphGroup`
          :cpp:func:`BaseIniParser::parse` promises -- see #collectParseResult for its exact shape.
          #commandGraphs / #downloadResourceGraphs still work, and are still the *live* graphs
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     * @tparam ParserBase
     @rst
     Which :cpp:class:`BaseIniParser` specialization to actually derive from -- defaults to the
     obvious ``BaseIniParser<K, V, KeyHash, KeyEqual>``, which is what every plain C++ caller
     wants :raw-html:`<br />` :raw-html:`<br />`

     It is a parameter purely so the `pybind11`_ layer can splice its own subclass (the one holding
     the ``_iniFile``/``_modsToFix`` `Python`_ state the pure-Python ``BaseIniParser`` had) into the
     hierarchy *between* this class and the core base. Without it, ``py::class_<GIMIParser,
     BaseIniParser>`` could not be registered against a `Python`_-state-carrying base at all, since
     `pybind11`_ inheritance needs a single, real C++ inheritance path -- and the alternatives
     (virtual inheritance, or dropping the inheritance claim) are both worse
     @endrst
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>,
               typename ParserBase = BaseIniParser<K, V, KeyHash, KeyEqual>>
    class GIMIParser: public ParserBase {
        public:
            static_assert(std::is_base_of_v<BaseIniParser<K, V, KeyHash, KeyEqual>, ParserBase>,
                           "GIMIParser's ParserBase must derive from BaseIniParser<K, V, KeyHash, KeyEqual>");

            using Base = ParserBase;
            using GraphGroup = typename BaseIniParser<K, V, KeyHash, KeyEqual>::GraphGroup;

            /**
             * @copydoc IniGraphGroup::ModObj
             */
            using ModObj = std::pair<std::string, std::string>;

            /**
             * @copydoc IniGraphGroup::ModObjHash
             */
            using ModObjHash = typename IniGraphGroup<K, V, KeyHash, KeyEqual>::ModObjHash;

            /**
             * @brief The type of `section`_ this parses
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The type of `KVP`_ block within a `section`_
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The `KVP`_ state tracked while walking a `section`_
             */
            using Colouring = IfContentPartColouring<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            /**
             * @brief The type of caller/callee graph this builds
             */
            using Graph = IniSectionGraph<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ``.ini`` file this parses
             */
            using Context = IniParseContext<K, V, KeyHash, KeyEqual>;

            /**
             * @brief One file this mod may have to download
             */
            using DownloadData = IniParseDownloadData<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The kind of edit #commandEdits is
             */
            using GroupEdit = BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The by-`KVP`_ classification strategy this parser uses by default
             */
            using Classifier = GIMISectionClassifier<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             One way of deciding which mod object(s) a `section`_ (or one
             :cpp:class:`IfContentPart` of it) belongs to :raw-html:`<br />` :raw-html:`<br />`

             Takes, in order: this parser, the name of the `section`_, the `section`_ itself,
             whether only one result is wanted, the :cpp:class:`IfContentPart` being classified
             (``nullptr`` unless #trackKeys), and that part's `KVP`_ colouring (``nullptr``, same
             condition). Returns the mod objects the `section`_ belongs to, empty for none --
             see this class's own note on why this is 6 arguments and not the original's 7
             @endrst
             */
            using ObjTargetFunc = std::function<std::vector<ModObj>(GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>&, const std::string&,
                                                                     Section*, bool, ContentPart*, const Colouring*)>;

            /**
             * @brief
             @rst
             Where one register's missing file has to be referenced from -- the typed replacement
             for the original's ``Union[Set[IfContentPart], Set[IfTemplate]]``, see this class's own
             note
             @endrst
             */
            struct DownloadTargets {
                /**
                 * @brief
                 @rst
                 The :cpp:class:`IfContentPart`\\s missing the register, when the download is
                 referenced per-part (:cpp:func:`IniParseDownloadData::refToSection` is ``false``)
                 @endrst
                 */
                std::set<ContentPart*> parts;

                /**
                 * @brief
                 @rst
                 The `sections`_ missing the register, when the download is referenced once at the
                 top of a `section`_ (:cpp:func:`IniParseDownloadData::refToSection` is ``true``)
                 @endrst
                 */
                std::set<Section*> sections;

                /**
                 * @brief Whether this entry populated #sections rather than #parts
                 */
                bool refToSection = false;
            };

            /**
             * @brief The registers, per mod object, whose file the mod is missing -- what #getDownloads returns
             */
            using DownloadNeeds = tsl::ordered_map<ModObj, tsl::ordered_map<K, DownloadTargets, KeyHash, KeyEqual>, ModObjHash>;

            /**
             * @brief The files to download, per mod object then per register
             */
            using Downloads = tsl::ordered_map<ModObj, tsl::ordered_map<K, DownloadData*, KeyHash, KeyEqual>, ModObjHash>;

            /**
             * @brief The download resource graphs, per mod object then per register
             */
            using ResourceGraphs = tsl::ordered_map<ModObj, tsl::ordered_map<K, Graph*, KeyHash, KeyEqual>, ModObjHash>;

            /**
             * @brief
             @rst
             The ``.ini``-domain customization points this class needs -- ``K``/``V`` are not
             ``std::string`` for every instantiation, so no ``.ini`` keyword can be spelled as a
             literal here
             @endrst
             */
            struct ParserConfig {
                /**
                 * @brief The customization points the default :cpp:class:`GIMISectionClassifier` uses
                 */
                typename Classifier::ClassifierConfig classifier;

                /**
                 * @brief The run configuration any `section`_ built by this parser uses
                 */
                IfTemplateRunConfig<K, V> runConfig;

                /**
                 * @brief Converts a `section`_ name into the `KVP`_ value that references it
                 */
                std::function<V(const std::string&)> valOfSectionName;
            };

            /**
             * @brief The `Aho-Corasick`_ automaton #classifyByTextureOverrideName matches names with
             *
             @rst
             A ``std::nullopt`` value marks the ``remap`` keyword -- a `section`_ whose name
             contains it was written by this software and is never a classification target. Every
             other entry carries the mod object its search text stands for
             @endrst
             */
            using NameClassifier = BaseAhoCorasickDFA<std::optional<ModObj>>;

            /**
             * @brief The default #ParserConfig for a plain, ``std::string``-keyed C++ caller
             */
            static ParserConfig defaultConfig();

            /**
             * @brief Constructs a new parser
             *
             * @param ctx
             @rst
             The ``.ini`` file to parse -- **borrowed**, and must outlive this parser. See
             :cpp:class:`IniParseContext` for why this isn't an :cpp:class:`IniFile`
             @endrst
             * @param modObjs The mod objects to parse, in the order they should be walked. **Default**: empty
             * @param objTargetFuncs How to find each mod object's root `sections`_ -- see #objTargetFuncs. **Default**: empty
             * @param downloads The files to download if the mod is missing some -- **borrowed**. **Default**: empty
             * @param commandEdits Further edits to apply to the parsed command graphs -- **borrowed**, nullable. **Default**: ``nullptr``
             * @param makeGlobalGraph Whether to build a graph over the entire .ini file. **Default**: ``true``
             * @param disjointModObjs Whether each `section`_ belongs to at most one mod object. **Default**: ``true``
             * @param trackKeys Whether to track the `KVPs`_ in the .ini file. **Default**: ``true``
             * @param keysToTrack Which keys to track, or ``std::nullopt`` for all of them. **Default**: ``std::nullopt``
             * @param config The .ini-domain customization points to use. **Default**: #defaultConfig
             */
            explicit GIMIParser(Context* ctx, std::vector<ModObj> modObjs = {}, std::vector<ObjTargetFunc> objTargetFuncs = {},
                                 Downloads downloads = {}, GroupEdit* commandEdits = nullptr,
                                 bool makeGlobalGraph = true, bool disjointModObjs = true,
                                 bool trackKeys = true,
                                 std::optional<std::unordered_set<K, KeyHash, KeyEqual>> keysToTrack = std::nullopt,
                                 ParserConfig config = defaultConfig());

            /**
             * @brief
             @rst
             How to find the root `sections`_ of each mod object :raw-html:`<br />`
             :raw-html:`<br />`

             Every entry is consulted in order. When empty, #parse falls back to
             :cpp:func:`classifyByTextureOverrideName` (or, for the by-`KVP`_ strategy against a
             classified ``.ini`` file, a default :cpp:class:`GIMISectionClassifier`)
             @endrst
             */
            std::vector<ObjTargetFunc> objTargetFuncs;

            /**
             * @brief The files to download if the mod is missing some -- borrowed, not owned
             */
            Downloads downloads;

            /**
             * @brief Whether to build a graph over the entire .ini file
             */
            bool makeGlobalGraph;

            /**
             * @brief Whether the set of `sections`_ for each mod object should be disjoint
             */
            bool disjointModObjs;

            /**
             * @brief Whether to track the `KVPs`_ in the .ini file
             */
            bool trackKeys;

            /**
             * @brief
             @rst
             Which `KVP`_ keys to track, or ``std::nullopt`` to track every key encountered
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Only has any effect when #trackKeys and #makeGlobalGraph are both ``true``
             @endrst
             */
            std::optional<std::unordered_set<K, KeyHash, KeyEqual>> keysToTrack;

            /**
             * @brief
             @rst
             The names of the `sections`_ this parser (or one of its associated fixers) added to
             the ``.ini`` file, to be taken back out by #removeAddedIfTemplates :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                #parse never adds to this, matching the pure-Python original exactly -- the
                ``Resource...RemapDL`` and ``TextureOverride...RemapFix`` `sections`_ it
                synthesizes stay in the ``.ini`` file, because the fixers that run after it need
                them. It is public so a subclass or fixer that *does* want its own additions
                cleaned up can record them
             @endrst
             */
            std::unordered_set<std::string> addedIfTemplateNames;

            /**
             * @brief The ``.ini`` file being parsed -- borrowed, see the constructor
             */
            Context* ctx() const;

            /**
             * @copydoc ctx() const
             */
            void setCtx(Context* ctx);

            /**
             * @brief
             @rst
             The mod objects to parse, in walk order :raw-html:`<br />` :raw-html:`<br />`

             A ``std::vector`` rather than a set: the pure-Python original is documented as taking
             a ``Set`` but every real caller hands it an ``OrderedSet``, and that order is
             load-bearing -- it decides the order the command graphs are built (and so rendered)
             in. Duplicates are dropped on assignment, first occurrence winning
             @endrst
             */
            const std::vector<ModObj>& modObjs() const;

            /**
             * @copydoc modObjs() const
             */
            void setModObjs(std::vector<ModObj> newModObjs);

            /**
             * @brief The distinct components of #modObjs, in first-seen order
             */
            const std::vector<std::string>& components() const;

            /**
             * @brief
             @rst
             Further edits to apply to the parsed command graphs -- borrowed, ``nullptr`` for none
             @endrst
             */
            GroupEdit* commandEdits() const;

            /**
             * @copydoc commandEdits() const
             */
            void setCommandEdits(GroupEdit* newCommandEdits);

            /**
             * @brief
             @rst
             The caller/callee graph of ``TextureOverride`` command `sections`_ for each mod object
             :raw-html:`<br />` :raw-html:`<br />`

             These live in group ``0`` of :cpp:func:`IniParseContext::graphGroups`, which owns them
             -- this is a borrowed view, rebuilt on each call
             @endrst
             */
            std::vector<std::pair<ModObj, Graph*>> commandGraphs() const;

            /**
             * @brief The command graph for one mod object, or ``nullptr`` if there isn't one
             *
             * @param modObj The mod object to look up
             */
            Graph* getCommandGraph(const ModObj& modObj) const;

            /**
             * @brief
             @rst
             The caller/callee graphs of the ``Resource...RemapDL`` `sections`_ this parser
             synthesized, per mod object then per register :raw-html:`<br />` :raw-html:`<br />`

             Owned by :cpp:func:`IniParseContext::graphGroups` (they are deliberately in no group),
             borrowed here
             @endrst
             */
            const ResourceGraphs& downloadResourceGraphs() const;

            /**
             * @brief
             @rst
             The names of the `sections`_ used as the "entry point" into each mod object's group of
             ``TextureOverride`` `sections`_ :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                These are not necessarily the roots of the graph -- they may be a child of some
                other `section`_. Based off heuristics, the probability of them being the roots is
                nonetheless pretty high
             @endrst
             */
            const tsl::ordered_map<ModObj, std::vector<std::string>, ModObjHash>& sectionTargets() const;

            /**
             * @brief The graph over the entire .ini file, or ``nullptr`` if it hasn't been built
             */
            Graph* globalGraph() const;

            /**
             * @copydoc globalGraph() const
             */
            void setGlobalGraph(Graph* newGlobalGraph);

            /**
             * @brief The .ini-domain customization points this instance uses
             */
            const ParserConfig& config() const;

            /**
             * @brief
             @rst
             Removes the `sections`_ recorded in #addedIfTemplateNames from the ``.ini`` file, and
             empties that record
             @endrst
             */
            void removeAddedIfTemplates();

            void clear() override;

            /**
             * @brief Builds the graph over the entire .ini file
             *
             * @return The built graph -- owned by :cpp:func:`IniParseContext::graphGroups`
             */
            Graph* buildGlobalGraph();

            /**
             * @brief
             @rst
             Classifies a ``TextureOverride`` `section`_ into mod objects by its **name**
             :raw-html:`<br />` :raw-html:`<br />`

             A `section`_ qualifies when its (lowercased, stripped) name starts with
             ``textureoverride``, does **not** contain ``remap`` anywhere after that prefix, and
             *ends with* one of the ``"{component}{object}"`` search texts
             @endrst
             *
             * @param parser The parser to classify for
             * @param sectionName The name of the `section`_ to classify
             * @param disjoint Whether to stop at the first match. **Default**: ``true``
             * @param modObjs
             @rst
             The mod objects to classify against, or ``nullptr`` to use 'parser''s own #modObjs.
             **Default**: ``nullptr``
             @endrst
             * @param fromRoots
             @rst
             Whether to make sure 'parser''s #globalGraph exists first. Has no other effect --
             faithful to the pure-Python original, whose own parameter is likewise consulted
             nowhere else. **Default**: ``true``
             @endrst
             *
             * @return The mod objects the `section`_ was classified into, empty for none
             */
            static std::vector<ModObj> classifyByTextureOverrideName(GIMIParser<K, V, KeyHash, KeyEqual, ParserBase>& parser, const std::string& sectionName,
                                                                      bool disjoint = true, const std::vector<ModObj>* modObjs = nullptr,
                                                                      bool fromRoots = true);

            /**
             * @brief Builds the command graph of every mod object with a known entry point
             */
            void parseCommands();

            /**
             * @brief
             @rst
             Works out where each of #downloads has to be referenced from, for the registers the
             mod is actually missing
             @endrst
             */
            DownloadNeeds getDownloads();

            /**
             * @brief
             @rst
             Adds each required download resource `section`_ to the ``.ini`` file and references it
             from wherever #getDownloads said it was needed
             @endrst
             *
             * @param partsNeedDownload What #getDownloads returned
             */
            void addDownloads(const DownloadNeeds& partsNeedDownload);

            /**
             * @brief Sets up the required download resources, if not already set up
             */
            void setupDownloads();

            /**
             * @brief
             @rst
             Applies #commandEdits to the command graphs. No-op when there are none
             @endrst
             */
            virtual void editCommands();

            /**
             * @brief
             @rst
             Collects everything the last #parse produced into the one
             :cpp:class:`IniGraphGroup` #parse hands back :raw-html:`<br />` :raw-html:`<br />`

             The group holds, in this order:

             #. every graph in #commandGraphs, under its own ``(component, mod object)`` key
             #. every graph in #downloadResourceGraphs, under
                ``(IniGraphModObjKeywords::Download, <the download's name>)`` -- see that class for
                why the component half is a reserved name. A download whose name is already in the
                group is skipped, so one resource shared by several registers appears once

             :raw-html:`<br />`

             This is the same group ``GIMIFixer`` builds by hand out of those two members today.

             :raw-html:`<br />`

             .. note::
                The graphs in the returned group are **deep copies**, so the group owns them and
                stays valid however the parser is used afterwards. It has to be: an
                :cpp:class:`IniGraphGroup` owns its graphs (``std::unique_ptr``), while every graph
                this parser holds is borrowed from its :cpp:class:`IniParseContext` (see
                :cpp:class:`IIniGraphGroups`'s ownership contract). ``GIMIFixer`` already
                deep-copies the command graphs for the same reason -- so a second fixer pass sees
                the parser's own state unedited :raw-html:`<br />` :raw-html:`<br />`

                The `pybind11`_ layer overrides this to hand back the *live* graph objects in a
                `Python`_ ``IniGraphGroup`` instead, since a `Python`_ group holds real references
                rather than owning anything -- see ``PyGIMIParser::parseToPy``
             @endrst
             *
             * @return A one-element vector holding the collected group
             */
            virtual std::vector<GraphGroup> collectParseResult() const;

            /**
             * @brief
             @rst
             Parses the ``.ini`` file, then collects the result -- see #collectParseResult for
             what comes back :raw-html:`<br />` :raw-html:`<br />`

             The parse itself happens in four phases: find each mod object's entry-point
             `sections`_ (#getSectionTargets), build their command graphs (#parseCommands),
             synthesize whatever download resources the mod is missing (#setupDownloads), then
             apply #commandEdits (#editCommands)
             @endrst
             */
            std::vector<GraphGroup> parse() override;

            /**
             * @brief
             @rst
             Finds the "entry point" `section`_ names of every mod object in #modObjs, into
             #sectionTargets :raw-html:`<br />` :raw-html:`<br />`

             Public rather than protected because the pure-Python original's own
             ``_getSectionTargets`` is only private by naming convention, and the `Python`_ API
             keeps exposing it under that name
             @endrst
             */
            virtual void getSectionTargets();

        private:
            Context* ctx_;
            std::vector<ModObj> modObjs_;
            std::vector<std::string> components_;
            GroupEdit* commandEdits_;
            ResourceGraphs downloadResourceGraphs_;
            tsl::ordered_map<ModObj, std::vector<std::string>, ModObjHash> sectionTargets_;
            Graph* globalGraph_;
            bool downloadsAdded_;

            // Every download resource `section`_ name built by this parse. A resource is identified
            // by its `section`_ name (which is what a .ini file can only have one of), not by the
            // register that asked for it -- see createDownloadResource's own note.
            std::unordered_set<std::string> createdDownloadResources_;
            ParserConfig config_;

            // The pure-Python original stashes this in its free-form 'tempKwargs' dict; here it is
            // a real member, rebuilt whenever #modObjs changes (that dict is instead cleared
            // wholesale by clear(), which is coarser than it needs to be).
            std::unique_ptr<NameClassifier> nameClassifier_;
            std::vector<ModObj> nameClassifierModObjs_;

            // Only built (and only kept alive) for the by-KVP strategy's own fallback, when the
            // caller supplied no objTargetFuncs at all.
            std::unique_ptr<Classifier> defaultClassifier_;

            using GraphMapEntry = std::pair<ModObj, Graph*>;

            void refreshComponents();
            NameClassifier& nameClassifier(const std::vector<ModObj>& modObjs);
            std::vector<ObjTargetFunc> resolveObjTargetFuncs(bool byKVPs);
            void getSectionTargetsBySectionNames(tsl::ordered_map<ModObj, std::vector<std::string>, ModObjHash>& result);
            void getSectionTargetsByKVPs(tsl::ordered_map<ModObj, std::vector<std::string>, ModObjHash>& result);
            /**
             * @brief
             @rst
             Builds the ``Resource...RemapDL`` `section`_ for one register's missing file, adds it
             to that register's own resource graph, and records the download the ``.ini`` file has
             to make :raw-html:`<br />` :raw-html:`<br />`

             The `section`_ is built **once per name**, not once per register: a ``.ini`` file can
             only hold one `section`_ of a given name, so one
             :cpp:class:`IniParseDownloadData` referenced from several registers (or several mod
             objects) contributes exactly one `section`_ and one download, shared by every resource
             graph that asked for it :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The pure-Python original guards per ``(mod object, register)`` resource graph
                instead, so a shared download is built once per register -- and the second build
                raises ``ValueError: ... FileDownload: Python instance was disowned``, because
                ``RemapIniDownload`` takes ownership of the ``FileDownload`` the first one already
                handed it. That made sharing a download across registers unusable rather than
                merely wasteful :raw-html:`<br />` :raw-html:`<br />`

                Sharing one ``FileDownload`` object across two *different*
                :cpp:class:`IniParseDownloadData`\s is still not supported, for the same
                ownership reason -- give each its own
             @endrst
             *
             * @param modTypeName The name of the mod type, used to name the `section`_
             * @param modObj Which mod object's resource graphs this belongs to
             * @param reg The register whose file is missing
             * @param downloadData The download to build the resource for
             * @param iniFolder The folder the .ini file lives in
             *
             * @return The name of the resource `section`_ to reference
             */
            std::string createDownloadResource(const std::string& modTypeName, const ModObj& modObj, const K& reg,
                                                DownloadData& downloadData, const std::string& iniFolder);
    };
}

#include "GIMIParser.tpp"

#endif
