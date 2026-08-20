#ifndef AGRemapPyBind_PyDFA_H
#define AGRemapPyBind_PyDFA_H

#include <tuple>
#include <vector>

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

        virtual void pyAddKeywordTransition(const py::object& srcId, const py::object& keyword, const py::object& destId);
        virtual void pyAddFuncTransition(const py::object& srcId, const py::function& func, const py::object& destId);
        virtual void addTransition(const py::object& srcId, const py::object& keyword, const py::object& destId);
        virtual void addTransitions(const py::object& srcId, const py::object& keywords, const py::object& destId);

        virtual std::tuple<py::object, bool, bool> pytransition(const py::object& keyword);

        virtual std::tuple<std::vector<py::object>, bool> pyGetKeywordTransitions(const py::object& id);
};


class PyBindDFA: public PyDFA {
    public:
        void clear() override;
        void reset() override;
        bool isAccept(const py::object& id) override;
        bool stateExists(const py::object& id) override;
        size_t stateSize() const override;
        size_t acceptSize() const override;
        bool isStart(const py::object& id) override;
        bool addState(const py::object& id, std::optional<bool> isAccept = std::nullopt, bool isStart = false) override;
        bool hasKeywordTransition(const py::object& srcId, const py::object& keyword) override;
        std::optional<py::object> getKeywordToState(const py::object& srcId, const py::object& keyword) override;

        void pyAddKeywordTransition(const py::object& srcId, const py::object& keyword, const py::object& destId) override;
        void pyAddFuncTransition(const py::object& srcId, const py::function& func, const py::object& destId) override;
        void addTransition(const py::object& srcId, const py::object& keyword, const py::object& destId) override;
        void addTransitions(const py::object& srcId, const py::object& keywords, const py::object& destId) override;
        std::tuple<py::object, bool, bool> pytransition(const py::object& keyword) override;

        std::tuple<std::vector<py::object>, bool> pyGetKeywordTransitions(const py::object& id) override;
};


void initCppDFA(pybind11::module_ &m);

#endif