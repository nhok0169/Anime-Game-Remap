#ifndef AGRemapCore_IniResourceModel_H
#define AGRemapCore_IniResourceModel_H

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
#include <utility>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Contains data for some particular resource in a .ini file :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``IniResourceModel`` class (``model/iniresources/IniResourceModel.py``)
     @endrst
     */
    class IniResourceModel {
        public:

            /**
             * @brief Constructs new data for a resource in a .ini file
             *
             * @param iniFolderPath The folder path to where the .ini file of the resource is located
             */
            explicit IniResourceModel(std::string iniFolderPath): iniFolderPath(std::move(iniFolderPath)) {}

            virtual ~IniResourceModel() = default;

            /**
             * @brief The folder path to where the .ini file of the resource is located
             */
            std::string iniFolderPath;
    };
}

#endif
