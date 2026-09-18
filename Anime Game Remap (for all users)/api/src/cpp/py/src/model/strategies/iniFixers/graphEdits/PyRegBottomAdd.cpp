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

#include "PyRegBottomAdd.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <pybind11/stl.h>

#include "PyRegSurroundedAdd.h"         // parseAdditions / additionsToPy -- the same (key, value)
                                        // tuple-or-list shape every adding edit accepts
#include "../regEdits/PyBaseRegEdit.h"  // PyPartRanges, for a partFilter's return value


namespace {

// Wraps a Python partFilter callable as the core PartFilter -- same shape, and same reason, as
// PyRegSurroundedAdd.cpp's parsePartFilter.
PyRegBottomAdd::Core::PartFilter parsePartFilter(const py::object &partFilter, py::object modType, py::object ini) {
    if (partFilter.is_none() || !PyCallable_Check(partFilter.ptr())) {
        return {};
    }

    py::object heldFilter = partFilter;
    py::object heldModType = std::move(modType);
    py::object heldIni = std::move(ini);

    return [heldFilter, heldModType, heldIni](const PyRegBottomAdd::Core::IterData &iterData, const AGRC::ModType *,
                                               AGRC::IniFile *) -> PyRegBottomAdd::Core::OrderRanges {
        py::object result = heldFilter(py::cast(&iterData, py::return_value_policy::reference), heldModType, heldIni);

        PyPartRanges ranges(result);
        const PyRegBottomAdd::Core::OrderRanges *parsedRanges = ranges.get();
        if (parsedRanges == nullptr) {
            throw py::type_error("A RegBottomAdd partFilter must return a Ranges (or a list of (start, end) bounds), not None");
        }

        return *parsedRanges;
    };
}


std::optional<PyRegBottomAdd::Core::KeySet> parseKeysToTrack(const py::object &keysToTrack) {
    if (keysToTrack.is_none()) {
        return std::nullopt;
    }

    PyRegBottomAdd::Core::KeySet result;
    for (auto key : keysToTrack) {
        result.insert(py::str(key).cast<std::string>());
    }

    return result;
}

}


PyRegBottomAdd::PyRegBottomAdd(py::object additionsObj):
    Core(parseAdditions(additionsObj)) {}


void initCppRegBottomAdd(pybind11::module_ &m) {
    py::class_<PyRegBottomAdd, PyBaseIniGraphEdit, py::smart_holder> cls(m, "RegBottomAdd", R"doc(
This class inherits from :class:`BaseIniGraphEdit`

Adds `KVPs`_ **at the bottom of every root** `section`_ of a caller/callee graph, at that
`section`_'s own nesting depth

The additions land together, in the order given, in a fresh last :class:`IfContentPart`. Being at
the `section`_'s own depth is the whole point: a position inside an ``if`` block runs only when that
block is taken, and "as late as possible" is exactly such a position whenever the `section`_ ends in
one.

.. code-block:: ini

    [TextureOverrideCharacterHead]
    ...
    drawindexed = 3252, 0, 0
    if $accessory == 1
       drawindexed = 618, 3252, 0
    endif
    ; the additions go HERE, outside the block, whatever the toggles are set to

**What this is for**: a fix with something more to say after everything the mod does --- another
draw call, another texture binding, a trailing command --- that has to run unconditionally.

.. note::
    :class:`RegFillMissing` in ``BottomCover`` mode reaches the same position, but only for a
    register the graph is MISSING. That is a different question, and it answers "do nothing" for a
    register the mod already has
    )doc");

    cls.def(py::init([](py::object additions) {
        return std::make_unique<PyRegBottomAdd>(std::move(additions));
    }), py::arg("additions"));

    cls.def_property("additions", [](const PyRegBottomAdd &self) {
        return additionsToPy(self.additions);
    }, [](PyRegBottomAdd &self, py::object additions) {
        self.additions = parseAdditions(additions);
    }, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: The `KVP`_ entries to add, in order --- a single
``(key, value)`` tuple may be assigned and reads back as a one-entry list
    )doc"));

    cls.def("edit", [](PyRegBottomAdd &self, py::object graph, const py::object &modType,
                       const std::string &modName, const py::object &partFilter, bool trackKeys,
                       const py::object &keysToTrack) {
        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);

        self.Core::edit(parsedGraph, nullptr, modName, parsePartFilter(partFilter, modType, py::none()),
                        trackKeys, parseKeysToTrack(keysToTrack));

        // UNLIKE its neighbours, this edit appends brand-new IfContentParts, which the graph's
        // Python-side keep-alive has never seen -- the same reason RegFillMissing's cover modes
        // refresh. Leaving it out hands back wrappers for parts that no longer own their memory.
        parsedGraph.refreshKeepAlive();

        // The original Python object, so 'result is graph' holds
        return graph;
    }, py::arg("graph"), py::arg("modType"), py::arg("modName") = "", py::arg("partFilter") = py::none(),
       py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(),
       py::doc(R"doc(
Adds :attr:`additions` at the bottom of every root `section`_ of 'graph', in a fresh last
:class:`IfContentPart` at the `section`_'s own depth

.. note::
    'trackKeys'/'keysToTrack' are the caller's key-tracking defaults, handed down by
    :class:`BaseIniGraphEdit`'s contract. This edit never colours the graph, so it has no use for
    them --- they are accepted only so the shared call convention keeps working

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to edit

modType: Optional[:class:`ModType`]
    The mod type handed to 'partFilter'

modName: :class:`str`
    The name of the mod being fixed to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partFilter: Optional[Callable]
    Asked about each root's existing last :class:`IfContentPart` --- the part a new one is appended
    after --- and a root it rejects is left alone entirely :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

trackKeys: :class:`bool`
    Unused --- see the note above :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[Set[:class:`str`]]
    Unused --- see the note above :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IniSectionGraph`
    The same graph that was passed in
    )doc"));
}
