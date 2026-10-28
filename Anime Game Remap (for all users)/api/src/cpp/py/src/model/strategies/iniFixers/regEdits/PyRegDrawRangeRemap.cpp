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

#include "PyRegDrawRangeRemap.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyRegDrawRangeRemap::PyRegDrawRangeRemap(std::vector<std::size_t> keptTriangleIds, py::object rerouteObj):
    Core(std::move(keptTriangleIds)), rerouteObj(std::move(rerouteObj)) {}


void PyRegDrawRangeRemap::refresh(const py::object &modType) {
    (void)modType;

    if (rerouteObj.is_none()) {
        reroute = {};
        return;
    }

    py::object held = rerouteObj;
    reroute = [held](std::size_t firstIndex, std::size_t indexCount, const std::string &remapped) -> std::optional<std::string> {
        py::object result = held(firstIndex, indexCount, remapped);
        if (result.is_none()) {
            return std::nullopt;
        }
        return py::str(result).cast<std::string>();
    };
}


void initCppRegDrawRangeRemap(pybind11::module_ &m) {
    py::class_<PyRegDrawRangeRemap, PyBaseRegEdit, py::smart_holder> cls(m, "RegDrawRangeRemap", R"doc(
This class inherits from :class:`BaseRegEdit`

Remaps the ranges a part's ``drawindexed`` lines draw through an index buffer that lost some of its
triangles

A mod that draws for itself writes ``drawindexed = <count>, <start>, <base>``, and a mod with toggles
writes several such lines, each a range of its object's index buffer that one toggle shows or hides.
When a fix writes that index buffer again without some of its triangles (the ones another component
draws, say), the mod's own numbers overrun the written buffer and every range after a removed triangle
draws the wrong triangles.

:attr:`keptTriangleIds` lists, in ascending order, the SOURCE index of every triangle the written buffer
keeps. A range's new start is the number of kept triangles before it and its new count the number
inside it, both times 3; any further number on the line (the base vertex) is kept as it is. A line that
is not at least two non-negative numbers --- ``drawindexed = auto`` --- is left alone.

.. code-block:: ini

    ; keptTriangleIds = [0, 1, 2, 5, 6, 7]: triangles 3 and 4 were removed
    drawindexed = 9, 0, 0       ; -> 9, 0, 0
    drawindexed = 9, 9, 0       ; -> 3, 9, 0   (only triangle 5 of 3..5 is kept)
    drawindexed = auto          ; untouched

A range may also be drawn some other way: :attr:`reroute` is asked about every remapped line, and when
it answers with a name the line is replaced, at the same position, by ``run = <name>`` --- eg. a
``CustomShader`` `section`_ that draws the remapped range under a blend state of its own.

Parameters
----------
keptTriangleIds: List[:class:`int`]
    The source index of every triangle the written index buffer keeps, ascending :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``[]``

reroute: Optional[Callable[[:class:`int`, :class:`int`, :class:`str`], Optional[:class:`str`]]]
    Decides whether a remapped range is drawn by a ``run =`` instead: takes the range's first index and
    index count as the MOD wrote them, and the remapped value of the line, and returns the name to ``run``,
    or ``None`` to keep the ``drawindexed`` line. ``None`` keeps every line :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](std::vector<std::size_t> keptTriangleIds, py::object reroute) {
        return std::make_unique<PyRegDrawRangeRemap>(std::move(keptTriangleIds), std::move(reroute));
    }), py::arg("keptTriangleIds") = std::vector<std::size_t>{}, py::arg("reroute") = py::none());

    cls.def_readwrite("keptTriangleIds", &PyRegDrawRangeRemap::keptTriangleIds, R"doc(
List[:class:`int`]: The source index of every triangle the written index buffer keeps, ascending
    )doc");

    cls.def_property("reroute", [](const PyRegDrawRangeRemap &self) {
        return self.rerouteObj;
    }, [](PyRegDrawRangeRemap &self, py::object reroute) {
        self.rerouteObj = std::move(reroute);
    }, py::doc(R"doc(
Optional[Callable[[:class:`int`, :class:`int`, :class:`str`], Optional[:class:`str`]]]: Which remapped ranges are drawn by a ``run =`` instead, or ``None`` to keep every line
    )doc"));

    cls.def_static("parseRange", &PyRegDrawRangeRemap::Core::parseRange, py::arg("value"), py::doc(R"doc(
The numbers of a ``drawindexed`` value --- ``count, start[, base, ...]`` --- or ``None`` when it is not
at least two numbers, or its count or start is negative (``auto``)

Parameters
----------
value: :class:`str`
    The value of the ``drawindexed`` line

Returns
-------
Optional[List[:class:`int`]]
    The numbers, in the order they are written
    )doc"));

    cls.def_static("remapRange", &PyRegDrawRangeRemap::Core::remapRange, py::arg("value"), py::arg("keptTriangleIds"), py::doc(R"doc(
The value of a ``drawindexed`` line remapped through 'keptTriangleIds', or ``None`` for one
:meth:`parseRange` cannot read

Parameters
----------
value: :class:`str`
    The value of the ``drawindexed`` line

keptTriangleIds: List[:class:`int`]
    The source index of every kept triangle, ascending

Returns
-------
Optional[:class:`str`]
    The remapped value, written ``count, start[, base, ...]``
    )doc"));

    bindRegEditEdit<PyRegDrawRangeRemap>(cls, R"doc(
Remaps every ``drawindexed`` line of 'part' through :attr:`keptTriangleIds`, then replaces each one
:attr:`reroute` names with a ``run =`` line at the same position

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
