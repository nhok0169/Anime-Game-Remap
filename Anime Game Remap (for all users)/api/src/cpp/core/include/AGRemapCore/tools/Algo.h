#ifndef AGRemapCore_Algo_H
#define AGRemapCore_Algo_H

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

#include <cstdint>
#include <vector>
#include <queue>


namespace AGRemapCore {

    /**
     * @brief Tools for some basic algorithms
     */
    class Algo {
        public:

            /**
             * @brief Merges k sorted lists toghether
             * @note 
             @rst
             Implemented using the `standard heap solution`_ (See `k-way merge problem`_ for more details)
             @endrst
             *
             * @tparam T The type of the elements in the list
             * @tparam Compare The compare function
             *
             * @param sortedLsts The sorted lists to merge
             * @param compare
             @rst
             The `compare function`_ for comparing elements in the lists
             @endrst
             * @param result The resultant list to hold all elements from the given lists merged toghether, preserving ordering
             */
            template <typename T, typename Compare>
            static void merge(const std::vector<const std::vector<T>*> &sortedLsts, const Compare& compare, std::vector<T> &result);
            
            /**
             * @brief 
             @rst
             Performs `binary search`_ to search for 'target' in 'lst'
             @endrst
             *
             * @tparam T The type of the elements in the list
             * @tparam Compare The compare function
             *
             * @param lst The sorted list we are searching from
             * @param target The target element to search for in the list
             * @param compare
             @rst
             The `compare function`_ for comparing elements in the list with the target element
             @endrst
             * @param found A resultant pointer that indicates whether target element is found in the list
             *
             * @return The found index or the index that we expect the target element to be in the list
             */
            template <typename T, typename Compare>
            static size_t binarySearch(const std::vector<T> &lst, const T &target, const Compare& compare, bool &found);

            /**
             * @brief
             @rst
             Inserts 'target' into 'lst' using `binary search`_
             @endrst
             *
             * @tparam T The type of the elements in the list
             * @tparam Compare The compare function
             * 
             * @param lst The sorted list we want to insert the target element
             * @param target The target element to insert
             * @param compare
             @rst
             The `compare function`_ for comparing elements in the list with the target element
             @endrst
             * @param optionalInsert Whether to still insert the target element into the list if the element target element is found in the list
             *
             * @return Whether the target element has been inserted into the list
             */
            template <typename T, typename Compare>
            static bool binaryInsert(std::vector<T> &lst, const T &target,  const Compare& compare, bool optionalInsert = false);
    };
}

#include "Algo.tpp"

#endif