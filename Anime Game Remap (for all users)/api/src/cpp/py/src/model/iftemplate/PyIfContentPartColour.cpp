#include "PyIfContentPartColour.h"

#include "../../tools/PyRanges.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// StateValue is std::variant<py::object, std::vector<std::pair<long long, py::object>>> --
// pybind11's automatic std::variant support can't be used to *load* this from Python (same issue
// as KeyRemapValue/ReplaceSpec in PyOrderedMultiMap.cpp: the py::object alternative trivially
// accepts anything, including a Python list that should actually hit the vector alternative), so
// this dispatches manually, matching this project's established convention for that case. The
// reverse (C++ -> Python) direction has no such ambiguity (the active alternative is already
// known), but is still done by hand here too, for symmetry and to avoid relying on pybind11's
// variant caster being available at all.
PyIfContentPartColouring::StateValue parseStateValue(const py::object &value) {
    if (py::isinstance<py::list>(value)) {
        return PyIfContentPartColouring::StateValue(value.cast<std::vector<std::pair<long long, std::string>>>());
    }
    return PyIfContentPartColouring::StateValue(py::str(value).cast<std::string>());
}

std::optional<PyIfContentPartColouring::StateValue> parseOptionalStateValue(const py::object &value) {
    if (value.is_none()) {
        return std::nullopt;
    }
    return parseStateValue(value);
}

py::object stateValueToPy(const PyIfContentPartColouring::StateValue &value) {
    if (std::holds_alternative<std::string>(value)) {
        return py::cast(std::get<std::string>(value));
    }
    return py::cast(std::get<std::vector<std::pair<long long, std::string>>>(value));
}

py::object optionalStateValueToPy(const std::optional<PyIfContentPartColouring::StateValue> &value) {
    if (!value.has_value()) {
        return py::none();
    }
    return stateValueToPy(*value);
}

std::vector<std::pair<std::string, py::object>> itemsAsTuples(const PyIfContentPartColouring &self) {
    std::vector<std::pair<std::string, py::object>> result;
    for (const auto &entry : self.items()) {
        result.emplace_back(entry.first, stateValueToPy(entry.second));
    }
    return result;
}

}


