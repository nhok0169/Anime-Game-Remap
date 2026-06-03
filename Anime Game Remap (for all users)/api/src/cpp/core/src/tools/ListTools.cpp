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