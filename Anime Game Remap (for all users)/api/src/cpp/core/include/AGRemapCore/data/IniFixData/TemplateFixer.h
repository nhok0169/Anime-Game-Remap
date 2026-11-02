#ifndef AGRemapCore_TemplateFixer_H
#define AGRemapCore_TemplateFixer_H

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

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/GraphEditVariables.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/VarSectionsTake.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCreate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The base of a plain-C++ fixer that fixes one :cpp:class:`AGRemapCore::IniFile` by a list of edits
     :raw-html:`<br />` :raw-html:`<br />`

     A subclass builds its edits in its constructor and puts them, in the order they run, in
     :cpp:member:`GIMIFixer::graphGroupEdits`. The sections it writes of its own (sections that belong
     to no mod object) are built as `section`_\ s with :cpp:func:`makeSection` or
     :cpp:func:`renderSection` and handed over through :cpp:member:`GIMIFixer::appendedSections`.
     This class then supplies what every such fixer needs and none of them is about:

     * the fix context: an :cpp:class:`IniFileFixContext` over the parser's ``.ini`` file, owned here
       and installed as the fixer's context
     * the section renderer: :cpp:func:`renderIfTemplate`
     * applying the edits: :cpp:func:`applyGraphGroupEdits` hands every edit the REAL ``.ini`` file, so
       a :cpp:class:`ResRegCollect` can build the resources it collects (the :cpp:class:`GIMIFixer`
       default hands them ``nullptr``)
     * giving up: :cpp:func:`giveUp` replaces the edit list with one :cpp:class:`GraphGroupRemove`, so
       the fixer writes nothing for the ``.ini`` file rather than the mod's own sections again under
       their source names

     :raw-html:`<br />`

     .. note::
        `C++`_ only. A fixer built from `Python`_ gets the same behaviour from the bound
        :cpp:class:`GIMIFixer`, whose `pybind11`_ layer already hands its edits the real `Python`_
        ``IniFile``, so there is nothing of this class for such a fixer to reach
     @endrst
     */
    class TemplateFixer: public GIMIFixer<std::string, std::string> {
        public:
            /**
             * @brief The fixer this derives from
             */
            using Fixer = GIMIFixer<std::string, std::string>;

            /**
             * @brief A section the fixer writes of its own
             */
            using Section = IfTemplate<std::string, std::string>;

            /**
             * @brief The lines of one section: ``(key, value)`` pairs, in order
             */
            using KVPs = std::vector<std::pair<std::string, std::string>>;

            /**
             * @brief
             @rst
             Which of :cpp:class:`IniNamingTools`' naming conventions a fix renames a `section`_ by --
             each resource kind has its own (``…AmberAmberCNRemapIB``, not ``…AmberIBAmberCNRemapFix``),
             and a section that is no resource takes :cpp:func:`IniNamingTools::getRemapFixName`'s
             @endrst
             */
            enum class RemapName {
                /**
                 * @brief :cpp:func:`IniNamingTools::getRemapFixName`
                 */
                Fix,

                /**
                 * @brief :cpp:func:`IniNamingTools::getRemapBlendName`
                 */
                Blend,

                /**
                 * @brief :cpp:func:`IniNamingTools::getRemapPositionName`
                 */
                Position,

                /**
                 * @brief :cpp:func:`IniNamingTools::getRemapTexcoordName`
                 */
                Texcoord,

                /**
                 * @brief :cpp:func:`IniNamingTools::getRemapIbName`
                 */
                Ib
            };

            /**
             * @brief
             @rst
             What a renamed `section`_ is finally called, from its old name and the name
             :cpp:func:`remapName` gave it -- see :cpp:func:`remapRename`
             @endrst
             */
            using RenameWrap = std::function<std::string(const std::string& oldName, const std::string& renamed)>;

            /**
             * @brief The asset specs a :cpp:class:`RegAssetRemap` remaps, keyed by register
             */
            using AssetSpecs = std::vector<std::pair<std::string, RegAssetRemap<std::string, std::string>::AssetSpec>>;

            /**
             * @brief Builds the fixer of one .ini file
             *
             * @param parser
             @rst
             The parser that read the ``.ini`` file -- **borrowed**, nullable. Its
             :cpp:func:`BaseIniParser::getIniFile` is the file :cpp:member:`ctx_` wraps
             @endrst
             * @param toModName The mod being fixed to, or empty to fix to every mod the parser's mod type remaps to
             * @param modTypeId The mod type being fixed from, or ``std::nullopt`` for none
             */
            TemplateFixer(BaseIniParser<std::string, std::string>* parser, const std::string& toModName,
                          std::optional<int> modTypeId);

            TemplateFixer(const TemplateFixer&) = delete;
            TemplateFixer& operator=(const TemplateFixer&) = delete;

            /**
             * @brief Whether #giveUp has been called
             */
            bool hasGivenUp() const;

            /**
             * @brief
             @rst
             The resource naming every fixer's resource edits use: a resource's file is its ``filename``
             register, and a register value and a file path are the same string
             @endrst
             *
             * @param extras
             @rst
             Lines forced onto every resource section the edit writes -- see
             :cpp:member:`BaseResEdit::ResEditConfig::extraKVPs`. **Default**: none
             @endrst
             */
            static BaseResEdit<std::string, std::string>::ResEditConfig resEditConfig(KVPs extras = {});

            /**
             * @brief
             @rst
             How a section the fixer writes follows ``run =``: the value of a ``run`` key names the
             section it calls, unchanged
             @endrst
             */
            static IfTemplateRunConfig<std::string, std::string> sectionRunConfig();

            /**
             * @brief
             @rst
             One `section`_ of one content part holding 'kvps', named 'name' and following
             ``run =`` by :cpp:func:`sectionRunConfig`
             @endrst
             */
            static std::unique_ptr<Section> makeSection(const std::string& name, const KVPs& kvps);

            /**
             * @brief
             @rst
             The text of the `section`_ :cpp:func:`makeSection` builds, rendered by
             :cpp:func:`renderIfTemplate` and followed by one ``"\n"``
             @endrst
             */
            static std::string renderSection(const std::string& name, const KVPs& kvps);

            /**
             * @brief
             @rst
             Whether a ``run =`` value calls one of the fix libraries every remapped draw re-issues --
             :cpp:member:`IniKeywords::ORFixPath` or :cpp:member:`IniKeywords::NNFixPath` -- matched
             case-insensitively, as 3dmigoto matches a command list's path
             @endrst
             *
             * @param value The ``run =`` value
             */
            static bool isFixLibraryCall(const std::string& value);

            /**
             * @brief
             @rst
             The text of a `section`_ that withdraws a `TexFx`_ request for one draw the fix leaves
             to the game: ``$\TexFx\use_default_shader = -1`` on that draw's ``hash`` and
             ``match_first_index``
             @endrst
             *
             * @param name The `section`_'s name
             * @param hash The draw's ``hash``
             * @param firstIndex The draw's ``match_first_index``
             */
            static std::string texFxGuardSection(const std::string& name, const std::string& hash, const std::string& firstIndex);

        protected:
            /**
             * @brief What the parser read of the ``.ini`` file -- its :cpp:func:`BaseIniParser::parseData`
             */
            const IniParseData& parseData() const {
                return this->getParser()->parseData();
            }

            /**
             * @brief
             @rst
             Runs every :cpp:member:`GIMIFixer::graphGroupEdits` entry over the groups with the real
             ``.ini`` file, in order :raw-html:`<br />` :raw-html:`<br />`

             Does nothing when there are no groups. Otherwise runs each edit's ``editFromIni``, then
             adds :cpp:func:`addFixSection`'s sections to the groups, then -- when the fixer has given
             up and :cpp:member:`withdrawDownloadsOnGiveUp` is set -- clears the ``.ini`` file's
             registered downloads, then runs each :cpp:member:`fixSectionEdits` entry
             @endrst
             *
             * @param modName The name of the mod being fixed to
             */
            void applyGraphGroupEdits(const std::string& modName) override;

            /**
             * @brief
             @rst
             Gives up: the edit list becomes one :cpp:class:`GraphGroupRemove`, so the fixer writes
             nothing for the ``.ini`` file, and #hasGivenUp answers ``true``
             @endrst
             */
            void giveUp();

            /**
             * @brief #giveUp, saying why in the log
             *
             * @param logLine The line logged
             */
            void giveUp(const std::string& logLine);

            /**
             * @brief
             @rst
             Whether the ``.ini`` file calls `TexFx`_ anywhere -- a ``run =`` into
             :cpp:member:`IniKeywords::TexFxFolder`
             @endrst
             */
            bool iniCallsTexFx() const;

            /**
             * @brief The version the .ini file is fixed to, or ``std::nullopt`` when there is no .ini file or it names none
             */
            std::optional<Version> iniToVersion() const;

            /**
             * @brief
             @rst
             The name the fix gives a section of the mod's: :cpp:func:`IniNamingTools::getRemapFixName`
             of 'name' for :cpp:member:`toModName_`
             @endrst
             */
            std::string remapFixName(const std::string& name) const;

            /**
             * @brief The name the fix gives a section of the mod's, by the convention of 'kind'
             *
             * @param kind The naming convention
             * @param name The section's name
             */
            std::string remapName(RemapName kind, const std::string& name) const;

            /**
             * @brief
             @rst
             A :cpp:class:`GraphRename` to :cpp:func:`remapName`'s names of 'kind' -- or, with 'wrap',
             to what 'wrap' makes of the old name and that one
             @endrst
             *
             * @param kind The naming convention
             * @param wrap What a renamed section is finally called. **Default**: the renamed name
             */
            std::unique_ptr<GraphRename<std::string, std::string>> remapRename(RemapName kind, RenameWrap wrap = {}) const;

            /**
             * @brief
             @rst
             The :cpp:class:`RegAssetRemap` of a section's ``hash`` from 'fromName' onto
             :cpp:member:`toModName_`, through the fixer's own mod type's hashes, plus 'extraSpecs'
             @endrst
             *
             * @param fromName The mod type the hashes are reverse-looked-up under
             * @param toVersion The version the hashes are remapped to
             * @param notFound What a hash with no mapping becomes; ``std::nullopt`` leaves it as it is.
             *      **Default**: :cpp:member:`IniKeywords::HashNotFound`
             * @param extraSpecs More registers to remap the same way. **Default**: none
             */
            std::unique_ptr<RegAssetRemap<std::string, std::string>> hashRemap(
                const std::string& fromName, const std::optional<Version>& toVersion,
                const std::optional<std::string>& notFound = IniKeywords::HashNotFound, AssetSpecs extraSpecs = {}) const;

            /**
             * @brief
             @rst
             Adds a `section`_ the fix writes itself -- a resource it declares, a command list, a
             section on a draw the mod has none for -- to the ``.ini`` file the fix renders
             :raw-html:`<br />` :raw-html:`<br />`

             The sections go into the groups as ONE graph per group (see :cpp:class:`GraphCreate`),
             after every :cpp:member:`GIMIFixer::graphGroupEdits` entry has run and before
             :cpp:member:`fixSectionEdits`, so an edit run there reads and changes them as it does
             the mod's own. A section the group already renders with the same name and text is written
             once :raw-html:`<br />` :raw-html:`<br />`

             :cpp:func:`giveUp` drops the sections added before it, and a section added AFTER it is still
             written, in a group of its own -- which is how a file that only watches the source carries
             its watching sections and nothing else
             @endrst
             *
             * @param section The section
             * @param group The one group it goes into. **Default**: the first group, and every copy too
             *      when :cpp:member:`fixSectionsInCopies` is set
             */
            void addFixSection(std::unique_ptr<Section> section, std::optional<std::size_t> group = std::nullopt);

            /**
             * @brief
             @rst
             An edit for :cpp:member:`GIMIFixer::graphGroupEdits` that makes `sections`_ an earlier
             edit built in a graph into the fix's own: each named `section`_ of graph 'graph' goes to
             :cpp:func:`addFixSection`, and the graph is removed :raw-html:`<br />` :raw-html:`<br />`

             This is how a fix section is built from the mod's own sections by edits: copy them into a
             graph of their own (:cpp:class:`GraphCopy`), edit that graph, then take the result. The
             edit is owned by this fixer :raw-html:`<br />` :raw-html:`<br />`

             The sections taken are written after every section :cpp:func:`addFixSection` added before the
             edit runs -- or, with 'inPlace', where this call stands among them: after the sections added
             before it and before the ones added after it
             @endrst
             *
             * @param graph The graph, in the first group
             * @param sections The `sections`_ to take, in order. A name the graph does not hold is skipped
             * @param comment A comment written above each section taken. **Default**: none
             * @param inPlace Whether the sections keep this call's place among the fix's sections. **Default**: ``false``
             *
             * @return The edit
             */
            GroupEdit* takeFixSections(const ModObj& graph, std::vector<std::string> sections, const std::string& comment = "",
                                       bool inPlace = false);

            /**
             * @brief Whether a section :cpp:func:`addFixSection` adds with no group goes into every copy as well. **Default**: ``false``
             */
            bool fixSectionsInCopies = false;

            /**
             * @brief
             @rst
             The edits to run after :cpp:func:`addFixSection`'s sections are in the groups, in order
             -- **borrowed**, like :cpp:member:`GIMIFixer::graphGroupEdits` :raw-html:`<br />`
             :raw-html:`<br />`

             What it is for: an edit that reads or changes the fix's own sections, or that has to run
             on every group :cpp:member:`GIMIFixer::graphGroupEdits` made (see
             :cpp:class:`IterGraphGroupEdit`). :cpp:func:`giveUp` empties it
             @endrst
             */
            std::vector<GroupEdit*> fixSectionEdits;

            /**
             * @brief
             @rst
             The `Z3`_ context the conditions of :cpp:func:`addFixSection`'s sections were built in.
             **Borrowed**; ``nullptr`` (the default) for sections with no ``if``
             @endrst
             */
            Z3Context* fixSectionsZ3 = nullptr;

            /**
             * @brief The graph id :cpp:func:`addFixSection`'s sections are added under in group 'group'
             */
            static GraphRemove<std::string, std::string>::GraphId fixSectionsGraphId(std::size_t group);

            /**
             * @brief
             @rst
             Whether a fixer that gave up also clears the downloads the parser registered on the
             ``.ini`` file, so a fix that writes nothing fetches nothing either. **Default**: ``false``
             @endrst
             */
            bool withdrawDownloadsOnGiveUp = false;

            /**
             * @brief The fix context over the parser's .ini file, installed as this fixer's context
             */
            mutable IniFileFixContext ctx_;

            /**
             * @brief The mod being fixed to, as the constructor was given it
             */
            std::string toModName_;

        private:
            void addFixSectionGraphs(const std::string& modName);

            // The sections variable of the sections added with no group, and of the ones for group 'group'
            static std::string fixSectionsVariable();
            static std::string fixSectionsVariable(std::size_t group);

            // the sections the fix writes of its own, as variables, and the edits that take and write them
            GraphEditVariables fixVariables_;
            std::vector<std::unique_ptr<VarSectionsTake>> takeEdits_;
            std::unique_ptr<GraphCreate<std::string, std::string>> createEdit_;
            bool gaveUp_ = false;
            GraphGroupRemove<std::string, std::string> removeEveryGroup_;
    };
}

#endif
