#ifndef AGRemapCore_IniClassifyStats_H
#define AGRemapCore_IniClassifyStats_H

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

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/strategies/ModTypeIdData.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Stores the statistics about the classification result of a .ini file
     @endrst
     */
    class IniClassifyStats {
        public:

            /**
             * @brief
             @rst
             Constructs the statistics about the classification result of a .ini file
             @endrst
             *
             * @param modType The types of mod found, keyed by their id
             * @param isMod Whether the .ini file belongs to a mod
             * @param isFixed Whether the .ini file is fixed
             */
            explicit IniClassifyStats(tsl::ordered_map<int, ModTypeIdData> modType = {}, bool isMod = false, bool isFixed = false);

            /**
             * @brief
             @rst
             The types of mod found, keyed by their id :raw-html:`<br />` :raw-html:`<br />`

             See :cpp:member:`IniClassifier::hashGameTypeIds`'s note for why the keys are plain
             ``int``\\s rather than :cpp:enum:`ModTypeId` itself
             @endrst
             */
            tsl::ordered_map<int, ModTypeIdData> modType;

            /**
             * @brief Whether the .ini file belongs to a mod
             */
            bool isMod;

            /**
             * @brief Whether the .ini file is fixed
             */
            bool isFixed;
    };
}

#endif
