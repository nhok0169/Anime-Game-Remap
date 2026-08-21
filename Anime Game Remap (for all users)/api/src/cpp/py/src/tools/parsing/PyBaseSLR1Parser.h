#ifndef AGRemapPyBind_PyBaseSLR1Parser_H
#define AGRemapPyBind_PyBaseSLR1Parser_H

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "PyParseTree.h"
#include "../PyTools.h"
#include "AGRemapCore/tools/parsing/BaseSLR1Parser.h"
#include "AGRemapCore/tools/parsing/SyntaxErr.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief The `pybind11`_-facing name for `AGRC::BaseSLR1Parser`\\<py::object, PyObjectHash, PyObjectEqual\\>
 */
using PyBaseSLR1Parser = AGRC::BaseSLR1Parser<py::object, PyObjectHash, PyObjectEqual>;


/**
 * @brief
 @rst
 Raises the real ``FixRaidenBoss2.exceptions.SyntaxErr.SyntaxErr`` for a caught
 :cpp:class:`AGRC::SyntaxErr` -- same pattern as ``PyBaseTokenizer.cpp``'s own
 ``raisePySyntaxErr`` (see that file for the full rationale on why this is looked up lazily by
 dotted path off the *calling* binding module's own ``__name__``, rather than a hardcoded
 import or a shared core exception type). Call :cpp:func:`initSyntaxErrModulePath` once, from
 this binding's own ``initCppXxx``, before this can be used
 @endrst
 */
[[noreturn]] void raisePySyntaxErr(const AGRC::SyntaxErr &e);

/**
 * @brief Derives and caches "\<core's own parent package\>.exceptions.SyntaxErr" from the bound
 *      module's own ``__name__`` -- see #raisePySyntaxErr
 */
void initSyntaxErrModulePath(pybind11::module_ &m);

void initCppBaseSLR1Parser(pybind11::module_ &m);


namespace detail_PyBaseSLR1Parser {
    // py::cast(id) alone is ambiguous when Id is already py::object (pybind can't deduce a
    // target type from a py::object argument to the "C++ value -> py::object" overload of
    // py::cast) -- this template is used for BOTH Id = py::object (PyBaseSLR1Parser) and
    // Id = std::string (SympyParser/IfPredParser), so it needs to handle both without relying
    // on py::cast doing the right thing for an already-py::object input.
    template <typename Id>
    py::object idToPyObject(const Id &id) {
        if constexpr (std::is_same_v<Id, py::object>) {
            return id;
        } else {
            return py::cast(id);
        }
    }
}


/**
 * @brief
 @rst
 Binds the method surface every :cpp:class:`AGRC::BaseSLR1Parser` instantiation shares
 (#Productions/#Id-agnostic beyond what template argument deduction already handles) onto an
 already-created ``py::class_<T, ...>`` -- used both for :cpp:type:`PyBaseSLR1Parser` itself and
 for the fixed-grammar `std::string`-#Id subclasses (:cpp:class:`AGRC::SympyParser`,
 :cpp:class:`AGRC::IfPredParser`), so this method surface (and its docstrings) is written once
 @endrst
 *
 * @tparam T A type publicly inheriting some `AGRC::BaseSLR1Parser` instantiation
 * @tparam PyClass The ``py::class_<T, ...>`` type being bound
 */
