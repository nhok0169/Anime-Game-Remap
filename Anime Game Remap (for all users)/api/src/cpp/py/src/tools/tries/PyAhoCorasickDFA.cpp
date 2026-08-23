#include "PyAhoCorasickDFA.h"


namespace py = pybind11;


using PyOptGetReturn = std::tuple<std::optional<std::string>, py::object>;
using PyFindReturn = std::tuple<std::optional<std::string>, size_t>;
using PyFindAllReturn = std::unordered_map<std::string, std::vector<std::tuple<size_t, size_t>>>;
using PyFindFirstAllReturn = std::unordered_map<std::string, std::tuple<size_t, size_t>>;
using PyFindMaximalReturn = std::variant<std::tuple<std::optional<std::string>, size_t>, std::tuple<std::vector<std::string>, std::vector<size_t>>>;
using PyGetMaximalReturn = std::variant<std::tuple<std::optional<std::string>, py::object>, std::tuple<std::vector<std::string>, std::vector<py::object>>>;
using PyGetReturn = std::tuple<std::optional<std::string>, py::object>;
using PyGetAllReturn = std::unordered_map<std::string, py::object>;
using PyMaximalStartsWithReturn = std::optional<std::string>;
using PyGetKeyValReturn = py::object;


template class AGRC::BaseAhoCorasickDFA<py::object>;


PyAhoCorasickDFA::PyAhoCorasickDFA(const std::optional<std::unordered_map<std::string, py::object>> &data,
                                   const std::optional<DupHandler2> &handler)
    : AGRC::BaseAhoCorasickDFA<py::object>(data, handler) {}

static void pyRaiseNoKeyword(const std::string &txt) {
    throw py::key_error("the AhoCorasick DFA does not contain any keyword for the given text: '" + txt + "'");
}


std::tuple<std::optional<std::string>, py::object> PyAhoCorasickDFA::pyOptGet(const std::string &txt, bool errorOnNotFound, const py::object &defaultRes) {
    size_t resultInd = 0;
    const std::string *keyword = findPtr(txt, &resultInd);

    if (keyword != nullptr) {
        std::uint64_t keywordId = keywords.getKey(*keyword);
        py::object val = vals.at(keywordId);
        return {*keyword, val};
    }

    if (errorOnNotFound) {
        pyRaiseNoKeyword(txt);
    } else {
        return {std::nullopt, defaultRes};
    }
}

py::object PyAhoCorasickDFA::pyGetItem(const std::string &txt) {
    size_t keywordInd = 0;
    const std::string *keyword = findMaximalPtr(txt, &keywordInd);

    if (keyword == nullptr) {
        pyRaiseNoKeyword(txt);
    }

    const std::uint64_t &keywordId = this->keywords.getKey(*keyword);
    const py::object &val = this->vals.at(keywordId);
    return val;
}

std::tuple<std::optional<std::string>, size_t> PyAhoCorasickDFA::pyFind(const std::string &txt) {
    size_t resultInd = 0;
    const std::string *keywordPtr = findPtr(txt, &resultInd);

    if (keywordPtr == nullptr) {
        return {std::nullopt, resultInd};
    }
    return {*keywordPtr, resultInd};
}

std::variant<std::tuple<std::optional<std::string>, size_t>, std::tuple<std::vector<std::string>, std::vector<size_t>>> PyAhoCorasickDFA::pyFindMaximal(const std::string &txt, size_t count, const std::optional<KeywordPredicate> &pred) {
    if (count <= 1) {
        size_t keywordInd;
        const std::string *keyword = findMaximalPtr(txt, &keywordInd, pred);

        if (keyword == nullptr) {
            return std::make_tuple(std::nullopt, keywordInd);
        }

        return std::make_tuple(*keyword, keywordInd);
    }

    auto [views, indices] = findMaximal(txt, count, pred);
    std::vector<std::string> safeStrings(views.begin(), views.end());
    return std::make_tuple(std::move(safeStrings), std::move(indices));
}

