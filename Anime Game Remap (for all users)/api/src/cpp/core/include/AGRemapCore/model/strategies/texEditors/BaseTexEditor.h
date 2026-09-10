#ifndef AGRemapCore_BaseTexEditor_H
#define AGRemapCore_BaseTexEditor_H

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

    class TextureFile;

    /**
     * @brief
     @rst
     Base class to edit some ``.dds`` file
     @endrst
     */
    class BaseTexEditor {
        public:
            virtual ~BaseTexEditor() = default;

            /**
             * @brief Edits the texture file. No-op by default
             *
             * @param texFile The texture ``.dds`` file to be modified
             * @param fixedTexFile The name of the fixed texture file
             */
            virtual void fix(TextureFile &texFile, const std::string &fixedTexFile);
    };
}

#endif
