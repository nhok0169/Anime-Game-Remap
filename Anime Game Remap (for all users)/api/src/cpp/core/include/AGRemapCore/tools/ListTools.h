#ifndef AGRemapCore_ListTools_H
#define AGRemapCore_ListTools_H

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