#include "PyBaseIniRemover.h"

#include <utility>

namespace py = pybind11;


PyBaseIniRemover::PyBaseIniRemover(py::object iniFile):
    PyBaseIniRemoverCore(nullptr), iniFileObj(std::move(iniFile)) {}


void initCppBaseIniRemover(py::module_ &m) {
    py::class_<PyBaseIniRemoverCore, py::smart_holder>(m, "CppBaseIniRemover", R"doc(
The shared C++ base of every remover, exposed so that one built on the C++ side -- by a
:class:`IniRemoveBuilder`'s default factory, or by anything in ``AGRemapCore`` -- can still cross into
`Python`_ :raw-html:`<br />` :raw-html:`<br />`

Not usually what you want: a remover created **from** `Python`_ is a :class:`BaseIniRemover`, which
inherits from this and carries the extra `Python`_ state. This class exists so the boundary never
has to hand back ``None`` for a core-side object it has no richer type for
    )doc");


    py::class_<PyBaseIniRemover, PyBaseIniRemoverCore, py::smart_holder> cls(m, "BaseIniRemover", R"doc(
Base class to remove fixes from a .ini file

Parameters
----------
iniFile: :class:`IniFile`
    The .ini file to remove the fix from
    )doc");

    cls.def(py::init<py::object>(), py::arg("iniFile") = py::none());

    bindBaseIniRemoverCommonMethods<PyBaseIniRemover>(cls, R"doc(
Removes the fix from the .ini file

Parameters
----------
parse: :class:`bool`
    Whether to also parse for the .*RemapBlend.buf files that need to be removed :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

writeBack: :class:`bool`
    Whether to write back the new text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

context: :class:`IniRemovalContext`
    The per-call options for this removal :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, which means a default-constructed one

Returns
-------
:class:`str`
    The new content of the .ini file
    )doc");
}