static void raiseNoKeywordsFound(const std::string &txt) {
    throw py::key_error("The text, '" + txt + "', does not contain any matching keywords");
}

std::tuple<std::optional<std::string>, py::object> PyAhoCorasickDFA::pyGet(const std::string &txt, bool errorOnNotFound, const py::object &defaultVal) {
    auto [keywordPtr, valPtr] = getKVPPtr(txt);
    if (keywordPtr == nullptr && errorOnNotFound) {
        raiseNoKeywordsFound(txt);
    } else if (keywordPtr == nullptr) {
        return std::make_tuple(std::nullopt, defaultVal);
    }

    return std::make_tuple(*keywordPtr, *valPtr);
}

std::unordered_map<std::string, py::object> PyAhoCorasickDFA::pyGetAll(const std::string &txt) {
    auto result = getAll(txt);

    std::unordered_map<std::string, py::object> convertedMap;
    convertedMap.reserve(result.size());

    for (const auto& [key, ptr] : result) {
        if (ptr != nullptr) {
            convertedMap[key] = *ptr; 
        } else {
            convertedMap[key] = py::none();
        }
    }

    return convertedMap; 
}

std::variant<std::tuple<std::optional<std::string>, py::object>, std::tuple<std::vector<std::string>, std::vector<py::object>>> PyAhoCorasickDFA::pyGetMaximal(const std::string &txt, bool errorOnNotFound, const py::object &defaultVal, size_t count, const std::optional<KeywordPredicate> &pred) {
    if (count <= 1) {
        auto [resKeyword, resVal] = getMaximalPtr(txt, pred);
        if (resKeyword == nullptr && errorOnNotFound) {
            raiseNoKeywordsFound(txt);
        } else if (resKeyword == nullptr) {
            return std::make_tuple(std::nullopt, defaultVal);
        }

        return std::make_tuple(*resKeyword, *resVal);
    }

    auto [resKeywords, resVals] = getMaximal(txt, count, pred);

    bool noResult = resKeywords.empty();
    if (noResult && errorOnNotFound) {
        raiseNoKeywordsFound(txt);
    } else if (noResult) {
        std::vector<std::string> emptyKeywords;
        std::vector<py::object> emptyVals;
        return std::make_tuple(std::move(emptyKeywords), std::move(emptyVals));
    }

    std::vector<std::string> safeKeywords(resKeywords.begin(), resKeywords.end());
    std::vector<py::object> safeVals;
    safeVals.reserve(resVals.size());

    for (const py::object* ptr : resVals) {
        if (ptr != nullptr) {
            safeVals.push_back(*ptr);
        } else {
            safeVals.push_back(py::none());
        }
    }

    return std::make_tuple(std::move(safeKeywords), std::move(safeVals));
}

std::optional<std::string> PyAhoCorasickDFA::pyMaximalStartsWith(const std::string &txt) {
    auto resultPtr = maximalStartsWithPtr(txt);
    if (resultPtr == nullptr) {
        return std::nullopt;
    }

    return *resultPtr;
}

py::object PyAhoCorasickDFA::pyGetKeyVal(const std::string &txt, bool errorOnNotFound, const py::object &defaultVal) {
    auto resultPtr = getKeyValPtr(txt);
    if (resultPtr == nullptr && errorOnNotFound) {
        raiseNoKeywordsFound(txt);
    } else if (resultPtr == nullptr) {
        return defaultVal;
    }

    return *resultPtr;
}


PyOptGetReturn PyBindAhoCorasickDFA::pyOptGet(const std::string &txt, bool errorOnNotFound, const py::object &defaultRes) {
    PYBIND11_OVERRIDE_NAME(
        PyOptGetReturn,
        PyAhoCorasickDFA,
        "get",
        pyOptGet,
        txt, errorOnNotFound, defaultRes
    );
}

