#ifndef AGRemapCore_GIMIFixer_H
#define AGRemapCore_GIMIFixer_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/constants/IniBoilerPlate.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"
#include "AGRemapCore/tools/Heading.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixContext.h"
// For the dynamic_cast in fixImpl: modTypeName() -- the mod type being fixed FROM -- lives on
//   RemapIniFixContext rather than on the IniFixContext this class's Context typedef names.
#include "AGRemapCore/model/strategies/iniFixers/RemapIniFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Fixes a ``.ini`` file used by a ``GIMI``-style importer :raw-html:`<br />` :raw-html:`<br />`

     Where :cpp:class:`GIMIParser` works out *which* `sections`_ belong to which mod object, this
     turns that into the new ``.ini`` file text. It does three things, in order:

     #. **Take the parsed graphs.** Every graph :cpp:func:`GIMIParser::parse` handed back is deep-
        copied into this fixer's own :cpp:func:`graphGroups`, so editing them leaves the parser's
        own state untouched and a second fixer over the same ``.ini`` file starts from the same
        place. See #getFix
     #. **Edit them, once per mod being fixed to.** Every #graphGroupEdits entry runs against the
        whole group, for each name in #getModsToFix -- this is where a graph is renamed, split,
        remapped, or has its resources collected
     #. **Render and write.** Each group becomes one ``.ini`` file's worth of text, wrapped in the
        ``.ini`` file's own boilerplate and (optionally) appended to its original content. Group
        ``0`` overwrites the source file; later groups are written beside it as ``RemapFixCopy``
        files. See #fixImpl

     :raw-html:`<br />`

     .. note::
        Divergences from the pure-Python original, all deliberate:

        * **The initial groups come from the** :cpp:type:`BaseIniFixer::ParseData` **handed to**
          #fixImpl, not from reaching back into the parser's ``commandGraphs``/
          ``downloadResourceGraphs``. That data *is* those two, already collected into the one
          group shape the original built by hand -- see
          :cpp:func:`GIMIParser::collectParseResult`
        * **Every graph is deep-copied**, where the original deep-copies only the command graphs
          and shares the download-resource ones. Sharing half of them means an edit to a download
          graph silently writes through to the parser, which no caller wants and the original's own
          comment doesn't claim
        * **#getFix hands back a destination per group** rather than a
          ``Dict[Union[str, int], IniGraphGroup]``. The original keys by file path, falling back to
          an integer id when the ``.ini`` file has no path -- but that id is only ever the group's
          own index, so this returns the paths positionally and drops the union
        * **``hideOrig`` saves the ``.ini`` file's own text.** The original saves ``self._fileTxt``,
          which no fixer ever sets -- so passing ``hideOrig = True`` raises ``AttributeError``
          before it can restore anything. Reported rather than reproduced
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     * @tparam FixerBase
     @rst
     Which :cpp:class:`BaseIniFixer` specialization to derive from -- a parameter for exactly the
     reason :cpp:class:`GIMIParser`'s own ``ParserBase`` is one; see that parameter's own
     documentation
     @endrst
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>,
               typename FixerBase = BaseIniFixer<K, V, KeyHash, KeyEqual>>
    class GIMIFixer: public FixerBase {
        public:
            static_assert(std::is_base_of_v<BaseIniFixer<K, V, KeyHash, KeyEqual>, FixerBase>,
                           "GIMIFixer's FixerBase must derive from BaseIniFixer<K, V, KeyHash, KeyEqual>");

            using Base = FixerBase;
            using Core = BaseIniFixer<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniFixer::ParseData
             */
            using ParseData = typename Core::ParseData;

            /**
             * @copydoc BaseIniFixer::FixResult
             */
            using FixResult = typename Core::FixResult;

            /**
             * @brief The type of group this fixer edits
             */
            using GraphGroup = IniGraphGroup<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The type of graph inside a group
             */
            using Graph = IniSectionGraph<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The type of `section`_ inside a graph
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc IniGraphGroup::ModObj
             */
            using ModObj = std::pair<std::string, std::string>;

            /**
             * @copydoc IniGraphGroup::ModObjHash
             */
            using ModObjHash = typename IniGraphGroup<K, V, KeyHash, KeyEqual>::ModObjHash;

            /**
             * @brief The kind of edit #graphGroupEdits holds
             */
            using GroupEdit = BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ``.ini`` file this fixes
             */
            using Context = IniFixContext<K, V, KeyHash, KeyEqual>;

            /**
             * @brief Where this fixer's own groups live -- owned, see :cpp:func:`IniFixContext::makeGraphGroups`
             */
            using GraphGroups = IIniGraphGroups<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             Where each finished group is written, one entry per group in group order --
             ``std::nullopt`` for a group whose ``.ini`` file has no path at all. See this class's
             own note on why this isn't a ``Dict[Union[str, int], ...]``
             @endrst
             */
            using FixTargets = std::vector<std::optional<std::string>>;

            /**
             * @brief
             @rst
             Renders one `section`_ to text -- the same customization point
             :cpp:func:`IniSectionGraph::toStr` takes, and required here for the same reason:
             neither :cpp:class:`IfTemplate` nor :cpp:class:`IfContentPart` has a core ``toStr``,
             so what a `section`_ *looks like* is the caller's business
             @endrst
             */
            using SectionToStr = std::function<std::string(Section&, const std::string&, bool)>;

            /**
             * @brief
             @rst
             The ``.ini``-domain customization points this class needs -- see #SectionToStr
             @endrst
             */
            struct FixerConfig {
                /**
                 * @brief How to render one `section`_. #groupToStr renders nothing when this is empty
                 */
                SectionToStr sectionToStr;
            };

            /**
             * @brief Constructs a new fixer
             *
             * @param parser The associated parser to retrieve data for the fix -- **borrowed**, may be ``nullptr``
             * @param ctx The ``.ini`` file being fixed -- **borrowed**, and must outlive this fixer
             * @param graphGroupEdits The edits to apply to the parsed graphs -- **borrowed**. **Default**: empty
             * @param modsToFix
             @rst
             The mods to fix to, or ``std::nullopt`` to ask the ``.ini`` file
             (:cpp:func:`IniFixContext::modsToFix`). **Default**: ``std::nullopt``
             @endrst
             * @param prevFixer
             @rst
             A fixer whose already-edited groups this one continues from instead of starting fresh
             -- **borrowed**, nullable. **Default**: ``nullptr``
             @endrst
             * @param config The .ini-domain customization points to use. **Default**: an empty #FixerConfig
             */
            explicit GIMIFixer(typename Core::Parser* parser, Context* ctx,
                                std::vector<GroupEdit*> graphGroupEdits = {},
                                std::optional<std::vector<std::string>> modsToFix = std::nullopt,
                                GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>* prevFixer = nullptr,
                                FixerConfig config = {});

            /**
             * @brief
             @rst
             The edits applied to the parsed graphs, in order, once per name in #getModsToFix --
             borrowed, not owned
             @endrst
             */
            std::vector<GroupEdit*> graphGroupEdits;

            /**
             * @brief The mods to fix to, or ``std::nullopt`` to ask the ``.ini`` file -- see #getModsToFix
             */
            std::optional<std::vector<std::string>> modsToFix;

            /**
             * @brief
             @rst
             What every generated ``.ini`` **copy** is prefixed with :raw-html:`<br />`
             :raw-html:`<br />`

             A fix that merges several mod objects onto one emits an extra ``.ini`` file per extra
             object (see :cpp:func:`IniFileFixContext::fixedFilePath` for how they are named). Each
             of those is a file the user never asked for and did not write, so each one opens by
             saying what it is :raw-html:`<br />` :raw-html:`<br />`

             Only the **copies** get it. Group 0 is the mod's own ``.ini`` file, which needs no
             explanation of why it exists :raw-html:`<br />` :raw-html:`<br />`

             **Default**: empty -- no preamble at all. A generated file that says nothing about
             itself is the quieter default, and a caller that wants the explanation opts in.
             :cpp:member:`IniComments::GIMIObjMergerPreamble` is the one written for this purpose
             @endrst
             */
            std::string copyPreamble;

            /**
             * @brief
             @rst
             Mod objects whose **original** `sections`_ are commented out of the source text this
             fix appends :raw-html:`<br />` :raw-html:`<br />`

             Every fix that runs with ``withSrc`` opens with the ``.ini`` file's own content, so the
             original mod keeps working alongside the remap. That is usually what you want, but not
             always: when the fix *replaces* what a mod object drew rather than adding beside it,
             leaving the original in means the game draws both. Naming that mod object here
             comments its `sections`_ out with :cpp:member:`IniKeywords::HideOriginalComment`, so
             the appended source still explains what was there without it still being live
             :raw-html:`<br />` :raw-html:`<br />`

             Raiden's 6.1 fix is the case this exists for: it rewrites ``head``/``body``/``dress``
             in place, so the original three have to go :raw-html:`<br />` :raw-html:`<br />`

             :raw-html:`<br />`

             .. note::
                This is **independent of the ``hideOrig`` flag** #fixImpl is handed. That flag is
                the *user's* "hide the original mod entirely" request and hides everything
                #touchedSectionNames covers; this is the *fixer's* own "these particular objects
                cannot survive my fix", and applies whether or not the user asked for anything.
                When both are in play the two sets are hidden together

             .. note::
                Unlike #touchedSectionNames, nothing is filtered out of this set -- no
                :cpp:member:`IniGraphModObjKeywords::Download` check, no
                :cpp:member:`IniKeywords::Remap` check. Those exist to stop an *inferred* set from
                hiding something it shouldn't; this set is written out by hand, so it is taken at
                its word

             **Default**: empty -- hide nothing beyond what ``hideOrig`` asks for
             @endrst
             */
            std::unordered_set<ModObj, ModObjHash> hiddenModObjs;

            /**
             * @brief
             @rst
             A fixer whose already-edited groups this one continues from -- borrowed, ``nullptr``
             for none :raw-html:`<br />` :raw-html:`<br />`

             When set, #getFix runs *that* fixer's edit pass first and takes over its groups
             outright (a pointer move, not a copy), leaving it empty -- the direct equivalent of
             the original's ``self.prevFixer.getFix(onlyEditObjGraphs = True)`` /
             ``self.prevFixer.clear()`` pair
             @endrst
             */
            GIMIFixer<K, V, KeyHash, KeyEqual, FixerBase>* prevFixer;

            /**
             * @brief The ``.ini`` file being fixed -- borrowed, see the constructor
             */
            Context* ctx() const;

            /**
             * @copydoc ctx() const
             */
            void setCtx(Context* ctx);

            /**
             * @brief
             @rst
             This fixer's own groups -- ``nullptr`` until the first #getFix builds them
             :raw-html:`<br />` :raw-html:`<br />`

             The equivalent of the pure-Python original's ``graphGroups`` list
             @endrst
             */
            GraphGroups* graphGroups() const;

            /**
             * @brief The .ini-domain customization points this instance uses
             */
            const FixerConfig& config() const;

            /**
             * @brief
             @rst
             Where each group of the last #fixImpl was written, one entry per group -- see
             #FixTargets. Empty until a fix has run
             @endrst
             */
            const FixTargets& fixTargets() const;

            /**
             * @brief
             @rst
             The finished text of each group of the last #fixImpl, in group order, *before* the
             boilerplate and the original content were added :raw-html:`<br />` :raw-html:`<br />`

             Kept alongside #fixTargets so a caller can still reach the content of a group whose
             ``.ini`` file had no path -- :cpp:type:`BaseIniFixer::FixResult` is keyed by path and
             necessarily drops those
             @endrst
             */
            const std::vector<std::string>& fixedContents() const;

            /**
             * @brief
             @rst
             The mods to fix to -- #modsToFix when it was set explicitly, otherwise whatever the
             ``.ini`` file says (:cpp:func:`IniFixContext::modsToFix`)
             @endrst
             */
            std::vector<std::string> getModsToFix() const;

            /**
             * @brief Drops this fixer's groups, matching the pure-Python original's ``self.graphGroups = []``
             */
            void clear() override;

            /**
             * @brief
             @rst
             Builds this fixer's #graphGroups from 'parseData' (or takes over #prevFixer's), then
             runs every #graphGroupEdits entry over them once per mod being fixed to
             @endrst
             *
             * @param parseData The parse data to fix from -- see :cpp:type:`BaseIniFixer::ParseData`
             * @param onlyEditObjGraphs
             @rst
             Whether to stop after editing, without working out where anything gets written.
             Returns an empty #FixTargets in that case, and the results are left on #graphGroups --
             what a #prevFixer is run with. **Default**: ``false``
             @endrst
             *
             * @return Where each group should be written -- see #FixTargets
             */
            virtual FixTargets getFix(ParseData& parseData, bool onlyEditObjGraphs = false);

            /**
             * @brief
             @rst
             Renders the group at 'groupInd' to text -- every graph in it, joined by blank lines
             :raw-html:`<br />` :raw-html:`<br />`

             Returns an empty string when #config has no :cpp:type:`SectionToStr`, since there is
             then no way to turn a `section`_ into text at all
             @endrst
             *
             * @param groupInd Which group to render
             */
            std::string groupToStr(std::size_t groupInd) const;

            /**
             * @brief
             @rst
             The key one group's fix is filed under in the result of :cpp:func:`BaseIniFixer::fix`
             :raw-html:`<br />` :raw-html:`<br />`

             **Choosing this is the fixer's job, not the ``.ini`` file's.** Where a fix gets
             *written* and what it is *keyed by* are two different questions:
             :cpp:func:`IniFixContext::fixedFilePath` answers the first and legitimately has no
             answer for an ``.ini`` file built from raw text, but the second always has one. This
             used to conflate them, so a text-only ``.ini`` file silently fixed to an empty map --
             the fix was produced and then dropped for want of a key :raw-html:`<br />`
             :raw-html:`<br />`

             The default is the fixed file path when there is one, and the group index otherwise --
             unique per group either way. Override to key by something else
             @endrst
             *
             * @param groupInd Which group is being filed
             * @param fixedFilePath
             @rst
             Where that group will be written, or ``std::nullopt`` when there is nowhere -- see
             :cpp:func:`IniFixContext::fixedFilePath`
             @endrst
             *
             * @return The key to file this group's fix under -- never empty
             */
            /**
             * @brief
             @rst
             Wraps one group's rendered `sections`_ in a heading naming the mod this fixer remaps
             onto -- ``; ***** JeanCN *****`` ... ``; ******************`` :raw-html:`<br />`
             :raw-html:`<br />`

             Only used when :cpp:member:`IniFixingContext::labelTargets` says the ``.ini`` file has
             more than one target to tell apart. See
             :cpp:member:`IniBoilerPlate::DefaultModHeadingSideLen`
             @endrst
             *
             * @param content The rendered group, returned unchanged when there is no target to name
             *
             * @return The labelled block
             */
            std::string labelTargetBlock(const std::string& content) const;

            virtual std::string fixKey(std::size_t groupInd,
                                        const std::optional<std::string>& fixedFilePath) const;

            /**
             * @brief
             @rst
             The names of every `section`_ this fix touched -- what
             :cpp:func:`IniFixContext::hideOriginalSections` is handed when ``hideOrig`` is on
             :raw-html:`<br />` :raw-html:`<br />`

             The `sections`_ of every command graph in every group, which is exactly the set the
             fix rewrites: comment those out and the original mod stops being displayed, leaving
             only the remap. Empty until #graphGroups has been built

             :raw-html:`<br />`

             .. note::
                Two kinds of graph are left out:

                * the download-resource ones. Their `sections`_ are the resources the mod objects
                  *point at* rather than the commands the fix replaces -- and a graph keyed under
                  :cpp:member:`IniGraphModObjKeywords::Download` also holds `sections`_ this
                  software synthesized, which the original ``.ini`` file has no line of to comment
                  out in the first place
                * any whose mod object carries :cpp:member:`IniKeywords::Remap` in *either* half of
                  its ``(component, object)`` name, matched anywhere in it and case-insensitively.
                  That is the same convention
                  :cpp:func:`GIMIParser::classifyByTextureOverrideName` uses to refuse to classify
                  a `section`_ this software already wrote: such a mod object is the *output* of a
                  remap, not part of the original mod, so hiding it would hide the fix

             .. note::
                The pure-Python original collects the same thing while it renders, straight into
                the ``.ini`` file's own ``_remappedSectionNames`` -- and only for the *blend*
                command graph, because that was the only chain it ever rewrote. Reading it off the
                built groups instead keeps the ``.ini`` file out of it and covers every mod object
                this fixer actually fixed
             @endrst
             */
            std::unordered_set<std::string> touchedSectionNames() const;

            /**
             * @brief
             @rst
             The names of every `section`_ belonging to a mod object in #hiddenModObjs -- the
             second half of what :cpp:func:`IniFixContext::hideOriginalSections` is handed
             :raw-html:`<br />` :raw-html:`<br />`

             Read off the built groups, so these are the `sections`_ as the fix left them. A mod
             object named in #hiddenModObjs that no group actually has a graph for contributes
             nothing rather than raising -- naming an object this ``.ini`` file happens not to
             contain is a normal thing for a per-mod-type table to do. Empty until #graphGroups has
             been built
             @endrst
             */
            std::unordered_set<std::string> hiddenSectionNames() const;

        protected:

            /**
             * @brief
             @rst
             Runs every #graphGroupEdits entry over #graphGroups, once for 'modName'
             :raw-html:`<br />` :raw-html:`<br />`

             ``ini``/``modType`` are passed as ``nullptr`` -- there is nothing castable to hand
             over, the same situation every ``graphGroupEdits/`` caller is in. The `pybind11`_
             layer overrides this and passes the real `Python`_ objects instead
             @endrst
             *
             * @param modName The name of the mod being fixed to
             */
            virtual void applyGraphGroupEdits(const std::string& modName);

            FixResult fixImpl(ParseData& parseData, bool keepBackup, bool fixOnly, bool hideOrig,
                               bool withBoilerPlate, bool withSrc, IniFixingContext fixingCtx) override;

            /**
             * @brief The ``.ini`` file being fixed -- borrowed, see the constructor
             */
            Context* ctx_;

            /**
             * @brief This fixer's own groups -- owned, see #graphGroups
             */
            std::unique_ptr<GraphGroups> graphGroups_;

            /**
             * @brief
             @rst
             Every graph's `section`_ names **as they were before any** #graphGroupEdits **ran**,
             one entry per group :raw-html:`<br />` :raw-html:`<br />`

             Both #touchedSectionNames and #hiddenSectionNames read this rather than the live
             graphs, and must: hiding comments out `sections`_ **in the original ``.ini`` file's own
             text**, so it needs the names that text actually contains. An edit that renames
             `sections`_ (:cpp:class:`GraphRename`, which is how a remapped `section`_ gets its
             name at all) leaves the live graphs holding names the original file has no line of --
             hiding on those would comment out nothing and leave the original mod live *alongside*
             the fix
             @endrst
             */
            std::vector<tsl::ordered_map<ModObj, std::vector<std::string>, ModObjHash>> preEditSectionNames_;

            /**
             * @brief Where each group of the last #fixImpl was written -- see #fixTargets
             */
            FixTargets fixTargets_;

            /**
             * @brief The finished text of each group of the last #fixImpl -- see #fixedContents
             */
            std::vector<std::string> fixedContents_;

        private:
            FixerConfig config_;
    };
}

#include "GIMIFixer.tpp"

#endif
