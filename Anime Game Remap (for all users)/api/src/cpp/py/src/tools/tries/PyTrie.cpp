#include "PyTrie.h"
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;


template class AGRC::BaseTrie<py::object>;


PyTrie::PyTrie(const std::optional<std::unordered_map<std::string, py::object>> &data,
               const std::optional<DupHandler2> &handler)
    : AGRC::BaseTrie<py::object>(data, handler) {}


py::object PyTrie::pyOptGet(const std::string &keyword, bool errorOnNotFound, const py::object &defaultRes) {
    const py::object *resultPtr = getPtr(keyword);
    if (resultPtr != nullptr) return *resultPtr;

    if (errorOnNotFound) {
        throw py::key_error("the Trie does not contain the keyword, '" + keyword + "'");
    } else {
        return defaultRes;
    }
}

py::object PyTrie::pyGetItem(const std::string &keyword) {
    return pyOptGet(keyword);
}


py::object PyBindTrie::pyOptGet(const std::string &keyword, bool errorOnNotFound, const py::object &defaultRes) {
    PYBIND11_OVERRIDE_NAME(
        py::object,
        PyTrie,
        "get",
        pyOptGet,
        keyword, errorOnNotFound, defaultRes
    );
}

py::object PyBindTrie::pyGetItem(const std::string &keyword) {
    PYBIND11_OVERRIDE_NAME(
        py::object,
        PyTrie,
        "__getitem__",
        pyGetItem,
        keyword
    );
}

bool PyBindTrie::contains(const std::string &keyword) {
    auto overload = static_cast<bool (PyTrie::*)(const std::string&)>(&PyTrie::contains);

    PYBIND11_OVERRIDE_NAME(
        bool,
        PyTrie,
        "__contains__",
        contains,
        keyword
    );
}

void PyBindTrie::clear() {
    PYBIND11_OVERRIDE(
        void,
        PyTrie,
        clear
    );
}

size_t PyBindTrie::size() {
    PYBIND11_OVERRIDE_NAME(
        size_t,
        PyTrie,
        "__len__",
        size
    );
}

void PyBindTrie::build(const std::optional<std::unordered_map<std::string, py::object>> &data) {
    PYBIND11_OVERRIDE(
        void,
        PyTrie,
        build,
        data
    );
}

bool PyBindTrie::add(const std::string &key, const py::object &val) {
    auto overload = static_cast<bool (PyTrie::*)(const std::string &, const py::object &)>(&PyTrie::add);

    PYBIND11_OVERRIDE(
        bool,
        PyTrie,
        add,
        key, val
    );
}


void initCppTrie(pybind11::module_ &m) {
    py::class_<PyTrie, PyBindTrie>(m, "CppTrie", R"doc(
A class for a basic `trie`_ implemented in C++

:raw-html:`<br />`

.. container:: operations

    **Supported Operations:**

    .. describe:: key in x

        Determines if 'key' is found

    .. describe:: x[key]

        Retrieves the corresponding value to 'key'

    .. describe:: x[key] = val

        Sets the new `KVP`_

    .. describe:: len(x)

        Retrieves the number of elements

Parameters
----------
data: Optional[Dict[:class:`str`, T]]
    Any initial data to insert :raw-html:`<br />` :raw-html:`<br />`

    The keys are the keywords to put into the `trie`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

handleDuplicate: Optional[Callable[[:class:`str`, T, T], T]]
    Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`

    The function takes in the following parameters:

    #. The duplicate keyword in both `KVPs`_
    #. The value of the existing `KVP`_
    #. The value of the new `KVP`_

    If this value is ``None``, will return the value of the new `KVP`_ by default :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
        )doc")

        .def(py::init<const std::optional<std::unordered_map<std::string, py::object>> &, const std::optional<std::function<py::object(const std::string&, const py::object&, const py::object&)>>&>(),
             py::arg("data") = py::none(), py::arg("handleDuplicate") = py::none())

        .def_property("handleDuplicate", &PyTrie::getHandleDuplicateStrRef, py::overload_cast<const PyTrie::DupHandler2 &>(&PyTrie::setHandleDuplicate),
    py::doc(R"doc(
Function to handle the case where 2 `KVPs`_ inserted have the same key(word) :raw-html:`<br />` :raw-html:`<br />`

The function takes in the following parameters:

#. The duplicate keyword in both `KVPs`_
#. The value of the existing `KVP`_
#. The value of the new `KVP`_

:getter: Retrieves the function
:setter: Sets the new function
:type: Callable[[:class:`str`, T, T], T]
                        )doc"))
                        
    .def("__getitem__", &PyTrie::pyGetItem, py::arg("key"), 
    py::doc(R"doc(Retrieves the corresponding value to 'key')doc"))
    
    .def("__setitem__", py::overload_cast<const std::string &, const py::object &>(&PyTrie::add), py::arg("key"), py::arg("val"),
    py::doc(R"doc(Sets the new `KVP`_)doc"))
    
    .def("__contains__", py::overload_cast<const std::string &>(&PyTrie::contains), py::arg("key"),
    py::doc(R"doc(Determines if 'key' is found)doc"))

    .def("__len__", &PyTrie::size,
    py::doc(R"doc(Retrieves the number of elements)doc"))
    
    .def("clear", &PyTrie::clear,
    py::doc(R"doc(Clears the data)doc"))
    
    .def("build", &PyTrie::build, py::arg("data") = py::none(),
    py::doc(R"doc(
Rebuilds the `trie`_

Parameters
----------
data: Optional[Dict[:class:`str`, T]]
    Any initial data to put into the `trie`_ :raw-html:`<br />` :raw-html:`<br />`

    The keys are the keywords to put into the trie and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
        )doc"))
        
    .def("add", py::overload_cast<const std::string &, const py::object &>(&PyTrie::add), py::arg("keyword"), py::arg("value"),
    py::doc(R"doc(
Adds a new keyword

Parameters
----------
keyword: :class:`str`
    The keyword to add

value: T
    The value associated with the keyword

Returns
-------
:class:`bool`
    Whether the keyword has already been inserted
    )doc"))
        
    .def("get", &PyTrie::pyOptGet, py::arg("keyword"), py::arg("errorOnNotFound") = true, py::arg("default") = py::none(),
    py::doc(R"doc(
Retrieves the corresponding value to 'keyword'

Parameters
----------
keyword: :class:`str`
    The keyword to get the corresponding value for

errorOnNotFound: :class:`bool`  
    If the keyword is not found, whether to raise an exception

default: Any
    If 'errorOnNotFound' is ``False``, then the default value to return if 'keyword' is not found

Raises
------
:class:`KeyError`
    If 'keyword' is not found

Returns
-------
Union[T, Any]
    Either the found value for the keyword or the value specified at 'default', if 'keyword' is not found and
    'errorOnNotFound' is set to ``False``
        )doc"));
}