#include "PyIfPredParser.h"

#include <memory>
#include <string>

#include "../../tools/parsing/PyBaseSLR1Parser.h"
#include "AGRemapCore/model/iftemplate/IfPredParser.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    std::unique_ptr<AGRC::IfPredParser> makeIfPredParser(std::string startToken, std::string endToken, std::string nullToken, bool setup) {
        return std::make_unique<AGRC::IfPredParser>(std::move(startToken), std::move(endToken), std::move(nullToken), setup);
    }

}


void initCppIfPredParser(pybind11::module_ &m) {
    // See PySympyParser.cpp's own comment on why this isn't registered as a real Python subclass
    // of BaseSLR1Parser.
    auto cls = py::class_<AGRC::IfPredParser>(m, "IfPredParser", R"doc(
The context-free parser used for conditional predicates within a .ini file

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

        .def(py::init(&makeIfPredParser), py::arg("startToken") = "STARTTOKEN", py::arg("endToken") = "ENDTOKEN",
             py::arg("nullToken") = "EPSILON", py::arg("setup") = true);

    bindBaseSLR1ParserCommonMethods<AGRC::IfPredParser>(cls);
}