py::object PyBindAhoCorasickDFA::pyGetItem(const std::string &keyword) {
    PYBIND11_OVERRIDE_NAME(
        py::object,
        PyAhoCorasickDFA,
        "__getitem__",
        pyGetItem,
        keyword
    );
}

PyFindReturn PyBindAhoCorasickDFA::pyFind(const std::string &txt) {
    PYBIND11_OVERRIDE_NAME(
        PyFindReturn,
        PyAhoCorasickDFA,
        "find",
        pyFind,
        txt
    );
}

bool PyBindAhoCorasickDFA::contains(const std::string &txt) {
    PYBIND11_OVERRIDE_NAME(
        bool,
        PyAhoCorasickDFA,
        "__contains__",
        contains,
        txt
    );
}

void PyBindAhoCorasickDFA::clear() {
    PYBIND11_OVERRIDE(
        void,
        PyAhoCorasickDFA,
        clear
    );
}

size_t PyBindAhoCorasickDFA::size() {
    PYBIND11_OVERRIDE_NAME(
        size_t,
        PyAhoCorasickDFA,
        "__len__",
        size,
    );
}

void PyBindAhoCorasickDFA::build(const std::optional<std::unordered_map<std::string, py::object>> &data) {
    PYBIND11_OVERRIDE(
        void,
        PyAhoCorasickDFA,
        build,
        data
    );
}

bool PyBindAhoCorasickDFA::add(const std::string &key, const py::object &val) {
    auto overload = static_cast<bool (PyAhoCorasickDFA::*)(const std::string &, const py::object &)>(&PyAhoCorasickDFA::add);

    PYBIND11_OVERRIDE(
        bool,
        PyAhoCorasickDFA,
        add,
        key, val
    );
}

PyFindAllReturn PyBindAhoCorasickDFA::findAll(const std::string &txt) {
    auto overload = static_cast<PyFindAllReturn (PyAhoCorasickDFA::*)(const std::string&)>(&PyAhoCorasickDFA::findAll);

    PYBIND11_OVERRIDE(
        PyFindAllReturn,
        PyAhoCorasickDFA,
        findAll,
        txt
    );
}

PyFindFirstAllReturn PyBindAhoCorasickDFA::findFirstAll(const std::string &txt) {
    auto overload = static_cast<PyFindFirstAllReturn (PyAhoCorasickDFA::*)(const std::string&)>(&PyAhoCorasickDFA::findFirstAll);

    PYBIND11_OVERRIDE(
        PyFindFirstAllReturn,
        PyAhoCorasickDFA,
        findFirstAll,
        txt
    );
}

PyFindMaximalReturn PyBindAhoCorasickDFA::pyFindMaximal(const std::string &txt, size_t count, const std::optional<KeywordPredicate> &pred) {
    PYBIND11_OVERRIDE_NAME(
        PyFindMaximalReturn,
        PyAhoCorasickDFA,
        "findMaximal",
        pyFindMaximal,
        txt,
        count,
        pred
    );
}

PyGetAllReturn PyBindAhoCorasickDFA::pyGetAll(const std::string &txt) {
    PYBIND11_OVERRIDE_NAME(
        PyGetAllReturn,
        PyAhoCorasickDFA,
        "getAll",
        pyGetAll,
        txt
    );
}

PyGetMaximalReturn PyBindAhoCorasickDFA::pyGetMaximal(const std::string &txt, bool errorOnNotFound, const py::object &defaultVal, size_t count, const std::optional<KeywordPredicate> &pred) {
    PYBIND11_OVERRIDE_NAME(
        PyGetMaximalReturn,
        PyAhoCorasickDFA,
        "getMaximal",
        pyGetMaximal,
        txt,
        errorOnNotFound,
        defaultVal,
        count,
        pred
    );
}

PyGetReturn PyBindAhoCorasickDFA::pyGet(const std::string &txt, bool errorOnNotFound, const py::object &defaultVal) {
    PYBIND11_OVERRIDE_NAME(
        PyGetReturn,
        PyAhoCorasickDFA,
        "get",
        pyGet,
        txt,
        errorOnNotFound,
        defaultVal
    );
}

