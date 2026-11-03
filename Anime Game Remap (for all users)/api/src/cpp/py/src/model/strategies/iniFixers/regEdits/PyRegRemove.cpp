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

#include "PyRegRemove.h"

#include <memory>
#include <string>
#include <utility>

#include <pybind11/functional.h>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyRegRemove::PyRegRemove(py::object removeKeysObj): Core({}), removeKeysObj(std::move(removeKeysObj)) {}

void PyRegRemove::refresh(const py::object &modType) {
    (void)modType;

    // parseRemoveKeys (PyIfContentPart.h) is the exact same helper IfContentPart.removeKeys' own
    // binding uses.
    removeKeys = parseRemoveKeys(removeKeysObj);
}


void initCppRegRemove(pybind11::module_ &m) {
    py::class_<PyRegRemove, PyBaseRegEdit, py::smart_holder> cls(m, "RegRemove", R"doc(
This class inherits from :class:`BaseRegEdit`

Bulk-removes register keys for some :class:`IfContentPart`

Parameters
----------
removeKeys: Union[Dict[Any, Optional[Callable[[:class:`int`, Any], :class:`bool`]]], List[Tuple[Any, Optional[Callable[[:class:`int`, Any], :class:`bool`]]]]]
    Each key to remove, mapped to its own optional check predicate :raw-html:`<br />` :raw-html:`<br />`

    See :meth:`IfContentPart.removeKeys` for the full semantics of how the predicates decide
    which occurrences of a key actually get removed
    )doc");

    cls.def(py::init([](py::object removeKeys) {
        return std::make_unique<PyRegRemove>(std::move(removeKeys));
    }), py::arg("removeKeys"));

    cls.def_static("valueStartsWith", [](std::string prefix) {
        return AGRC::RegRemove<>::valueStartsWith(std::move(prefix));
    }, py::arg("prefix"), py::doc(R"doc(
A check accepting a line whose value starts with 'prefix', ignoring case and leading space -- or whose value is ``ref``
followed by such a value, since a binding may name its resource either way (``vs-cb4 = ResourceMergedSkeleton`` and
``vs-cb4 = ref ResourceMergedSkeleton``). A 'prefix' of ``ref`` itself accepts every ``ref`` binding

Parameters
----------
prefix: :class:`str`
    The start of the value, in any case

Returns
-------
Callable[[:class:`int`, :class:`str`], :class:`bool`]
    The check, for :attr:`removeKeys`
    )doc"));

    cls.def_property("removeKeys", [](const PyRegRemove &self) {
        return self.removeKeysObj;
    }, [](PyRegRemove &self, py::object removeKeys) {
        self.removeKeysObj = std::move(removeKeys);
    }, py::doc(R"doc(
Dict[Any, Optional[Callable[[:class:`int`, Any], :class:`bool`]]]: Each key to remove, mapped to
its own optional check predicate
    )doc"));

    bindRegEditEdit<PyRegRemove>(cls, R"doc(
Removes every key in :attr:`removeKeys` from 'part', by forwarding straight to
:meth:`IfContentPart.removeKeys`

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
