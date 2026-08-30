#include "PyGraphGroupEdit.h"

#include <utility>

#include "../graphEdits/PyBaseIniGraphEdit.h"  // reuses PyBaseIniGraphEdit (the isinstance
                                                // target for the graph-edit half)
#include "../regEdits/PyBaseRegEdit.h"  // reuses PyBaseRegEdit (the isinstance target for the
                                        // register-edit half) and PyPartRanges (the "a bound
                                        // Ranges, or a raw list of bounds" resolver)


namespace {

// The pure-Python original handed a 'defaultFilter' returning Ranges.createFull() to any edit
// with no filter of its own. Built once, on first use, so a graph edit that reads its 'partFilter'
// argument unconditionally still gets a real callable rather than None.
//
// Held through a deliberately-never-deleted pointer, NOT as a plain 'static py::object'. A
// function-local static holding a Python reference is destroyed during C++ static destruction,
// which on CPython runs *after* Py_Finalize -- decref'ing an object into an already-torn-down
// interpreter. Confirmed the hard way: the plain-static version printed its results correctly and
// then exited with 0xC0000005 (access violation) with no traceback, so it looks like a clean run
// unless the process's exit code is actually checked.
py::object defaultPartFilter() {
    static py::object *filter = nullptr;
    if (filter == nullptr) {
        filter = new py::object(py::cpp_function([](const py::object &, const py::object &, const py::object &) {
            // PyRanges<long long>, not the bare AGRC::Ranges -- only the former is a registered
            // Python type, and returning the core type raises "Unregistered type" at call time
            // (not at bind time), so a plain import check never catches it.
            AGRC::Ranges<long long> full = AGRC::Ranges<long long>::createFull();
            return PyRanges<long long>(full.ranges, false);
        }));
    }

    return *filter;
}


// Converts a core key set back into a real Python set, for handing a graph edit the group's own
// 'keysToTrack'. The keys are already py::object for this instantiation, so nothing is re-parsed.
py::set keySetToPy(const PyGraphGroupEdit::KeySet &keys) {
    py::set result;
    for (const py::object &key : keys) {
        result.add(key);
    }
    return result;
}


// Forwards one entry of a graph's edit list to whatever Python object the caller actually put
// there. See AGRC::GraphGroupEdit::PartEdit's own note for why the dispatch (rather than a C++
// base pointer) is the abstraction.
class PyPartEdit: public PyGraphGroupEdit::PartEdit {
    public:
        PyPartEdit(py::object filter, PyGraphGroupEdit &owner, PyIniGraphGroups &groups, py::object ini, py::object modType):
            filter_(std::move(filter)), owner_(owner), groups_(groups), ini_(std::move(ini)), modType_(std::move(modType)) {
            if (py::isinstance<PyBaseIniGraphEdit>(filter_)) {
                kind_ = PyGraphGroupEdit::PartEditKind::GraphEdit;
            } else if (py::isinstance<PyBaseRegEdit>(filter_)) {
                kind_ = PyGraphGroupEdit::PartEditKind::RegEdit;
            }
        }

        PyGraphGroupEdit::PartEditKind kind() const override {
            return kind_;
        }

        PyGraphGroupEdit::Graph* editGraph(PyGraphGroupEdit::Graph &graph, AGRC::IniFile *, const AGRC::ModType *,
                                            const std::string &modName, const PyGraphGroupEdit::PartFilter *keyFilter,
                                            bool trackKeys, const std::optional<PyGraphGroupEdit::KeySet> &keysToTrack) override {
            // 'ini'/'modType' come from the captured Python objects, not the (always null) C++
            // parameters -- the Python-side IniFile/ModType are pure-Python classes with no C++
            // counterpart to cast to.
            py::object graphObj = groups_.graphToPy(&graph);
            py::object partFilter = owner_.partFilterToPy(keyFilter);
            py::object result;

            // The group's own key-tracking settings are handed down as keyword arguments: a graph
            // edit walks the graph itself, so nothing GraphGroupEdit builds would otherwise reach
            // it. An edit with its own setting combines the two (see RegFillMissing); one without
            // simply ignores them.
            py::object keysToTrackObj = py::none();
            if (keysToTrack.has_value()) {
                keysToTrackObj = keySetToPy(*keysToTrack);
            }

            if (!ini_.is_none()) {
                result = filter_.attr("editFromIni")(graphObj, ini_, modType_, py::arg("modName") = modName,
                                                     py::arg("partFilter") = partFilter,
                                                     py::arg("trackKeys") = trackKeys,
                                                     py::arg("keysToTrack") = keysToTrackObj);
            } else {
                result = filter_.attr("edit")(graphObj, modType_, py::arg("modName") = modName,
                                              py::arg("partFilter") = partFilter,
                                              py::arg("trackKeys") = trackKeys,
                                              py::arg("keysToTrack") = keysToTrackObj);
            }

            if (result.is_none() || result.is(graphObj)) {
                return &graph;
            }

            // A graph edit that genuinely hands back a *different* graph -- register it with the
            // view so the core can keep working with it as a plain pointer.
            return groups_.adopt(std::move(result));
        }

