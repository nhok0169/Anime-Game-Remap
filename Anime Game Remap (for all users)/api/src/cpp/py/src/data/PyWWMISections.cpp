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

#include "AGRemapCore/data/IniFixData/WWMISections.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppWWMISections(pybind11::module_ &m) {
    using Sections = AGRC::WWMISections;

    auto cls = py::class_<Sections>(m, "WWMISections", R"doc(
The sections a fix of a Wuthering Waves mod writes to drive WWMI, the mod loader in the game: the merged skeleton a mod
from before WWMI's merged skeleton has none of, a remapped copy of it for a target past 256 bones, one merge list per
target draw slot, the blend remap that runs WWMI's ``BlendRemapper``, and the section of a target draw slot nothing of
the mod is drawn through :raw-html:`<br />` :raw-html:`<br />`

Each builder fills a fixed template with the values it is given. Every name goes through :attr:`Skeleton.name` /
:attr:`BlendRemap.name`, so the sections carry the fix's own names and an undo removes them)doc");

    py::class_<Sections::Skeleton>(cls, "Skeleton", "What :meth:`WWMISections.skeletonSections` writes")
        .def(py::init<>())
        .def_readwrite("name", &Sections::Skeleton::name, "Callable[[:class:`str`], :class:`str`]: The fix's name for a section of the given name")
        .def_readwrite("cb4Hash", &Sections::Skeleton::cb4Hash,
                       ":class:`str`: The hash of the game's bone-data constant buffer, which the merges are gated on")
        .def_readwrite("boneDataFilter", &Sections::Skeleton::boneDataFilter,
                       ":class:`str`: The ``filter_index`` the fix marks that buffer with")
        .def_readwrite("mergedSkeletonSlots", &Sections::Skeleton::mergedSkeletonSlots,
                       ":class:`int`: How many bones the merged skeleton holds")
        .def_readwrite("remapped", &Sections::Skeleton::remapped,
                       ":class:`bool`: Whether the target is past 256 bones: the merge then feeds a private REMAPPED skeleton "
                       "(:attr:`blendRemapForward`, WWMI's ``SkeletonRemapper``), bound in place of the merged one")
        .def_readwrite("latched", &Sections::Skeleton::latched,
                       ":class:`bool`: Whether the remap runs once a frame from a latch elsewhere; otherwise each merge list runs it first")
        .def_readwrite("currentPoseInCb3Only", &Sections::Skeleton::currentPoseInCb3Only,
                       ":class:`bool`: Whether a draw with its current pose in ``vs-cb3`` alone merges into the MAIN skeleton")
        .def_readwrite("bindPrevPoseAlways", &Sections::Skeleton::bindPrevPoseAlways,
                       ":class:`bool`: Whether ``vs-cb3`` gets the previous pose after every merge, whatever the marker says")
        .def_readwrite("blendRemapBones", &Sections::Skeleton::blendRemapBones,
                       ":class:`int`: The blend remap's bone count, for the remap")
        .def_readwrite("blendRemapForward", &Sections::Skeleton::blendRemapForward,
                       ":class:`str`: The resource of the blend remap's forward ids, for the remap")
        .def_readwrite("slotWindows", &Sections::Skeleton::slotWindows,
                       "List[Tuple[:class:`str`, :class:`str`]]: Each target draw slot's bone window, as ``(vg_offset, vg_count)``");

    py::class_<Sections::BlendRemap>(cls, "BlendRemap", "What :meth:`WWMISections.blendRemapSections` writes")
        .def(py::init<>())
        .def_readwrite("name", &Sections::BlendRemap::name, "Callable[[:class:`str`], :class:`str`]: The fix's name for a section of the given name")
        .def_readwrite("blendFile", &Sections::BlendRemap::blendFile, ":class:`str`: The remapped blend file")
        .def_readwrite("remapFile", &Sections::BlendRemap::remapFile,
                       "Callable[[:class:`str`], :class:`str`]: The files of the remap's three id buffers: ``VertexVG``, ``Forward`` and ``Reverse``")
        .def_readwrite("blendStride", &Sections::BlendRemap::blendStride, ":class:`int`: The blend's stride")
        .def_readwrite("influences", &Sections::BlendRemap::influences, ":class:`int`: How many influences each vertex of the blend carries")
        .def_readwrite("blendReg", &Sections::BlendRemap::blendReg, ":class:`str`: The register the remapped blend is bound at");

    py::class_<Sections::Hide>(cls, "Hide", "What :meth:`WWMISections.hideSection` writes")
        .def(py::init<>())
        .def_readwrite("name", &Sections::Hide::name, ":class:`str`: The section's name")
        .def_readwrite("comment", &Sections::Hide::comment, ":class:`str`: The comment written above it")
        .def_readwrite("vb0Hash", &Sections::Hide::vb0Hash, ":class:`str`: The target's ``vb0`` hash")
        .def_readwrite("indexOffset", &Sections::Hide::indexOffset, ":class:`str`: The slot's ``match_first_index``")
        .def_readwrite("indexCount", &Sections::Hide::indexCount, ":class:`str`: The slot's ``match_index_count``")
        .def_readwrite("vgOffset", &Sections::Hide::vgOffset, ":class:`str`: The slot's ``vg_offset``")
        .def_readwrite("vgCount", &Sections::Hide::vgCount, ":class:`str`: The slot's ``vg_count``")
        .def_readwrite("slot", &Sections::Hide::slot, ":class:`str`: A name of the slot for its frame latch (``$state_id_<slot>``)")
        .def_readwrite("legacyMergeList", &Sections::Hide::legacyMergeList,
                       ":class:`str`: For a mod from before WWMI's merged skeleton: the list that merges the slot's bones "
                       "(:meth:`WWMISections.mergeListName`), run before the draw is skipped -- or empty for a mod with a merged skeleton")
        .def_readwrite("mergeWindowList", &Sections::Hide::mergeWindowList,
                       ":class:`str`: For a target past 256 bones: the list that merges the slot's window into the fix's own skeleton, "
                       "run on every draw -- or empty, which runs :attr:`mergeSkeletonList` once a frame through the slot's latch")
        .def_readwrite("mergeSkeletonList", &Sections::Hide::mergeSkeletonList,
                       ":class:`str`: The list run once a frame without :attr:`mergeWindowList`")
        .def_readwrite("stashed", &Sections::Hide::stashed,
                       "List[Tuple[:class:`str`, :class:`str`]]: Lines that capture the game's own bindings on the slot, run while a pixel shader is bound");

    cls.def_static("mergeListName", &Sections::mergeListName, py::arg("skeleton"), py::arg("slot"), py::doc(R"doc(
The merge list of one target draw slot

Parameters
----------
skeleton: :class:`WWMISections.Skeleton`
    What the sections are named by

slot: :class:`int`
    The slot

Returns
-------
:class:`str`
    The list's name)doc"))
       .def_static("skeletonSections", &Sections::skeletonSections, py::arg("z3"), py::arg("skeleton"), py::doc(R"doc(
The skeleton's sections: the merged skeleton and its read-write copies, the marker on the game's bone-data constant
buffer, the remapped skeleton with the list that remaps it and the merge a hidden slot runs (for
:attr:`Skeleton.remapped`), and one merge list per slot (:meth:`mergeListName`)

Parameters
----------
z3: :class:`Z3Context`
    The Z3 context the ``if`` lines are parsed in

skeleton: :class:`WWMISections.Skeleton`
    What to write

Returns
-------
List[:class:`IfTemplate`]
    The sections, in order)doc"))
       .def_static("blendRemapInitList", &Sections::blendRemapInitList, py::arg("blendRemap"), py::doc(R"doc(
The list :meth:`blendRemapSections` runs WWMI's ``BlendRemapper`` from

Parameters
----------
blendRemap: :class:`WWMISections.BlendRemap`
    What the sections are named by

Returns
-------
:class:`str`
    The list's name)doc"))
       .def_static("blendRemapSections", &Sections::blendRemapSections, py::arg("z3"), py::arg("blendRemap"), py::doc(R"doc(
The blend remap's resources and the one-time run of WWMI's ``BlendRemapper`` (:meth:`blendRemapInitList`), guarded by
a ``local`` so it costs one dispatch on the first drawn frame and binds the remapped blend after

Parameters
----------
z3: :class:`Z3Context`
    The Z3 context the ``if`` lines are parsed in

blendRemap: :class:`WWMISections.BlendRemap`
    What to write

Returns
-------
List[:class:`IfTemplate`]
    The sections, in order)doc"))
       .def_static("hideSection", &Sections::hideSection, py::arg("z3"), py::arg("hide"), py::doc(R"doc(
The ``TextureOverride`` of a target draw slot nothing of the mod is drawn through: it skips the skin's own geometry and
still merges the slot's bones, which other slots' vertices may be weighted to

Parameters
----------
z3: :class:`Z3Context`
    The Z3 context the ``if`` lines are parsed in

hide: :class:`WWMISections.Hide`
    What to write

Returns
-------
:class:`IfTemplate`
    The section)doc"));
}
