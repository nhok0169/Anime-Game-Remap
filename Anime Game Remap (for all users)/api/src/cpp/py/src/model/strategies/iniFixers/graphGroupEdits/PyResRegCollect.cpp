#include "PyResRegCollect.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../regEdits/PyBaseRegEdit.h"  // reuses PyPartRanges (the "a bound Ranges, or a raw list
                                        // of bounds" resolver every predicate result goes through)


namespace {

using PyResRegCollectCore = AGRC::ResRegCollect<py::object, py::object, PyObjectHash, PyObjectEqual>;


// Turns one Python remap target -- a (iniIndex, component, object) tuple, optionally with a rename
// callable as a 4th element -- into the core's own target. Shared shape with PyGraphGroupRemap's
// own parsing, kept separate because that one additionally has to remember the Python callable for
// its remapGraphs callback; nothing here ever hands a rename function back to Python.
PyResRegCollectCore::Remapper::RemapTarget parseRemapTarget(const py::object &targetObj) {
    PyResRegCollectCore::Remapper::RemapTarget result(parseGraphId(targetObj));

    py::sequence targetSeq = targetObj.cast<py::sequence>();
    if (targetSeq.size() > 3) {
        py::object renameFuncObj = py::reinterpret_borrow<py::object>(targetSeq[3]);
        if (!renameFuncObj.is_none()) {
            result.renameFunc = [renameFuncObj](const std::string &oldSectionName) {
                return renameFuncObj(py::str(oldSectionName)).cast<std::string>();
            };
        }
    }

    return result;
}

}


PyResRegCollect::PyResRegCollect(py::object srcRegs, py::object resEdits, py::object partPredicates, py::object resPredicates,
                                  py::object remaps, py::object trackKeys, py::object keysToTrack):
    srcRegsObj(std::move(srcRegs)), resEditsObj(std::move(resEdits)), partPredicatesObj(std::move(partPredicates)),
    resPredicatesObj(std::move(resPredicates)), remapsObj(std::move(remaps)), trackKeysObj(std::move(trackKeys)),
    keysToTrackObj(std::move(keysToTrack)) {
    refresh();
}


void PyResRegCollect::refresh() {
    srcRegs.clear();
    resEdits.clear();
    partPredicates.clear();
    resPredicates.clear();
    remaps.clear();
    trackKeys.clear();
    keysToTrack.clear();

    if (!srcRegsObj.is_none()) {
        for (auto item : srcRegsObj.cast<py::dict>()) {
            srcRegs.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                      py::reinterpret_borrow<py::object>(item.second));
        }
    }

    if (!resEditsObj.is_none()) {
        for (auto item : resEditsObj.cast<py::dict>()) {
            // A pure-Python subclass of BaseResEdit still holds a real C++ subobject, so this cast
            // reaches it too. A duck-typed class that does *not* derive from BaseResEdit at all
            // cannot be used here -- unlike the pure-Python original, which never checked.
            resEdits.insert_or_assign(py::str(item.first).cast<std::string>(),
                                       py::reinterpret_borrow<py::object>(item.second).cast<PyBaseResEditCore*>());
        }
    }

    if (!partPredicatesObj.is_none()) {
        for (auto item : partPredicatesObj.cast<py::dict>()) {
            py::object predicate = py::reinterpret_borrow<py::object>(item.second);
            if (predicate.is_none()) {
                continue;
            }

            partPredicates.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                             [predicate](const IterData &iterData) -> OrderRanges {
                py::object result = predicate(py::cast(&iterData, py::return_value_policy::reference));

                PyPartRanges ranges(result);
                const OrderRanges *parsedRanges = ranges.get();
                if (parsedRanges == nullptr) {
                    throw py::type_error("A ResRegCollect partPredicate must return a Ranges (or a list of (start, end) bounds), not None");
                }

                return *parsedRanges;
            });
        }
    }

    if (!resPredicatesObj.is_none()) {
        for (auto item : resPredicatesObj.cast<py::dict>()) {
            py::object predicate = py::reinterpret_borrow<py::object>(item.second);
            if (predicate.is_none()) {
                continue;
            }

            resPredicates.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                            [predicate](const py::object &reg, const py::object &val, const IterData &iterData) {
                return predicate(reg, val, py::cast(&iterData, py::return_value_policy::reference)).cast<bool>();
            });
        }
    }

    if (!remapsObj.is_none()) {
        for (auto item : remapsObj.cast<py::dict>()) {
            tsl::ordered_map<std::string, Remapper::RemapTarget> targets;

            for (auto targetItem : py::reinterpret_borrow<py::object>(item.second).cast<py::dict>()) {
                targets.insert_or_assign(py::str(targetItem.first).cast<std::string>(),
                                          parseRemapTarget(py::reinterpret_borrow<py::object>(targetItem.second)));
            }

            remaps.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)), std::move(targets));
        }
    }

    // 'trackKeys' is either one bool for every graph or the granular per-graph dict -- exactly the
    // pure-Python original's own 'isinstance(trackKeys, bool)' fork.
    trackKeysIsGlobal = py::isinstance<py::bool_>(trackKeysObj);
    trackKeysGlobal = trackKeysIsGlobal ? trackKeysObj.cast<bool>() : false;

    if (!trackKeysIsGlobal && !trackKeysObj.is_none()) {
        for (auto item : trackKeysObj.cast<py::dict>()) {
            trackKeys.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                        py::reinterpret_borrow<py::object>(item.second).cast<bool>());
        }
    }

    if (!keysToTrackObj.is_none()) {
        for (auto item : keysToTrackObj.cast<py::dict>()) {
            py::object keysObj = py::reinterpret_borrow<py::object>(item.second);
            std::optional<std::unordered_set<py::object, PyObjectHash, PyObjectEqual>> keys;

            if (!keysObj.is_none()) {
                std::unordered_set<py::object, PyObjectHash, PyObjectEqual> parsedKeys;
                for (auto keyItem : keysObj) {
                    parsedKeys.insert(py::reinterpret_borrow<py::object>(keyItem));
                }
                keys = std::move(parsedKeys);
            }

            keysToTrack.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)), std::move(keys));
        }
    }
}


