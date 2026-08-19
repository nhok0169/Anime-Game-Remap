#ifndef AGRemapCore_IniClassifyStats_H
#define AGRemapCore_IniClassifyStats_H

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/strategies/ModType.h"


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
            explicit IniClassifyStats(tsl::ordered_map<int, ModType> modType = {}, bool isMod = false, bool isFixed = false);

            /**
             * @brief
             @rst
             The types of mod found, keyed by their id :raw-html:`<br />` :raw-html:`<br />`

             See :cpp:member:`IniClassifier::hashModTypeIds`'s note for why the keys are plain
             ``int``\\s rather than :cpp:enum:`ModTypeId` itself
             @endrst
             */
            tsl::ordered_map<int, ModType> modType;

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