template <typename T, typename PyClass>
void bindBaseSLR1ParserCommonMethods(PyClass &cls) {
    cls.def_property_readonly("productions", [](T &self) {
            py::dict result;
            for (const auto &[prodId, production] : self.productions()) {
                result[detail_PyBaseSLR1Parser::idToPyObject(prodId)] = py::make_tuple(production.first, production.second);
            }
            return result;
        }, py::doc(R"doc(
Dict[Hashable, Tuple[:class:`str`, List[:class:`str`]]]: The production rules of the `CFG`_, keyed by the id of each production rule
        )doc"))

        .def_property_readonly("nonTermSymbols", &T::nonTermSymbols, py::doc(R"doc(
Set[:class:`str`]: The set of non-terminal symbols of the `CFG`_, as of the last time 'productions' was set
        )doc"))

        .def_property("startSymbol", &T::startSymbol, [](T &self, std::string newStartSymbol) {
            try {
                self.setStartSymbol(std::move(newStartSymbol));
            } catch (const std::invalid_argument &e) {
                throw py::key_error(e.what());
            }
        }, py::doc(R"doc(
:class:`str`: The starting non-terminal symbol

:getter: Retrieves the starting non-terminal symbol
:setter: Sets the new starting non-terminal symbol
        )doc"))

        .def_readwrite("startToken", &T::startToken, py::doc(R"doc(
:class:`str`: The name of the starting token for an input string
        )doc"))

        .def_readwrite("endToken", &T::endToken, py::doc(R"doc(
:class:`str`: The name of the ending token for an input string
        )doc"))

        .def_readwrite("nullToken", &T::nullToken, py::doc(R"doc(
:class:`str`: The name for the empty token
        )doc"))

        .def_readwrite("nullable", &T::nullable, py::doc(R"doc(
Dict[:class:`str`, :class:`bool`]: The `Nullable Set`_

The keys are the non-terminal symbols and the values are whether each symbol is nullable
        )doc"))

        .def_readwrite("first", &T::first, py::doc(R"doc(
Dict[:class:`str`, Set[:class:`str`]]: The `First Set`_ for only each single non-terminal symbol
        )doc"))

        .def_readwrite("follow", &T::follow, py::doc(R"doc(
Dict[:class:`str`, Set[:class:`str`]]: The `Follow Set`_
        )doc"))

        .def("clear", &T::clear, py::doc(R"doc(
Clears all the setup from the parser
        )doc"))

        .def("getNonTermSymbols", &T::getNonTermSymbols, py::doc(R"doc(
Retrieves the set of non-terminal symbols of the `CFG`_

Returns
-------
Set[:class:`str`]
    The set of non-terminal symbols
        )doc"))

        .def("getNullableSet", &T::getNullableSet, py::doc(R"doc(
Computes the `Nullable Set`_

Returns
-------
Dict[:class:`str`, :class:`bool`]
    Whether each non-terminal symbol is nullable
        )doc"))

        .def("getFirstSet", &T::getFirstSet, py::arg("updateNullable") = true, py::doc(R"doc(
Computes the `First Set`_ for only each single non-terminal symbol

Parameters
----------
updateNullable: :class:`bool`
    Whether to update the `Nullable Set`_

    **Default**: ``True``

Returns
-------
Dict[:class:`str`, Set[:class:`str`]]
    The first terminal symbols to appear for a non-terminal symbol
        )doc"))

        .def("getFirst", &T::getFirst, py::arg("symbols"), py::arg("nullable"), py::arg("first"), py::doc(R"doc(
Retrieves the first terminal symbols to appear given a list of symbols

Parameters
----------
symbols: List[:class:`str`]
    The symbols to read

nullable: Dict[:class:`str`, :class:`bool`]
    The `Nullable Set`_

first: Dict[:class:`str`, Set[:class:`str`]]
    The `First Set`_ for only each single non-terminal symbol

Returns
-------
Set[:class:`str`]
    The first terminal symbols to appear given 'symbols'
        )doc"))

        .def("getFollowSet", &T::getFollowSet, py::arg("updateNullable") = true, py::arg("updateFirst") = true, py::doc(R"doc(
Computes the `Follow Set`_

Parameters
----------
updateNullable: :class:`bool`
    Whether to update the `Nullable Set`_

    **Default**: ``True``

updateFirst: :class:`bool`
    Whether to update the `First Set`_

    **Default**: ``True``

Returns
-------
Dict[:class:`str`, Set[:class:`str`]]
    The `Follow Set`_
        )doc"))

        .def("setup", &T::setup, py::doc(R"doc(
Initializes any necessary setup for the parser
        )doc"))

        .def("parse", [](T &self, std::vector<AGRC::Token> tokens, const AGRC::ParseContext *ctx) -> PyParseTree {
            try {
                if constexpr (std::is_same_v<typename T::Tree, PyParseTree>) {
                    return self.parse(std::move(tokens), ctx);
                } else {
                    return convertToPyParseTree(self.parse(std::move(tokens), ctx));
                }
            } catch (const AGRC::SyntaxErr &e) {
                raisePySyntaxErr(e);
            }
        }, py::arg("tokens"), py::arg("ctx") = py::none(), py::doc(R"doc(
Parses an input text

Parameters
----------
tokens: List[:class:`Token`]
    The tokenized tokens of the input text :raw-html:`<br />` :raw-html:`<br />`

    Usually obtained by running some sort of tokenizer, such as :class:`BaseTokenizer`

ctx: Optional[:class:`ParseContext`]
    The context for parsing :raw-html:`<br />` :raw-html:`<br />`

    If this argument is ``None``, a context is constructed from the concatenation of every
    token's value

    **Default**: ``None``

Raises
------
:class:`SyntaxErr`
    If the parse tree cannot be constructed

Returns
-------
:class:`ParseTree`
    The constructed parse tree
        )doc"));
}

#endif
