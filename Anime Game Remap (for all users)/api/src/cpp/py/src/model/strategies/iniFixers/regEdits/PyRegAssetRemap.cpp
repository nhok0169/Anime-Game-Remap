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

#include "PyRegAssetRemap.h"

#include <memory>
#include <utility>
#include <vector>

#include "../../../PyVersion.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// One entry's asset table plus its not-found value. Two shapes are accepted because the two real
// uses want different ones: a drawn object's hash remap writes the HashNotFound sentinel when the
// target has no row, and the face's leaves the value alone -- see GIMICharFixer's note on why a
// missing tex_face_diffuse row is not the same kind of fact as a missing ib row.
PyRegAssetRemap::Core::AssetSpec parseAssetSpec(const py::object &value) {
    py::object tableObj = value;
    std::optional<std::string> notFoundVal;

    if (py::isinstance<py::tuple>(value) || py::isinstance<py::list>(value)) {
        py::sequence seq = value.cast<py::sequence>();
        if (seq.size() == 0) {
            throw py::value_error("A RegAssetRemap asset entry cannot be an empty sequence");
        }

        tableObj = py::reinterpret_borrow<py::object>(seq[0]);

        if (seq.size() > 1) {
            py::object notFoundObj = py::reinterpret_borrow<py::object>(seq[1]);
            if (!notFoundObj.is_none()) {
                notFoundVal = py::str(notFoundObj).cast<std::string>();
            }
        }
    }

    if (tableObj.is_none()) {
        return PyRegAssetRemap::Core::AssetSpec(nullptr, std::move(notFoundVal));
    }

    return PyRegAssetRemap::Core::AssetSpec(tableObj.cast<PyRegAssetRemap::Assets*>(), std::move(notFoundVal));
}

}


PyRegAssetRemap::PyRegAssetRemap(py::object assetsObj, std::string toModName, std::string fromModName,
                                   const py::object &fromVersion, const py::object &toVersion):
    Core({}, std::move(toModName), std::move(fromModName), parseVersionArg(fromVersion),
          parseVersionArg(toVersion)),
    assetsObj(std::move(assetsObj)) {}


void PyRegAssetRemap::refresh(const py::object &modType) {
    (void)modType;  // unused -- the asset tables come from assetsObj
    assets.clear();

    if (assetsObj.is_none()) {
        return;
    }

    py::dict src = py::cast<py::dict>(assetsObj);
    assets.reserve(src.size());

    for (auto item : src) {
        assets.emplace_back(py::str(item.first).cast<std::string>(),
                             parseAssetSpec(py::reinterpret_borrow<py::object>(item.second)));
    }
}


void initCppRegAssetRemap(pybind11::module_ &m) {
    py::class_<PyRegAssetRemap, PyBaseRegEdit, py::smart_holder> cls(m, "RegAssetRemap", R"doc(
This class inherits from :class:`BaseRegEdit`

Class for remapping the **asset values** on specific registers of some :class:`IfContentPart` -- a
hash naming the mod being fixed *from* becomes the equivalent hash naming the mod being fixed *to*

.. note::
    The lookup runs in two steps: the old value is reverse-looked-up to find which row owns it, and
    that row's key is then forward-looked-up against :attr:`toModName`. So this edit does **not**
    need to be told which kind of asset a register holds -- the old value says so

.. warning::
    Leave :attr:`fromModName` empty and the reverse lookup becomes non-deterministic wherever the
    source and the target **share** a value, which every CN pair does for at least one asset. Landing
    on the target's own row asks the remap graph for ``target -> target``, which is not an edge, and
    the register is written as the not-found value instead. Fill it in

.. warning::
    Not for a ``match_first_index``. Reverse-looking-up ``0`` is ambiguous -- it is every character's
    head index -- so the lookup fails and writes the not-found sentinel into a numeric field. Use a
    :class:`RegNewVals` with a forward lookup per mod object instead

Parameters
----------
assets: Dict[:class:`str`, Union[:class:`ModMappedAssets`, Tuple[:class:`ModMappedAssets`, Optional[:class:`str`]]]]
    Which registers are remapped, and against which asset table :raw-html:`<br />` :raw-html:`<br />`

    The keys are register names. A value may be the table on its own, or a
    ``(table, notFoundVal)`` pair -- where ``notFoundVal`` is written when the old value has no
    mapping, and ``None`` (the default) leaves such a value untouched

    :raw-html:`<br />`

    The table objects are held, so a table owned by a :class:`ModType` stays alive as long as this
    edit does

toModName: :class:`str`
    The name of the mod being fixed to

fromModName: :class:`str`
    The name of the mod being fixed from, which is what makes the reverse lookup deterministic --
    see the warning above :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

fromVersion: Optional[Union[:class:`str`, :class:`float`, :class:`Version`]]
    The version being fixed from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

toVersion: Optional[Union[:class:`str`, :class:`float`, :class:`Version`]]
    The version being fixed to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object assets, std::string toModName, std::string fromModName,
                         const py::object &fromVersion, const py::object &toVersion) {
        return std::make_unique<PyRegAssetRemap>(std::move(assets), std::move(toModName), std::move(fromModName),
                                                  fromVersion, toVersion);
    }), py::arg("assets"), py::arg("toModName"), py::arg("fromModName") = "",
        py::arg("fromVersion") = py::none(), py::arg("toVersion") = py::none());

    cls.def_property("assets", [](const PyRegAssetRemap &self) {
        return self.assetsObj;
    }, [](PyRegAssetRemap &self, py::object assets) {
        self.assetsObj = std::move(assets);
    }, py::doc(R"doc(
Dict[:class:`str`, Union[:class:`ModMappedAssets`, Tuple[:class:`ModMappedAssets`, Optional[:class:`str`]]]]: Which
registers are remapped, and against which asset table -- see this class's constructor
    )doc"));

    cls.def_readwrite("toModName", &PyRegAssetRemap::toModName, py::doc(R"doc(
:class:`str`: The name of the mod being fixed to
    )doc"));

    cls.def_readwrite("fromModName", &PyRegAssetRemap::fromModName, py::doc(R"doc(
:class:`str`: The name of the mod being fixed from -- see this class's warning on why leaving this
empty is a real hazard
    )doc"));

    bindRegEditEdit<PyRegAssetRemap>(cls, R"doc(
Remaps every register named in :attr:`assets` onto :attr:`toModName`'s equivalent value

Parameters
----------
part: :class:`IfContentPart`
    The part of the `IfTemplate` that is being editted

sectionName: :class:`str`
    The name of the `section`_ that is being editted. Unused by this edit

modType: Optional[:class:`ModType`]
    The type of mod to fix. Unused by this edit -- the asset tables come from :attr:`assets`

modName: :class:`str`
    The name of the mod to fix to. Unused by this edit, which uses :attr:`toModName` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partRanges: Optional[:class:`Ranges`]
    The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IfContentPart`
    The same part that was passed in, after editing
    )doc");
}
