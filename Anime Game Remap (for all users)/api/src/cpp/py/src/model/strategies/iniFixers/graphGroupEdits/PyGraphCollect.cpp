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

#include "PyGraphCollect.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"

#include "PyBaseIniGraphGroupEdit.h"
#include "PyIniGraphGroups.h"
#include "../regEdits/PyBaseRegEdit.h"  // reuses PyPartRanges


namespace {

std::vector<std::string> keysFromPy(const py::object& obj) {
    std::vector<std::string> keys;
    if (py::isinstance<py::str>(obj)) {
        keys.push_back(obj.cast<std::string>());
        return keys;
    }

    for (auto key : py::reinterpret_borrow<py::iterable>(obj)) {
        keys.push_back(py::str(key).cast<std::string>());
    }
    return keys;
}

}


PyGraphCollect::PyGraphCollect(py::object keys, py::object contextKeys, py::object partPredicates, py::object valPredicates,
                               py::object trackKeys, py::object keysToTrack):
    keysObj(keys.is_none() ? py::object(py::dict()) : std::move(keys)),
    contextKeysObj(contextKeys.is_none() ? py::object(py::dict()) : std::move(contextKeys)),
    partPredicatesObj(std::move(partPredicates)), valPredicatesObj(std::move(valPredicates)),
    trackKeysObj(std::move(trackKeys)), keysToTrackObj(std::move(keysToTrack)) {
    refresh();
}


void PyGraphCollect::refresh() {
    this->keys.clear();
    keyMatches.clear();
    contextKeys.clear();
    partPredicates.clear();
    valPredicates.clear();
    trackKeys.clear();
    keysToTrack.clear();

    if (!keysObj.is_none()) {
        for (auto item : keysObj.cast<py::dict>()) {
            const GraphId graphId = parseGraphId(py::reinterpret_borrow<py::object>(item.first));

            // a test of the key's name: see GraphCollect::keyMatches
            if (!py::isinstance<py::str>(item.second) && PyCallable_Check(item.second.ptr())) {
                py::object match = py::reinterpret_borrow<py::object>(item.second);
                keyMatches.insert_or_assign(graphId, [match](const std::string& key) { return match(key).cast<bool>(); });
                continue;
            }

            this->keys.insert_or_assign(graphId, keysFromPy(py::reinterpret_borrow<py::object>(item.second)));
        }
    }

    if (!contextKeysObj.is_none()) {
        for (auto item : contextKeysObj.cast<py::dict>()) {
            contextKeys.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                         keysFromPy(py::reinterpret_borrow<py::object>(item.second)));
        }
    }

    if (!partPredicatesObj.is_none()) {
        for (auto item : partPredicatesObj.cast<py::dict>()) {
            py::object predicate = py::reinterpret_borrow<py::object>(item.second);
            if (predicate.is_none()) {
                continue;
            }

            partPredicates.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                            [predicate](const IterData& iterData) -> OrderRanges {
                py::object result = predicate(py::cast(&iterData, py::return_value_policy::reference));

                PyPartRanges ranges(result);
                const OrderRanges* parsedRanges = ranges.get();
                if (parsedRanges == nullptr) {
                    throw py::type_error("A GraphCollect partPredicate must return a Ranges (or a list of (start, end) bounds), not None");
                }

                return *parsedRanges;
            });
        }
    }

    if (!valPredicatesObj.is_none()) {
        for (auto item : valPredicatesObj.cast<py::dict>()) {
            py::object predicate = py::reinterpret_borrow<py::object>(item.second);
            if (predicate.is_none()) {
                continue;
            }

            valPredicates.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                           [predicate](const std::string& key, const std::string& val, const IterData& iterData) {
                return predicate(key, val, py::cast(&iterData, py::return_value_policy::reference)).cast<bool>();
            });
        }
    }

    // one bool for every graph, or the per-graph dict -- ResRegCollect's own fork
    trackKeysIsGlobal = trackKeysObj.is_none() || py::isinstance<py::bool_>(trackKeysObj);
    trackKeysGlobal = (!trackKeysObj.is_none() && trackKeysIsGlobal) ? trackKeysObj.cast<bool>() : false;
    if (!trackKeysIsGlobal) {
        for (auto item : trackKeysObj.cast<py::dict>()) {
            trackKeys.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                       py::reinterpret_borrow<py::object>(item.second).cast<bool>());
        }
    }

    if (!keysToTrackObj.is_none()) {
        for (auto item : keysToTrackObj.cast<py::dict>()) {
            py::object keysObj = py::reinterpret_borrow<py::object>(item.second);
            std::optional<KeySet> tracked;
            if (!keysObj.is_none()) {
                KeySet parsed;
                for (auto key : keysObj) {
                    parsed.insert(py::cast<std::string>(key));
                }
                tracked = std::move(parsed);
            }

            keysToTrack.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)), std::move(tracked));
        }
    }
}


