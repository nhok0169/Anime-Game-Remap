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

        .def("shaderChannel", [](const AGRC::TextureFile &self, std::size_t channel, bool truncate) {
            const std::vector<std::uint8_t> values = self.shaderChannel(channel, truncate);
            return py::bytes(reinterpret_cast<const char*>(values.data()), values.size());
        }, py::arg("channel"), py::arg("truncate") = true, py::doc(R"doc(
The values a shader reads from one channel of :meth:`getPixels`, one byte a pixel

A texture stored sRGB (one :meth:`getGamma` is set for) reaches the shader DECODED: the sampler linearises each of
R, G and B. The decoded value times 255 is truncated when the shader casts it to an integer code (a material-code
map), or rounded when it is a value. Alpha, and every channel of a texture not stored sRGB, is its byte

Parameters
----------
channel: :class:`int`
    The channel: 0 (R), 1 (G), 2 (B) or 3 (A)

truncate: :class:`bool`
    Whether a decoded value is truncated rather than rounded :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

Raises
------
:class:`IndexError`
    If ``channel`` is past 3

Returns
-------
:class:`bytes`
    The values, row-major
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

        // keep_alive<1, 2>: the texture holds a BORROWED TexCache*, so without this a cache that
        // Python has no other reference to is collected while this texture still points at it --
        // and the next open() reads freed memory. The same reason PyFileDownload's own borrowed
        // cache pointer is never handed over from Python at all.
        .def("setCache", &AGRC::TextureFile::setCache, py::arg("cache"), py::keep_alive<1, 2>(),
              py::doc(R"doc(
Gives this texture a run-level :class:`TexCache`, so a source it has already decoded is not decoded
again and an image it has already written is copied rather than re-encoded

``None`` (the default) disables both, which is what every caller had before the cache existed.
:class:`RemapService` sets this itself on the textures it fixes, so this is only needed when
driving :class:`TextureFile` directly

Parameters
----------
cache: Optional[:class:`TexCache`]
    The cache to share, or ``None`` for none. The cache is kept alive for as long as this texture is
        )doc"))

        .def("getCache", &AGRC::TextureFile::getCache, py::return_value_policy::reference_internal,
              py::doc(R"doc(
The cache given to this texture by :meth:`setCache`

Returns
-------
Optional[:class:`TexCache`]
    The shared cache, or ``None`` if this texture is not caching
        )doc"))

        .def("open", &AGRC::TextureFile::open, py::doc(R"doc(
Opens the texture file at :attr:`src`, decoding it into :meth:`getPixels`

If the file does not exist, :attr:`hasImage` becomes ``False`` and :meth:`getPixels` is cleared
        )doc"))

        .def("save", &AGRC::TextureFile::save, py::arg("compress") = true, py::arg("mipmaps") = false, py::doc(R"doc(
Saves :meth:`getPixels` to the texture file at :attr:`src`

``mipmaps`` writes the full mip chain (box-filtered from :meth:`getPixels`) instead of the single
top level. Every texture the game ships carries its chain, and one written without it is sampled
from its top level at every distance, which on a fine texture such as hair reads in game as
scattered off-colour pixels that move with the camera. **Default**: ``False``

If :attr:`gamma` is set, the R/G/B channels of :meth:`getPixels` are gamma-corrected first (see
:class:`CppGammaFilter`), in place. The file is re-encoded to whatever compressed format it was
originally :meth:`open`-ed with -- or, for a texture file that was never successfully opened (eg. a
brand new file), BC7

Parameters
----------
compress: :class:`bool`
    Whether to re-encode to that compressed format, or write the RGBA8 buffer out as a plain
    32-bit uncompressed ``.dds`` :raw-html:`<br />` :raw-html:`<br />`

    BCn encoding is almost the whole cost of an edit -- on a 4096x2048 ``BC7_UNORM`` texture,
    ~1.5s to decode against ~15.5s to re-encode -- so ``False`` makes the round trip roughly eight
    times faster in exchange for a file about four times larger. It is also exactly what the
    `Pillow`_ engine does unconditionally, which never encodes BCn at all :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``True``
        )doc"))

        .def("saveAs", &AGRC::TextureFile::saveAs, py::arg("dest"), py::doc(R"doc(
Saves :meth:`getPixels` to 'dest', leaving both :attr:`src` and :meth:`getPixels` untouched -- the
on-disk format is chosen from 'dest's own file extension

A ``.dds`` destination is re-encoded to the same compressed format :meth:`save` would use; **any
other extension is written uncompressed**, straight from the RGBA8 buffer. `Compressonator`_
handles ``.png``, ``.bmp`` and ``.jpg`` itself this way, which is what makes this the "convert a
texture into something an ordinary image viewer can open" entry point

.. note::
    Unlike :meth:`save`, this **never** applies :attr:`gamma`. Gamma here is a pre-correction for
    the ``.dds``/BCn sRGB round trip specifically (and :meth:`save` applies it destructively, in
    place, to :meth:`getPixels`) -- neither is wanted when the point is to look at the texture's
    actual decoded pixels

Parameters
----------
dest: :class:`str`
    The file path to write to

Returns
-------
:class:`bool`
    Whether the file was actually written
        )doc"));

    auto textureFile = py::reinterpret_borrow<py::class_<AGRC::TextureFile>>(m.attr("CppTextureFile"));
    py::class_<AGRC::TextureFile::DDSHeader>(textureFile, "DDSHeader", R"doc(
What the header of a ``.dds`` file says, without reading its pixels
    )doc")
        .def(py::init<>())
        .def_readwrite("width", &AGRC::TextureFile::DDSHeader::width, ":class:`int`: The width of the top mip, in pixels")
        .def_readwrite("height", &AGRC::TextureFile::DDSHeader::height, ":class:`int`: The height of the top mip, in pixels")
        .def_readwrite("mipCount", &AGRC::TextureFile::DDSHeader::mipCount, ":class:`int`: How many mips the file holds -- at least 1")
        .def_readwrite("dxgiFormat", &AGRC::TextureFile::DDSHeader::dxgiFormat, R"doc(
:class:`int`: The pixel format, as a ``DXGI_FORMAT`` value: the ``DX10`` extension's, or for a legacy
header ``61`` (``R8_UNORM``) for one 8-bit red channel and ``87`` (``B8G8R8A8_UNORM``) for 32-bit BGRA.
``0`` for any other legacy format
        )doc");

    textureFile.def_static("readDDSHeader", &AGRC::TextureFile::readDDSHeader, py::arg("path"), py::doc(R"doc(
Reads the header of the ``.dds`` file at ``path``

Parameters
----------
path: :class:`str`
    The file

Returns
-------
Optional[:class:`CppTextureFile.DDSHeader`]
    The header, or ``None`` when the file is shorter than a header or is not a ``.dds`` file
    )doc"));
}
