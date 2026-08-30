#ifndef AGRemapCore_IniFixBuilder_H
#define AGRemapCore_IniFixBuilder_H

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModAssets.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A factory that builds the :cpp:class:`BaseIniFixer` for one ``.ini`` file, optionally picking
     *which* fixer (and with which arguments) based on the mod's name and the game version the
     ``.ini`` file came from :raw-html:`<br />` :raw-html:`<br />`

     The C++ counterpart to the pure-Python ``IniFixBuilder``
     (``model/strategies/iniFixers/IniFixBuilder.py``), and what
     :cpp:member:`ModType::iniFixBuilder` holds. The exact sibling of
     :cpp:class:`IniParseBuilder`, with the same two flavours (fixed factory, or a version-dependent
     #ArgsRepo) -- read that class's notes first; only the differences are repeated here
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The one structural difference from :cpp:class:`IniParseBuilder`: a fixer is built from the
        **parser** for that ``.ini`` file rather than from the file itself, matching the pure-Python
        original's ``iniFixBuilder.build(self._iniParser, ...)``. That is also why
        :cpp:func:`IniFile::fix` cannot build a fixer for a mod type whose parser it could not build
        -- the same reason the original's ``_getFixer`` refuses to build while ``self._iniParser``
        is still ``None``

     .. note::
        These builders are deliberately **not** folded into a shared class template with
        :cpp:class:`IniParseBuilder`. The third member of the family, the pure-Python
        ``IniRemoveBuilder``, is not this shape at all -- it derives from ``FlyweightBuilder``, has
        no version dimension, and carries ``cache``/``id`` flyweight semantics instead -- so a
        template spanning just these two would abstract over a coincidence rather than a real
        family
     @endrst
     */
    class IniFixBuilder {
        public:

            /**
             * @brief
             @rst
             Builds one fixer, already bound to the parser it takes its data from -- the C++
             stand-in for the pure-Python original's ``(cls, args, kwargs)`` triple, see
             :cpp:type:`IniParseBuilder::Factory` :raw-html:`<br />` :raw-html:`<br />`

             The :cpp:class:`BaseIniParser` argument is what the original's
             ``Builder.build(parser)`` passes positionally to the fixer's constructor; it may be
             ``nullptr``, since :cpp:class:`BaseIniFixer` allows an unbound fixer
             @endrst
             */
            using Factory = std::function<std::shared_ptr<BaseIniFixer<>>(BaseIniParser<>*)>;

            /**
             * @brief
             @rst
             The version-dependent lookup table a #build consults -- the C++ counterpart to the
             pure-Python ``IniFixBuilderArgs`` (``model/assets/IniFixBuilderArgs.py``)
             :raw-html:`<br />` :raw-html:`<br />`

             **Four** index columns -- ``fromVersion``, ``fromModName``, ``toVersion``,
             ``toModName`` -- of which **two** are version columns (``fromVersion`` at position 0 and
             ``toVersion`` at position 2). A fixer is chosen for a whole *(source mod at a source
             version)* -> *(target mod at a target version)* pair

             .. note::
                That two-version shape is why this is a :cpp:class:`ModAssets` and **not** a
                :cpp:class:`ModDictAssets` like :cpp:type:`IniParseBuilder::ArgsRepo` -- the same
                reason :cpp:class:`VGRemaps` needs one. It is also why one source mod can have
                *several* fixers at once (``Jean`` fixes to both ``JeanCN`` and ``JeanSea``), which
                is what #buildAll exists to return

             .. note::
                The pure-Python original invokes its generator as ``builderArgsGenerator(self)``,
                passing the builder itself. That indirection is **not** mirrored here, for two
                reasons: nothing would use it (a C++ row is already a closure, so anything it needs
                is captured rather than passed in), and it does not actually work in the original
                either -- every generator in ``data/IniFixBuilderData.py`` is a zero-argument
                ``classmethod``, so that call raises ``TypeError`` (verified against the live
                package). The parse-side original calls its generator with no arguments, which is
                the behaviour reproduced here
             @endrst
             */
            using ArgsRepo = ModAssets<std::string, Factory>;

            /**
             * @brief
             @rst
             The #Factory used when nothing else supplies one -- constructs a plain
             :cpp:class:`BaseIniFixer` bound to the given parser :raw-html:`<br />` :raw-html:`<br />`

             Stands in for the pure-Python original's ``IniFixBuilder(GIMIFixer)`` default. It is
             the *base* class rather than a ``GIMIFixer`` simply because no concrete C++ fixer has
             been ported yet -- change this one function when one lands, and every fallback path
             picks it up at once
             @endrst
             */
            static Factory defaultFactory();

            /**
             * @brief Constructs a builder that always builds a plain :cpp:class:`BaseIniFixer` --
             *      see #defaultFactory
             */
            IniFixBuilder();

            /**
             * @brief Constructs a builder that always uses the same factory, whatever the version
             *
             * @param factory
             @rst
             The factory to build every fixer with. If this is empty, #defaultFactory is used
             instead, mirroring the pure-Python original's own ``iniFixBuilder`` null-fallback
             @endrst
             */
            explicit IniFixBuilder(Factory factory);

            /**
             * @brief Constructs a builder that picks its factory by mod name and game version
             *
             * @param builderArgs
             @rst
             The lookup table to resolve a factory from -- see #ArgsRepo. Held by ``shared_ptr`` so
             one table is shared by every :cpp:class:`ModType` of a game, exactly as the pure-Python
             original's 43 ``IniFixBuilder(ModDataAssets.IniFixBuilderArgs.value)`` calls share a
             single ``IniFixBuilderArgs`` instance :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, the builder degrades to the #defaultFactory-only behaviour of
             the default constructor
             @endrst
             * @param errorOnNotFound
             @rst
             What #build does when 'builderArgs' holds no row for the mod name it was asked about --
             see :cpp:func:`IniParseBuilder::IniParseBuilder`'s own 'errorOnNotFound' argument for
             the full rationale :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             */
            explicit IniFixBuilder(std::shared_ptr<const ArgsRepo> builderArgs, bool errorOnNotFound = false);

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
             Builds the fixer that fixes one specific source mod onto one specific target mod
             :raw-html:`<br />` :raw-html:`<br />`

             For a fixed-factory builder every key argument is ignored entirely -- matching the
             pure-Python original's own "this argument has no effect if ``_buildCls`` is not
             ``None``" warning. Otherwise the four are looked up in #getBuilderArgs :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Use #buildAll when the target mod is not known up front -- which is the normal case,
                since one source mod can fix to several targets
             @endrst
             *
             * @param parser The parser the built fixer takes its data from -- passed straight to
             *      the #Factory, and may be ``nullptr``
             * @param fromModName The name of the source mod (:cpp:member:`ModType::name`)
             * @param toModName The name of the target mod being fixed to
             * @param fromVersion
             @rst
             The game version the ``.ini`` file originates from
             (:cpp:member:`IniFile::fromVersion`). ``std::nullopt`` means the latest listed
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param toVersion
             @rst
             The game version being fixed *to* (:cpp:member:`IniFile::toVersion`). ``std::nullopt``
             means the latest listed :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @throws std::out_of_range If #getErrorOnNotFound is ``true`` and the key has no row
             *
             * @return The built fixer -- never ``nullptr``
             */
            std::shared_ptr<BaseIniFixer<>> build(BaseIniParser<>* parser, const std::string& fromModName,
                                                 const std::string& toModName,
                                                 const std::optional<Version>& fromVersion = std::nullopt,
                                                 const std::optional<Version>& toVersion = std::nullopt) const;

            /**
             * @brief
             @rst
             Builds **every** fixer for one source mod -- one per target mod it fixes to
             :raw-html:`<br />` :raw-html:`<br />`

             This is the normal entry point. A source mod routinely fixes to more than one target
             (``Jean`` fixes to both ``JeanCN`` and ``JeanSea``), so fixing an ``.ini`` file means
             running each of those fixers, not picking one :raw-html:`<br />` :raw-html:`<br />`

             The ``(fromVersion, fromModName, toVersion)`` triple is held fixed and ``toModName`` is
             the axis fanned out over, via :cpp:func:`ModAssets::getAll` -- so each target mod
             resolves its own best ``toVersion`` row independently, rather than one winning version
             being picked across all of them :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A **fixed-factory** builder has no table to fan out over, so it returns exactly one
                entry, keyed by the empty string. That keeps callers uniform without pretending the
                fixed flavour has targets it does not

             .. note::
                Unlike #build this never throws for "nothing matched" -- an empty vector is a
                legitimate answer, and :cpp:func:`IniFile::fix` treats it as "this mod type
                contributes nothing"
             @endrst
             *
             * @param parser The parser the built fixers take their data from -- may be ``nullptr``
             * @param fromModName The name of the source mod (:cpp:member:`ModType::name`)
             * @param fromVersion The game version the ``.ini`` file originates from. **Default**: ``std::nullopt``
             * @param toVersion The game version being fixed *to*. **Default**: ``std::nullopt``
             * @param filteredToModNames
             @rst
             If given, only target mods whose name is in this set are built -- the
             :cpp:member:`IniFile::filteredToModTypeNames` filter :raw-html:`<br />`
             :raw-html:`<br />`

             ``std::nullopt`` (the default) means **no filtering**: every target mod the table lists
             for this source is built
             @endrst
             *
             * @return One ``(toModName, fixer)`` pair per target mod, in no particular order
             */
            std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>> buildAll(
                BaseIniParser<>* parser, const std::string& fromModName,
                const std::optional<Version>& fromVersion = std::nullopt,
                const std::optional<Version>& toVersion = std::nullopt,
                const std::optional<std::unordered_set<std::string>>& filteredToModNames = std::nullopt) const;

        private:
            // Empty exactly when builderArgs_ is set -- see IniParseBuilder's own note.
            Factory factory_;

            std::shared_ptr<const ArgsRepo> builderArgs_;

            bool errorOnNotFound_ = false;
    };
}

#endif
