#include "PyBaseRegEdit.h"

#include "AGRemapCore/model/strategies/iniFixers/BaseIniGraphPartEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniPartEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyPartRanges::PyPartRanges(const py::object &partRanges) {
    if (partRanges.is_none()) {
        return;
    }

    // A bound Ranges instance -- borrowed straight through, no copy. PyRanges<long long> is what
    // 'Ranges' is registered as on the Python side (PyRanges<int> is the separate 'RangesInt').
    if (py::isinstance<PyRanges<long long>>(partRanges)) {
        ranges_ = py::cast<PyRanges<long long> *>(partRanges);
        return;
    }

    // A raw list of (start, end) bounds -- materialized as-is, with normalize = false, matching
    // parseRanges()'s own treatment of this shape (it hands the raw vector straight to the
    // ranges-taking method without normalizing it either).
    owned_.emplace(py::cast<std::vector<AGRC::Ranges<long long>::Range>>(partRanges), false);
    ranges_ = &(*owned_);
}

const AGRC::Ranges<long long>* PyPartRanges::get() const {
    return ranges_;
}


void initCppBaseRegEdit(pybind11::module_ &m) {
    // These carry the bare 'BaseIniPartEdit'/'BaseIniGraphPartEdit' names: the pure-Python classes
    // that used to hold them are deleted, so the Cpp- prefix these were first registered under is
    // no longer needed (see Architecture/CLAUDE.md's Cpp-prefix rule). They exist so BaseRegEdit's
    // real C++ inheritance chain is also real at the pybind11 level (isinstance/attribute
    // inheritance, and 'clear' below). BaseIniGraphEdit/BaseIniGraphGroupEdit are registered
    // against these same two bases by their own bindings.
    py::class_<AGRC::BaseIniPartEdit, py::smart_holder>(m, "BaseIniPartEdit", R"doc(
Base class for a filter that edits some part of a `.ini` file

.. note::
    The deleted pure-Python original also declared ``edit``/``editFromIni`` here, as
    ``(*args, modType, modName = "", **kwargs) -> Any``. That signature has no C++ equivalent --
    every subclass takes genuinely different arguments and returns a different type -- so each
    subclass family declares its own **typed** ``edit``/``editFromIni`` pair instead (see
    :class:`BaseRegEdit`), and only :meth:`clear` (which really is common) lives here
    )doc")

        .def(py::init<>())

        .def("clear", &AGRC::BaseIniPartEdit::clear, py::doc(R"doc(
Clears any saved state information. No-op by default
        )doc"));


    py::class_<AGRC::BaseIniGraphPartEdit, AGRC::BaseIniPartEdit, py::smart_holder>(m, "BaseIniGraphPartEdit", R"doc(
This class inherits from :class:`BaseIniPartEdit`

Base class for a filter that edits some part of a caller/callee graph (:class:`IniSectionGraph`)
within a `.ini` file

Adds nothing of its own over :class:`BaseIniPartEdit` -- exactly like the pure-Python original,
this exists purely to mark the graph-editing half of the edit hierarchy apart from the rest
    )doc")

        .def(py::init<>());


    py::class_<PyBaseRegEdit, AGRC::BaseIniGraphPartEdit, py::smart_holder>(m, "BaseRegEdit", R"doc(
This class inherits from :class:`BaseIniGraphPartEdit`

Base class for a filter that edits some registers within an :class:`IfContentPart`
    )doc")

        .def(py::init<>())

        // Deliberately routed through self.attr("edit") rather than bound to the C++
        // BaseRegEdit::editFromIni (which calls this->edit(...) as a *C++* virtual call): with no
        // trampoline in play, a pure-Python subclass overriding only 'edit' has no C++-side vtable
        // entry for that override, so a C++-internal virtual call would silently run the no-op base
        // implementation. Python attribute lookup resolves to whatever 'edit' really is on the
        // most-derived object -- a C++-native override (eg. RegAdd) or a pure-Python one -- exactly
        // like the pure-Python original's own 'return self.edit(...)' did.
        .def("editFromIni", [](py::object self, const py::object &part, const std::string &sectionName,
                               const py::object &ini, const py::object &modType, const std::string &modName,
                               const py::object &partRanges) {
            // 'ini' is deliberately unused -- exactly as in the pure-Python original.
            (void)ini;
            return self.attr("edit")(part, sectionName, modType, modName, partRanges);
        }, py::arg("part"), py::arg("sectionName"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "",
           py::arg("partRanges") = py::none(), py::doc(R"doc(
Edits the registers for the current :class:`IfContentPart` with state info from 'ini'

.. note::
    This forwards straight to :meth:`edit` and ignores 'ini' entirely, exactly as the pure-Python
    original does

Parameters
----------
part: :class:`IfContentPart`
    The part of the `IfTemplate` that is being editted

sectionName: :class:`str`
    The name of the `section`_ that is being editted

ini: Optional[:class:`IniFile`]
    The associated .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partRanges: Optional[:class:`Ranges`]
    The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IfContentPart`
    The resultant part of the `IfTemplate` that got its registers editted
        )doc"))

        .def("edit", [](PyBaseRegEdit &self, py::object part, const std::string &sectionName,
                        const py::object &modType, const std::string &modName, const py::object &partRanges) {
            (void)self;
            (void)modType;
            (void)sectionName;
            (void)modName;
            (void)partRanges;
            return part;
        }, py::arg("part"), py::arg("sectionName"), py::arg("modType"), py::arg("modName") = "",
           py::arg("partRanges") = py::none(), py::doc(R"doc(
Edits the registers for the current :class:`IfContentPart`. No-op by default, returning 'part'
untouched

Parameters
----------
part: :class:`IfContentPart`
    The part of the `IfTemplate` that is being editted

sectionName: :class:`str`
    The name of the `section`_ that is being editted

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partRanges: Optional[:class:`Ranges`]
    The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IfContentPart`
    The resultant part of the `IfTemplate` that got its registers editted
        )doc"));
}
