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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/ChannelMapFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/GuidedScaleFilter.h"
#include "PyTexFilterCommon.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppChannelMapFilter(pybind11::module_ &m) {
    py::class_<AGRC::ChannelMapFilter, AGRC::BaseTexFilter, py::smart_holder> cls(m, "ChannelMapFilter", R"doc(
This class inherits from :class:`CppBaseTexFilter`

Writes each channel of every pixel from a channel of the same pixel through a 256-entry table: a material-code map
swapped between two characters' codes, a channel set to a constant, a channel copied from another, alpha clamped, a
colour gain. Every channel is read before any is written :raw-html:`<br />` :raw-html:`<br />`

How a channel is read is :attr:`read`: the stored bytes, or the values a shader reads
(:meth:`CppTextureFile.shaderChannel`) -- a code map stored sRGB reaches its shader decoded. With :attr:`data` the
written bytes are data rather than colour, and the texture's gamma is cleared so they are saved as they are

Parameters
----------
read: :class:`ChannelMapFilter.Read`
    How every source channel is read :raw-html:`<br />` :raw-html:`<br />`

    **Default**: :attr:`ChannelMapFilter.Read.Raw`

data: :class:`bool`
    Whether the written bytes are data rather than colour :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc");

    py::enum_<AGRC::ChannelMapFilter::Read>(cls, "Read", "How a source channel is read")
        .value("Raw", AGRC::ChannelMapFilter::Read::Raw, "The stored bytes")
        .value("ShaderCode", AGRC::ChannelMapFilter::Read::ShaderCode,
               "As a shader reads a code: decoded, truncated (:meth:`CppTextureFile.shaderChannel`)")
        .value("ShaderValue", AGRC::ChannelMapFilter::Read::ShaderValue, "As a shader reads a value: decoded, rounded");

    py::class_<AGRC::ChannelMapFilter::Channel>(cls, "Channel", "What one output channel is written from")
        .def(py::init<>())
        .def(py::init([](int source, std::vector<std::uint8_t> table) {
            return AGRC::ChannelMapFilter::Channel{source, std::move(table)};
        }), py::arg("source"), py::arg("table") = std::vector<std::uint8_t>{})
        .def_readwrite("source", &AGRC::ChannelMapFilter::Channel::source,
            py::doc(":class:`int`: The channel read (0 to 3), or -1 to leave this channel as it is. **Default**: -1"))
        .def_readwrite("table", &AGRC::ChannelMapFilter::Channel::table,
            py::doc("List[:class:`int`]: The value written for each value read: 256 entries, or empty to write the value read. **Default**: empty"));

    cls.def(py::init([](AGRC::ChannelMapFilter::Read read, bool data) {
            AGRC::ChannelMapFilter filter;
            filter.read = read;
            filter.data = data;
            return filter;
        }), py::arg("read") = AGRC::ChannelMapFilter::Read::Raw, py::arg("data") = false)
        .def_readwrite("channels", &AGRC::ChannelMapFilter::channels,
            py::doc("List[:class:`ChannelMapFilter.Channel`]: The output channels R, G, B and A, in that order -- assign the whole list, or use :meth:`setChannel`"))
        .def_readwrite("read", &AGRC::ChannelMapFilter::read,
            py::doc(":class:`ChannelMapFilter.Read`: How every source channel is read. **Default**: :attr:`ChannelMapFilter.Read.Raw`"))
        .def_readwrite("data", &AGRC::ChannelMapFilter::data,
            py::doc(":class:`bool`: Whether the written bytes are data (codes, flags) rather than colour: the gamma is cleared. **Default**: ``False``"))
        .def("setChannel", [](AGRC::ChannelMapFilter &self, int channel, int source, std::vector<std::uint8_t> table) {
            if (channel < 0 || channel > 3) {
                throw py::index_error("ChannelMapFilter.setChannel: 'channel' must be 0 to 3");
            }
            self.channels[static_cast<std::size_t>(channel)] = {source, std::move(table)};
        }, py::arg("channel"), py::arg("source"), py::arg("table") = std::vector<std::uint8_t>{}, py::doc(R"doc(
Writes output 'channel' from 'source' through 'table'

Parameters
----------
channel: :class:`int`
    The output channel, 0 (R) to 3 (A)

source: :class:`int`
    The channel read, or -1 to leave the output channel as it is

table: List[:class:`int`]
    The value written for each value read: 256 entries, or empty to write the value read :raw-html:`<br />` :raw-html:`<br />`

    **Default**: empty
        )doc"))
        .def_static("constant", &AGRC::ChannelMapFilter::constant, py::arg("value"), py::doc(R"doc(
A table that writes 'value' for every value read

Parameters
----------
value: :class:`int`
    The value

Returns
-------
List[:class:`int`]
    The table
        )doc"))
        .def_static("codeMap", &AGRC::ChannelMapFilter::codeMap, py::arg("table"), py::doc(R"doc(
A filter for a material-code map: each texel's code -- the red channel as a shader reads it
(:attr:`ChannelMapFilter.Read.ShaderCode`) -- through 'table', written to R, G and B as data. A single-channel map
comes in as ``(v, v, v, 255)`` and is written the same way

Parameters
----------
table: List[:class:`int`]
    The code written for each code read: 256 entries

Returns
-------
:class:`ChannelMapFilter`
    The filter
        )doc"))
        .def("transform", [](AGRC::ChannelMapFilter &self, py::object texFileObj) {
            AGRC::TextureFile &texFile = syncTextureFileFromImg(texFileObj);
            self.transform(texFile);
            syncTextureFileToImg(texFileObj);
        }, py::arg("texFile"), py::doc(R"doc(
Writes the channels of 'texFile'

Parameters
----------
texFile: :class:`CppTextureFile`
    The texture to be edited
        )doc"));
}


void initCppGuidedScaleFilter(pybind11::module_ &m) {
    py::class_<AGRC::GuidedScaleFilter, AGRC::BaseTexFilter, py::smart_holder>(m, "GuidedScaleFilter", R"doc(
This class inherits from :class:`CppBaseTexFilter`

Scales the colour of every pixel by a factor another texture -- the GUIDE -- chooses: the guide's channel at the same
UV (nearest texel, the two textures may differ in size) picks one of 256 RGB factors. What a texel IS comes from the
guide, never from its own colour: a material-code map says which material a diffuse texel paints, a normal map's
alpha which shading band it sits in :raw-html:`<br />` :raw-html:`<br />`

With :attr:`linear` the factor applies to the colour decoded from sRGB (2.2) when the texture is stored sRGB, and
the result is encoded back. Results are rounded and clamped to 0-255. Alpha is untouched. A guide that is not there
(:attr:`guide` empty, or a file that does not open) leaves the texture as it is

Parameters
----------
guide: :class:`str`
    The guide texture's file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

channel: :class:`int`
    The guide's channel that chooses the factor, 0 (R) to 3 (A) :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``0``

factors: List[Tuple[:class:`float`, :class:`float`, :class:`float`]]
    The RGB factor for each value of the guide's channel: 256 entries, or empty to leave every texel :raw-html:`<br />` :raw-html:`<br />`

    **Default**: empty

read: :class:`ChannelMapFilter.Read`
    How the guide's channel is read :raw-html:`<br />` :raw-html:`<br />`

    **Default**: :attr:`ChannelMapFilter.Read.Raw`

linear: :class:`bool`
    Whether to scale the colour decoded from sRGB, for a texture stored sRGB :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc")
        .def(py::init([](std::string guide, int channel, std::vector<std::array<double, 3>> factors,
                         AGRC::ChannelMapFilter::Read read, bool linear) {
            AGRC::GuidedScaleFilter filter;
            filter.guide = std::move(guide);
            filter.channel = channel;
            filter.factors = std::move(factors);
            filter.read = read;
            filter.linear = linear;
            return filter;
        }), py::arg("guide") = "", py::arg("channel") = 0, py::arg("factors") = std::vector<std::array<double, 3>>{},
            py::arg("read") = AGRC::ChannelMapFilter::Read::Raw, py::arg("linear") = false)
        .def_readwrite("guide", &AGRC::GuidedScaleFilter::guide, py::doc(":class:`str`: The guide texture's file"))
        .def_readwrite("channel", &AGRC::GuidedScaleFilter::channel,
            py::doc(":class:`int`: The guide's channel that chooses the factor: 0 (R) to 3 (A)"))
        .def_readwrite("factors", &AGRC::GuidedScaleFilter::factors,
            py::doc("List[Tuple[:class:`float`, :class:`float`, :class:`float`]]: The RGB factor for each value of the guide's channel: 256 entries, or empty to leave every texel"))
        .def_readwrite("read", &AGRC::GuidedScaleFilter::read,
            py::doc(":class:`ChannelMapFilter.Read`: How the guide's channel is read"))
        .def_readwrite("linear", &AGRC::GuidedScaleFilter::linear,
            py::doc(":class:`bool`: Whether to scale the colour decoded from sRGB, for a texture stored sRGB"))
        .def("transform", [](AGRC::GuidedScaleFilter &self, py::object texFileObj) {
            AGRC::TextureFile &texFile = syncTextureFileFromImg(texFileObj);
            self.transform(texFile);
            syncTextureFileToImg(texFileObj);
        }, py::arg("texFile"), py::doc(R"doc(
Scales the colour of 'texFile' by the guide

Parameters
----------
texFile: :class:`CppTextureFile`
    The texture to be edited
        )doc"));
}
