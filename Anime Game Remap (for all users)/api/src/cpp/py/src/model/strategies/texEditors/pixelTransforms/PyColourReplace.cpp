#include "PyColourReplace.h"

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/ColourReplace.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"
#include "../../../textures/PyColourRange.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppColourReplace(pybind11::module_ &m) {
    py::class_<AGRC::ColourReplace, AGRC::BasePixelTransform, py::smart_holder>(m, "CppColourReplace", R"doc(
This class inherits from :class:`CppBasePixelTransform`

Replaces a coloured pixel
    )doc")

        .def(py::init([](AGRC::Colour replaceColour, py::object coloursToReplace, bool replaceAlpha) {
            return std::make_unique<AGRC::ColourReplace>(replaceColour, parseColourOrRangeSet(coloursToReplace), replaceAlpha);
        }), py::arg("replaceColour"), py::arg("coloursToReplace") = py::none(), py::arg("replaceAlpha") = true, py::doc(R"doc(
Constructs a new colour-replace pixel transform

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

        .def_readwrite("replaceColour", &AGRC::ColourReplace::replaceColour, py::doc(R"doc(
:class:`CppColour`: The colour to fill in
        )doc"))

        .def_property("coloursToReplace", [](const AGRC::ColourReplace &self) {
            return colourOrRangeSetToPy(self.coloursToReplace);
        }, [](AGRC::ColourReplace &self, py::object coloursToReplace) {
            self.coloursToReplace = parseColourOrRangeSet(coloursToReplace);
        }, py::doc(R"doc(
Optional[Set[Union[:class:`CppColour`, :class:`CppColourRange`]]]: The colours to find to be
replaced. If this value is ``None``, then will always replace the colour of the pixel
        )doc"))

        .def_readwrite("replaceAlpha", &AGRC::ColourReplace::replaceAlpha, py::doc(R"doc(
:class:`bool`: Whether to also replace the alpha channel of the original colour
        )doc"));
}
