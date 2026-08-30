#ifndef AGRemapPyBind_PyIniGraphGroups_H
#define AGRemapPyBind_PyIniGraphGroups_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>

#include "../../../PyIniSectionGraph.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IIniGraphGroups.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `Python`_-backed :cpp:class:`AGRemapCore::IIniGraphGroups` -- a live view over the actual
 ``List[IniGraphGroup]`` a caller passed in :raw-html:`<br />` :raw-html:`<br />`

 Every ``graphGroupEdits/`` algorithm lives in `AGRemapCore` and is written against
 :cpp:class:`AGRemapCore::IIniGraphGroups`; this is the implementation that lets those algorithms
 run directly against real `Python`_ objects, with **no conversion at the boundary**. That matters:
 ``IniGraphGroup.graphs`` is a genuine `Python`_ ``dict`` whose *reference* semantics real call
 sites depend on (``GIMIParser.py`` reads ``graphGroups[0].graphs`` back out expecting the same
 object), and the graphs inside it are real ``IniSectionGraph`` `Python`_ objects carrying their own
 keep-alive bookkeeping. Copying either into a C++ container and back would silently break both.
 @endrst
 */
class PyIniGraphGroups: public AGRC::IIniGraphGroups<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:
        using Base = AGRC::IIniGraphGroups<py::object, py::object, PyObjectHash, PyObjectEqual>;
        using Graph = Base::Graph;
        using Section = Base::Section;
        using ModObj = Base::ModObj;

        /**
         * @brief Constructs a view over a Python ``List[IniGraphGroup]``
         *
         * @param graphGroups The list to operate on -- **borrowed**, and mutated in place
         */
        explicit PyIniGraphGroups(py::list graphGroups);

        /**
         * @brief The list this view operates on -- the same object the caller passed in
         */
        py::list list() const;

        std::size_t size() const override;
        void insertGroup(std::size_t groupInd) override;
        void removeGroup(std::size_t groupInd) override;
        std::vector<ModObj> modObjs(std::size_t groupInd) const override;
        std::size_t graphCount(std::size_t groupInd) const override;
        Graph* getGraph(std::size_t groupInd, const ModObj& modObj) const override;
        void addGraph(std::size_t groupInd, const ModObj& modObj, Graph* graph) override;
        Graph* removeGraph(std::size_t groupInd, const ModObj& modObj) override;
        Graph* deepcopyGraph(const Graph& src, bool minimal = true, bool newPartIds = true) override;
        Graph* createGraph(std::unordered_map<std::string, Section*> sections, std::vector<std::string> targetSectionNames,
                            bool copySections = false, AGRC::Z3Context* z3Ctx = nullptr) override;

        /**
         * @brief
         @rst
         The `Python`_ object for a graph this view has handed out, or ``None`` if it has never
         seen that graph :raw-html:`<br />` :raw-html:`<br />`

         Needed by any binding that has to hand a core-produced graph back to `Python`_ code (eg.
         a user-supplied ``createToGraph`` callback)
         @endrst
         *
         * @param graph The graph to look up
         */
        py::object graphToPy(const Graph* graph) const;

        /**
         * @brief
         @rst
         Registers an already-constructed `Python`_ ``IniSectionGraph`` with this view, so it can
         later be passed to #addGraph like any graph this view produced itself
         @endrst
         *
         * @param graph The Python graph object to adopt
         *
         * @return The registered graph, or ``nullptr`` if 'graph' is ``None``
         */
        Graph* adopt(py::object graph);

        /**
         * @brief Converts a #ModObj into the ``(component, object)`` tuple Python keys the graph dicts use
         *
         * @param modObj The mod object to convert
         */
        static py::tuple modObjToPy(const ModObj& modObj);

        /**
         * @brief The inverse of #modObjToPy
         *
         * @param key The Python key to convert
         */
        static ModObj modObjFromPy(const py::handle& key);

    private:
        py::list graphGroups_;

        // Every graph this view has ever handed out, keyed by the raw pointer core algorithms
        // identify it by. This is what makes 'addGraph(..., Graph*)' expressible against Python
        // objects at all: pybind11 can't cast a bare AGRC::IniSectionGraph<py::object, ...>*
        // back to Python (only the derived PyIniSectionGraph is a registered type), so the
        // pointer -> object direction has to be remembered rather than recomputed.
        mutable std::unordered_map<const Graph*, py::object> handles_;

        // Strong references to graphs that are (currently) in no group at all -- a fresh
        // deepcopyGraph/createGraph result, or one taken back out by removeGraph. Keeps
        // IIniGraphGroups's "every pointer stays valid for this object's lifetime" promise.
        py::list ownedGraphs_;

        py::object group(std::size_t groupInd) const;
        py::dict groupGraphs(std::size_t groupInd) const;
        Graph* track(py::object graph) const;
};

#endif
