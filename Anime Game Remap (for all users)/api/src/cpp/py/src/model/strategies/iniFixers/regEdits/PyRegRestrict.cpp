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

#include "PyRegRestrict.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {
    struct PyKeyFilter {
        py::object filter;
        bool operator()(const std::string &key) const {
            py::gil_scoped_acquire gil;
            return py::cast<bool>(filter(key));
        }
    };

    std::optional<std::vector<std::string>> allowedKeysOf(const py::object &allowedKeys) {
        if (allowedKeys.is_none()) {
            return std::nullopt;
        }
        std::vector<std::string> parsed;
        for (auto key : allowedKeys) {
            parsed.push_back(py::str(key).cast<std::string>());
        }
        return parsed;
    }

    PyRegRestrict::KeyFilter keyFilterOf(const py::object &keyFilter) {
        return keyFilter.is_none() ? PyRegRestrict::KeyFilter() : PyRegRestrict::KeyFilter(PyKeyFilter{keyFilter});
    }
}


void initCppRegRestrict(pybind11::module_ &m) {
    py::class_<PyRegRestrict, PyBaseRegEdit, py::smart_holder> cls(m, "RegRestrict", R"doc(
This class inherits from :class:`BaseRegEdit`

Restricts the registers an :class:`IfContentPart` binds to the ones its TARGET reads, and to one
binding each

A remapped `section`_ inherits its registers from the MOD's `section`_, and a mod binds whatever its
SOURCE reads. Against a different target that surplus is not harmless: the target's shader reads
the extra register as something else, where the target's own mod leaves it unbound so the GAME's
texture serves it. And a register bound twice keeps only the LAST binding, silently --- so a fix
that moves one texture onto a register the mod already fills loses the texture it moved.

.. code-block:: ini

    ; allowedKeys = ["ps-t0", "ps-t1", "ps-t2"], keyFilter = a ps-t register
    ps-t1 = ResourceDiffuse
    ps-t0 = ResourceNormalMap
    ps-t2 = ResourceLightMap
    ps-t2 = ResourceMetalMap       ; removed: ps-t2 is already bound above
    ps-t3 = ResourceShadowRamp     ; removed: the target does not read ps-t3
    ib = ResourceIb                ; untouched: not a key this edit governs

.. note::
    The unit is ONE part, not a `section`_. A merged mod binds ``ps-t2`` once in each branch of an
    ``if`` / ``else if`` chain, and each branch is its own part: those are separate paths through
    the `section`_, not one register bound twice

Parameters
----------
allowedKeys: Optional[List[:class:`str`]]
    The governed keys a part may keep, or ``None`` to keep every governed key and only remove
    repeated bindings :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

keyFilter: Optional[Callable[[:class:`str`], :class:`bool`]]
    Which keys this edit governs, or ``None`` for every key :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

keepFirstOnly: :class:`bool`
    Whether a governed key bound more than once in a part keeps only its FIRST binding :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc");

    cls.def(py::init([](py::object allowedKeys, py::object keyFilter, bool keepFirstOnly) {
        return std::make_unique<PyRegRestrict>(allowedKeysOf(allowedKeys), keyFilterOf(keyFilter), keepFirstOnly);
    }), py::arg("allowedKeys") = py::none(), py::arg("keyFilter") = py::none(), py::arg("keepFirstOnly") = true);

    cls.def_property("allowedKeys", [](const PyRegRestrict &self) {
        return self.allowedKeys.has_value() ? py::object(py::cast(*self.allowedKeys)) : py::none();
    }, [](PyRegRestrict &self, py::object allowedKeys) {
        self.allowedKeys = allowedKeysOf(allowedKeys);
    }, py::doc(R"doc(
Optional[List[:class:`str`]]: The governed keys a part may keep, or ``None`` for all of them
    )doc"));

    cls.def_property("keyFilter", [](const PyRegRestrict &self) {
        const PyKeyFilter *own = self.keyFilter ? self.keyFilter.target<PyKeyFilter>() : nullptr;
        return own != nullptr ? own->filter : (self.keyFilter ? py::cast(self.keyFilter) : py::none());
    }, [](PyRegRestrict &self, py::object keyFilter) {
        self.keyFilter = keyFilterOf(keyFilter);
    }, py::doc(R"doc(
Optional[Callable[[:class:`str`], :class:`bool`]]: Which keys this edit governs, or ``None`` for every key
    )doc"));

    cls.def_readwrite("keepFirstOnly", &PyRegRestrict::keepFirstOnly, R"doc(
:class:`bool`: Whether a governed key bound more than once in a part keeps only its FIRST binding
    )doc");

    bindRegEditEdit<PyRegRestrict>(cls, R"doc(
Removes every governed key of 'part' that is not in :attr:`allowedKeys`, and --- with
:attr:`keepFirstOnly` --- every binding of a governed key after its first

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
