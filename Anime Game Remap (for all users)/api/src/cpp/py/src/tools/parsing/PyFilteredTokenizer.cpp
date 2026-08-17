#include "PyFilteredTokenizer.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

#include <pybind11/stl.h>

#include "AGRemapCore/tools/parsing/FilteredTokenizer.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppFilteredTokenizer(pybind11::module_ &m) {
    // Registered as a *real* Python subclass of BaseTokenizer (the base must already be
    // registered -- initCppBaseTokenizer runs before this one in bindings.cpp) so isinstance()
    // and attribute inheritance work, not just a doc comment claiming it: every BaseTokenizer
    // method (clear, reset, addKeyword, simplifiedMaximalMunch, tokens, startStateId, ...) is
    // inherited for free through normal Python MRO -- nothing here needs rebinding them. See
    // PyDFA.cpp's BaseDFA/DFA pair for the same pattern.
    //
    // No trampoline: nothing subclasses this from Python yet (IfPredTokenizer/SympyTokenizer
    // still subclass the pure-Python FilteredTokenizerOld).
    //
    // Registered under the bare 'FilteredTokenizer' name (no 'Cpp' prefix) -- the deprecated
    // bare-named pure-Python class is now 'FilteredTokenizerOld', so nothing shadows this
    // registration; see Documentation/CLAUDE.md's naming-pitfall section.
    py::class_<AGRC::FilteredTokenizer, AGRC::BaseTokenizer>(m, "FilteredTokenizer", R"doc(
This class inherits from :class:`BaseTokenizer`

A tokenizer that still accepts all tokens, but does not include certain tokens into the tokenized result

Parameters
----------
tokens: Dict[:class:`str`, :class:`str`]
    The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

    The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens

keywordTokenIds: Set[:class:`str`]
    The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names

filteredTokenIds: Set[:class:`str`]
    The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result

setup: :class:`bool`
    Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc")

        .def(py::init<std::unordered_map<std::string, std::string>, std::unordered_set<std::string>, std::unordered_set<std::string>, bool>(),
    py::arg("tokens"), py::arg("keywordTokenIds"), py::arg("filteredTokenIds"), py::arg("setup") = true)

        .def_property_readonly("keywordTokenIds", &AGRC::FilteredTokenizer::keywordTokenIds, py::doc(R"doc(
Set[:class:`str`]: The ids of the accepting states in the `DFA`_ such that their corresponding tokens are simply keyword names
        )doc"))

        .def_property_readonly("filteredTokenIds", &AGRC::FilteredTokenizer::filteredTokenIds, py::doc(R"doc(
Set[:class:`str`]: The ids of the accepting states in the `DFA`_ to not include their corresponding tokens into the tokenized result
        )doc"));
}
