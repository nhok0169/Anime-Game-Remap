#include "PyColourReplaceFilter.h"

#include "AGRemapCore/model/strategies/texEditors/texFilters/ColourReplaceFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "PyTexFilterCommon.h"
#include "../../../textures/PyColourRange.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppColourReplaceFilter(pybind11::module_ &m) {
    py::class_<AGRC::ColourReplaceFilter, AGRC::BaseTexFilter, py::smart_holder>(m, "CppColourReplaceFilter", R"doc(
This class inherits from :class:`CppBaseTexFilter`

Replaces specific colours in the image
    )doc")

        .def(py::init([](AGRC::Colour replaceColour, py::object coloursToReplace, bool replaceAlpha) {
            return std::make_unique<AGRC::ColourReplaceFilter>(replaceColour, parseColourOrRangeSet(coloursToReplace), replaceAlpha);
        }), py::arg("replaceColour"), py::arg("coloursToReplace") = py::none(), py::arg("replaceAlpha") = true, py::doc(R"doc(
Constructs a new colour-replace filter

Parameters
----------
replaceColour: :class:`CppColour`
    The colour to fill in

coloursToReplace: Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]
    The colours to find to be replaced. If this value is ``None``, then will always replace the
    colour of the pixel. **Default**: ``None``

replaceAlpha: :class:`bool`
    Whether to also replace the alpha channel of the original colour. **Default**: ``True``
        )doc"))

        .def_readwrite("replaceColour", &AGRC::ColourReplaceFilter::replaceColour, py::doc(R"doc(
:class:`CppColour`: The colour to fill in
        )doc"))

        .def_property("coloursToReplace", [](const AGRC::ColourReplaceFilter &self) {
            return colourOrRangeSetToPy(self.coloursToReplace);
        }, [](AGRC::ColourReplaceFilter &self, py::object coloursToReplace) {
            self.coloursToReplace = parseColourOrRangeSet(coloursToReplace);
        }, py::doc(R"doc(
Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]: The colours to find to be
replaced. If this value is ``None``, then will always replace the colour of the pixel
        )doc"))

        .def_readwrite("replaceAlpha", &AGRC::ColourReplaceFilter::replaceAlpha, py::doc(R"doc(
:class:`bool`: Whether to also replace the alpha channel of the original colour
        )doc"))

        // Overrides the inherited 'transform' binding: this filter's real C++ logic operates on
        // the core RGBA8 buffer, but 'texFile.img' (a Pillow Image) is the source of truth shared
        // with the other, still-unported filters in a TexEditor chain -- see PyTexFilterCommon.h.
        .def("transform", [](AGRC::ColourReplaceFilter &self, py::object texFileObj) {
            AGRC::TextureFile &texFile = syncTextureFileFromImg(texFileObj);
            self.transform(texFile);
            syncTextureFileToImg(texFileObj);
        }, py::arg("texFile"), py::doc(R"doc(
Replaces the matching colours across the entire image

Parameters
----------
texFile: :class:`TextureFile`
    The texture to be edited
        )doc"));
}
