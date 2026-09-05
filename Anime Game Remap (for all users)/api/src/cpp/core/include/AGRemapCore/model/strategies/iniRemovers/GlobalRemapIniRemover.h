#ifndef AGRemapCore_GlobalRemapIniRemover_H
#define AGRemapCore_GlobalRemapIniRemover_H

#include <functional>
#include <memory>
#include <string>

#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemovalContext.h"
#include "AGRemapCore/model/strategies/iniRemovers/RemapIniRemover.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`RemapIniRemover`, and so from
     :cpp:class:`BaseIniRemover` :raw-html:`<br />` :raw-html:`<br />`

     The **general-use** remover: a :cpp:class:`RemapIniRemover` that always removes the fix without
     asking which :cpp:class:`ModType` it belongs to :raw-html:`<br />` :raw-html:`<br />`

     Everything about how the fix is found is :cpp:class:`RemapIniRemover`'s -- read that class for
     the algorithm. The only difference is that #remove forces
     :cpp:member:`IniRemovalContext::ignoreModType` on, whatever the caller passed, so every
     candidate `section`_ is a target: everything the fix boilerplate surrounds, plus every
     ``Remap``-named leftover outside it, whoever they belong to :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        **When this is the right remover.** :cpp:class:`RemapIniRemover`'s strict rule decides a
        ``Remap``-named leftover *outside* the boilerplate by asking whether its ``hash`` belongs to
        one of the ``.ini`` file's :cpp:class:`ModType`\\s. A file that has **no** mod types cannot
        answer that question at all, so on that file the strict rule can only ever recognize the
        boilerplate half and would leave every leftover standing forever. This class is the answer
        for exactly that file: one the classifier says belongs to a mod
        (:cpp:member:`IniClassifyStats::isMod`) but could not attribute to any
        :cpp:enum:`ModTypeId`. :cpp:func:`IniFile::removeFix` reaches for it in that state, and only
        when its ``readAllIni`` was asked for -- see that method

     .. note::
        A subclass rather than "construct a :cpp:class:`RemapIniRemover` and pass
        ``IniRemovalContext(true)``", deliberately. The sweep is then a property of *the remover*
        rather than of every call site that happens to remember to ask for it, which is what lets it
        be handed out by an :cpp:class:`IniRemoveBuilder` (see
        :cpp:func:`GlobalIniRemoveBuilders::globalRemoveBuilder`) and reached through
        :cpp:member:`ModType::iniRemoveBuilder` like any other remover -- neither of which carries a
        :cpp:class:`IniRemovalContext` anywhere in its signature

     .. note::
        It inherits :cpp:class:`RemapIniRemover` rather than :cpp:class:`BaseIniRemover` directly
        because it *is* one -- "a :cpp:class:`RemapIniRemover` that assumes
        :cpp:member:`IniRemovalContext::ignoreModType`" is the whole specification, and every knob
        (#headings, #remapKeyword, #hideOriginalComment, the resource keywords) means the same thing
        here. Deriving straight from :cpp:class:`BaseIniRemover` would mean carrying a second copy
        of an 800-line algorithm that has to stay bug-for-bug identical
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     * @tparam RemoverBase
     @rst
     The :cpp:class:`BaseIniRemover` specialization to inherit from, spliced through
     :cpp:class:`RemapIniRemover` -- see that class's own parameter of the same name
     :raw-html:`<br />` :raw-html:`<br />`

     **Default**: ``BaseIniRemover<K, V, KeyHash, KeyEqual>``
     @endrst
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>,
              typename RemoverBase = BaseIniRemover<K, V, KeyHash, KeyEqual>>
    class GlobalRemapIniRemover: public RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase> {
        public:

            /**
             * @brief The remover this specializes -- everything but #remove comes from here
             */
            using Base = RemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>;

            /**
             * @brief The ``.ini`` file this reads and rewrites, behind an interface
             */
            using Context = typename Base::Context;

            /**
             * @brief The ``.ini``-domain customization points this needs -- :cpp:class:`RemapIniRemover`'s own
             */
            using RemoverConfig = typename Base::RemoverConfig;

            /**
             * @brief
             @rst
             An :cpp:type:`IniRemoveBuilder::Factory` that builds one of these over an
             :cpp:class:`IniFileRemoveContext` -- the counterpart of
             :cpp:func:`RemapIniRemover::factory`, and what
             :cpp:func:`GlobalIniRemoveBuilders::globalRemoveBuilder` is built on :raw-html:`<br />`
             :raw-html:`<br />`

             Only meaningful for the plain ``<std::string, std::string>`` instantiation, since
             :cpp:class:`IniFileRemoveContext` is the only thing an :cpp:class:`IniFile*` can be
             turned into
             @endrst
             */
            static std::function<std::shared_ptr<BaseIniRemover<>>(IniFile*)> factory();

            /**
             * @brief Constructs a new remover
             *
             * @param ctx
             @rst
             The ``.ini`` file to remove the fix from, behind its interface -- non-owning, and it
             must outlive this remover. See :cpp:func:`RemapIniRemover::RemapIniRemover`
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param config The .ini-domain customization points to use. **Default**: :cpp:func:`RemapIniRemover::defaultConfig`
             */
            explicit GlobalRemapIniRemover(Context* ctx = nullptr, RemoverConfig config = Base::defaultConfig());

            /**
             * @brief
             @rst
             :cpp:func:`RemapIniRemover::remove`, with
             :cpp:member:`IniRemovalContext::ignoreModType` forced on :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                'context' is taken **by value** and the flag is set on this copy, so a caller's own
                :cpp:class:`IniRemovalContext` is never written through -- it simply does not get a
                say on that one member. Every other member of it is honoured normally
             @endrst
             *
             * @param parse Ignored -- see :cpp:func:`RemapIniRemover::remove`. **Default**: ``false``
             * @param writeBack Whether to write back the new text content of the .ini file. **Default**: ``true``
             * @param context
             @rst
             The per-call options for this removal, with
             :cpp:member:`IniRemovalContext::ignoreModType` ignored :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: a default-constructed one
             @endrst
             *
             * @return The new text content of the .ini file
             */
            std::string remove(bool parse = false, bool writeBack = true, IniRemovalContext context = IniRemovalContext()) override;
    };
}

#include "GlobalRemapIniRemover.tpp"

#endif
