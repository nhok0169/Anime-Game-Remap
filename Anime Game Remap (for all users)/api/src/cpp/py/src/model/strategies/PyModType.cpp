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

#include "PyModType.h"

#include <optional>

#include "../assets/PyHashes.h"
#include "../assets/PyIndices.h"
#include "../assets/PyModMappedAssets.h"
#include "../assets/PyVGRemaps.h"
#include "../assets/PyVertexCounts.h"
#include "../files/PyIniFile.h"
#include "iniFixers/PyIniFixBuilder.h"
#include "iniParsers/PyIniParseBuilder.h"
#include "iniRemovers/PyIniRemoveBuilder.h"
#include "../../tools/PyRanges.h"

#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/assets/Indices.h"
#include "AGRemapCore/model/assets/IndexCounts.h"
#include "AGRemapCore/model/assets/ShapeKeyChecksums.h"
#include "AGRemapCore/model/assets/DrawPasses.h"
#include "AGRemapCore/model/assets/TextureRegisters.h"
#include "AGRemapCore/model/assets/GameShaders.h"
#include "AGRemapCore/model/assets/ShapeKeyDispatchSizes.h"
#include "AGRemapCore/model/assets/BoneCentroids.h"
#include "AGRemapCore/model/assets/ShapeKeyRemaps.h"
#include "AGRemapCore/model/assets/MaterialCodes.h"
#include "AGRemapCore/model/assets/MaterialColours.h"
#include "AGRemapCore/model/assets/TextureDescs.h"
#include "AGRemapCore/model/assets/TextureThumbprints.h"
#include "AGRemapCore/model/assets/VGCounts.h"
#include "AGRemapCore/model/assets/VGOffsets.h"
#include "AGRemapCore/model/assets/VGRemaps.h"
#include "AGRemapCore/model/assets/VertexCounts.h"
#include "AGRemapCore/model/files/IniFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {
    // Carried between initCppModType and initCppModTypeLateBindings -- see PyModType.h for why
    // fixIni cannot be registered in the first pass.
    std::optional<py::class_<AGRC::ModType>> &modTypeClsSlot() {
        static std::optional<py::class_<AGRC::ModType>> slot;
        return slot;
    }
}


// Only ever called after initCppModType has filled the slot -- bindings.cpp registers the two in
// that order, and the late pass has nothing to attach to otherwise.
static py::class_<AGRC::ModType> &modTypeCls() {
    return *modTypeClsSlot();
}


