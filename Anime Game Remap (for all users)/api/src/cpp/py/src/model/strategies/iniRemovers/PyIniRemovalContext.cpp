#include "PyIniRemovalContext.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIniRemovalContext(py::module_ &m) {
    // No numpydoc "Attributes" section here even though this class is nothing but one attribute:
    // pairing that with the def_readwrite below is what makes Sphinx report a
    // "duplicate object description" for the member. The attribute documents itself instead.
    py::class_<AGRC::IniRemovalContext> cls(m, "IniRemovalContext", R"doc(
The per-call options handed to :meth:`BaseIniRemover.remove`

.. note::
    Not to be confused with the .ini file a remover reads through, despite the near-identical name.
    This is a plain bag of options describing *one particular removal* and knows nothing about any
    file

Parameters
----------
ignoreModType: :class:`bool`
    Whether to remove the fix without asking which :class:`ModType` it belongs to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc");

    // A real constructor argument rather than a settable-afterwards attribute alone, so the common
    // spelling is a one-liner at the call site. Note there is deliberately no default for the class
    // itself anywhere: a mutable object handed to py::arg("...") = <literal> is pybind's version of
    // Python's mutable-default-argument bug, so every caller of this defaults to None instead and
    // constructs a fresh one (see bindBaseIniRemoverCommonMethods).
    cls.def(py::init<bool>(), py::arg("ignoreModType") = false)

       .def_readwrite("ignoreModType", &AGRC::IniRemovalContext::ignoreModType, py::doc(R"doc(
:class:`bool`: Whether to remove the fix without asking which :class:`ModType` it belongs to :raw-html:`<br />` :raw-html:`<br />`

:meth:`RemapIniRemover.remove` normally treats a candidate `section`_ as this software's own output only
when it carries this software's marker -- either it sits inside the fix boilerplate, or some
reachable colouring state gives its ``hash`` `KVP`_ a value belonging to one of the .ini file's mod
types. That second half is what recognizes the ``Remap``-named leftovers *outside* the boilerplate :raw-html:`<br />` :raw-html:`<br />`

With this set, the hash half is skipped and **every** candidate is taken -- every `section`_ inside
the boilerplate plus every ``Remap``-named `section`_ outside it, whoever they belong to. That is
what the pure-Python ``RemapIniRemover`` this replaced always did :raw-html:`<br />` :raw-html:`<br />`

:class:`IniFile` asks for it on its **last** mod type, so that every earlier pass takes only what it
can prove is its own and the final pass clears whatever is still standing. Without it, a leftover
carrying no usable ``hash`` would survive every pass -- the exact debris an interrupted or partly
undone fix leaves behind
       )doc"));
}
