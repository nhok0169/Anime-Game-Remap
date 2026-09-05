#ifndef AGRemapCore_GlobalModTypes_H
#define AGRemapCore_GlobalModTypes_H

#include <vector>

#include "AGRemapCore/model/strategies/ModType.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Every :cpp:class:`ModType` the software ships with, and the one place that files them into
     :cpp:class:`ModTypeIdTools`'s global registry :raw-html:`<br />` :raw-html:`<br />`

     The counterpart to the pure-Python ``ModTypes`` enum (``constants/ModTypes.py``), whose
     ``getAll()`` likewise builds the shipped mod types on demand. Genshin Impact is currently the
     only game with a builder, so #all is :cpp:func:`GIBuilder::all` today; a second game would be
     aggregated here rather than at each call site

     .. important::
        #registerAll is **not** called automatically by anything in ``AGRemapCore``, and that is
        deliberate. (#registerMissing *is*, by
        :cpp:func:`GlobalIniClassifiers::classifier` -- but only ever to fill gaps, never to
        overwrite what a caller registered.) :cpp:func:`ModTypeIdTools::getModType` and
        :cpp:func:`ModTypeIdTools::findByName` deliberately report only what was explicitly
        registered, which is what lets a caller (and the core's own tests) do
        :cpp:func:`ModTypeIdTools::clear` followed by
        :cpp:func:`ModTypeIdTools::registerModType` and get a registry containing *exactly* the
        mod types it asked for. Self-populating those lookups on first use would quietly break
        that. An application wanting the shipped mod types asks for them
     @endrst
     */
    class GlobalModTypes {
        public:

            GlobalModTypes() = delete;

            /**
             * @brief
             @rst
             Every shipped :cpp:class:`ModType`, freshly built on each call :raw-html:`<br />`
             :raw-html:`<br />`

             Fresh rather than shared because a :cpp:class:`ModType` owns mutable asset tables
             (:cpp:member:`ModType::hashes` and friends support ``addRepoRows``/``addMap``), so one
             caller adding a hash must not be visible to every other one -- the same reasoning
             :cpp:class:`Hashes`'s own constructor records for copying its prototype repo
             @endrst
             *
             * @return All the shipped mod types
             */
            static std::vector<ModType> all();

            /**
             * @brief
             @rst
             Files every mod type from #all into :cpp:class:`ModTypeIdTools`'s global registry, so
             :cpp:func:`ModTypeIdTools::getModType` can resolve them by id and
             :cpp:func:`ModTypeIdTools::findByName` by name or alias :raw-html:`<br />`
             :raw-html:`<br />`

             Idempotent: registering a mod type twice replaces the existing entry rather than
             duplicating it, and re-adding a name to the lookup DFA is a no-op. Note it registers
             *in addition to* whatever is already there rather than replacing the registry --
             call :cpp:func:`ModTypeIdTools::clear` first to start from empty
             @endrst
             */
            static void registerAll();

            /**
             * @brief
             @rst
             Files every shipped :cpp:class:`ModType` that is **not already registered** into
             :cpp:class:`ModTypeIdTools`'s global registry, leaving every id the caller registered
             for itself exactly as it found it :raw-html:`<br />` :raw-html:`<br />`

             The difference from #registerAll is only what happens on a collision: that one
             overwrites, this one yields. This is what the *implicit* population behind
             :cpp:func:`GlobalIniClassifiers::classifier` uses, so that asking for the default
             classifier can no longer silently replace a :cpp:class:`ModType` the caller had
             already registered under one of the shipped ids -- a caller that registered an id has
             said what it wants for that id, and the default filling in the rest is a different
             statement from the default overruling it

             .. note::
                A caller that genuinely wants the shipped mod types to win calls #registerAll,
                which still overwrites. Nothing implicit does
             @endrst
             */
            static void registerMissing();
    };
}

#endif
