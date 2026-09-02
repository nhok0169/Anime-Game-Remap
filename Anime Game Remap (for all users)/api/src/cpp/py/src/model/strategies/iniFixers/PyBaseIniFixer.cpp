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
    auto cls = py::class_<PyBaseIniFixer>(m, "BaseIniFixer", R"doc(
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
