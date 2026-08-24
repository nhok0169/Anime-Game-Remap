#include "PyTextureFile.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/model/files/TextureFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTextureFile(pybind11::module_ &m) {
    py::class_<AGRC::TextureFile>(m, "CppTextureFile", R"doc(
The `Compressonator`_-backed engine behind :class:`TextureFile` -- decodes/encodes a ``.dds``
texture file to/from a flat, uncompressed RGBA8 pixel buffer (see :meth:`getPixels`/
:meth:`setPixels`), remembering the original compressed format so :meth:`save` can re-encode back
to it.

.. note::
    This class is Pillow-free -- :class:`TextureFile` itself layers a real `Pillow`_ ``Image`` (at
    its ``img`` attribute) on top of this class's raw pixel buffer, for the sake of the other
    texture filters in this codebase that still work directly against a real Pillow image
    )doc")

        .def(py::init<std::string>(), py::arg("src"), py::doc(R"doc(
Constructs a new texture file. Does not read anything from disk yet -- see :meth:`open`

Parameters
----------
src: :class:`str`
    The source file path for the texture file
        )doc"))

        .def_property("src", &AGRC::TextureFile::getSrc, &AGRC::TextureFile::setSrc, py::doc(R"doc(
:class:`str`: The source file path for the texture file
        )doc"))

        .def_property_readonly("hasImage", &AGRC::TextureFile::hasImage, py::doc(R"doc(
:class:`bool`: Whether a texture is currently loaded (:meth:`open` succeeded and found a real file)
        )doc"))

        .def_property_readonly("width", &AGRC::TextureFile::getWidth, py::doc(R"doc(
:class:`int`: The width, in pixels, of the currently loaded texture (0 if :attr:`hasImage` is
``False``)
        )doc"))

        .def_property_readonly("height", &AGRC::TextureFile::getHeight, py::doc(R"doc(
:class:`int`: The height, in pixels, of the currently loaded texture (0 if :attr:`hasImage` is
``False``)
        )doc"))

        .def_property("gamma", &AGRC::TextureFile::getGamma, &AGRC::TextureFile::setGamma, py::doc(R"doc(
Optional[:class:`float`]: The luminance parameter used to gamma-correct the R/G/B channels on the
next :meth:`save`, or ``None`` to skip gamma correction entirely
        )doc"))

        .def("getPixels", [](const AGRC::TextureFile &self) {
            const std::vector<std::uint8_t> &pixels = self.getPixels();
            return py::bytes(reinterpret_cast<const char*>(pixels.data()), pixels.size());
        }, py::doc(R"doc(
The current pixel buffer, as flat RGBA8 bytes (4 bytes per pixel, row-major)

Returns
-------
:class:`bytes`
    The current pixel buffer
        )doc"))

        .def("setPixels", [](AGRC::TextureFile &self, py::bytes pixels, int width, int height) {
            std::string raw = pixels;
            std::vector<std::uint8_t> data(raw.begin(), raw.end());
            self.setPixels(std::move(data), width, height);
        }, py::arg("pixels"), py::arg("width"), py::arg("height"), py::doc(R"doc(
Replaces the current pixel buffer, eg. with pixels edited outside of this class

Parameters
----------
pixels: :class:`bytes`
    The new flat RGBA8 pixel buffer (4 bytes per pixel, row-major)

width: :class:`int`
    The width, in pixels, of 'pixels'

height: :class:`int`
    The height, in pixels, of 'pixels'
        )doc"))

        .def("getPixel", &AGRC::TextureFile::getPixel, py::arg("x"), py::arg("y"), py::doc(R"doc(
The colour of the pixel at ('x', 'y'). No bounds checking is performed

Parameters
----------
x: :class:`int`
    The x-coordinate of the pixel

y: :class:`int`
    The y-coordinate of the pixel

Returns
-------
:class:`CppColour`
    The colour of the pixel
        )doc"))

        .def("setPixel", &AGRC::TextureFile::setPixel, py::arg("x"), py::arg("y"), py::arg("colour"), py::doc(R"doc(
Sets the colour of the pixel at ('x', 'y'). No bounds checking is performed

Parameters
----------
x: :class:`int`
    The x-coordinate of the pixel

y: :class:`int`
    The y-coordinate of the pixel

colour: :class:`CppColour`
    The new colour for the pixel
        )doc"))

        .def("open", &AGRC::TextureFile::open, py::doc(R"doc(
Opens the texture file at :attr:`src`, decoding it into :meth:`getPixels`

If the file does not exist, :attr:`hasImage` becomes ``False`` and :meth:`getPixels` is cleared
        )doc"))

        .def("save", &AGRC::TextureFile::save, py::doc(R"doc(
Saves :meth:`getPixels` to the texture file at :attr:`src`

If :attr:`gamma` is set, the R/G/B channels of :meth:`getPixels` are gamma-corrected first (see
:class:`CppGammaFilter`), in place. The file is re-encoded to whatever compressed format it was
originally :meth:`open`-ed with -- or, for a texture file that was never successfully opened (eg. a
brand new file), BC7
        )doc"));
}
