#include "PyRegRemap.h"

#include <memory>
#include <utility>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyRegRemap::PyRegRemap(py::object keyRemapObj): Core({}), keyRemapObj(std::move(keyRemapObj)) {}

void PyRegRemap::refresh(const py::object &modType) {
    (void)modType;

    // parseKeyRemap (PyIOrderedMultiMap.h) is the exact same helper IfContentPart.remapKeys' own
    // binding uses, so a CppKeyRemapData/CppRemappedKeyData value works here identically.
    keyRemap = parseIniKeyRemap(py::cast<py::dict>(keyRemapObj));
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
        return std::make_unique<PyRegRemap>(std::move(keyRemap));
    }), py::arg("keyRemap"));

    cls.def_property("keyRemap", [](const PyRegRemap &self) {
        return self.keyRemapObj;
    }, [](PyRegRemap &self, py::object keyRemap) {
        self.keyRemapObj = std::move(keyRemap);
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
