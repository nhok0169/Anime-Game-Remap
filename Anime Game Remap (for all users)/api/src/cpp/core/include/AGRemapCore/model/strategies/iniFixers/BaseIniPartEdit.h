#ifndef AGRemapCore_BaseIniPartEdit_H
#define AGRemapCore_BaseIniPartEdit_H

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


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class for a filter that edits some part of a ``.ini`` file :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        The pure-Python original also declares ``edit``/``editFromIni`` here, as
        ``(*args, modType, modName = "", **kwargs) -> Any``. That signature can't be expressed in
        C++ -- every subclass takes genuinely different arguments and returns a different type
        (:cpp:class:`BaseIniGraphEdit` edits an :cpp:class:`IniSectionGraph`,
        :cpp:class:`BaseRegEdit` edits an :cpp:class:`IfContentPart`,
        :cpp:class:`BaseIniGraphGroupEdit` edits a whole vector of
        :cpp:class:`IniGraphGroup`\\s) -- so each of those declares its own **typed**
        ``edit``/``editFromIni`` pair instead, and only #clear (which really is common) lives here
     @endrst
     */
    class BaseIniPartEdit {
        public:
            virtual ~BaseIniPartEdit() = default;

            /**
             * @brief Clears any saved state information. No-op by default
             */
            virtual void clear();
    };
}

#endif
