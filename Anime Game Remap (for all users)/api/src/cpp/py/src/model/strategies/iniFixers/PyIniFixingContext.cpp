#include "PyIniFixingContext.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIniFixingContext(py::module_ &m) {
    // No numpydoc "Attributes" section here even though this class is nothing but one attribute:
    // pairing that with the def_readwrite below is what makes Sphinx report a
    // "duplicate object description" for the member. The attribute documents itself instead.
    py::class_<AGRC::IniFixingContext> cls(m, "IniFixingContext", R"doc(
The per-call options handed to :meth:`BaseIniFixer.fix`

.. note::
    Not to be confused with the .ini file a fixer writes through, despite the near-identical name.
    This is a plain bag of options describing *one particular fix* and knows nothing about any file

Parameters
----------
isFirstModType: :class:`bool`
    Whether this fixer is running for the first :class:`ModType` of the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

isLastModType: :class:`bool`
    Whether this fixer is running for the last :class:`ModType` of the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc");

    // A real constructor argument rather than a settable-afterwards attribute alone, so the common
    // spelling is a one-liner at the call site. Note there is deliberately no default for the class
    // itself anywhere: a mutable object handed to py::arg("...") = <literal> is pybind's version of
    // Python's mutable-default-argument bug, so every caller of this defaults to None instead and
    // constructs a fresh one (see bindBaseIniFixerCommonMethods).
    cls.def(py::init<bool, bool>(), py::arg("isFirstModType") = true, py::arg("isLastModType") = true)

       .def_readwrite("isFirstModType", &AGRC::IniFixingContext::isFirstModType, py::doc(R"doc(
:class:`bool`: Whether this fixer is running for the first :class:`ModType` of the .ini file :raw-html:`<br />` :raw-html:`<br />`

The mirror image of :attr:`IniFixingContext.isLastModType`, and there for the same reason: several
fixers chain over one .ini file, so anything that touches the file itself rather than only the fix
has to happen exactly once :raw-html:`<br />` :raw-html:`<br />`

:meth:`GIMIFixer.fix` uses it to gate ``keepBackup`` -- disabling the existing .ini file as a backup
is the whole file's business, and a later mod type doing it again would be backing up a file the
first pass already moved aside. The condition it gates is otherwise unchanged: ``keepBackup`` still
also needs ``fixOnly`` and an .ini file that already exists on disk :raw-html:`<br />` :raw-html:`<br />`

**Default**: ``True``, so a fixer driven directly -- the only one, hence both the first and the last
-- backs up as it always did
       )doc"))

       .def_readwrite("isLastModType", &AGRC::IniFixingContext::isLastModType, py::doc(R"doc(
:class:`bool`: Whether this fixer is running for the last :class:`ModType` of the .ini file :raw-html:`<br />` :raw-html:`<br />`

One .ini file can be fixed by several fixers in turn -- one per mod type it was classified as, and
one per target mod each of those fixes to. They chain over the same file, so anything that rewrites
the file's own text rather than only adding to the fix has to happen exactly once, at the end :raw-html:`<br />` :raw-html:`<br />`

:meth:`GIMIFixer.fix` uses it for its ``hideOrig`` pass for that reason: commenting out the original
mod's `sections`_ is the whole file's business, not one mod type's. With this ``False``, ``hideOrig``
is ignored :raw-html:`<br />` :raw-html:`<br />`

**Default**: ``True``, so a fixer driven directly -- one fixer, one call, no chain -- behaves as
though it were the only one, which it is. It is the *chaining* caller that has to say otherwise, and
:class:`IniFile` is the one that does
       )doc"));
}
