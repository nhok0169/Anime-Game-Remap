#include "PyGIBuilder.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppGIBuilder(pybind11::module_ &m) {
    // 'Cpp'-prefixed -- collides with the live pure-Python 'GIBuilder' class in
    // constants/GIBuilder.py; see PyModType.cpp's binding comment for the same reasoning.
    py::class_<AGRC::GIBuilder>(m, "CppGIBuilder", R"doc(
Creates new :class:`CppModType` objects for GI (Genshin Impact) mods

Mirrors the pure-Python :class:`GIBuilder` class, but builds the lighter, C++-side
:class:`CppModType` (id, name, and aliases only) instead of the full pure-Python :class:`ModType`
    )doc")

        .def_static("amber", &AGRC::GIBuilder::amber, py::doc(R"doc(Creates the :class:`CppModType` for Amber)doc"))
        .def_static("amberCN", &AGRC::GIBuilder::amberCN, py::doc(R"doc(Creates the :class:`CppModType` for AmberCN)doc"))
        .def_static("ayaka", &AGRC::GIBuilder::ayaka, py::doc(R"doc(Creates the :class:`CppModType` for Ayaka)doc"))
        .def_static("ayakaSpringBloom", &AGRC::GIBuilder::ayakaSpringBloom, py::doc(R"doc(Creates the :class:`CppModType` for AyakaSpringBloom)doc"))
        .def_static("arlecchino", &AGRC::GIBuilder::arlecchino, py::doc(R"doc(Creates the :class:`CppModType` for Arlecchino)doc"))
        .def_static("barbara", &AGRC::GIBuilder::barbara, py::doc(R"doc(Creates the :class:`CppModType` for Barbara)doc"))
        .def_static("barbaraSummerTime", &AGRC::GIBuilder::barbaraSummerTime, py::doc(R"doc(Creates the :class:`CppModType` for BarbaraSummerTime)doc"))
        .def_static("cherryHutao", &AGRC::GIBuilder::cherryHutao, py::doc(R"doc(Creates the :class:`CppModType` for CherryHuTao)doc"))
        .def_static("diluc", &AGRC::GIBuilder::diluc, py::doc(R"doc(Creates the :class:`CppModType` for Diluc)doc"))
        .def_static("dilucFlamme", &AGRC::GIBuilder::dilucFlamme, py::doc(R"doc(Creates the :class:`CppModType` for DilucFlamme)doc"))
        .def_static("fischl", &AGRC::GIBuilder::fischl, py::doc(R"doc(Creates the :class:`CppModType` for Fischl)doc"))
        .def_static("fischlHighness", &AGRC::GIBuilder::fischlHighness, py::doc(R"doc(Creates the :class:`CppModType` for FischlHighness)doc"))
        .def_static("ganyu", &AGRC::GIBuilder::ganyu, py::doc(R"doc(Creates the :class:`CppModType` for Ganyu)doc"))
        .def_static("ganyuTwilight", &AGRC::GIBuilder::ganyuTwilight, py::doc(R"doc(Creates the :class:`CppModType` for GanyuTwilight)doc"))
        .def_static("huTao", &AGRC::GIBuilder::huTao, py::doc(R"doc(Creates the :class:`CppModType` for HuTao)doc"))
        .def_static("jean", &AGRC::GIBuilder::jean, py::doc(R"doc(Creates the :class:`CppModType` for Jean)doc"))
        .def_static("jeanCN", &AGRC::GIBuilder::jeanCN, py::doc(R"doc(Creates the :class:`CppModType` for JeanCN)doc"))
        .def_static("jeanSea", &AGRC::GIBuilder::jeanSea, py::doc(R"doc(Creates the :class:`CppModType` for JeanSea)doc"))
        .def_static("kaeya", &AGRC::GIBuilder::kaeya, py::doc(R"doc(Creates the :class:`CppModType` for Kaeya)doc"))
        .def_static("kaeyaSailwind", &AGRC::GIBuilder::kaeyaSailwind, py::doc(R"doc(Creates the :class:`CppModType` for KaeyaSailwind)doc"))
        .def_static("keqing", &AGRC::GIBuilder::keqing, py::doc(R"doc(Creates the :class:`CppModType` for Keqing)doc"))
        .def_static("keqingOpulent", &AGRC::GIBuilder::keqingOpulent, py::doc(R"doc(Creates the :class:`CppModType` for KeqingOpulent)doc"))
        .def_static("kirara", &AGRC::GIBuilder::kirara, py::doc(R"doc(Creates the :class:`CppModType` for Kirara)doc"))
        .def_static("kiraraBoots", &AGRC::GIBuilder::kiraraBoots, py::doc(R"doc(Creates the :class:`CppModType` for KiraraBoots)doc"))
        .def_static("klee", &AGRC::GIBuilder::klee, py::doc(R"doc(Creates the :class:`CppModType` for Klee)doc"))
        .def_static("kleeBlossomingStarlight", &AGRC::GIBuilder::kleeBlossomingStarlight, py::doc(R"doc(Creates the :class:`CppModType` for KleeBlossomingStarlight)doc"))
        .def_static("lisa", &AGRC::GIBuilder::lisa, py::doc(R"doc(Creates the :class:`CppModType` for Lisa)doc"))
        .def_static("lisaStudent", &AGRC::GIBuilder::lisaStudent, py::doc(R"doc(Creates the :class:`CppModType` for LisaStudent)doc"))
        .def_static("mona", &AGRC::GIBuilder::mona, py::doc(R"doc(Creates the :class:`CppModType` for Mona)doc"))
        .def_static("monaCN", &AGRC::GIBuilder::monaCN, py::doc(R"doc(Creates the :class:`CppModType` for MonaCN)doc"))
        .def_static("nilou", &AGRC::GIBuilder::nilou, py::doc(R"doc(Creates the :class:`CppModType` for Nilou)doc"))
        .def_static("nilouBreeze", &AGRC::GIBuilder::nilouBreeze, py::doc(R"doc(Creates the :class:`CppModType` for NilouBreeze)doc"))
        .def_static("ningguang", &AGRC::GIBuilder::ningguang, py::doc(R"doc(Creates the :class:`CppModType` for Ningguang)doc"))
        .def_static("ningguangOrchid", &AGRC::GIBuilder::ningguangOrchid, py::doc(R"doc(Creates the :class:`CppModType` for Ningguang)doc"))
        .def_static("raiden", &AGRC::GIBuilder::raiden, py::doc(R"doc(Creates the :class:`CppModType` for Ei)doc"))
        .def_static("rosaria", &AGRC::GIBuilder::rosaria, py::doc(R"doc(Creates the :class:`CppModType` for Rosaria)doc"))
        .def_static("rosariaCN", &AGRC::GIBuilder::rosariaCN, py::doc(R"doc(Creates the :class:`CppModType` for RosariaCN)doc"))
        .def_static("shenhe", &AGRC::GIBuilder::shenhe, py::doc(R"doc(Creates the :class:`CppModType` for Shenhe)doc"))
        .def_static("shenheFrostFlower", &AGRC::GIBuilder::shenheFrostFlower, py::doc(R"doc(Creates the :class:`CppModType` for ShenheFrostFlower)doc"))
        .def_static("xiangling", &AGRC::GIBuilder::xiangling, py::doc(R"doc(Creates the :class:`CppModType` for Xiangling)doc"))
        .def_static("xianglingCheer", &AGRC::GIBuilder::xianglingCheer, py::doc(R"doc(Creates the :class:`CppModType` for XianglingCheer)doc"))
        .def_static("xingqiu", &AGRC::GIBuilder::xingqiu, py::doc(R"doc(Creates the :class:`CppModType` for Xingqiu)doc"))
        .def_static("xingqiuBamboo", &AGRC::GIBuilder::xingqiuBamboo, py::doc(R"doc(Creates the :class:`CppModType` for XingqiuBamboo)doc"));
}
