#ifndef AGRemapCore_Algo_H
#define AGRemapCore_Algo_H

#include <cstdint>
#include <vector>


namespace AGRemapCore {
    class Algo {
        public:
            template <typename T, typename Compare>
            static size_t binarySearch(std::vector<T> &lst, const T &target, const Compare& compare, bool &found);

            template <typename T, typename Compare>
            static bool binaryInsert(std::vector<T> &lst, const T &target,  const Compare& compare, bool optionalInsert = false);
    };
}

#include "Algo.tpp"

#endif