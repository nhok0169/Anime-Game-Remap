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

#include "AGRemapCore/tools/Algo.h"


namespace AGRemapCore {
    template <typename T>
    struct MergeElement {
        size_t listIdx;
        size_t elemIdx;
        const T* value;
    };


    template <typename T, typename Compare>
    void Algo::merge(const std::vector<const std::vector<T>*> &sortedLsts, const Compare& compare, std::vector<T> &result) {
        result.clear();
        
        size_t total_elements = 0;
        for (const auto* lstPtr : sortedLsts) {
            if (lstPtr != nullptr) {
                total_elements += lstPtr->size();
            }
        }
        result.reserve(total_elements);

        auto heapCompare = [&compare](const MergeElement<T>& a, const MergeElement<T>& b) {
            return compare(*b.value, *a.value) < 0; 
        };

        std::priority_queue<MergeElement<T>, std::vector<MergeElement<T>>, decltype(heapCompare)> minHeap(heapCompare);

        for (size_t i = 0; i < sortedLsts.size(); ++i) {
            if (sortedLsts[i] != nullptr && !sortedLsts[i]->empty()) {
                minHeap.push({i, 0, &((*sortedLsts[i])[0])});
            }
        }

        while (!minHeap.empty()) {
            MergeElement<T> curr = minHeap.top();
            minHeap.pop();

            result.push_back(*curr.value);

            size_t nextElemIdx = curr.elemIdx + 1;
            if (nextElemIdx < sortedLsts[curr.listIdx]->size()) {
                minHeap.push({curr.listIdx, nextElemIdx, &((*sortedLsts[curr.listIdx])[nextElemIdx])});
            }
        }
    }

    static size_t findMid(size_t left, size_t right) {
        return left + (right - left) / 2;
    }

    template <typename T, typename Compare>
    size_t Algo::binarySearch(const std::vector<T> &lst, const T &target, const Compare& compare, bool &found) {
        size_t left = 0;
        if (lst.empty()) {
            found = false;
            return left; 
        }

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
                if (mid == 0) break;
                right = mid - 1;
            } else {
                left = mid + 1;
            }

            mid = findMid(left, right);
        }

        found = false;
        return left;
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