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

#include "PyRegBranchAdd.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <pybind11/stl.h>

#include "PyRegSurroundedAdd.h"         // parseAdditions -- the same (key, value) tuple-or-list shape every adding edit accepts
#include "../regEdits/PyBaseRegEdit.h"  // PyPartRanges, for a partFilter's return value


namespace {

// Wraps a Python partFilter callable as the core PartFilter -- same shape, and same reason, as
// PyRegBottomAdd.cpp's parsePartFilter.
PyRegBranchAdd::Core::PartFilter parsePartFilter(const py::object &partFilter, py::object modType) {
    if (partFilter.is_none() || !PyCallable_Check(partFilter.ptr())) {
        return {};
    }

    py::object heldFilter = partFilter;
    py::object heldModType = std::move(modType);

    return [heldFilter, heldModType](const PyRegBranchAdd::Core::IterData &iterData, const AGRC::ModType *,
                                     AGRC::IniFile *) -> PyRegBranchAdd::Core::OrderRanges {
        py::object result = heldFilter(py::cast(&iterData, py::return_value_policy::reference), heldModType, py::none());

        PyPartRanges ranges(result);
        const PyRegBranchAdd::Core::OrderRanges *parsedRanges = ranges.get();
        if (parsedRanges == nullptr) {
            throw py::type_error("A RegBranchAdd partFilter must return a Ranges (or a list of (start, end) bounds), not None");
        }

        return *parsedRanges;
    };
}


std::optional<PyRegBranchAdd::Core::KeySet> parseKeysToTrack(const py::object &keysToTrack) {
    if (keysToTrack.is_none()) {
        return std::nullopt;
    }

    PyRegBranchAdd::Core::KeySet result;
    for (auto key : keysToTrack) {
        result.insert(py::str(key).cast<std::string>());
    }

    return result;
}


// What a Python branchOf answers, as the core Branch:
//   None                                  -> nothing belongs here
//   (key, additions)                      -> append 'additions' in the branch named 'key'
//   (key, additions, replacements)        -> ...and SET 'replacements' there
// An empty key also means nothing belongs here, as it does in the core.
PyRegBranchAdd::Core::Branch parseBranch(const py::object &result) {
    PyRegBranchAdd::Core::Branch branch;
    if (result.is_none()) {
        return branch;
    }

    py::sequence items = py::reinterpret_borrow<py::sequence>(result);
    const std::size_t count = py::len(items);
    if (count == 0 || count > 3) {
        throw py::type_error("A RegBranchAdd branchOf must return None or a (key, additions[, replacements]) tuple");
    }

    branch.key = py::str(items[0]).cast<std::string>();
    if (count > 1 && !items[1].is_none()) {
        branch.additions = parseAdditions(items[1]);
    }

    if (count > 2 && !items[2].is_none()) {
        branch.replacements = parseAdditions(items[2]);
    }

    return branch;
}

}


PyRegBranchAdd::PyRegBranchAdd(py::object branchOfObj): Core({}), branchOfObj(std::move(branchOfObj)) {}


void PyRegBranchAdd::refresh() {
    if (branchOfObj.is_none()) {
        branchOf = {};
        return;
    }

    py::object held = branchOfObj;
    branchOf = [held](const AGRC::Z3Predicate &query, const Core::IterData &iterData) {
        // The predicate by value: it is a small handle, and Python may keep it past this call.
        return parseBranch(held(query, py::cast(&iterData, py::return_value_policy::reference)));
    };
}


void initCppRegBranchAdd(pybind11::module_ &m) {
    py::class_<PyRegBranchAdd, PyBaseIniGraphEdit, py::smart_holder> cls(m, "RegBranchAdd", R"doc(
This class inherits from :class:`BaseIniGraphEdit`

Adds `KVPs`_ **inside** a conditional branch, with the entries decided by the condition that branch
runs under

.. code-block:: ini

    [CommandListCharacterBody]
    if $swapvar == 0
       ib = ResourceBodyIb0
       ; these entries go HERE, and are computed for THIS branch
    else if $swapvar == 1
       ib = ResourceBodyIb1
       drawindexed = 200, 0, 0
       ; ...and separately HERE, computed for this one
    endif

**What this is for.** A mod whose branches are different models --- a merged mod's ``$swapvar`` chain
is a dozen mods behind one .ini --- where what the fix has to add differs per branch because the
geometry does: a ``drawindexed``'s count and offset, a blend's ``draw`` vertex count.

**How a branch is identified.** Not by position: 'branchOf' is handed the :class:`Z3Predicate` the
part runs under and answers with a key naming the branch, so the caller decides by satisfiability.
Answering ``None`` or an empty key declines the part --- a `section`_'s unconditional preamble, say,
which every branch's condition is satisfiable with

.. note::
    Where a key appears in several parts, the entries go in the **last** of them, in the graph's own
    iteration order --- the one everything else in the branch has already run before

.. note::
    Reach for :class:`RegBottomAdd` first: it lands one block at the `section`_'s own depth,
    outside every ``if``, which is right whenever the addition does not depend on which branch is
    taken. This edit is for when it does

Parameters
----------
branchOf: Optional[Callable[[:class:`Z3Predicate`, :class:`SectionIterData`], Optional[Tuple[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], List[Tuple[:class:`str`, :class:`str`]]]]]]
    Decides what belongs in each branch, from the condition a part runs under and the part itself.
    It answers ``None`` for nothing, or a tuple of: :raw-html:`<br />` :raw-html:`<br />`

    #. The key naming the branch (empty for nothing)
    #. The `KVPs`_ to append in that branch
    #. *Optional:* the `KVPs`_ to SET in that branch --- replacing what the branch carries, and
       added where it carries nothing. For a key the branch already answers for itself, like a
       ``draw``, where a second one would not correct the first :raw-html:`<br />` :raw-html:`<br />`

    ``None`` makes the edit a no-op :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object branchOf) {
        return std::make_unique<PyRegBranchAdd>(std::move(branchOf));
    }), py::arg("branchOf") = py::none());

    cls.def_property("branchOf", [](const PyRegBranchAdd &self) {
        return self.branchOfObj;
    }, [](PyRegBranchAdd &self, py::object branchOf) {
        self.branchOfObj = std::move(branchOf);
    }, py::doc(R"doc(
Optional[Callable[[:class:`Z3Predicate`, :class:`SectionIterData`], Optional[Tuple]]]: Decides what belongs in each branch
    )doc"));

    cls.def("edit", [](PyRegBranchAdd &self, py::object graph, const py::object &modType,
                       const std::string &modName, const py::object &partFilter, bool trackKeys,
                       const py::object &keysToTrack) {
        self.refresh();
        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);

        self.Core::edit(parsedGraph, nullptr, modName, parsePartFilter(partFilter, modType),
                        trackKeys, parseKeysToTrack(keysToTrack));

        // The original Python object, so 'result is graph' holds
        return graph;
    }, py::arg("graph"), py::arg("modType"), py::arg("modName") = "", py::arg("partFilter") = py::none(),
       py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(),
       py::doc(R"doc(
Adds each branch's `KVPs`_ inside that branch of 'graph', as :attr:`branchOf` decides them

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
    Asked about each part, and a part it rejects is left alone entirely :raw-html:`<br />` :raw-html:`<br />`

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
