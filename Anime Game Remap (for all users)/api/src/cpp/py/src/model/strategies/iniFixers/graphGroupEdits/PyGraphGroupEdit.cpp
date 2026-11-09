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

#include "PyGraphGroupEdit.h"
#include "PyIniGraphGroups.h"
#include "../PyEditScope.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <utility>

#include "../graphEdits/PyBaseIniGraphEdit.h"  // reuses PyBaseIniGraphEdit (the isinstance
                                                // target for the graph-edit half)
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "../../../../tools/PyRanges.h"
#include "../regEdits/PyBaseRegEdit.h"  // reuses PyBaseRegEdit (the isinstance target for the
                                        // register-edit half) and PyPartRanges (the "a bound
                                        // Ranges, or a raw list of bounds" resolver)


namespace {

using Edit = PyGraphGroupEdit;
using PartEdit = Edit::PartEdit;
using PartFilter = Edit::PartFilter;
using KeySet = Edit::KeySet;
using IterData = Edit::IterData;
using OrderRanges = Edit::OrderRanges;
using ModObj = Edit::ModObj;
using Graph = Edit::Graph;
using ContentPart = Edit::ContentPart;


// The pure-Python original handed a 'defaultFilter' returning Ranges.createFull() to any edit with no filter of its own.
// Held through a deliberately-never-deleted pointer, NOT a plain 'static py::object': a function-local static holding a
// Python reference is destroyed after Py_Finalize, which exited 0xC0000005 with no traceback.
py::object defaultPartFilter() {
    static py::object *filter = nullptr;
    if (filter == nullptr) {
        filter = new py::object(py::cpp_function([](const py::object &, const py::object &, const py::object &) {
            // PyRanges<long long>, not the bare AGRC::Ranges -- only the former is a registered Python type
            AGRC::Ranges<long long> full = AGRC::Ranges<long long>::createFull();
            return PyRanges<long long>(full.ranges, false);
        }));
    }
    return *filter;
}

py::set keySetToPy(const KeySet &keys) {
    py::set result;
    for (const std::string &key : keys) {
        result.add(key);
    }
    return result;
}

}


// shared with the other graph group edits (PyGraphGroupEdit.h)
std::shared_ptr<void> holdPy(py::object object) {
    return std::shared_ptr<void>(new py::object(std::move(object)), [](void *held) {
        py::gil_scoped_acquire gil;
        delete static_cast<py::object *>(held);
    });
}

// an object of a class the library binds, as itself (a Python subclass's type is not the registered one)
bool isBound(const py::handle &object) {
    const py::detail::type_info *info = py::detail::get_type_info(Py_TYPE(object.ptr()));
    return info != nullptr && info->type == Py_TYPE(object.ptr());
}

// the Python graphs of a call's groups, for its scope (PyEditScope)
std::function<py::object(const void *)> graphToPyOf(PyIniGraphGroups &groups) {
    return [&groups](const void *graph) { return groups.graphToPy(static_cast<const Graph *>(graph)); };
}

std::function<void *(py::object)> adoptOf(PyIniGraphGroups &groups) {
    return [&groups](py::object graph) -> void * { return groups.adopt(std::move(graph)); };
}


namespace {

// A key filter given in Python: called with the C++ iteration data, mod type and .ini file; kept as itself so the
// filter reads back (keyFilters) and a graph edit is handed the callable it was given
struct PyKeyFilter {
    py::object filter;

    OrderRanges operator()(const IterData &iterData, const AGRC::ModType *modType, AGRC::IniFile *ini) const {
        py::gil_scoped_acquire gil;
        py::object result = filter(py::cast(&iterData, py::return_value_policy::reference), editModTypeToPy(modType), editIniToPy(ini));
        PyPartRanges ranges(result);
        const OrderRanges *parsed = ranges.get();
        if (parsed == nullptr) {
            throw py::type_error("A GraphGroupEdit keyFilter must return a Ranges (or a list of (start, end) bounds), not None");
        }
        return *parsed;
    }
};

py::object keyFilterToPy(const PartFilter *keyFilter) {
    if (keyFilter != nullptr) {
        if (const PyKeyFilter *own = keyFilter->target<PyKeyFilter>(); own != nullptr) {
            return own->filter;
        }
    }
    return defaultPartFilter();
}


// An edit that is not one of the library's own (a Python subclass): called through its own Python editFromIni, with the
// C++ part or graph and the C++ mod type and .ini file cast to Python -- so it runs the same in a Python list and a C++
// one
class PyCallPartEdit: public PartEdit {
    public:
        explicit PyCallPartEdit(py::object edit): edit_(std::move(edit)) {
            if (py::isinstance<PyBaseIniGraphEdit>(edit_)) {
                kind_ = Edit::PartEditKind::GraphEdit;
            } else if (py::isinstance<PyBaseRegEdit>(edit_)) {
                kind_ = Edit::PartEditKind::RegEdit;
            }
        }

