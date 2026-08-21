#include "PySympyParser.h"

#include <memory>
#include <string>

#include "../../tools/parsing/PyBaseSLR1Parser.h"
#include "AGRemapCore/model/iftemplate/SympyParser.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    std::unique_ptr<AGRC::SympyParser> makeSympyParser(std::string startToken, std::string endToken, std::string nullToken, bool setup) {
        return std::make_unique<AGRC::SympyParser>(std::move(startToken), std::move(endToken), std::move(nullToken), setup);
    }

}


void initCppSympyParser(pybind11::module_ &m) {
    // Not registered as a real Python subclass of BaseSLR1Parser -- AGRC::SympyParser's own C++
    // base is BaseSLR1Parser<std::string>, a different, unrelated template instantiation from
    // PyBaseSLR1Parser's BaseSLR1Parser<py::object, ...> (nothing in this codebase does
    // `isinstance(x, BaseSLR1Parser)`, so this costs nothing real). Shares its full method
    // surface via bindBaseSLR1ParserCommonMethods instead -- see that function's own doc comment.
    auto cls = py::class_<AGRC::SympyParser>(m, "SympyParser", R"doc(
The context-free parser used for a subset of the string representation of a `sympy logic query`_

eg.

.. code-block:: ini
    :linenos:

    ~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))

Parameters
-----------
startToken: :class:`str`
    The name of the starting token for an input string

    **Default**: ``STARTTOKEN``

endToken: :class:`str`
    The name of the ending token for an input string

    **Default**: ``ENDTOKEN``

nullToken: :class:`str`
    The name for the empty token

    **Default**: ``EPSILON``

setup: :class:`bool`
    Whether to initialize all the setup for the parser automatically by calling :meth:`setup`

    **Default**: ``True``
    )doc")

        .def(py::init(&makeSympyParser), py::arg("startToken") = "STARTTOKEN", py::arg("endToken") = "ENDTOKEN",
             py::arg("nullToken") = "EPSILON", py::arg("setup") = true);

    bindBaseSLR1ParserCommonMethods<AGRC::SympyParser>(cls);
}
