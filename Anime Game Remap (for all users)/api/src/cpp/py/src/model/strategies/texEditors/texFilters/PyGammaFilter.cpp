#include "PyGammaFilter.h"

#include "AGRemapCore/model/strategies/texEditors/texFilters/GammaFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppGammaFilter(pybind11::module_ &m) {
    py::class_<AGRC::GammaFilter, AGRC::BaseTexFilter, py::smart_holder>(m, "CppGammaFilter", R"doc(
This class inherits from :class:`CppBaseTexFilter`

Performs a `Gamma Correction`_ on the texture file, using the following simple power-law
relationship, applied independently to every pixel's R/G/B channels (the alpha channel is left
untouched):

.. code-block::

    V_out = V_in ^ (1 / gamma)

Where ``V_out`` is the perceived brightness by human eyes while ``V_in`` is the actual brightness
of the image.

.. note::
    Higher :attr:`gamma` values make the image look brighter and less saturated; lower
    :attr:`gamma` values make the image look darker and more saturated
    )doc")

        .def(py::init<double>(), py::arg("gamma"), py::doc(R"doc(
Constructs a new gamma filter

Parameters
----------
gamma: :class:`float`
    The luminance parameter for how bright humans perceive the image
        )doc"))

        .def_readwrite("gamma", &AGRC::GammaFilter::gamma, py::doc(R"doc(
:class:`float`: The luminance parameter for how bright humans perceive the image
        )doc"));
}
