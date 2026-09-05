#ifndef AGRemapCore_GlobalIniClassifiers_H
#define AGRemapCore_GlobalIniClassifiers_H

#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifier.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Global, shared classifier module used by the software to help identify what mod a .ini file
     belongs to :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``GlobalIniClassifiers`` class (``constants/GlobalIniClassifiers.py``)
     -- a ``DeferredEnum`` there, lazily building its one ``IniClassifierOld`` instance the first
     time it's accessed, so the (potentially expensive) construction only ever happens once, and
     only if something actually needs it. #classifier below gets the same lazy,
     build-once-then-reuse behavior for free from a C++11 function-local ``static`` (guaranteed
     thread-safe, exactly-once initialization) :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        #classifier arrives **fully populated** with every shipped GI mod type: its lazy
        initializer walks :cpp:func:`GlobalModTypes::all` and registers each one's
        `section`_-name keywords (:cpp:func:`ModTypeIdTools::getSectionKeywords`) via
        :cpp:func:`IniClassifier::addGIModType`. That mirrors the pure-Python original, whose
        ``GlobalIniClassifiers.Classifier`` is likewise an ``IniClassifierOld`` built through an
        ``IniClassifierBuilderOld`` :raw-html:`<br />` :raw-html:`<br />`

        No hashes are registered, only keywords -- the pure-Python builder identifies a mod type by
        `section`_ name alone, so passing hashes here would be a behaviour change rather than a
        port. A caller wanting an empty classifier constructs an :cpp:class:`IniClassifier`
        directly instead of going through this class :raw-html:`<br />` :raw-html:`<br />`

        Asking for it also files the shipped mod types into :cpp:class:`ModTypeIdTools`'s registry
        (via :cpp:func:`GlobalModTypes::registerMissing`), because the two are halves of one
        default: a classifier finds mod type *ids*, and :cpp:class:`IniFile` then asks
        :cpp:class:`ModTypeIdTools` to turn each one back into a :cpp:class:`ModType`. Filling only
        one leaves :cpp:func:`IniFile::classify` naming an id it cannot resolve :raw-html:`<br />`
        :raw-html:`<br />`

        Two things about *that* half specifically, both of which it used to get wrong by being
        welded to this function's one-shot lazy initializer:

        * it is re-done whenever :cpp:func:`ModTypeIdTools::clear` has emptied the registry since
          the last look (tracked by :cpp:func:`ModTypeIdTools::generation`). Before, a ``clear()``
          after the first use left this classifier naming ids nothing could resolve for the rest of
          the process -- every ``.ini`` file coming back ``isMod == true`` with no mod types at all
        * it uses :cpp:func:`GlobalModTypes::registerMissing`, not
          :cpp:func:`GlobalModTypes::registerAll`, so a :cpp:class:`ModType` the caller registered
          for itself under one of the shipped ids is left alone rather than silently replaced the
          first time anything classifies

        :raw-html:`<br />`

        That does **not** make registration implicit in general -- see
        :cpp:func:`GlobalModTypes::registerAll`'s own note. A caller that injects its own
        classifier never comes through here and keeps full control of the registry
     @endrst
     */
    class GlobalIniClassifiers {
        public:

            GlobalIniClassifiers() = delete;

            /**
             * @brief
             @rst
             The shared default :cpp:class:`IniClassifier` used to identify whether a .ini file
             belongs to some mod, lazily constructed on first access and reused for every later call
             @endrst
             *
             * @return A reference to the shared default classifier
             */
            static IniClassifier& classifier();
    };
}

#endif
