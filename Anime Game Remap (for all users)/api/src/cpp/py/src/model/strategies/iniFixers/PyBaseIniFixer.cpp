#include "PyBaseIniFixer.h"

#include <memory>
#include <utility>


PyBaseIniFixer::PyBaseIniFixer(py::object parser):
    PyBaseIniFixerCore(nullptr), parserObj(std::move(parser)), iniFileObj(py::none()) {
    // The inherited AGRemapCore::BaseIniParser* is always nullptr: a Python parser has no C++
    // counterpart to point at, the same situation PyBaseIniParser's own .ini file is in.
    if (!parserObj.is_none() && py::hasattr(parserObj, "_iniFile")) {
        iniFileObj = parserObj.attr("_iniFile");
    }
}


py::object PyBaseIniFixer::fixToPy(bool keepBackup, bool fixOnly, bool hideOrig, AGRemapCore::IniFixingContext fixingCtx) {
    ParseData empty;
    fix(empty, keepBackup, fixOnly, hideOrig, fixingCtx);
    return py::none();
}


void initCppBaseIniFixer(pybind11::module_ &m) {
    // py::smart_holder throughout this hierarchy -- required so a shared_ptr<BaseIniFixer> can be
    // extracted from an existing Python object, which is what MultiModFixer's "children" dict
    // is made of. The default unique_ptr holder cannot produce one. Same reason IniResource
    // carries it; pybind11 needs the holder consistent across the whole hierarchy.
    py::class_<PyBaseIniFixerCore, py::smart_holder>(m, "CppBaseIniFixer", R"doc(
The shared C++ base of every fixer, exposed so that one built on the C++ side -- by a
:class:`CppIniFixBuilder`'s default factory, or by anything in ``AGRemapCore`` -- can still cross into
`Python`_ :raw-html:`<br />` :raw-html:`<br />`

Not usually what you want: a fixer created **from** `Python`_ is a :class:`BaseIniFixer`, which
inherits from this and carries the extra `Python`_ state. This class exists so the boundary never
has to hand back ``None`` for a core-side object it has no richer type for
    )doc");


    auto cls = py::class_<PyBaseIniFixer, PyBaseIniFixerCore, py::smart_holder>(m, "BaseIniFixer", R"doc(
Base class to fix a .ini file

Parameters
----------
parser: :class:`BaseIniParser`
    The associated parser to retrieve data for the fix
    )doc");

    cls.def(py::init([](py::object parser) {
        return std::make_unique<PyBaseIniFixer>(std::move(parser));
    }), py::arg("parser") = py::none());

    bindBaseIniFixerCommonMethods<PyBaseIniFixer>(cls, R"doc(
Fixes the .ini file

Parameters
----------
keepBackup: :class:`bool`
    Whether to keep backups for the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

fixOnly: :class:`bool`
    Whether to only fix the .ini file without undoing any fixes :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

hideOrig: :class:`bool`
    Whether to hide the mod for the original character. :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

context: Optional[:class:`IniFixingContext`]
    The per-call options for this fix. If ``None``, a default one is built, which says this
    fixer is the .ini file's last :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
Optional[Dict[Union[:class:`str`, :class:`int`], :class:`str`]]
    The new content of the fixed .ini file(s) -- always ``None`` here, since this base class fixes
    nothing. See :meth:`GIMIFixer.fix` for a real one
    )doc");
}
