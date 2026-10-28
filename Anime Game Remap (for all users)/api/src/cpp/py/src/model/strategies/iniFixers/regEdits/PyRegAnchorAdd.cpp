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

#include "PyRegAnchorAdd.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/stl.h>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyRegAnchorAdd::PyRegAnchorAdd(std::vector<std::string> anchorKeys, py::object adderObj, Core::Placement placement,
                               py::object anchorIfObj, bool firstOnly, bool skipExisting):
    Core(std::move(anchorKeys), nullptr, placement, nullptr, firstOnly, skipExisting), adderObj(std::move(adderObj)), anchorIfObj(std::move(anchorIfObj)) {}


void PyRegAnchorAdd::refresh(const py::object &modType) {
    (void)modType;

    if (anchorIfObj.is_none()) {
        anchorIf = {};
    } else {
        py::object filter = anchorIfObj;
        anchorIf = [filter](const std::string &key) -> bool {
            return py::bool_(filter(key));
        };
    }

    if (adderObj.is_none()) {
        adder = {};
        return;
    }

    py::object held = adderObj;
    adder = [held](const std::string &key, const std::string &value) -> std::optional<std::vector<std::pair<std::string, std::string>>> {
        py::object result = held(key, value);
        if (result.is_none()) {
            return std::nullopt;
        }
        return result.cast<std::vector<std::pair<std::string, std::string>>>();
    };
}


