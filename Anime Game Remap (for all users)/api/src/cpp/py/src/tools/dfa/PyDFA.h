#ifndef PyDFA_H
#define PyDFA_H

#include <tuple>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../PyTools.h"
#include "AGRemapCore/tools/dfa/BaseDFA.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


extern template class AGRC::BaseDFA<py::object, py::object, PyObjectEqual, PyObjectHash, PyObjectEqual, PyObjectHash>;
extern template class AGRC::BiMap<py::object, std::uint64_t, PyObjectHash, PyObjectEqual, std::hash<std::uint64_t>, std::equal_to<std::uint64_t>>;


class PyDFA: public AGRC::BaseDFA<py::object, py::object, PyObjectEqual, PyObjectHash, PyObjectEqual, PyObjectHash> {
    public:
        PyDFA();

        std::optional<py::object> getStartIdOpt();
        std::optional<py::object> getCurrentStateIdOpt();

        void setPyStartId(const py::object& newStartId);
        void setPyCurrentStateId(const py::object& newCurrentStateId);

        void pyAddKeywordTransition(const py::object& srcId, const py::object& keyword, const py::object& destId);
        void pyAddFuncTransition(const py::object& srcId, const py::function& func, const py::object& destId);
        void addTransition(const py::object& srcId, const py::object& keyword, const py::object& destId);
        void addTransitions(const py::object& srcId, const py::object& keywords, const py::object& destId);

        std::tuple<py::object, bool, bool> pytransition(const py::object& keyword);
};

void initCppDFA(pybind11::module_ &m);

#endif