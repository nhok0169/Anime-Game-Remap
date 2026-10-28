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

#include "PyRegDrawRangeShift.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/stl.h>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void PyRegDrawRangeShift::refresh(const py::object &modType) {
    (void)modType;
}


void initCppRegDrawRangeShift(pybind11::module_ &m) {
    py::class_<PyRegDrawRangeShift, PyBaseRegEdit, py::smart_holder> cls(m, "RegDrawRangeShift", R"doc(
This class inherits from :class:`BaseRegEdit`

Moves the ranges a part's ``drawindexed`` lines draw by a fixed number of indices, for an index buffer that
now sits behind other indices

When a fix concatenates several index buffers into one, every buffer after the first starts :attr:`offset`
indices in, and the ``drawindexed = <count>, <start>, <base>`` lines written for it address the start of the
merged buffer instead of its own part of it. This edit adds :attr:`offset` to each line's start and keeps
every other field as written; ``drawindexed = auto`` becomes the buffer's whole range,
``<count>, <offset>, 0``. A line whose start is not a plain non-negative number is left as it is.

.. code-block:: ini

    ; offset = 30, count = 12
    drawindexed = 6, 0, 0       ; -> 6, 30, 0
    drawindexed = 6, 6          ; -> 6, 36
    drawindexed = auto          ; -> 12, 30, 0
    drawindexed = $n, $s, 0     ; untouched

Some draws take their start from a variable the part sets (eg. a library's command list drawing
``drawindexed = $_1, $_2, 0``): every key of :attr:`startKeys` --- matched case-insensitively, ignoring
surrounding whitespace --- holds such a start, and its plain non-negative values are moved by :attr:`offset` too

Parameters
----------
offset: :class:`int`
    How many indices the ranges are moved by :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``0``

count: :class:`int`
    How many indices the buffer holds, which is what ``drawindexed = auto`` becomes :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``0``

startKeys: List[:class:`str`]
    Further keys whose values are start indices into the same buffer :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``[]``
    )doc");

    cls.def(py::init([](long long offset, long long count, std::vector<std::string> startKeys) {
        return std::make_unique<PyRegDrawRangeShift>(offset, count, std::move(startKeys));
    }), py::arg("offset") = 0, py::arg("count") = 0, py::arg("startKeys") = std::vector<std::string>{});

    cls.def_readwrite("offset", &PyRegDrawRangeShift::offset, R"doc(
:class:`int`: How many indices the ranges are moved by
    )doc");

    cls.def_readwrite("count", &PyRegDrawRangeShift::count, R"doc(
:class:`int`: How many indices the buffer holds, which is what ``drawindexed = auto`` becomes
    )doc");

    cls.def_readwrite("startKeys", &PyRegDrawRangeShift::startKeys, R"doc(
List[:class:`str`]: Further keys whose values are start indices into the same buffer, matched case-insensitively and ignoring surrounding whitespace
    )doc");

    cls.def_static("shiftRange", &PyRegDrawRangeShift::Core::shiftRange, py::arg("value"), py::arg("offset"), py::arg("count"), py::doc(R"doc(
The value of a ``drawindexed`` line moved by 'offset': ``auto`` becomes ``<count>, <offset>, 0``, a line
whose second field is a plain non-negative number has 'offset' added to it (every field then written
stripped, separated by ``", "``), and anything else is returned as it is

Parameters
----------
value: :class:`str`
    The value of the ``drawindexed`` line

offset: :class:`int`
    How many indices to move the range by

count: :class:`int`
    How many indices the buffer holds

Returns
-------
:class:`str`
    The moved value
    )doc"));

    bindRegEditEdit<PyRegDrawRangeShift>(cls, R"doc(
Moves every ``drawindexed`` line of 'part' by :attr:`offset`, then every plain non-negative value of the
keys :attr:`startKeys` names

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
