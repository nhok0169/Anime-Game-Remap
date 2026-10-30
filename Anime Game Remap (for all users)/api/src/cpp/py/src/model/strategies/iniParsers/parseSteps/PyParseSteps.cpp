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

#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/iniParsers/parseSteps/BaseIniParseStep.h"
#include "AGRemapCore/model/strategies/iniParsers/parseSteps/DrawRead.h"
#include "AGRemapCore/model/strategies/iniParsers/parseSteps/LibraryRead.h"
#include "AGRemapCore/model/strategies/iniParsers/parseSteps/ResourceRead.h"
#include "AGRemapCore/model/strategies/iniParsers/parseSteps/SkeletonNumberingRead.h"
#include "AGRemapCore/model/strategies/iniParsers/parseSteps/TextureOverrideRead.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;

namespace {
    const char* const ObjFilterDoc = R"doc(Optional[Callable[[Tuple[:class:`str`, :class:`str`]], :class:`bool`]]: Which mod objects are read, by their ``(component, mod object)``; ``None`` reads every one)doc";
}


void initCppParseSteps(pybind11::module_ &m) {
    using Step = AGRC::BaseIniParseStep<>;

    py::class_<Step, AGRC::BaseIniPartEdit, py::smart_holder>(m, "BaseIniParseStep", R"doc(
This class inherits from :class:`BaseIniPartEdit`

Base class for one step of reading a mod: it reads part of the .ini file into the parser's
:attr:`BaseIniParser.parseData`

A parser is a list of steps, the way a fixer is a list of edits: :attr:`GIMIParser.parseSteps` holds them in
the order they run, once the parser has found each mod object's `sections`_ and built their command graphs.
That list may also hold :class:`BaseIniGraphGroupEdit`\ s, which edit the command graphs where they sit in it
    )doc")
        .def_readwrite("deferred", &Step::deferred, py::doc(R"doc(
:class:`bool`: Whether the parser runs this step on the first ask of its :attr:`BaseIniParser.parseData` rather
than during the parse, for a step that may open many files. The deferred steps of a parser run together, in
their order in the list, after every step that is not deferred :raw-html:`<br />` :raw-html:`<br />`

**Default**: ``False``
        )doc"));

    py::class_<AGRC::ResourceRead, Step, py::smart_holder>(m, "ResourceRead", R"doc(
This class inherits from :class:`BaseIniParseStep`

Reads the file every resource `section`_ names: each `section`_ with a ``filename`` goes into
:meth:`IniParseData.resourceFiles`, under its own name
    )doc")
        .def(py::init<>());

    py::class_<AGRC::TextureOverrideRead, Step, py::smart_holder>(m, "TextureOverrideRead", R"doc(
This class inherits from :class:`BaseIniParseStep`

Reads whether the mod replaces textures by their hash: a ``TextureOverride`` `section`_ with a ``hash`` and a
``this =`` puts its art on every draw that binds that texture, whatever the draw's own `section`_ binds -- see
:meth:`IniParseData.replacesTexturesByHash`

Parameters
----------
draws: Optional[Callable[[Tuple[:class:`str`, :class:`str`]], :class:`bool`]]
    The mod objects whose root `sections`_ are draws rather than texture overrides, even with a ``hash`` and a
    ``this =`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")
        .def(py::init<AGRC::TextureOverrideRead::ObjFilter>(), py::arg("draws") = py::none())
        .def_readwrite("draws", &AGRC::TextureOverrideRead::draws,
                       py::doc(R"doc(Optional[Callable[[Tuple[:class:`str`, :class:`str`]], :class:`bool`]]: The mod objects whose root `sections`_ are draws)doc"));

    py::class_<AGRC::DrawRead, Step, py::smart_holder>(m, "DrawRead", R"doc(
This class inherits from :class:`BaseIniParseStep`

Reads what each mod object's root `sections`_ draw:

* :meth:`IniParseData.drawParts`: the parts holding a ``drawindexed`` (every part of the `section`_ counted,
  its ``if`` lines too)
* :meth:`IniParseData.drawRanges`: each ``drawindexed = <count>, <start>, <base>``
* :meth:`IniParseData.lateBindings`: the texture registers a `section`_ binds only after it has drawn -- read
  only when the mod does not replace textures by hash (:meth:`IniParseData.replacesTexturesByHash`, so run
  :class:`TextureOverrideRead` first), because such a mod's art reaches those draws anyway
* :meth:`IniParseData.undrawnObjs`: a mod object whose `sections`_ draw nothing and skip the game's draw
  (``handling = skip``) -- the mod hides it. A mod object that draws nothing and leaves the game's geometry on
  screen is not hidden, and when EVERY read mod object would be undrawn none is, since that is a mod this
  reading does not understand

Whether a `section`_ draws follows ``run =``: a called `section`_ that draws counts, and so does a call this
.ini file cannot resolve, unless its name holds one of :attr:`silentLibraries`

Parameters
----------
objs: Optional[Callable[[Tuple[:class:`str`, :class:`str`]], :class:`bool`]]
    The mod objects to read :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

texRegPrefix: :class:`str`
    What the key of a texture register starts with, for the late bindings :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``"ps-t"``

silentLibraries: List[:class:`str`]
    Text that names a library whose command lists draw nothing, in a ``run =`` this .ini file cannot resolve,
    ignoring case :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``[]``
    )doc")
        .def(py::init<AGRC::DrawRead::ObjFilter, std::string, std::vector<std::string>>(),
             py::arg("objs") = py::none(), py::arg("texRegPrefix") = std::string("ps-t"),
             py::arg("silentLibraries") = std::vector<std::string>{})
        .def_readwrite("objs", &AGRC::DrawRead::objs, py::doc(ObjFilterDoc))
        .def_readwrite("texRegPrefix", &AGRC::DrawRead::texRegPrefix,
                       py::doc(R"doc(:class:`str`: What the key of a texture register starts with)doc"))
        .def_readwrite("silentLibraries", &AGRC::DrawRead::silentLibraries,
                       py::doc(R"doc(List[:class:`str`]: Text that names a library whose command lists draw nothing)doc"));

    py::class_<AGRC::LibraryRead, Step, py::smart_holder>(m, "LibraryRead", R"doc(
This class inherits from :class:`BaseIniParseStep`

Reads which libraries each mod object's root `sections`_ use: a library is used when any key or value of those
`sections`_ names it -- see :meth:`IniParseData.libraries`

Parameters
----------
libraries: List[Tuple[:class:`str`, :class:`str`]]
    The libraries to look for, in the order they are filed: each is its name in :meth:`IniParseData.libraries`
    and the text that names it in a line, ignoring case

objs: Optional[Callable[[Tuple[:class:`str`, :class:`str`]], :class:`bool`]]
    The mod objects to read :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")
        .def(py::init<std::vector<AGRC::LibraryRead::Library>, AGRC::LibraryRead::ObjFilter>(),
             py::arg("libraries"), py::arg("objs") = py::none())
        .def_readwrite("libraries", &AGRC::LibraryRead::libraries,
                       py::doc(R"doc(List[Tuple[:class:`str`, :class:`str`]]: The libraries to look for: ``(name, text that names it)``)doc"))
        .def_readwrite("objs", &AGRC::LibraryRead::objs, py::doc(ObjFilterDoc));

    py::class_<AGRC::SkeletonNumberingRead, Step, py::smart_holder>(m, "SkeletonNumberingRead", R"doc(
This class inherits from :class:`BaseIniParseStep`

Reads which skeleton numbering the mod's blend gives its bone ids in, with a :class:`SkeletonNumberingProbe` over
the mod's own buffers, into :meth:`IniParseData.blendVersion`

The buffers are the files of the resource `sections`_ this names, as :meth:`IniParseData.resourceFile` has them
-- so run :class:`ResourceRead` first. Nothing is read with fewer than two numberings, with no centroids, or when
a buffer is missing

Parameters
----------
numberings: List[:class:`SkeletonNumberingProbe.Numbering`]
    The candidate numberings -- see :meth:`SkeletonNumberingProbe.probe`

centroids: Dict[:class:`int`, Tuple[:class:`float`, :class:`float`, :class:`float`]]
    Each bone's rest-pose centroid -- see :meth:`SkeletonNumberingProbe.probe`

positionResource: :class:`str`
    The resource `section`_ of the mod's positions

blendResource: :class:`str`
    The resource `section`_ of the mod's blend

vertexVGResource: :class:`str`
    Text in the name of the resource `section`_ of the mod's 16-bit bone ids, compared lowercased and with
    underscores removed, or empty for none. The first such `section`_ in name order is read
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

requiredKey: :class:`str`
    A key at least one root `section`_ of :attr:`objs` must carry for the mod to be probed, or empty to probe
    every mod :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

objs: Optional[Callable[[Tuple[:class:`str`, :class:`str`]], :class:`bool`]]
    The mod objects whose root `sections`_ :attr:`requiredKey` is looked for in :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")
        .def(py::init<std::vector<AGRC::SkeletonNumberingProbe::Numbering>, std::unordered_map<long long, std::array<double, 3>>,
                      std::string, std::string, std::string, std::string, AGRC::SkeletonNumberingRead::ObjFilter>(),
             py::arg("numberings"), py::arg("centroids"), py::arg("positionResource"), py::arg("blendResource"),
             py::arg("vertexVGResource") = std::string(), py::arg("requiredKey") = std::string(), py::arg("objs") = py::none())
        .def_readwrite("numberings", &AGRC::SkeletonNumberingRead::numberings,
                       py::doc(R"doc(List[:class:`SkeletonNumberingProbe.Numbering`]: The candidate numberings)doc"))
        .def_readwrite("centroids", &AGRC::SkeletonNumberingRead::centroids,
                       py::doc(R"doc(Dict[:class:`int`, Tuple[:class:`float`, :class:`float`, :class:`float`]]: Each bone's rest-pose centroid)doc"))
        .def_readwrite("positionResource", &AGRC::SkeletonNumberingRead::positionResource,
                       py::doc(R"doc(:class:`str`: The resource `section`_ of the mod's positions)doc"))
        .def_readwrite("blendResource", &AGRC::SkeletonNumberingRead::blendResource,
                       py::doc(R"doc(:class:`str`: The resource `section`_ of the mod's blend)doc"))
        .def_readwrite("vertexVGResource", &AGRC::SkeletonNumberingRead::vertexVGResource,
                       py::doc(R"doc(:class:`str`: Text in the name of the resource `section`_ of the mod's 16-bit bone ids)doc"))
        .def_readwrite("requiredKey", &AGRC::SkeletonNumberingRead::requiredKey,
                       py::doc(R"doc(:class:`str`: A key the mod must carry to be probed, or empty for none)doc"))
        .def_readwrite("objs", &AGRC::SkeletonNumberingRead::objs,
                       py::doc(R"doc(Optional[Callable[[Tuple[:class:`str`, :class:`str`]], :class:`bool`]]: The mod objects whose root `sections`_ the required key is looked for in)doc"));
}
