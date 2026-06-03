#include "PyDFA.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;


template class AGRC::BaseDFA<py::object, py::object, PyObjectEqual, PyObjectHash, PyObjectEqual, PyObjectHash>;
template class AGRC::BiMap<py::object, std::uint64_t, PyObjectHash, PyObjectEqual, std::hash<std::uint64_t>, std::equal_to<std::uint64_t>>;


PyDFA::PyDFA(): BaseDFA() {

}

std::optional<py::object> PyDFA::getStartIdOpt() {
    const py::object *startId = getStartIdPtr();
    if (startId == nullptr) return std::nullopt;
    return *startId;
}

std::optional<py::object> PyDFA::getCurrentStateIdOpt() {
    const py::object *currentStateId = getCurrentStateIdPtr();
    if (currentStateId == nullptr) return std::nullopt;
    return *currentStateId;
}

void PyDFA::setPyStartId(const py::object& newStartId) {
    bool hasSet = setStartId(newStartId);
    if (!hasSet) {
        std::string newStartIdStr = py::str(newStartId).cast<std::string>();
        throw py::key_error("The id, '" + newStartIdStr + "s' cannot be set as the new start state since the id does not correspond to a valid state in the DFA");
    }
}

void PyDFA::setPyCurrentStateId(const py::object& newCurrentStateId) {
    bool hasSet = setCurrentStateId(newCurrentStateId);
    if (!hasSet) {
        std::string newCurrentStateIdStr = py::str(newCurrentStateId).cast<std::string>();
        throw py::key_error("The id, '" + newCurrentStateIdStr + "' cannot be set as the new current state since the id does not correspond to a valid state in the DFA");
    }
}

static void throwTransitionSrcNotExists(const py::object& srcId) {
    std::string srcIdStr = py::str(srcId).cast<std::string>();
    throw py::key_error("The id, '" + srcIdStr + "' cannot be set as the source state of a new transition since the id does not correspond to a valid state in the DFA");
}

void PyDFA::pyAddKeywordTransition(const py::object& srcId, const py::object& keyword, const py::object& destId) {
    if (!addKeywordTransition(srcId, keyword, destId)) {
        throwTransitionSrcNotExists(srcId);
    }
}

void PyDFA::pyAddFuncTransition(const py::object& srcId, const py::function& func, const py::object& destId) {
    std::function<bool(const py::object&)> cppFunc = [func](const py::object& obj) -> bool {
        return func(obj).cast<bool>();
    };

    if (!addFuncTransition(srcId, cppFunc, destId)) {
        throwTransitionSrcNotExists(srcId);
    }
}

void PyDFA::addTransition(const py::object& srcId, const py::object& keyword, const py::object& destId) {
    if (!PyCallable_Check(keyword.ptr())) {
        pyAddKeywordTransition(srcId, keyword, destId);
    } else {
        pyAddFuncTransition(srcId, keyword, destId);
    }
}

void PyDFA::addTransitions(const py::object& srcId, const py::object& keywords, const py::object& destId) {
    if (!py::isinstance<py::list>(keywords)) {
        addTransition(srcId, keywords, destId);
        return;
    }

    for (py::handle keyword: keywords) {
        addTransition(srcId, keyword.cast<py::object>(), destId);
    }
}

std::tuple<py::object, bool, bool> PyDFA::pytransition(const py::object& keyword) {
    py::object newState;
    bool isAccept;
    bool transitionTaken;

    transition(keyword, &newState, &isAccept, &transitionTaken);
    return {newState, isAccept, transitionTaken};
}

