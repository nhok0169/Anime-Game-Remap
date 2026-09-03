#ifndef AGRemapCore_IniParseBuilder_H
#define AGRemapCore_IniParseBuilder_H

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

     The C++ counterpart to the pure-Python ``IniParseBuilder``
     (``model/strategies/iniParsers/IniParseBuilder.py``), and what
     :cpp:member:`ModType::iniParseBuilder` holds. It comes in the same two flavours the original
     does:

     * **Fixed** -- one #Factory used for every ``.ini`` file, whatever its version. The equivalent
       of the original's ``IniParseBuilder(GIMIParser)``
     * **Version-dependent** -- an #ArgsRepo looked up by ``(modName, version)`` on every #build,
       so a 5.7-era ``.ini`` file of some mod gets a different parser (or a differently-configured
       one) than a 4.0-era one. The equivalent of the original's
       ``IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value)``

     :raw-html:`<br />`

     .. note::
        The pure-Python original stores a ``(cls, args, kwargs)`` triple and splats it at build
        time, because Python has no other way to carry "a constructor with some of its arguments
        already chosen". C++ does -- a closure -- so the whole triple collapses into the single
        #Factory callable, with the arguments captured rather than stored alongside a class object.
        A row the original writes as ``(GIMIObjParser, [{"head", "body"}], {...})`` is written here
        as a lambda returning ``std::make_shared<GIMIObjParser>(iniFile, ...)``, and is type-checked
        at compile time instead of at build time

     .. note::
        There is deliberately no equivalent of the original's ``@lru_cache(maxsize = 64)`` on
        ``_getBuilderArgs``. That cache exists to avoid re-running an argument-*generator* function
        against a linear-scanning lookup table; here the lookup is :cpp:func:`ModDictAssets::get`
        (a hash lookup plus a binary search) and the #Factory it finds is already built, so there
        is nothing left to memoize. The observable behaviour still matches: whatever a row's lambda
        **captures** is constructed once and shared across every build, exactly as the
        ``lru_cache``\\d triple's argument objects are, while the parser itself is freshly
        constructed per call
     @endrst
     */
    class IniParseBuilder {
        public:

            /**
             * @brief
             @rst
             Builds one parser, already bound to the ``.ini`` file it will read :raw-html:`<br />`
             :raw-html:`<br />`

             The C++ stand-in for the pure-Python original's ``(cls, args, kwargs)`` triple -- see
             this class's own note on why that collapses into a single callable here. The
             :cpp:class:`IniFile` argument is what the original's ``Builder.build(iniFile)`` passes
             positionally to the parser's constructor; it may be ``nullptr``, since
             :cpp:class:`BaseIniParser` allows an unbound parser
             @endrst
             */
            using Factory = std::function<std::shared_ptr<BaseIniParser<>>(IniFile*, std::optional<int>)>;

            /**
             * @brief
             @rst
             The version-dependent lookup table a #build consults -- the C++ counterpart to the
             pure-Python ``IniParseBuilderArgs`` (``model/assets/IniParseBuilderArgs.py``)
             :raw-html:`<br />` :raw-html:`<br />`

             Two index columns, matching that original's own ``["version", "name"]``: the game
             version at position ``0``, and the mod's name at position ``1``. A version resolves by
             inclusive floor-match (see :cpp:func:`ModDictAssets::get`), which is what makes "the
             4.0 row keeps applying until a 5.7 row supersedes it" work :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                :cpp:class:`ModDictAssets` rather than :cpp:class:`ModAssets` (which is what the
                pure-Python ``IniParseBuilderArgs`` inherits) -- this table has exactly one version
                column, which is the case :cpp:class:`ModDictAssets`'s own class note calls out as
                belonging in the hash-based table rather than the linear-scanning one
             @endrst
             */
            using ArgsRepo = ModDictAssets<std::string, Factory>;

            /**
             * @brief
             @rst
             The #Factory used when nothing else supplies one -- constructs a plain
             :cpp:class:`BaseIniParser` bound to the given file :raw-html:`<br />` :raw-html:`<br />`

             Stands in for the pure-Python original's ``IniParseBuilder(GIMIParser)`` default. It is
             the *base* class rather than a ``GIMIParser`` simply because no concrete C++ parser has
             been ported yet -- change this one function when one lands, and every fallback path
             picks it up at once
             @endrst
             */
            static Factory defaultFactory();

            /**
             * @brief Constructs a builder that always builds a plain :cpp:class:`BaseIniParser` --
             *      see #defaultFactory
             */
            IniParseBuilder();

            /**
             * @brief Constructs a builder that always uses the same factory, whatever the version
             *
             * @param factory
             @rst
             The factory to build every parser with. If this is empty, #defaultFactory is used
             instead -- mirroring the pure-Python original's own
             ``if (iniParseBuilder is None): iniParseBuilder = IniParseBuilder(GIMIParser)``
             fallback
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
             game (all 43 GI mod types share a single one, exactly as the pure-Python original's 43
             ``IniParseBuilder(ModDataAssets.IniParseBuilderArgs.value)`` calls share a single
             ``IniParseBuilderArgs`` instance) :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, the builder degrades to the #defaultFactory-only behaviour of
             the default constructor
             @endrst
             * @param errorOnNotFound
             @rst
             What #build does when 'builderArgs' holds no row for the mod name it was asked about
             :raw-html:`<br />` :raw-html:`<br />`

             * ``false`` (the default) -- fall back to #defaultFactory, so one unlisted mod type
               degrades to a plain parser rather than aborting the run. This matches how the rest
               of this pipeline handles a missing strategy (:cpp:func:`IniFile::parse` skips a mod
               type with no parser, :cpp:func:`IniFile::fix` skips one with no fixer) rather than
               the pure-Python original, whose ``ModAssets.get`` defaults to
               ``errorOnNotFound = True`` and raises
             * ``true`` -- let :cpp:func:`ModDictAssets::get`'s ``std::out_of_range`` propagate,
               matching the pure-Python original exactly

             :raw-html:`<br />`

             .. note::
                This only covers a mod name with **no row at any version**. A mod name that has a
                row at some older version always resolves (to that older row) for every later
                version, by :cpp:class:`ModDictAssets`'s floor-match -- the same way the
                pure-Python table's 4.0 entries cover every version after 4.0

             **Default**: ``false``
             @endrst
             */
            explicit IniParseBuilder(std::shared_ptr<const ArgsRepo> builderArgs, bool errorOnNotFound = false);

            /**
             * @brief
             @rst
             The lookup table this builder resolves factories from, or ``nullptr`` if it is a
             fixed-factory builder -- the equivalent of the pure-Python original's ``_builderArgs``
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

             For a fixed-factory builder, 'modName'/'version' are ignored entirely -- matching the
             pure-Python original's own "this argument has no effect if ``_buildCls`` is not
             ``None``" warning. Otherwise the pair is looked up in #getBuilderArgs
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