void initCppRegAnchorAdd(pybind11::module_ &m) {
    py::class_<PyRegAnchorAdd, PyBaseRegEdit, py::smart_holder> cls(m, "RegAnchorAdd", R"doc(
This class inherits from :class:`BaseRegEdit`

Adds lines next to every line of a part whose key is one of the :attr:`anchorKeys`, or replaces that line

For each such line (an *anchor*), :attr:`adder` is handed the anchor's key and value and answers with the
lines to add --- or ``None`` to leave that anchor alone. Where the lines go is :attr:`placement`:
immediately before the anchor, immediately after it, or in its place. The lines keep the order
:attr:`adder` gives them, and the anchors are visited from the last to the first, so every anchor of the
part is visited exactly once however many lines are added.

.. code-block:: ini

    ; anchorKeys = ["drawindexed"], placement = RegAnchorAdd.Placement.After,
    ; adder: "6, 0, 0" -> [("run", "CommandListMark")], anything else -> None
    drawindexed = 6, 0, 0
    run = CommandListMark       ; added
    drawindexed = 3, 6, 0       ; left alone

A key matches an anchor key case-insensitively, ignoring surrounding whitespace, as 3DMigoto reads keys;
:attr:`anchorIf`, when set, decides instead. Only the lines whose order index is inside 'partRanges' are anchors

Parameters
----------
anchorKeys: List[:class:`str`]
    The keys whose lines are anchors :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``[]``

adder: Optional[Callable[[:class:`str`, :class:`str`], Optional[List[Tuple[:class:`str`, :class:`str`]]]]]
    Takes an anchor's key and value and returns the lines to add for it, as ``(key, value)`` pairs, or
    ``None`` to leave the anchor alone. An empty list with :attr:`Placement.Replace` removes the anchor.
    ``None`` leaves every anchor alone :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

placement: :class:`RegAnchorAdd.Placement`
    Where the added lines go :raw-html:`<br />` :raw-html:`<br />`

    **Default**: :attr:`RegAnchorAdd.Placement.After`

anchorIf: Optional[Callable[[:class:`str`], :class:`bool`]]
    Takes a line's key and decides whether that line is an anchor, in place of 'anchorKeys' --- eg. every
    key starting with ``ps-t``. ``None`` leaves the choice to 'anchorKeys' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

firstOnly: :class:`bool`
    Whether only the part's FIRST anchor --- the one with the lowest order index inside 'partRanges' --- gets
    lines, eg. to capture a register just before the line that first rebinds it :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

skipExisting: :class:`bool`
    Whether a line the part already holds --- the same key and the same value, both compared case-insensitively
    and ignoring surrounding whitespace --- is left out, so a part visited twice gets its lines once :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc");

    py::enum_<PyRegAnchorAdd::Core::Placement>(cls, "Placement", R"doc(
Where a :class:`RegAnchorAdd`'s lines go, relative to their anchor
    )doc")
        .value("Before", PyRegAnchorAdd::Core::Placement::Before, "Immediately before the anchor")
        .value("After", PyRegAnchorAdd::Core::Placement::After, "Immediately after the anchor")
        .value("Replace", PyRegAnchorAdd::Core::Placement::Replace, "In the anchor's place: the anchor is removed");

    cls.def(py::init([](std::vector<std::string> anchorKeys, py::object adder, PyRegAnchorAdd::Core::Placement placement,
                        py::object anchorIf, bool firstOnly, bool skipExisting) {
        return std::make_unique<PyRegAnchorAdd>(std::move(anchorKeys), std::move(adder), placement, std::move(anchorIf),
                                                firstOnly, skipExisting);
    }), py::arg("anchorKeys") = std::vector<std::string>{}, py::arg("adder") = py::none(),
        py::arg("placement") = PyRegAnchorAdd::Core::Placement::After, py::arg("anchorIf") = py::none(),
        py::arg("firstOnly") = false, py::arg("skipExisting") = false);

    cls.def_readwrite("anchorKeys", &PyRegAnchorAdd::anchorKeys, R"doc(
List[:class:`str`]: The keys whose lines are anchors, matched case-insensitively and ignoring surrounding whitespace
    )doc");

    cls.def_property("adder", [](const PyRegAnchorAdd &self) {
        return self.adderObj;
    }, [](PyRegAnchorAdd &self, py::object adder) {
        self.adderObj = std::move(adder);
    }, py::doc(R"doc(
Optional[Callable[[:class:`str`, :class:`str`], Optional[List[Tuple[:class:`str`, :class:`str`]]]]]: The lines each anchor gets, or ``None`` to leave every anchor alone
    )doc"));

    cls.def_property("anchorIf", [](const PyRegAnchorAdd &self) {
        return self.anchorIfObj;
    }, [](PyRegAnchorAdd &self, py::object anchorIf) {
        self.anchorIfObj = std::move(anchorIf);
    }, py::doc(R"doc(
Optional[Callable[[:class:`str`], :class:`bool`]]: Which keys make a line an anchor, in place of :attr:`anchorKeys`, or ``None`` to leave the choice to :attr:`anchorKeys`
    )doc"));

    cls.def_readwrite("placement", &PyRegAnchorAdd::placement, R"doc(
:class:`RegAnchorAdd.Placement`: Where the added lines go
    )doc");

    cls.def_readwrite("firstOnly", &PyRegAnchorAdd::firstOnly, R"doc(
:class:`bool`: Whether only the part's first anchor inside 'partRanges' gets lines
    )doc");

    cls.def_readwrite("skipExisting", &PyRegAnchorAdd::skipExisting, R"doc(
:class:`bool`: Whether a line the part already holds (same key and value, compared case-insensitively and ignoring surrounding whitespace) is left out
    )doc");

    cls.def_static("holds", [](py::object part, const std::string &key, const std::string &value) {
        return PyRegAnchorAdd::Core::holds(py::cast<PyIfContentPart &>(part), key, value);
    }, py::arg("part"), py::arg("key"), py::arg("value"), py::doc(R"doc(
Whether 'part' holds a line of 'key' and 'value', both compared case-insensitively and ignoring surrounding whitespace

Parameters
----------
part: :class:`IfContentPart`
    The part to search

key: :class:`str`
    The key of the line

value: :class:`str`
    The value of the line

Returns
-------
:class:`bool`
    Whether 'part' holds it
    )doc"));

    bindRegEditEdit<PyRegAnchorAdd>(cls, R"doc(
Adds :attr:`adder`'s lines at every anchor of 'part' inside 'partRanges', from the last anchor to the first

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
