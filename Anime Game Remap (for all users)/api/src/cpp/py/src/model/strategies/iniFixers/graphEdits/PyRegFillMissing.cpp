#include "PyRegFillMissing.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../../iftemplate/PyIfContentPart.h"
#include "../regEdits/PyBaseRegEdit.h"  // reuses PyPartRanges (the "a bound Ranges, or a raw list
                                        // of bounds" resolver) for a partFilter's return value


namespace {

// The dotted path to the still-pure-Python constants.RegFillMissingMode, derived once at module
// init from this 'core' module's own '__name__' rather than hardcoded -- a literal
// "FixRaidenBoss2...." path breaks under this repo's Unit Tester harness, which imports the whole
// package as 'src.py.FixRaidenBoss2'. Same technique (and same reason) as PyGraphGroupEdit.cpp's
// baseIniGraphEditModulePath.
std::string &regFillMissingModeModulePath() {
    static std::string path;
    return path;
}


// Reads the '.value' string a Python Enum member carries, or the object's own str() when it has no
// '.value' at all -- the shape both parse helpers below compare against.
std::string enumValueOf(const py::object &member) {
    py::object value = py::hasattr(member, "value") ? member.attr("value") : member;
    return py::str(value).cast<std::string>();
}

}


py::object pyRegFillMissingModeEnum() {
    // Never deleted: a function-local static holding a Python reference is destroyed during C++
    // static destruction, which on CPython runs *after* Py_Finalize -- decref'ing an object into an
    // already-torn-down interpreter. Same reasoning (and same confirmed 0xC0000005 crash) as
    // PyGraphGroupEdit.cpp's own never-deleted statics.
    static py::object *modeEnum = nullptr;
    if (modeEnum == nullptr) {
        modeEnum = new py::object(py::module_::import(regFillMissingModeModulePath().c_str()).attr("RegFillMissingMode"));
    }

    return *modeEnum;
}


AGRC::RegFillMissingMode parseFillMissingMode(const py::object &mode) {
    if (mode.is_none()) {
        return AGRC::RegFillMissingMode::FillMissing;
    }

    // Read through '.value' rather than by identity: RegFillMissingMode is still a pure-Python
    // Enum, so there is no C++ member to compare against -- only the string each member carries.
    // Same shape as PyResEdit.cpp's parseGraphReplaceMode.
    if (enumValueOf(mode) == "topdownCover") {
        return AGRC::RegFillMissingMode::TopdownCover;
    }

    return AGRC::RegFillMissingMode::FillMissing;
}


AGRC::DownloadMode parseIniDownloadMode(const py::object &ini) {
    if (ini.is_none() || !py::hasattr(ini, "downloadMode")) {
        return AGRC::DownloadMode::Normal;
    }

    std::string parsed = enumValueOf(ini.attr("downloadMode"));

    if (parsed == "disabled") {
        return AGRC::DownloadMode::Disabled;
    }

    if (parsed == "always") {
        return AGRC::DownloadMode::Always;
    }

    return AGRC::DownloadMode::Normal;
}


PyRegFillMissing::Core::FillMissingFunc parseFillMissing(const py::object &fillMissing, const py::object &reg, bool toFront) {
    // The isinstance checks are deliberately str-then-list, in that order, matching the
    // pure-Python original's own _getFillMissingFunc chain.
    if (py::isinstance<py::str>(fillMissing)) {
        return PyRegFillMissing::Core::makeFillMissing(reg, fillMissing, toFront);
    }

    if (py::isinstance<py::list>(fillMissing)) {
        std::vector<std::pair<py::object, py::object>> kvps;
        for (auto item : fillMissing) {
            py::sequence kvp = py::reinterpret_borrow<py::object>(item).cast<py::sequence>();
            if (kvp.size() < 2) {
                throw py::type_error("Every entry of a 'fillMissing' list must be a (key, value) tuple");
            }

            kvps.emplace_back(py::reinterpret_borrow<py::object>(kvp[0]), py::reinterpret_borrow<py::object>(kvp[1]));
        }

        return PyRegFillMissing::Core::makeFillMissing(std::move(kvps), toFront);
    }

    if (fillMissing.is_none() || !PyCallable_Check(fillMissing.ptr())) {
        return {};
    }

    // Captures the Python object by value: the resulting std::function only ever runs synchronously
    // inside a binding call, so the GIL is always held when it is invoked.
    py::object func = fillMissing;
    return [func](PyIfContentPart &part) {
        func(py::cast(&part, py::return_value_policy::reference));
    };
}


