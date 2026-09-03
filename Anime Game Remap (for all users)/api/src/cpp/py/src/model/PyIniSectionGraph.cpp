#include "PyIniSectionGraph.h"

#include <string>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyCallGraph.h"
#include "PyNodeIdentity.h"
#include "PySectionIterData.h"
#include "iftemplate/PyIfTemplate.h"
#include "../tools/PyGeneratorIterator.h"
#include "../tools/z3/PyZ3Context.h"
#include "../tools/z3/PyZ3Predicate.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

AGRC::IfTemplateRunConfig<std::string, std::string> makeRunConfig() {
    return AGRC::IfTemplateRunConfig<std::string, std::string>{
        "run",
        // Both conversions are the identity now that a .ini KVP value *is* a std::string.
        [](const std::string &v) { return v; },
        [](const std::string &name) { return name; }
    };
}

std::unordered_map<std::string, PyIfTemplate*> parseSections(const py::dict &sections) {
    std::unordered_map<std::string, PyIfTemplate*> result;
    for (auto item : sections) {
        std::string key = py::reinterpret_borrow<py::object>(item.first).cast<std::string>();
        PyIfTemplate *val = py::reinterpret_borrow<py::object>(item.second).cast<PyIfTemplate*>();
        result[key] = val;
    }
    return result;
}

std::vector<std::string> parseNames(const py::object &names) {
    std::vector<std::string> result;
    for (auto item : names) {
        result.push_back(py::reinterpret_borrow<py::object>(item).cast<std::string>());
    }
    return result;
}

py::dict sectionsToPy(const std::unordered_map<std::string, PyIfTemplate*> &sections) {
    py::dict result;
    for (const auto &entry : sections) {
        result[py::cast(entry.first)] = py::cast(entry.second, py::return_value_policy::reference);
    }
    return result;
}

// Same dispatch as PyIfTemplate.cpp's own (file-local) 'partToPy' -- duplicated rather than shared
// across translation units, since this one is only ever used to pre-warm a wrapper's registration
// (the return value itself is discarded by every caller here), not returned to Python.
py::object partWrapperToPy(AGRC::IfTemplatePart *part) {
    if (part == nullptr) {
        return py::none();
    }
    if (auto *predPart = dynamic_cast<AGRC::IfPredPart*>(part)) {
        return py::cast(predPart, py::return_value_policy::reference);
    }
    return py::cast(static_cast<PyIfContentPart*>(part), py::return_value_policy::reference);
}

py::set contentPartSetToPy(const std::set<PyIfContentPart*> &parts) {
    py::set result;
    for (auto *part : parts) {
        result.add(py::cast(part, py::return_value_policy::reference));
    }
    return result;
}

py::object iterDataToPy(PySectionIterData &data) {
    return py::cast(data);
}

py::object iterQueryDataToPy(PySectionIterQueryData &data) {
    return py::cast(data);
}

py::object sectionPairToPy(std::pair<std::string, PyIfTemplate*> &data) {
    return py::make_tuple(py::cast(data.first), py::cast(data.second, py::return_value_policy::reference));
}

}


void PyIniSectionGraph::refreshKeepAlive() {
    keepAlive_ = sectionsToPy(sections());

    // See this class's own top-level note on #partsKeepAlive_ -- every part reachable from every
    // section needs its own stable, already-registered wrapper for the id(part) correlation
    // buildCallGraph()/buildPartPredecessorGraph()/iterByContentPart() (and callers like
    // RegSurroundedAdd.py that key off id(part) themselves) all depend on.
    py::list parts;
    for (const auto &entry : sections()) {
        for (const auto &part : entry.second->parts()) {
            if (part) {
                parts.append(partWrapperToPy(part.get()));
            }
        }
    }
    partsKeepAlive_ = std::move(parts);
}


void PyIniSectionGraph::setZ3CtxKeepAlive(py::object z3Ctx) {
    z3CtxKeepAlive_ = std::move(z3Ctx);
}


py::object PyIniSectionGraph::z3CtxKeepAlive() const {
    return z3CtxKeepAlive_;
}


