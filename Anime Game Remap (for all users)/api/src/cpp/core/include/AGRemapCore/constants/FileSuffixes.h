#ifndef AGRemapCore_FileSuffixes_H
#define AGRemapCore_FileSuffixes_H

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

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Suffixes this software puts on the files it creates :raw-html:`<br />` :raw-html:`<br />`

     A complete port of the pure-Python ``FileSuffixes`` enum (``constants/FileSuffixes.py``)

     .. note::
        Despite the class name, these are markers looked for **anywhere** in a file's base name,
        not only at its end -- the pure-Python original searches for them with the same
        substring-matching machinery it uses for :cpp:class:`FilePrefixes`, and a remap copy is
        named ``<something>RemapFix.ini``, so the marker never actually sits at the very end
     @endrst
     */
    class FileSuffixes {
        public:

            /**
             * @brief
             @rst
             What marks a ``.ini`` file as a *copy* this software generated, rather than one a mod
             author wrote :raw-html:`<br />` :raw-html:`<br />`

             :cpp:func:`RemapService::fix` filters these out of its walk -- handing one back to the
             fix would be feeding it its own output
             @endrst
             */
            static inline const std::string RemapFixCopy = "RemapFix";
    };
}

#endif
