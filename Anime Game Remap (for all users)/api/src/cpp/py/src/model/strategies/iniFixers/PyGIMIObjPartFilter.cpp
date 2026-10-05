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

#include "PyGIMIObjPartFilter.h"

#include <memory>
#include <unordered_set>
#include <utility>

#include <pybind11/stl.h>

#include "../../PyVersion.h"
#include "graphGroupEdits/PyIniGraphGroups.h"
#include "../../../tools/PyRanges.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

PyGIMIObjPartFilter::Assets* parseAssets(const py::object &table) {
    if (table.is_none()) {
        return nullptr;
    }

    return table.cast<PyGIMIObjPartFilter::Assets*>();
}


std::unordered_set<std::string> parseKeySet(const py::object &keys) {
    std::unordered_set<std::string> result;
    if (keys.is_none()) {
        return result;
    }

    for (auto key : keys) {
        result.insert(py::str(key).cast<std::string>());
    }

    return result;
}

}


PyGIMIObjPartFilter::PyGIMIObjPartFilter(py::object hashes, py::object indices, const py::object &indexHashKeys,
                                           const py::object &version):
    Core(parseAssets(hashes), parseAssets(indices), parseKeySet(indexHashKeys), parseVersionArg(version)),
    hashesObj(std::move(hashes)), indicesObj(std::move(indices)) {}


void initCppGIMIObjPartFilter(pybind11::module_ &m) {
    py::class_<PyGIMIObjPartFilter, py::smart_holder> cls(m, "GIMIObjPartFilter", R"doc(
Builds the `KVP`_ window belonging to one mod object, for a GIMI character whose drawn objects
**share a hash** and are told apart only by the ``match_first_index`` that follows it

.. danger::
    An edit that writes into that shared region without a window is not merely imprecise -- head and
    body are distinguished by the very ``match_first_index`` such an edit overwrites, so a stray
    write puts one object's vertex range onto another. Any edit reaching into a shared-hash region
    needs one of these, and needs :meth:`keysToTrack` asked for rather than restated

Parameters
----------
hashes: Optional[:class:`ModMappedAssets`]
    The ``hash`` asset table -- normally a :class:`ModType`'s ``hashes``. Held, so this filter
    cannot outlive it

indices: Optional[:class:`ModMappedAssets`]
    The ``match_first_index`` asset table -- normally a :class:`ModType`'s ``indices``. Held for the
    same reason

indexHashKeys: Optional[Set[:class:`str`]]
    The *types* of hash (the last index column of a hash row, eg. ``ib``) whose mod objects are
    told apart by a ``match_first_index`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

version: Optional[Union[:class:`str`, :class:`float`, :class:`Version`]]
    The version of the .ini file, or ``None`` for the latest :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object hashes, py::object indices, const py::object &indexHashKeys,
                         const py::object &version) {
        return std::make_unique<PyGIMIObjPartFilter>(std::move(hashes), std::move(indices), indexHashKeys, version);
    }), py::arg("hashes") = py::none(), py::arg("indices") = py::none(),
        py::arg("indexHashKeys") = py::none(), py::arg("version") = py::none());

    cls.def("keysToTrack", [](const PyGIMIObjPartFilter &self) {
        py::set result;
        for (const std::string &key : self.keysToTrack()) {
            result.add(py::str(key));
        }

        return result;
    }, py::doc(R"doc(
The `KVP`_ keys a :class:`GraphGroupEdit` has to be tracking for :meth:`filter` to see anything

.. note::
    Ask for this rather than restating it, so the two can never drift apart -- a filter reading a key
    the edit was not tracking silently returns an empty window, and an empty window silently makes
    the edit do nothing

Returns
-------
Set[:class:`str`]
    The keys to track
    )doc"));

    cls.def("filter", [](const PyGIMIObjPartFilter &self, const py::object &modObj) {
        const PyGIMIObjPartFilter::Core::ModObj parsed = PyIniGraphGroups::modObjFromPy(modObj);

        // Captured by reference-to-self is deliberate: the returned callable is handed straight to a
        // GraphGroupEdit that lives no longer than this filter does, which is the same lifetime rule
        // the C++ GIMIObjPartFilter::filter already has ('this' is captured there too).
        //
        // The (modType, ini) parameters are accepted and ignored, because that is the shape a
        // GraphGroupEdit keyFilter is called with -- see PyGraphGroupEdit.
        return py::cpp_function([&self, parsed](const AGRC::SectionIterData<std::string, std::string> &iterData,
                                                 const py::object &, const py::object &) {
            // PyRanges, not the core Ranges: only the former is a registered pybind11 type, so
            // returning the base by value fails at call time with "Unable to convert function
            // return value to a Python type" -- and a keyFilter is only ever called from inside
            // an edit, where that error surfaces as the whole .ini file being skipped.
            if (iterData.colouring == nullptr) {
                return PyRanges<long long>(std::vector<PyGIMIObjPartFilter::Core::OrderRanges::Range>{});
            }

            return PyRanges<long long>(self.window(parsed, *iterData.colouring).ranges, false);
        }, py::arg("iterData"), py::arg("modType") = py::none(), py::arg("ini") = py::none());
    }, py::arg("modObj"), py::doc(R"doc(
Builds the window function for one mod object, ready to hand to a :class:`GraphGroupEdit`'s
``keyFilters``

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The component and object naming the mod object whose `KVPs`_ the window covers

Returns
-------
Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]
    The window function
    )doc"));

    cls.def_readwrite("indexHashKeys", &PyGIMIObjPartFilter::indexHashKeys, py::doc(R"doc(
Set[:class:`str`]: The types of hash whose mod objects are told apart by a ``match_first_index``
    )doc"));
}
