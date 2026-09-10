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

#include "AGRemapCore/tools/ListTools.h"


namespace AGRemapCore {
    std::vector<std::ptrdiff_t> ListTools::getIndsAfterRemove(const std::vector<size_t>& removedInds, size_t lstLen) {
        std::vector<std::ptrdiff_t> shifts(lstLen, 0);
        std::ptrdiff_t shift = 0;
        size_t remPos = 0;

        for (size_t i = 0; i < lstLen; ++i) {
            while (remPos < removedInds.size() && removedInds[remPos] == i) {
                --shift;
                ++remPos;
            }

            shifts[i] = shift;
        }

        return shifts;
    }
}