py::object PyResRegCollect::resCallsToPy() const {
    py::dict result;

    for (const auto &srcEntry : resCalls) {
        py::dict sections;

        for (const auto &sectionEntry : srcEntry.second) {
            py::dict parts;

            for (const auto &partEntry : sectionEntry.second) {
                py::dict calls;
                for (const auto &call : partEntry.second) {
                    calls[py::cast(call.orderInd)] = call.val;
                }

                parts[py::cast(partEntry.first)] = std::move(calls);
            }

            sections[py::str(sectionEntry.first)] = std::move(parts);
        }

        result[graphIdToPy(srcEntry.first)] = std::move(sections);
    }

    return result;
}


void initCppResRegCollect(pybind11::module_ &m) {
    auto cls = py::class_<PyResRegCollect, PyBaseIniGraphGroupEdit, py::smart_holder>(m, "ResRegCollect", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Creates the :class:`IniSectionGraph` for a particular resource

Parameters
----------
srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]
    The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`

    The keys in the dictionary are the location of which :class:`IniSectionGraph` to search for,
    which contains:

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

resEdits: Dict[:class:`str`, :class:`BaseResEdit`]
    Describes how a resource should be built :raw-html:`<br />` :raw-html:`<br />`

    The keys are the names for the subtype of the resource and the values are the edit for each type
    of resource

partPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`Ranges`]]]
    The predicates for which particular order indices to process for some :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

resPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]]
    The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`

    Each predicate takes in:

    #. The register name that holds the reference
    #. The name of the resource reference
    #. The data that contains info on the part and its `section`_

    :raw-html:`<br />`

    **Default**: ``None``

remaps: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]]
    Whether to remap the graphs searched from :attr:`srcRegs`. The values follow the same format as
    :attr:`GraphGroupRemap.remap` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

trackKeys: Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]
    Whether to track the `KVPs`_ in the .ini file when searching for particular resources :raw-html:`<br />` :raw-html:`<br />`

    If this parameter is a boolean, this flag will be globally used for all graphs :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]
    Specific keys to track in the .ini file when searching particular resources. A ``None`` value
    tracks every key encountered :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object srcRegs, py::object resEdits, py::object partPredicates, py::object resPredicates,
                        py::object remaps, py::object trackKeys, py::object keysToTrack) {
        return std::make_unique<PyResRegCollect>(std::move(srcRegs), std::move(resEdits), std::move(partPredicates),
                                                  std::move(resPredicates), std::move(remaps), std::move(trackKeys),
                                                  std::move(keysToTrack));
    }), py::arg("srcRegs"), py::arg("resEdits"), py::arg("partPredicates") = py::none(),
        py::arg("resPredicates") = py::none(), py::arg("remaps") = py::none(), py::arg("trackKeys") = false,
        py::arg("keysToTrack") = py::none());

    cls.def_property("srcRegs", [](const PyResRegCollect &self) { return self.srcRegsObj; },
                     [](PyResRegCollect &self, py::object value) { self.srcRegsObj = std::move(value); },
                     py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]: The different registers that