PyMaximalStartsWithReturn PyBindAhoCorasickDFA::pyMaximalStartsWith(const std::string &txt) {
    PYBIND11_OVERRIDE_NAME(
        PyMaximalStartsWithReturn,
        PyAhoCorasickDFA,
        "maximalStartsWith",
        pyMaximalStartsWith,
        txt
    );
}

PyGetKeyValReturn PyBindAhoCorasickDFA::pyGetKeyVal(const std::string &txt, bool errorOnNotFound, const py::object &defaultVal) {
    PYBIND11_OVERRIDE_NAME(
        PyGetKeyValReturn,
        PyAhoCorasickDFA,
        "getKeyVal",
        pyGetKeyVal,
        txt,
        errorOnNotFound,
        defaultVal
    );
}

void initCppAhoCorasickDFA(pybind11::module_ &m) {
    py::class_<PyAhoCorasickDFA, PyBindAhoCorasickDFA>(m, "CppAhoCorasickDFA", R"doc(
A class for an `Aho-Corasick`_ `DFA`_ implemented in C++

:raw-html:`<br />`

.. container:: operations

    **Supported Operations:**

    .. describe:: txt in x

        Determines if a keyword is found within 'txt'

    .. describe:: x[keyword]

        Retrieves the corresponding value to 'keyword'

    .. describe:: x[keyword] = val

        Sets the new `KVP`_

    .. describe:: len(x)

        Retrieves the number of elements

Parameters
----------
data: Optional[Dict[:class:`str`, T]]
    Any initial data to insert :raw-html:`<br />` :raw-html:`<br />`

    The keys are the keywords to put into the `DFA`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

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

        .def_property("handleDuplicate", &PyAhoCorasickDFA::getHandleDuplicateStrRef, py::overload_cast<const PyAhoCorasickDFA::DupHandler2 &>(&PyAhoCorasickDFA::setHandleDuplicate),
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

    .def("__getitem__", &PyAhoCorasickDFA::pyGetItem, py::arg("keyword"),
    py::doc(R"doc(Retrieves the corresponding value to 'keyword')doc"))

    .def("__setitem__", py::overload_cast<const std::string &, const py::object &>(&PyAhoCorasickDFA::add), py::arg("keyword"), py::arg("val"),
    py::doc(R"doc(Sets the new `KVP`_)doc"))

    .def("__contains__", &PyAhoCorasickDFA::contains, py::arg("txt"),
    py::doc(R"doc(Determines if a keyword is found within 'txt')doc"))

    .def("clear", &PyAhoCorasickDFA::clear,
    py::doc(R"doc(Clears the data)doc"))

    .def("__len__", &PyAhoCorasickDFA::size,
    py::doc(R"doc(Retrieves the number of elements)doc"))

    .def("build", &PyAhoCorasickDFA::build, py::arg("data") = py::none(),
    py::doc(R"doc(
Rebuilds the `DFA`_

Parameters
----------
data: Optional[Dict[:class:`str`, T]]
    Any initial data to put into the `DFA`_ :raw-html:`<br />` :raw-html:`<br />`

    The keys are the keywords to put into the `DFA`_ and the values are the corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
        )doc"))

    .def("add", py::overload_cast<const std::string &, const py::object &>(&PyAhoCorasickDFA::add), py::arg("keyword"), py::arg("value"),
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

    .def("get", &PyAhoCorasickDFA::pyOptGet, py::arg("txt"), py::arg("errorOnNotFound") = true, py::arg("default") = py::none(),
    py::doc(R"doc(
Retrieves the corresponding value from the first keyword fround in 'txt'

.. note::
    This function retrieves the corresponding value after running :meth:`find`

Parameters
----------
txt: :class:`str`
    The text to search for a keyword

errorOnNotFound: :class:`bool`  
    If no keywords are found, whether to raise an exception

default: Any
    If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found

Raises
------
:class:`KeyError`
    If no keywords are found

Returns
-------
Tuple[Optional[:class:`str`], Union[T, Any]]
    Retrieves the following resultant data:

    #. The first keyword found
    #. Either the found value for the first keyword found or the value specified at 'default', if no keywords were found and
        'errorOnNotFound' is set to ``False``
        )doc"))

    .def("contains", &PyAhoCorasickDFA::contains, py::arg("txt"),
    py::doc(R"doc(
Determines if 'txt' contains a corresponding keyword from the `DFA`_

Parameters
----------
txt: :class:`str`
    The text to search

Returns
-------
:class:`bool`
    Whether text contains a corresponding keyword
        )doc"))

    .def("find", &PyAhoCorasickDFA::pyFind, py::arg("txt"),
    py::doc(R"doc(
Finds the first keyword within 'txt'

Parameters
----------
txt: :class:`str`
    The text to search for the keyword

Returns
-------
Tuple[Optional[:class:`str`], :class:`int`]
    Data of the found keyword containing:

    #. The keyword found
    #. The starting index of where the keyword was found. The index is only valid if the keyword is found.
        )doc"))

    .def("findAll", py::overload_cast<const std::string &>(&PyAhoCorasickDFA::findAll), py::arg("txt"),
    py::doc(R"doc(
Finds all occurences of the keywords from the `DFA`_ in the given text

Parameters
----------
txt: :class:`str`
    The text to search for keywords

Returns
-------
Dict[:class:`str`, List[Tuple[:class:`int`, :class:`int`]]]
    The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`

    * The keys are the keywords found
    * The values are all instances of the keyword found
    * The tuple contains the starting index of the found instance and the ending index of the found instance
        )doc"))
        
    .def("findFirstAll", py::overload_cast<const std::string &>(&PyAhoCorasickDFA::findFirstAll), py::arg("txt"),
    py::doc(R"doc(
Finds the first occurences of the keywords from the `DFA`_ in the given text

Parameters
----------
txt: :class:`str`
    The text to search for keywords

Returns
-------
Dict[:class:`str`, Tuple[:class:`int`, :class:`int`]]
    The indices for all the found keywords within the given text :raw-html:`<br />` :raw-html:`<br />`

    * The keys are the keywords found
    * The tuple contains the starting index of the found instance and the ending index of the first found instance
        )doc"))
        
    .def("findMaximal", &PyAhoCorasickDFA::pyFindMaximal, py::arg("txt"), py::arg("count") = 1, py::arg("pred") = py::none(),
    py::doc(R"doc(
Finds the first few largest keywords within 'txt'

.. note::
    This function is a greedy version of :meth:`find` or `Maximal Munch`_ that consumes only a limited amount of tokens

Parameters
----------
txt: :class:`str`
    The text to search for the keyword

count: :class:`int`
    The count of how many keywords to find in the search string :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``1``

pred: Optional[Callable[[:class:`str`], :class:`bool`]]
    If provided, only a keyword satisfying this predicate can be picked -- among the keywords
    ending at a given position, the largest one satisfying 'pred' is picked, not necessarily the
    largest one overall :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
Tuple[Union[Optional[:class:`str`], List[:class:`str`]], Union[:class:`int`, List[:class:`int`]]]
    Data of the found keyword: :raw-html:`<br />` :raw-html:`<br />`

    * If the 'count' argument is less than or equal to 1, then the data will contain:

        #. The keyword found
        #. The starting index of where the keyword was found.

    * If the 'count' argument is greater than 1, then the data will contain:

        #. The list of keywords found
        #. The corresponding starting indices for where the keyword were found
        )doc"))

    .def("get", &PyAhoCorasickDFA::pyGet, py::arg("txt"), py::arg("errorOnNotFound") = true, py::arg("default") = py::none(),
    py::doc(R"doc(
Retrieves the corresponding value from the first keyword fround in 'txt'

.. note::
    This function retrieves the corresponding value after running :meth:`find`

Parameters
----------
txt: :class:`str`
    The text to search for a keyword

errorOnNotFound: :class:`bool`  
    If no keywords are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

default: Any
    If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Raises
------
:class:`KeyError`
    If no keywords are found

Returns
-------
Tuple[Optional[:class:`str`], Union[T, Any]]
    Retrieves the following resultant data:

    #. The first keyword found
    #. Either the found value for the first keyword found or the value specified at 'default', if no keywords were found and
        'errorOnNotFound' is set to ``False``
        )doc"))

    .def("getAll", &PyAhoCorasickDFA::pyGetAll, py::arg("txt"),
    py::doc(R"doc(
Retrieves all the corresponding values to all the keywords found within 'txt'

Parameters
----------
txt: :class:`str`
    The text to search for keywords

Returns
-------
Dict[:class:`str`, T]
    The corresponding values to the keywords :raw-html:`<br />` :raw-html:`<br />`

    The keys are the keywords found and the values are the values to the keywords
        )doc"))
        
    .def("getMaximal", &PyAhoCorasickDFA::pyGetMaximal, py::arg("txt"), py::arg("errorOnNotFound") = true, py::arg("default") = py::none(), py::arg("count") = 1, py::arg("pred") = py::none(),
    py::doc(R"doc(
Retrieves the corresponding value from the first largest keyword fround in 'txt'

.. note::
    This function retrieves the corresponding value after running :meth:`findMaximal`

Parameters
----------
txt: :class:`str`
    The text to search for a keyword

errorOnNotFound: :class:`bool`
    If no keywords are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

default: Any
    If 'errorOnNotFound' is ``False``, then the default value to return if no keywords are found :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

count: :class:`int`
    The count of how many keywords to find in the search string :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``1``

pred: Optional[Callable[[:class:`str`], :class:`bool`]]
    If provided, only a keyword satisfying this predicate can be picked -- among the keywords
    ending at a given position, the largest one satisfying 'pred' is picked, not necessarily the
    largest one overall :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Raises
------
:class:`KeyError`
    If no keywords are found

Returns
-------
Tuple[Union[Optional[:class:`str`], List[:class:`str`]], Union[T, Any, List[T]]]
    Retrieves the following resultant data: :raw-html:`<br />` :raw-html:`<br />`

    * If the 'count' argument is less than or equal to 1, then the data contains:

        #. The first largest keyword found
        #. Either the found value for the first largest keyword found or the value specified at 'default', if no keywords were found and
           'errorOnNotFound' is set to ``False``

    * If the 'count' argument is greater than 1, then the data contains:

        #. The list of keywords found
        #. The corresponding found values to the keywords
        )doc"))
        
    .def("maximalStartsWith", &PyAhoCorasickDFA::pyMaximalStartsWith, py::arg("txt"),
    py::doc(R"doc(
Finds the largest keyword that is a prefix of the search text

Parameters
----------
txt: :class:`str`
    The text to search keywords

Returns 
-------
Optional[:class:`str`]
    The keyword that is found to be the prefix of the search text, if available
        )doc"))
        
    .def("getKeyVal", &PyAhoCorasickDFA::pyGetKeyVal, py::arg("txt"), py::arg("errorOnNotFound") = true, py::arg("default") = py::none(),
    py::doc(R"doc(
Retrieves the corresponding value of the key given in 'txt'

Parameters
----------
txt: :class:`str`
    The keyword to find the corresponding value

errorOnNotFound: :class:`bool`  
    If no keywords are found, whether to raise an exception

default: Any
    If 'errorsOnNotFound' is ``False``, then the default value to return if no keywords are found

Raises
------
:class:`KeyError`
    If the keyword is not found

Returns
-------
Union[T, Any]
    Either the found value for the corresponding keyword or the value specified at 'default', if no keywords were found and
    'errorOnNotFound' is set to ``False``
    )doc"));
}
