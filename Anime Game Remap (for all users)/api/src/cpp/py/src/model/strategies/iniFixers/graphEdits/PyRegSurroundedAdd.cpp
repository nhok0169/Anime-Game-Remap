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


namespace {

// One (key, value) tuple: a 2-sequence whose two items are both strings. A list of tuples can never
// look like this, so the single-tuple convenience form is unambiguous.
bool isSingleKvp(const py::object &obj) {
    if (!py::isinstance<py::sequence>(obj) || py::isinstance<py::str>(obj)) {
        return false;
    }
    py::sequence seq = obj.cast<py::sequence>();
    return seq.size() == 2 && py::isinstance<py::str>(seq[0]) && py::isinstance<py::str>(seq[1]);
}

std::pair<std::string, std::string> parseOneKvp(const py::object &obj) {
    if (!py::isinstance<py::sequence>(obj) || py::isinstance<py::str>(obj)) {
        throw py::type_error("Each entry of 'additions' must be a (key, value) tuple");
    }
    py::sequence seq = obj.cast<py::sequence>();
    if (seq.size() != 2) {
        throw py::type_error("Each entry of 'additions' must be a (key, value) tuple");
    }
    return {py::str(seq[0]).cast<std::string>(), py::str(seq[1]).cast<std::string>()};
}

}


PyRegSurroundedAdd::Core::Additions parseAdditions(const py::object &additionsObj) {
    PyRegSurroundedAdd::Core::Additions result;
    if (additionsObj.is_none()) {
        return result;
    }

    if (isSingleKvp(additionsObj)) {
        result.push_back(parseOneKvp(additionsObj));
        return result;
    }

    if (py::isinstance<py::str>(additionsObj)) {
        throw py::type_error("'additions' must be a (key, value) tuple or a list of them");
    }

    for (auto item : additionsObj) {
        result.push_back(parseOneKvp(py::reinterpret_borrow<py::object>(item)));
    }

    return result;
}


