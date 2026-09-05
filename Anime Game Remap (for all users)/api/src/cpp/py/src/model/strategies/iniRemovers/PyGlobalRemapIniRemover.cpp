#include "PyGlobalRemapIniRemover.h"

#include <utility>


PyGlobalRemapIniRemover::PyGlobalRemapIniRemover(py::object iniFile): PyRemapIniRemover(std::move(iniFile)) {}


std::string PyGlobalRemapIniRemover::remove(bool parse, bool writeBack, AGRC::IniRemovalContext context) {
    // The one thing this class is -- see AGRemapCore::GlobalRemapIniRemover::remove, which this mirrors.
    // 'context' is our own copy, so the caller's object is never written through.
    context.ignoreModType = true;
    return PyRemapIniRemover::remove(parse, writeBack, context);
}


void initCppGlobalRemapIniRemover(py::module_ &m) {
    py::class_<PyGlobalRemapIniRemover, PyRemapIniRemover, py::smart_holder> cls(m, "GlobalRemapIniRemover", R"doc(
This class inherits from :class:`RemapIniRemover`

General use class for removing the fixes from .ini files, without asking which type of mod the fix
belonged to

Everything about how the fix is found is :class:`RemapIniRemover`'s -- the only difference is that
:meth:`GlobalRemapIniRemover.remove` always behaves as though it were passed a
:class:`IniRemovalContext` with ``ignoreModType`` set.

This is the remover for a .ini file that belongs to a mod but could not be attributed to any type of
mod: :class:`RemapIniRemover`'s stricter rule decides a ``Remap``-named leftover `section`_ outside
the fix boilerplate by asking whether its ``hash`` belongs to one of the .ini file's types of mod,
and a file with no types of mod cannot answer that at all -- so every such leftover would be left
standing. :meth:`IniFile.removeFix` reaches for this class in exactly that state, when its
``readAllIni`` was asked for.

Parameters
----------
iniFile: :class:`IniFile`
    The .ini file to remove the fix from
    )doc");

    cls.def(py::init<py::object>(), py::arg("iniFile") = py::none());

    // Re-bound rather than inherited from the RemapIniRemover registration, for the same reason
    // bindBaseIniRemoverCommonMethods exists at all: an inherited 'remove' would dispatch through
    // the base's own lambda, which is typed on PyRemapIniRemover.
    bindBaseIniRemoverCommonMethods<PyGlobalRemapIniRemover>(cls, R"doc(
Removes the fix from the .ini file, without asking which type of mod it belonged to

The fix is whatever the fix boilerplate surrounds, plus every ``Remap``-named leftover outside it --
whoever they belong to -- together with everything those reference and everything that references
them.

Parameters
----------
parse: :class:`bool`
    Ignored -- the resources that went with the removed `sections`_ are always collected, and are
    available from :meth:`RemapIniRemover.getRemovedResources` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

writeBack: :class:`bool`
    Whether to write back the new text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

context: :class:`IniRemovalContext`
    The per-call options for this removal. Its ``ignoreModType`` is ignored -- this class always
    behaves as though it were ``True`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, which means a default-constructed one

Returns
-------
:class:`str`
    The new content of the .ini file
    )doc");
}
