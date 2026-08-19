#include "PyModTypeId.h"

#include <optional>

#include <pybind11/stl.h>

#include "AGRemapCore/constants/ModTypeId.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppModTypeId(pybind11::module_ &m) {
    // Registered under the bare 'ModTypeId' name (no 'Cpp' prefix) -- no pure-Python class of this
    // exact bare name exists to shadow (the pure-Python equivalents are the differently-named
    // 'ModTypeNames' enum and the deprecated StrEnumOld-based 'ModTypes' class), so nothing to
    // disambiguate from; see Documentation/CLAUDE.md's naming-pitfall section /
    // Architecture/CLAUDE.md's 'Cpp' prefix rule.
    py::enum_<AGRC::ModTypeId>(m, "ModTypeId", R"doc(
The names of the different types of mods this fix will fix from or fix to

Mirrors the keys of the pure-Python ``ModTypeNames`` enum (``constants/ModTypeNames.py``)
    )doc")
        .value("Amber", AGRC::ModTypeId::Amber, R"doc(Amber from GI)doc")

        .value("AmberCN", AGRC::ModTypeId::AmberCN, R"doc(Amber Chinese version from GI)doc")

        .value("Ayaka", AGRC::ModTypeId::Ayaka, R"doc(Ayaka from GI)doc")

        .value("AyakaSpringbloom", AGRC::ModTypeId::AyakaSpringbloom, R"doc(Ayaka Fontaine skin from GI)doc")

        .value("Arlecchino", AGRC::ModTypeId::Arlecchino, R"doc(Arlecchino from GI)doc")

        .value("ArlecchinoBoss", AGRC::ModTypeId::ArlecchinoBoss, R"doc(The first phase of the Arlecchino boss from GI)doc")

        .value("Barbara", AGRC::ModTypeId::Barbara, R"doc(Barbara from GI)doc")

        .value("BarbaraSummertime", AGRC::ModTypeId::BarbaraSummertime, R"doc(Barbara summer skin from GI)doc")

        .value("CherryHuTao", AGRC::ModTypeId::CherryHuTao, R"doc(Hu Tao Lantern Rite skin from GI)doc")

        .value("Diluc", AGRC::ModTypeId::Diluc, R"doc(Diluc from GI)doc")

        .value("DilucFlamme", AGRC::ModTypeId::DilucFlamme, R"doc(Diluc Red Dead of the Night skin from GI)doc")

        .value("Fischl", AGRC::ModTypeId::Fischl, R"doc(Fischl from GI)doc")

        .value("FischlHighness", AGRC::ModTypeId::FischlHighness, R"doc(Fischl summer skin from GI)doc")

        .value("Ganyu", AGRC::ModTypeId::Ganyu, R"doc(Ganyu from GI)doc")

        .value("GanyuTwilight", AGRC::ModTypeId::GanyuTwilight, R"doc(Ganyu Lantern Rite skin from GI)doc")

        .value("HuTao", AGRC::ModTypeId::HuTao, R"doc(HuTao from GI)doc")

        .value("Jean", AGRC::ModTypeId::Jean, R"doc(Jean from GI)doc")

        .value("JeanCN", AGRC::ModTypeId::JeanCN, R"doc(Jean Chinese version from GI)doc")

        .value("JeanSea", AGRC::ModTypeId::JeanSea, R"doc(Jean summer skin from GI)doc")

        .value("Kaeya", AGRC::ModTypeId::Kaeya, R"doc(Kaeya from GI)doc")

        .value("KaeyaSailwind", AGRC::ModTypeId::KaeyaSailwind, R"doc(KaeyaSailwind from GI)doc")

        .value("Keqing", AGRC::ModTypeId::Keqing, R"doc(Keqing from GI)doc")

        .value("KeqingOpulent", AGRC::ModTypeId::KeqingOpulent, R"doc(Keqing Lantern Rite skin from GI)doc")

        .value("Kirara", AGRC::ModTypeId::Kirara, R"doc(Kirara from GI)doc")

        .value("KiraraBoots", AGRC::ModTypeId::KiraraBoots, R"doc(Kirara summer skin from GI)doc")

        .value("Klee", AGRC::ModTypeId::Klee, R"doc(Klee from GI)doc")

        .value("KleeBlossomingStarlight", AGRC::ModTypeId::KleeBlossomingStarlight, R"doc(Klee summer skin from GI)doc")

        .value("Lisa", AGRC::ModTypeId::Lisa, R"doc(Lisa from GI)doc")

        .value("LisaStudent", AGRC::ModTypeId::LisaStudent, R"doc(Lisa Sumeru skin from GI)doc")

        .value("Mona", AGRC::ModTypeId::Mona, R"doc(Mona from GI)doc")

        .value("MonaCN", AGRC::ModTypeId::MonaCN, R"doc(Mona Chinese version from GI)doc")

        .value("Nilou", AGRC::ModTypeId::Nilou, R"doc(Nilou from GI)doc")

        .value("NilouBreeze", AGRC::ModTypeId::NilouBreeze, R"doc(Nilou summer skin from GI)doc")

        .value("Ningguang", AGRC::ModTypeId::Ningguang, R"doc(Ningguang from GI)doc")

        .value("NingguangOrchid", AGRC::ModTypeId::NingguangOrchid, R"doc(Ningguang Lantern Rite from GI)doc")

        .value("Raiden", AGRC::ModTypeId::Raiden, R"doc(Ei from GI)doc")

        .value("RaidenBoss", AGRC::ModTypeId::RaidenBoss, R"doc(The first phase of the Raiden Shogun boss from GI)doc")

        .value("Rosaria", AGRC::ModTypeId::Rosaria, R"doc(Rosaria from GI)doc")

        .value("RosariaCN", AGRC::ModTypeId::RosariaCN, R"doc(Rosaria Chinese version from GI)doc")

        .value("Shenhe", AGRC::ModTypeId::Shenhe, R"doc(Shenhe from GI)doc")

        .value("ShenheFrostFlower", AGRC::ModTypeId::ShenheFrostFlower, R"doc(Shenhe Lantern Rite skin from GI)doc")

        .value("Xiangling", AGRC::ModTypeId::Xiangling, R"doc(Xiangling from GI)doc")

        .value("XianglingCheer", AGRC::ModTypeId::XianglingCheer, R"doc(Xiangling Lantern Rite skin from GI)doc")

        .value("Xingqiu", AGRC::ModTypeId::Xingqiu, R"doc(Xingqiu from GI)doc")

        .value("XingqiuBamboo", AGRC::ModTypeId::XingqiuBamboo, R"doc(Xingqiu Lantern Rite skin from GI)doc");

    // Also bare-named -- no pure-Python 'ModTypeIdTools' class exists to shadow either.
    py::class_<AGRC::ModTypeIdTools>(m, "ModTypeIdTools", R"doc(
Tools for handling :class:`ModTypeId`
    )doc")
        .def_static("getEnum", &AGRC::ModTypeIdTools::getEnum, py::arg("value"), py::doc(R"doc(
Retrieves the corresponding :class:`ModTypeId` for some integer value, checking that the value
actually corresponds to one of :class:`ModTypeId`'s declared values

Parameters
----------
value: :class:`int`
    The integer value to convert

Returns
-------
Optional[:class:`ModTypeId`]
    The corresponding :class:`ModTypeId`, if 'value' is valid
        )doc"));
}