std::optional<PyRegFillMissing::Core::KeySet> parseKeysToTrack(const py::object &keysToTrack) {
    // None means "every key", not "no keys" -- see this function's own doc comment.
    if (keysToTrack.is_none()) {
        return std::nullopt;
    }

    PyRegFillMissing::Core::KeySet result;
    for (auto key : keysToTrack) {
        result.insert(py::reinterpret_borrow<py::object>(key));
    }

    return result;
}


// Publishes 'ini' on a PyRegFillMissing for the lifetime of one forwarded 'edit' call -- see
// PyRegFillMissing::currentIni's own note.
class CurrentIniScope {
    public:
        CurrentIniScope(PyRegFillMissing &edit, py::object ini): edit_(edit), previous_(edit.currentIni) {
            edit_.currentIni = std::move(ini);
        }

        ~CurrentIniScope() {
            edit_.currentIni = std::move(previous_);
        }

        CurrentIniScope(const CurrentIniScope&) = delete;
        CurrentIniScope& operator=(const CurrentIniScope&) = delete;

    private:
        PyRegFillMissing &edit_;
        py::object previous_;
};


PyRegFillMissing::Core::PartFilter parsePartFilter(const py::object &partFilter, py::object modType, py::object ini) {
    if (partFilter.is_none() || !PyCallable_Check(partFilter.ptr())) {
        return {};
    }

    // Captured by value: the resulting std::function only ever runs synchronously inside a binding
    // call, so the GIL is always held when it is invoked.
    py::object heldFilter = partFilter;
    py::object heldModType = std::move(modType);
    py::object heldIni = std::move(ini);

    return [heldFilter, heldModType, heldIni](const PyRegFillMissing::Core::IterData &iterData, const AGRC::ModType *,
                                               AGRC::IniFile *) -> PyRegFillMissing::Core::OrderRanges {
        // The captured Python modType/ini are handed over rather than the core's own (always-null)
        // pointers -- same shape, and same reason, as PyGraphGroupEdit's own keyFilters.
        py::object result = heldFilter(py::cast(&iterData, py::return_value_policy::reference), heldModType, heldIni);

        PyPartRanges ranges(result);
        const PyRegFillMissing::Core::OrderRanges *parsedRanges = ranges.get();
        if (parsedRanges == nullptr) {
            throw py::type_error("A RegFillMissing partFilter must return a Ranges (or a list of (start, end) bounds), not None");
        }

        return *parsedRanges;
    };
}


PyRegFillMissing::PyRegFillMissing(py::object regObj, py::object fillMissingObj, py::object fillModeObj, bool dependOnDownload,
                                    bool trackKeys, py::object keysToTrackObj):
    Core(std::move(regObj)), fillMissingObj(std::move(fillMissingObj)), keysToTrackObj(std::move(keysToTrackObj)) {

    // An omitted 'fillMode' materializes the real RegFillMissingMode.FillMissing member rather than
    // staying None, so reading '.fillMode' back matches what the pure-Python original's own default
    // argument stored.
    this->fillModeObj = fillModeObj.is_none() ? pyRegFillMissingModeEnum().attr("FillMissing") : std::move(fillModeObj);
    this->dependOnDownload = dependOnDownload;
    this->trackKeys = trackKeys;
}


void PyRegFillMissing::refresh() {
    fillMode = parseFillMissingMode(fillModeObj);

    // Mode and fill function are re-derived together -- see this method's own doc comment.
    bool toFront = (fillMode == AGRC::RegFillMissingMode::TopdownCover);
    fillMissing = parseFillMissing(fillMissingObj, reg, toFront);

    // Re-derived per edit for the same reason as the two above: an in-place mutation of the set the
    // caller passed has to be honoured.
    keysToTrack = parseKeysToTrack(keysToTrackObj);
}


