// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "PyMaterialBandRemapFilter.h"

#include <optional>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppMaterialBandRemapFilter(pybind11::module_ &m) {
    py::class_<AGRC::MaterialBandRemapFilter, AGRC::BaseTexFilter, py::smart_holder> cls(m, "CppMaterialBandRemapFilter", R"doc(
This class inherits from :class:`CppBaseTexFilter`

Moves a light map's MATERIAL BANDS from one skin's legend onto another's, optionally conditioned on
the DIFFUSE underneath each pixel. See :cpp:class:`AGRemapCore::MaterialBandRemapFilter` for why the
legend differs per skin and per object, and why a move off band ``0`` wants a gate

.. warning::
    Every decision is made from the **ORIGINAL** alpha, never from a value an earlier band wrote:
    the moves are typically a *permutation*, and applied in sequence a permutation chases itself

A diffuse that is absent or cannot be read makes every gate PASS

:raw-html:`<br />`

.. container:: operations

    **Supported Operations:**

    .. describe:: x(texFile)

        Calls :meth:`CppBaseTexFilter.transform` for the filter, ``x``

:raw-html:`<br />`

Examples
--------

The per-object light map edit a fixer config's ``lightMapEdit`` wants, from one table:

.. code-block:: python
    :linenos:

    import FixRaidenBoss2 as FRB

    Band = FRB.CppMaterialBandRemapFilter.Band
    bands = [Band(76, 77, 178),                  # gold onto the target's gold
             Band(115, 127, 255, FRB.CppMaterialBandRemapFilter.skinColoured)]

    config.lightMapEdit = lambda diffusePath: FRB.CppMaterialBandRemapFilter(bands, diffusePath)
    )doc");

    py::class_<AGRC::MaterialBandRemapFilter::Band>(cls, "Band", R"doc(
One move: a source band (or an inclusive range of bands) to the alpha to write, with an optional test
on the diffuse under the pixel
    )doc")
        .def(py::init([](int band, int to, std::optional<AGRC::MaterialBandRemapFilter::ColourPredicate> when, bool negate) {
            return AGRC::MaterialBandRemapFilter::Band(static_cast<std::uint8_t>(band), static_cast<std::uint8_t>(to),
                                                       when.value_or(nullptr), negate);
        }), py::arg("band"), py::arg("to"), py::arg("when") = py::none(), py::arg("negate") = false, py::doc(R"doc(
Constructs a move off a single band

Parameters
----------
band: :class:`int`
    The source band

to: :class:`int`
    The alpha to write

when: Optional[Callable[[:class:`int`, :class:`int`, :class:`int`], :class:`bool`]]
    The test the diffuse's red, green and blue under the pixel must pass. ``None`` moves unconditionally.
    Pass :meth:`CppMaterialBandRemapFilter.skinColoured` or :meth:`CppMaterialBandRemapFilter.whiteFurColoured`
    to keep the test in C++ -- a Python function here is called once per pixel :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

negate: :class:`bool`
    Whether ``when`` is inverted -- the move happens where the diffuse does **not** pass it :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
        )doc"))
        .def(py::init([](int low, int high, int to, std::optional<AGRC::MaterialBandRemapFilter::ColourPredicate> when, bool negate) {
            return AGRC::MaterialBandRemapFilter::Band(static_cast<std::uint8_t>(low), static_cast<std::uint8_t>(high),
                                                       static_cast<std::uint8_t>(to), when.value_or(nullptr), negate);
        }), py::arg("low"), py::arg("high"), py::arg("to"), py::arg("when") = py::none(), py::arg("negate") = false, py::doc(R"doc(
Constructs a move off a range of bands

Parameters
----------
low: :class:`int`
    The lowest alpha of the source band, inclusive

high: :class:`int`
    The highest alpha of the source band, inclusive

to: :class:`int`
    The alpha to write

when: Optional[Callable[[:class:`int`, :class:`int`, :class:`int`], :class:`bool`]]
    As for the single-band constructor :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

negate: :class:`bool`
    Whether ``when`` is inverted :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
        )doc"))
        .def_property("low", [](const AGRC::MaterialBandRemapFilter::Band &self) { return static_cast<int>(self.low); },
                      [](AGRC::MaterialBandRemapFilter::Band &self, int v) { self.low = static_cast<std::uint8_t>(v); },
                      py::doc(":class:`int`: The lowest alpha of the source band, inclusive"))
        .def_property("high", [](const AGRC::MaterialBandRemapFilter::Band &self) { return static_cast<int>(self.high); },
                      [](AGRC::MaterialBandRemapFilter::Band &self, int v) { self.high = static_cast<std::uint8_t>(v); },
                      py::doc(":class:`int`: The highest alpha of the source band, inclusive"))
        .def_property("to", [](const AGRC::MaterialBandRemapFilter::Band &self) { return static_cast<int>(self.to); },
                      [](AGRC::MaterialBandRemapFilter::Band &self, int v) { self.to = static_cast<std::uint8_t>(v); },
                      py::doc(":class:`int`: The alpha to write"))
        .def_readwrite("negate", &AGRC::MaterialBandRemapFilter::Band::negate,
                       py::doc(":class:`bool`: Whether the diffuse test is inverted"))
        .def_property_readonly("hasTest", [](const AGRC::MaterialBandRemapFilter::Band &self) { return static_cast<bool>(self.when); },
                               py::doc(":class:`bool`: Whether the move is conditioned on the diffuse"));

    cls
        .def(py::init([](std::optional<std::vector<AGRC::MaterialBandRemapFilter::Band>> bands, std::string diffusePath) {
            return AGRC::MaterialBandRemapFilter(bands.value_or(std::vector<AGRC::MaterialBandRemapFilter::Band>{}), std::move(diffusePath));
        }), py::arg("bands") = py::none(), py::arg("diffusePath") = "", py::doc(R"doc(
Constructs a new material band remap filter

Parameters
----------
bands: Optional[List[:class:`CppMaterialBandRemapFilter.Band`]]
    The moves to apply, all from the ORIGINAL alpha; the FIRST whose source range contains a pixel's
    band is the one that applies :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, no moves

diffusePath: :class:`str`
    The diffuse to read the gates against. If this names no readable file, every gate passes :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``
        )doc"))

        .def_readwrite("bands", &AGRC::MaterialBandRemapFilter::bands,
                       py::doc("List[:class:`CppMaterialBandRemapFilter.Band`]: The moves to apply, all from the ORIGINAL alpha"))
        .def_readwrite("diffusePath", &AGRC::MaterialBandRemapFilter::diffusePath,
                       py::doc(":class:`str`: The diffuse to read the gates against"))

        .def_static("skinColoured", &AGRC::MaterialBandRemapFilter::skinColoured, py::arg("red"), py::arg("green"), py::arg("blue"),
                    py::doc(R"doc(
Whether a colour looks like SKIN -- warm, not too dark, red at least green at least blue, and separated
enough to not be a grey

Parameters
----------
red: :class:`int`
    The red channel, 0-255

green: :class:`int`
    The green channel, 0-255

blue: :class:`int`
    The blue channel, 0-255

Returns
-------
:class:`bool`
    Whether the colour looks like skin
                    )doc"))

        .def_static("whiteFurColoured", &AGRC::MaterialBandRemapFilter::whiteFurColoured, py::arg("red"), py::arg("green"), py::arg("blue"),
                    py::doc(R"doc(
Whether a colour looks like WHITE FUR -- bright, and close to grey

Parameters
----------
red: :class:`int`
    The red channel, 0-255

green: :class:`int`
    The green channel, 0-255

blue: :class:`int`
    The blue channel, 0-255

Returns
-------
:class:`bool`
    Whether the colour looks like white fur
                    )doc"));
}