void initCppIniSectionGraph(pybind11::module_ &m) {
    bindGeneratorIterator<PySectionIterData>(m, "SectionIterDataIterator");
    bindGeneratorIterator<PySectionIterQueryData>(m, "SectionIterQueryDataIterator");
    bindGeneratorIterator<std::pair<std::string, PyIfTemplate*>>(m, "IniSectionGraphSectionIterator");

    py::class_<PyIniSectionGraph>(m, "IniSectionGraph", R"doc(
Class for constructing a directed subgraph for how the `sections`_ in the .ini file are ran

.. container:: operations

    **Supported Operations:**

    .. describe:: for sectionName, section in x

        Iterates through all the `sections`_ of the graph using `DFS`_

Parameters
----------
sections: Dict[:class:`str`, :class:`IfTemplate`]
    All the `sections`_ of the constructed subgraph

targetSectionNames: Union[Set[:class:`str`], List[:class:`str`]]
    Names of the desired `sections`_ we want our subgraph to have

build: :class:`bool`
    Whether to build the graph. **Default**: ``True``

copySections: :class:`bool`
    Whether to make a deep copy of the referenced `sections`_. **Default**: ``False``

z3Ctx: Optional[:class:`Z3Context`]
    The `Z3`_ context every :class:`Z3Predicate` produced by this graph belongs to. **Default**: ``None``
    )doc")

        .def(py::init([](const py::dict &sections, const py::object &targetSectionNames, bool build, bool copySections, py::object z3Ctx) {
            AGRC::Z3Context *z3CtxPtr = z3Ctx.is_none() ? nullptr : z3Ctx.cast<AGRC::Z3Context*>();
            std::unordered_map<std::string, PyIfTemplate*> parsedSections = parseSections(sections);
            std::vector<std::string> parsedTargets = parseNames(targetSectionNames);
            auto result = std::make_unique<PyIniSectionGraph>(std::move(parsedSections), std::move(parsedTargets), makeRunConfig(), build, copySections, z3CtxPtr);
            // 'sections' (and every value inside it) is still guaranteed alive here (a live
            // Python argument to this very call) -- see PyIniSectionGraph.h's own top-level note
            // on why this is required, not optional, for the common
            // 'IniSectionGraph({"a": IfTemplate(...)}, [...])' inline-construction pattern.
            result->refreshKeepAlive();
            // 'z3Ctx' (if given) is still guaranteed alive here -- same reasoning, one level down,
            // for the equally common 'IniSectionGraph(..., z3Ctx = Z3Context())' pattern; see
            // PyIniSectionGraph.h's own top-level note.
            result->setZ3CtxKeepAlive(std::move(z3Ctx));
            return result;
        }), py::arg("sections"), py::arg("targetSectionNames"), py::arg("build") = true, py::arg("copySections") = false, py::arg("z3Ctx") = py::none())

        .def_property_readonly("sections", [](PyIniSectionGraph &self) {
            return sectionsToPy(self.sections());
        }, py::doc(R"doc(Dict[:class:`str`, :class:`IfTemplate`]: All the `sections`_ of the constructed subgraph)doc"))

        .def_property_readonly("neighbours", [](PyIniSectionGraph &self) {
            return py::cast(self.neighbours());
        }, py::doc(R"doc(Dict[:class:`str`, List[:class:`str`]]: The out-neighbours of the subgraph)doc"))

        .def_property_readonly("roots", [](PyIniSectionGraph &self) {
            return py::cast(self.roots());
        }, py::doc(R"doc(List[:class:`str`]: The root nodes of the subgraph)doc"))

        .def_property("targetSectionNames", [](PyIniSectionGraph &self) {
            return py::cast(self.targetSectionNames());
        }, [](PyIniSectionGraph &self, const py::object &newTargets) {
            self.setTargetSectionNames(parseNames(newTargets));
        }, py::doc(R"doc(List[:class:`str`]: Names of the desired `sections`_ we want our subgraph to have)doc"))

        .def("combine", [](PyIniSectionGraph &self, const py::list &newGraphs) {
            std::vector<PyIniSectionGraphCore*> graphs;
            for (auto item : newGraphs) {
                graphs.push_back(py::reinterpret_borrow<py::object>(item).cast<PyIniSectionGraph*>());
            }
            self.combine(graphs);
            // 'newGraphs' (and every graph inside it, and transitively every section reachable
            // from them) is still guaranteed alive here -- see the constructor binding's own
            // comment on why this synchronous refresh is required.
            self.refreshKeepAlive();
        }, py::arg("newGraphs"),
    py::doc(R"doc(
Combines this graph with other graphs

Parameters
----------
newGraphs: List[:class:`IniSectionGraph`]
    The new graphs to combine with
        )doc"))

        .def("build", [](PyIniSectionGraph &self, py::object sections, py::object targetSectionNames, bool copySections) {
            std::optional<std::unordered_map<std::string, PyIfTemplate*>> parsedSections;
            if (!sections.is_none()) {
                parsedSections = parseSections(sections.cast<py::dict>());
            }
            std::optional<std::vector<std::string>> parsedTargets;
            if (!targetSectionNames.is_none()) {
                parsedTargets = parseNames(targetSectionNames);
            }
            self.build(parsedSections, parsedTargets, copySections);
            // 'sections' (if provided) is still guaranteed alive here -- see the constructor
            // binding's own comment on why this synchronous refresh is required.
            self.refreshKeepAlive();
        }, py::arg("sections") = py::none(), py::arg("targetSectionNames") = py::none(), py::arg("copySections") = false,
    py::doc(R"doc(Constructs the subgraph for the `sections`_ using `DFS`_)doc"))

        .def("getSection", [](PyIniSectionGraph &self, const std::string &sectionName, bool raiseException) -> py::object {
            PyIfTemplate *result = self.getSection(sectionName, raiseException);
            if (result == nullptr) return py::none();
            return py::cast(result, py::return_value_policy::reference);
        }, py::arg("sectionName"), py::arg("raiseException") = true,
    py::doc(R"doc(Retrieves the :class:`IfTemplate` for a certain `section`_)doc"))

        .def("getRootSections", [](PyIniSectionGraph &self) {
            py::list result;
            for (auto *section : self.getRootSections()) {
                result.append(py::cast(section, py::return_value_policy::reference));
            }
            return result;
        }, py::doc(R"doc(Retrieves the `sections`_ corresponding to the roots of the graph)doc"))

        .def("isEmpty", &PyIniSectionGraph::isEmpty, py::doc(R"doc(Determines whether the graph is empty)doc"))

        .def("getNeighbourNames", [](PyIniSectionGraph &self, const std::string &sectionName) {
            return py::cast(self.getNeighbourNames(sectionName));
        }, py::arg("sectionName"), py::doc(R"doc(Retrieves the names of the out-neighbour `sections`_)doc"))

        .def("getNeighbours", [](PyIniSectionGraph &self, const std::string &sectionName) {
            return sectionsToPy(self.getNeighbours(sectionName));
        }, py::arg("sectionName"), py::doc(R"doc(Retrieves the out-neighbours of some `section`_)doc"))

        .def("getChildren", [](PyIniSectionGraph &self, const py::object &targetSections, bool getNeighbourChildren) {
            auto result = self.getChildren(parseNames(targetSections), getNeighbourChildren);
            py::dict pyResult;
            for (auto &entry : result) {
                pyResult[py::cast(entry.first)] = py::cast(entry.second);
            }
            return pyResult;
        }, py::arg("targetSections"), py::arg("getNeighbourChildren") = true,
    py::doc(R"doc(Retrieves the children `sections`_ of the `sections`_ specified at 'targetSections')doc"))

        .def("rename", [](PyIniSectionGraph &self, const py::function &renameFunc) {
            self.rename([&renameFunc](const std::string &name) {
                return renameFunc(name).cast<std::string>();
            });
            // Doesn't introduce any new external Python object references (only relabels
            // sections already tracked in keepAlive_), so this isn't strictly load-bearing --
            // done anyway to keep keepAlive_'s own keys in sync with sections()'s current names.
            self.refreshKeepAlive();
        }, py::arg("renameFunc"), py::doc(R"doc(Renames the `sections`_ and reconstructs the graph)doc"))

        .def("refreshPartIds", &PyIniSectionGraph::refreshPartIds, py::arg("minimal") = true,
    py::doc(R"doc(Regenerates the ids of the parts)doc"))

        .def("deepcopy", [](PyIniSectionGraph &self, bool minimal, bool newPartIds) {
            // AGRC::IniSectionGraph::deepcopy() constructs a fresh *base*-typed object directly
            // (generic, Python-free code with no notion of this Python-only subclass) -- promote
            // it into a real PyIniSectionGraph and give it its own keep-alive set before handing
            // it back to Python. Every section this copy holds is genuinely owned by its own
            // ownedSections_ (a real deep copy), not borrowed, so there's nothing external this
            // particular refresh needs to protect against being collected -- but doing it anyway
            // keeps this class's own invariant ("keepAlive_ matches sections()") true unconditionally.
            auto baseResult = static_cast<PyIniSectionGraphCore&>(self).deepcopy(minimal, newPartIds);
            auto result = std::make_unique<PyIniSectionGraph>(std::move(*baseResult));
            result->refreshKeepAlive();
            // The copy shares 'self''s own z3Ctx_ pointer (AGRC::IniSectionGraph::deepcopy()
            // deliberately doesn't deep-copy it) -- propagate the same keep-alive reference so it
            // survives independently of whether 'self' itself is later collected. See
            // PyIniSectionGraph.h's own top-level note.
            result->setZ3CtxKeepAlive(self.z3CtxKeepAlive());
            return result;
        }, py::arg("minimal") = true, py::arg("newPartIds") = true,
    py::doc(R"doc(Performs a deep copy on the object)doc"))
        .def("__copy__", [](PyIniSectionGraph &self) {
            auto baseResult = static_cast<PyIniSectionGraphCore&>(self).deepcopy();
            auto result = std::make_unique<PyIniSectionGraph>(std::move(*baseResult));
            result->refreshKeepAlive();
            result->setZ3CtxKeepAlive(self.z3CtxKeepAlive());
            return result;
        })
        .def("__deepcopy__", [](PyIniSectionGraph &self, py::dict) {
            auto baseResult = static_cast<PyIniSectionGraphCore&>(self).deepcopy();
            auto result = std::make_unique<PyIniSectionGraph>(std::move(*baseResult));
            result->refreshKeepAlive();
            result->setZ3CtxKeepAlive(self.z3CtxKeepAlive());
            return result;
        }, py::arg("memo"))

        .def("isKeyFullyCover", [](PyIniSectionGraph &self, const std::string &key) {
            return py::cast(self.isKeyFullyCover(key));
        }, py::arg("key"), py::doc(R"doc(Determines whether a key fully covers all the conditional branches of a `section`_)doc"))

        .def("rootsAreFullyCovered", [](PyIniSectionGraph &self, const std::string &key) {
            return py::cast(self.rootsAreFullyCovered(key));
        }, py::arg("key"), py::doc(R"doc(Convenience over :meth:`isKeyFullyCover`, filtered to :attr:`roots`)doc"))

        .def("getKeyMissingParts", [](PyIniSectionGraph &self, const std::string &key) {
            auto result = self.getKeyMissingParts(key);
            py::dict pyResult;
            for (auto &entry : result) {
                pyResult[py::cast(entry.first)] = contentPartSetToPy(entry.second);
            }
            return pyResult;
        }, py::arg("key"), py::doc(R"doc(Retrieves the parts in the `sections`_ that are not covered by 'key')doc"))

        .def_static("computeSectionPredecessors", [](PyIfTemplate &section) {
            // Pre-warm a stable wrapper for every part of 'section' before computing anything --
            // see PyIniSectionGraph.h's own top-level note on #partsKeepAlive_ for why a bare
            // 'py::cast(ptr, reference)' (what pyIdOfPart does) isn't safe on its own here: unlike
            // every other caller of pyIdOfPart in this file, this static method has no graph
            // instance of its own to carry a persistent keep-alive, so this local list has to do
            // that job just for the duration of this one call -- long enough, since only the
            // returned integer ids (not the wrappers themselves) need to outlive it.
            py::list partsKeepAlive;
            for (const auto &part : section.parts()) {
                if (part) {
                    partsKeepAlive.append(partWrapperToPy(part.get()));
                }
            }

            auto result = PyIniSectionGraph::computeSectionPredecessors(section.parts());
            py::dict pyResult;
            for (auto &entry : result) {
                py::object keyId = py::cast(pyIdOfPart(entry.first));
                py::list preds;
                for (auto *p : entry.second) {
                    preds.append(py::cast(pyIdOfPart(p)));
                }
                pyResult[keyId] = preds;
            }
            return pyResult;
        }, py::arg("section"),
    py::doc(R"doc(
Computes, for every :class:`IfContentPart` in a `section`_'s flat, textually-ordered parts list, the
``id()`` of every :class:`IfContentPart` that must run immediately before it on some path through
this `section`_ alone

Parameters
----------
section: :class:`IfTemplate`
    The `section`_ to compute predecessors for

Returns
-------
Dict[:class:`int`, List[:class:`int`]]
    The ``id()`` of every :class:`IfContentPart`, mapped to the ``id()`` of its predecessors
        )doc"))

        .def("buildPartPredecessorGraph", [](PyIniSectionGraph &self) {
            auto result = self.buildPartPredecessorGraph();
            py::dict pyResult;
            for (auto &entry : result) {
                py::object keyId = py::cast(pyIdOfPart(entry.first));
                py::list preds;
                for (auto *p : entry.second) {
                    preds.append(py::cast(pyIdOfPart(p)));
                }
                pyResult[keyId] = preds;
            }
            return pyResult;
        }, py::doc(R"doc(
Builds a graph-wide version of :meth:`computeSectionPredecessors`, additionally linking a
``run =`` call's own part as a predecessor of whatever `section`_ it calls into

Returns
-------
Dict[:class:`int`, List[:class:`int`]]
    The ``id()`` of every reachable :class:`IfContentPart`, mapped to the ``id()`` of its predecessors
        )doc"))

        .def("buildCallGraph", &PyIniSectionGraph::buildCallGraph,
    py::doc(R"doc(Builds a `call graph`_ over the :class:`IfContentPart`\s of this graph)doc"))

        .def("normalize", &PyIniSectionGraph::normalize,
    py::doc(R"doc(Normalizes the branching structure of all `sections`_ in :attr:`sections`)doc"))

        // AGRC::IniSectionGraph::toStr needs a per-section renderer (the core has no notion of what
        // a section's text looks like -- see IfTemplate's own note on why it has no core toStr
        // either), so this supplies one that goes back through genuine Python attribute lookup on
        // IfTemplate.toStr. Bound only now: IniGraphGroup.toStr has always called
        // `graph.toStr(autoindent = ...)` on each of its graphs, which raised AttributeError
        // against a C++-backed graph -- a gap PyIniGraphGroup.h itself flagged when it was written.
        .def("toStr", [](PyIniSectionGraph &self, bool autoindent) {
            return self.toStr([](PyIfTemplate &section, const std::string &linePrefix, bool sectionAutoindent) {
                py::object sectionObj = py::cast(&section, py::return_value_policy::reference);
                return sectionObj.attr("toStr")(py::arg("linePrefix") = linePrefix,
                                                 py::arg("autoindent") = sectionAutoindent).cast<std::string>();
            }, autoindent);
        }, py::arg("autoindent") = true,
    py::doc(R"doc(
Converts all the `sections`_ of this graph to a string, walked outwards from :attr:`roots` using
`DFS`_ and joined with blank lines

Parameters
-----------
autoindent: :class:`bool`
    Whether to compute the proper tab indent for each `section`_

    **Default**: ``True``

Returns
--------
:class:`str`
    The string representation
        )doc"))

        .def_static("iterSectsByContentPart", [](const py::dict &sections, const py::list &roots, int states, bool colour, py::object colourKeys) {
            std::optional<std::unordered_set<std::string>> keys;
            if (!colourKeys.is_none()) {
                keys = colourKeys.cast<std::unordered_set<std::string>>();
            }
            auto gen = PyIniSectionGraph::iterSectsByContentPart(parseSections(sections), parseNames(roots), makeRunConfig(), states, colour, keys);
            return PyGeneratorIterator<PySectionIterData>(std::move(gen), iterDataToPy);
        }, py::arg("sections"), py::arg("roots"), py::arg("states") = 1, py::arg("colour") = false, py::arg("colourKeys") = py::none(),
    py::doc(R"doc(An iterator that iterates through all :class:`IfContentPart` of the `sections`_ using `DFS`_)doc"))

        .def("iterByContentPart", [](PyIniSectionGraph &self, int states, bool colour, py::object colourKeys) {
            std::optional<std::unordered_set<std::string>> keys;
            if (!colourKeys.is_none()) {
                keys = colourKeys.cast<std::unordered_set<std::string>>();
            }
            auto gen = self.iterByContentPart(states, colour, keys);
            return PyGeneratorIterator<PySectionIterData>(std::move(gen), iterDataToPy);
        }, py::arg("states") = 1, py::arg("colour") = false, py::arg("colourKeys") = py::none(),
    py::doc(R"doc(An iterator that iterates through all :class:`IfContentPart` of the `sections`_ of this graph using `DFS`_)doc"))

        .def("__iter__", [](PyIniSectionGraph &self) {
            auto gen = self.iterSections();
            return PyGeneratorIterator<std::pair<std::string, PyIfTemplate*>>(std::move(gen), sectionPairToPy);
        }, py::doc(R"doc(Iterates through all the `sections`_ of the graph using `DFS`_, yielding ``(sectionName, section)`` pairs)doc"))

        .def("iterByQuery", [](PyIniSectionGraph &self, py::object queryPath, bool simplify, int states, bool colour, py::object colourKeys) {
            std::vector<AGRC::Z3Predicate> parsedQueryPath;
            if (!queryPath.is_none()) {
                if (py::isinstance<py::list>(queryPath)) {
                    for (auto item : queryPath) {
                        parsedQueryPath.push_back(py::reinterpret_borrow<py::object>(item).cast<AGRC::Z3Predicate>());
                    }
                } else {
                    parsedQueryPath.push_back(queryPath.cast<AGRC::Z3Predicate>());
                }
            }

            std::optional<std::unordered_set<std::string>> keys;
            if (!colourKeys.is_none()) {
                keys = colourKeys.cast<std::unordered_set<std::string>>();
            }

            auto gen = self.iterByQuery(std::move(parsedQueryPath), simplify, states, colour, keys);
            return PyGeneratorIterator<PySectionIterQueryData>(std::move(gen), iterQueryDataToPy);
        }, py::arg("queryPath") = py::none(), py::arg("simplify") = false, py::arg("states") = 1,
           py::arg("colour") = false, py::arg("colourKeys") = py::none(),
    py::doc(R"doc(
An iterator that iterates through all the :class:`IfContentPart`\s of the graph and also retrieves
the conditional logical predicate that each :class:`IfContentPart` resides in
        )doc"))

        .def("processIfContentByQuery", [](PyIniSectionGraph &self, const py::function &processIfContent, py::object queryPath,
                                            bool simplify, int states, bool colour, py::object colourKeys) {
            std::vector<AGRC::Z3Predicate> parsedQueryPath;
            if (!queryPath.is_none()) {
                if (py::isinstance<py::list>(queryPath)) {
                    for (auto item : queryPath) {
                        parsedQueryPath.push_back(py::reinterpret_borrow<py::object>(item).cast<AGRC::Z3Predicate>());
                    }
                } else {
                    parsedQueryPath.push_back(queryPath.cast<AGRC::Z3Predicate>());
                }
            }

            std::optional<std::unordered_set<std::string>> keys;
            if (!colourKeys.is_none()) {
                keys = colourKeys.cast<std::unordered_set<std::string>>();
            }

            auto gen = self.iterByQuery(std::move(parsedQueryPath), simplify, states, colour, keys);
            while (gen.next()) {
                processIfContent(gen.value());
            }
        }, py::arg("processIfContent"), py::arg("queryPath") = py::none(), py::arg("simplify") = false, py::arg("states") = 1,
           py::arg("colour") = false, py::arg("colourKeys") = py::none(),
    py::doc(R"doc(Processes all :class:`IfContentPart`\s of the graph that require the conditional logic predicate they reside in)doc"));
}
