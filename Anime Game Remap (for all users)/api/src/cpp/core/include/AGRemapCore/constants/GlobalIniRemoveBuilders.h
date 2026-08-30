#ifndef AGRemapCore_GlobalIniRemoveBuilders_H
#define AGRemapCore_GlobalIniRemoveBuilders_H

#include <memory>

#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Global, shared builder used by the software to create the modules that remove fixes from a
     ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``GlobalIniRemoveBuilders`` class
     (``constants/GlobalIniRemoveBuilders.py``) -- a ``DeferredEnum`` there, lazily building its one
     ``IniRemoveBuilder(IniRemover)`` the first time it's accessed. #removeBuilder below gets the
     same lazy, build-once-then-reuse behavior from a C++11 function-local ``static`` (guaranteed
     thread-safe, exactly-once initialization), exactly as :cpp:class:`GlobalIniClassifiers` does
     :raw-html:`<br />` :raw-html:`<br />`

     This is what :cpp:class:`ModType` falls back to when constructed with no remove builder of its
     own, matching the pure-Python ``ModType``'s own
     ``iniRemoveBuilder = GlobalIniRemoveBuilders.RemoveBuilder.value`` default :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        Because the returned builder is shared *and* caches (see :cpp:class:`IniRemoveBuilder`),
        every :cpp:class:`ModType` that falls back to it ends up sharing **one** remover instance,
        rebound per :cpp:func:`IniRemoveBuilder::build` call. That is the pure-Python behavior too

     .. note::
        The builder returned here wraps :cpp:func:`IniRemoveBuilder::defaultFactory`, so it produces
        a bare :cpp:class:`BaseIniRemover` rather than the pure-Python original's real
        ``IniRemover`` -- no concrete C++ remover has been ported yet
     @endrst
     */
    class GlobalIniRemoveBuilders {
        public:

            GlobalIniRemoveBuilders() = delete;

            /**
             * @brief
             @rst
             The shared default :cpp:class:`IniRemoveBuilder`, lazily constructed on first access
             and reused for every later call
             @endrst
             *
             * @return The shared default remove builder
             */
            static const std::shared_ptr<IniRemoveBuilder>& removeBuilder();
    };
}

#endif