        const py::object &edit() const {
            return edit_;
        }

        Edit::PartEditKind kind() const override {
            return kind_;
        }

        Graph *editGraph(Graph &graph, AGRC::IniFile *ini, const AGRC::ModType *modType, const std::string &modName,
                         const PartFilter *keyFilter, bool trackKeys, const std::optional<KeySet> &keysToTrack) override {
            py::gil_scoped_acquire gil;
            PyEditScope *scope = PyEditScope::current();
            if (scope == nullptr || !scope->graphToPy) {
                // run from C++ (a compiled fixer's list): a graph edit of the library's runs its C++ side; a Python one
                // has no Python graph to be handed
                if (!isBound(edit_)) {
                    throw py::type_error("A graph edit written in Python runs only in a Python fixer's list, not in a compiled one");
                }
                return &edit_.cast<PyBaseIniGraphEdit *>()->editFromIni(graph, ini, modType, modName,
                                                                        keyFilter != nullptr ? *keyFilter : PartFilter(),
                                                                        trackKeys, keysToTrack);
            }
            py::object graphObj = scope->graphToPy(&graph);
            py::object result = edit_.attr("editFromIni")(graphObj, editIniToPy(ini), editModTypeToPy(modType), py::arg("modName") = modName,
                                                          py::arg("partFilter") = keyFilterToPy(keyFilter),
                                                          py::arg("trackKeys") = trackKeys,
                                                          py::arg("keysToTrack") = keysToTrack.has_value() ? py::object(keySetToPy(*keysToTrack))
                                                                                                           : py::none());
            if (result.is_none() || result.is(graphObj)) {
                return &graph;
            }

            // a graph edit that hands back a different graph: adopted into the call's groups
            return static_cast<Graph *>(scope->adoptGraph(std::move(result)));
        }

        void editPart(ContentPart &part, const std::string &sectionName, AGRC::IniFile *ini, const AGRC::ModType *modType,
                      const std::string &modName, const OrderRanges &partRanges) override {
            py::gil_scoped_acquire gil;
            edit_.attr("editFromIni")(py::cast(&part, py::return_value_policy::reference), sectionName, editIniToPy(ini),
                                      editModTypeToPy(modType), py::arg("modName") = modName,
                                      py::arg("partRanges") = py::cast(PyRanges<long long>(partRanges.ranges, false)));
        }

