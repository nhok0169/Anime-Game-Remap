#ifndef AGRemapCore_IniRemoveBuilder_H
#define AGRemapCore_IniRemoveBuilder_H

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
#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     A factory that builds the :cpp:class:`BaseIniRemover` for one ``.ini`` file, optionally picking
     *which* remover based on the mod's name and the game version the ``.ini`` file came from
     :raw-html:`<br />` :raw-html:`<br />`

     What :cpp:member:`ModType::iniRemoveBuilder` holds. The sibling of :cpp:class:`IniParseBuilder` and
     :cpp:class:`IniFixBuilder`, with the same two flavours (fixed factory, or a version-dependent
     #ArgsRepo) -- read :cpp:class:`IniParseBuilder`'s notes first; only the differences are repeated
     here :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This builds a fresh remover per #build call, like its two siblings -- it is **not** a
        flyweight. A remover handed out here is bound to the file it was built for and stays that
        way, so two :cpp:class:`IniFile`\\s never share one remover, and there is no cache to
        clear

     .. note::
        By default every mod type shares one remover class (the single global builder,
        :cpp:func:`GlobalIniRemoveBuilders::removeBuilder`), with no per-mod or per-version
        variation. The #ArgsRepo lookup exists so that per-mod removers *can* be expressed when
        they are needed; see :cpp:class:`IniRemoveBuilderData` for the table
     @endrst
     */
    class IniRemoveBuilder {
        public:

            /**
             * @brief
             @rst
             Builds one remover, already bound to the ``.ini`` file it will act on -- see
             :cpp:type:`IniParseBuilder::Factory` :raw-html:`<br />` :raw-html:`<br />`

             The :cpp:class:`IniFile` argument is the file the remover will act on; it may be
             ``nullptr``, since
             :cpp:class:`BaseIniRemover` allows an unbound remover
             @endrst
             */
            using Factory = std::function<std::shared_ptr<BaseIniRemover<>>(IniFile*)>;

            /**
             * @brief
             @rst
             The version-dependent lookup table a #build can consult -- the same ``{version, name}``
             shape as :cpp:type:`IniParseBuilder::ArgsRepo`, and filled in by
             :cpp:class:`IniRemoveBuilderData` :raw-html:`<br />` :raw-html:`<br />`

             See this class's own note on why it exists
             @endrst
             */
            using ArgsRepo = ModDictAssets<std::string, Factory>;

            /**
             * @brief
             @rst
             The #Factory used when nothing else supplies one -- constructs a
             :cpp:class:`RemapIniRemover` bound to the given file, via
             :cpp:func:`RemapIniRemover::factory` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This is the single choke point for the whole family: every row of
                :cpp:class:`IniRemoveBuilderData` returns it, and so do the default constructor and
                #build's not-found fallback. Change it here and every path picks the new remover up
                at once
             @endrst
             */
            static Factory defaultFactory();

            /**
             * @brief Constructs a builder that always builds a :cpp:class:`RemapIniRemover` --
             *      see #defaultFactory
             */
            IniRemoveBuilder();

            /**
             * @brief Constructs a builder that always uses the same factory, whatever the version
             *
             * @param factory
             @rst
             The factory to build every remover with. If this is empty, #defaultFactory is used
             instead
             @endrst
             */
            explicit IniRemoveBuilder(Factory factory);

            /**
             * @brief Constructs a builder that picks its factory by mod name and game version
             *
             * @param builderArgs
             @rst
             The lookup table to resolve a factory from -- see #ArgsRepo. Held by ``shared_ptr`` so
             one table is shared by every :cpp:class:`ModType` of a game, the same way the other two
             builders share theirs :raw-html:`<br />` :raw-html:`<br />`

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
            explicit IniRemoveBuilder(std::shared_ptr<const ArgsRepo> builderArgs, bool errorOnNotFound = false);

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
             Builds the remover for one ``.ini`` file, **already bound to it** :raw-html:`<br />`
             :raw-html:`<br />`

             #. Resolve which #Factory applies: from #getBuilderArgs using 'modName'/'version' when a
                table is in use, otherwise the builder's single fixed factory
             #. Construct a remover with it -- a **new** one on every call, see this class's own
                warning
             #. Bind the result to 'iniFile' with :cpp:func:`BaseIniRemover::setIniFile` before
                returning

             :raw-html:`<br />`

             .. note::
                Step 3 is not redundant with step 2 even though #defaultFactory's remover binds
                itself: a caller-supplied #Factory is free to ignore its :cpp:class:`IniFile`
                argument, and this is what guarantees the result is bound regardless
             @endrst
             *
             * @param iniFile The .ini file the remover will act on -- passed to the #Factory, and
             *      bound onto the result. May be ``nullptr``
             * @param modName
             @rst
             The name of the mod to build the remover for (:cpp:member:`ModType::name`)
             :raw-html:`<br />` :raw-html:`<br />`

             Ignored entirely when this builder has no #getBuilderArgs, matching how
             :cpp:func:`IniParseBuilder::build` treats it for a fixed-factory builder
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``""``
             @endrst
             * @param version
             @rst
             The game version the ``.ini`` file originates from (:cpp:member:`IniFile::version`). If
             ``std::nullopt``, the latest listed version for 'modName' is used :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @throws std::out_of_range If #getErrorOnNotFound is ``true`` and 'modName' has no row
             *      at any version
             *
             * @return The remover -- never ``nullptr``
             */
            std::shared_ptr<BaseIniRemover<>> build(IniFile* iniFile, const std::string& modName = "",
                                                   const std::optional<Version>& version = std::nullopt) const;

        private:
            // Empty exactly when builderArgs_ is set -- see IniParseBuilder's own note.
            Factory factory_;

            std::shared_ptr<const ArgsRepo> builderArgs_;

            bool errorOnNotFound_ = false;

            // Resolves which factory this call should use -- the table when there is one, the fixed
            // factory otherwise. Never returns an empty std::function.
            Factory resolveFactory(const std::string& modName, const std::optional<Version>& version) const;
    };
}

#endif