        void editPart(PyGraphGroupEdit::ContentPart &part, const std::string &sectionName, AGRC::IniFile *, const AGRC::ModType *,
                       const std::string &modName, const PyGraphGroupEdit::OrderRanges &partRanges) override {
            py::object partObj = py::cast(&part, py::return_value_policy::reference);
            // Rebuilt with normalize = false from the already-normalized range list -- the same
            // shape PyIfContentPartColour.cpp uses to hand a core Ranges back to Python.
            py::object rangesObj = py::cast(PyRanges<long long>(partRanges.ranges, false));

            if (!ini_.is_none()) {
                filter_.attr("editFromIni")(partObj, sectionName, ini_, modType_, py::arg("modName") = modName,
                                            py::arg("partRanges") = rangesObj);
            } else {
                filter_.attr("edit")(partObj, sectionName, modType_, py::arg("modName") = modName,
                                     py::arg("partRanges") = rangesObj);
            }
        }

    private:
        py::object filter_;
        PyGraphGroupEdit &owner_;
        PyIniGraphGroups &groups_;
        py::object ini_;
        py::object modType_;
        PyGraphGroupEdit::PartEditKind kind_ = PyGraphGroupEdit::PartEditKind::None;
};


// Reads one entry out of a Python list, or returns None when the index is past the end -- the
// direct equivalent of the pure-Python original's own "{} if (i >= someLen) else someList[i]".
py::object entryAt(const py::object &container, std::size_t index) {
    if (container.is_none()) {
        return py::none();
    }

    py::sequence seq = container.cast<py::sequence>();
    if (index >= static_cast<std::size_t>(seq.size())) {
        return py::none();
    }

    return py::reinterpret_borrow<py::object>(seq[index]);
}


py::object dictGet(const py::object &container, const py::object &key) {
    if (container.is_none()) {
        return py::none();
    }

    return container.attr("get")(key, py::none());
}

}


PyGraphGroupEdit::PyGraphGroupEdit(py::object editsObj, py::object trackKeysObj, py::object keysToTrackObj, py::object keyFiltersObj):
    Core({}), editsObj(std::move(editsObj)), trackKeysObj(std::move(trackKeysObj)),
    keysToTrackObj(std::move(keysToTrackObj)), keyFiltersObj(std::move(keyFiltersObj)) {}


py::object PyGraphGroupEdit::partFilterToPy(const PartFilter *keyFilter) const {
    if (keyFilter == nullptr) {
        return defaultPartFilter();
    }

    auto it = partFilterObjs_.find(static_cast<const void*>(keyFilter));
    if (it == partFilterObjs_.end()) {
        return defaultPartFilter();
    }

    return it->second;
}


