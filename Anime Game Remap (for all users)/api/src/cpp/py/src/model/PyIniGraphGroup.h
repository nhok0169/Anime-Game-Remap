#ifndef AGRemapPyBind_PyIniGraphGroup_H
#define AGRemapPyBind_PyIniGraphGroup_H

#include <string>

#include <pybind11/pybind11.h>

// The C++ port of IniGraphGroup.py -- a thin wrapper around a group of caller/callee graphs
// within a .ini file, keyed by (component name, mod object name) tuples.
//
// Deliberately a pybind-layer-only class, with no AGRemapCore core counterpart: every real value
// stored in #graphs is already a fully C++-backed CppIniSectionGraph Python object (IniSectionGraph
// itself was fully replaced in an earlier migration), and this class does no graph algorithms of
// its own -- it's just add/remove/toStr over a dict. #graphs is a genuine Python ``dict`` (not a
// converted std::unordered_map) specifically because real call sites depend on Python dict
// reference semantics: GIMIParser.py does `IniGraphGroup(graphs = self.commandGraphs)` then later
// reads `graphGroups[0].graphs` back out, expecting the *same* dict object identity throughout
// (this is exactly how the pure-Python original behaved too -- `self.graphs = graphs` is a bare
// reference assignment, not a copy). A py::dict handle naturally replicates that; a converted
// std::unordered_map member would silently break the aliasing (copy in, copy out).
class PyIniGraphGroup {
    public:
        pybind11::dict graphs;

        explicit PyIniGraphGroup(pybind11::dict graphs);

        void addGraph(pybind11::object modObj, pybind11::object graph);

        // dict.pop(modObj, None) equivalent -- returns the removed graph, or None if not found.
        pybind11::object removeGraph(pybind11::object modObj);

        // Calls '.toStr(autoindent=autoindent)' on each graph in #graphs and joins the non-empty
        // results with "\n\n" -- faithful port of the pure-Python original's own toStr(). This used
        // to raise AttributeError, because IniSectionGraph had no toStr() binding at all (a gap
        // left over from that class's own, earlier migration, flagged here at the time rather than
        // fixed); PyIniSectionGraph.cpp now binds one, so both this and a bare
        // IniSectionGraph.toStr() work.
        std::string toStr(bool autoindent = true) const;
};

void initCppIniGraphGroup(pybind11::module_ &m);

#endif