void initCppIParseResult(pybind11::module_ &m) {
    py::class_<AGRC::IParseResult, py::smart_holder>(m, "IParseResult", R"doc(
An edit with a result a parser keeps: after running an edit of :attr:`GIMIParser.parseSteps` or
:attr:`GIMIParser.deferredParseSteps`, the parser hands its :class:`IniParseData` to the edit's :meth:`fileInto`
when the edit is one of these

:class:`GraphCollect` files its rows this way
    )doc")
        .def("fileInto", &AGRC::IParseResult::fileInto, py::arg("data"), R"doc(
Files what the last run found into 'data'

Parameters
----------
data: :class:`IniParseData`
    The parser's data
        )doc");
}


void initCppGraphCollect(pybind11::module_ &m) {
    using Row = PyGraphCollect::Core::Row;

    auto cls = py::class_<PyGraphCollect, PyBaseIniGraphGroupEdit, AGRC::IParseResult, py::smart_holder>(m, "GraphCollect", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit` and :class:`IParseResult`

Collects the `KVPs`_ of some keys from :class:`IniSectionGraph`\ s, editing nothing

For each graph named in ``keys``, every :class:`IfContentPart` the graph reaches (through ``run =``, depth first)
is searched for each key, and every occurrence is recorded as a :class:`GraphCollect.Row`: where it is, its
value, and optionally

* ``contextKeys``: what other keys hold at that point, as the part's :class:`IfContentPartColouring` tracks them
  along the walk -- a key's occurrences in the part with their lines, or else the value it carries in from
  earlier on the same path (a caller before its ``run =``, or an earlier part). A key the part sets replaces
  the value carried in, and a value set on a sibling ``if`` branch is never carried
* ``withConditions``: the condition of the ``if`` blocks the part sits in

The rows stay until the next :meth:`edit` or :meth:`clear`. Run as one of :attr:`GIMIParser.parseSteps` (or
:attr:`GIMIParser.deferredParseSteps`), a collect with a :attr:`name` files its rows in the parser's
:attr:`BaseIniParser.parseData` (:meth:`IniParseData.collected`)

.. warning::
    The rows are a snapshot of the graphs as they were when the collect ran, and nothing keeps them up to date.
    An edit run afterwards that renames, copies, moves or removes a section, or changes a value, leaves every
    row it touched describing what is no longer there -- and such a row still reads like a good one. Read the
    rows before any edit that changes the graphs they came from, or run the collect again

Parameters
----------
keys: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Union[:class:`str`, List[:class:`str`], Callable[[:class:`str`], :class:`bool`]]]
    The keys to collect, keyed by which graph to search (``(.ini index, component, mod object)``) -- one key, a
    list searched in order, or a test of a key's NAME: every key of the graph's `KVPs`_ the test accepts is
    collected, in the order the keys first appear in the graph's `sections`_. The graphs given keys are searched
    before the graphs given a test

contextKeys: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], List[:class:`str`]]]
    The keys whose state is recorded with each row, keyed by graph -- see :attr:`GraphCollect.Row.context`
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

name: :class:`str`
    The name the rows are filed under in :class:`IniParseData` when this runs as one of
    :attr:`GIMIParser.parseSteps` -- empty files nothing :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`Ranges`]]]
    Which order indices of each part to collect from, keyed by graph :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

valPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]]
    Which occurrences to collect, keyed by graph -- each takes the key, the value and the part's data
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

trackKeys: Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]
    Whether to track `KVPs`_ while searching -- one flag for every graph, or one per graph. A graph with
    ``contextKeys`` is always tracked :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]
    Which keys to track while searching each graph; ``None`` tracks every key. A graph's ``contextKeys`` are
    always tracked :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

withConditions: :class:`bool`
    Whether each row records the condition its part sits in -- the walk then solves a predicate per part
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

everyGroup: :class:`bool`
    Whether every setting keyed by a graph applies in every group that holds that graph, rather than only in the
    group its index names -- as :attr:`ResRegCollect.everyGroup` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc");

    py::class_<Row>(cls, "Row", R"doc(
One collected occurrence of a key
    )doc")
        .def_property_readonly("graphId", [](const Row& row) { return graphIdToPy(row.graphId); },
                               py::doc(R"doc(Tuple[:class:`int`, :class:`str`, :class:`str`]: The graph it was found in)doc"))
        .def_readonly("section", &Row::section, py::doc(R"doc(:class:`str`: The `section`_ it is in)doc"))
        .def_readonly("rootSection", &Row::rootSection,
                      py::doc(R"doc(:class:`str`: The root `section`_ the walk reached it from -- only with ``withConditions``, else empty)doc"))
        .def_readonly("partId", &Row::partId, py::doc(R"doc(:class:`int`: The id of its part within the `section`_)doc"))
        .def_readonly("line", &Row::line, py::doc(R"doc(:class:`int`: Its order index within the part)doc"))
        .def_readonly("key", &Row::key, py::doc(R"doc(:class:`str`: Its key)doc"))
        .def_readonly("value", &Row::value, py::doc(R"doc(:class:`str`: Its value)doc"))
        .def_readonly("context", &Row::context, py::doc(R"doc(
List[Tuple[:class:`str`, List[Tuple[Optional[:class:`int`], :class:`str`]]]]: What each of the graph's context keys
holds at the part, in the order they were named -- the key's occurrences in the part with their lines, or the value
carried in from earlier on the path with no line
        )doc"))
        .def_readonly("condition", &Row::condition,
                      py::doc(R"doc(Optional[:class:`Z3Predicate`]: The condition of the ``if`` blocks the part sits in -- only with ``withConditions``)doc"))
        .def_readonly("guards", &Row::guards, py::doc(R"doc(
List[:class:`str`]: The test of each ``if`` / ``else if`` branch the part sits in within its own `section`_, outermost
first, as written without its surrounding whitespace (``$swapvar == 1``); an ``else`` branch is recorded as ``else``. Unlike :attr:`condition`, this is
the text of the section alone -- the conditions of a caller's ``run =`` are not in it -- and it is always filled
        )doc"))
        .def_readonly("blocks", &Row::blocks, py::doc(R"doc(
List[:class:`int`]: Which branch of each ``if`` block the part sits in within its own `section`_, outermost first: the
position, among the section's parts, of the ``if`` / ``else if`` / ``else`` line that opens the branch. It names the
same branches as :attr:`guards`, but tells apart two branches whose tests read the same -- two parts share a branch
exactly when one's list starts with the other's. Always filled
        )doc"));

    cls.def(py::init([](py::object keys, py::object contextKeys, std::string name, py::object partPredicates,
                        py::object valPredicates, py::object trackKeys, py::object keysToTrack, bool withConditions,
                        bool everyGroup) {
        auto result = std::make_unique<PyGraphCollect>(std::move(keys), std::move(contextKeys), std::move(partPredicates),
                                                       std::move(valPredicates), std::move(trackKeys), std::move(keysToTrack));
        result->name = std::move(name);
        result->withConditions = withConditions;
        result->everyGroup = everyGroup;
        return result;
    }), py::arg("keys"), py::arg("contextKeys") = py::none(), py::arg("name") = std::string(),
        py::arg("partPredicates") = py::none(), py::arg("valPredicates") = py::none(), py::arg("trackKeys") = false,
        py::arg("keysToTrack") = py::none(), py::arg("withConditions") = false, py::arg("everyGroup") = false);

    cls.def_readwrite("keys", &PyGraphCollect::keysObj,
                      py::doc(R"doc(Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Union[:class:`str`, List[:class:`str`], Callable[[:class:`str`], :class:`bool`]]]: The keys to collect, keyed by graph)doc"))
       .def_readwrite("contextKeys", &PyGraphCollect::contextKeysObj,
                      py::doc(R"doc(Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], List[:class:`str`]]: The keys whose state is recorded with each row, keyed by graph)doc"))
       .def_readwrite("partPredicates", &PyGraphCollect::partPredicatesObj,
                      py::doc(R"doc(Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterData`], :class:`Ranges`]]]: Which order indices to collect from, keyed by graph)doc"))
       .def_readwrite("valPredicates", &PyGraphCollect::valPredicatesObj,
                      py::doc(R"doc(Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterData`], :class:`bool`]]]: Which occurrences to collect, keyed by graph)doc"))
       .def_readwrite("trackKeys", &PyGraphCollect::trackKeysObj,
                      py::doc(R"doc(Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]: Whether to track `KVPs`_ while searching)doc"))
       .def_readwrite("keysToTrack", &PyGraphCollect::keysToTrackObj,
                      py::doc(R"doc(Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]: Which keys to track while searching each graph)doc"))
       .def_readwrite("name", &PyGraphCollect::name,
                      py::doc(R"doc(:class:`str`: The name the rows are filed under in :class:`IniParseData` -- empty files nothing)doc"))
       .def_readwrite("withConditions", &PyGraphCollect::withConditions,
                      py::doc(R"doc(:class:`bool`: Whether each row records the condition its part sits in)doc"))
       .def_readwrite("everyGroup", &PyGraphCollect::everyGroup,
                      py::doc(R"doc(:class:`bool`: Whether every setting keyed by a graph applies in every group that holds that graph)doc"));

    cls.def_property_readonly("rows", [](const PyGraphCollect& self) { return self.rows(); },
                              py::doc(R"doc(List[:class:`GraphCollect.Row`]: The rows the last :meth:`edit` collected, in the order they were found)doc"));

    cls.def("clear", [](PyGraphCollect& self) { self.Core::clear(); },
            py::doc(R"doc(Empties :attr:`rows`)doc"));

    const auto run = [](PyGraphCollect& self, py::list graphGroups, const std::string& modName) {
        self.refresh();
        PyIniGraphGroups groups(graphGroups);
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    };

    cls.def("edit", [run](PyGraphCollect& self, py::list graphGroups, const py::object& modType, const std::string& modName) {
        (void)modType;
        return run(self, std::move(graphGroups), modName);
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Collects the rows, replacing those of a previous run -- the groups are not changed

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to search for each .ini file

modType: Optional[:class:`ModType`]
    The type of mod. Unused

modName: :class:`str`
    The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in
    )doc"));

    cls.def("editFromIni", [run](PyGraphCollect& self, py::list graphGroups, const py::object& ini, const py::object& modType,
                                 const std::string& modName) {
        (void)ini;
        (void)modType;
        return run(self, std::move(graphGroups), modName);
    }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
:meth:`edit`, for a .ini file -- collecting reads nothing more from it

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to search for each .ini file

ini: :class:`IniFile`
    The associated .ini file. Unused

modType: Optional[:class:`ModType`]
    The type of mod. Unused

modName: :class:`str`
    The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in
    )doc"));
}