void PyGraphGroupEdit::refresh(PyIniGraphGroups &groups, py::object ini, py::object modType) {
    partEdits_.clear();
    partFilterObjs_.clear();

    std::vector<IniEdits> parsed;

    // 'trackKeys' is either one bool for every graph or the granular per-.ini-file structure --
    // exactly the pure-Python original's own 'isinstance(self.trackKeys, bool)' fork.
    trackKeysIsGlobal = py::isinstance<py::bool_>(trackKeysObj);
    trackKeysGlobal = trackKeysIsGlobal ? trackKeysObj.cast<bool>() : false;

    std::size_t editsLen = editsObj.is_none() ? 0 : static_cast<std::size_t>(py::len(editsObj));

    // The PartFilter -> Python-callable map is filled in a second pass below, for the same reason
    // PyGraphGroupRemap's own rename-function map is: it is keyed by the address of the *stored*
    // std::function, which every reallocation of 'parsed' would invalidate.
    for (std::size_t i = 0; i < editsLen; ++i) {
        py::object iniEditsObj = entryAt(editsObj, i);
        py::object iniKeyFiltersObj = entryAt(keyFiltersObj, i);
        py::object iniKeysToTrackObj = entryAt(keysToTrackObj, i);
        py::object iniTrackKeysObj = trackKeysIsGlobal ? py::none() : entryAt(trackKeysObj, i);

        IniEdits iniEdits;
        if (iniEditsObj.is_none()) {
            parsed.push_back(std::move(iniEdits));
            continue;
        }

        for (auto item : iniEditsObj.cast<py::dict>()) {
            py::object modObjKey = py::reinterpret_borrow<py::object>(item.first);
            ModObj modObj = PyIniGraphGroups::modObjFromPy(modObjKey);

            std::vector<PartEdit*> filters;
            for (auto filterItem : py::reinterpret_borrow<py::object>(item.second)) {
                partEdits_.push_back(std::make_unique<PyPartEdit>(py::reinterpret_borrow<py::object>(filterItem), *this,
                                                                  groups, ini, modType));
                filters.push_back(partEdits_.back().get());
            }

            py::object objKeyFilters = dictGet(iniKeyFiltersObj, modObjKey);

            // A single callable (rather than a list) means "use this same filter for every edit of
            // this graph" -- the pure-Python original's own 'if (callable(objKeyFilters))' branch.
            // Kept on this side of the boundary because "is this a list or a callable?" is a
            // Python-typing question with no C++ equivalent.
            std::vector<py::object> objKeyFilterList;
            if (!objKeyFilters.is_none() && py::hasattr(objKeyFilters, "__call__")) {
                objKeyFilterList.assign(filters.size(), objKeyFilters);
            } else if (!objKeyFilters.is_none()) {
                for (auto keyFilterItem : objKeyFilters) {
                    objKeyFilterList.push_back(py::reinterpret_borrow<py::object>(keyFilterItem));
                }
            }

            std::vector<PartFilter> keyFilters;
            for (py::object &keyFilterObj : objKeyFilterList) {
                if (keyFilterObj.is_none()) {
                    keyFilters.emplace_back();
                    continue;
                }

                py::object heldKeyFilter = keyFilterObj;
                py::object heldModType = modType;
                py::object heldIni = ini;
                keyFilters.emplace_back([heldKeyFilter, heldModType, heldIni](const IterData &iterData, const AGRC::ModType *,
                                                                              AGRC::IniFile *) -> OrderRanges {
                    py::object result = heldKeyFilter(py::cast(&iterData, py::return_value_policy::reference), heldModType, heldIni);

                    PyPartRanges ranges(result);
                    const OrderRanges *parsedRanges = ranges.get();
                    if (parsedRanges == nullptr) {
                        throw py::type_error("A GraphGroupEdit keyFilter must return a Ranges (or a list of (start, end) bounds), not None");
                    }

                    return *parsedRanges;
                });
            }

            std::optional<KeySet> objKeysToTrack;
            py::object keysToTrackVal = dictGet(iniKeysToTrackObj, modObjKey);
            if (!keysToTrackVal.is_none()) {
                KeySet keys;
                for (auto keyItem : keysToTrackVal) {
                    keys.insert(py::reinterpret_borrow<py::object>(keyItem));
                }
                objKeysToTrack = std::move(keys);
            }

            bool objTrackKeys = false;
            if (!trackKeysIsGlobal) {
                py::object trackKeysVal = dictGet(iniTrackKeysObj, modObjKey);
                objTrackKeys = trackKeysVal.is_none() ? false : trackKeysVal.cast<bool>();
            }

            iniEdits.edits.insert_or_assign(modObj, std::move(filters));
            iniEdits.keyFilters.insert_or_assign(modObj, std::move(keyFilters));
            iniEdits.keysToTrack.insert_or_assign(modObj, std::move(objKeysToTrack));
            iniEdits.trackKeys.insert_or_assign(modObj, objTrackKeys);
        }

        parsed.push_back(std::move(iniEdits));
    }

    edits = std::move(parsed);

    // Now that 'edits' has stopped growing, record which Python callable each stored PartFilter
    // was built from -- see this class's own note on why the address is the key.
    std::size_t iniInd = 0;
    for (IniEdits &iniEdits : edits) {
        py::object iniKeyFiltersObj = entryAt(keyFiltersObj, iniInd);

        for (const auto &entry : iniEdits.keyFilters) {
            py::object modObjKey = PyIniGraphGroups::modObjToPy(entry.first);
            py::object objKeyFilters = dictGet(iniKeyFiltersObj, modObjKey);
            if (objKeyFilters.is_none()) {
                continue;
            }

            std::vector<py::object> objKeyFilterList;
            if (py::hasattr(objKeyFilters, "__call__")) {
                objKeyFilterList.assign(entry.second.size(), objKeyFilters);
            } else {
                for (auto keyFilterItem : objKeyFilters) {
                    objKeyFilterList.push_back(py::reinterpret_borrow<py::object>(keyFilterItem));
                }
            }

            std::vector<PartFilter> &storedFilters = iniEdits.keyFilters.at(entry.first);
            for (std::size_t j = 0; j < storedFilters.size() && j < objKeyFilterList.size(); ++j) {
                if (objKeyFilterList[j].is_none()) {
                    continue;
                }

                partFilterObjs_[static_cast<const void*>(&storedFilters[j])] = objKeyFilterList[j];
            }
        }

        ++iniInd;
    }
}