py::list additionsToPy(const PyRegSurroundedAdd::Core::Additions &additions) {
    py::list result;
    for (const auto &[key, val] : additions) {
        result.append(py::make_tuple(key, val));
    }
    return result;
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


PyRegSurroundedAdd::PyRegSurroundedAdd(py::object additionsObj, py::object beforeRegsObj, py::object afterRegsObj, bool latest,
                                       py::object optBeforeRegsObj, py::object optAfterRegsObj):
    Core(parseAdditions(additionsObj), parseRegMap(beforeRegsObj), parseRegMap(afterRegsObj), latest, parseRegMap(optBeforeRegsObj),
         parseRegMap(optAfterRegsObj)),
    beforeRegsObj(beforeRegsObj.is_none() ? py::dict() : beforeRegsObj.cast<py::dict>()),
    afterRegsObj(afterRegsObj.is_none() ? py::dict() : afterRegsObj.cast<py::dict>()),
    optBeforeRegsObj(optBeforeRegsObj.is_none() ? py::dict() : optBeforeRegsObj.cast<py::dict>()),
    optAfterRegsObj(optAfterRegsObj.is_none() ? py::dict() : optAfterRegsObj.cast<py::dict>()) {}


void initCppRegSurroundedAdd(pybind11::module_ &m) {
    py::class_<PyRegSurroundedAdd, PyBaseIniGraphEdit, py::smart_holder> cls(m, "RegSurroundedAdd", R"doc(
This class inherits from :class:`BaseIniGraphEdit`

Adds a `KVP`_ into some caller/callee graph of :class:`IniSectionGraph`, at every location that is
`surrounded` by a particular set of registers: after every register specified at 'beforeRegs' has
been seen at least once (and accepted by its predicate) and before every register specified at
'afterRegs' has been seen at least once (and accepted by its predicate)

Parameters
----------
additions: Union[Tuple[:class:`str`, :class:`str`], List[Tuple[:class:`str`, :class:`str`]]]
    The `KVP`_ entries to add -- one ``(key, value)`` tuple, or a list of them. All of them land
    together at each chosen position, as consecutive lines in this order; an empty list makes the
    edit a no-op

beforeRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
    The registers that must come before :attr:`additions` (ie. :attr:`additions` gets added after
    these registers) :raw-html:`<br />` :raw-html:`<br />`

    * The keys are the names of the registers
    * The values are the predicates for which particular occurence of the register to accept,
      taking in the value of the occurence -- ``None`` accepts any occurence

    This condition is only satisfied once at least one accepted occurence has been seen for
    **every** key specified in this argument :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

afterRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
    The registers that must come after :attr:`additions` -- same format/semantics as
    :attr:`beforeRegs`, except the condition applies for coming after :attr:`additions` instead of
    before it :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

latest: :class:`bool`
    Whether to add :attr:`additions` at the latest valid location within the surrounded window,
    instead of the earliest one :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

optBeforeRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
    Registers of which **at least one** must come before :attr:`additions` -- same format as
    :attr:`beforeRegs`, but "any of" rather than "all of" :raw-html:`<br />` :raw-html:`<br />`

    Combined with :attr:`beforeRegs` by conjunction: the window only opens once every
    :attr:`beforeRegs` register **and** at least one of these has been seen (and accepted by its
    predicate). ``None``/empty means no extra constraint :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        Across branches/``run =`` calls each register is tracked with its own guarantee and the
        group counts as satisfied where *some* register's guarantee holds -- a position reached
        only through paths that each satisfy a *different* register of this group is not credited.
        In practice that position's own predecessor parts already claimed the window, so nothing
        is lost by the dedup that follows

    :raw-html:`<br />`

    **Default**: ``None``

optAfterRegs: Optional[Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]]
    Registers of which **at least one** must come after :attr:`additions` -- the "any of"
    counterpart of :attr:`afterRegs`, exactly as :attr:`optBeforeRegs` is to :attr:`beforeRegs`
    :raw-html:`<br />` :raw-html:`<br />`

    Combined with :attr:`afterRegs` by conjunction. Nothing is inserted at all if none of these
    registers exists anywhere in the graph (the same rule :attr:`afterRegs` applies to each of its
    own registers). ``None``/empty means no extra constraint :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    // py::init(factory) rather than py::init<...>(): the core class owns std::function members
    // through its own beforeRegs/afterRegs predicate maps, and a factory returning a unique_ptr
    // avoids ever needing to move-construct the class itself -- see PyGraphRemove.cpp's identical
    // note.
    cls.def(py::init([](py::object additions, py::object beforeRegs, py::object afterRegs, bool latest, py::object optBeforeRegs,
                        py::object optAfterRegs) {
        return std::make_unique<PyRegSurroundedAdd>(std::move(additions), std::move(beforeRegs), std::move(afterRegs), latest,
                                                    std::move(optBeforeRegs), std::move(optAfterRegs));
    }), py::arg("additions"), py::arg("beforeRegs") = py::none(), py::arg("afterRegs") = py::none(), py::arg("latest") = false,
        py::arg("optBeforeRegs") = py::none(), py::arg("optAfterRegs") = py::none());

    cls.def_property("additions", [](const PyRegSurroundedAdd &self) {
        return additionsToPy(self.additions);
    }, [](PyRegSurroundedAdd &self, py::object additions) {
        self.additions = parseAdditions(additions);
    }, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: The `KVP`_ entries to add, in order -- a single
``(key, value)`` tuple may be assigned and reads back as a one-entry list
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
before :attr:`additions`
    )doc"));

    cls.def_property("afterRegs", [](const PyRegSurroundedAdd &self) {
        return self.afterRegsObj;
    }, [](PyRegSurroundedAdd &self, py::object afterRegs) {
        self.afterRegsObj = afterRegs.is_none() ? py::dict() : afterRegs.cast<py::dict>();
        self.afterRegs = parseRegMap(self.afterRegsObj);
    }, py::doc(R"doc(
Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]: The registers that must come
after :attr:`additions`
    )doc"));

    cls.def_property("optBeforeRegs", [](const PyRegSurroundedAdd &self) {
        return self.optBeforeRegsObj;
    }, [](PyRegSurroundedAdd &self, py::object optBeforeRegs) {
        self.optBeforeRegsObj = optBeforeRegs.is_none() ? py::dict() : optBeforeRegs.cast<py::dict>();
        self.optBeforeRegs = parseRegMap(self.optBeforeRegsObj);
    }, py::doc(R"doc(
Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]: Registers of which at least
one must come before :attr:`additions` (on top of every register in :attr:`beforeRegs`)
    )doc"));

    cls.def_property("optAfterRegs", [](const PyRegSurroundedAdd &self) {
        return self.optAfterRegsObj;
    }, [](PyRegSurroundedAdd &self, py::object optAfterRegs) {
        self.optAfterRegsObj = optAfterRegs.is_none() ? py::dict() : optAfterRegs.cast<py::dict>();
        self.optAfterRegs = parseRegMap(self.optAfterRegsObj);
    }, py::doc(R"doc(
Dict[:class:`str`, Optional[Callable[[:class:`str`], :class:`bool`]]]: Registers of which at least
one must come after :attr:`additions` (on top of every register in :attr:`afterRegs`)
    )doc"));

    cls.def_readwrite("latest", &PyRegSurroundedAdd::latest, py::doc(R"doc(
:class:`bool`: Whether to add :attr:`additions` at the latest valid location within the surrounded
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
Fills 'graph' with a `surrounded` window insertion of :attr:`additions`, honouring :attr:`latest`
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