void initCppModType(pybind11::module_ &m) {
    // 'Cpp'-prefixed -- unlike 'ModTypeIdData' (which has no pure-Python counterpart of the same
    // exact name), 'ModType' collides with the live pure-Python 'ModType' class in
    // model/strategies/ModType.py, which builds its own richer representation from this C++ data,
    // so the bound name is disambiguated with the 'Cpp' prefix; see Documentation/CLAUDE.md's
    // naming-pitfall section / Architecture/CLAUDE.md's 'Cpp' prefix rule.
    modTypeClsSlot().emplace(m, "ModType", R"doc(
Heavy data for a type of mod

Meant to carry the full C++-side representation of a mod type -- contrast with the cheap
:class:`ModTypeIdData` an ini classifier (e.g. :class:`BaseIniClassifier`) holds instead. The
Python-side :class:`ModType` is meant to build itself using this data.

Parameters
----------
gameTypeId: :class:`int`
    The id for the game this type of mod belongs to -- stored as-is, with no validation that it
    corresponds to one of :class:`GameTypeId`'s declared values (see :class:`GameTypeIdTools` if
    that's needed)

modTypeId: :class:`int`
    The id for this specific type of mod -- stored as-is, with no validation that it corresponds
    to one of :class:`ModTypeId`'s declared values (see :class:`ModTypeIdTools` if that's needed),
    so a custom mod type using some id not registered in :class:`ModTypeId` can still be represented

name: :class:`str`
    The default name for the type of mod

aliases: Optional[List[:class:`str`]]
    Other alternative names for the type of mod :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``[]``
    )doc");

    modTypeCls()

        // The four asset tables are constructor arguments here, not just readable attributes:
        // ModType's own constructor takes them, and a caller building a custom mod type has to be
        // able to hand it its own hashes/indices. They were absent for as long as the Python-facing
        // Hashes/Indices were a separate class hierarchy from the AGRC::Hashes/AGRC::Indices this
        // constructor wants -- see PyModMappedAssets.h.
        //
        // shared_ptr, matching ModType's own members: two ModTypes handed the SAME table share its
        // mutations (addRepoRows/addMap), which is exactly what the pure-Python original did with
        // its ordinary object references. nullptr/None means "give me my own fully-populated
        // default", which is NOT the same as an empty table.
        //
        // The four WWMI-only tables come after the builders in the C++ constructor (so no
        // positional C++ caller moved) but are not exposed here in that position: the builders are
        // not bound, so the lambda passes nullptr for them and forwards the tables after vgRemaps.
        .def(py::init([](int gameTypeId, int modTypeId, const std::string &name, const std::vector<std::string> &aliases,
                         std::shared_ptr<AGRC::Hashes> hashes, std::shared_ptr<AGRC::Indices> indices,
                         std::shared_ptr<AGRC::VertexCounts> vertexCounts, std::shared_ptr<AGRC::VGRemaps> vgRemaps,
                         std::shared_ptr<AGRC::IndexCounts> indexCounts, std::shared_ptr<AGRC::VGOffsets> vgOffsets,
                         std::shared_ptr<AGRC::VGCounts> vgCounts, std::shared_ptr<AGRC::ShapeKeyChecksums> shapeKeyChecksums,
                         std::shared_ptr<AGRC::DrawPasses> drawPasses, std::shared_ptr<AGRC::TextureRegisters> textureRegisters,
                         std::shared_ptr<AGRC::GameShaders> gameShaders, std::shared_ptr<AGRC::ShapeKeyDispatchSizes> shapeKeyDispatchSizes,
                         std::shared_ptr<AGRC::BoneCentroids> boneCentroids, std::shared_ptr<AGRC::ShapeKeyRemaps> shapeKeyRemaps,
                         std::shared_ptr<AGRC::MaterialCodes> materialCodes, std::shared_ptr<AGRC::MaterialColours> materialColours,
                         std::shared_ptr<AGRC::TextureDescs> textureDescs, std::shared_ptr<AGRC::TextureThumbprints> textureThumbprints) {
                 return AGRC::ModType(gameTypeId, modTypeId, name, aliases, std::move(hashes), std::move(indices),
                                      std::move(vertexCounts), std::move(vgRemaps), nullptr, nullptr, nullptr,
                                      std::move(indexCounts), std::move(vgOffsets), std::move(vgCounts), std::move(shapeKeyChecksums),
                                      std::move(drawPasses), std::move(textureRegisters), std::move(gameShaders),
                                      std::move(shapeKeyDispatchSizes), std::move(boneCentroids), std::move(shapeKeyRemaps),
                                      std::move(materialCodes), std::move(materialColours), std::move(textureDescs),
                                      std::move(textureThumbprints));
             }),
             py::arg("gameTypeId"), py::arg("modTypeId"), py::arg("name"),
             py::arg("aliases") = std::vector<std::string>{},
             py::arg("hashes") = nullptr, py::arg("indices") = nullptr,
             py::arg("vertexCounts") = nullptr, py::arg("vgRemaps") = nullptr,
             py::arg("indexCounts") = nullptr, py::arg("vgOffsets") = nullptr,
             py::arg("vgCounts") = nullptr, py::arg("shapeKeyChecksums") = nullptr,
             py::arg("drawPasses") = nullptr, py::arg("textureRegisters") = nullptr, py::arg("gameShaders") = nullptr,
             py::arg("shapeKeyDispatchSizes") = nullptr, py::arg("boneCentroids") = nullptr, py::arg("shapeKeyRemaps") = nullptr,
             py::arg("materialCodes") = nullptr, py::arg("materialColours") = nullptr, py::arg("textureDescs") = nullptr,
             py::arg("textureThumbprints") = nullptr)

        .def_readwrite("indexCounts", &AGRC::ModType::indexCounts,
    py::doc(R"doc(:class:`IndexCounts`: The ``match_index_count`` of each of the mod's draw slots (WuWa)

The first of the four WWMI-only asset tables -- this, :attr:`vgOffsets`, :attr:`vgCounts` and
:attr:`shapeKeyChecksums` -- each an :class:`Indices` sibling keyed by the draw slot as its ``type``.
Same defaulting and sharing rules as :attr:`hashes`; a GI mod type's holds no row of its own
    )doc"))

        .def_readwrite("vgOffsets", &AGRC::ModType::vgOffsets,
    py::doc(R"doc(:class:`VGOffsets`: The ``vg_offset`` of each of the mod's draw slots (WuWa) -- see :attr:`indexCounts`)doc"))

        .def_readwrite("vgCounts", &AGRC::ModType::vgCounts,
    py::doc(R"doc(:class:`VGCounts`: The ``vg_count`` of each of the mod's draw slots (WuWa) -- see :attr:`indexCounts`)doc"))

        .def_readwrite("shapeKeyChecksums", &AGRC::ModType::shapeKeyChecksums,
    py::doc(R"doc(:class:`ShapeKeyChecksums`: The shape-key ``checksum`` of the mod (WuWa) -- see :attr:`indexCounts`)doc"))

        .def_readwrite("drawPasses", &AGRC::ModType::drawPasses,
    py::doc(R"doc(:class:`DrawPasses`: The draw passes of each of the mod's draw slots (WuWa) -- see :meth:`getDrawPass`

The first of six versioned tables of a character's GAME data that a fix reads at the run's version rather than carrying
it: this, :attr:`textureRegisters`, :attr:`gameShaders`, :attr:`shapeKeyDispatchSizes`, :attr:`boneCentroids` and
:attr:`shapeKeyRemaps`. Same defaulting and sharing rules as :attr:`hashes`
    )doc"))

        .def_readwrite("textureRegisters", &AGRC::ModType::textureRegisters,
    py::doc(R"doc(:class:`TextureRegisters`: Where the mod's own draws bind each texture role -- see :meth:`getTextureRegister`)doc"))

        .def_readwrite("gameShaders", &AGRC::ModType::gameShaders,
    py::doc(R"doc(:class:`GameShaders`: The game pixel shaders a fix draws the mod's parts with -- see :meth:`getGameShader`)doc"))

        .def_readwrite("shapeKeyDispatchSizes", &AGRC::ModType::shapeKeyDispatchSizes,
    py::doc(R"doc(:class:`ShapeKeyDispatchSizes`: The height WWMI dispatches the mod's shape keys with -- see :meth:`getShapeKeyDispatchSize`)doc"))

        .def_readwrite("boneCentroids", &AGRC::ModType::boneCentroids,
    py::doc(R"doc(:class:`BoneCentroids`: Each bone's rest-pose centroid on the mod's own model -- see :meth:`getBoneCentroids`)doc"))

        .def_readwrite("shapeKeyRemaps", &AGRC::ModType::shapeKeyRemaps,
    py::doc(R"doc(:class:`ShapeKeyRemaps`: The order the mod's shape keys are put in on another character -- see :meth:`getShapeKeyRemap`)doc"))

        .def_readwrite("materialCodes", &AGRC::ModType::materialCodes,
    py::doc(R"doc(:class:`MaterialCodes`: What the codes of the mod's single-channel code maps mean -- see :meth:`getMaterialCode`)doc"))

        .def_readwrite("materialColours", &AGRC::ModType::materialColours,
    py::doc(R"doc(:class:`MaterialColours`: The colours the mod's material masks mark each material with -- see :meth:`getMaterialColour`)doc"))

        .def_readwrite("textureThumbprints", &AGRC::ModType::textureThumbprints,
    py::doc(R"doc(:class:`TextureThumbprints`: What the mod's game textures look like, as thumbprints -- see :meth:`getTextureThumbprints`)doc"))

        .def_readwrite("textureDescs", &AGRC::ModType::textureDescs,
    py::doc(R"doc(:class:`TextureDescs`: How the game creates the mod's game textures -- see :meth:`getTextureDesc`)doc"))

        .def_readwrite("hashes", &AGRC::ModType::hashes,
    py::doc(R"doc(:class:`Hashes`: The hashes related to the mod and its fix

A :class:`ModType` constructed without one gets a **fully-populated** :class:`Hashes` -- every hash
the software ships with -- not an empty table

.. note::
    Shared, not copied: two :class:`ModType`\s handed the same table both see any
    :meth:`ModMappedAssets.addRepoRows`/:meth:`ModMappedAssets.addMap` made through either of them
    )doc"))

        .def_readwrite("indices", &AGRC::ModType::indices,
    py::doc(R"doc(:class:`Indices`: The indices related to the mod and its fix

Same defaulting and sharing rules as :attr:`hashes`
    )doc"))

        .def_readwrite("vertexCounts", &AGRC::ModType::vertexCounts,
    py::doc(R"doc(:class:`VertexCounts`: The vertex counts related to the mod

Same defaulting and sharing rules as :attr:`hashes`
    )doc"))

        .def_readwrite("vgRemaps", &AGRC::ModType::vgRemaps,
    py::doc(R"doc(:class:`VGRemaps`: The vertex group remaps for the mod

.. warning::
    Unlike :attr:`hashes`/:attr:`indices`/:attr:`vertexCounts`, the default here is the **shared**
    table every mod type uses, not a fresh one -- so mutating a defaulted :attr:`vgRemaps` is
    visible to every other mod type that also defaulted
    )doc"))

        .def_readwrite("gameTypeId", &AGRC::ModType::gameTypeId,
    py::doc(R"doc(:class:`int`: The id for the game this type of mod belongs to)doc"))

        .def_readwrite("modTypeId", &AGRC::ModType::modTypeId,
    py::doc(R"doc(:class:`int`: The id for this specific type of mod)doc"))

        .def_readwrite("name", &AGRC::ModType::name,
    py::doc(R"doc(:class:`str`: The default name for the type of mod)doc"))

        .def_readwrite("aliases", &AGRC::ModType::aliases,
    py::doc(R"doc(List[:class:`str`]: Other alternative names for the type of mod)doc"))

        .def("isName", &AGRC::ModType::isName, py::arg("name"), py::doc(R"doc(
Determines whether this mod type goes by some name

Compared case-insensitively against :attr:`ModType.name` and every entry in
:attr:`ModType.aliases`

Parameters
----------
name: :class:`str`
    The name to check

Returns
-------
:class:`bool`
    Whether this mod type goes by 'name'
        )doc"))

        .def("getModsToFix", &AGRC::ModType::getModsToFix, py::doc(R"doc(
Retrieves the names of the mods this mod type can be fixed onto, read from the remap targets
that exist in its asset tables

Returns
-------
Set[:class:`str`]
    The names of the mods to fix to
        )doc"))

        .def("getVertexCount", &AGRC::ModType::getVertexCount, py::arg("version") = py::none(), py::doc(R"doc(
Retrieves the number of vertices for this mod

Parameters
----------
version: Optional[:class:`Version`]
    The game version wanted :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest

Returns
-------
Optional[:class:`int`]
    The vertex count, or ``None`` if this mod type has no row for it
        )doc"))

        .def("getIndexCount", &AGRC::ModType::getIndexCount, py::arg("type"), py::arg("component") = "", py::arg("version") = py::none(), py::doc(R"doc(
Retrieves the ``match_index_count`` of one of this mod's draw slots (WuWa)

The four WWMI lookups (this, :meth:`getVGOffset`, :meth:`getVGCount`, :meth:`getShapeKeyChecksum`)
share one shape: the slot is the ``type`` (``component0``, ``component1``, ...), ``component`` is
``""`` on every shipped row, and the value comes back as the string the ``.ini`` carries, as an
:class:`Indices` value does

Parameters
----------
type: :class:`str`
    The draw slot, eg. ``component3``

component: :class:`str`
    The component column :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

version: Optional[:class:`Version`]
    The game version wanted :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest

Returns
-------
Optional[:class:`str`]
    The value, or ``None`` if this mod type has no row for it
        )doc"))

        .def("getVGOffset", &AGRC::ModType::getVGOffset, py::arg("type"), py::arg("component") = "", py::arg("version") = py::none(),
             py::doc(R"doc(Retrieves the ``vg_offset`` of one of this mod's draw slots (WuWa) -- see :meth:`getIndexCount`)doc"))

        .def("getVGCount", &AGRC::ModType::getVGCount, py::arg("type"), py::arg("component") = "", py::arg("version") = py::none(),
             py::doc(R"doc(Retrieves the ``vg_count`` of one of this mod's draw slots (WuWa) -- see :meth:`getIndexCount`)doc"))

        .def("getShapeKeyChecksum", &AGRC::ModType::getShapeKeyChecksum, py::arg("type") = "shapekeys", py::arg("component") = "", py::arg("version") = py::none(),
             py::doc(R"doc(Retrieves the shape-key ``checksum`` of this mod (WuWa) -- see :meth:`getIndexCount`; ``type`` is ``shapekeys`` on every shipped row)doc"))

        .def("getDrawPass", &AGRC::ModType::getDrawPass, py::arg("component"), py::arg("type"), py::arg("version") = py::none(), py::doc(R"doc(
One of :attr:`drawPasses`' values for a draw slot

Parameters
----------
component: :class:`str`
    The draw slot, eg. ``component3``

type: :class:`str`
    The pass and what is read of it: ``<pass>`` (its vertex shaders, comma-separated), ``<pass>.ps`` (its pixel
    shader) or ``<pass>.tags`` (each vertex shader's ``filter_index``) -- eg. ``main``, ``other.ps``

version: Optional[:class:`Version`]
    The game version wanted -- the newest row at or below it :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest

Returns
-------
Optional[:class:`str`]
    The value, or ``None`` when the mod has no such row
        )doc"))

        .def("getTextureRegister", &AGRC::ModType::getTextureRegister, py::arg("component"), py::arg("role"), py::arg("version") = py::none(), py::doc(R"doc(
The register the mod's own draw of a slot binds a texture role at (:attr:`textureRegisters`) -- else the game's default
for the role: the row named after the mod's game, with an empty component

Parameters
----------
component: :class:`str`
    The draw slot, eg. ``component3``

role: :class:`str`
    The role, eg. ``upperDiffuse``; ``<role>@rabbitfx`` for the key the role is handed to RabbitFX under;
    ``<role>@<pass>``, eg. ``upperDiffuse@other``, for where another pass of the slot (:meth:`getDrawPass`'s
    ``other``) binds it

version: Optional[:class:`Version`]
    The game version wanted -- see :meth:`getDrawPass`

    **Default**: ``None``

Returns
-------
Optional[:class:`str`]
    The register, or ``None`` when neither the mod nor its game has a row for the role
        )doc"))

        .def("getTextureRole", &AGRC::ModType::getTextureRole, py::arg("component"), py::arg("reg"), py::arg("version") = py::none(),
             py::arg("drawPass") = "", py::doc(R"doc(
The texture role the mod's own draw of a slot binds at a register -- :meth:`getTextureRegister` read backwards, with the
game's default where the mod has no row of the role. A RabbitFX key answers with the role it carries

Parameters
----------
component: :class:`str`
    The draw slot, eg. ``component3``

reg: :class:`str`
    The register, eg. ``ps-t3``, or a RabbitFX key

version: Optional[:class:`Version`]
    The game version wanted -- see :meth:`getDrawPass`

    **Default**: ``None``

drawPass: :class:`str`
    The pass of the slot, as :meth:`getDrawPass` names it (``other``, ``other2``, ...): its ``<role>@<pass>`` rows
    are read, and the role answered without the suffix. Empty for the slot's main pass (its plain rows and RabbitFX
    keys)

    **Default**: ``""``

Returns
-------
Optional[:class:`str`]
    The role, or ``None`` when no role is bound there
        )doc"))

        .def("getGameShader", &AGRC::ModType::getGameShader, py::arg("type"), py::arg("version") = py::none(), py::doc(R"doc(
One of :attr:`gameShaders`' values: a shader's HLSL (``<shader>``) or a constant buffer its draw read
(``<shader>.<register>``, eg. ``LynaeUpper.ps-cb4``: 32-bit words in hex, comma-separated)

Parameters
----------
type: :class:`str`
    The shader, eg. ``LynaeUpper``, or one of its constant buffers, eg. ``LynaeUpper.ps-cb4``

version: Optional[:class:`Version`]
    The game version wanted -- see :meth:`getDrawPass`

    **Default**: ``None``

Returns
-------
Optional[:class:`str`]
    The value, or ``None`` when the mod has no such row
        )doc"))

        .def("getShapeKeyDispatchSize", &AGRC::ModType::getShapeKeyDispatchSize, py::arg("version") = py::none(), py::doc(R"doc(
The height WWMI dispatches the mod's shape keys with (:attr:`shapeKeyDispatchSizes`)

Parameters
----------
version: Optional[:class:`Version`]
    The game version wanted -- see :meth:`getDrawPass`

    **Default**: ``None``

Returns
-------
Optional[:class:`str`]
    The height, or ``None`` when the mod has no row
        )doc"))

        .def("getBoneCentroids", &AGRC::ModType::getBoneCentroids, py::arg("version") = py::none(), py::doc(R"doc(
Each bone's rest-pose centroid on the mod's own model (:attr:`boneCentroids`) -- what :class:`SkeletonNumberingProbe`
scores a mod's numbering against

Parameters
----------
version: Optional[:class:`Version`]
    The game version wanted -- for each bone, the newest row at or below it

    **Default**: ``None``

Returns
-------
Dict[:class:`int`, List[:class:`float`]]
    Bone id -> its centroid, ``[x, y, z]``; empty when the mod has none
        )doc"))

        .def("getMaterialCode", &AGRC::ModType::getMaterialCode, py::arg("component"), py::arg("type"), py::arg("version") = py::none(), py::doc(R"doc(
A field or a code of one of the mod's code maps (:attr:`materialCodes`): ``<role>.<field>`` the bits the field
occupies (``propsDetail.flags.film``), ``<role>.<field>.<name>`` one code of it (``pouchDetail.material.plain``)

Parameters
----------
component: :class:`str`
    The draw slot, eg. ``component6``

type: :class:`str`
    The row's type

version: Optional[:class:`Version`]
    The game version wanted -- see :meth:`getDrawPass`

    **Default**: ``None``

Returns
-------
Optional[:class:`int`]
    The number, or ``None`` when the mod has no such row
        )doc"))

        .def("getMaterialColour", &AGRC::ModType::getMaterialColour, py::arg("component"), py::arg("type"), py::arg("version") = py::none(), py::doc(R"doc(
The colour one of the mod's material masks marks a material with (:attr:`materialColours`), eg. ``torsoMask.skin``

Parameters
----------
component: :class:`str`
    The draw slot, eg. ``component3``

type: :class:`str`
    The row's type, ``<role>.<material>``

version: Optional[:class:`Version`]
    The game version wanted -- see :meth:`getDrawPass`

    **Default**: ``None``

Returns
-------
Optional[:class:`CppColour`]
    The colour, or ``None`` when the mod has no such row
        )doc"))

        .def("getTextureDesc", &AGRC::ModType::getTextureDesc, py::arg("textureHash"), py::arg("version") = py::none(), py::doc(R"doc(
How the game creates one of the mod's game textures (:attr:`textureDescs`) -- what :meth:`RabbitFX.tagOf` takes

Parameters
----------
textureHash: :class:`str`
    The texture's hash

version: Optional[:class:`Version`]
    The game version wanted -- see :meth:`getDrawPass`

    **Default**: ``None``

Returns
-------
Optional[Tuple[:class:`str`, :class:`int`, :class:`int`, :class:`int`]]
    ``(format, width, height, mips)`` -- the format by its name, ``BC7_TYPELESS`` -- or ``None`` when the mod has no such row
        )doc"))

        .def("getTextureThumbprints", &AGRC::ModType::getTextureThumbprints, py::arg("version") = py::none(), py::doc(R"doc(
The thumbprint of each of the mod's game textures (:attr:`textureThumbprints`), by the texture's hash -- what
:attr:`WWMITextureFacts.textureThumbprints` holds

Parameters
----------
version: Optional[:class:`Version`]
    The game version wanted -- see :meth:`getDrawPass`

    **Default**: ``None``

Returns
-------
Dict[:class:`str`, List[:class:`int`]]
    Hash -> its thumbprint's bytes (16 x 16, row by row); empty when the mod has no rows
        )doc"))

        .def("getShapeKeyRemap", &AGRC::ModType::getShapeKeyRemap, py::arg("modName"), py::arg("fromVersion") = py::none(), py::arg("toVersion") = py::none(), py::doc(R"doc(
The source shape key each of another mod's shape-key slots takes, from this mod (:attr:`shapeKeyRemaps`) -- what
:meth:`WWMIShapeKeys.reorder` is handed

Parameters
----------
modName: :class:`str`
    The mod remapped to

fromVersion: Optional[:class:`Version`]
    This mod's version -- the newest row at or below it

    **Default**: ``None``

toVersion: Optional[:class:`Version`]
    The other mod's version -- likewise

    **Default**: ``None``

Returns
-------
Optional[List[:class:`int`]]
    The order (``-1`` for a slot that takes none), or ``None`` when the table has none for the pair
        )doc"))

        .def("getVGRemap", &AGRC::ModType::getVGRemap, py::arg("modName"), py::arg("fromVersion") = py::none(),
             py::arg("toVersion") = py::none(), py::arg("fromComp") = py::none(), py::arg("toComp") = py::none(),
             py::doc(R"doc(
Retrieves the vertex group remap for fixing this mod type onto another

Parameters
----------
modName: :class:`str`
    The name of the mod being fixed onto

fromVersion: Optional[:class:`Version`]
    The version being fixed from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest

toVersion: Optional[:class:`Version`]
    The version being fixed to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest

fromComp: Optional[:class:`str`]
    The component being fixed from. ``None`` leaves the column unconstrained :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``None``

toComp: Optional[:class:`str`]
    The component being fixed onto, with the same ``None`` meaning :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
Optional[:class:`VGRemap`]
    The remap, or ``None`` if the table has no matching row
        )doc"))

        .def("getHelpStr", &AGRC::ModType::getHelpStr, py::doc(R"doc(
Retrieves the help text describing this mod type, as the CLI prints it

Returns
-------
:class:`str`
    The help text
        )doc"))

        .def("getHashRanges", [](const AGRC::ModType &self, const AGRC::IfContentPartColouring<std::string, std::string> &partColours,
                                 const std::optional<AGRC::Version> &version, const py::object &nonVersionVals) {
            // Built fresh per call from py::none() rather than defaulted to a container literal --
            // py::arg("x") = <mutable container> is pybind11's version of Python's
            // mutable-default-argument bug, and every caller would share the one instance.
            std::vector<std::optional<std::string>> parsed;
            if (!nonVersionVals.is_none()) {
                parsed = nonVersionVals.cast<std::vector<std::optional<std::string>>>();
            }

            // The bound "Ranges" class is the PyRanges<long long> wrapper, not AGRC::Ranges
            // itself -- returning the core type raises "Unregistered type" at call time.
            AGRC::Ranges<long long> result = self.getHashRanges(partColours, version, parsed);
            return PyRanges<long long>(result.ranges, false);
        }, py::arg("partColours"), py::arg("version") = py::none(), py::arg("nonVersionVals") = py::none(),
             py::doc(R"doc(
Retrieves the valid ranges of order indices within an :class:`IfContentPart` whose ``hash`` values
belong to this mod type

Parameters
----------
partColours: :class:`IfContentPartColouring`
    The current states of the :class:`IfContentPart`

version: Optional[:class:`Version`]
    The version the hashes should come from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning any

nonVersionVals: Optional[List[Optional[:class:`str`]]]
    Values for the non-version index columns, used to narrow which instance of a hash is wanted
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`Ranges`
    The valid ranges of indices
        )doc"));
}


void initCppModTypeLateBindings(pybind11::module_ &m) {
    // The three builders are registered after this file's first pass (their build() signatures
    // name IniFile), and pybind11 bakes an attribute's type string at registration time -- so
    // these have to wait here or core.pyi gets a raw "AGRemapCore::IniFixBuilder".
    // Read/write so a caller can swap in their own, exactly as the pure-Python ModType allows.
    modTypeCls()
        .def_readwrite("iniParseBuilder", &AGRC::ModType::iniParseBuilder,
    py::doc(R"doc(:class:`IniParseBuilder`: The builder for the parser that reads a .ini file of this mod type)doc"))

        .def_readwrite("iniFixBuilder", &AGRC::ModType::iniFixBuilder,
    py::doc(R"doc(:class:`IniFixBuilder`: The builder for the fixer that fixes a .ini file of this mod type)doc"))

        .def_readwrite("iniRemoveBuilder", &AGRC::ModType::iniRemoveBuilder,
    py::doc(R"doc(:class:`IniRemoveBuilder`: The builder for the remover that removes a previous fix)doc"));

    // See PyModType.h for why this is a second pass rather than part of the chain above.
    // See PyModType.h for why this is a second pass rather than part of the chain above.
    modTypeCls().def("fixIni", &AGRC::ModType::fixIni, py::arg("iniFile"), py::arg("keepBackup") = true,
                     py::arg("fixOnly") = false, py::doc(R"doc(
Fixes a .ini file, but **only if that file was classified as this mod type** -- a no-op otherwise

Returns nothing: the fix it produces is written out by
:meth:`IniFile.fix` rather than handed back. Call that directly to see it

Parameters
----------
iniFile: :class:`IniFile`
    The .ini file to fix

keepBackup: :class:`bool`
    Whether to keep a backup copy of the original .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

fixOnly: :class:`bool`
    Whether to only fix without removing any previous fix :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
        )doc"));
}