    private:
        py::object edit_;
        Edit::PartEditKind kind_ = Edit::PartEditKind::None;
};


// One edit of a graph's list, as the table holds it: an edit of the library's goes in as itself, any other through
// PyCallPartEdit. What the table borrows is kept alive by the GraphGroupEdit (its 'owned')
PartEdit *partEditOf(Edit &self, const py::object &edit) {
    self.owned.push_back(holdPy(edit));
    if (isBound(edit)) {
        if (py::isinstance<AGRC::RegPartEdit<>>(edit)) {
            return edit.cast<AGRC::RegPartEdit<> *>();
        }
        if (py::isinstance<PyBaseRegEdit>(edit)) {
            auto adapter = std::make_shared<AGRC::RegPartEdit<>>(edit.cast<PyBaseRegEdit *>());
            self.owned.push_back(adapter);
            return adapter.get();
        }
    }
    // a graph edit -- the library's or a Python subclass -- runs through its own editFromIni: it runs once per graph, so
    // the Python call costs nothing that matters, and it is the one path every graph edit's binding honours
    if (!py::isinstance<PyBaseIniGraphEdit>(edit) && !py::isinstance<PyBaseRegEdit>(edit)) {
        throw py::type_error("A GraphGroupEdit's edits must be a BaseIniGraphEdit, a BaseRegEdit or a RegPartEdit, not "
                             + std::string(py::str(py::type::of(edit).attr("__name__"))));
    }
    auto adapter = std::make_shared<PyCallPartEdit>(edit);
    self.owned.push_back(adapter);
    return adapter.get();
}

// the edit in a table entry, as Python reads it back
py::object partEditToPy(PartEdit *part) {
    if (auto *reg = dynamic_cast<AGRC::RegPartEdit<> *>(part); reg != nullptr) {
        return reg->sectionFilter ? py::cast(reg, py::return_value_policy::reference)
                                  : py::cast(reg->edit(), py::return_value_policy::reference);
    }
    if (auto *graphEdit = dynamic_cast<AGRC::GraphPartEdit<> *>(part); graphEdit != nullptr) {
        return py::cast(graphEdit->edit(), py::return_value_policy::reference);
    }
    if (auto *call = dynamic_cast<PyCallPartEdit *>(part); call != nullptr) {
        return call->edit();
    }
    return py::none();
}

// each .ini file's dict of a list argument, 'None' as no entries
std::vector<py::dict> perIni(const py::object &arg) {
    std::vector<py::dict> result;
    if (arg.is_none()) {
        return result;
    }
    for (auto item : arg) {
        result.push_back(item.is_none() ? py::dict() : py::reinterpret_borrow<py::dict>(item));
    }
    return result;
}

void ensureInis(Edit &self, std::size_t count) {
    if (self.edits.size() < count) {
        self.edits.resize(count);
    }
}

void setEdits(Edit &self, const py::object &edits) {
    self.owned.clear();
    const std::vector<py::dict> inis = perIni(edits);
    self.edits.resize(inis.size());
    for (std::size_t i = 0; i < inis.size(); ++i) {
        self.edits[i].edits.clear();
        for (auto item : inis[i]) {
            std::vector<PartEdit *> parts;
            for (auto edit : item.second) {
                parts.push_back(partEditOf(self, py::reinterpret_borrow<py::object>(edit)));
            }
            self.edits[i].edits.insert_or_assign(PyIniGraphGroups::modObjFromPy(item.first), std::move(parts));
        }
    }
}

void setTrackKeys(Edit &self, const py::object &trackKeys) {
    self.trackKeysIsGlobal = py::isinstance<py::bool_>(trackKeys);
    self.trackKeysGlobal = self.trackKeysIsGlobal ? trackKeys.cast<bool>() : false;
    for (Edit::IniEdits &ini : self.edits) {
        ini.trackKeys.clear();
    }
    if (self.trackKeysIsGlobal) {
        return;
    }
    const std::vector<py::dict> inis = perIni(trackKeys);
    ensureInis(self, inis.size());
    for (std::size_t i = 0; i < inis.size(); ++i) {
        for (auto item : inis[i]) {
            self.edits[i].trackKeys.insert_or_assign(PyIniGraphGroups::modObjFromPy(item.first),
                                                     !item.second.is_none() && item.second.cast<bool>());
        }
    }
}

void setKeysToTrack(Edit &self, const py::object &keysToTrack) {
    for (Edit::IniEdits &ini : self.edits) {
        ini.keysToTrack.clear();
    }
    const std::vector<py::dict> inis = perIni(keysToTrack);
    ensureInis(self, inis.size());
    for (std::size_t i = 0; i < inis.size(); ++i) {
        for (auto item : inis[i]) {
            std::optional<KeySet> keys;
            if (!item.second.is_none()) {
                keys = KeySet();
                for (auto key : item.second) {
                    keys->insert(py::cast<std::string>(key));
                }
            }
            self.edits[i].keysToTrack.insert_or_assign(PyIniGraphGroups::modObjFromPy(item.first), std::move(keys));
        }
    }
}

void setKeyFilters(Edit &self, const py::object &keyFilters) {
    for (Edit::IniEdits &ini : self.edits) {
        ini.keyFilters.clear();
    }
    const std::vector<py::dict> inis = perIni(keyFilters);
    ensureInis(self, inis.size());
    for (std::size_t i = 0; i < inis.size(); ++i) {
        for (auto item : inis[i]) {
            const ModObj obj = PyIniGraphGroups::modObjFromPy(item.first);
            py::object given = py::reinterpret_borrow<py::object>(item.second);
            std::vector<PartFilter> filters;
            // a single callable (rather than a list) is the filter of every edit of that graph
            if (!given.is_none() && py::hasattr(given, "__call__")) {
                const auto found = self.edits[i].edits.find(obj);
                const std::size_t count = found == self.edits[i].edits.end() ? 1 : found->second.size();
                filters.assign(count, PartFilter(PyKeyFilter{given}));
            } else if (!given.is_none()) {
                for (auto filter : given) {
                    filters.push_back(filter.is_none() ? PartFilter() : PartFilter(PyKeyFilter{py::reinterpret_borrow<py::object>(filter)}));
                }
            }
            self.edits[i].keyFilters.insert_or_assign(obj, std::move(filters));
        }
    }
}

}