void initCppDFA(pybind11::module_ &m) {
    using BaseDFACls = AGRC::BaseDFA<py::object, py::object, PyObjectEqual, PyObjectHash, PyObjectEqual, PyObjectHash>;

    py::class_<BaseDFACls>(m, "BaseDFA")
        .def("clear", &BaseDFACls::clear,
             py::doc(R"doc(
Clears the `DFA`_
            )doc"))

        .def("reset", &BaseDFACls::reset,
             py::doc(R"doc(
Resets the `DFA`_ to return back to its starting state
            )doc"))

        .def("isAccept", &BaseDFACls::isAccept, py::arg("id"), 
            py::doc(R"doc(
Determines whether some state is an accepting state

Paramters
---------
id: `Hashable`_
    The id of the state

Returns
-------
:class:`bool`
    Whether the corresponding state is an accepting state
            )doc"))

        .def("stateExists", &BaseDFACls::stateExists, py::arg("id"), 
            py::doc(R"doc(
Determines whether some state exists in the `DFA`_

Paramters
---------
id: `Hashable`_
    The id of the state

Returns
-------
:class:`bool`
    Whether the id corresponds to a state in the `DFA`_
            )doc"))

        .def("stateLen", &BaseDFACls::stateSize, 
            py::doc(R"doc(
Retrieves the number of states in the `DFA`_

Returns
-------
:class:`int`
    The number of states in the `DFA`_
            )doc"))

        .def("acceptLen", &BaseDFACls::acceptSize, 
            py::doc(R"doc(
Retrieves the number of accepting states in the `DFA`_

Returns
-------
:class:`int`
    The number of accepting states in the `DFA`_
            )doc"))

        .def("isStart", &BaseDFACls::isStart, py::arg("id"), 
            py::doc(R"doc(
Determines whether some state is a starting state

Paramters
---------
id: `Hashable`_
    The id of the state

Returns
-------
:class:`bool`
    Whether the corresponding state is a starting state
            )doc"))

        .def("addState", py::overload_cast<const py::object&, std::optional<bool>, bool>(&BaseDFACls::addState), py::arg("id"), py::arg("isAccept") = py::none(), py::arg("isStart") = false,
            py::doc(R"doc(
Add a new state to the `DFA`

Parameters
----------
id: Hashable
    The id for the state

isAccept: Optional[:class:`bool`]
    Whether the state is an accepting state :raw-html:`<br />` :raw-html:`<br />`

    * If this value is ``None`` and the state already exists, then will not change whether the existing state is accepting or not.
    * Otherwise, if this value is ``None`` and the state does not already exists, then will not set the state as accepting. :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

isStart: :class:`bool`
    Whether to set the state as the new starting state

    .. warning::
        A `DFA`_ can only have 1 start state

    .. warning::
        If the `DFA`_ is empty and you add a new state, will set this state as the start state

    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
:class:`bool`
    Whether the state was newly added
            )doc"));

    py::class_<PyDFA, BaseDFACls>(m, "DFA", 
        R"doc(
Class for a `DFA (Deterministic Finite Automaton)`_
        )doc")

        .def(py::init<>())
        .def_property("startId", &PyDFA::getStartIdOpt, &PyDFA::setPyStartId,
                      py::doc(R"doc(
The id to the start state

.. warning::
    The setter will not set the new id for the state if the newly given start id does not correspond
    to any state within the `DFA`_

:getter: Retrieves the start id
:setter: Sets the new start id
:type: Hashable
                    )doc"))

        .def_property("currentStateId", &PyDFA::getCurrentStateIdOpt, &PyDFA::setPyCurrentStateId,
                      py::doc(R"doc(
The id of the state the `DFA`_ is currently at

.. warning::
    The setter will not set the new id for the state if the newly current id does not correspond
    to any state within the `DFA`_

:getter: Retrieves the id of the current state
:setter: Sets the new id of the current state the `DFA`_ is on
:type: Hashable
                        )doc"))

        .def("addKeywordTransition", &PyDFA::pyAddKeywordTransition, py::arg("srcId"), py::arg("keyword"), py::arg("destId"),
            py::doc(R"doc(
Adds a transition to the `DFA`_ such that the transition is based off a keyword

Parameters
----------
srcId: `Hashable`_
    The id of the source state for the transition

    .. caution::
        The id to the source state must refer to an existing state to the `DFA`_

keyword: `Hashable`_
    The keyword or predicate function that will trigger a transition from the source state to the destination state

    .. warning::
        If the source state already has such a transition, then will overwrite the destination state for this transition

destId: `Hashable`_
    The id of the destionation state for the transition

    .. note::
        The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
        will create a new state in the `DFA`_
            )doc"))

        .def("addFuncTransition", &PyDFA::pyAddFuncTransition, py::arg("srcId"), py::arg("func"), py::arg("destId"),
            py::doc(R"doc(
Adds a transition to the `DFA`_ such that the transition is based off a predicate function

Parameters
----------
srcId: `Hashable`_
    The id of the source state for the transition

    .. caution::
        The id to the source state must refer to an existing state to the `DFA`_

func: Callable[[Hashable], :class:`bool`]
    The predicate function that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`

    The function will take in a keyword as an argument

    .. warning::
        If the source state already has such a transition, then will overwrite the destination state for this transition

destId: `Hashable`_
    The id of the destionation state for the transition

    .. note::
        The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
        will create a new state in the `DFA`_
            )doc"))
    
        .def("addTransition", &PyDFA::addTransition, py::arg("srcId"), py::arg("keyword"), py::arg("destId"),
            py::doc(R"doc(
Adds a transition to the `DFA`_

Parameters
----------
srcId: `Hashable`_
    The id of the source state for the transition

    .. caution::
        The id to the source state must refer to an existing state to the `DFA`_

keyword: Union[`Hashable`_, Callable[[Hashable], :class:`bool`]]
    The keyword or predicate function that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`

    If keyword is a predicate function, the function will take in a keyword as an argument

    .. warning::
        If the source state already has such a transition, then will overwrite the destination state for this transition

destId: `Hashable`_
    The id of the destionation state for the transition

    .. note::
        The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
        will create a new state in the `DFA`_
            )doc"))
            
        .def("addTransitions", &PyDFA::addTransitions, py::arg("srcId"), py::arg("keywords"), py::arg("destId"),
            py::doc(R"doc(
Adds a group of transitions from one state to another state

Parameters
----------
srcId: `Hashable`_
    The id of the source state for the transition

    .. caution::
        The id to the source state must refer to an existing state to the `DFA`_

keywords: Union[List[Union[`Hashable`_, Callable[[Hashable], :class:`bool`]]], `Hashable`_, Callable[[Hashable], :class:`bool`]]
    The keywords or predicate functions that will trigger a transition from the source state to the destination state :raw-html:`<br />` :raw-html:`<br />`

    For predicate functions, the function will take in a keyword as an argument

    .. warning::
        If the source state already has such a transition, then will overwrite the destination state for this transition

destId: `Hashable`_
    The id of the destionation state for the transition

    .. note::
        The id of this state does not need to exist yet in the `DFA`_ . If the id of this state does not exist, then
        will create a new state in the `DFA`_
            )doc"))
            
        .def("transition", &PyDFA::pytransition, py::arg("keyword"),
            py::doc(R"doc(
Transitions to a new state

Parameters
----------
keyword: Hashable
    The keyword to trigger the transition to the new state

Returns
-------
Tuple[Hashable, :class:`bool`, :class:`bool`]
    Resultant data regarding the new transitioned state, which includes:

    #. The id of the new state
    #. Whether the new state is an accepting state
    #. Whether a transition was taken 
            )doc"));
}