#include "PyCorrectGamma.h"

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/CorrectGamma.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppCorrectGamma(pybind11::module_ &m) {
    py::class_<AGRC::CorrectGamma, AGRC::BasePixelTransform, py::smart_holder>(m, "CppCorrectGamma", R"doc(
This class inherits from :class:`CppBasePixelTransform`

Performs a `Gamma Correction`_ on an individual pixel using the following simple power-law
relationship:

.. code-block::

    V_out = V_in ^ (1 / gamma)

Where ``V_out`` is the perceived brightness by human eyes while ``V_in`` is the actual brightness
of the image.

.. note::
    Higher :attr:`gamma` values make the image look brighter and less saturated; lower
    :attr:`gamma` values make the image look darker and more saturated
    )doc")

        .def(py::init<double>(), py::arg("gamma"), py::doc(R"doc(
Constructs a new gamma-correction pixel transform

Parameters
----------
gamma: :class:`float`
    The luminance parameter for how bright humans perceive the image
        )doc"))

        .def_readwrite("gamma", &AGRC::CorrectGamma::gamma, py::doc(R"doc(
:class:`float`: The luminance parameter for how bright humans perceive the image
        )doc"))

        .def_static("correctGamma", &AGRC::CorrectGamma::correctGamma, py::arg("pixelValue"), py::arg("gamma"), py::doc(R"doc(
The equation for the gamma correction done at every colour channel pixel

Parameters
----------
pixelValue: :class:`int`
    The value of the pixel for some colour channel, in [0, 255]

gamma: :class:`float`
    The luminance parameter for how bright humans perceive the image

Returns
-------
:class:`int`
    The gamma corrected pixel value
        )doc"));
}
