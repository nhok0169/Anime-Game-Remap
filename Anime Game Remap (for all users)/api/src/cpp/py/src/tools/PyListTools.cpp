#include <pybind11/pybind11.h>
#include "PyListTools.h"

namespace py = pybind11;


py::list PyListTools::removeParts(py::list lst, py::list partIndices) {
    const py::ssize_t n = lst.size();

    if (partIndices.empty()) {
        return lst;
    }

    std::vector<int> diff(n + 1, 0);

    for (auto item : partIndices) {
        py::tuple t = item.cast<py::tuple>();

        if (t.size() != 2) {
            throw std::runtime_error("partIndices must contain (start, end) tuples");
        }

        py::ssize_t start = t[0].cast<py::ssize_t>();
        py::ssize_t end   = t[1].cast<py::ssize_t>();

        if (start < 0) start = 0;
        if (end > n) end = n;
        if (start >= end) continue;

        diff[start] += 1;
        diff[end]   -= 1;
    }

    py::list result;
    int active = 0;

    for (py::ssize_t i = 0; i < n; ++i) {
        active += diff[i];

        if (active == 0) {
            result.append(lst[i]);
        }
    }

    return result;
}

py::list PyListTools::removeByInds(py::list lst, const std::unordered_set<std::size_t>& inds) {
    py::list result;
    const std::size_t lstSize = py::len(lst);

    for (std::size_t i = 0; i < lstSize; ++i) {
        if (inds.find(i) == inds.end()) {
            result.append(lst[i]);
        }
    }

    return result;
}

py::list PyListTools::addLstsByInds(py::list lst, const std::map<long long, py::list>& subLsts) {
    py::list result;
    const long long lstSize = static_cast<long long>(py::len(lst));
    long long prev = 0;
    long long ind;

    for (const auto& [rawInd, subLst] : subLsts) {
        ind = rawInd;

        if (ind < 0) {
            ind = 0;
        } else if (ind > lstSize) {
            ind = lstSize;
        }

        for (long long i = prev; i < ind; ++i) {
            result.append(lst[i]);
        }

        for (const auto item : subLst) {
            result.append(item);
        }

        prev = ind;
    }

    for (long long i = prev; i < lstSize; ++i) {
        result.append(lst[i]);
    }

    return result;
}


void initCppListTools(py::module_ &m) {
    py::class_<PyListTools>(m, "CppListTools", 
        "C++ Tools for handling with Lists")

        .def_static("removeParts", &PyListTools::removeParts, 
                    py::arg("lst"), py::arg("part_indices"),
                    py::doc(R"doc(
                        Removes many indices from a list

                        Parameters
                        ----------
                        lst: List[T]
                            The desired list to have its parts removed

                        inds: Set[:class:`int`]
                            The indices to the elements in the list that needs to be removed

                        Returns
                        -------
                        List[T]
                            The new list with elements specified by indices removed
                    )doc"))

        .def_static("removeByInds", &PyListTools::removeByInds,
                    py::arg("lst"), py::arg("inds"),
                    py::doc(R"doc(
                        Removes many indices from a list

                        Parameters
                        ----------
                        lst: List[T]
                            The desired list to have its parts removed

                        inds: Set[:class:`int`]
                            The indices to the elements in the list that needs to be removed :raw-html:`<br />` :raw-html:`<br />`

                        Returns
                        -------
                        List[T]
                            The new list with elements specified by indices removed
                    )doc"))

        .def_static("addLstsByInds", &PyListTools::addLstsByInds,
                    py::arg("lst"), py::arg("subLsts"),
                    py::doc(R"doc(
                        Inserts multiple sublists into the main list by index

                        Parameters
                        ----------
                        lst: List[T]
                            The main list to work with
                        
                        subLsts: Dict[:class:`int`, List[T]]
                            The sublists to insert into the main list :raw-html:`<br />` :raw-html:`<br />`

                            The keys are the indices to insert the sublists and the values are the sublists

                        Returns
                        -------
                        List[T]
                            The resultant combined list
                    )doc"))

        .def_static("getIndsAfterRemove", &PyListTools::getIndsAfterRemove,
                    py::arg("removedInds"), py::arg("lstLen"),
                    py::doc(R"doc(
                        Retrieve the index shifts in some data structure,
                        after the list got elements removed by indices

                        Parameters
                        ----------
                        removedInds: List[:class:`int`] 
                            The indices to elements that got removed from the list :raw-html:`<br />` :raw-html:`<br />`

                            Assume that the list in sorted order

                        lstLen: :class:`int`
                            The length of the original list, before its elements got removed

                        Returns
                        -------
                        List[:class:`int`]
                            A list containing how much each index is shifted
                    )doc"));
}