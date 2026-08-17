#include "PyBaseTokenizer.h"

#include <string>
#include <unordered_map>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/tools/parsing/BaseTokenizer.h"
#include "AGRemapCore/tools/parsing/SyntaxErr.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    // A single Python character (len == 1) -> the raw ASCII byte addASCIIRangeTransitions
    // expects. Mirrors the Python original's own implicit assumption (ord(startChar)/
    // ord(endChar)) that these are always single-character strings -- not something a bare
    // 'char' parameter could accept directly from an arbitrary Python str argument.
    char parseASCIIChar(const std::string &name, const std::string &value) {
        if (value.size() != 1) {
            throw py::value_error(name + " must be a single character, got '" + value + "'");
        }
        return value[0];
    }

    // The fully-qualified dotted path to FixRaidenBoss2.exceptions.SyntaxErr, derived (once, at
    // module init -- see initCppBaseTokenizer) from this 'core' module's own '__name__' rather
    // than hardcoded. A hardcoded "FixRaidenBoss2...." path breaks under this repo's own Unit
    // Tester harness, which imports the whole package as 'src.py.FixRaidenBoss2' instead of a
    // top-level 'FixRaidenBoss2' -- confirmed via ModuleNotFoundError there before this fix.
    // Deriving it from 'core's own '__name__' (e.g. "FixRaidenBoss2.core" in a normal install,
    // "src.py.FixRaidenBoss2.core" here) works under either import scheme.
    std::string &syntaxErrModulePath() {
        static std::string path;
        return path;
    }

    // Raises the real FixRaidenBoss2.exceptions.SyntaxErr.SyntaxErr (not a generic RuntimeError,
    // and not some bespoke core.SyntaxErr type) so callers keep working unchanged once they're
    // migrated onto this class -- 'except SyntaxErr' (e.g. IfPredPart.py's getLogicQuery)
    // depends on this being the exact same class, not just something with a similar name.
    //
    // Imported lazily (only when an error actually fires), not at module-load time: 'core' is
    // loaded from inside FixRaidenBoss2/__init__.py's own 'from .core import ...', so importing
    // back up into FixRaidenBoss2 at module scope here would be a real circular import.
    // Deferring it to first use is safe -- FixRaidenBoss2 has always already finished importing
    // by the time this runs, since nothing can call a bound method before the module exposing
    // it has finished loading. getErrLocation()/__str__ formatting deliberately stays in that
    // one pure-Python class rather than being duplicated in AGRemapCore::SyntaxErr.
    [[noreturn]] void raisePySyntaxErr(const AGRC::SyntaxErr &e) {
        py::object syntaxErrClass = py::module_::import(syntaxErrModulePath().c_str()).attr("SyntaxErr");
        py::object excInstance = syntaxErrClass(py::cast(e.ctx()), py::cast(e.token()), py::cast(e.process()));
        PyErr_SetObject(syntaxErrClass.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    std::vector<AGRC::Token> callSimplifiedMaximalMunch(AGRC::BaseTokenizer &self, const AGRC::ParseContext &ctx, bool includeFiltered) {
        try {
            return self.simplifiedMaximalMunch(ctx, includeFiltered);
        } catch (const AGRC::SyntaxErr &e) {
            raisePySyntaxErr(e);
        }
    }

}


void initCppBaseTokenizer(pybind11::module_ &m) {
    // Registered under the bare 'BaseTokenizer' name (no 'Cpp' prefix) -- the deprecated
    // bare-named pure-Python class is now 'BaseTokenizerOld', so nothing shadows this
    // registration; see Documentation/CLAUDE.md's naming-pitfall section.
    //
    // No trampoline: FilteredTokenizer is a real C++ subclass (see PyFilteredTokenizer.cpp), but
    // nothing subclasses this from *Python* yet (IfPredTokenizer/SympyTokenizer still subclass
    // the pure-Python FilteredTokenizerOld). Note for whoever ports those next: BaseTokenizer's
    // constructor calls this->setup() internally in C++, and a C++ virtual call made from within
    // a base class's own constructor can never reach a more-derived override (Python or C++) --
    // that's true regardless of a trampoline, since the derived object isn't fully constructed
    // yet. FilteredTokenizer's own constructor works around this by always passing setup=false
    // up to BaseTokenizer's constructor, then calling this->setup() itself afterwards (see its
    // constructor's own comment) -- a future Python-subclassable version would need the same
    // "call setup() after construction, not from inside the base's own constructor" treatment.
    // Derive "<core's own parent package>.exceptions.SyntaxErr" from 'm's actual '__name__'
    // (see syntaxErrModulePath's comment for why this can't just be hardcoded).
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    syntaxErrModulePath() = parentPackage.empty() ? "exceptions.SyntaxErr" : (parentPackage + ".exceptions.SyntaxErr");

    py::class_<AGRC::BaseTokenizer>(m, "BaseTokenizer", R"doc(
The base class used for tokenizing text

Parameters
----------
tokens: Dict[:class:`str`, :class:`str`]
    The tokens used for tokenization :raw-html:`<br />` :raw-html:`<br />`

    The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens

setup: :class:`bool`
    Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc")

        .def(py::init<std::unordered_map<std::string, std::string>, bool>(),
    py::arg("tokens"), py::arg("setup") = true)

        .def("clear", &AGRC::BaseTokenizer::clear, py::doc(R"doc(
Clears the `DFA`_ of the tokenizer
        )doc"))

        .def("reset", &AGRC::BaseTokenizer::reset, py::doc(R"doc(
Resets the state of the `DFA`_ for the tokenizer
        )doc"))

        .def("addStartState", &AGRC::BaseTokenizer::addStartState, py::doc(R"doc(
Adds the start state representing an empty string

Returns
-------
:class:`str`
    The id of the start state
        )doc"))

        .def("addKeyword", &AGRC::BaseTokenizer::addKeyword, py::arg("keyword"), py::doc(R"doc(
Adds a keyword into the `DFA`_ of the tokenizer

Parameters
----------
keyword: :class:`str`
    The keyword to add

Returns
-------
:class:`str`
    The id of the accepting node in the `DFA`_
        )doc"))

        .def("addASCIIRangeTransitions", [](AGRC::BaseTokenizer &self, const std::string &srcId, const std::string &startChar, const std::string &endChar, const std::string &destId) {
            self.addASCIIRangeTransitions(srcId, parseASCIIChar("startChar", startChar), parseASCIIChar("endChar", endChar), destId);
        }, py::arg("srcId"), py::arg("startChar"), py::arg("endChar"), py::arg("destId"), py::doc(R"doc(
Adds a group of transitions from one state to another according to a range of `ASCII`_ characters

Parameters
----------
srcId: :class:`str`
    The id of the source state for the transition

startChar: :class:`str`
    The starting character within the ASCII range to add a transition for

endChar: :class:`str`
    The ending character within the ASCII range to add a transition for

destId: :class:`str`
    The id of the destination state for the transition
        )doc"))

        .def("setup", &AGRC::BaseTokenizer::setup, py::doc(R"doc(
Performs any necessary setup to the tokenizer
        )doc"))

        .def("simplifiedMaximalMunch", [](AGRC::BaseTokenizer &self, const AGRC::ParseContext &src, bool includeFiltered) {
            return callSimplifiedMaximalMunch(self, src, includeFiltered);
        }, py::arg("src"), py::arg("includeFiltered") = false, py::doc(R"doc(
Tokenizes the source text into tokens using the `Simplified Maximal Munch`_ algorithm

Parameters
----------
src: Union[:class:`str`, :class:`ParseContext`]
    The source text to be tokenized

includeFiltered: :class:`bool`
    Ignored by this base class -- see :class:`FilteredTokenizer` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Raises
------
:class:`SyntaxErr`
    The provided source text cannot be correctly tokenized

Returns
-------
List[:class:`Token`]
    The list of tokens to the source text
        )doc"))

        .def("simplifiedMaximalMunch", [](AGRC::BaseTokenizer &self, const std::string &src, bool includeFiltered) {
            return callSimplifiedMaximalMunch(self, AGRC::ParseContext(src), includeFiltered);
        }, py::arg("src"), py::arg("includeFiltered") = false)

        .def_property_readonly("tokens", &AGRC::BaseTokenizer::tokens, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: The tokens used for tokenization

The keys are the ids to the accepting states of the `DFA`_ and the values are the tokens
        )doc"))

        .def_property_readonly("startStateId", &AGRC::BaseTokenizer::startStateId, py::doc(R"doc(
:class:`str`: The id of the starting state of the `DFA`_
        )doc"));
}
