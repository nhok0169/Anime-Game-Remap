#include "PyRegDelimitedAdd.h"

#include <optional>
#include <string>
#include <utility>

#include "PyRegSurroundedAdd.h"         // parseAddition / parseRegMap -- the same (key, value) tuple and
                                        // {register: Optional[predicate]} dict shapes
#include "../regEdits/PyBaseRegEdit.h"  // PyPartRanges, for a partFilter's return value, same as
                                        // PyRegSurroundedAdd.cpp


namespace {

// Wraps a Python partFilter callable as the core PartFilter -- same shape, and same reason, as
// PyRegSurroundedAdd.cpp's parsePartFilter.
PyRegDelimitedAdd::Core::PartFilter parsePartFilter(const py::object &partFilter, py::object modType, py::object ini) {
    if (partFilter.is_none() || !PyCallable_Check(partFilter.ptr())) {
        return {};
    }

    py::object heldFilter = partFilter;
    py::object heldModType = std::move(modType);
    py::object heldIni = std::move(ini);

    return [heldFilter, heldModType, heldIni](const PyRegDelimitedAdd::Core::IterData &iterData, const AGRC::ModType *,
                                               AGRC::IniFile *) -> PyRegDelimitedAdd::Core::OrderRanges {
        py::object result = heldFilter(py::cast(&iterData, py::return_value_policy::reference), heldModType, heldIni);

        PyPartRanges ranges(result);
        const PyRegDelimitedAdd::Core::OrderRanges *parsedRanges = ranges.get();
        if (parsedRanges == nullptr) {
            throw py::type_error("A RegDelimitedAdd partFilter must return a Ranges (or a list of (start, end) bounds), not None");
        }

        return *parsedRanges;
    };
}


std::optional<PyRegDelimitedAdd::Core::KeySet> parseKeysToTrack(const py::object &keysToTrack) {
    if (keysToTrack.is_none()) {
        return std::nullopt;
    }

    PyRegDelimitedAdd::Core::KeySet result;
    for (auto key : keysToTrack) {
        result.insert(py::str(key).cast<std::string>());
    }

    return result;
}

}


PyRegDelimitedAdd::PyRegDelimitedAdd(py::object additionObj, py::object delimiterRegsObj):
    Core(parseAddition(additionObj), parseRegMap(delimiterRegsObj)),
    delimiterRegsObj(delimiterRegsObj.is_none() ? py::dict() : delimiterRegsObj.cast<py::dict>()) {}


