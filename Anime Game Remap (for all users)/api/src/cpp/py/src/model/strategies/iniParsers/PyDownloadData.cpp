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

#include "PyDownloadData.h"

#include <utility>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/files/FileDownload.h"


namespace {

using KVPs = std::vector<std::pair<std::string, std::string>>;

// `KVPs`_ given as a list of (key, value) pairs or a dict, in their order
KVPs kvpsFromPy(const py::object &kvps) {
    KVPs result;
    if (kvps.is_none()) {
        return result;
    }

    if (py::isinstance<py::dict>(kvps)) {
        for (auto item : kvps.cast<py::dict>()) {
            result.emplace_back(py::str(item.first).cast<std::string>(), py::str(item.second).cast<std::string>());
        }
        return result;
    }

    for (auto item : kvps) {
        auto pair = py::reinterpret_borrow<py::object>(item).cast<std::pair<std::string, std::string>>();
        result.push_back(std::move(pair));
    }
    return result;
}

}


void initCppDownloadData(pybind11::module_ &m) {
    py::class_<PyIniParseDownloadDataCore, PyDownloadDataTrampoline<PyIniParseDownloadDataCore>, py::smart_holder>(m, "IniParseDownloadData", R"doc(
A file a mod is missing and has to download, as a parser asks for it -- how a `section`_ references it, the resource
`section`_ that names the file, and the download itself

Subclass it to download something :class:`DownloadData` cannot express: override all six methods (``name`` and
``refToSection`` included, which :class:`DownloadData` holds as data instead). A parser builds
the resource `section`_ and the download with :meth:`createResSection` and :meth:`createFileDownload`, and registers
what they return with the .ini file itself
    )doc")
        .def(py::init<>())
        .def("name", &PyIniParseDownloadDataCore::name, py::doc(R"doc(
The name of the download resource in the .ini file

Returns
-------
:class:`str`
    The name
        )doc"))
        .def("refToSection", &PyIniParseDownloadDataCore::refToSection, py::doc(R"doc(
Whether the download is referenced once from the top of each `section`_ that needs it, rather than from each of
the section's parts

Returns
-------
:class:`bool`
    ``True`` to reference it from the top of the `section`_
        )doc"))
        .def("addToPart", &PyIniParseDownloadDataCore::addToPart, py::arg("part"), py::arg("key"), py::arg("val"), py::doc(R"doc(
Adds a reference to the download into 'part'

Parameters
----------
part: :class:`IfContentPart`
    The part to add the reference to

key: :class:`str`
    The key of the download reference `KVP`_

val: :class:`str`
    The value of the download reference `KVP`_
        )doc"))
        .def("addToSection", &PyIniParseDownloadDataCore::addToSection, py::arg("section"), py::arg("key"), py::arg("val"), py::doc(R"doc(
Adds a reference to the download to the front of 'section'

Parameters
----------
section: :class:`IfTemplate`
    The `section`_ to add the reference to

key: :class:`str`
    The key of the download reference `KVP`_

val: :class:`str`
    The value of the download reference `KVP`_
        )doc"))
        .def("createResSection", &PyIniParseDownloadDataCore::createResSection, py::arg("sectionName"), py::doc(R"doc(
Builds the `section`_ that holds the download's resource

Parameters
----------
sectionName: :class:`str`
    The name of the `section`_

Returns
-------
:class:`IfTemplate`
    The built `section`_
        )doc"))
        .def("createFileDownload", &PyIniParseDownloadDataCore::createFileDownload, py::arg("iniFolder"), py::doc(R"doc(
Builds the download of the file

Parameters
----------
iniFolder: :class:`str`
    The folder the .ini file lives in

Returns
-------
Optional[:class:`IniResource`]
    The download, or ``None`` if there is nothing to download
        )doc"));

    py::class_<PyDownloadDataCore, PyIniParseDownloadDataCore, PyDownloadDataTrampoline<PyDownloadDataCore>, py::smart_holder>(m, "DownloadData", R"doc(
This class inherits from :class:`IniParseDownloadData`

A file to download from a url, and the resource `section`_ that names it: ``filename = <the file>`` after any
:attr:`resourceKVPs`. Every part that references the download also gets :attr:`downloadRefKVPs` after the
reference -- eg. :meth:`blendRefKVPs` for a downloaded ``Blend.buf``, which is bound and drawn by hand

Parameters
----------
name: :class:`str`
    The name of the download resource in the .ini file

download: :class:`FileDownload`
    The file download to initiate -- the class keeps its own copy

refToSection: :class:`bool`
    Whether to reference the download once from the top of each `section`_ that needs it, rather than from each
    of its parts :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

downloadRefKVPs: Optional[Union[List[Tuple[:class:`str`, :class:`str`]], Dict[:class:`str`, :class:`str`]]]
    Any `KVPs`_ to add after each reference to the download :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

resourceKVPs: Optional[Union[List[Tuple[:class:`str`, :class:`str`]], Dict[:class:`str`, :class:`str`]]]
    Any `KVPs`_ to add before the file path in the resource `section`_ -- eg. ``type`` and ``stride`` for a buffer
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")
        .def(py::init([](std::string name, const AGRC::FileDownload &download, bool refToSection, py::object downloadRefKVPs,
                         py::object resourceKVPs) {
                 return std::make_unique<PyDownloadDataTrampoline<PyDownloadDataCore>>(
                     std::move(name), std::make_unique<AGRC::FileDownload>(download.url, download.filename, download.cache),
                     AGRC::DownloadTools::makeConfig(), refToSection, kvpsFromPy(downloadRefKVPs), kvpsFromPy(resourceKVPs));
             }),
             py::arg("name"), py::arg("download"), py::arg("refToSection") = false, py::arg("downloadRefKVPs") = py::none(),
             py::arg("resourceKVPs") = py::none())
        .def_property_readonly("name", &PyDownloadDataCore::name, py::doc(R"doc(
:class:`str`: The name of the download resource in the .ini file
        )doc"))
        .def_property_readonly("refToSection", &PyDownloadDataCore::refToSection, py::doc(R"doc(
:class:`bool`: Whether the download is referenced once from the top of each `section`_ that needs it, rather than
from each of its parts
        )doc"))
        .def_property_readonly("download", &PyDownloadDataCore::download, py::return_value_policy::reference_internal, py::doc(R"doc(
:class:`FileDownload`: The file download to initiate
        )doc"))
        .def_property_readonly("downloadRefKVPs", &PyDownloadDataCore::downloadRefKVPs, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: The `KVPs`_ added after each reference to the download
        )doc"))
        .def_property_readonly("resourceKVPs", &PyDownloadDataCore::resourceKVPs, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: The `KVPs`_ added before the file path in the resource `section`_
        )doc"))
        .def_static("blendRefKVPs", &PyDownloadDataCore::blendRefKVPs, py::arg("vertexCount"), py::doc(R"doc(
The two `KVPs`_ that must follow a reference to a downloaded ``Blend.buf``: ``handling = skip`` and
``draw = <vertexCount>,0``. The .ini file it is dropped into has no draw of its own for it, so without them the blend
is bound and never drawn from -- pass them as :attr:`downloadRefKVPs`

Parameters
----------
vertexCount: :class:`int`
    How many vertices the drawn model has

Returns
-------
List[Tuple[:class:`str`, :class:`str`]]
    The `KVPs`_
        )doc"));
}
