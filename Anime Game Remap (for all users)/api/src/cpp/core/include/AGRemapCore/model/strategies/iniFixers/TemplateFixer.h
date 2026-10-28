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

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
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

        protected:
            /**
             * @brief
             @rst
             Runs every :cpp:member:`GIMIFixer::graphGroupEdits` entry over the groups with the real
             ``.ini`` file, in order :raw-html:`<br />` :raw-html:`<br />`

             Does nothing when there are no groups. Otherwise calls :cpp:func:`beforeGraphGroupEdits`,
             then each edit's ``editFromIni``, then -- when the fixer has given up and
             :cpp:member:`withdrawDownloadsOnGiveUp` is set -- clears the ``.ini`` file's registered
             downloads, then :cpp:func:`afterGraphGroupEdits`
             @endrst
             *
             * @param modName The name of the mod being fixed to
             */
            void applyGraphGroupEdits(const std::string& modName) override;

            /**
             * @brief What a subclass does before the edits run, at fix time. **Default**: nothing
             *
             * @param modName The name of the mod being fixed to
             */
            virtual void beforeGraphGroupEdits(const std::string& modName);

            /**
             * @brief What a subclass does after the edits have run, at fix time. **Default**: nothing
             *
             * @param modName The name of the mod being fixed to
             */
            virtual void afterGraphGroupEdits(const std::string& modName);

            /**
             * @brief
             @rst
             Gives up: the edit list becomes one :cpp:class:`GraphGroupRemove`, so the fixer writes
             nothing for the ``.ini`` file, and #hasGivenUp answers ``true``
             @endrst
             */
            void giveUp();

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
            bool gaveUp_ = false;
            GraphGroupRemove<std::string, std::string> removeEveryGroup_;
    };
}

#endif
