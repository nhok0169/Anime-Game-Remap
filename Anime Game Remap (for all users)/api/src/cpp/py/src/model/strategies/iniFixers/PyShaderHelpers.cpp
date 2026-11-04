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

#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/iniFixers/CustomShaderBackups.h"
#include "AGRemapCore/model/strategies/iniFixers/ShaderFilterTags.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppShaderFilterTags(pybind11::module_ &m) {
    using Tags = AGRC::ShaderFilterTags;

    auto cls = py::class_<Tags>(m, "ShaderFilterTags", R"doc(
The shaders a fix tells apart by a ``filter_index``, and what it writes for them

3DMigoto sets a shader's ``filter_index`` from a ``[ShaderOverride...]`` `section`_ on its hash, and a section of the
mod then asks ``if ps == <index>`` (or ``vs``) to act on one pass of a draw only. This holds the shaders a fix tags, in
the order they were tagged, with their values; writes the ``[ShaderOverride...]`` sections (:meth:`section`); and
writes the condition true on a draw by any of some shaders, or by none of them (:meth:`condition`)

Which value each shader gets is the fixer's choice. 3DMigoto keys a ``[ShaderOverride]`` by its shader hash across
EVERY loaded .ini, so two fixes that tag the same shader must give it the same value

:example:

.. code-block:: python

    import FixRaidenBoss2 as FRB

    tags = FRB.ShaderFilterTags()
    tags.add("a24c103adbe204f1", "51840.1")
    tags.add("a9f95d035488717a", "51840.2")
    tags.condition("ps", ["a24c103adbe204f1", "a9f95d035488717a"])   # 'ps == 51840.1 || ps == 51840.2'
    section = tags.section(0, "ShaderOverrideMain")                  # hash = a24c103adbe204f1 / filter_index = 51840.1
    )doc");

    py::class_<Tags::Tag>(cls, "Tag", R"doc(One tagged shader)doc")
        .def_readonly("shaderHash", &Tags::Tag::shaderHash, py::doc(R"doc(:class:`str`: The shader's hash)doc"))
        .def_readonly("filterIndex", &Tags::Tag::filterIndex, py::doc(R"doc(:class:`str`: The ``filter_index`` it is given)doc"));

    cls.def(py::init<>())
       .def("addNumbered", &Tags::addNumbered, py::arg("shaderHashes"), py::arg("given"), py::arg("base"), py::arg("step"),
            py::doc(R"doc(
Tags each of ``shaderHashes`` not tagged yet, in order: with its value in ``given`` when it has one, else with the next of
a numbered run -- ``base``, ``base + step``, ``base + 2 step``, ... -- which only the shaders without a given value advance

Parameters
----------
shaderHashes: List[:class:`str`]
    The shaders, in the order they are tagged

given: Dict[:class:`str`, :class:`str`]
    The values some of them must have -- one another fix also tags, which has to agree

base: :class:`float`
    The run's first value

step: :class:`float`
    The run's step
)doc"))
       .def("add", &Tags::add, py::arg("shaderHash"), py::arg("filterIndex"), py::doc(R"doc(
Tags a shader, unless it is tagged already (its first value stays)

Parameters
----------
shaderHash: :class:`str`
    The shader's hash

filterIndex: :class:`str`
    The ``filter_index`` it is given

Returns
-------
:class:`bool`
    Whether the shader was tagged by this call
        )doc"))
       .def("filterIndexOf", &Tags::filterIndexOf, py::arg("shaderHash"), py::doc(R"doc(
The ``filter_index`` of a shader

Parameters
----------
shaderHash: :class:`str`
    The shader's hash

Returns
-------
Optional[:class:`str`]
    The value, or ``None`` when the shader is not tagged
        )doc"))
       .def("tags", &Tags::tags, py::return_value_policy::reference_internal, py::doc(R"doc(
The tagged shaders, in the order they were tagged

Returns
-------
List[:class:`ShaderFilterTags.Tag`]
    The tags
        )doc"))
       .def("condition", &Tags::condition, py::arg("reg"), py::arg("shaderHashes"), py::arg("none") = false, py::doc(R"doc(
The .ini condition true on a draw by any of 'shaderHashes' -- ``<reg> == <index> || ...`` -- or, with 'none', by none of
them -- ``<reg> != <index> && ...``. Each shader is named once, in the order given; a shader that is not tagged is left
out

Parameters
----------
reg: :class:`str`
    The register a shader's ``filter_index`` is read from: ``ps`` or ``vs``

shaderHashes: List[:class:`str`]
    The shaders' hashes

none: :class:`bool`
    Whether the condition is true on a draw by none of the shaders :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
:class:`str`
    The condition, or empty when no shader of 'shaderHashes' is tagged
        )doc"))
       .def("section", &Tags::section, py::arg("index"), py::arg("name"), py::doc(R"doc(
The ``[ShaderOverride...]`` `section`_ that tags shader number 'index' (:meth:`tags`): ``hash`` and ``filter_index``

Parameters
----------
index: :class:`int`
    The shader's position in :meth:`tags`

name: :class:`str`
    The section's name

Returns
-------
:class:`IfTemplate`
    The section
        )doc"));
}


void initCppCustomShaderBackups(pybind11::module_ &m) {
    using Backups = AGRC::CustomShaderBackups;

    py::class_<Backups>(m, "CustomShaderBackups", R"doc(
Saves the resource registers a ``[CustomShader...]`` assigns before it runs, and puts them back after

3DMigoto restores a CustomShader's shaders and render state when it returns, but NOT the resources its section assigns.
A later draw of the same game draw -- another part on the same slot, drawn by the game's own shader -- then reads them:
a body shaded with another character's material constants renders black. :meth:`wrap` puts a save line
(``<backup> = ref <reg>``) before the ``run =`` lines of a CustomShader for each register its lines assign, and a
restore line (``<reg> = ref <backup>``) after them. Each backup is a resource `section`_ of its own, which
:meth:`section` writes

Parameters
----------
backupName: Callable[[:class:`str`], :class:`str`]
    The name of the resource `section`_ that backs up a register

Examples
--------

.. code-block:: python

    import FixRaidenBoss2 as FRB

    backups = FRB.CustomShaderBackups(lambda reg: "ResourceBackup" + reg.replace("-", ""))
    backups.wrap([("run", "CustomShaderMine")], [("ps-t4", "ResourceMask"), ("drawindexed", "auto")])
    # [('ResourceBackuppst4', 'ref ps-t4'), ('run', 'CustomShaderMine'), ('ps-t4', 'ref ResourceBackuppst4')]
    sections = [backups.section(reg) for reg in backups.registers()]
    )doc")
        .def(py::init<Backups::BackupName>(), py::arg("backupName"))
        .def_readwrite("backupName", &Backups::backupName,
                       py::doc(R"doc(Callable[[:class:`str`], :class:`str`]: The name of the resource `section`_ that backs up a register)doc"))
        .def_static("bindsRegister", &Backups::bindsRegister, py::arg("key"), py::doc(R"doc(
Whether a key binds a resource register: ``ps-t<n>``, ``ps-cb<n>``, ``vs-t<n>`` or ``vs-cb<n>``

Parameters
----------
key: :class:`str`
    The key

Returns
-------
:class:`bool`
    Whether it binds a resource register
        )doc"))
        .def("wrap", &Backups::wrap, py::arg("runs"), py::arg("lines"), py::doc(R"doc(
'runs' with a save line for each register 'lines' assign before them, and a restore line for each after them, the
registers in their sorted order. The registers are remembered for :meth:`registers`

Parameters
----------
runs: List[Tuple[:class:`str`, :class:`str`]]
    The ``run =`` lines of the CustomShaders

lines: List[Tuple[:class:`str`, :class:`str`]]
    The lines the CustomShaders' sections write

Returns
-------
List[Tuple[:class:`str`, :class:`str`]]
    The lines, saving and restoring
        )doc"))
        .def("registers", &Backups::registers, py::doc(R"doc(
Every register :meth:`wrap` has saved, in their sorted order

Returns
-------
List[:class:`str`]
    The registers
        )doc"))
        .def("section", &Backups::section, py::arg("reg"), py::doc(R"doc(
The resource `section`_ that backs up 'reg': empty, named by :attr:`backupName`

Parameters
----------
reg: :class:`str`
    The register

Returns
-------
:class:`IfTemplate`
    The section
        )doc"))
        .def("clear", &Backups::clear, py::doc(R"doc(Forgets every register saved so far)doc"));
}
