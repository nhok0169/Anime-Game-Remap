#ifndef AGRemapCore_GlobalIniRemoveBuilders_H
#define AGRemapCore_GlobalIniRemoveBuilders_H

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

#include <memory>

#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Global, shared builder used by the software to create the modules that remove fixes from a
     ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     #removeBuilder below is built lazily, the first time it is accessed, and reused afterwards -- a
     C++11 function-local ``static`` (guaranteed thread-safe, exactly-once initialization), exactly
     as :cpp:class:`GlobalIniClassifiers` does :raw-html:`<br />` :raw-html:`<br />`

     This is what :cpp:class:`ModType` falls back to when constructed with no remove builder of its
     own :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The *builder* is shared by every :cpp:class:`ModType` that falls back to it, but the
        **removers** are not: :cpp:func:`IniRemoveBuilder::build` constructs a fresh one per call,
        bound to that caller's ``.ini`` file -- see :cpp:class:`IniRemoveBuilder`'s own warning

     .. note::
        The builder returned here wraps :cpp:func:`IniRemoveBuilder::defaultFactory`, so it produces
        a real :cpp:class:`RemapIniRemover`

     .. note::
        #globalRemoveBuilder is the second one here, and produces the general-use
        :cpp:class:`GlobalRemapIniRemover` instead
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