void initCppGraphGroupEdit(pybind11::module_ &m) {
    py::class_<PyListScope, std::unique_ptr<PyListScope, py::nodelete>>(m, "EditScope", R"doc(
What a callable of an edit (a predicate, a value producer, ``sectionsOf``, a :class:`GraphVarApply` function) may read of
the run it is called in: the ``.ini`` file a parser's or fixer's list runs on, its mod type, and its parse data -- the
:class:`IniParseData` the list's results (:class:`IParseResult`, eg. :class:`GraphCollect`'s rows) are filed into. A list
is the same for every ``.ini`` file, so this is how its callables read the one they run on :raw-html:`<br />` :raw-html:`<br />`

Each is ``None`` outside a run, and when the list runs from C++)doc")
        .def_static("ini", []() { PyListScope *list = PyListScope::current(); return list == nullptr ? py::none() : list->ini; },
            R"doc(The ``.ini`` file the list runs on

Returns
-------
Optional[:class:`IniFile`]
    The file)doc")
        .def_static("modType", []() { PyListScope *list = PyListScope::current(); return list == nullptr ? py::none() : list->modType; },
            R"doc(The mod type the list runs for

Returns
-------
Optional[:class:`ModType`]
    The mod type)doc")
        .def_static("parseData", []() { PyListScope *list = PyListScope::current(); return list == nullptr ? py::none() : list->parseData; },
            R"doc(The parse data of the ``.ini`` file the list runs on: what the parser and the fixer's lists have filed so far

Returns
-------
Optional[:class:`IniParseData`]
    The data)doc");

    py::class_<AGRC::RegPartEdit<>, py::smart_holder>(m, "RegPartEdit", R"doc(
A :class:`BaseRegEdit` with the `sections`_ it applies to, by name -- an entry of a :class:`GraphGroupEdit`'s edit list
for an edit meant for only some of the `sections`_ a graph reaches (eg. only a mod's override command list)

Parameters
----------
edit: :class:`BaseRegEdit`
    The edit

sectionFilter: Optional[Callable[[:class:`str`], :class:`bool`]]
    Whether the edit applies to the `section`_ of a name. ``None`` applies it to every `section`_ :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")
        .def(py::init([](PyBaseRegEdit *edit, AGRC::RegPartEdit<>::SectionFilter sectionFilter) {
                 return AGRC::RegPartEdit<>(edit, std::move(sectionFilter));
             }), py::arg("edit"), py::arg("sectionFilter") = py::none(), py::keep_alive<1, 2>())
        .def_property_readonly("edit", [](const AGRC::RegPartEdit<> &self) { return self.edit(); },
                               py::return_value_policy::reference_internal, ":class:`BaseRegEdit`: The edit")
        .def_readwrite("sectionFilter", &AGRC::RegPartEdit<>::sectionFilter,
                       "Optional[Callable[[:class:`str`], :class:`bool`]]: Whether the edit applies to the `section`_ of a name");

    py::class_<PyGraphGroupEdit, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphGroupEdit", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Edits the individual :class:`IniSectionGraph` from a group of graphs

Parameters
----------
edits: List[Dict[Tuple[:class:`str`, :class:`str`], List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`, :class:`RegPartEdit`]]]]
    The specific edits to make on the individual graphs (a :class:`RegPartEdit` for a register edit on only some of
    the `sections`_) :raw-html:`<br />` :raw-html:`<br />`

    * Each element of the outer list contains the edits for each .ini file
    * The keys in the dictionary contain the name of the component and the name of the mod object
    * The values of the dictionary are the individual edits for the corresponding graph

trackKeys: Union[:class:`bool`, List[Dict[Tuple[:class:`str`, :class:`str`], :class:`bool`]]]
    Whether to track the `KVPs`_ in the .ini file for the edits passed into :attr:`edits` :raw-html:`<br />` :raw-html:`<br />`

    For a :class:`BaseRegEdit`, this class walks the parts itself and hands each :attr:`keyFilters`
    entry a populated :attr:`SectionIterData.colouring`. For a :class:`BaseIniGraphEdit` -- which
    walks the graph itself and so never sees a colouring this class built -- the flag is instead
    **handed down** to that edit's own ``edit``/``editFromIni`` as its ``trackKeys`` argument, for
    the edit to honour. An edit carrying its own key-tracking setting combines the two (see
    :class:`RegFillMissing`); one without simply ignores it :raw-html:`<br />` :raw-html:`<br />`

    If this parameter is a boolean, this flag will be globally used for all graphs. Otherwise, more
    granular flag setting can be made. The structure of the granular version of the data is as
    follows: :raw-html:`<br />` :raw-html:`<br />`

    * Each element of the outer list contains the edits for each .ini file
    * The keys in the dictionary contain the name of the component and the name of the mod object
    * The values of the dictionary are the values of the flag

    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[List[Dict[Tuple[:class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]]
    Specific keys to track in the .ini file for the edits passed into :attr:`edits` -- handed down
    to a :class:`BaseIniGraphEdit` the same way :attr:`trackKeys` is :raw-html:`<br />` :raw-html:`<br />`

    * Each element of the outer list contains the edits for each .ini file
    * The keys in the dictionary contain the name of the component and the name of the mod object
    * The values are the keys to track for each graph. If the value is ``None``, then will keep
      track of all the keys encountered in some :class:`IfContentPart` for that graph

    :raw-html:`<br />`

    **Default**: ``None``

keyFilters: Optional[List[Dict[Tuple[:class:`str`, :class:`str`], Union[List[Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]], Callable[[:class:`SectionIterData`], :class:`bool`]]]]]
    Functions to only process on specific parts of a `section`_ :raw-html:`<br />` :raw-html:`<br />`

    * Each element of the outer list contains the predicates for each .ini file
    * The keys are the name of the component and the name of the mod object
    * The values are functions that retrieve the ranges of valid order indices to process for some
      :class:`IfContentPart`. A single function (instead of a list) applies to every edit of that graph

    :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object edits, py::object trackKeys, py::object keysToTrack, py::object keyFilters, bool everyGroup,
                        py::object onlyWith, py::object onlyWithout) {
        auto edit = std::make_unique<PyGraphGroupEdit>();
        setEdits(*edit, edits);
        setTrackKeys(*edit, trackKeys);
        setKeysToTrack(*edit, keysToTrack);
        setKeyFilters(*edit, keyFilters);
        edit->everyGroup = everyGroup;
        edit->onlyWith = PyIniGraphGroups::modObjsFromPy(onlyWith);
        edit->onlyWithout = PyIniGraphGroups::modObjsFromPy(onlyWithout);
        return edit;
    }), py::arg("edits"), py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(),
        py::arg("keyFilters") = py::none(), py::arg("everyGroup") = false, py::arg("onlyWith") = py::none(),
        py::arg("onlyWithout") = py::none());

    cls.def_readwrite("everyGroup", &PyGraphGroupEdit::everyGroup, R"doc(
:class:`bool`: Whether the FIRST entry of :attr:`edits` (and of :attr:`keyFilters` / :attr:`keysToTrack`) applies in every
group, instead of each entry in the group of its own index -- an edit on an object wherever it lands, written before the
number of groups is known (eg. a step of a fixer's list, whose remap makes the groups). **Default**: ``False``
    )doc");

    cls.def_property("onlyWith", [](const PyGraphGroupEdit &self) -> py::object {
        return PyIniGraphGroups::modObjsToPy(self.onlyWith);
    }, [](PyGraphGroupEdit &self, const py::object &obj) {
        self.onlyWith = PyIniGraphGroups::modObjsFromPy(obj);
    }, py::doc(R"doc(
Optional[List[Tuple[:class:`str`, :class:`str`]]]: Objects -- one, or a list -- a graph of any of which, in ANY group, is what lets the edit run: nothing is
edited when no group holds one -- eg. a parser's marker graph for something only some mods have, so a list that is the
same for every mod edits only those. As :attr:`GraphCreate.onlyWith`. **Default**: ``None``
    )doc"));

    cls.def_property("onlyWithout", [](const PyGraphGroupEdit &self) -> py::object {
        return PyIniGraphGroups::modObjsToPy(self.onlyWithout);
    }, [](PyGraphGroupEdit &self, const py::object &obj) {
        self.onlyWithout = PyIniGraphGroups::modObjsFromPy(obj);
    }, py::doc(R"doc(
Optional[List[Tuple[:class:`str`, :class:`str`]]]: Objects -- one, or a list -- a graph of any of which, in ANY group, stops the edit: nothing is edited when
one is there. As :attr:`GraphCreate.onlyWithout`. **Default**: ``None``
    )doc"));

    cls.def_property("edits", [](const PyGraphGroupEdit &self) {
        py::list result;
        for (const PyGraphGroupEdit::IniEdits &ini : self.edits) {
            py::dict graphs;
            for (const auto &[obj, parts] : ini.edits) {
                py::list edits;
                for (PartEdit *part : parts) {
                    edits.append(partEditToPy(part));
                }
                graphs[PyIniGraphGroups::modObjToPy(obj)] = edits;
            }
            result.append(graphs);
        }
        return result;
    }, [](PyGraphGroupEdit &self, py::object edits) {
        setEdits(self, edits);
    }, py::doc(R"doc(
List[Dict[Tuple[:class:`str`, :class:`str`], List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`, :class:`RegPartEdit`]]]]:
The specific edits to make on the individual graphs -- read from the edit's own table, so a step of a compiled fix reads
its edits too
    )doc"));

    cls.def_property("trackKeys", [](const PyGraphGroupEdit &self) -> py::object {
        if (self.trackKeysIsGlobal) {
            return py::bool_(self.trackKeysGlobal);
        }
        py::list result;
        for (const PyGraphGroupEdit::IniEdits &ini : self.edits) {
            py::dict graphs;
            for (const auto &[obj, track] : ini.trackKeys) {
                graphs[PyIniGraphGroups::modObjToPy(obj)] = track;
            }
            result.append(graphs);
        }
        return result;
    }, [](PyGraphGroupEdit &self, py::object trackKeys) {
        setTrackKeys(self, trackKeys);
    }, py::doc(R"doc(
Union[:class:`bool`, List[Dict[Tuple[:class:`str`, :class:`str`], :class:`bool`]]]: Whether to track
the `KVPs`_ in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits`
    )doc"));

    cls.def_property("keysToTrack", [](const PyGraphGroupEdit &self) {
        py::list result;
        for (const PyGraphGroupEdit::IniEdits &ini : self.edits) {
            py::dict graphs;
            for (const auto &[obj, keys] : ini.keysToTrack) {
                graphs[PyIniGraphGroups::modObjToPy(obj)] = keys.has_value() ? py::object(keySetToPy(*keys)) : py::none();
            }
            result.append(graphs);
        }
        return result;
    }, [](PyGraphGroupEdit &self, py::object keysToTrack) {
        setKeysToTrack(self, keysToTrack);
    }, py::doc(R"doc(
List[Dict[Tuple[:class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]: Specific keys to track
in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits`
    )doc"));

    cls.def_property("keyFilters", [](const PyGraphGroupEdit &self) {
        py::list result;
        for (const PyGraphGroupEdit::IniEdits &ini : self.edits) {
            py::dict graphs;
            for (const auto &[obj, filters] : ini.keyFilters) {
                py::list given;
                for (const PartFilter &filter : filters) {
                    const PyKeyFilter *own = filter ? filter.target<PyKeyFilter>() : nullptr;
                    given.append(own == nullptr ? py::none() : own->filter);
                }
                graphs[PyIniGraphGroups::modObjToPy(obj)] = given;
            }
            result.append(graphs);
        }
        return result;
    }, [](PyGraphGroupEdit &self, py::object keyFilters) {
        setKeyFilters(self, keyFilters);
    }, py::doc(R"doc(
List[Dict[Tuple[:class:`str`, :class:`str`], Union[List[Optional[Callable]], Callable]]]: Functions
for any :class:`BaseRegEdit` to only process on specific parts of a `section`_
    )doc"));

    cls.def("edit", [](PyGraphGroupEdit &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        PyIniGraphGroups groups(graphGroups);
        PyEditScope scope(modType, py::none(), graphToPyOf(groups), adoptOf(groups));
        self.edit(groups, editModTypeFromPy(modType), modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Edits a group of caller/callee graphs

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after editing
    )doc"));

    cls.def("editFromIni", [](PyGraphGroupEdit &self, py::list graphGroups, const py::object &ini, const py::object &modType,
                              const std::string &modName) {
        PyIniGraphGroups groups(graphGroups);
        PyEditScope scope(modType, ini, graphToPyOf(groups), adoptOf(groups));
        self.editFromIni(groups, editIniFromPy(ini), editModTypeFromPy(modType), modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Edits a group of caller/callee graphs with state info from 'ini'

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

ini: :class:`IniFile`
    The associated original .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after editing
    )doc"));
}