void initCppRegFillMissing(pybind11::module_ &m) {
    // Derive "<core's own parent package>.constants.RegFillMissingMode" from 'm's actual
    // '__name__' -- see regFillMissingModeModulePath's comment for why this can't just be
    // hardcoded.
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    std::size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }

    std::string relativePath = "constants.RegFillMissingMode";
    regFillMissingModeModulePath() = parentPackage.empty() ? relativePath : (parentPackage + "." + relativePath);

    py::class_<PyRegFillMissing, PyBaseIniGraphEdit, py::smart_holder> cls(m, "RegFillMissing", R"doc(
This class inherits from :class:`BaseIniGraphEdit`

Fills the :class:`IfContentPart`\s of some caller/callee graph that are missing a particular
register

Parameters
----------
reg: :class:`str`
    The register to search for

fillMissing: Union[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]
    How to fill in the :class:`IfContentPart`\s with their corresponding values :raw-html:`<br />` :raw-html:`<br />`

    If this argument is a string, will add the following line to: ``reg = fillMissing``
    If this argument is a list of tuples, will add the `KVPs`_ specified by each tuple into the missing part
    Otherwise, will modify the missing part according to the specified function

fillMode: Optional[:class:`RegFillMissingMode`]
    What mode used to search and fill the missing register :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``RegFillMissingMode.FillMissing``

dependOnDownload: :class:`bool`
    Whether the editting is dependent on :attr:`IniFile.downloadMode` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

trackKeys: :class:`bool`
    Whether to keep track of the `KVPs`_ seen so far for colouring while walking the graph, so that
    the ``partFilter`` given to :meth:`edit` receives a populated
    :attr:`SectionIterData.colouring` to decide from :raw-html:`<br />` :raw-html:`<br />`

    When ``False``, that ``colouring`` is ``None`` and a filter can only discriminate on the part
    or the `section`_ itself :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[Set[:class:`str`]]
    Which keys 'trackKeys' should colour :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None``, then **every** key is tracked :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    // py::init(factory) rather than py::init<...>(): the core class owns std::function members
    // through its own and its base's typedefs, and a factory returning a unique_ptr avoids ever
    // needing to move-construct the class itself -- see PyGraphRemove.cpp's identical note.
    cls.def(py::init([](py::object reg, py::object fillMissing, py::object fillMode, bool dependOnDownload,
                        bool trackKeys, py::object keysToTrack) {
        return std::make_unique<PyRegFillMissing>(std::move(reg), std::move(fillMissing), std::move(fillMode),
                                                   dependOnDownload, trackKeys, std::move(keysToTrack));
    }), py::arg("reg"), py::arg("fillMissing"), py::arg("fillMode") = py::none(), py::arg("dependOnDownload") = false,
        py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none());

    cls.def_property("reg", [](const PyRegFillMissing &self) {
        return self.reg;
    }, [](PyRegFillMissing &self, py::object reg) {
        self.reg = std::move(reg);
    }, py::doc(R"doc(
:class:`str`: The register to search for
    )doc"));

    cls.def_property("fillMissing", [](const PyRegFillMissing &self) {
        return self.fillMissingObj;
    }, [](PyRegFillMissing &self, py::object fillMissing) {
        self.fillMissingObj = std::move(fillMissing);
    }, py::doc(R"doc(
Union[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]: How to
fill in the :class:`IfContentPart`\s with their corresponding values
    )doc"));

    cls.def_property("fillMode", [](const PyRegFillMissing &self) {
        return self.fillModeObj;
    }, [](PyRegFillMissing &self, py::object fillMode) {
        self.fillModeObj = std::move(fillMode);
    }, py::doc(R"doc(
:class:`RegFillMissingMode`: What mode used to search and fill the missing register
    )doc"));

    cls.def_property("dependOnDownload", [](const PyRegFillMissing &self) {
        return self.dependOnDownload;
    }, [](PyRegFillMissing &self, bool dependOnDownload) {
        self.dependOnDownload = dependOnDownload;
    }, py::doc(R"doc(
:class:`bool`: Whether the editting is dependent on :attr:`IniFile.downloadMode`
    )doc"));

    cls.def_property("trackKeys", [](const PyRegFillMissing &self) {
        return self.trackKeys;
    }, [](PyRegFillMissing &self, bool trackKeys) {
        self.trackKeys = trackKeys;
    }, py::doc(R"doc(
:class:`bool`: Whether to keep track of the `KVPs`_ seen so far for colouring while walking the
graph, so that the ``partFilter`` given to :meth:`edit` receives a populated
:attr:`SectionIterData.colouring`
    )doc"));

    cls.def_property("keysToTrack", [](const PyRegFillMissing &self) {
        return self.keysToTrackObj;
    }, [](PyRegFillMissing &self, py::object keysToTrack) {
        self.keysToTrackObj = std::move(keysToTrack);
    }, py::doc(R"doc(
Optional[Set[:class:`str`]]: Which keys :attr:`trackKeys` should colour, or ``None`` for every key
    )doc"));

    cls.def_static("fillMissingGraph", [](py::object graph, const py::object &reg, const py::object &fillMissing) {
        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);
        PyRegFillMissing::Core::fillMissingGraph(parsedGraph, reg, parseFillMissing(fillMissing, reg, false));
        return graph;
    }, py::arg("graph"), py::arg("reg"), py::arg("fillMissing"), py::doc(R"doc(
Fills the :class:`IfContentPart`\s from 'graph' that are missing 'reg'

Each part is filled at most once, even when it is reachable from more than one `section`_

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to search

reg: :class:`str`
    The register to search

fillMissing: Union[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]
    How to modify the parts that are missing the desired register -- the same three shapes
    :attr:`fillMissing` accepts

Returns
-------
:class:`IniSectionGraph`
    The same graph that was passed in, with its missing parts filled
    )doc"));

    cls.def_static("addCover", [](py::object graph, const py::object &reg, const py::object &fillMissing) {
        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);
        PyRegFillMissing::Core::addCover(parsedGraph, reg, parseFillMissing(fillMissing, reg, true));

        // addCover appends brand-new IfContentParts to each root section, which the graph's
        // Python-side keep-alive has never seen -- see PyIniSectionGraph's own note on why every
        // reachable part needs a stable, registered wrapper.
        parsedGraph.refreshKeepAlive();
        return graph;
    }, py::arg("graph"), py::arg("reg"), py::arg("fillMissing"), py::doc(R"doc(
Fills a fresh top :class:`IfContentPart` at each of 'graph''s roots, if 'reg' is missing in some
:class:`IfContentPart` of 'graph'

Nothing is added at all when every root already fully covers 'reg'

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to search

reg: :class:`str`
    The register to search

fillMissing: Union[:class:`str`, List[Tuple[:class:`str`, :class:`str`]], Callable[[:class:`IfContentPart`], Any]]
    How to modify the parts that are missing the desired register -- the same three shapes
    :attr:`fillMissing` accepts

Returns
-------
:class:`IniSectionGraph`
    The same graph that was passed in, with its roots covered
    )doc"));

    cls.def("edit", [](PyRegFillMissing &self, py::object graph, const py::object &modType,
                       const std::string &modName, const py::object &partFilter, bool trackKeys,
                       const py::object &keysToTrack) {
        self.refresh();

        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);

        // The C++ core takes 'modType' as a nullable ModType*, and the Python-side ModType is a
        // pure-Python class with no C++ counterpart to cast to, so nullptr is the only honest thing
        // to pass down that parameter. The *Python* object is captured by the wrapped partFilter
        // instead, which is what actually gets to read it.
        // The caller's trackKeys/keysToTrack are handed to the core, which combines them with this
        // edit's own -- see AGRemapCore::RegFillMissing::effectiveTrackKeys/effectiveKeysToTrack.
        self.Core::edit(parsedGraph, nullptr, modName, parsePartFilter(partFilter, modType, self.currentIni),
                        trackKeys, parseKeysToTrack(keysToTrack));

        // TopdownCover appends brand-new parts to each root -- see the addCover binding above.
        parsedGraph.refreshKeepAlive();

        // Returns the original Python object rather than py::cast()-ing the C++ reference back, so
        // 'result is graph' holds (matching the pure-Python original's plain 'return graph').
        return graph;
    }, py::arg("graph"), py::arg("modType"), py::arg("modName") = "", py::arg("partFilter") = py::none(),
       py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(),
       py::doc(R"doc(
Fills the parts of 'graph' that are missing :attr:`reg`, by whichever strategy :attr:`fillMode`
names -- :meth:`fillMissingGraph` for ``RegFillMissingMode.FillMissing``, :meth:`addCover` for
``RegFillMissingMode.TopdownCover``

'partFilter' restricts *which* parts get filled: it is asked once per candidate part, and an empty
:class:`Ranges` result skips that one. Under ``RegFillMissingMode.TopdownCover`` it is asked once
per root instead, against that root's own first :class:`IfContentPart`. This is the same convention
:class:`GraphGroupEdit` already applies to its register edits -- only *which* parts are chosen; a
non-empty result's actual ranges are not consulted, since filling a part appends a whole `KVP`_
rather than editing occurrences at particular order indices

Set :attr:`trackKeys` to give that filter a populated :attr:`SectionIterData.colouring` to decide
from, narrowed to :attr:`keysToTrack`

.. note::
    The pure-Python original accepted 'partFilter' and dropped it, so this edit applied to every
    missing part unconditionally. Honouring it is a deliberate behaviour change; an omitted
    'partFilter' still fills everything, exactly as before

.. note::
    Under ``RegFillMissingMode.TopdownCover`` the colouring handed to 'partFilter' is empty by
    construction -- nothing precedes a root -- so :attr:`SectionIterData.sectionName` /
    :attr:`SectionIterData.section` are the useful discriminators there, not the tracked `KVPs`_.
    A root `section`_ holding no :class:`IfContentPart` at all is accepted, there being nothing to
    discriminate on

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to edit

modType: Optional[:class:`ModType`]
    The type of mod to fix. Not read here -- only handed to 'partFilter'

modName: :class:`str`
    The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
    Which parts may be filled -- an empty :class:`Ranges` result skips that part, ``None`` accepts
    every part :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IniSectionGraph`
    The same graph that was passed in, after editing
    )doc"));

    // The download-mode branching lives here rather than being delegated to the C++ core's own
    // DownloadMode-taking editFromIni overload, because it has to reach :meth:`edit` through
    // genuine Python attribute lookup: with no trampoline in play, a pure-Python subclass
    // overriding only 'edit' has no C++-side vtable entry for that override, so a C++-internal
    // call would silently run this class's own implementation instead. The core overload stays for
    // plain C++ callers, which have no such indirection to preserve.
    cls.def("editFromIni", [](py::object self, py::object graph, const py::object &ini, const py::object &modType,
                              const std::string &modName, const py::object &partFilter, bool trackKeys,
                              const py::object &keysToTrack) -> py::object {
        // 'partFilter' IS forwarded, unlike in the pure-Python original (whose own editFromIni
        // called self.edit(graph, modType, modName = modName) and dropped it). Dropping it would
        // silently disable part selection for exactly the callers that supply a filter:
        // GraphGroupEdit routes through editFromIni, not edit, whenever it has an .ini file.
        PyRegFillMissing &edit = py::cast<PyRegFillMissing &>(self);

        // 'ini' is published on the instance for the duration of the forwarded 'edit' call, so the
        // partFilter is handed the real .ini file rather than None -- see PyRegFillMissing's own
        // note on why it can't simply ride along as an 'edit' argument. Restored (not just
        // cleared) on the way out so a nested edit can't clobber an outer one, and restored even
        // when the forwarded call raises.
        CurrentIniScope iniScope(edit, ini);

        if (!edit.dependOnDownload) {
            return self.attr("edit")(graph, modType, py::arg("modName") = modName,
                                     py::arg("partFilter") = partFilter, py::arg("trackKeys") = trackKeys,
                                     py::arg("keysToTrack") = keysToTrack);
        }

        AGRC::DownloadMode downloadMode = parseIniDownloadMode(ini);
        if (downloadMode == AGRC::DownloadMode::Disabled) {
            return graph;
        }

        if (downloadMode == AGRC::DownloadMode::Always) {
            PyIniSectionGraph &parsedGraph = parseGraphArg(graph);
            parsedGraph.normalize();

            // Normalizing splits sections into fresh IfContentParts the graph's Python-side
            // keep-alive has never seen -- see PyIniSectionGraph's own note.
            parsedGraph.refreshKeepAlive();
        }

        return self.attr("edit")(graph, modType, py::arg("modName") = modName,
                                 py::arg("partFilter") = partFilter, py::arg("trackKeys") = trackKeys,
                                 py::arg("keysToTrack") = keysToTrack);
    }, py::arg("graph"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "",
       py::arg("partFilter") = py::none(), py::arg("trackKeys") = false,
       py::arg("keysToTrack") = py::none(), py::doc(R"doc(
Fills the parts of 'graph' that are missing :attr:`reg`, honouring the download mode 'ini' was read
under

When :attr:`dependOnDownload` is ``False`` this is just :meth:`edit`. Otherwise
``DownloadMode.Disabled`` skips the edit entirely, and ``DownloadMode.Always`` normalizes the
graph's branching structure first, so that a part missing the register on *some* branch is
guaranteed to be its own :class:`IfContentPart`

.. note::
    An 'ini' of ``None``, or one carrying no ``downloadMode`` attribute, reads as
    ``DownloadMode.Normal`` -- the mode under which this behaves identically to
    :attr:`dependOnDownload` being ``False``

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to edit

ini: Optional[:class:`IniFile`]
    The associated .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix. Not read here -- only handed to 'partFilter'

modName: :class:`str`
    The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
    Which parts may be filled -- an empty :class:`Ranges` result skips that part. Forwarded
    straight to :meth:`edit` :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        The third argument handed to 'partFilter' is always ``None`` rather than 'ini'.
        :meth:`edit` is reached through genuine Python attribute lookup (so a subclass's own
        override still wins), and its signature -- inherited from :class:`BaseIniGraphEdit` -- has
        nowhere to carry an .ini file. A plain C++ caller of the core class does get the real one

    :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IniSectionGraph`
    The same graph that was passed in, after editing
    )doc"));
}
