#ifndef AGRemapCore_TextureKindData_H
#define AGRemapCore_TextureKindData_H
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
#include <vector>


namespace AGRemapCore {
    namespace Data {
        /**
         * @brief
         @rst
         The data table backing :cpp:class:`TextureKinds`: what each of a character's texture roles is, per game version :raw-html:`<br />` :raw-html:`<br />`

         Rows are ``({version, name, component, type}, value)`` rows -- the shape :cpp:func:`getIndexDataRows` has :raw-html:`<br />` :raw-html:`<br />`

         .. note::
            Kept by hand, and checked: :cpp:func:`WWMIFixerConfig::withGameData` refuses a role a config names that has
            no row here, so ``Tools/Misc/Diagnostics/genWWMIAssetData.py --check`` (which fills every pair's config)
            fails on a missing one. A new role's row says what it is, with ``ModTypeIdTools::getName`` for the name
         @endrst
         */
        const std::vector<std::pair<std::vector<std::string>, std::string>>& getTextureKindDataRows();
    }
}

#endif
