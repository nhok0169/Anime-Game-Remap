#ifndef AGRemapCore_ListTools_H
#define AGRemapCore_ListTools_H

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

#include <vector>
#include <cstddef>


namespace AGRemapCore {
    /**
     * @brief Tools for handling with Lists
     */
    class ListTools {
        public:

            /**
             * @brief Retrieve the index shifts in some data structure,
             * after the list got elements removed by indices
             * 
             * @param removedInds The indices to elements that got removed from the list
             * @param lstLen The length of the original list, before its elements got removed
             * 
             * @return A list containing how much each index is shifted
             */
            static std::vector<std::ptrdiff_t> getIndsAfterRemove(const std::vector<size_t>& removedInds, size_t lstLen);
    };
}

#endif