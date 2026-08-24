#include "PyTexEditor.h"

#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTexEditor(pybind11::module_ &m) {
    py::class_<AGRC::TexEditor, AGRC::BaseTexEditor, py::smart_holder>(m, "CppTexEditor", R"doc(
This class inherits from :class:`CppBaseTexEditor`

The pure-C++-SDK-facing engine behind :class:`TexEditor` -- runs a fixed sequence of
:class:`CppBaseTexFilter`\s over a texture file. :meth:`~CppBaseTexEditor.fix` is a no-op unless a
filter list was passed to the constructor.

.. note::
    The Python-facing :class:`TexEditor` overrides :meth:`~CppBaseTexEditor.fix` itself instead of
    using this class's filter list, so that its own ``filters`` attribute can hold arbitrary Python
    callables (not just objects this constructor can accept) -- see that class for the
    Python-visible behavior
    )doc")

        .def(py::init<>());
}
