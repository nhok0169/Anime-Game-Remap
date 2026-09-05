#include "PyBufEditor.h"

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyBaseBufEditor.h"
#include "../../files/PyBufFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    // Same conversion PyBufFile.cpp's own CppBufFile.fix binding does for its inline 'filters'
    // argument -- kept private to this file since a BufEditor's filters are re-parsed only when
    // the whole list is (re)assigned, not once per fix() call the way CppBufFile.fix's are.
    std::vector<AGRC::BufFile::Filter> parseFiltersObj(const py::object &filters) {
        std::vector<py::function> rawFilters;
        if (!filters.is_none()) {
            rawFilters = filters.cast<std::vector<py::function>>();
        }

        return parseFilters(rawFilters);
    }

}


AGRC::BufFile::FixResult PyBufEditor::fix(AGRC::BufFile &bufFile, const std::optional<std::string> &fixedBufFile) {
    py::gil_scoped_acquire gil;
    py::function override = py::get_override(static_cast<const AGRC::BaseBufEditor*>(this), "fix");
    if (!override) {
        return AGRC::BufEditor::fix(bufFile, fixedBufFile);
    }

    py::object result = override(bufFile, fixedBufFile);
    return pyToFixResult(result);
}


void initCppBufEditor(pybind11::module_ &m) {
    // PyBufEditor (not AGRC::BufEditor) is registered as the type -- it carries #filtersObj, which
    // AGRC::BufEditor itself has no room for (that member is purely a pybind11-layer convenience,
    // like CoreModMappedAssets::nonVersionIndexNames). Skipping the unregistered AGRC::BufEditor in
    // this base list is the same shape PyResIdentity/PyResReplace/PyResCreate already use against
    // PyBaseResEditCore -- pybind only needs one registered base to establish the isinstance chain.
    py::class_<PyBufEditor, AGRC::BaseBufEditor, py::smart_holder>(m, "BufEditor", R"doc(
This class inherits from :class:`BaseBufEditor`

Class to edit some ``.buf`` file by running a fixed sequence of filters over it

Parameters
----------
filters: Optional[List[Callable[[Dict[:class:`str`, List[Any]], :class:`int`, :class:`int`, :class:`int`], Dict[:class:`str`, List[Any]]]]]
    The filters used to edit the data for each line in the ``.buf`` file :raw-html:`<br />` :raw-html:`<br />`

    The filters take in the following arguments:

    #. The data for a particular line
    #. The starting byte index of the line that is read
    #. The line index being processed
    #. The size of each line :raw-html:`<br />` :raw-html:`<br />`

    The output of the filters is the resultant data that consists where the keys are the names of
    the elements within a line in the ``.buf`` file and the values are the resultant data for each
    element in the line :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")

        .def(py::init([](py::object filters) {
            auto editor = std::make_unique<PyBufEditor>(parseFiltersObj(filters));
            editor->filtersObj = filters.is_none() ? py::list() : py::list(filters);
            return editor;
        }), py::arg("filters") = py::none())

        .def_property("filters", [](PyBufEditor &self) {
            return self.filtersObj;
        }, [](PyBufEditor &self, py::object filters) {
            self.filters = parseFiltersObj(filters);
            self.filtersObj = filters.is_none() ? py::list() : py::list(filters);
        }, py::doc(R"doc(
List[Callable[[Dict[:class:`str`, List[Any]], :class:`int`, :class:`int`, :class:`int`], Dict[:class:`str`, List[Any]]]]:
The filters used to edit the data for each line in the ``.buf`` file
        )doc"));
}
