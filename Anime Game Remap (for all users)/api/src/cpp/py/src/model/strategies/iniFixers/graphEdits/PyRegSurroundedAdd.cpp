#include "PyRegSurroundedAdd.h"

#include <optional>
#include <string>
#include <utility>

#include "../regEdits/PyBaseRegEdit.h"  // reuses PyPartRanges (the "a bound Ranges, or a raw list of
                                        // bounds" resolver) for a partFilter's return value, same as
                                        // PyRegFillMissing.cpp


namespace {

// Wraps a Python partFilter callable as the core PartFilter -- same shape, and same reason, as
// PyRegFillMissing.cpp's parsePartFilter.
PyRegSurroundedAdd::Core::PartFilter parsePartFilter(const py::object &partFilter, py::object modType, py::object ini) {
    if (partFilter.is_none() || !PyCallable_Check(partFilter.ptr())) {
        return {};
    }

    // Captured by value: the resulting std::function only ever runs synchronously inside a binding
    // call, so the GIL is always held when it is invoked.
    py::object heldFilter = partFilter;
    py::object heldModType = std::move(modType);
    py::object heldIni = std::move(ini);

    return [heldFilter, heldModType, heldIni](const PyRegSurroundedAdd::Core::IterData &iterData, const AGRC::ModType *,
                                               AGRC::IniFile *) -> PyRegSurroundedAdd::Core::OrderRanges {
        py::object result = heldFilter(py::cast(&iterData, py::return_value_policy::reference), heldModType, heldIni);

        PyPartRanges ranges(result);
        const PyRegSurroundedAdd::Core::OrderRanges *parsedRanges = ranges.get();
        if (parsedRanges == nullptr) {
            throw py::type_error("A RegSurroundedAdd partFilter must return a Ranges (or a list of (start, end) bounds), not None");
        }

        return *parsedRanges;
    };
}


// Converts a Python 'keysToTrack' argument into the core's own optional key set -- same convention
// as PyRegFillMissing.cpp's parseKeysToTrack.
std::optional<PyRegSurroundedAdd::Core::KeySet> parseKeysToTrack(const py::object &keysToTrack) {
    if (keysToTrack.is_none()) {
        return std::nullopt;
    }

    PyRegSurroundedAdd::Core::KeySet result;
    for (auto key : keysToTrack) {
        result.insert(py::str(key).cast<std::string>());
    }

    return result;
}

}


std::pair<std::string, std::string> parseAddition(const py::object &additionObj) {
    py::sequence seq = additionObj.cast<py::sequence>();
    if (seq.size() < 2) {
        throw py::type_error("A RegSurroundedAdd 'addition' must be a (key, value) tuple");
    }

    return {py::str(seq[0]).cast<std::string>(), py::str(seq[1]).cast<std::string>()};
}


PyRegSurroundedAdd::Core::RegMap parseRegMap(const py::object &regsObj) {
    PyRegSurroundedAdd::Core::RegMap result;
    if (regsObj.is_none()) {
        return result;
    }

    py::dict regs = regsObj.cast<py::dict>();
    for (auto item : regs) {
        std::string reg = py::str(item.first).cast<std::string>();
        py::object pred = py::reinterpret_borrow<py::object>(item.second);

        // None (or anything non-callable) means "accept any value" -- an empty Predicate, matching
        // the pure-Python original's own None-means-any-occurence convention.
        if (pred.is_none() || !PyCallable_Check(pred.ptr())) {
            result[reg] = {};
            continue;
        }

        result[reg] = [pred](const std::string &val) {
            return pred(py::cast(val)).cast<bool>();
        };
    }

    return result;
}


PyRegSurroundedAdd::PyRegSurroundedAdd(py::object additionObj, py::object beforeRegsObj, py::object afterRegsObj, bool latest):
    Core(parseAddition(additionObj), parseRegMap(beforeRegsObj), parseRegMap(afterRegsObj), latest),
    beforeRegsObj(beforeRegsObj.is_none() ? py::dict() : beforeRegsObj.cast<py::dict>()),
    afterRegsObj(afterRegsObj.is_none() ? py::dict() : afterRegsObj.cast<py::dict>()) {}


