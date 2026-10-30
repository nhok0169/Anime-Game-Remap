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

#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/SkeletonNumberingRead.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"

#include "PyBaseIniGraphGroupEdit.h"
#include "PyIniGraphGroups.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppSkeletonNumberingRead(pybind11::module_ &m) {
    using Read = AGRC::SkeletonNumberingRead;

    auto cls = py::class_<Read, PyBaseIniGraphGroupEdit, AGRC::IParseResult, py::smart_holder>(m, "SkeletonNumberingRead", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit` and :class:`IParseResult`

Reads which skeleton numbering the mod's blend gives its bone ids in, with a :class:`SkeletonNumberingProbe` over
the mod's own buffers, editing nothing -- and files it as :meth:`IniParseData.blendVersion`

The buffers are the files the .ini's resource `sections`_ named here give, found without regard to the case of their
names, so the probe runs only from :meth:`editFromIni` with a real .ini file. Nothing is read with fewer than two
numberings, with no centroids, or when a buffer is missing

Parameters
----------
numberings: List[:class:`SkeletonNumberingProbe.Numbering`]
    The candidate numberings -- see :meth:`SkeletonNumberingProbe.probe`

centroids: Dict[:class:`int`, Tuple[:class:`float`, :class:`float`, :class:`float`]]
    Each bone's rest-pose centroid -- see :meth:`SkeletonNumberingProbe.probe`

positionResource: :class:`str`
    The resource `section`_ of the mod's positions

blendResource: :class:`str`
    The resource `section`_ of the mod's blend

vertexVGResource: :class:`str`
    Text in the name of the resource `section`_ of the mod's 16-bit bone ids, compared lowercased and with
    underscores removed, or empty for none. The first such `section`_ in name order is read
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

requiredKey: :class:`str`
    A key at least one root `section`_ of the graphs of :attr:`objs` (in the first group) must carry for the mod to
    be probed, or empty to probe every mod :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

objs: Optional[Callable[[Tuple[:class:`str`, :class:`str`]], :class:`bool`]]
    The mod objects whose root `sections`_ :attr:`requiredKey` is looked for in; ``None`` looks in every one
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init<std::vector<AGRC::SkeletonNumberingProbe::Numbering>, std::unordered_map<long long, std::array<double, 3>>,
                     std::string, std::string, std::string, std::string, Read::ObjFilter>(),
            py::arg("numberings"), py::arg("centroids"), py::arg("positionResource"), py::arg("blendResource"),
            py::arg("vertexVGResource") = std::string(), py::arg("requiredKey") = std::string(), py::arg("objs") = py::none())
       .def_readwrite("numberings", &Read::numberings,
                      py::doc(R"doc(List[:class:`SkeletonNumberingProbe.Numbering`]: The candidate numberings)doc"))
       .def_readwrite("centroids", &Read::centroids,
                      py::doc(R"doc(Dict[:class:`int`, Tuple[:class:`float`, :class:`float`, :class:`float`]]: Each bone's rest-pose centroid)doc"))
       .def_readwrite("positionResource", &Read::positionResource,
                      py::doc(R"doc(:class:`str`: The resource `section`_ of the mod's positions)doc"))
       .def_readwrite("blendResource", &Read::blendResource,
                      py::doc(R"doc(:class:`str`: The resource `section`_ of the mod's blend)doc"))
       .def_readwrite("vertexVGResource", &Read::vertexVGResource,
                      py::doc(R"doc(:class:`str`: Text in the name of the resource `section`_ of the mod's 16-bit bone ids)doc"))
       .def_readwrite("requiredKey", &Read::requiredKey,
                      py::doc(R"doc(:class:`str`: A key the mod must carry to be probed, or empty for none)doc"))
       .def_readwrite("objs", &Read::objs,
                      py::doc(R"doc(Optional[Callable[[Tuple[:class:`str`, :class:`str`]], :class:`bool`]]: The mod objects whose root `sections`_ the required key is looked for in)doc"))
       .def_property_readonly("version", &Read::version,
                              py::doc(R"doc(Optional[:class:`str`]: The version the last run found, or ``None``)doc"))
       .def("clear", &Read::clear, py::doc(R"doc(Forgets :attr:`version`)doc"));

    cls.def("edit", [](Read& self, py::list graphGroups, const py::object& modType, const std::string& modName) {
        (void)modType;
        PyIniGraphGroups groups(graphGroups);
        self.edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Probes nothing: there is no .ini file to read the buffers' names from

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in
    )doc"));

    cls.def("editFromIni", [](Read& self, py::list graphGroups, const py::object& ini, const py::object& modType,
                              const std::string& modName) {
        (void)modType;
        PyIniGraphGroups groups(graphGroups);
        self.editFromIni(groups, ini.is_none() ? nullptr : ini.cast<AGRC::IniFile*>(), nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Probes the mod's buffers -- see this class's own description

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs of each .ini file, for :attr:`requiredKey`

ini: Optional[:class:`IniFile`]
    The .ini file whose resources name the buffers -- ``None`` probes nothing

modType: Optional[:class:`ModType`]
    Unused

modName: :class:`str`
    Unused :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in
    )doc"));
}
