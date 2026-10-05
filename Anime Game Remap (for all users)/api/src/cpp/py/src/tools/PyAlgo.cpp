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

#include "PyAlgo.h"

#include <vector>

namespace py = pybind11;

py::list PyAlgo::pyMerge(const std::vector<std::vector<py::object>>& sortedLsts, const py::function& compare) {
    std::vector<const std::vector<py::object>*> listPtrs;
    listPtrs.reserve(sortedLsts.size());
    for (const auto& lst : sortedLsts) {
        listPtrs.push_back(&lst);
    }

    auto compareFn = [&compare](const py::object& lhs, const py::object& rhs) {
        return compare(lhs, rhs).cast<int>();
    };

    std::vector<py::object> merged;
    AGRC::Algo::merge(listPtrs, compareFn, merged);

    return py::cast(merged);
}

std::pair<bool, std::size_t> PyAlgo::pyBinarySearch(const std::vector<py::object>& lst, const py::object& target, const py::function& compare) {
    if (lst.empty()) {
        return {false, 0};
    }

    auto compareFn = [&compare](const py::object& lhs, const py::object& rhs) {
        return compare(lhs, rhs).cast<int>();
    };

    bool found = false;
    std::size_t index = AGRC::Algo::binarySearch(lst, target, compareFn, found);
    return {found, index};
}

void initCppAlgo(pybind11::module_ &m) {
    py::class_<PyAlgo>(m, "CppAlgo", "C++ Tools for handling algorithm operations")
        .def_static("merge", &PyAlgo::pyMerge,
                    py::arg("sorted_lsts"), py::arg("compare"),
                    py::doc(R"doc(
Merges multiple sorted lists into one sorted list.

Parameters
----------
sorted_lsts: List[List[T]]
    The sorted lists to merge.

compare: Callable[[T, T], int]
    The compare function used to order list elements.

Returns
-------
List[T]
    The merged list of all input elements in sorted order.
                    )doc"))

        .def_static("binarySearch", &PyAlgo::pyBinarySearch,
                    py::arg("lst"), py::arg("target"), py::arg("compare"),
                    py::doc(R"doc(
Performs a binary search for the target element in a sorted list.

Parameters
----------
lst: List[T]
    The sorted list to search.

target: T
    The target element to search for.

compare: Callable[[T, T], int]
    The compare function used to compare list elements with the target.

Returns
-------
Tuple[bool, int]
    A tuple where the first value indicates whether the target was found,
    and the second value is the index of the found element or insertion point.
                    )doc"));
}