void initCppIfContentPartColour(pybind11::module_ &m) {
    py::class_<PyIfContentPartColourChange> changeCls(m, "IfContentPartColourChange", R"doc(
Class to store the change in state of a particular key for a :class:`IfContentPartColouring`

Parameters
----------
old: Optional[Any]
    The old value of a particular key -- either a plain value (the key's value came from some
    previous :class:`IfContentPart`), or a ``List[Tuple[int, Any]]`` (the key's values come
    from the current :class:`IfContentPart`, each paired with its index of occurrence) :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the key didn't exist beforehand
        )doc");

    changeCls
        .def(py::init([](const py::object &old) {
            return std::make_unique<PyIfContentPartColourChange>(parseOptionalStateValue(old));
        }), py::arg("old") = py::none())

        .def_property("old",
            [](const PyIfContentPartColourChange &self) { return optionalStateValueToPy(self.old); },
            [](PyIfContentPartColourChange &self, const py::object &old) { self.old = parseOptionalStateValue(old); },
    py::doc(R"doc(Optional[Any]: The old value of a particular key)doc"))

        .def("clone", &PyIfContentPartColourChange::clone, py::doc(R"doc(Creates a copy of this change record)doc"))

        .def("__copy__", &PyIfContentPartColourChange::clone, py::doc(R"doc(Creates a copy of this change record (equivalent to :meth:`clone`); supports ``copy.copy()``)doc"))
        .def("__deepcopy__", [](PyIfContentPartColourChange &self, py::dict) { return self.clone(); }, py::arg("memo"),
    py::doc(R"doc(Creates a copy of this change record (equivalent to :meth:`clone`); supports ``copy.deepcopy()``)doc"));


    py::class_<PyIfContentPartColouring>(m, "IfContentPartColouring", R"doc(
Class that keeps track of the current state of the `KVPs`_ within a :class:`IfContentPart` --
the C++-backed port of the deprecated pure-Python original (since removed)

:raw-html:`<br />`

.. container:: operations

    **Supported Operations:**

    .. describe:: key in x

        Determines if 'key' currently has a tracked state

    .. describe:: len(x)

        Retrieves the number of keys currently tracked

    .. describe:: x[key]

        Retrieves the current state for 'key'; raises :class:`KeyError` if not tracked

    .. describe:: x[key] = value

        Sets the current state for 'key'

    .. describe:: del x[key]

        Removes the current state for 'key'; raises :class:`KeyError` if not tracked

    .. describe:: iter(x)

        Iterates every currently-tracked key, in insertion order

:raw-html:`<br />` :raw-html:`<br />`

* The keys are the names of the register keys
* The values are either:

    * A plain value, indicating the value of the `KVP`_ comes from some previous :class:`IfContentPart`, OR
    * A ``List[Tuple[int, Any]]``. The list indicates that the values of the corresponding key
      come from the current :class:`IfContentPart`, each tuple containing the new state value
      for the corresponding key and its index of occurrence within the current part

Parameters
----------
src: Optional[Dict[Any, Any]]
    Initial state to populate this colouring with, in the same key -> value shape described above :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
        )doc")

        .def(py::init([](const py::object &src) {
            auto instance = std::make_unique<PyIfContentPartColouring>();
            if (!src.is_none()) {
                for (auto item : src.cast<py::dict>()) {
                    std::string key = py::str(item.first).cast<std::string>();
                    py::object value = py::reinterpret_borrow<py::object>(item.second);
                    instance->set(key, parseStateValue(value));
                }
            }
            return instance;
        }), py::arg("src") = py::none())

        .def("clone", &PyIfContentPartColouring::clone, py::doc(R"doc(Creates a copy of this colouring)doc"))

        .def("__copy__", &PyIfContentPartColouring::clone, py::doc(R"doc(Creates a copy of this colouring (equivalent to :meth:`clone`); supports ``copy.copy()``)doc"))
        .def("__deepcopy__", [](PyIfContentPartColouring &self, py::dict) { return self.clone(); }, py::arg("memo"),
    py::doc(R"doc(Creates a copy of this colouring (equivalent to :meth:`clone`); supports ``copy.deepcopy()``)doc"))

        .def("contains", &PyIfContentPartColouring::contains, py::arg("key"), py::doc(R"doc(Checks whether 'key' currently has a tracked state)doc"))
        .def("__contains__", &PyIfContentPartColouring::contains, py::arg("key"), py::doc(R"doc(Determines whether 'key' currently has a tracked state)doc"))

        .def("size", &PyIfContentPartColouring::size, py::doc(R"doc(Retrieves the number of keys currently tracked)doc"))
        .def("__len__", &PyIfContentPartColouring::size, py::doc(R"doc(Retrieves the number of keys currently tracked)doc"))

        .def("empty", &PyIfContentPartColouring::empty, py::doc(R"doc(Checks whether no keys are currently tracked)doc"))

        .def("get", [](const PyIfContentPartColouring &self, const std::string &key, const py::object &defaultVal) {
            auto val = self.get(key);
            if (!val.has_value()) {
                return defaultVal;
            }
            return stateValueToPy(*val);
        }, py::arg("key"), py::arg("default") = py::none(),
    py::doc(R"doc(Retrieves the current state for 'key', or 'default' if not tracked)doc"))

        .def("__getitem__", [](const PyIfContentPartColouring &self, const std::string &key) {
            try {
                return stateValueToPy(self.at(key));
            } catch (const std::out_of_range&) {
                throw py::key_error(py::str(key));
            }
        }, py::arg("key"), py::doc(R"doc(Retrieves the current state for 'key'; raises :class:`KeyError` if not tracked)doc"))

        .def("set", [](PyIfContentPartColouring &self, const std::string &key, const py::object &value) {
            self.set(key, parseStateValue(value));
        }, py::arg("key"), py::arg("value"), py::doc(R"doc(Sets the current state for 'key', inserting it if not already tracked)doc"))

        .def("__setitem__", [](PyIfContentPartColouring &self, const std::string &key, const py::object &value) {
            self.set(key, parseStateValue(value));
        }, py::arg("key"), py::arg("value"), py::doc(R"doc(Sets the current state for 'key', inserting it if not already tracked)doc"))

        .def("erase", &PyIfContentPartColouring::erase, py::arg("key"),
    py::doc(R"doc(Removes the current state for 'key', if any; returns whether 'key' was actually tracked)doc"))

        .def("__delitem__", [](PyIfContentPartColouring &self, const std::string &key) {
            if (!self.erase(key)) {
                throw py::key_error(py::str(key));
            }
        }, py::arg("key"), py::doc(R"doc(Removes the current state for 'key'; raises :class:`KeyError` if not tracked)doc"))

        .def("clear", &PyIfContentPartColouring::clear, py::doc(R"doc(Removes every tracked key)doc"))

        .def("keys", &PyIfContentPartColouring::keys, py::doc(R"doc(Retrieves every currently-tracked key, in insertion order)doc"))

        .def("items", &itemsAsTuples, py::doc(R"doc(Retrieves every currently-tracked ``(key, state)`` pair, in insertion order)doc"))

        .def("__iter__", [](const PyIfContentPartColouring &self) {
            return py::iter(py::cast(self.keys()));
        }, py::doc(R"doc(Iterates every currently-tracked key, in insertion order)doc"))

        .def("updateColouring", &PyIfContentPartColouring::updateColouring,
             py::arg("ifContentPart"), py::arg("targetKeys") = py::none(), py::arg("updatePreviousKVPs") = true,
    py::doc(R"doc(
Updates the current state of the `KVPs`_ based on the current :class:`IfContentPart`

Parameters
----------
ifContentPart: :class:`IfContentPart`
    The part to update the new `KVPs`_ from

targetKeys: Optional[Set[Any]]
    The target keys to keep track of :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None``, then will keep track of all the keys :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

updatePreviousKVPs: :class:`bool`
    Whether to also update the `KVP`_ values from previous :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

Returns
-------
Dict[Any, :class:`IfContentPartColourChange`]
    The change in the state :raw-html:`<br />` :raw-html:`<br />`

    The keys are the names of the keys and the values are the state change for the keys
        )doc"))

        .def("restore", &PyIfContentPartColouring::restore, py::arg("colourChange"),
    py::doc(R"doc(
Restores to a previous state

Parameters
----------
colourChange: Dict[Any, :class:`IfContentPartColourChange`]
    The change in the state, as returned by :meth:`updateColouring`
        )doc"))

        .def("getIndVals", &PyIfContentPartColouring::getIndVals, py::arg("key"), py::arg("filter") = py::none(),
    py::doc(R"doc(
Retrieves both the corresponding values and the index of where the value occurs

.. note::
    Unlike :meth:`getVals`, ``filter`` is only ever applied when ``key``'s state comes from the
    current :class:`IfContentPart` (a list of indexed occurrences) -- a value carried over from
    a previous part is always returned unfiltered, as ``(None, value)``.

Parameters
----------
key: Any
    The key to search for

filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
    A predicate to filter certain values returned :raw-html:`<br />` :raw-html:`<br />`

    The predicate takes in the following parameters:

    #. The index the value appears in the current :class:`IfContentPart`. If this argument is
       ``None``, then the value was carried over from a previous part
    #. The corresponding value

    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
List[Tuple[Optional[:class:`int`], Any]]
    Both the values and their index within the current :class:`IfContentPart`. Empty if ``key``
    isn't tracked.
        )doc"))

        .def("getVals", &PyIfContentPartColouring::getVals, py::arg("key"), py::arg("filter") = py::none(),
    py::doc(R"doc(
Retrieves the values for a given key, keeping duplicates and occurrence order

Parameters
----------
key: Any
    The key to search for

filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
    Same meaning as :meth:`getIndVals`'s own ``filter`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
List[Any]
    The resultant values. Empty if ``key`` isn't tracked.
        )doc"))

        .def("getUniqueVals", &PyIfContentPartColouring::getUniqueVals, py::arg("key"), py::arg("filter") = py::none(),
    py::doc(R"doc(
Same as :meth:`getVals`, except the result is deduplicated into a real ``set`` -- a departure from
the deprecated Python source's own ``getVals(unique=True)``, split into its own method the same
way :class:`IfContentPart` itself splits ``getVals``/``getKeys`` rather than returning a value
whose type depends on an argument

Parameters
----------
key: Any
    The key to search for

filter: Optional[Callable[[Optional[:class:`int`], Any], :class:`bool`]]
    Same meaning as :meth:`getIndVals`'s own ``filter`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
Set[Any]
    The resultant unique values. Empty if ``key`` isn't tracked.
        )doc"))

        .def("getRanges", [](const PyIfContentPartColouring &self,
                              const std::optional<std::unordered_map<std::string, bool>> &keysExists,
                              const std::optional<std::unordered_map<std::string, PyIfContentPartColouring::Filter>> &keyFilters,
                              bool existsRequireAll, bool filtersRequireAll, bool globalRequireAll, bool includeKeyDefs) {
            AGRC::Ranges<long long> result = self.getRanges(keysExists, keyFilters, existsRequireAll, filtersRequireAll, globalRequireAll, includeKeyDefs);
            return PyRanges<long long>(result.ranges, false);
        }, py::arg("keysExists") = py::none(), py::arg("keyFilters") = py::none(),
           py::arg("existsRequireAll") = true, py::arg("filtersRequireAll") = true,
           py::arg("globalRequireAll") = true, py::arg("includeKeyDefs") = true,
    py::doc(R"doc(
Retrieves the ranges of indices within the current part that satisfy specified conditions for each key

Parameters
----------
keysExists: Optional[Dict[Any, :class:`bool`]]
    Checks whether a key exists or does not exist :raw-html:`<br />` :raw-html:`<br />`

    The keys are the names of the registers and the values are whether to check for the
    existence/non-existence of the register :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

keyFilters: Optional[Dict[Any, Callable[[Optional[:class:`int`], Any], :class:`bool`]]]
    The conditions to satisfy for each key :raw-html:`<br />` :raw-html:`<br />`

    The keys are the names of the registers and the values are the predicates, taking the same
    parameters as :meth:`getIndVals`'s own ``filter`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

existsRequireAll: :class:`bool`
    Whether the retrieved ranges must satisfy all existence checks at ``keysExists`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

filtersRequireAll: :class:`bool`
    Whether the retrieved ranges must satisfy all the predicates specified at ``keyFilters`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

globalRequireAll: :class:`bool`
    Whether the retrieved ranges must satisfy checks in both ``keysExists`` and ``keyFilters`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

includeKeyDefs: :class:`bool`
    Whether to include indices where the values for the keys specified at ``keysExists`` or
    ``keyFilters`` are being (re)defined :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

Returns
-------
:class:`Ranges`
    The valid ranges that satisfy the specified conditions
        )doc"));

    // Registered only now that IfContentPartColouring exists: pybind11 bakes a def()'s signature
    // string at registration time, so declaring this alongside the rest of IfContentPartColourChange
    // rendered 'colouring' as a raw C++ template name -- which pybind11_stubgen then split on its
    // commas into a run of bogus 'std' parameters in core.pyi.
    changeCls.def("restore", &PyIfContentPartColourChange::restore<std::string, std::hash<std::string>, std::equal_to<std::string>,
                                                                  std::hash<std::string>, std::equal_to<std::string>>,
             py::arg("colouring"), py::arg("key"),
    py::doc(R"doc(
Restores the old value for a particular key within ``colouring``

Parameters
----------
colouring: :class:`IfContentPartColouring`
    The colouring to restore a value within

key: :class:`str`
    The key to restore -- if ``key`` isn't currently in ``colouring``, this has no effect
        )doc"));
}
