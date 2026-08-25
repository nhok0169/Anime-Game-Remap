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
        Unlike the pure-Python original (built via ``IniClassifierBuilderOld``, which registers real
        GI mod type data via hashes/`section`_ keywords), #classifier currently returns a bare,
        freshly-constructed :cpp:class:`IniClassifier` with **no mod types registered on it yet** --
        porting the equivalent of ``IniClassifierBuilderOld`` (something that calls
        :cpp:func:`IniClassifier::addGIModType`/:cpp:func:`IniClassifier::addWuWaModType` for every
        real mod type) hasn't happened yet. :cpp:class:`IniFile` falls back to this when no explicit
        classifier is given, so until that builder is ported, real-world classification results
        through the default classifier will be limited to whatever's actually been registered on
        this instance (nothing, by default)
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
