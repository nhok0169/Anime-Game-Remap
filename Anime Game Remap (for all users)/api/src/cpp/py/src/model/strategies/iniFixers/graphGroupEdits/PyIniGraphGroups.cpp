#include "PyIniGraphGroups.h"

#include <memory>
#include <utility>

#include "../../../PyIniGraphGroup.h"
#include "../../../iftemplate/PyIfTemplate.h"


namespace {

// The same run configuration PyIniSectionGraph's own constructor binding builds -- duplicated here
// rather than shared, matching how PyIniSectionGraph.cpp already keeps it file-local.
AGRC::IfTemplateRunConfig<py::object, py::object> makeRunConfig() {
    return AGRC::IfTemplateRunConfig<py::object, py::object>{
        py::cast(std::string("run")),
        [](const py::object &v) { return v.cast<std::string>(); },
        [](const std::string &name) { return py::cast(name); }
    };
}

}


PyIniGraphGroups::PyIniGraphGroups(py::list graphGroups): graphGroups_(std::move(graphGroups)) {
}


py::list PyIniGraphGroups::list() const {
    return graphGroups_;
}


py::tuple PyIniGraphGroups::modObjToPy(const ModObj &modObj) {
    return py::make_tuple(py::str(modObj.first), py::str(modObj.second));
}


PyIniGraphGroups::ModObj PyIniGraphGroups::modObjFromPy(const py::handle &key) {
    // Every real graph key in this codebase is a plain (componentName, objectName) tuple of str.
    // Anything else can't identify a graph for a core algorithm at all, so it maps to the empty
    // key rather than raising -- a graph under such a key is simply never matched.
    if (!py::isinstance<py::tuple>(key)) {
        return ModObj();
    }

    py::tuple keyTuple = py::reinterpret_borrow<py::tuple>(key);
    if (keyTuple.size() < 2) {
        return ModObj();
    }

    return ModObj(py::str(keyTuple[0]).cast<std::string>(), py::str(keyTuple[1]).cast<std::string>());
}


py::object PyIniGraphGroups::group(std::size_t groupInd) const {
    if (groupInd >= static_cast<std::size_t>(py::len(graphGroups_))) {
        return py::none();
    }

    return graphGroups_[groupInd];
}


py::dict PyIniGraphGroups::groupGraphs(std::size_t groupInd) const {
    py::object currentGroup = group(groupInd);
    if (currentGroup.is_none()) {
        return py::dict();
    }

    return currentGroup.attr("graphs").cast<py::dict>();
}


PyIniGraphGroups::Graph* PyIniGraphGroups::track(py::object graph) const {
    if (graph.is_none()) {
        return nullptr;
    }

    Graph *result = graph.cast<PyIniSectionGraph*>();
    handles_[result] = std::move(graph);
    return result;
}


py::object PyIniGraphGroups::graphToPy(const Graph *graph) const {
    auto it = handles_.find(graph);
    if (it == handles_.end()) {
        return py::none();
    }

    return it->second;
}


PyIniGraphGroups::Graph* PyIniGraphGroups::adopt(py::object graph) {
    if (graph.is_none()) {
        return nullptr;
    }

    Graph *result = track(graph);
    ownedGraphs_.append(std::move(graph));
    return result;
}


std::size_t PyIniGraphGroups::size() const {
    return static_cast<std::size_t>(py::len(graphGroups_));
}


void PyIniGraphGroups::insertGroup(std::size_t groupInd) {
    std::size_t at = groupInd;
    std::size_t currentSize = size();
    if (at > currentSize) {
        at = currentSize;
    }

    graphGroups_.attr("insert")(at, py::cast(std::make_unique<PyIniGraphGroup>(py::dict())));
}


