#include "PyGIBuilder.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppGIBuilder(pybind11::module_ &m) {
    // 'Cpp'-prefixed -- collides with the live pure-Python 'GIBuilder' class in
    // constants/GIBuilder.py; see PyModType.cpp's binding comment for the same reasoning.
    py::class_<AGRC::GIBuilder>(m, "GIBuilder", R"doc(
Creates new :class:`ModType` objects for GI (Genshin Impact) mods

Mirrors the pure-Python :class:`GIBuilder` class, but builds the lighter, C++-side
:class:`ModType` (id, name, and aliases only) instead of the full pure-Python :class:`ModType`
    )doc")

        .def_static("amber", &AGRC::GIBuilder::amber, py::doc(R"doc(Creates the :class:`ModType` for Amber)doc"))
        .def_static("amberCN", &AGRC::GIBuilder::amberCN, py::doc(R"doc(Creates the :class:`ModType` for AmberCN)doc"))
        .def_static("ayaka", &AGRC::GIBuilder::ayaka, py::doc(R"doc(Creates the :class:`ModType` for Ayaka)doc"))
        .def_static("ayakaSpringBloom", &AGRC::GIBuilder::ayakaSpringBloom, py::doc(R"doc(Creates the :class:`ModType` for AyakaSpringBloom)doc"))
        .def_static("arlecchino", &AGRC::GIBuilder::arlecchino, py::doc(R"doc(Creates the :class:`ModType` for Arlecchino)doc"))
        .def_static("barbara", &AGRC::GIBuilder::barbara, py::doc(R"doc(Creates the :class:`ModType` for Barbara)doc"))
        .def_static("barbaraSummerTime", &AGRC::GIBuilder::barbaraSummerTime, py::doc(R"doc(Creates the :class:`ModType` for BarbaraSummerTime)doc"))
        .def_static("cherryHutao", &AGRC::GIBuilder::cherryHutao, py::doc(R"doc(Creates the :class:`ModType` for CherryHuTao)doc"))
        .def_static("diluc", &AGRC::GIBuilder::diluc, py::doc(R"doc(Creates the :class:`ModType` for Diluc)doc"))
        .def_static("dilucFlamme", &AGRC::GIBuilder::dilucFlamme, py::doc(R"doc(Creates the :class:`ModType` for DilucFlamme)doc"))
        .def_static("fischl", &AGRC::GIBuilder::fischl, py::doc(R"doc(Creates the :class:`ModType` for Fischl)doc"))
        .def_static("fischlHighness", &AGRC::GIBuilder::fischlHighness, py::doc(R"doc(Creates the :class:`ModType` for FischlHighness)doc"))
        .def_static("ganyu", &AGRC::GIBuilder::ganyu, py::doc(R"doc(Creates the :class:`ModType` for Ganyu)doc"))
        .def_static("ganyuTwilight", &AGRC::GIBuilder::ganyuTwilight, py::doc(R"doc(Creates the :class:`ModType` for GanyuTwilight)doc"))
        .def_static("huTao", &AGRC::GIBuilder::huTao, py::doc(R"doc(Creates the :class:`ModType` for HuTao)doc"))
        .def_static("jean", &AGRC::GIBuilder::jean, py::doc(R"doc(Creates the :class:`ModType` for Jean)doc"))
        .def_static("jeanCN", &AGRC::GIBuilder::jeanCN, py::doc(R"doc(Creates the :class:`ModType` for JeanCN)doc"))
        .def_static("jeanSea", &AGRC::GIBuilder::jeanSea, py::doc(R"doc(Creates the :class:`ModType` for JeanSea)doc"))
        .def_static("kaeya", &AGRC::GIBuilder::kaeya, py::doc(R"doc(Creates the :class:`ModType` for Kaeya)doc"))
        .def_static("kaeyaSailwind", &AGRC::GIBuilder::kaeyaSailwind, py::doc(R"doc(Creates the :class:`ModType` for KaeyaSailwind)doc"))
        .def_static("keqing", &AGRC::GIBuilder::keqing, py::doc(R"doc(Creates the :class:`ModType` for Keqing)doc"))
        .def_static("keqingOpulent", &AGRC::GIBuilder::keqingOpulent, py::doc(R"doc(Creates the :class:`ModType` for KeqingOpulent)doc"))
        .def_static("kirara", &AGRC::GIBuilder::kirara, py::doc(R"doc(Creates the :class:`ModType` for Kirara)doc"))
        .def_static("kiraraBoots", &AGRC::GIBuilder::kiraraBoots, py::doc(R"doc(Creates the :class:`ModType` for KiraraBoots)doc"))
        .def_static("klee", &AGRC::GIBuilder::klee, py::doc(R"doc(Creates the :class:`ModType` for Klee)doc"))
        .def_static("kleeBlossomingStarlight", &AGRC::GIBuilder::kleeBlossomingStarlight, py::doc(R"doc(Creates the :class:`ModType` for KleeBlossomingStarlight)doc"))
        .def_static("lisa", &AGRC::GIBuilder::lisa, py::doc(R"doc(Creates the :class:`ModType` for Lisa)doc"))
        .def_static("lisaStudent", &AGRC::GIBuilder::lisaStudent, py::doc(R"doc(Creates the :class:`ModType` for LisaStudent)doc"))
        .def_static("mona", &AGRC::GIBuilder::mona, py::doc(R"doc(Creates the :class:`ModType` for Mona)doc"))
        .def_static("monaCN", &AGRC::GIBuilder::monaCN, py::doc(R"doc(Creates the :class:`ModType` for MonaCN)doc"))
        .def_static("nilou", &AGRC::GIBuilder::nilou, py::doc(R"doc(Creates the :class:`ModType` for Nilou)doc"))
        .def_static("nilouBreeze", &AGRC::GIBuilder::nilouBreeze, py::doc(R"doc(Creates the :class:`ModType` for NilouBreeze)doc"))
        .def_static("ningguang", &AGRC::GIBuilder::ningguang, py::doc(R"doc(Creates the :class:`ModType` for Ningguang)doc"))
        .def_static("ningguangOrchid", &AGRC::GIBuilder::ningguangOrchid, py::doc(R"doc(Creates the :class:`ModType` for Ningguang)doc"))
        .def_static("raiden", &AGRC::GIBuilder::raiden, py::doc(R"doc(Creates the :class:`ModType` for Ei)doc"))
        .def_static("rosaria", &AGRC::GIBuilder::rosaria, py::doc(R"doc(Creates the :class:`ModType` for Rosaria)doc"))
        .def_static("rosariaCN", &AGRC::GIBuilder::rosariaCN, py::doc(R"doc(Creates the :class:`ModType` for RosariaCN)doc"))
        .def_static("shenhe", &AGRC::GIBuilder::shenhe, py::doc(R"doc(Creates the :class:`ModType` for Shenhe)doc"))
        .def_static("shenheFrostFlower", &AGRC::GIBuilder::shenheFrostFlower, py::doc(R"doc(Creates the :class:`ModType` for ShenheFrostFlower)doc"))
        .def_static("xiangling", &AGRC::GIBuilder::xiangling, py::doc(R"doc(Creates the :class:`ModType` for Xiangling)doc"))
        .def_static("xianglingCheer", &AGRC::GIBuilder::xianglingCheer, py::doc(R"doc(Creates the :class:`ModType` for XianglingCheer)doc"))
        .def_static("xingqiu", &AGRC::GIBuilder::xingqiu, py::doc(R"doc(Creates the :class:`ModType` for Xingqiu)doc"))
        .def_static("xingqiuBamboo", &AGRC::GIBuilder::xingqiuBamboo, py::doc(R"doc(Creates the :class:`ModType` for XingqiuBamboo)doc"));
}