void initCppRegSurroundedAdd(pybind11::module_ &m) {
    py::class_<PyRegSurroundedAdd, PyBaseIniGraphEdit, py::smart_holder> cls(m, "RegSurroundedAdd", R"doc(
This class inherits from :class:`BaseIniGraphEdit`

Adds a `KVP`_ into some caller/callee graph of :class:`IniSectionGraph`, at every location that is
`surrounded` by a particular set of registers: after every register specified at 'beforeRegs' has
been seen at least once (and accepted by its predicate) and before every register specified at
'afterRegs' has been seen at least once (and accepted by its predicate)

Parameters
----------
addition: Tuple[:class:`str`, :class:`str`]
    The `KVP`_ to add

beforeRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
    The registers that must come before :attr:`addition` (ie. :attr:`addition` gets added after
    these registers) :raw-html:`<br />` :raw-html:`<br />`

    * The keys are the names of the registers
    * The values are the predicates for which particular occurence of the register to accept,
      taking in the value of the occurence -- ``None`` accepts any occurence

    This condition is only satisfied once at least one accepted occurence has been seen for
    **every** key specified in this argument :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

afterRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
    The registers that must come after :attr:`addition` -- same format/semantics as
    :attr:`beforeRegs`, except the condition applies for coming after :attr:`addition` instead of
    before it :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

latest: :class:`bool`
    Whether to add :attr:`addition` at the latest valid location within the surrounded window,
    instead of the earliest one :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc");

    // py::init(factory) rather than py::init<...>(): the core class owns std::function members
    // through its own beforeRegs/afterRegs predicate maps, and a factory returning a unique_ptr
    // avoids ever needing to move-construct the class itself -- see PyGraphRemove.cpp's identical
    // note.
    cls.def(py::init([](py::object addition, py::object beforeRegs, py::object afterRegs, bool latest) {
        return std::make_unique<PyRegSurroundedAdd>(std::move(addition), std::move(beforeRegs), std::move(afterRegs), latest);
    }), py::arg("addition"), py::arg("beforeRegs") = py::none(), py::arg("afterRegs") = py::none(), py::arg("latest") = false);

    cls.def_property("addition", [](const PyRegSurroundedAdd &self) {
        return py::make_tuple(self.addition.first, self.addition.second);
    }, [](PyRegSurroundedAdd &self, py::object addition) {
        self.addition = parseAddition(addition);
    }, py::doc(R"doc(
Tuple[:class:`str`, :class:`str`]: The `KVP`_ to add
    )doc"));

    // Reassigning either of these does not rebuild the base class's own private filter/tracked-key
    // state -- see this class's own top-level note for why that faithfully mirrors the pure-Python
    // original's identical staleness.
    cls.def_property("beforeRegs", [](const PyRegSurroundedAdd &self) {
        return self.beforeRegsObj;
    }, [](PyRegSurroundedAdd &self, py::object beforeRegs) {
        self.beforeRegsObj = beforeRegs.is_none() ? py::dict() : beforeRegs.cast<py::dict>();
        self.beforeRegs = parseRegMap(self.beforeRegsObj);
    }, py::doc(R"doc(
Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]: The registers that must come
before :attr:`addition`
    )doc"));

    cls.def_property("afterRegs", [](const PyRegSurroundedAdd &self) {
        return self.afterRegsObj;
    }, [](PyRegSurroundedAdd &self, py::object afterRegs) {
        self.afterRegsObj = afterRegs.is_none() ? py::dict() : afterRegs.cast<py::dict>();
        self.afterRegs = parseRegMap(self.afterRegsObj);
    }, py::doc(R"doc(
Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]: The registers that must come
after :attr:`addition`
    )doc"));

    cls.def_readwrite("latest", &PyRegSurroundedAdd::latest, py::doc(R"doc(
:class:`bool`: Whether to add :attr:`addition` at the latest valid location within the surrounded
window, instead of the earliest one
    )doc"));

    cls.def("edit", [](PyRegSurroundedAdd &self, py::object graph, const py::object &modType,
                       const std::string &modName, const py::object &partFilter, bool trackKeys,
                       const py::object &keysToTrack) {
        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);

        // The C++ core takes 'modType' as a nullable ModType*, and the Python-side ModType is a
        // pure-Python class with no C++ counterpart to cast to, so nullptr is the only honest thing
        // to pass down that parameter -- the *Python* object is captured by the wrapped partFilter
        // instead, which is what actually gets to read it.
        self.Core::edit(parsedGraph, nullptr, modName, parsePartFilter(partFilter, modType, py::none()),
                        trackKeys, parseKeysToTrack(keysToTrack));

        // This edit only ever inserts a KVP into an already-existing IfContentPart (addKVPAt), never
        // a brand-new one -- unlike RegFillMissing's TopdownCover/GraphRename's rebuild, so no
        // refreshKeepAlive() is needed here.

        // Returns the original Python object rather than py::cast()-ing the C++ reference back, so
        // 'result is graph' holds (matching the pure-Python original's plain 'return graph').
        return graph;
    }, py::arg("graph"), py::arg("modType"), py::arg("modName") = "", py::arg("partFilter") = py::none(),
       py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(),
       py::doc(R"doc(
Fills 'graph' with a `surrounded` window insertion of :attr:`addition`, honouring :attr:`latest`
for which valid location within each window is chosen

.. note::
    'trackKeys'/'keysToTrack' are the caller's key-tracking defaults, handed down by
    :class:`BaseIniGraphEdit`'s contract (:class:`GraphGroupEdit` passes its own). This edit builds
    its own colourings from its own :attr:`beforeRegs`/:attr:`afterRegs`, so it has no use for
    them -- they are accepted only so the shared call convention keeps working

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
