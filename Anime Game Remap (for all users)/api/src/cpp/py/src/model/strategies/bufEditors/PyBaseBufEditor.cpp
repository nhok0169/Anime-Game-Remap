#include "PyBaseBufEditor.h"

#include <pybind11/stl.h>

#include "../../files/PyBufFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


AGRC::BufFile::FixResult PyBaseBufEditor::fix(AGRC::BufFile &bufFile, const std::optional<std::string> &fixedBufFile) {
    py::gil_scoped_acquire gil;
    py::function override = py::get_override(static_cast<const AGRC::BaseBufEditor*>(this), "fix");
    if (!override) {
        return AGRC::BaseBufEditor::fix(bufFile, fixedBufFile);
    }

    py::object result = override(bufFile, fixedBufFile);
    return pyToFixResult(result);
}


void initCppBaseBufEditor(pybind11::module_ &m) {
    py::class_<AGRC::BaseBufEditor, PyBaseBufEditor, py::smart_holder>(m, "BaseBufEditor", R"doc(
Base class to edit some ``.buf`` file
    )doc")

        .def(py::init<>())

        .def("fix", [](AGRC::BaseBufEditor &self, AGRC::BufFile &bufFile, py::object fixedBufFile) {
            std::optional<std::string> fixedOpt;
            if (!fixedBufFile.is_none()) {
                fixedOpt = fixedBufFile.cast<std::string>();
            }

            AGRC::BufFile::FixResult result = self.fix(bufFile, fixedOpt);
            return fixResultToPy(result);
        }, py::arg("bufFile"), py::arg("fixedBufFile") = py::none(), py::doc(R"doc(
Edits the ``.buf`` file. No-op by default

Parameters
----------
bufFile: :class:`CppBufFile`
    The binary ``.buf`` file to be modified

fixedBufFile: Optional[:class:`str`]
    The name of the fixed ``.buf`` file. If this is ``None``, the fixed bytes are returned directly
    instead of being written to a file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
Union[:class:`str`, :class:`bytearray`]
    If the argument ``fixedBufFile`` is ``None``, then will return an array of bytes for the fixed
    ``.buf`` file :raw-html:`<br />` :raw-html:`<br />`
    Otherwise will return the filename to the fixed ``.buf`` file
        )doc"));
}