void PyIniGraphGroups::removeGroup(std::size_t groupInd) {
    if (groupInd >= size()) {
        return;
    }

    // Anything still in the group is about to lose its last reference along with the group, so
    // move it into ownedGraphs_ first -- see IIniGraphGroups's ownership contract.
    py::dict graphs = groupGraphs(groupInd);
    for (auto item : graphs) {
        ownedGraphs_.append(py::reinterpret_borrow<py::object>(item.second));
    }

    graphGroups_.attr("pop")(groupInd);
}


std::vector<PyIniGraphGroups::ModObj> PyIniGraphGroups::modObjs(std::size_t groupInd) const {
    std::vector<ModObj> result;

    py::dict graphs = groupGraphs(groupInd);
    for (auto item : graphs) {
        result.push_back(modObjFromPy(item.first));
    }

    return result;
}


std::size_t PyIniGraphGroups::graphCount(std::size_t groupInd) const {
    return static_cast<std::size_t>(py::len(groupGraphs(groupInd)));
}


PyIniGraphGroups::Graph* PyIniGraphGroups::getGraph(std::size_t groupInd, const ModObj &modObj) const {
    py::dict graphs = groupGraphs(groupInd);
    py::tuple key = modObjToPy(modObj);

    if (!graphs.contains(key)) {
        return nullptr;
    }

    return track(graphs[key]);
}


void PyIniGraphGroups::addGraph(std::size_t groupInd, const ModObj &modObj, Graph *graph) {
    if (groupInd >= size() || graph == nullptr) {
        return;
    }

    py::object graphObj = graphToPy(graph);
    if (graphObj.is_none()) {
        return;
    }

    py::object currentGroup = group(groupInd);
    currentGroup.attr("addGraph")(modObjToPy(modObj), graphObj);
}


PyIniGraphGroups::Graph* PyIniGraphGroups::removeGraph(std::size_t groupInd, const ModObj &modObj) {
    py::object currentGroup = group(groupInd);
    if (currentGroup.is_none()) {
        return nullptr;
    }

    py::object removed = currentGroup.attr("removeGraph")(modObjToPy(modObj));
    if (removed.is_none()) {
        return nullptr;
    }

    return adopt(std::move(removed));
}


PyIniGraphGroups::Graph* PyIniGraphGroups::deepcopyGraph(const Graph &src, bool minimal, bool newPartIds) {
    // AGRC::IniSectionGraph::deepcopy() builds a fresh *base*-typed graph (Python-free code with
    // no notion of the PyIniSectionGraph subclass), so promote it and give it the same keep-alive
    // treatment PyIniSectionGraph's own 'deepcopy' binding does -- see PyIniSectionGraph.h's
    // top-level note for why skipping either step is a real crash, not a theoretical one.
    auto baseResult = src.deepcopy(minimal, newPartIds);
    auto result = std::make_unique<PyIniSectionGraph>(std::move(*baseResult));
    result->refreshKeepAlive();

    // The copy shares 'src''s own raw Z3Context* pointer, so it needs 'src''s own strong reference
    // to that context propagated across too.
    py::object srcObj = graphToPy(&src);
    if (!srcObj.is_none()) {
        result->setZ3CtxKeepAlive(srcObj.cast<PyIniSectionGraph*>()->z3CtxKeepAlive());
    }

    return adopt(py::cast(std::move(result)));
}


PyIniGraphGroups::Graph* PyIniGraphGroups::createGraph(std::unordered_map<std::string, Section*> sections, std::vector<std::string> targetSectionNames,
                                                        bool copySections, AGRC::Z3Context *z3Ctx) {
    auto result = std::make_unique<PyIniSectionGraph>(std::move(sections), std::move(targetSectionNames), makeRunConfig(), true, copySections, z3Ctx);
    result->refreshKeepAlive();

    // Unlike deepcopyGraph there is no source graph to inherit a Z3Context strong reference from.
    // The context here always belongs to the IniFile that owns it (one Z3Context per IniFile, see
    // this subsystem's own notes) and that IniFile outlives the edit, so the raw pointer stays
    // valid without this view holding its own reference.
    return adopt(py::cast(std::move(result)));
}
