#include "PyIfPredTokenizer.h"

#include <pybind11/stl.h>

#include "AGRemapCore/tools/parsing/IfPredTokenizer.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIfPredTokenizer(pybind11::module_ &m) {
    // Real Python subclass of FilteredTokenizer (base already registered by
    // initCppFilteredTokenizer, called before this in bindings.cpp) -- inherits every
    // FilteredTokenizer/BaseTokenizer method (simplifiedMaximalMunch, addKeyword, tokens, ...)
    // for free through normal Python MRO, nothing to rebind here.
    //
    // Registered under the bare 'IfPredTokenizer' name (no 'Cpp' prefix) -- the deprecated
    // bare-named pure-Python class is now 'IfPredTokenizerOld', so nothing shadows this
    // registration; see Documentation/CLAUDE.md's naming-pitfall section.
    //
    // GlobalCompilerParts.IfPredTokenizer / IfPredPart.py's getLogicQuery/getIfPredStr still
    // construct the pure-Python IfPredTokenizerOld, not this class -- left that way
    // deliberately, matching this port's own step-by-step precedent (verify a new class
    // standalone before migrating call sites). Wiring this in is a natural next step.
    py::class_<AGRC::IfPredTokenizer, AGRC::FilteredTokenizer>(m, "IfPredTokenizer", R"doc(
This class inherits from :class:`FilteredTokenizer`

The tokenizer used for conditional predicates within a .ini file

eg.

.. code-block:: ini
    :linenos:
    :emphasize-lines: 1,3

    if pred1
        ...
    else if pred2
        ...
    endif

Parameters
----------
setup: :class:`bool`
    Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc")

        .def(py::init<bool>(), py::arg("setup") = true);
}
