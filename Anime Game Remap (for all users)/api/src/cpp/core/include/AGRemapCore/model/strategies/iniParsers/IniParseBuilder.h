#ifndef AGRemapCore_IniParseBuilder_H
#define AGRemapCore_IniParseBuilder_H

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

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModDictAssets.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     A factory that builds the :cpp:class:`BaseIniParser` for one ``.ini`` file, optionally picking
     *which* parser (and with which arguments) based on the mod's name and the game version the
     ``.ini`` file came from :raw-html:`<br />` :raw-html:`<br />`

     What :cpp:member:`ModType::iniParseBuilder` holds. It comes in two flavours:

     * **Fixed** -- one #Factory used for every ``.ini`` file, whatever its version
     * **Version-dependent** -- an #ArgsRepo looked up by ``(modName, version)`` on every #build,
       so a 5.7-era ``.ini`` file of some mod gets a different parser (or a differently-configured
       one) than a 4.0-era one

     :raw-html:`<br />`

     .. note::
        A #Factory is a closure: a "constructor with some of its arguments already chosen". A row
        is written as a lambda returning, for example, ``std::make_shared<SomeParser>(iniFile, ...)``,
        with its arguments captured, and is type-checked at compile time

     .. note::
        Whatever a row's lambda **captures** is constructed once and shared across every build,
        while the parser itself is freshly constructed per call. Nothing is memoized: the lookup is
        :cpp:func:`ModDictAssets::get` (a hash lookup plus a binary search) and the #Factory it finds
        is already built
     @endrst
     */
    class IniParseBuilder {
        public:

            /**
             * @brief
             @rst
             Builds one parser, already bound to the ``.ini`` file it will read :raw-html:`<br />`
             :raw-html:`<br />`

             See this class's own note. The :cpp:class:`IniFile` argument is the file the parser
             will read; it may be ``nullptr``, since :cpp:class:`BaseIniParser` allows an unbound
             parser. The ``std::optional<int>`` argument is the id of the mod type the parser is
             built for -- see #build's 'modTypeId'
             @endrst
             */
            using Factory = std::function<std::shared_ptr<BaseIniParser<>>(IniFile*, std::optional<int>)>;

            /**
             * @brief
             @rst
             The version-dependent lookup table a #build consults :raw-html:`<br />` :raw-html:`<br />`

             Two index columns, ``["version", "name"]``: the game
             version at position ``0``, and the mod's name at position ``1``. A version resolves by
             inclusive floor-match (see :cpp:func:`ModDictAssets::get`), which is what makes "the
             4.0 row keeps applying until a 5.7 row supersedes it" work :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                :cpp:class:`ModDictAssets` rather than :cpp:class:`ModAssets` -- this table has exactly one version
                column, which is the case :cpp:class:`ModDictAssets`'s own class note calls out as
                belonging in the hash-based table rather than the linear-scanning one
             @endrst
             */
            using ArgsRepo = ModDictAssets<std::string, Factory>;

            /**
             * @brief
             @rst
             The #Factory used when nothing else supplies one -- constructs a default-configured
             :cpp:class:`GIMIParser` reading the given file through its own
             :cpp:class:`IniFileParseContext` :raw-html:`<br />` :raw-html:`<br />`

             Every fallback path in this class goes through this one function
             @endrst
             */
            static Factory defaultFactory();

            /**
             * @brief Constructs a builder that always builds a default :cpp:class:`GIMIParser` --
             *      see #defaultFactory
             */
            IniParseBuilder();

            /**
             * @brief Constructs a builder that always uses the same factory, whatever the version
             *
             * @param factory
             @rst
             The factory to build every parser with. If this is empty, #defaultFactory is used
             instead
             @endrst
             */
            explicit IniParseBuilder(Factory factory);

            /**
             * @brief Constructs a builder that picks its factory by mod name and game version
             *
             * @param builderArgs
             @rst
             The lookup table to resolve a factory from -- see #ArgsRepo :raw-html:`<br />`
             :raw-html:`<br />`

             Held by ``shared_ptr`` because one table is shared by every :cpp:class:`ModType` of a
             game (every GI mod type shares a single one) :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, the builder degrades to the #defaultFactory-only behaviour of
             the default constructor
             @endrst
             * @param errorOnNotFound
             @rst
             What #build does when 'builderArgs' holds no row for the mod name it was asked about
             :raw-html:`<br />` :raw-html:`<br />`

             * ``false`` (the default) -- fall back to #defaultFactory, so one unlisted mod type
               degrades to a default parser rather than aborting the run. This matches how the rest
               of this pipeline handles a missing strategy (:cpp:func:`IniFile::parse` skips a mod
               type with no parser, :cpp:func:`IniFile::fix` skips one with no fixer)
             * ``true`` -- let :cpp:func:`ModDictAssets::get`'s ``std::out_of_range`` propagate

             :raw-html:`<br />`

             .. note::
                This only covers a mod name with **no row at any version**. A mod name that has a
                row at some older version always resolves (to that older row) for every later
                version, by :cpp:class:`ModDictAssets`'s floor-match -- a 4.0 row covers every
                version after 4.0 until a newer row supersedes it

             **Default**: ``false``
             @endrst
             */
            explicit IniParseBuilder(std::shared_ptr<const ArgsRepo> builderArgs, bool errorOnNotFound = false);

            /**
             * @brief
             @rst
             The lookup table this builder resolves factories from, or ``nullptr`` if it is a
             fixed-factory builder
             @endrst
             */
            const std::shared_ptr<const ArgsRepo>& getBuilderArgs() const;

            /**
             * @brief Whether #build throws rather than falling back when the mod name has no row --
             *      see the constructor's 'errorOnNotFound' argument
             */
            bool getErrorOnNotFound() const;

            /**
             * @brief
             @rst
             Builds the parser for one ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

             A parser registered for 'modName'/'version' through :cpp:class:`StrategyOverrides`
             wins over everything else. Otherwise, for a fixed-factory builder, 'modName'/'version'
             are ignored; for a version-dependent one the pair is looked up in #getBuilderArgs
             @endrst
             *
             * @param iniFile The .ini file the built parser will read -- passed straight to the
             *      #Factory, and may be ``nullptr``
             * @param modName The name of the mod to build the parser for (:cpp:member:`ModType::name`)
             * @param version
             @rst
             The game version the ``.ini`` file originates from (:cpp:member:`IniFile::version`)
             :raw-html:`<br />` :raw-html:`<br />`

             If this is ``std::nullopt``, the latest listed version for 'modName' is used
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param modTypeId
             @rst
             Which of the ``.ini`` file's mod types the parser is being built for -- handed straight
             to the #Factory, and what lets #defaultFactory's context answer
             :cpp:func:`IniParseContext::modTypeName` and
             :cpp:func:`IniParseContext::hasModType` :raw-html:`<br />` :raw-html:`<br />`

             This is the id the mod type was **filed under**, which need not equal its own
             ``modTypeId`` -- see :cpp:func:`IniFile::getParser` :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``std::nullopt``, meaning the parser is built for no particular mod type
             @endrst
             *
             * @throws std::out_of_range If #getErrorOnNotFound is ``true`` and 'modName' has no row
             *      at any version
             *
             * @return The built parser -- never ``nullptr``
             */
            std::shared_ptr<BaseIniParser<>> build(IniFile* iniFile, const std::string& modName,
                                                  const std::optional<Version>& version = std::nullopt,
                                                  std::optional<int> modTypeId = std::nullopt) const;

        private:
            // Empty exactly when builderArgs_ is set -- the two flavours are mutually exclusive,
            // the same way the pure-Python original nulls out '_buildCls' once '_builderArgs' is
            // provided.
            Factory factory_;

            std::shared_ptr<const ArgsRepo> builderArgs_;

            bool errorOnNotFound_ = false;
    };
}

#endif
