#include "AGRemapCore/tools/Algo.h"


namespace AGRemapCore {
    static size_t findMid(size_t left, size_t right) {
        return left + (right - left) / 2;
    }

    template <typename T, typename Compare>
    size_t Algo::binarySearch(std::vector<T> &lst, const T &target, const Compare& compare, bool &found) {
        size_t left = 0;
        size_t right = lst.size() - 1;
        size_t mid = findMid(left, right);
        std::int8_t compResult = 0;

        while (left <= right) {
            const T& midItem = lst[mid];
            compResult = compare(midItem, target);

            if (compResult == 0) {
                found = true;
                return mid;
            } else if (compResult > 0) {
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }

        found = false;
        return 0;
    }

    template <typename T, typename Compare>
    bool Algo::binaryInsert(std::vector<T> &lst, const T &target, const Compare& compare, bool optionalInsert) {
        bool found = false;
        bool inserted = false;

        size_t insertInd = binarySearch(lst, target, compare, found);
        if (!optionalInsert || !found) {
            lst.emplace(lst.begin() + insertInd, target);
            inserted = true;
        }

        return inserted;
    }
}