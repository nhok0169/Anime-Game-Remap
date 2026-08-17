#include "PySympyTokenizer.h"

#include <pybind11/stl.h>

#include "AGRemapCore/tools/parsing/SympyTokenizer.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppSympyTokenizer(pybind11::module_ &m) {
    // Real Python subclass of FilteredTokenizer -- see PyIfPredTokenizer.cpp's comment, the same
    // reasoning applies here (base already registered, methods inherited for free, no trampoline,
    // registered under the bare name since the deprecated pure-Python class is 'SympyTokenizerOld',
    // and GlobalCompilerParts/SympyIfPredGenerator still construct that one deliberately, for now).
    py::class_<AGRC::SympyTokenizer, AGRC::FilteredTokenizer>(m, "SympyTokenizer", R"doc(
This class inherits from :class:`FilteredTokenizer`

The tokenizer used for a subset of the string representation of a `sympy logic query`_

eg.

.. code-block::
    :linenos:

    ~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))

Parameters
----------
setup: :class:`bool`
    Whether to initialize all the setup for the tokenizer automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc")

        .def(py::init<bool>(), py::arg("setup") = true);
}
