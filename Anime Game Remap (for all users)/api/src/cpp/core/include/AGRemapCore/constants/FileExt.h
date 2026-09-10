#ifndef AGRemapCore_FileExt_H
#define AGRemapCore_FileExt_H

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
     Different file extensions for files :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``FileExt`` enum (``constants/FileExt.py``)
     @endrst
     */
    class FileExt {
        public:

            /**
             * @brief Initialization file extension
             */
            static inline const std::string Ini = ".ini";

            /**
             * @brief Text file extension
             */
            static inline const std::string Txt = ".txt";

            /**
             * @brief Buffer file extension
             */
            static inline const std::string Buf = ".buf";

            /**
             * @brief
             @rst
             `Direct Draw Surface`_ file extension
             @endrst
             */
            static inline const std::string DDS = ".dds";
    };
}

#endif
