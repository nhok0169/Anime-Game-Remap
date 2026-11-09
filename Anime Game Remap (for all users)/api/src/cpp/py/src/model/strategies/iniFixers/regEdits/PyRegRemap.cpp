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

#include "PyRegRemap.h"

#include <memory>
#include <utility>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {
    // the C++ member as Python reads it: key -> the keys it becomes (or its KeyRemapData)
    py::dict keyRemapToPy(const PyRegRemap &edit) {
        py::dict result;
        for (const auto &[key, value] : edit.keyRemap) {
            result[py::str(key)] = py::cast(value);
        }
        return result;
    }
}


void initCppRegRemap(pybind11::module_ &m) {
    py::class_<PyRegRemap, PyBaseRegEdit, py::smart_holder> cls(m, "RegRemap", R"doc(
This class inherits from :class:`BaseRegEdit`

Bulk-renames the register keys for some :class:`IfContentPart`

Parameters
----------
keyRemap: Dict[:class:`str`, Union[List[Union[:class:`str`, :class:`CppRemappedKeyData`]], :class:`CppKeyRemapData`]]
    The old key -> remap rules mapping to apply :raw-html:`<br />` :raw-html:`<br />`

    See :meth:`IfContentPart.remapKeys` for the full semantics of how a rule set is evaluated
    for a given key's occurrences
    )doc");

    cls.def(py::init([](py::object keyRemap) {
        // parseIniKeyRemap (PyIOrderedMultiMap.h) is the helper IfContentPart.remapKeys' own binding uses
        return std::make_unique<PyRegRemap>(parseIniKeyRemap(py::cast<py::dict>(keyRemap)));
    }), py::arg("keyRemap"));

    cls.def_property("keyRemap", [](const PyRegRemap &self) {
        return keyRemapToPy(self);
    }, [](PyRegRemap &self, py::object keyRemap) {
        self.keyRemap = parseIniKeyRemap(py::cast<py::dict>(keyRemap));
    }, py::doc(R"doc(
Dict[:class:`str`, Union[List[Union[:class:`str`, :class:`CppRemappedKeyData`]], :class:`CppKeyRemapData`]]:
The old key -> remap rules mapping to apply
    )doc"));

    bindRegEditEdit<PyRegRemap>(cls, R"doc(
Applies :attr:`keyRemap` to 'part', by forwarding straight to :meth:`IfContentPart.remapKeys`

Parameters
----------
part: :class:`IfContentPart`
    The part of the `IfTemplate` that is being editted

sectionName: :class:`str`
    The name of the `section`_ that is being editted. Unused by this edit

modType: Optional[:class:`ModType`]
    The type of mod to fix. Unused by this edit

modName: :class:`str`
    The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

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
