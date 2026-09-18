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

#include "PyGIMIComponentBuilders.h"

#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyGIMICharBuilders.h"         // PyIniParseFactory / PyIniFixFactory, the same wrappers
                                        // CppStrategyOverrides recognises
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"

// TexEditor::Filter is std::function<void(TextureFile&)>, and pybind11/functional.h needs the
// COMPLETE type to decide how to convert it -- see PyGIMICharBuilders.cpp's identical note.
#include "AGRemapCore/model/files/TextureFile.h"
#include "../tools/PyRefFunction.h"
#include "../model/strategies/texEditors/PyTexEditor.h"   // PyTexFilter


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppGIMIComponentBuilders(pybind11::module_ &m) {
    // ------------------------------------------------------------------- the parser config
    py::class_<AGRC::GIMIComponentParserConfig> parserConfig(m, "GIMIComponentParserConfig", R"doc(
What one SKIN OF SEVERAL COMPONENTS looks like, for :func:`makeGIMIComponentParser`

A skin like ``YelanTranquil`` is a ``Body``, a ``Bang`` and an ``Eye`` with separate `blend`_,
position, texcoord and ``ib`` buffers, and the asset tables file each component's hashes under that
COMPONENT's mod type name. :func:`makeGIMICharParser` cannot express that --- its mod objects are
all ``("", obj)``, its index map has the single key ``ib``, and its hash filter names one mod type.

The mod objects this produces are ``(component, kind)`` --- ``("Body", "blend")``,
``("Bang", "position")`` --- and ``(component, slot)`` for a drawn object.
    )doc");

    py::class_<AGRC::GIMIComponentParserConfig::Slot>(parserConfig, "Slot", R"doc(
One drawn slot of a component --- a range of its ``ib``, told apart by ``match_first_index``
    )doc")
        .def(py::init<>())
        .def_readwrite("name", &AGRC::GIMIComponentParserConfig::Slot::name,
                        py::doc(":class:`str`: The slot's name, eg. ``A``"))
        .def_readwrite("index", &AGRC::GIMIComponentParserConfig::Slot::index, py::doc(R"doc(
:class:`str`: The slot's ``match_first_index``, as a literal

Kept here rather than in :class:`Indices` because that table's reverse lookup buckets every row
holding a value by version and searches only the newest bucket at or below the version asked --- a
slot filed as ``"0"`` at 5.7 would make the 5.7 bucket *the* bucket for ``"0"``, and every classic
character's head would resolve to a slot named ``A``
        )doc"))
        .def_readwrite("diffuseReg", &AGRC::GIMIComponentParserConfig::Slot::diffuseReg,
                        py::doc(":class:`str`: The register this slot's diffuse is bound to. **Default**: ``\"ps-t0\"``"))
        .def_readwrite("lightMapReg", &AGRC::GIMIComponentParserConfig::Slot::lightMapReg,
                        py::doc(":class:`str`: The register this slot's light map is bound to. **Default**: ``\"ps-t1\"``"))
        .def_readwrite("normalMapReg", &AGRC::GIMIComponentParserConfig::Slot::normalMapReg,
                        py::doc(":class:`str`: The register this slot's normal map is bound to, or ``\"\"`` for a slot without one"))
        .def_readwrite("noTextures", &AGRC::GIMIComponentParserConfig::Slot::noTextures, py::doc(R"doc(
:class:`bool`: Whether this slot has no textures of its own and reads another slot's

**Default**: ``False``
        )doc"))
        .def_readwrite("textureDonor", &AGRC::GIMIComponentParserConfig::Slot::textureDonor, py::doc(R"doc(
:class:`str`: The ``<Component>;<Slot>`` whose textures the GAME draws this slot with, for a slot
that binds none of its own --- ``""`` for a slot that always has them

A GIMI ``TextureOverride`` binds registers for the draw call its hash matches and no other, so a
slot whose own `section`_ declares no ``ps-t`` renders with whatever the GAME had bound, however
thoroughly the mod repainted its own copy of the donor's atlas. Naming the donor here is what lets
those textures be DOWNLOADED rather than read out of the mod
        )doc"));

    py::class_<AGRC::GIMIComponentParserConfig::Component>(parserConfig, "Component", R"doc(
One component of the skin --- its own buffers, its own slots, its own mod type name
    )doc")
        .def(py::init<>())
        .def_readwrite("name", &AGRC::GIMIComponentParserConfig::Component::name,
                        py::doc(":class:`str`: The component's name, eg. ``Body``"))
        .def_readwrite("modTypeName", &AGRC::GIMIComponentParserConfig::Component::modTypeName, py::doc(R"doc(
:class:`str`: The mod type NAME this component's hashes are filed under, eg. ``YelanTranquilBody``

Each classifier is filtered to its own component's name, because a hash value is unique to one
character and so exactly one classifier can answer for any `section`_
        )doc"))
        .def_readwrite("slots", &AGRC::GIMIComponentParserConfig::Component::slots,
                        py::doc("List[:class:`GIMIComponentParserConfig.Slot`]: The component's drawn slots"))
        .def_readwrite("texcoordStride", &AGRC::GIMIComponentParserConfig::Component::texcoordStride, py::doc(R"doc(
:class:`int`: This component's texcoord stride --- they need not agree across a skin

**Default**: ``20``
        )doc"))
        .def_readwrite("vertexCount", &AGRC::GIMIComponentParserConfig::Component::vertexCount, py::doc(R"doc(
:class:`int`: The GAME model's vertex count for this component, for a downloaded `blend`_'s ``draw``
line --- ``0`` leaves the line out

**Default**: ``0``
        )doc"));

    parserConfig
        .def(py::init<>())
        .def_readwrite("modTypeId", &AGRC::GIMIComponentParserConfig::modTypeId,
                        py::doc(":class:`ModTypeId`: The skin this parses"))
        .def_readwrite("downloadCharFolder", &AGRC::GIMIComponentParserConfig::downloadCharFolder,
                        py::doc(":class:`str`: The character folder the default downloads come from"))
        .def_readwrite("downloadVersionFolder", &AGRC::GIMIComponentParserConfig::downloadVersionFolder,
                        py::doc(":class:`str`: The version folder the default downloads come from, eg. ``5_7``"))
        .def_readwrite("downloadPrefix", &AGRC::GIMIComponentParserConfig::downloadPrefix,
                        py::doc(":class:`str`: The prefix every downloaded file's name starts with"))
        .def_readwrite("components", &AGRC::GIMIComponentParserConfig::components,
                        py::doc("List[:class:`GIMIComponentParserConfig.Component`]: Every component of the skin"))
        .def_readwrite("positionStride", &AGRC::GIMIComponentParserConfig::positionStride,
                        py::doc(":class:`int`: The position stride, shared by every component. **Default**: ``40``"))
        .def_readwrite("blendStride", &AGRC::GIMIComponentParserConfig::blendStride,
                        py::doc(":class:`int`: The `blend`_ stride, shared by every component. **Default**: ``32``"));

    // ------------------------------------------------------------------- the merge fixer config
    py::class_<AGRC::GIMIMergeFixerConfig> fixerConfig(m, "GIMIMergeFixerConfig", R"doc(
What a MULTI-COMPONENT SOURCE landing on a ONE-MESH TARGET does, for :func:`makeGIMIMergeFixer`

The inverse of :func:`makeGIMIComponentFixer`'s direction, and the shape where several source
components merge onto one target: their `blend`_, position and texcoord buffers are laid end to
end, each component's `blend`_ remapped through its OWN reverse vertex-group row first, and every
object's ``ib`` offset by its component's first vertex. Always ONE ``.ini`` group out --- the target
draws through one set of buffer hashes, so unlike a split there is nothing to write a second file
for.
    )doc");

    py::class_<AGRC::GIMIMergeFixerConfig::Slot>(fixerConfig, "Slot", R"doc(
One source slot, and which of the target's objects it lands on
    )doc")
        .def(py::init<>())
        .def_readwrite("name", &AGRC::GIMIMergeFixerConfig::Slot::name,
                        py::doc(":class:`str`: The slot's name, eg. ``A``"))
        .def_readwrite("index", &AGRC::GIMIMergeFixerConfig::Slot::index,
                        py::doc(":class:`str`: The slot's ``match_first_index`` in the SOURCE, as a literal"))
        .def_readwrite("to", &AGRC::GIMIMergeFixerConfig::Slot::to, py::doc(R"doc(
:class:`str`: The TARGET object this slot lands on, lowercase --- eg. ``body``, ``head``

Two slots naming the same object are merged into one `section`_: their index buffers are
concatenated and drawn together
        )doc"))
        .def_readwrite("normalMap", &AGRC::GIMIMergeFixerConfig::Slot::normalMap,
                        py::doc(":class:`bool`: Whether this slot carries a normal map the target has no room for. **Default**: ``False``"))
        .def_readwrite("borrowFrom", &AGRC::GIMIMergeFixerConfig::Slot::borrowFrom, py::doc(R"doc(
:class:`str`: The ``<Component>;<Slot>`` this slot reads its textures from when it has none of its
own --- ``""`` for a slot that always has them
        )doc"))
        .def_readwrite("indexCount", &AGRC::GIMIMergeFixerConfig::Slot::indexCount, py::doc(R"doc(
:class:`int`: The GAME model's index count for this slot, used only when the mod does not have the
slot's ``ib`` on disk

Only ever read for a target object SEVERAL slots land on, which is the only place an index count is
needed. A slot the mod DOES have is measured from its own file, so a modded mesh of a different size
is unaffected

**Default**: ``0``
        )doc"));

    py::class_<AGRC::GIMIMergeFixerConfig::Component>(fixerConfig, "Component", R"doc(
One source component, in MERGE order
    )doc")
        .def(py::init<>())
        .def_readwrite("name", &AGRC::GIMIMergeFixerConfig::Component::name,
                        py::doc(":class:`str`: The component's name, eg. ``Body``"))
        .def_readwrite("slots", &AGRC::GIMIMergeFixerConfig::Component::slots,
                        py::doc("List[:class:`GIMIMergeFixerConfig.Slot`]: The component's draw slots"))
        .def_readwrite("vertexCount", &AGRC::GIMIMergeFixerConfig::Component::vertexCount, py::doc(R"doc(
:class:`int`: The GAME model's vertex count for this component, used only when the mod does not have
the component at all

A component the mod is missing is downloaded, and downloads are fetched AFTER the ``.ini`` file is
written --- so the count that goes into ``draw``, ``override_vertex_count`` and every later
component's vertex offset cannot be measured from the file. A downloaded buffer is the game's own,
so its length is this number

**Default**: ``0``
        )doc"));

    fixerConfig
        .def(py::init<>())
        .def_readwrite("components", &AGRC::GIMIMergeFixerConfig::components, py::doc(R"doc(
List[:class:`GIMIMergeFixerConfig.Component`]: Every component of the source, in MERGE order

The first takes vertex offset 0, so its index buffers pass through untouched --- worth putting the
biggest one there
        )doc"))
        .def_readwrite("targetObjs", &AGRC::GIMIMergeFixerConfig::targetObjs,
                        py::doc("List[:class:`str`]: The TARGET's drawn objects, lowercase, in draw order"))
        .def_readwrite("faceReg", &AGRC::GIMIMergeFixerConfig::faceReg, py::doc(R"doc(
:class:`str`: The register the target binds its face diffuse to, or ``""`` to leave the face graph
alone

GI 6.x swapped the face diffuse and the face light map, so a mod still writing the pre-6.x register
hands its diffuse to the light map slot --- the white shiny cheek spots
        )doc"))
        // A property over toPyRefFunction rather than def_readwrite: pybind11's own conversion hands
        // the filter this RETURNS a copy of the texture, so a light map edit written in Python ran and
        // saved the unedited light map.
        .def_property("lightMapEdit",
            [](const AGRC::GIMIMergeFixerConfig &self) {
                return fromPyRefFunction<AGRC::TexEditor::Filter(const std::string&),
                                         py::typing::Optional<PyTexFilter>(const std::string&)>(self.lightMapEdit);
            },
            [](AGRC::GIMIMergeFixerConfig &self, const PyOptionalCallable<py::typing::Optional<PyTexFilter>(const std::string&)> &lightMapEdit) {
                self.lightMapEdit = toPyRefFunction<AGRC::TexEditor::Filter(const std::string&)>(lightMapEdit);
            }, py::doc(R"doc(
Optional[Callable[[:class:`str`], Optional[Callable[[:class:`CppTextureFile`], ``None``]]]]: Builds the
light map edit for one object, closed over that object's diffuse path

A light map's alpha is a material band and the legend differs per skin, so the bands have to be
moved. Conditioning each move on the DIFFUSE under the pixel is what keeps a PORT --- which carries
its source character's legend --- from being mangled

The edit it returns is handed the texture itself, not a copy, so it edits in place --- eg. through
:meth:`CppTextureFile.getPixels` / :meth:`CppTextureFile.setPixels`. Returning ``None`` leaves that
object's light map alone
        )doc"))
        .def_readwrite("mipmaps", &AGRC::GIMIMergeFixerConfig::mipmaps,
                        py::doc(":class:`bool`: Whether written textures carry a mip chain. **Default**: ``True``"))
        .def_readwrite("compressTextures", &AGRC::GIMIMergeFixerConfig::compressTextures, py::doc(R"doc(
:class:`bool`: Whether edited textures are block-compressed

Off by default because BC7 blurs a material band across its boundary, and a band is read as an exact
value

**Default**: ``False``
        )doc"))
        .def_readwrite("copyPreamble", &AGRC::GIMIMergeFixerConfig::copyPreamble,
                        py::doc(":class:`str`: The comment written at the top of each generated `section`_ group"));

    // ------------------------------------------------------------------- the factories
    m.def("makeGIMIComponentParser", [](const AGRC::GIMIComponentParserConfig &config) {
        return PyIniParseFactory{AGRC::makeGIMIComponentParser(config)};
    }, py::arg("config"), py::doc(R"doc(
Builds the parser for a mod of a SKIN MADE OF SEVERAL COMPONENTS

One classifier per component, each filtered to that component's own mod type name, and a `section`_
offered to each in turn --- a hash value is unique to one character, so at most one answers. The
slot a drawn `section`_ belongs to is resolved from the config's literal ``match_first_index``
rather than by a reverse lookup, for the reason
:attr:`GIMIComponentParserConfig.Slot.index` records.

Parameters
----------
config: :class:`GIMIComponentParserConfig`
    What this skin's components are

Returns
-------
:class:`CppIniParseFactory`
    The factory, for :meth:`CppStrategyOverrides.setParser`
    )doc"));

    m.def("makeGIMIMergeFixer", [](const AGRC::GIMIMergeFixerConfig &config) {
        return PyIniFixFactory{AGRC::makeGIMIMergeFixer(config)};
    }, py::arg("config"), py::doc(R"doc(
Builds the fixer for a MULTI-COMPONENT SOURCE landing on a ONE-MESH TARGET

The third fixer template, next to :func:`makeGIMICharFixer` (one mesh onto one mesh) and
:func:`makeGIMIComponentFixer` (one mesh onto several components). It merges the mod's buffers as
one resource group, re-slots every register the target lays out differently, moves the material
bands, and writes a single ``.ini`` group.

.. note::
    Two things it does that no other template needs. A target object SEVERAL components land on is
    not one draw call: the merged index buffer is member after member, and a mod's own
    ``drawindexed`` lines address only the first, so the rest are appended. And a member that binds
    no textures of its own renders with the GAME's, which are downloaded --- so members that
    disagree are drawn separately, each with its own bindings and its own fix call

Parameters
----------
config: :class:`GIMIMergeFixerConfig`
    What this source's components are and where they land

Returns
-------
:class:`CppIniFixFactory`
    The factory, for :meth:`CppStrategyOverrides.setFixer`
    )doc"));
}