void initCppRegDelimitedAdd(pybind11::module_ &m) {
    py::class_<PyRegDelimitedAdd, PyBaseIniGraphEdit, py::smart_holder> cls(m, "RegDelimitedAdd", R"doc(
This class inherits from :class:`BaseIniGraphEdit`

Adds a `KVP`_ into some caller/callee graph of :class:`IniSectionGraph` **exactly once per
delimiter-free stretch of every execution path**

Cut every execution path through the graph at each accepted occurence of a register in
:attr:`delimiterRegs`. That leaves *segments*: from the start of the path to its first delimiter,
from each delimiter to the next, and from the last delimiter to the end of the path. This edit
places :attr:`addition` so that **every segment of every path contains it exactly once, as late as
possible**, using two placement rules:

1. immediately before every accepted delimiter occurence, in every part, and
2. at the end of every *path-terminal* part -- a part after which nothing more executes on its
   path (the end of a section that no other section ``run``\s)

The motivating case is a texture fix (``NNFix``/``ORFix``) that must be issued right before every
``drawindexed`` it applies to, and issued once more -- but never twice in a row -- at the end of a
path that renders nothing:

.. code-block:: ini

   a = 1
   if $x == 2
      insertion = NNFix      ; before the draw
      drawindexed = a
   else
     foo = 2                 ; nothing here -- the shared end below covers this path
   endif
   b = 3
   insertion = NNFix         ; once, at the end of both paths

.. note::
    Do not reach for :class:`RegSurroundedAdd` for this rule -- that edit inserts once per
    surrounded **window** across the graph, so two delimiters in sequence get a single insertion
    before the last of them

.. note::
    ``run =`` is modeled as a call with return: the end of a called section is not a path end (its
    caller continues afterwards), so the trailing insertion lands in the caller. A section that is
    **both** a root and a ``run`` target gets **no** trailing insertion -- a single position cannot
    be a path end when reached directly and not when called, and this edit never doubles the
    addition. Real mods do not ``run`` their hash-triggered sections

Parameters
----------
addition: Tuple[:class:`str`, :class:`str`]
    The `KVP`_ to add

delimiterRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
    The registers whose accepted occurences cut every execution path into segments :raw-html:`<br />` :raw-html:`<br />`

    * The keys are the names of the registers (eg. ``drawindexed``, ``drawindexedinstanced``)
    * The values are the predicates for which particular occurence of the register to accept,
      taking in the value of the occurence -- ``None`` accepts any occurence

    ``None``/empty means every path is a single segment: :attr:`addition` is added once at the end
    of every path and nowhere else :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    // py::init(factory), same as PyRegSurroundedAdd: the core holds std::function predicates
    cls.def(py::init([](py::object addition, py::object delimiterRegs) {
        return std::make_unique<PyRegDelimitedAdd>(std::move(addition), std::move(delimiterRegs));
    }), py::arg("addition"), py::arg("delimiterRegs") = py::none());

    cls.def_property("addition", [](const PyRegDelimitedAdd &self) {
        return py::make_tuple(self.addition.first, self.addition.second);
    }, [](PyRegDelimitedAdd &self, py::object addition) {
        self.addition = parseAddition(addition);
    }, py::doc(R"doc(
Tuple[:class:`str`, :class:`str`]: The `KVP`_ to add
    )doc"));

    cls.def_property("delimiterRegs", [](const PyRegDelimitedAdd &self) {
        return self.delimiterRegsObj;
    }, [](PyRegDelimitedAdd &self, py::object delimiterRegs) {
        self.delimiterRegsObj = delimiterRegs.is_none() ? py::dict() : delimiterRegs.cast<py::dict>();
        self.delimiterRegs = parseRegMap(self.delimiterRegsObj);
    }, py::doc(R"doc(
Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]: The registers whose accepted
occurences cut every execution path into the segments :attr:`addition` is added exactly once into
    )doc"));

    cls.def("edit", [](PyRegDelimitedAdd &self, py::object graph, const py::object &modType,
                       const std::string &modName, const py::object &partFilter, bool trackKeys,
                       const py::object &keysToTrack) {
        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);

        // nullptr for the core's ModType*, and the Python object captured by the wrapped
        // partFilter instead -- see PyRegSurroundedAdd.cpp's identical note
        self.Core::edit(parsedGraph, nullptr, modName, parsePartFilter(partFilter, modType, py::none()),
                        trackKeys, parseKeysToTrack(keysToTrack));

        // Only ever inserts a KVP into an already-existing IfContentPart (addKVPAt), never a new
        // one -- no refreshKeepAlive() needed. Returns the original Python object so
        // 'result is graph' holds
        return graph;
    }, py::arg("graph"), py::arg("modType"), py::arg("modName") = "", py::arg("partFilter") = py::none(),
       py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(),
       py::doc(R"doc(
Adds :attr:`addition` exactly once into every delimiter-free segment of every execution path
through 'graph', as late as possible: immediately before every accepted delimiter, and at the end
of every path-terminal part

.. note::
    'trackKeys'/'keysToTrack' are the caller's key-tracking defaults, handed down by
    :class:`BaseIniGraphEdit`'s contract (:class:`GraphGroupEdit` passes its own). This edit reads
    its delimiters straight off each part and never colours the graph, so it has no use for them
    -- they are accepted only so the shared call convention keeps working

.. warning::
    'partFilter' gates each candidate position on its own -- a position its ranges do not contain
    is skipped, never relocated -- so a filter that excludes the only valid position of a segment
    breaks the exactly-once guarantee for that segment

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to edit

modType: Optional[:class:`ModType`]
    The type of mod to fix. Unused by this edit -- only forwarded to 'partFilter'

modName: :class:`str`
    The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
    Which order indices may be used within a part -- ``None`` accepts every index :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

trackKeys: :class:`bool`
    Unused by this edit. **Default**: ``False``

keysToTrack: Optional[Set[:class:`str`]]
    Unused by this edit. **Default**: ``None``

Returns
-------
:class:`IniSectionGraph`
    The same graph that was passed in, after editing
    )doc"));
}
