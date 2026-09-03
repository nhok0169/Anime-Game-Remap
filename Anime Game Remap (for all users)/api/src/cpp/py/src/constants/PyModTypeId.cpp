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
        )doc"))

        .def_static("getName", &AGRC::ModTypeIdTools::getName, py::arg("value"), py::doc(R"doc(
Retrieves the corresponding name for a :class:`ModTypeId`

Parameters
----------
value: :class:`ModTypeId`
    The :class:`ModTypeId` to retrieve the name for

Returns
-------
:class:`str`
    The name for 'value'
        )doc"))

        .def_static("getModType", &AGRC::ModTypeIdTools::getModType, py::arg("modTypeId"), py::doc(R"doc(
Retrieves the :class:`ModType` registered for a :class:`ModTypeId`, if one has been registered
(via :meth:`registerModType`)

This is a plain lookup into a global registry shared by every caller of :class:`ModTypeIdTools` --
it never builds a :class:`ModType` itself. If a :class:`ModTypeId` is never registered, nothing
is ever built for it, since building one can be expensive; only a :class:`ModTypeId` that's actually
been registered (typically by whichever builder -- e.g. :class:`GIBuilder` -- actually owns it)
can be retrieved here

Parameters
----------
modTypeId: :class:`int`
    The integer id for the :class:`ModTypeId` to retrieve the registered :class:`ModType` for --
    stored/looked-up as-is, with no validation that it corresponds to one of :class:`ModTypeId`'s
    declared values, so a custom mod type using some id not registered in :class:`ModTypeId` can
    still be looked up here

Returns
-------
Optional[:class:`ModType`]
    The registered :class:`ModType`, if one exists for 'modTypeId'
        )doc"))

        .def_static("registerModType", &AGRC::ModTypeIdTools::registerModType, py::arg("modType"), py::doc(R"doc(
Registers a :class:`ModType` into the global registry, under the :class:`ModTypeId` it owns
(``modType.modTypeId``) :raw-html:`<br />` :raw-html:`<br />`

If a :class:`ModType` is already registered for that :class:`ModTypeId`, it gets overwritten with
the new one

Parameters
----------
modType: :class:`ModType`
    The :class:`ModType` to register
        )doc"))

        .def_static("findByName", &AGRC::ModTypeIdTools::findByName, py::arg("name"), py::arg("gameTypeId") = py::none(), py::doc(R"doc(
Finds the :class:`ModTypeId` whose registered :class:`ModType` name or alias maximally matches
some string, similar to how :meth:`BaseIniClassifier.classify`'s section-name reading searches
its own registered keywords

Only searches names/aliases of :class:`ModType` s that have actually been registered (via
:meth:`registerModType`) -- an unregistered :class:`ModTypeId` can never be found this way, even if
'name' textually matches what :meth:`getName` would return for it :raw-html:`<br />` :raw-html:`<br />`

If more than one registered :class:`ModTypeId` shares the maximally-matched name (or alias) -- after
filtering by 'gameTypeId', when given -- the match is ambiguous and ``None`` is returned rather than
guessing

Parameters
----------
name: :class:`str`
    The string to search for a registered :class:`ModType` name/alias within

gameTypeId: Optional[:class:`GameTypeId`]
    If provided, only considers a :class:`ModType` registered under this :class:`GameTypeId` (via
    ``modType.gameTypeId``) a candidate match :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
Optional[:class:`ModTypeId`]
    The matched :class:`ModTypeId`, if exactly one unambiguous match was found
        )doc"))

        .def_static("clear", &AGRC::ModTypeIdTools::clear, py::doc(R"doc(
Clears the global registry -- every :class:`ModType` registered via :meth:`registerModType` is
forgotten, and :meth:`getModType`/:meth:`findByName` behave as if nothing was ever registered

Mirrors :meth:`HashTools.clear`/:meth:`CppHashTools.clear` -- meant for resetting shared global
state between independent uses (e.g. between unit tests)
        )doc"))

        .def_static("getHashRemapTargets", &AGRC::ModTypeIdTools::getHashRemapTargets, py::arg("value"), py::doc(R"doc(
Retrieves the mod types a given mod type's **hashes** can be remapped onto

This is the remap graph itself. It mirrors the ``map`` argument the pure-Python :class:`GIBuilder`
passes to each mod type's :class:`Hashes`, lifted out of the 43 individual factories into one table
so a target is named by :class:`ModTypeId` rather than by a bare string

.. note::
    Two :class:`ModTypeId`\s -- ``RaidenBoss`` and ``ArlecchinoBoss`` -- only ever appear as
    *targets* and are never a source, which is why :class:`GIBuilder` has no factory for them

Parameters
----------
value: :class:`ModTypeId`
    The mod type to look up the remap targets of

Returns
-------
List[:class:`ModTypeId`]
    The mod types 'value' remaps onto, or an empty list if it remaps onto none
        )doc"))

        .def_static("getIndexRemapTargets", &AGRC::ModTypeIdTools::getIndexRemapTargets, py::arg("value"), py::doc(R"doc(
Retrieves the mod types a given mod type's **indices** can be remapped onto

Identical to :meth:`getHashRemapTargets` for every mod type but one: ``Raiden`` remaps by hash only

Parameters
----------
value: :class:`ModTypeId`
    The mod type to look up the remap targets of

Returns
-------
List[:class:`ModTypeId`]
    The mod types 'value' remaps onto, or an empty list if it remaps onto none
        )doc"));
}
