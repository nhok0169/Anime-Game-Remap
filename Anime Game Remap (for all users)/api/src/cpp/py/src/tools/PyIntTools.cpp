#include "PyIntTools.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;


static void throwInvalidBaseError(bool error) {
    if (error) {
        throw py::type_error("Base must be greater than 1");
    }
}

std::tuple<std::vector<unsigned int>, bool> PyIntTools::pyInttoBase(long long num, unsigned int base) {
    bool isNegative;
    bool error;

    std::vector<unsigned int> digits = toBase(num, base, &isNegative, &error);
    throwInvalidBaseError(error);
    return std::make_tuple(digits, isNegative);
}

std::string PyIntTools::pyInttoStrBase(long long num, unsigned int base, const std::vector<std::string>& getDigit, const std::string& negativeChar) {
    bool error;
    std::string result = toStrBase(num, base, getDigit, negativeChar, &error);
    throwInvalidBaseError(error);
    return result;
}

std::string PyIntTools::pyInttoBase64(long long num, const std::optional<std::vector<std::string>>& getDigit, const std::string& negativeChar) {
    bool error;
    std::string result = toBase64(num, &error, getDigit, negativeChar);
    throwInvalidBaseError(error);
    return result;
}


void initCppIntTools(pybind11::module_ &m) {
        py::class_<PyIntTools>(m, "CppIntTools", 
        "C++ Tools for handling integers")

        .def_static("toBase", &PyIntTools::pyInttoBase,
                    py::arg("num"), py::arg("base"),
                    py::doc(R"doc(
                        Converts a base 10 number to an arbitrary base number

                        Parameters
                        ----------
                        num: :class:`int`
                            The base 10 number to convert

                        base: :class:`int`
                            The base to convert to

                        Raises
                        ------
                        :class:`TypeError`
                            The base is smaller or equal to 1

                        Returns
                        -------
                        Tuple[List[:class:`int`], :class:`bool`]
                            Retrieves the following data in the tuple:

                            #. The digits in the converted number
                            #. Whether the number is negative
                    )doc"))

        .def_static("toStrBase", &PyIntTools::pyInttoStrBase,
                    py::arg("num"), py::arg("base"), py::arg("getDigit"), py::arg("negativeChar"),
                    py::doc(R"doc(
Converts a base 10 number to an arbitrary base number, such that the characters in this arbitrary based number
are all characters

Parameters
----------
num: :class:`int`
    The base 10 number to convert

base: :class:`int`
    The base to convert to

getDigit: List[:class:`str`]
    The string representations of each digit. Each element is the string representation
    of the digit at the particular index of the list.

negativeChar: :class:`str`
    The character representation for the negative symbol

Returns
-------
:class:`str`
    The converted string representation of the arbitrary base number
                    )doc"))

        .def_static("toBase64", &PyIntTools::pyInttoBase64,
                    py::arg("num"), py::arg("getDigit") = py::none(), py::arg("negativeChar") = NEGATIVE_STR,
                    py::doc(R"doc(
Converts a base 10 number to a base 64 number

Parameters
----------
num: :class:`int`
    The base 10 number to convert

getDigit: List[:class:`str`]
    how to get the string representation of a digit. :raw-html:`<br />` :raw-html:`<br />`

    * If this argument is a list, each element is the string representation of the digit at the particular index of the string/list.
    * If this argument is ``None``, then will use the following string for each digit:

    ``ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_``

    This is the same digit representation as the `standard base 64`_ except that the 63rd digit (``/``) is replaced with the ``_`` character :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

negativeChar: :class:`str`
    The character representation for the negative symbol :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``"-"``

Returns
-------
:class:`str`
    The converted string representation of the arbitrary base 64 number
                    )doc"));
}