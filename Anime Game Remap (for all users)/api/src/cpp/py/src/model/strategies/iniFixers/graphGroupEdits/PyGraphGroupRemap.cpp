#include "PyGraphGroupRemap.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>


PyGraphGroupRemap::PyGraphGroupRemap(py::object remapObj): Core({}), remapObj(std::move(remapObj)) {}


void PyGraphGroupRemap::refresh() {
    RemapList parsed;
    renameFuncObjs_.clear();

    // Parsed in two passes: the RenameFunc -> Python-object map has to be keyed by the address of
    // the *stored* std::function, so it can only be filled once the vector has stopped growing
    // (every reallocation would otherwise invalidate the keys).
    std::vector<std::vector<py::object>> renameFuncObjs;

    if (!remapObj.is_none()) {
        py::dict remapDict = remapObj.cast<py::dict>();

        for (auto item : remapDict) {
            GraphId srcId = parseGraphId(py::reinterpret_borrow<py::object>(item.first));
            std::vector<RemapTarget> targets;
            std::vector<py::object> targetRenameFuncs;

            for (auto targetItem : py::reinterpret_borrow<py::object>(item.second)) {
                py::object targetObj = py::reinterpret_borrow<py::object>(targetItem);
                py::sequence targetSeq = targetObj.cast<py::sequence>();

                RemapTarget target(parseGraphId(targetObj));
                py::object renameFuncObj = py::none();

                // A 4th element is an optional rename callable -- matching the pure-Python
                // original's own "if (len(remappedObjData) == 3)" branch.
                if (targetSeq.size() > 3) {
                    renameFuncObj = py::reinterpret_borrow<py::object>(targetSeq[3]);
                }

                if (!renameFuncObj.is_none()) {
                    py::object heldRenameFunc = renameFuncObj;
                    target.renameFunc = [heldRenameFunc](const std::string &oldSectionName) {
                        return heldRenameFunc(py::str(oldSectionName)).cast<std::string>();
                    };
                }

                targets.push_back(std::move(target));
                targetRenameFuncs.push_back(std::move(renameFuncObj));
            }

            parsed.emplace_back(std::move(srcId), std::move(targets));
            renameFuncObjs.push_back(std::move(targetRenameFuncs));
        }
    }

    remap = std::move(parsed);

    for (std::size_t i = 0; i < remap.size(); ++i) {
        for (std::size_t j = 0; j < remap[i].second.size(); ++j) {
            if (renameFuncObjs[i][j].is_none()) {
                continue;
            }

            renameFuncObjs_[static_cast<const void*>(&remap[i].second[j].renameFunc)] = renameFuncObjs[i][j];
        }
    }
}


py::object PyGraphGroupRemap::renameFuncToPy(const RenameFunc &renameFunc) const {
    auto it = renameFuncObjs_.find(static_cast<const void*>(&renameFunc));
    if (it == renameFuncObjs_.end()) {
        return py::none();
    }

    return it->second;
}


void initCppGraphGroupRemap(pybind11::module_ &m) {
    py::class_<PyGraphGroupRemap, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphGroupRemap", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Remaps the graphs from a group of graphs

Parameters
----------
remap: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], List[Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]
    The remap for the graphs :raw-html:`<br />` :raw-html:`<br />`

    * The keys of the dictionary are the mod objects to remap from.
    * The values of the dictionary are the mod objects to remap to.
    * The tuples include:

        #. The index of the .ini file for the graph
        #. The name of the component for the graph
        #. The name of the mod object for the graph
        #. An optional rename function if the tuple has 4 values. The rename function takes in the old name of the `section`_
    )doc");

    // py::init(factory) rather than py::init<py::object>(): the core class owns std::function
    // members, and a factory returning a unique_ptr avoids ever needing to move-construct the
    // class itself -- see PyRegAdd.cpp's identical note.
    cls.def(py::init([](py::object remap) {
        return std::make_unique<PyGraphGroupRemap>(std::move(remap));
    }), py::arg("remap"));

    cls.def_property("remap", [](const PyGraphGroupRemap &self) {
        return self.remapObj;
    }, [](PyGraphGroupRemap &self, py::object remap) {
        self.remapObj = std::move(remap);
    }, py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], List[Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]:
