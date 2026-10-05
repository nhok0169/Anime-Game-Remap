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

#include <algorithm>
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

            /**
             * @brief
             @rst
             Appends 'value' to 'lst' unless it is already in it, keeping the order things were first
             seen in :raw-html:`<br />` :raw-html:`<br />`

             The order-preserving dedupe that ten sites in ``core`` wrote out by hand as
             ``if (std::find(v.begin(), v.end(), x) == v.end()) v.push_back(x);`` -- a linear scan
             per insert, which is right for the short lists this is used on (a mod's objects, a
             character's roles, a section's predecessors) and wrong for long ones
             @endrst
             *
             * @param lst The list to append to, modified in place
             * @param value The value to append if it is not already present
             *
             * @return Whether the value was appended
             */
            template <typename T>
            static bool pushDistinct(std::vector<T>& lst, const T& value) {
                if (std::find(lst.begin(), lst.end(), value) != lst.end()) {
                    return false;
                }

                lst.push_back(value);
                return true;
            }
    };
}

#endif