reference the particular resource
    )doc"));

    cls.def_property("resEdits", [](const PyResRegCollect &self) { return self.resEditsObj; },
                     [](PyResRegCollect &self, py::object value) { self.resEditsObj = std::move(value); },
                     py::doc(R"doc(
Dict[:class:`str`, :class:`BaseResEdit`]: Describes how a resource should be built
    )doc"));

    cls.def_property("partPredicates", [](const PyResRegCollect &self) { return self.partPredicatesObj; },
                     [](PyResRegCollect &self, py::object value) { self.partPredicatesObj = std::move(value); },
                     py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`Ranges`]]:
The predicates for which particular order indices to process for some :class:`IfContentPart`
    )doc"));

    cls.def_property("resPredicates", [](const PyResRegCollect &self) { return self.resPredicatesObj; },
                     [](PyResRegCollect &self, py::object value) { self.resPredicatesObj = std::move(value); },
                     py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]:
The predicates to check whether some reference to the resource should be used
    )doc"));

    cls.def_property("remaps", [](const PyResRegCollect &self) { return self.remapsObj; },
                     [](PyResRegCollect &self, py::object value) { self.remapsObj = std::move(value); },
                     py::doc(R"doc(
Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Tuple]]]: Whether
to remap the graphs searched from :attr:`srcRegs`
    )doc"));

    cls.def_property("trackKeys", [](const PyResRegCollect &self) { return self.trackKeysObj; },
                     [](PyResRegCollect &self, py::object value) { self.trackKeysObj = std::move(value); },
                     py::doc(R"doc(
Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]: Whether
to track the `KVPs`_ in the .ini file when searching for particular resources
    )doc"));

    cls.def_property("keysToTrack", [](const PyResRegCollect &self) { return self.keysToTrackObj; },
                     [](PyResRegCollect &self, py::object value) { self.keysToTrackObj = std::move(value); },
                     py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]: Specific keys to
track in the .ini file when searching particular resources
    )doc"));

    cls.def_property_readonly("resCalls", &PyResRegCollect::resCallsToPy, py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Dict[:class:`int`, Dict[:class:`int`, :class:`str`]]]]:
The calls to the resource

* The outer keys are the id of the graph the call was found in
* The second outer keys are the names of the `sections`_
* The third outer keys are the id of the part within the `section`_
* The inner keys are the order index the resource call is found at in the part
* The values are the names of the resource `sections`_

.. note::
    This is scratch state, rebuilt by every :meth:`edit` and cleared again by :meth:`editFromIni`.
    Reading it back gives a freshly built ``dict``, not a live view
    )doc"));

    cls.def("clear", [](PyResRegCollect &self) {
        self.refresh();
        self.Core::clear();
    }, py::doc(R"doc(Clears :attr:`resCalls` and every resource edit's own saved state)doc"));

    cls.def("edit", [](PyResRegCollect &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        self.refresh();

        PyIniGraphGroups groups(graphGroups);
        // nullptr for modType: nothing in this edit reads it directly, and the Python-side ModType
        // is a pure-Python class with no C++ counterpart to cast to. The resource edits that *do*
        // need it get it through their own context instead (see editFromIni).
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Collects and remaps the references to the resource

.. note::
    With no .ini file there is nothing to build the resources *for*, so this collects and remaps but
    builds nothing -- exactly as the pure-Python original did

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

    cls.def("editFromIni", [](PyResRegCollect &self, py::list graphGroups, const py::object &ini, const py::object &modType,
                              const std::string &modName) {
        self.refresh();

        PyIniGraphGroups groups(graphGroups);
        PyIniResEditContext ctx(ini, modType);
        self.Core::editWithContext(groups, ctx, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Collects, remaps and builds the resource for 'ini'

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

ini: :class:`IniFile`
    The associated original .ini file

modType: :class:`ModType`
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
