#ifndef AGRemapCore_IniRemoveBuilder_H
#define AGRemapCore_IniRemoveBuilder_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModDictAssets.h"
#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     A **flyweight** factory that hands out the :cpp:class:`BaseIniRemover` for a ``.ini`` file,
     reusing one shared remover instance per id rather than building a fresh one each time
     :raw-html:`<br />` :raw-html:`<br />`

     The C++ counterpart to the pure-Python ``IniRemoveBuilder``
     (``model/strategies/iniRemovers/IniRemoveBuilder.py``), and what
     :cpp:member:`ModType::iniRemoveBuilder` holds :raw-html:`<br />` :raw-html:`<br />`

     .. warning::
        This is **not** the same shape as :cpp:class:`IniParseBuilder` / :cpp:class:`IniFixBuilder`,
        even though it now shares their version-keyed lookup. The pure-Python original derives from
        ``FlyweightBuilder`` rather than ``Builder``, and two of its differences survive here:

        #. **It caches, and the cache lives on the builder.** The other two build a genuinely fresh
           strategy every call and let :cpp:class:`IniFile` own what it built; this one hands the
           *same* instance back to every caller sharing a cache key, so one remover is shared across
           every ``.ini`` file that resolves to it
        #. **The returned remover is re-bound on every call** (see #build), which is what makes
           reusing one shared instance workable at all

     .. note::
        The #ArgsRepo flavour is a deliberate **extension beyond** the pure-Python original, not a
        port of it: there is no ``IniRemoveBuilderData.py`` or ``IniRemoveBuilderArgs.py``, and the
        only ``IniRemoveBuilder`` the whole Python package ever constructs is the single global
        ``IniRemoveBuilder(IniRemover)`` in ``constants/GlobalIniRemoveBuilders.py`` -- every mod
        type there shares one remover class, with no per-mod or per-version variation. The lookup
        exists here so that per-mod removers *can* be expressed when they are needed; see
        :cpp:class:`IniRemoveBuilderData` for the table, whose rows are all currently stubs

     .. note::
        When an #ArgsRepo is in use the flyweight cache is keyed by **mod name** rather than by
        #getId, so two mod types whose rows resolve to different factories cannot collide on one
        cached instance. That is the one behavioural consequence of adding the lookup, and the
        reason the id is not simply reused as the key -- see #build
     @endrst
     */
    class IniRemoveBuilder {
        public:

            /**
             * @brief
             @rst
             Builds one remover, already bound to the ``.ini`` file it will act on -- the C++
             stand-in for the pure-Python original's ``(cls, args, kwargs)`` triple, see
             :cpp:type:`IniParseBuilder::Factory` :raw-html:`<br />` :raw-html:`<br />`

             The :cpp:class:`IniFile` argument is what the original's ``build(args = [iniFile])``
             passes positionally to the remover's constructor; it may be ``nullptr``, since
             :cpp:class:`BaseIniRemover` allows an unbound remover
             @endrst
             */
            using Factory = std::function<std::shared_ptr<BaseIniRemover>(IniFile*)>;

            /**
             * @brief
             @rst
             The version-dependent lookup table a #build can consult -- the same ``{version, name}``
             shape as :cpp:type:`IniParseBuilder::ArgsRepo`, and filled in by
             :cpp:class:`IniRemoveBuilderData` :raw-html:`<br />` :raw-html:`<br />`

             Unlike the other two, this has **no pure-Python counterpart** -- see this class's own
             note on why it exists anyway
             @endrst
             */
            using ArgsRepo = ModDictAssets<std::string, Factory>;

            /**
             * @brief
             @rst
             The #Factory used when nothing else supplies one -- constructs a plain
             :cpp:class:`BaseIniRemover` bound to the given file :raw-html:`<br />` :raw-html:`<br />`

             Stands in for the pure-Python original's ``IniRemoveBuilder(IniRemover)`` default (see
             ``constants/GlobalIniRemoveBuilders.py``). It is the *base* class rather than an
             ``IniRemover`` simply because no concrete C++ remover has been ported yet -- change
             this one function when one lands, and every fallback path picks it up at once
             @endrst
             */
            static Factory defaultFactory();

            /**
             * @brief
             @rst
             The id #build files a remover under when the caller names none :raw-html:`<br />`
             :raw-html:`<br />`

             The pure-Python original uses ``self._buildCls.__name__`` here. C++ has no runtime
             class name to read off a #Factory closure, so the id is a plain string chosen at
             construction, defaulting to this constant. The effect is the same either way: since the
             cache is per-builder and a builder has exactly one factory, the default id is simply a
             *stable* key, so one builder keeps one cached remover unless a caller deliberately asks
             for a different id
             @endrst
             */
            static const std::string& defaultId();

            /**
             * @brief Constructs a caching builder that hands out a plain :cpp:class:`BaseIniRemover`
             *      -- see #defaultFactory
             */
            IniRemoveBuilder();

            /**
             * @brief Constructs a builder
             *
             * @param factory
             @rst
             The factory to build a remover with. If this is empty, #defaultFactory is used instead
             @endrst
             * @param id
             @rst
             The id to file built removers under when #build's caller names none -- see #defaultId.
             An empty string means "use #defaultId" :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``""``
             @endrst
             * @param cache
             @rst
             The initial value of #cache :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             */
            explicit IniRemoveBuilder(Factory factory, std::string id = "", bool cache = true);

            /**
             * @brief Constructs a builder that picks its factory by mod name and game version
             *
             * @param builderArgs
             @rst
             The lookup table to resolve a factory from -- see #ArgsRepo. If this is ``nullptr``, the
             builder degrades to the #defaultFactory-only behaviour of the default constructor
             @endrst
             * @param errorOnNotFound
             @rst
             What #build does when 'builderArgs' holds no row for the mod name it was asked about --
             see :cpp:func:`IniParseBuilder::IniParseBuilder`'s own 'errorOnNotFound' argument
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             * @param cache
             @rst
             The initial value of #cache :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             */
            explicit IniRemoveBuilder(std::shared_ptr<const ArgsRepo> builderArgs, bool errorOnNotFound = false, bool cache = true);

            /**
             * @brief
             @rst
             Whether #build reuses a cached remover instead of constructing a new one every call
             :raw-html:`<br />` :raw-html:`<br />`

             A plain, publicly mutable member rather than a getter/setter pair, matching the
             pure-Python original's own public ``self.cache`` attribute (and this codebase's
             :cpp:member:`IniFile::downloadMode`) :raw-html:`<br />` :raw-html:`<br />`

             Setting this to ``false`` does **not** empty the cache -- see #clearCache. It only
             stops #build consulting or filling it, which is exactly what the original's
             ``if (not cache)`` early-out does :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             */
            bool cache = true;

            /**
             * @brief
             @rst
             The id built removers are filed under when #build's caller names none **and** no
             #ArgsRepo is in use -- see #defaultId and #build's own note on cache keys
             @endrst
             */
            const std::string& getId() const;

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
             Hands back the remover for one ``.ini`` file, **already bound to it** :raw-html:`<br />`
             :raw-html:`<br />`

             Mirrors the pure-Python original's ``build`` exactly:

             #. Resolve which #Factory applies: from #getBuilderArgs using 'modName'/'version' when a
                table is in use, otherwise the builder's single fixed factory
             #. If #cache is ``false``, construct a fresh remover with it and return it, consulting
                nothing
             #. Otherwise look up this call's **cache key** (see below); on a miss, construct one
                with that factory and store it there
             #. Either way, bind the result to 'iniFile' with
                :cpp:func:`BaseIniRemover::setIniFile` before returning -- the equivalent of the
                original's ``result.iniFile = iniFile``

             :raw-html:`<br />`

             The cache key is the first of these that applies:

             #. 'id', when the caller passed one -- an explicit override, matching the original's
                own ``id`` argument
             #. 'modName', when a #getBuilderArgs is in use and 'modName' is non-empty. **This is
                what stops two mod types whose rows resolve to different factories from colliding on
                one cached instance**, and is the one way this diverges from the original's
                id-only keying
             #. #getId otherwise

             :raw-html:`<br />`

             .. note::
                Step 4 happens on a cache **hit** too, not just on a miss. That is the whole reason
                a single shared remover can serve many ``.ini`` files: each call re-points it at the
                caller's file. The corollary is that a remover handed out here is only correctly
                bound until the *next* #build call landing on the same key -- so use it immediately
                rather than storing it, and do not remove fixes from two :cpp:class:`IniFile`\\s
                concurrently through one builder

             .. note::
                The cache is never keyed by the ``.ini`` file. Two different files resolving to the
                same key share an instance by design -- that is what "flyweight" means here
             @endrst
             *
             * @param iniFile The .ini file the remover will act on -- passed to the #Factory on a
             *      cache miss, and bound onto the result either way. May be ``nullptr``
             * @param modName
             @rst
             The name of the mod to build the remover for (:cpp:member:`ModType::name`) :raw-html:`<br />`
             :raw-html:`<br />`

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
             * @param id
             @rst
             An explicit cache key, overriding the mod-name/#getId defaults above :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @throws std::out_of_range If #getErrorOnNotFound is ``true`` and 'modName' has no row
             *      at any version
             *
             * @return The remover -- never ``nullptr``
             */
            std::shared_ptr<BaseIniRemover> build(IniFile* iniFile, const std::string& modName = "",
                                                   const std::optional<Version>& version = std::nullopt,
                                                   const std::optional<std::string>& id = std::nullopt);

            /**
             * @brief
             @rst
             Empties the flyweight cache, so the next #build constructs fresh removers
             :raw-html:`<br />` :raw-html:`<br />`

             No equivalent exists on the pure-Python original, whose ``_cache`` dict is private and
             never cleared. Added here because a cached remover holds a non-owning
             :cpp:class:`IniFile` pointer (see #build's note): if that file is destroyed while this
             builder outlives it -- easy, since :cpp:member:`ModType::iniRemoveBuilder` is shared
             and long-lived -- the cached remover is left pointing at freed memory until the next
             #build rebinds it. Clearing is the way to drop that dangling binding deliberately
             @endrst
             */
            void clearCache();

            /**
             * @brief How many removers are currently cached
             */
            std::size_t getCacheSize() const;

        private:
            // Empty exactly when builderArgs_ is set -- the two flavours are mutually exclusive,
            // matching IniParseBuilder/IniFixBuilder.
            Factory factory_;

            std::shared_ptr<const ArgsRepo> builderArgs_;

            bool errorOnNotFound_ = false;

            std::string id_;

            std::unordered_map<std::string, std::shared_ptr<BaseIniRemover>> cache_;

            // Resolves which factory this call should use -- the table when there is one, the fixed
            // factory otherwise. Never returns an empty std::function.
            Factory resolveFactory(const std::string& modName, const std::optional<Version>& version) const;
    };
}

#endif
