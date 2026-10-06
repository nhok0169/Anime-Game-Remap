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
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"
#include "AGRemapCore/data/IniFixData/SideMeshes.h"

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
        )doc"))
        .def_readwrite("donorNormalMap", &AGRC::GIMIComponentParserConfig::Slot::donorNormalMap, py::doc(R"doc(
:class:`bool`: Whether the donor's NORMAL MAP is downloaded too, at :attr:`normalMapReg` --- for a
target that reads normal maps. **Default**: ``False``
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
                        py::doc(":class:`int`: The `blend`_ stride, shared by every component. **Default**: ``32``"))
        .def_readwrite("downloadsByName", &AGRC::GIMIComponentParserConfig::downloadsByName, py::doc(R"doc(
:class:`bool`: Whether a slot's texture downloads follow the resource NAMES its own `section`_ binds, rather than
the registers

A mod written in the GAME's register order binds its textures somewhere other than the slot's registers:
eg. a LumineHeaven mod whose Eye binds only ``ps-t1 = ...Diffuse``. Decided per register, the slot would get the
game's diffuse at ``ps-t0`` as well, two textures would name a diffuse, and the merge's by-name reading would
refuse both. With this on, when a slot's own section binds its textures under names that are
believed (every one names exactly one role, no two alike), a role the mod binds gets no download, and a
missing role whose register holds another role's texture is downloaded onto a register the section leaves
free. A slot binding nothing, or in the slot's own order, is untouched.

**Default**: ``False``
        )doc"));

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
        .def_readwrite("outline", &AGRC::GIMIMergeFixerConfig::Slot::outline, py::doc(R"doc(
:class:`bool`: Whether this slot is drawn in the TARGET's outline pass. **Default**: ``True``

A skin may outline a slot with a shader of its own (CitlaliWhisperofStars' dress). Merged into a
target object it is drawn by the target's outline shader instead, which can cover it in black;
``False`` puts the member's block under ``if vs != 037730.0`` --- the ``filter_index`` ORFix gives every
outline vertex shader. Honoured for a merged member drawn by an appended block
        )doc"))
        .def_readwrite("splitFrom", &AGRC::GIMIMergeFixerConfig::Slot::splitFrom, py::doc(R"doc(
:class:`str`: Another slot of the SAME component whose triangles this slot takes a part of --- those whose light map
band under the centroid is in :attr:`splitBands` --- drawn onto :attr:`to`; the rest stay. For a skin slot holding
two kinds of surface the target shades in different draws (LumineHeaven's head: back hair and cloth). Empty by default
        )doc"))
        .def_readwrite("splitBands", &AGRC::GIMIMergeFixerConfig::Slot::splitBands, py::doc(R"doc(
List[Tuple[:class:`int`, :class:`int`]]: For a :attr:`splitFrom` slot, the light map alpha ranges (inclusive) that
select its triangles. Empty by default
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
        .def_readwrite("modTypeName", &AGRC::GIMIMergeFixerConfig::Component::modTypeName, py::doc(R"doc(
:class:`str`: The component's own mod type name, whose hashes find its sections --- empty for the skin's name followed
by :attr:`name`. For a component not named that way, eg. NeuvilletteMelusent's UNNAMED main mesh (``""``), filed as
``NeuvilletteMelusentMain``. **Default**: empty
        )doc"))
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
        )doc"))
        .def_readwrite("positionOffset", &AGRC::GIMIMergeFixerConfig::Component::positionOffset, py::doc(R"doc(
List[:class:`float`]: Added to every vertex position of this component as it is merged, in model units ---
``[0, 0, 0]`` (the default) writes the mod's own. NeuvilletteMelusent's Eye sits 1.24 cm lower than
Neuvillette's, and merged as it is the eyes looked down
        )doc"))
        .def_readwrite("positionOffsetFade", &AGRC::GIMIMergeFixerConfig::Component::positionOffsetFade, py::doc(R"doc(
List[:class:`float`]: The ``y`` range over which :attr:`positionOffset` fades to nothing --- full at or
below ``[0]``, none at or above ``[1]``, linear between. ``[0, 0]`` (the default) applies the whole
offset to every vertex

For a pair whose two models differ in HEIGHT rather than in origin. Dropping a mod so its soles meet a
lower target also drops its head, and the GAME draws the FACE for any mod carrying no face mesh of its
own, so the head must stay put: fade the offset out by the height of the face and the difference is
taken up through the body. **Default**: ``[0, 0]``
        )doc"))
        .def_readwrite("offsetOnlyWithGameFace", &AGRC::GIMIMergeFixerConfig::Component::offsetOnlyWithGameFace, py::doc(R"doc(
:class:`bool`: Whether :attr:`positionOffset` applies only while the mod keeps the GAME's face (not when it skips
the source's face diffuse and brings its own). ``False`` by default
        )doc"));

    py::enum_<AGRC::GIMIMergeFixerConfig::TargetLayout>(fixerConfig, "TargetLayout", R"doc(
How the TARGET's shader reads its textures
    )doc")
        .value("Plain", AGRC::GIMIMergeFixerConfig::TargetLayout::Plain,
               "``ps-t0`` diffuse, ``ps-t1`` light map, under ``NNFix``: a source slot's normal map is dropped and the rest shifted down")
        .value("NormalMap", AGRC::GIMIMergeFixerConfig::TargetLayout::NormalMap,
               "``ps-t0`` normal map, ``ps-t1`` diffuse, ``ps-t2`` light map, under ``ORFix``: a normal-map slot passes through, a plain one is shifted up");

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
        .def_readwrite("faceOnlyWhenMoved", &AGRC::GIMIMergeFixerConfig::faceOnlyWhenMoved, py::doc(R"doc(
:class:`bool`: Whether the mod's face section is copied onto the target ONLY when its diffuse has to
move onto :attr:`faceReg`

For two skins drawing the SAME face meshes on the SAME face hash, the mod's own section already fires
on the target and a copy is a second override on that hash --- a mod conflict on every reload.
**Default**: ``False``
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
        .def_property("diffuseEdits",
            [](const AGRC::GIMIMergeFixerConfig &self) {
                py::list result;
                for (const auto &edit : self.diffuseEdits) {
                    result.append(py::make_tuple(edit.first, fromPyRefFunction<void(AGRC::TextureFile&)>(edit.second)));
                }
                return result;
            },
            [](AGRC::GIMIMergeFixerConfig &self, const std::vector<std::pair<std::string, py::object>> &edits) {
                self.diffuseEdits.clear();
                for (const auto &edit : edits) {
                    self.diffuseEdits.emplace_back(edit.first, toTexFilter(edit.second));
                }
            }, py::doc(R"doc(
List[Tuple[:class:`str`, Callable[[:class:`CppTextureFile`], ``None``]]]: A diffuse edit per TARGET
object, applied to the diffuse every slot landing on that object draws with --- eg. a body diffuse
brought to alpha 0 where the target's shader reads alpha 255 as a glow. The edit is handed the texture
itself and edits it in place. **Default**: empty
        )doc"))
        .def_readwrite("targetLayout", &AGRC::GIMIMergeFixerConfig::targetLayout, py::doc(R"doc(
:class:`GIMIMergeFixerConfig.TargetLayout`: How the TARGET's shader reads its textures

**Default**: :attr:`GIMIMergeFixerConfig.TargetLayout.Plain`
        )doc"))
        .def_readwrite("texRegsByName", &AGRC::GIMIMergeFixerConfig::texRegsByName, py::doc(R"doc(
:class:`bool`: Whether a carried binding goes to the register its resource NAME says, rather than
staying where the mod put it

``NNFix`` and ``ORFix`` read a ROLE out of a fixed register (the normal map from ``ps-t0``, the
diffuse from ``ps-t1``, the light map from ``ps-t2``), so a remapped section that keeps the mod's
own bindings and then calls one has to put them there first --- and a mod dumped straight from the
game does not, since the game's own draw binds them in a different order. Naming decides the role,
a binding naming none is left alone, and this subsumes
:attr:`GIMIMergeFixerConfig.targetLayout`'s positional shift of a plain slot

**Default**: ``False``
        )doc"))
        .def_readwrite("downloadPrefix", &AGRC::GIMIMergeFixerConfig::downloadPrefix, py::doc(R"doc(
:class:`str`: The source character's download prefix --- the same string the parse row gives
:attr:`GIMIComponentParserConfig.downloadPrefix`

A mod may carry none of a component, and the merge then reads that component out of its downloads,
which land under this prefix. Empty disables the fallback
        )doc"))
        .def_readwrite("mipmaps", &AGRC::GIMIMergeFixerConfig::mipmaps,
                        py::doc(":class:`bool`: Whether written textures carry a mip chain. **Default**: ``True``"))
        .def_readwrite("texcoordStride", &AGRC::GIMIMergeFixerConfig::texcoordStride, py::doc(R"doc(
:class:`int`: The TARGET's texcoord stride: the merged ``Texcoord.buf`` is at least this wide, zero-padded at the end
of each line, and the copied section declares it --- for a target reading more UV sets than any source component
carries (Neuvillette's 20 bytes under NeuvilletteMelusent's 12). **Default**: ``0``, the widest component's
        )doc"))
        .def_readwrite("compressTextures", &AGRC::GIMIMergeFixerConfig::compressTextures, py::doc(R"doc(
:class:`bool`: Whether edited textures are block-compressed

Off by default because BC7 blurs a material band across its boundary, and a band is read as an exact
value

**Default**: ``False``
        )doc"))
        .def_readwrite("copyPreamble", &AGRC::GIMIMergeFixerConfig::copyPreamble,
                        py::doc(":class:`str`: The comment written at the top of each generated `section`_ group"))
        .def_readwrite("sideMeshes", &AGRC::GIMIMergeFixerConfig::sideMeshes, py::doc(R"doc(
List[:class:`str`]: The hash types of the SOURCE skin's side meshes (its own draws that are no mod object, eg.
``["ib_face", "ib_headupper"]``) --- a mod's section hiding one is written again on the target's hash of the
same type. Empty by default
        )doc"))
        .def_readwrite("texFxLayoutSwitch", &AGRC::GIMIMergeFixerConfig::texFxLayoutSwitch, py::doc(R"doc(
:class:`bool`: Whether a slot's TexFx calls are moved onto the TARGET's layout variant (``.0`` without a normal
map, ``.1`` with one at ``ps-t0``) when its source layout differs --- a mod's call names its own character's layout.
A call already naming the target's layout is kept. ``True`` by default
        )doc"))
        .def_readwrite("texFxGuardUnreached", &AGRC::GIMIMergeFixerConfig::texFxGuardUnreached, py::doc(R"doc(
:class:`bool`: Whether a target object NO slot is drawn through gets a section withdrawing a pending `TexFx`_
request, when the mod calls TexFx --- that object's own outline draw would otherwise serve it over the merged
buffers. ``False`` by default
        )doc"));

    // ------------------------------------------------------------------- side meshes, both templates
    py::class_<AGRC::SideMeshes>(m, "SideMeshes", R"doc(
A mod's sections on the SOURCE character's side meshes (its own draws that are no mod object -- the face, the
head-upper), written again on the TARGET's. Both multi-component templates use it: see
:attr:`GIMIComponentFixerConfig.sideMeshes` and :attr:`GIMIMergeFixerConfig.sideMeshes`
    )doc")
        .def_static("build", &AGRC::SideMeshes::build, py::arg("fileTxt"), py::arg("hashes"), py::arg("srcName"),
                    py::arg("fromVersion"), py::arg("types"), py::arg("targetName"), py::arg("toVersion"), py::doc(R"doc(
The re-issued sections: each section of ``fileTxt`` whose ``hash`` is one of the SOURCE's side meshes of a type in
``types``, its body copied and its ``hash`` replaced by the TARGET's of the same type, renamed with the target's
name and the Remap keyword. A mesh both characters share is left to the mod's own section

Parameters
----------
fileTxt: :class:`str`
    The mod's ``.ini`` text

hashes: :class:`Hashes`
    The hash table both characters' side meshes are filed in

srcName: :class:`str`
    The source's mod type name

fromVersion: Optional[:class:`Version`]
    The version the mod is written for, ``None`` for the latest

types: List[:class:`str`]
    The side-mesh hash types, eg. ``["ib_face", "ib_headupper"]``

targetName: :class:`str`
    The name the target's side-mesh rows are filed under

toVersion: Optional[:class:`Version`]
    The version the fix is for, ``None`` for the latest

Returns
-------
:class:`str`
    The sections with a leading comment, or an empty string
        )doc"));

    // ------------------------------------------------------------------- the component fixer config
    // The FORWARD template: a classic-shape mod onto a skin of several components. Bound so a new pair
    // can be prototyped as a config handed to the same factory its compiled row will call, exactly as
    // the merge template above is (bound 2026-09-23, for Charlotte -> CharlotteHurlock).
    py::class_<AGRC::GIMIComponentFixerConfig> componentConfig(m, "GIMIComponentFixerConfig", R"doc(
What one remap onto a skin of SEVERAL COMPONENTS does differently, for :func:`makeGIMIComponentFixer`

A classic GIMI character is one mesh with one vertex-group numbering. A skin like ``YelanTranquil``
is a ``Body``, a ``Bang`` and an ``Eye``, each with its own buffers, hashes and vertex groups, so a
classic mod remapped onto it has to be SPLIT into one set of buffers per component and drawn through
each component's draw slot. The library does it with one fixer PER TARGET COMPONENT, each built from
this same config by :func:`makeGIMIComponentFixer`
    )doc");

    py::enum_<AGRC::GIMIComponentFixerConfig::SourceLayout>(componentConfig, "SourceLayout", R"doc(
Which texture layout the SOURCE mod's object sections bind
    )doc")
        .value("Plain", AGRC::GIMIComponentFixerConfig::SourceLayout::Plain,
               "``ps-t0`` diffuse, ``ps-t1`` light map: shifted up onto a normal-map slot, with a flat normal map created")
        .value("NormalMap", AGRC::GIMIComponentFixerConfig::SourceLayout::NormalMap,
               "``ps-t0`` normal map, ``ps-t1`` diffuse, ``ps-t2`` light map: already what a normal-map slot reads, passed through")
        .value("Detect", AGRC::GIMIComponentFixerConfig::SourceLayout::Detect,
               "per object: the normal-map layout when its section binds ``ps-t2``, the plain one otherwise");

    py::class_<AGRC::GIMIComponentFixerConfig::Component>(componentConfig, "Component", R"doc(
One component of the TARGET skin, and how the mod is drawn through it
    )doc")
        .def(py::init<>())
        .def_readwrite("name", &AGRC::GIMIComponentFixerConfig::Component::name,
                        py::doc(":class:`str`: The component's name, as the vertex-group table's component column spells it, eg. ``Body``"))
        .def_readwrite("modTypeName", &AGRC::GIMIComponentFixerConfig::Component::modTypeName,
                        py::doc(":class:`str`: The mod type standing for this component as a fix TARGET, eg. ``YelanTranquilBody``"))
        .def_readwrite("slot", &AGRC::GIMIComponentFixerConfig::Component::slot,
                        py::doc(":class:`str`: The target draw slot the mod is drawn through, eg. ``A``"))
        .def_readwrite("slotIndex", &AGRC::GIMIComponentFixerConfig::Component::slotIndex, py::doc(R"doc(
:class:`str`: The slot's ``match_first_index`` on the target, as a literal, eg. ``"0"``

Held here rather than in :class:`Indices` for the reason :attr:`GIMIComponentParserConfig.Slot.index`
records. Empty falls back to the table
        )doc"))
        .def_readwrite("slotIndices", &AGRC::GIMIComponentFixerConfig::Component::slotIndices, py::doc(R"doc(
Every draw slot of this TARGET component, by ``match_first_index`` (eg. ``["0", "46620", "71025"]``), or empty

The slots no drawn source object is routed to -- and every slot of a component the mod draws nothing onto -- get
the TexFx guard :attr:`GIMIComponentFixerConfig.unremappedSlots` writes, read off the result rather than kept by
hand. Added to that field, never replacing it. **Default**: empty

:type: List[:class:`str`]
)doc"))
        .def_readwrite("objSlotIndices", &AGRC::GIMIComponentFixerConfig::Component::objSlotIndices, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: Per SOURCE object, the ``match_first_index`` of the slot it is
drawn through instead of :attr:`slotIndex`, eg. ``[("body", "53529")]``. A skin's slots draw on
different shaders, and a source object is shaded right only by a slot drawn on a shader like its own.
**Default**: empty
        )doc"))
        .def_readwrite("claimShare", &AGRC::GIMIComponentFixerConfig::Component::claimShare, py::doc(R"doc(
:class:`float`: For a cut component, the least share of a vertex's weight on this component's groups
for it to claim the vertex (``0`` to ``1``) -- a coat that should take only its hanging tails, not the
back panel they blend into, sets a high one and the seam moves to where the weights are clean. See
:attr:`VGComponentSpec.claimShare`. **Default**: ``0``, the plain majority
        )doc"))
        .def_readwrite("overlapRings", &AGRC::GIMIComponentFixerConfig::Component::overlapRings, py::doc(R"doc(
:class:`int`: For a cut component, how many rings of its neighbours' triangles it draws as well, past its
own edge -- a seam that opens when the skin poses is then covered by the other side's copy. Ownership is
unchanged. See :attr:`VGComponentSpec.overlapRings`. **Default**: ``0``
        )doc"))
        .def_readwrite("pushAway", &AGRC::GIMIComponentFixerConfig::Component::pushAway, py::doc(R"doc(
List[:class:`VGPushAway`]: Cloth pushed horizontally away from a point on this component, by its weight share on the
push's source groups --- for cloth that clips a limb the target moves differently. Empty by default
        )doc"))
        .def_readwrite("innerOutlineObjs", &AGRC::GIMIComponentFixerConfig::Component::innerOutlineObjs, py::doc(R"doc(
List[:class:`str`]: The SOURCE objects (lowercase, eg. ``"head"``) whose INNER layers draw no outline on this
component --- the faces turned in towards the head, or covered by another layer, get vertex colour alpha 0. For hair
of close two-sided sheets, whose inner outline shows through as dark shards on a skin whose outline sits further out.
Needs the component's ``Position.buf``. Empty by default
        )doc"))
        .def_readwrite("innerOutlineReach", &AGRC::GIMIComponentFixerConfig::Component::innerOutlineReach, py::doc(R"doc(
float: How far along its normal a vertex looks for a covering layer, in model units. ``0.1`` by default
        )doc"))
        .def_readwrite("innerOutlineFacingAxis", &AGRC::GIMIComponentFixerConfig::Component::innerOutlineFacingAxis, py::doc(R"doc(
bool: Whether a face turned in towards the vertical axis through the objects' centre is inner too. ``True`` by default
        )doc"))
        .def_readwrite("splitGroups", &AGRC::GIMIComponentFixerConfig::Component::splitGroups, py::doc(R"doc(
Dict[:class:`int`, List[Tuple[:class:`int`, :class:`float`]]]: Source groups whose weight this component SHARES
among several of its bones, as ``{source group: [(bone, share), ...]}`` --- see
:attr:`VGComponentSpec.splitGroups`. Empty by default
        )doc"))
        .def_readwrite("mirroredObjs", &AGRC::GIMIComponentFixerConfig::Component::mirroredObjs, py::doc(R"doc(
List[:class:`str`]: The SOURCE objects (lowercase) whose triangles get a MIRRORED INNER LAYER on this component,
for single-layer cloth whose back faces the target's shader does not shade as cloth --- see
:attr:`VGComponentSpec.mirroredIbs`. Cut components only; empty by default
        )doc"))
        .def_readwrite("mirrorOffset", &AGRC::GIMIComponentFixerConfig::Component::mirrorOffset,
                       py::doc(":class:`float`: How far inside the surface the mirrored layer sits, in model units --- ``0.005`` by default: at 1 mm it z-fought the surface from outside"))
        .def_readwrite("mirrorBackUV", &AGRC::GIMIComponentFixerConfig::Component::mirrorBackUV, py::doc(R"doc(
:class:`bool`: Whether the :attr:`mirroredObjs` layer reads the source's back-face UVs (``TEXCOORD1``) where it has them
--- a two-sided cloth shader textures its back faces that way (Lumine10's starry skirt lining). ``False`` by default
        )doc"))
        .def_readwrite("mirrorBackedReach", &AGRC::GIMIComponentFixerConfig::Component::mirrorBackedReach, py::doc(R"doc(
:class:`float`: How far behind a :attr:`mirroredObjs` triangle a layer of the mod facing the other way makes its twin
unneeded, in model units --- a coat with its own lining otherwise shows its twins as flat grey polygons through it. See
:attr:`VGComponentSpec.mirrorBackedReach`. ``0`` (the default) mirrors every triangle
        )doc"))
        .def_readwrite("texFxBlend", &AGRC::GIMIComponentFixerConfig::Component::texFxBlend, py::doc(R"doc(
:class:`float`: For a component whose mod's TexFx is dropped (:attr:`dropTexFx`): the opacity, 0 to 1, its
SEE-THROUGH draws are blended at instead --- ``0`` (the default) leaves them opaque

A ``drawindexed`` range of an object whose section binds a TexFx mask at ``ps-t69`` is see-through when most
of its vertices sit on a mask code of 1-254 (TexFx's own legend, on the red channel). Such a range is drawn
through a ``CustomShader`` of its own that blends only the G-buffer's colour target, keeping the game's
shaders
        )doc"))
        .def_readwrite("dropTexFx", &AGRC::GIMIComponentFixerConfig::Component::dropTexFx, py::doc(R"doc(
:class:`bool`: Whether this component's remapped sections drop the mod's TexFx transparency (``ps-t69`` /
``ps-t70`` and every ``run = CommandList\TexFx\...``) -- for a slot whose shader TexFx does not
recognise, where the transparency texture blanks the part out entirely instead of fading it. The part
then draws opaque. **Default**: ``False``
        )doc"))
        .def_readwrite("standIns", &AGRC::GIMIComponentFixerConfig::Component::standIns, py::doc(R"doc(
Dict[:class:`int`, :class:`int`]: For a cut component, source groups it does not own, each to the bone
of this component that stands in for it -- so a vertex on a seam keeps that weight instead of dropping
it. A group the component's own row maps is ignored. **Default**: ``{}``
        )doc"))
        .def_readwrite("negativeIndex", &AGRC::GIMIComponentFixerConfig::Component::negativeIndex, py::doc(R"doc(
:class:`bool`: ``True`` for the negative-index split (the component draws the whole mod, with every
other component's bones as sentinels), ``False`` for the graph cut. **Default**: ``False``
        )doc"))
        .def_readwrite("normalMap", &AGRC::GIMIComponentFixerConfig::Component::normalMap, py::doc(R"doc(
:class:`bool`: Whether the slot's shader reads the normal-map layout under ``ORFix`` (``False``: the
plain layout under ``NNFix``, where a mod object on the normal-map layout has its normal map dropped
and its diffuse and light map moved down). **Default**: ``True``
        )doc"))
        .def_readwrite("face", &AGRC::GIMIComponentFixerConfig::Component::face,
                        py::doc(":class:`bool`: Whether this component's fix carries the face graph --- exactly one should. **Default**: ``False``"))
        .def_readwrite("texcoordStride", &AGRC::GIMIComponentFixerConfig::Component::texcoordStride, py::doc(R"doc(
:class:`int`: The TARGET component's Texcoord stride, or ``0`` to keep the mod's own

The mod's buffer is zero-padded or truncated at the END (where ``TEXCOORD1`` sits) to this width.
**Default**: ``0``
        )doc"))
        .def_readwrite("positionOffset", &AGRC::GIMIComponentFixerConfig::Component::positionOffset, py::doc(R"doc(
List[:class:`float`]: A model-space translation ``[x, y, z]`` added to every vertex position written
for this component, or all zeros to keep the mod's own

A mod's vertices are in its SOURCE's bind pose. A part that must sit inside something the GAME draws
-- the eyes in a face mesh neither mod carries -- cannot be off by the difference: measure it by
differencing the target component's own Position buffer against this fix's output for the identity
mod. **Default**: ``[0, 0, 0]``
        )doc"))
        .def_readwrite("positionOffsetFade", &AGRC::GIMIComponentFixerConfig::Component::positionOffsetFade, py::doc(R"doc(
List[:class:`float`]: The ``y`` range over which :attr:`positionOffset` fades to nothing --- full at or
below ``[0]``, none at or above ``[1]``, linear between. ``[0, 0]`` (the default) is no fade: every
vertex takes the whole offset

**A character and a skin of different HEIGHT cannot be matched by a rigid translation.** Lifting a mod
so its soles meet the target's ground lifts its head too, and the GAME draws the FACE at its own fixed
place for any mod that carries no face mesh --- Citlali's sole sits 4.5 cm below CitlaliWhisperofStars'
and her head only 1 cm above, so a whole-model lift put the feet right and left the face 4.5 cm below
the head. Fading it out by the height of the face anchors the head where the game's face is and takes
the difference up through the body, which is where two models of one character actually differ. Set
``[0]`` to the source's sole and ``[1]`` just above its eyes.

This is a DEFORMATION, not a translation: right where the models differ in height, wrong where they
differ by an ORIGIN (Xiangling and XianglingCheer), which needs the whole model moved and no fade.
**Default**: ``[0, 0]``
        )doc"))
        .def_readwrite("offsetOnlyWithGameFace", &AGRC::GIMIComponentFixerConfig::Component::offsetOnlyWithGameFace, py::doc(R"doc(
:class:`bool`: Whether :attr:`positionOffset` applies only while the mod draws with the GAME's face

A mod that hides the game's face (``handling = skip`` on the source's face diffuse hash) draws its own
inside its head mesh, and its eyes are placed for that face: shifting them drops them below it. Read
per ``.ini`` file. **Default**: ``False``, the offset always applies
        )doc"))
        .def_readwrite("slotRegisters", &AGRC::GIMIComponentFixerConfig::Component::slotRegisters, py::doc(R"doc(
List[:class:`str`]: Every ``ps-t`` register the TARGET's own slot binds, or empty to leave the
registers alone

A remapped section may bind only these: a register beyond them means something else to the target's
shader, which its own mods leave to the GAME. Read them off the target's identity mod
        )doc"));

    componentConfig
        .def(py::init<>())
        .def_readwrite("targetSkin", &AGRC::GIMIComponentFixerConfig::targetSkin,
                        py::doc(":class:`str`: The target skin's name in the vertex-group table, eg. ``YelanTranquil``"))
        .def_readwrite("drawnObjs", &AGRC::GIMIComponentFixerConfig::drawnObjs,
                        py::doc("List[:class:`str`]: The SOURCE's drawn objects, lowercase, in draw order --- must match the parser's"))
        .def_readwrite("components", &AGRC::GIMIComponentFixerConfig::components,
                        py::doc("List[:class:`GIMIComponentFixerConfig.Component`]: Every component of the target, whichever one a fixer is for --- the split is joint"))
        .def_readwrite("hiddenComponents", &AGRC::GIMIComponentFixerConfig::hiddenComponents, py::doc(R"doc(
List[:class:`str`]: The TARGET components nothing is remapped onto, by mod type name --- their own
draw is suppressed, since a component the mod does not reach still draws the skin's own geometry
        )doc"))
        .def_readwrite("unremappedSlots", &AGRC::GIMIComponentFixerConfig::unremappedSlots, py::doc(R"doc(
List[Tuple[:class:`str`, List[:class:`str`]]]: The skin's draw SLOTS nothing is remapped onto, as
``(component mod type name, [match_first_index, ...])``

Each withdraws a pending `TexFx`_ request on that slot's draw, which would otherwise be served on a
slot the mod never reaches. Written only when the mod calls TexFx
        )doc"))
        .def_readwrite("sideMeshes", &AGRC::GIMIComponentFixerConfig::sideMeshes, py::doc(R"doc(
List[:class:`str`]: The hash types of the source's SIDE MESHES (its own draws that are no mod object,
eg. ``["ib_face", "ib_headupper"]``)

A mod's section on one of those hashes -- a mask hiding the face with ``ib = null`` -- is written again
on the target's hash of the same type (filed under :attr:`targetSkin`), since the skin draws its own
side meshes under other hashes. A mesh both characters share is left to the mod's own section. Empty
by default
        )doc"))
        // Properties over toTexFilter / toPyRefFunction rather than def_readwrite: pybind11's own
        // conversion of a filter hands it a COPY of the texture, so an edit written in Python would
        // run and save the unedited texture (see the merge config's lightMapEdit above).
        .def_property("diffuseEdits",
            [](const AGRC::GIMIComponentFixerConfig &self) {
                py::list result;
                for (const auto &edit : self.diffuseEdits) {
                    result.append(py::make_tuple(edit.first, fromPyRefFunction<void(AGRC::TextureFile&)>(edit.second)));
                }
                return result;
            },
            [](AGRC::GIMIComponentFixerConfig &self, const std::vector<std::pair<std::string, py::object>> &edits) {
                self.diffuseEdits.clear();
                for (const auto &edit : edits) {
                    self.diffuseEdits.emplace_back(edit.first, toTexFilter(edit.second));
                }
            }, py::doc(R"doc(
List[Tuple[:class:`str`, Callable[[:class:`CppTextureFile`], ``None``]]]: A diffuse edit per SOURCE
object, applied on normal-map slots only --- eg. a head diffuse brought to alpha 1. The edit is handed
the texture itself and edits it in place
            )doc"))
        .def_property("lightMapEdit",
            [](const AGRC::GIMIComponentFixerConfig &self) {
                return fromPyRefFunction<AGRC::TexEditor::Filter(const std::string&),
                                         py::typing::Optional<PyTexFilter>(const std::string&)>(self.lightMapEdit);
            },
            [](AGRC::GIMIComponentFixerConfig &self, const PyOptionalCallable<py::typing::Optional<PyTexFilter>(const std::string&)> &lightMapEdit) {
                self.lightMapEdit = toPyRefFunction<AGRC::TexEditor::Filter(const std::string&)>(lightMapEdit);
            }, py::doc(R"doc(
Optional[Callable[[:class:`str`], Optional[Callable[[:class:`CppTextureFile`], ``None``]]]]: Builds the
light map edit for one object, closed over that object's diffuse path

A light map's alpha is a material band and the legend is the mod AUTHOR's, so a band is moved by what
the diffuse under the pixel shows. The edit it returns is handed the texture itself and edits it in
place; returning ``None`` leaves that object's light map alone
            )doc"))
        .def_readwrite("lightMapObjs", &AGRC::GIMIComponentFixerConfig::lightMapObjs, py::doc(R"doc(
List[:class:`str`]: The SOURCE objects :attr:`lightMapEdit` applies to, or empty for every object ---
a band number means a different material on each object
        )doc"))
        .def_readwrite("flatNormal", &AGRC::GIMIComponentFixerConfig::flatNormal,
                        py::doc(":class:`CppColour`: The flat normal map created for a normal-map slot, sRGB pre-corrected. **Default**: ``(55, 55, 255, 255)``"))
        .def_readwrite("flatNormalSize", &AGRC::GIMIComponentFixerConfig::flatNormalSize,
                        py::doc(":class:`int`: The created normal map's size. **Default**: ``1024``"))
        .def_readwrite("mipmaps", &AGRC::GIMIComponentFixerConfig::mipmaps,
                        py::doc(":class:`bool`: Whether written textures carry a mip chain. **Default**: ``True``"))
        .def_readwrite("compressTextures", &AGRC::GIMIComponentFixerConfig::compressTextures, py::doc(R"doc(
:class:`bool`: Whether edited textures are BC7-compressed --- turn it OFF with a band table, since BC7
moves the alpha a band is read from. **Default**: ``True``
        )doc"))
        .def_readwrite("normaliseVertexColour", &AGRC::GIMIComponentFixerConfig::normaliseVertexColour,
                        py::doc(":class:`bool`: Whether the vertex colour's G and B are set to 128 in the split texcoord. **Default**: ``True``"))
        .def_readwrite("zeroSecondUV", &AGRC::GIMIComponentFixerConfig::zeroSecondUV,
                        py::doc(":class:`bool`: Whether a 20-byte texcoord's second UV set is zeroed. **Default**: ``True``"))
        .def_readwrite("sourceLayout", &AGRC::GIMIComponentFixerConfig::sourceLayout,
                        py::doc(":class:`GIMIComponentFixerConfig.SourceLayout`: The SOURCE mod's texture layout. **Default**: :attr:`GIMIComponentFixerConfig.SourceLayout.Plain`"))
        .def_readwrite("layoutFromOwnFixCall", &AGRC::GIMIComponentFixerConfig::layoutFromOwnFixCall, py::doc(R"doc(
:class:`bool`: Whether :attr:`GIMIComponentFixerConfig.SourceLayout.Detect` reads a section rendering through its own
``NNFix`` (and no ``ORFix``) as the PLAIN layout even when it binds ``ps-t2`` -- ``NNFix`` reads only ``ps-t0`` / ``ps-t1``.
**Default**: ``False``
        )doc"))
        .def_readwrite("texRegsByName", &AGRC::GIMIComponentFixerConfig::texRegsByName, py::doc(R"doc(
:class:`bool`: Whether each drawn object's texture bindings go to the register their resource NAME's role belongs on
(``ps-t0`` diffuse / ``ps-t1`` light map, or ``ps-t0`` normal map / ``ps-t1`` diffuse / ``ps-t2`` light map when a normal
map is among them) before any other texture edit -- for a mod written in the GAME's register order rather than GIMI's.
A binding naming no role stays put. **Default**: ``False``
        )doc"))
        .def_readwrite("faceSwapOnlyFromDiffuseReg", &AGRC::GIMIComponentFixerConfig::faceSwapOnlyFromDiffuseReg, py::doc(R"doc(
:class:`bool`: Whether the face's ``ps-t0`` <-> ``ps-t1`` swap runs only for a mod binding its face
diffuse at ``ps-t0`` (a pre-6.x mod). **Default**: ``False``
        )doc"))
        .def_readwrite("texFxLayoutSwitch", &AGRC::GIMIComponentFixerConfig::texFxLayoutSwitch, py::doc(R"doc(
:class:`bool`: Whether a remapped part's TexFx calls are moved onto the TARGET's layout variant (``.0`` without a normal
map, ``.1`` with one at ``ps-t0``) when its source layout differs --- a mod's call names its own character's layout.
A call already naming the target's layout is kept. ``True`` by default
        )doc"))
        .def_readwrite("fillDrawOnlyWhenUndrawn", &AGRC::GIMIComponentFixerConfig::fillDrawOnlyWhenUndrawn, py::doc(R"doc(
:class:`bool`: Whether a remapped slot section gets ``drawindexed = auto`` only when the mod's own section
draws on no path -- a mod toggling variants of one object on an ``if`` / ``else if`` chain with no ``else``
otherwise draws every variant at once (see :attr:`RegFillMissing.onlyWhenAbsent`). **Default**: ``False``
        )doc"))
        .def_readwrite("copyPreamble", &AGRC::GIMIComponentFixerConfig::copyPreamble,
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

    m.def("makeGIMIComponentFixer", [](const AGRC::GIMIComponentFixerConfig &config, const std::string &component) {
        return PyIniFixFactory{AGRC::makeGIMIComponentFixer(config, component)};
    }, py::arg("config"), py::arg("component"), py::doc(R"doc(
Builds the fixer that remaps a classic-shape GIMI mod onto ONE component of a multi-component skin

The second fixer template, next to :func:`makeGIMICharFixer` (one mesh onto one mesh) and
:func:`makeGIMIMergeFixer` (several components onto one mesh). Called once per component of the
target with the same config, each result registered for that component's own mod type

Parameters
----------
config: :class:`GIMIComponentFixerConfig`
    The pair's config --- the same one for every component

component: :class:`str`
    Which of :attr:`GIMIComponentFixerConfig.components` this fixer is for, by name

Returns
-------
:class:`CppIniFixFactory`
    The factory, for :meth:`CppStrategyOverrides.setFixer`
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