The remap for the graphs
    )doc"));

    cls.def_static("copyGraph", [](py::object fromGraph, const py::object &modObj, const py::object &newModObj,
                                   const py::object &renameFunc, const std::string &modName) -> py::object {
        // A standalone copy, unattached to any group list -- so this needs a graph-groups view of
        // its own purely to satisfy IIniGraphGroups's "the container owns every graph it produces"
        // contract. An empty list is enough: nothing here reads or writes a group.
        PyIniGraphGroups groups{py::list()};
        PyGraphGroupRemap::Graph *parsedFromGraph = groups.adopt(std::move(fromGraph));
        if (parsedFromGraph == nullptr) {
            return py::none();
        }

        PyGraphGroupRemap::RenameFunc parsedRenameFunc;
        if (!renameFunc.is_none()) {
            py::object heldRenameFunc = renameFunc;
            parsedRenameFunc = [heldRenameFunc](const std::string &oldSectionName) {
                return heldRenameFunc(py::str(oldSectionName)).cast<std::string>();
            };
        }

        PyGraphGroupRemap::Graph *result = PyGraphGroupRemap::copyGraph(groups, *parsedFromGraph, parseGraphId(modObj),
                                                                        parseGraphId(newModObj), parsedRenameFunc, modName);
        return groups.graphToPy(result);
    }, py::arg("fromGraph"), py::arg("modObj"), py::arg("newModObj"), py::arg("renameFunc") = py::none(),
       py::arg("modName") = "", py::doc(R"doc(
Deep-copies 'fromGraph' and renames every `section`_ in the copy

Parameters
----------
fromGraph: :class:`IniSectionGraph`
    The graph to copy

modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id of the graph being copied from

newModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id of the graph being copied to

renameFunc: Optional[Callable[[:class:`str`], :class:`str`]]
    The rename function to use. When ``None``, falls back to
    :meth:`IniNamingTools.getObjRemapFixName` against 'modObj'/'newModObj' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

modName: :class:`str`
    The name of the mod to fix to, used by that fallback :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`IniSectionGraph`
    The new, renamed copy
    )doc"));

    cls.def("remapGraphs", [](PyGraphGroupRemap &self, py::list graphGroups, const py::object &createToGraph) {
        self.refresh();

        PyIniGraphGroups groups(graphGroups);
        self.Core::remapGraphs(groups, [&self, &groups, &createToGraph](PyGraphGroupRemap::GraphGroups &,
                                                                        PyGraphGroupRemap::Graph &fromGraph,
                                                                        const PyGraphGroupRemap::GraphId &fromId,
                                                                        const PyGraphGroupRemap::GraphId &toId,
                                                                        const PyGraphGroupRemap::RenameFunc &renameFunc) {
            py::object result = createToGraph(groups.graphToPy(&fromGraph), graphIdToPy(fromId), graphIdToPy(toId),
                                              self.renameFuncToPy(renameFunc));
            return groups.adopt(std::move(result));
        });

        return graphGroups;
    }, py::arg("graphGroups"), py::arg("createToGraph"), py::doc(R"doc(
Remaps the graphs from a group of graphs

.. note::
    A target whose ``(component, object)`` key is already taken in the destination .ini file's group
    goes into an **additional** group for that same .ini file (created on demand), rather than
    overwriting the existing graph

.. note::
    A source .ini file whose original group is left with no graphs at all is dropped entirely

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to remap

createToGraph: Callable[[:class:`IniSectionGraph`, Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Callable[[:class:`str`], :class:`str`]]], :class:`IniSectionGraph`]
    The function used to create the new remapped graph :raw-html:`<br />` :raw-html:`<br />`

    The function takes in the following parameters:

    #. The graph to map from
    #. The id of the graph to map from. The tuple contains the index of the .ini file for the graph, the name of the component and the name of the mod object
    #. The id of the graph to map to. Note that the index of the .ini file may not correspond to the actual index of which .ini file holds the graph
    #. An optional rename function

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after the graphs were remapped
    )doc"));

    cls.def("edit", [](PyGraphGroupRemap &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        self.refresh();

        PyIniGraphGroups groups(graphGroups);
        // nullptr for modType: this edit never reads it (only 'modName' reaches copyGraph), and the
        // Python-side ModType is a pure-Python class with no C++ counterpart to cast to.
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Remaps the graphs, building each new graph with :meth:`copyGraph`

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix. Unused by this edit

modName: :class:`str`
    The name of the mod to fix to, handed to :meth:`copyGraph` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after the graphs were remapped
    )doc"));
}
