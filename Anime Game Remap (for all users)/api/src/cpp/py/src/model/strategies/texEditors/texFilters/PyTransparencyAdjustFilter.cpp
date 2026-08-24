#include "PyTransparencyAdjustFilter.h"

#include "AGRemapCore/model/strategies/texEditors/texFilters/TransparencyAdjustFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "PyTexFilterCommon.h"
#include "../../../textures/PyColourRange.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTransparencyAdjustFilter(pybind11::module_ &m) {
    py::class_<AGRC::TransparencyAdjustFilter, AGRC::BaseTexFilter, py::smart_holder>(m, "CppTransparencyAdjustFilter", R"doc(
This class inherits from :class:`CppBaseTexFilter`

Adjust the transparency (alpha channel) for an image
    )doc")

        .def(py::init([](int alphaChange, py::object coloursToFilter) {
            return std::make_unique<AGRC::TransparencyAdjustFilter>(alphaChange, parseColourOrRangeSet(coloursToFilter));
        }), py::arg("alphaChange"), py::arg("coloursToFilter") = py::none(), py::doc(R"doc(
Constructs a new transparency-adjust filter

Parameters
----------
alphaChange: :class:`int`
    How much to adjust the alpha channel of each pixel. Range from -255 to 255

    .. note::
        The alpha channel for an image is inclusively bounded from 0 to 255

coloursToFilter: Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]
    The specific colours to have their transparency adjusted. If this value is ``None``, then will
    adjust the transparency for the entire image. **Default**: ``None``
        )doc"))

        .def_readwrite("alphaChange", &AGRC::TransparencyAdjustFilter::alphaChange, py::doc(R"doc(
:class:`int`: How much to adjust the alpha channel of each pixel. Range from -255 to 255
        )doc"))

        .def_property("coloursToFilter", [](const AGRC::TransparencyAdjustFilter &self) {
            return colourOrRangeSetToPy(self.coloursToFilter);
        }, [](AGRC::TransparencyAdjustFilter &self, py::object coloursToFilter) {
            self.coloursToFilter = parseColourOrRangeSet(coloursToFilter);
        }, py::doc(R"doc(
Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]: The specific colours to have
their transparency adjusted. If this value is ``None``, then will adjust the transparency for the
entire image
        )doc"))

        // See PyColourReplaceFilter.cpp's identical override for why this doesn't just forward to
        // the inherited (buffer-only) 'transform' binding.
        .def("transform", [](AGRC::TransparencyAdjustFilter &self, py::object texFileObj) {
            AGRC::TextureFile &texFile = syncTextureFileFromImg(texFileObj);
            self.transform(texFile);
            syncTextureFileToImg(texFileObj);
        }, py::arg("texFile"), py::doc(R"doc(
Adjusts the transparency across the image

Parameters
----------
texFile: :class:`TextureFile`
    The texture to be edited
        )doc"));
}