void initCppGraphGroupEdit(pybind11::module_ &m) {
    py::class_<PyGraphGroupEdit, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphGroupEdit", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Edits the individual :class:`IniSectionGraph` from a group of graphs

Parameters
----------
edits: List[Dict[Tuple[:class:`str`, :class:`str`], List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`]]]]
    The specific edits to make on the individual graphs :raw-html:`<br />` :raw-html:`<br />`

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

    // py::init(factory) rather than py::init<...>(): the core class owns std::function members, and
    // a factory returning a unique_ptr avoids ever needing to move-construct the class itself --
    // see PyRegAdd.cpp's identical note.
    cls.def(py::init([](py::object edits, py::object trackKeys, py::object keysToTrack, py::object keyFilters) {
        return std::make_unique<PyGraphGroupEdit>(std::move(edits), std::move(trackKeys), std::move(keysToTrack),
                                                   std::move(keyFilters));
    }), py::arg("edits"), py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(),
        py::arg("keyFilters") = py::none());

    cls.def_property("edits", [](const PyGraphGroupEdit &self) {
        return self.editsObj;
    }, [](PyGraphGroupEdit &self, py::object edits) {
        self.editsObj = std::move(edits);
    }, py::doc(R"doc(
List[Dict[Tuple[:class:`str`, :class:`str`], List[Union[:class:`BaseIniGraphEdit`, :class:`BaseRegEdit`]]]]:
The specific edits to make on the individual graphs
    )doc"));

    cls.def_property("trackKeys", [](const PyGraphGroupEdit &self) {
        return self.trackKeysObj;
    }, [](PyGraphGroupEdit &self, py::object trackKeys) {
        self.trackKeysObj = std::move(trackKeys);
    }, py::doc(R"doc(
Union[:class:`bool`, List[Dict[Tuple[:class:`str`, :class:`str`], :class:`bool`]]]: Whether to track
the `KVPs`_ in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits`
    )doc"));

    cls.def_property("keysToTrack", [](const PyGraphGroupEdit &self) {
        return self.keysToTrackObj;
    }, [](PyGraphGroupEdit &self, py::object keysToTrack) {
        self.keysToTrackObj = std::move(keysToTrack);
    }, py::doc(R"doc(
List[Dict[Tuple[:class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]: Specific keys to track
in the .ini file for any :class:`BaseRegEdit` passed into :attr:`edits`
    )doc"));

    cls.def_property("keyFilters", [](const PyGraphGroupEdit &self) {
        return self.keyFiltersObj;
    }, [](PyGraphGroupEdit &self, py::object keyFilters) {
        self.keyFiltersObj = std::move(keyFilters);
    }, py::doc(R"doc(
List[Dict[Tuple[:class:`str`, :class:`str`], Union[List[Optional[Callable]], Callable]]]: Functions
for any :class:`BaseRegEdit` to only process on specific parts of a `section`_
    )doc"));

    cls.def("edit", [](PyGraphGroupEdit &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        PyIniGraphGroups groups(graphGroups);
        self.refresh(groups, py::none(), modType);

        // nullptr for both C++ collaborators: the real Python objects are captured by the
        // PartEdit adapters instead -- see PyPartEdit.
        self.Core::edit(groups, nullptr, modName);
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
        self.refresh(groups, ini, modType);

        self.Core::editFromIni(groups, nullptr, nullptr, modName);
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
