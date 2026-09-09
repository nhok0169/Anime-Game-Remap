#ifndef AGRemapCore_StrategyOverrides_H
#define AGRemapCore_StrategyOverrides_H

#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     A process-wide table of parser/fixer factories that take precedence over the built-in ones
     :raw-html:`<br />` :raw-html:`<br />`

     **What this is for.** Every `IniParser`_/`IniFixer`_ this library ships is compiled into
     `AGRemapCore`, which is what makes a run fast and what makes trying a new idea slow -- a change
     to one character's fix is a C++ rebuild. Registering a factory here overrides the built-in row
     for a mod **at runtime**, so a new parse or fix can be written in `Python`_, run, and thrown
     away without rebuilding anything. It exists for that prototyping loop; a fix worth keeping
     belongs in ``data/IniFixData/<Name>/``.

     **Matching mirrors the built-in tables** (:cpp:class:`ModDictAssets`): a request carrying a
     version takes the highest override at or below it, and a request carrying none means "the
     newest", taking the highest override registered. An override registered *without* a version is
     the fallback, used only when no versioned one applied.

     Exact matching was tried first and is wrong for what this class is for. A run resolves a mod's
     version off the ``.ini`` and normally passes **no** version at all, so an override registered
     for ``6.1`` never fired on an ordinary run -- which is the case "override Raiden 6.1" means.

     .. warning::
        Not synchronised. Register and clear **around** a run, never during one -- a mutex on
        :cpp:func:`findParser` would sit in the path of every ``.ini`` file for the benefit of a
        prototyping aid.
     @endrst
     */
    class StrategyOverrides {
        public:
            /**
             * @brief The factory an override supplies for a parser -- see :cpp:type:`IniParseBuilder::Factory`
             */
            using ParseFactory = IniParseBuilder::Factory;

            /**
             * @brief The factory an override supplies for a fixer -- see :cpp:type:`IniFixBuilder::Factory`
             */
            using FixFactory = IniFixBuilder::Factory;

            /**
             * @brief Registers a parser factory for 'modName'
             *
             * @param modName The mod type the override applies to, eg. ``"Raiden"``
             * @param version
             @rst
             The exact version to override, or ``std::nullopt`` for every version :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param factory What to build the parser with
             */
            static void setParser(std::string modName, std::optional<Version> version, ParseFactory factory);

            /**
             * @brief Registers a fixer factory for the ``fromModName`` -> ``toModName`` remap
             *
             * @param fromModName The mod being fixed
             * @param toModName The mod being fixed *to*
             * @param fromVersion
             @rst
             The exact version of ``fromModName`` to override, or ``std::nullopt`` for every version
             :raw-html:`<br />` :raw-html:`<br />`

             Only the *from* version is keyed on -- that is the one a run resolves off the ``.ini``
             file being fixed :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param factory What to build the fixer with
             */
            static void setFixer(std::string fromModName, std::string toModName,
                                  std::optional<Version> fromVersion, FixFactory factory);

            /**
             * @brief The parser factory registered for 'modName' at 'version', if any
             */
            static std::optional<ParseFactory> findParser(const std::string& modName,
                                                           const std::optional<Version>& version);

            /**
             * @brief The fixer factory registered for the 'fromModName' -> 'toModName' remap, if any
             */
            static std::optional<FixFactory> findFixer(const std::string& fromModName,
                                                        const std::string& toModName,
                                                        const std::optional<Version>& fromVersion);

            /**
             * @brief
             @rst
             Every ``toModName`` registered for 'fromModName' :raw-html:`<br />` :raw-html:`<br />`

             :cpp:func:`IniFixBuilder::buildAll` uses this to surface a target the built-in table has
             no row for at all, which is what lets a brand-new remap be prototyped rather than only
             an existing one replaced
             @endrst
             */
            static std::vector<std::string> fixerTargets(const std::string& fromModName);

            /**
             * @brief Removes every registered override
             */
            static void clear();

            /**
             * @brief
             @rst
             Whether nothing is registered :raw-html:`<br />` :raw-html:`<br />`

             Checked first by both builders so an ordinary run pays one empty-container test rather
             than a lookup
             @endrst
             */
            static bool empty();
    };
}

#endif
