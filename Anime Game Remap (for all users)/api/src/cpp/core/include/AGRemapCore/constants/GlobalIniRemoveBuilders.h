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
     ``IniRemoveBuilder(RemapIniRemover)`` the first time it's accessed. #removeBuilder below gets the
     same lazy, build-once-then-reuse behavior from a C++11 function-local ``static`` (guaranteed
     thread-safe, exactly-once initialization), exactly as :cpp:class:`GlobalIniClassifiers` does
     :raw-html:`<br />` :raw-html:`<br />`

     This is what :cpp:class:`ModType` falls back to when constructed with no remove builder of its
     own, matching the pure-Python ``ModType``'s own
     ``iniRemoveBuilder = GlobalIniRemoveBuilders.RemoveBuilder.value`` default :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        The *builder* is shared by every :cpp:class:`ModType` that falls back to it, but the
        **removers** are not: :cpp:func:`IniRemoveBuilder::build` constructs a fresh one per call,
        bound to that caller's ``.ini`` file. The pure-Python original shares one remover instead --
        see :cpp:class:`IniRemoveBuilder`'s own warning for why that was not mirrored

     .. note::
        The builder returned here wraps :cpp:func:`IniRemoveBuilder::defaultFactory`, so it produces
        a real :cpp:class:`RemapIniRemover` -- the same thing the pure-Python original's
        ``IniRemoveBuilder(RemapIniRemover)`` produces

     .. note::
        #globalRemoveBuilder is the second one here, and produces the general-use
        :cpp:class:`GlobalRemapIniRemover` instead. It has no pure-Python counterpart -- that class is new
        rather than ported
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

            /**
             * @brief
             @rst
             The shared :cpp:class:`IniRemoveBuilder` that produces the **general-use**
             :cpp:class:`GlobalRemapIniRemover`, lazily constructed on first access and reused for every
             later call :raw-html:`<br />` :raw-html:`<br />`

             This is the remover for a ``.ini`` file that belongs to a mod but could not be
             attributed to any :cpp:enum:`ModTypeId` -- see :cpp:class:`GlobalRemapIniRemover`'s own note
             on when that is the right one, and :cpp:func:`IniFile::removeFix`, which is what asks
             for it :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Kept separate from #removeBuilder rather than replacing it. The two differ only in
                which remover they hand out, and that difference is the whole point: #removeBuilder's
                :cpp:class:`RemapIniRemover` asks whose a leftover `section`_ is and this one's
                :cpp:class:`GlobalRemapIniRemover` never does, so a caller that *has* mod types to ask
                about still wants the former
             @endrst
             *
             * @return The shared general-use remove builder
             */
            static const std::shared_ptr<IniRemoveBuilder>& globalRemoveBuilder();
    };
}

#endif
