#ifndef AGRemapCore_GlobalIniClassifiers_H
#define AGRemapCore_GlobalIniClassifiers_H

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

#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifier.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Global, shared classifier module used by the software to help identify what mod a .ini file
     belongs to :raw-html:`<br />` :raw-html:`<br />`

     #classifier is built lazily the first time it is accessed, so the (potentially expensive)
     construction only ever happens once, and only if something actually needs it -- a C++11
     function-local ``static`` (guaranteed thread-safe, exactly-once initialization)
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        #classifier arrives **fully populated** with every shipped mod type: its lazy initializer
        walks :cpp:func:`GlobalModTypes::all` and registers each one. A GI mod type is registered
        via :cpp:func:`IniClassifier::addGIModType` with its `section`_-name keywords
        (:cpp:func:`ModTypeIdTools::getSectionKeywords`) and the hashes that identify it -- its
        ``ib``, ``draw_vb``, ``position_vb``, ``blend_vb`` and ``texcoord_vb`` hashes from every
        game version (and, for a skin of several components, its components' too). A WuWa mod type
        is registered via :cpp:func:`IniClassifier::addWuWaModType` with its ``vb0`` hashes alone.
        Texture hashes are never registered, since they are shared between characters, and a hash
        claimed by more than one mod type is registered for neither :raw-html:`<br />`
        :raw-html:`<br />`

        A caller wanting an empty classifier constructs an :cpp:class:`IniClassifier`
        directly instead of going through this class :raw-html:`<br />` :raw-html:`<br />`

        Asking for it also files the shipped mod types into :cpp:class:`ModTypeIdTools`'s registry
        (via :cpp:func:`GlobalModTypes::registerMissing`), because the two are halves of one
        default: a classifier finds mod type *ids*, and :cpp:class:`IniFile` then asks
        :cpp:class:`ModTypeIdTools` to turn each one back into a :cpp:class:`ModType`. Filling only
        one leaves :cpp:func:`IniFile::classify` naming an id it cannot resolve :raw-html:`<br />`
        :raw-html:`<br />`

        Two things about *that* half specifically, neither of which is part of the one-shot lazy
        initializer:

        * it is re-done whenever :cpp:func:`ModTypeIdTools::clear` has emptied the registry since
          the last look (tracked by :cpp:func:`ModTypeIdTools::generation`), so a ``clear()``
          after the first use does not leave this classifier naming ids nothing can resolve --
          which would have every ``.ini`` file come back ``isMod == true`` with no mod types at all
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
