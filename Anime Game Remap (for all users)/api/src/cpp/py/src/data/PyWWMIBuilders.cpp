#include "PyWWMIBuilders.h"

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
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyGIMICharBuilders.h"         // PyIniParseFactory / PyIniFixFactory, the same wrappers
#include "../model/strategies/texEditors/PyTexEditor.h"   // PyTexFilter
#include "../tools/PyRefFunction.h"
#include "AGRemapCore/constants/WWMINames.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/textures/Colour.h"
#include "AGRemapCore/model/strategies/ModType.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppWWMIBuilders(pybind11::module_ &m) {
    py::class_<AGRC::WWMINames>(m, "WWMINames", R"doc(
The names a Wuthering Waves fix reads and writes, beside :class:`IniKeywords`: the rows its parser files, the parts
of the sections and files its fixer names, and the words of its own small syntaxes. One place, so a parser or fixer
built from `Python`_ spells them as the compiled ones do
    )doc")
        .def_property_readonly_static("TextureOverridesRows", [](py::object) { return AGRC::WWMINames::TextureOverridesRows; },
            py::doc(":class:`str`: Each texture override's ``hash``, ``this`` and ``match_priority`` -- rows a WWMI parser files"))
        .def_property_readonly_static("TextureRolesRows", [](py::object) { return AGRC::WWMINames::TextureRolesRows; },
            py::doc(":class:`str`: The texture roles the ``.ini`` states, with their conditions -- rows a WWMI parser files"))
        .def_property_readonly_static("TextureTogglesRows", [](py::object) { return AGRC::WWMINames::TextureTogglesRows; },
            py::doc(":class:`str`: Each texture override's own ``this`` and texture registers, with their branches -- rows a WWMI parser files"))
        .def_property_readonly_static("CarriedRows", [](py::object) { return AGRC::WWMINames::CarriedRows; },
            py::doc(":class:`str`: What a linked ``.ini`` sets on a slot's draw that a fix carries as it is -- rows a WWMI parser files"))
        .def_property_readonly_static("Draw", [](py::object) { return AGRC::WWMINames::Draw; },
            py::doc(":class:`str`: A draw's number in a section or list named after it"))
        .def_property_readonly_static("DrawAfter", [](py::object) { return AGRC::WWMINames::DrawAfter; },
            py::doc(":class:`str`: The list run after a draw, and the variable that latches it"))
        .def_property_readonly_static("Pass", [](py::object) { return AGRC::WWMINames::Pass; },
            py::doc(":class:`str`: A component drawn by another of the target's passes too"))
        .def_property_readonly_static("OffPass", [](py::object) { return AGRC::WWMINames::OffPass; },
            py::doc(":class:`str`: A draw kept off some of a slot's passes"))
        .def_property_readonly_static("Reroute", [](py::object) { return AGRC::WWMINames::Reroute; },
            py::doc(":class:`str`: A draw sent to another slot's pass"))
        .def_property_readonly_static("Overlay", [](py::object) { return AGRC::WWMINames::Overlay; },
            py::doc(":class:`str`: A component's overlay draw"))
        .def_property_readonly_static("Mark", [](py::object) { return AGRC::WWMINames::Mark; },
            py::doc(":class:`str`: A component's mark lines"))
        .def_property_readonly_static("Glow", [](py::object) { return AGRC::WWMINames::Glow; },
            py::doc(":class:`str`: The glow list a linked ``.ini``'s lines are carried into"))
        .def_property_readonly_static("Carried", [](py::object) { return AGRC::WWMINames::Carried; },
            py::doc(":class:`str`: A file a linked ``.ini`` binds, carried into a resource of the fix's own"))
        .def_property_readonly_static("Latch", [](py::object) { return AGRC::WWMINames::Latch; },
            py::doc(":class:`str`: The suffix of the variable that runs something once a frame"))
        .def_property_readonly_static("MergeWindowList", [](py::object) { return AGRC::WWMINames::MergeWindowList; },
            py::doc(":class:`str`: The list that merges the target's skeleton windows (a target past 256 bones)"))
        .def_property_readonly_static("ShapeKeyZero", [](py::object) { return AGRC::WWMINames::ShapeKeyZero; },
            py::doc(":class:`str`: The resource of zero shape-key offsets a fix binds"))
        .def_property_readonly_static("GameSuffix", [](py::object) { return AGRC::WWMINames::GameSuffix; },
            py::doc(":class:`str`: The suffix of a resource that binds the source's game texture"))
        .def_property_readonly_static("TexcoordNoNaN", [](py::object) { return AGRC::WWMINames::TexcoordNoNaN; },
            py::doc(":class:`str`: The texcoord buffer a fix writes with no ``NaN``"))
        .def_property_readonly_static("RemapMergedSkeletonList", [](py::object) { return AGRC::WWMINames::RemapMergedSkeletonList; },
                                      py::doc(":class:`str`: The fix's list that remaps the merged skeleton into the target's numbering, once a frame"))
        .def_property_readonly_static("MergeSkeletonList", [](py::object) { return AGRC::WWMINames::MergeSkeletonList; },
                                      py::doc(":class:`str`: The list a hidden slot's section runs to merge its bones into the skeleton"))
        .def_property_readonly_static("ShaderOverridePass", [](py::object) { return AGRC::WWMINames::ShaderOverridePass; },
                                      py::doc(":class:`str`: The prefix of the ``[ShaderOverride...]`` that tags one of the target's passes with a ``filter_index``"))
        .def_property_readonly_static("OwnMeshSkin", [](py::object) { return AGRC::WWMINames::OwnMeshSkin; },
                                      py::doc(":class:`str`: The resource of a part drawn from mesh files of its own: its blend, remapped"))
        .def_property_readonly_static("OwnMeshBlend", [](py::object) { return AGRC::WWMINames::OwnMeshBlend; },
                                      py::doc(":class:`str`: The file that remapped blend is written to (``<target>Remap<OwnMeshBlend><component>.buf``)"))
        .def_property_readonly_static("RabbitFXPin", [](py::object) { return AGRC::WWMINames::RabbitFXPin; },
                                      py::doc(":class:`str`: The list that pins RabbitFX's routing flags on a component's passes"))
        .def_property_readonly_static("Stash", [](py::object) { return AGRC::WWMINames::Stash; },
                                      py::doc(":class:`str`: The resource a register of the target's is parked in while one of the source's shaders draws"))
        .def_property_readonly_static("ShaderBackup", [](py::object) { return AGRC::WWMINames::ShaderBackup; },
                                      py::doc(":class:`str`: The resource a register's binding is backed up in around a shader draw of the fix's"))
        .def_property_readonly_static("BoneDataFilter", [](py::object) { return AGRC::WWMINames::BoneDataFilter; },
                                      py::doc(":class:`str`: WWMI's marker on the game's bone-data constant buffer (its ``filter_index``)"))
        .def_property_readonly_static("MergedSkeletonStep", [](py::object) { return AGRC::WWMINames::MergedSkeletonStep; },
                                      py::doc(":class:`int`: The size WWMI Tools' template gives a merged skeleton buffer, in float4 slots: 256 bones of three"))
        .def_property_readonly_static("SlotPrefix", [](py::object) { return AGRC::WWMINames::SlotPrefix; },
                                      py::doc(":class:`str`: The prefix of a draw slot's name in the library's tables and a fix's mod objects: ``component3``"))
        .def_property_readonly_static("OverrideSharedResourcesList", [](py::object) { return AGRC::WWMINames::OverrideSharedResourcesList; },
                                      py::doc(":class:`str`: WWMI Tools' list that binds a mod's shared resources (a list of it per component binds that component's)"))
        .def_property_readonly_static("CleanupSharedResourcesList", [](py::object) { return AGRC::WWMINames::CleanupSharedResourcesList; },
                                      py::doc(":class:`str`: WWMI Tools' list that puts the shared resources back after a mod's draws"))
        .def_property_readonly_static("ShapeKeyStreamStride", [](py::object) { return AGRC::WWMINames::ShapeKeyStreamStride; },
                                      py::doc(":class:`int`: Bytes per vertex of the game's shape-key offset stream (``vb6``), off a frame dump's layout"))
        .def_property_readonly_static("BoneDataHash", [](py::object) { return AGRC::WWMINames::BoneDataHash; },
                                      py::doc(":class:`str`: The hash type of a character's bone-data constant buffer"))
        .def_property_readonly_static("ShapeKeysHash", [](py::object) { return AGRC::WWMINames::ShapeKeysHash; },
                                      py::doc(":class:`str`: The hash type of a character's shape-key buffer"))
        .def_property_readonly_static("ShapeKeyOffsetsObj", [](py::object) { return AGRC::WWMINames::ShapeKeyOffsetsObj; },
                                      py::doc(":class:`str`: The object the classifier files a mod's shape-key offsets section under"))
        .def_property_readonly_static("ShapeKeyScaleObj", [](py::object) { return AGRC::WWMINames::ShapeKeyScaleObj; },
                                      py::doc(":class:`str`: The object the classifier files a mod's shape-key scale section under"))
        .def_property_readonly_static("BlendRemap", [](py::object) { return AGRC::WWMINames::BlendRemap; },
            py::doc(":class:`str`: The blend remap buffers a fix writes"))
        .def_property_readonly_static("ChecksumNotFound", [](py::object) { return AGRC::WWMINames::ChecksumNotFound; },
            py::doc(":class:`str`: What a shape-key checksum the library cannot look up is written as"))
        .def_property_readonly_static("LiveSuffix", [](py::object) { return AGRC::WWMINames::LiveSuffix; },
            py::doc(":class:`str`: A role's hash the game binds once the texture's mips have streamed in: ``<role>Live``"))
        .def_property_readonly_static("OldSuffix", [](py::object) { return AGRC::WWMINames::OldSuffix; },
            py::doc(":class:`str`: A role's older hash: ``<role>Old``, ``<role>Old2``, ..."));

    py::class_<AGRC::WWMITextureFacts>(m, "WWMITextureFacts", R"doc(
What a Wuthering Waves character's textures ARE, read by her parser (and by her fixer for what a role
belongs to)

A texture is identified by a HASH and a REGISTER: the hash is either the texture's own -- the
`section`_ IS that texture and its ``this =`` names the file -- or the MESH's, in which case
:class:`GIMISectionClassifier` places the section on a component and the register says what the file
it binds is. WuWa almost always uses the first (609 files against 31 over this corpus) and GI the
second, and both are read because either mod can be written

A file the mod DECLARES that neither names is identified by its pixels, against
:attr:`textureThumbprints`. Files the mod does not declare are not candidates at all
    )doc")
        .def(py::init<>())
        .def_readwrite("roles", &AGRC::WWMITextureFacts::roles, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: Texture hash -> role, for every version of the source's textures a
mod may carry -- the current hashes and the older ones the community's hash maps and
``Data/Mod Downloads/WuWa/<Name>/<Name>HashLineage.json`` know

A file plays EVERY role its hashes name: a mod declares one file under two hashes when one atlas
serves two components, and taking only the first leaves the second component drawing with the
TARGET's own textures
        )doc"))
        .def_readwrite("registerRoles", &AGRC::WWMITextureFacts::registerRoles, py::doc(R"doc(
Dict[:class:`int`, Dict[:class:`str`, :class:`str`]]: Which role each register binds, per source
component, as ``component -> {register -> role}`` -- the other way a texture is identified, for a
`section`_ carrying the MESH's hash
        )doc"))
        .def_readwrite("textureThumbprints", &AGRC::WWMITextureFacts::textureThumbprints, py::doc(R"doc(
Dict[:class:`str`, List[:class:`int`]]: The game's own textures by hash, each a
:attr:`thumbprintSize` square grayscale box average of the decoded file
(a shipped config leaves it empty: :meth:`WWMIParserConfig.withGameData` and :meth:`WWMIFixerConfig.withGameData` fill it
from the character's :class:`TextureThumbprints` rows). A declared file nothing else names
is thumbprinted the same way and correlated against every entry; it IS the texture it correlates at
least :attr:`identityMin` with while every other stays under :attr:`identityGap`. Empty skips
the step

.. note::
    The thumbprints are read from here only, not from the fixer's config; leaving this empty makes
    the pass silently identify nothing
        )doc"))
        .def_readwrite("thumbprintSize", &AGRC::WWMITextureFacts::thumbprintSize,
                        py::doc(":class:`int`: The side of a thumbprint. **Default**: ``16``"))
        .def_readwrite("identityMin", &AGRC::WWMITextureFacts::identityMin,
                        py::doc(":class:`float`: The correlation a file needs with ONE thumbprint to be that texture. **Default**: ``0.97``"))
        .def_static("roleOfHashType", &AGRC::WWMITextureFacts::roleOfHashType, py::arg("type"), py::arg("roles"), py::doc(R"doc(
The role a HashData hash type of the character names: the role itself, or the role followed by
:attr:`WWMINames.LiveSuffix`, or by :attr:`WWMINames.OldSuffix` and an optional count (``Old2``)

Parameters
----------
type: :class:`str`
    The hash type

roles: Set[:class:`str`]
    The character's roles

Returns
-------
Optional[:class:`str`]
    The role, or ``None``
        )doc"))
        .def_readwrite("identityGap", &AGRC::WWMITextureFacts::identityGap,
                        py::doc(":class:`float`: The correlation every OTHER thumbprint must stay under. **Default**: ``0.90``"))
        .def_readwrite("downloadGameFolder", &AGRC::WWMITextureFacts::downloadGameFolder,
                        py::doc(":class:`str`: The game folder under ``Data/Mod Downloads`` this character's textures are fetched from. Empty (the default): her game's name, filled by :meth:`WWMIFixerConfig.withGameData`"))
        .def_readwrite("downloadCharFolder", &AGRC::WWMITextureFacts::downloadCharFolder,
                        py::doc(":class:`str`: This character's download folder under :attr:`downloadGameFolder`. Empty (the default): her name, filled by :meth:`WWMIFixerConfig.withGameData`"))
        .def_readwrite("downloadVersionFolder", &AGRC::WWMITextureFacts::downloadVersionFolder,
                        py::doc(":class:`str`: The version folder under :attr:`downloadCharFolder`, eg. ``\"2_5\"``. **Default**: ``\"\"``"))
        .def_readwrite("downloadPrefix", &AGRC::WWMITextureFacts::downloadPrefix,
                        py::doc(":class:`str`: The file prefix of this character's downloads, eg. ``\"Sanhua\"`` for ``SanhuaTexture<hash>.dds``. **Default**: ``\"\"``"))
        .def_readwrite("fallbackTextures", &AGRC::WWMITextureFacts::fallbackTextures, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: Role -> this character's texture hash of that role, for a planned
role the mod has NO file for: the register is bound to the character's own game texture, downloaded
as ``<downloadPrefix><Role>RemapDL.dds`` into the mod's texture folder. The mod's UVs are the
source's, so the target's texture, which an unbound register samples on the target's draw, is wrong
by construction.

Name the hash ``Data/Mod Downloads`` actually holds, which is the one thing this says that
:attr:`roles` does not -- the folder was built from one frame dump, and that is not always the
generation :class:`HashData` files as current. **Default**: empty
        )doc"))
        .def_readwrite("regionRoles", &AGRC::WWMITextureFacts::regionRoles, py::doc(R"doc(
Set[:class:`str`]: The roles whose texture marks REGIONS, such as a material mask. A file the mod ships
for one is refused when it is flat, or when one slot binds it for this role and another too (a mask
pointed at the slot's normal map); the role then takes :attr:`fallbackTextures`, unless the fix leaves
it to the game (:attr:`WWMIFixerConfig.flatLeftToGame`). **Default**: empty
        )doc"))
        .def_static("of", &AGRC::WWMITextureFacts::of, py::arg("id"), R"doc(
The facts of a Wuthering Waves character, or ``None`` for one that has none -- the same facts its compiled parser reads,
so a parser written elsewhere identifies the character's textures the same way

Parameters
----------
id: :class:`ModTypeId`
    The character

Returns
-------
Optional[:class:`WWMITextureFacts`]
    Its facts
        )doc");

    py::class_<AGRC::WWMIParserConfig> parserConfig(m, "WWMIParserConfig", R"doc(
What one Wuthering Waves character looks like to its parser, for :func:`makeWWMIParser`

A WWMI mod is ONE mesh drawn in several ranges: every ``[TextureOverrideComponentN]`` matches the
character's ``vb0`` hash and one draw slot's ``match_first_index``, and the buffers, the bone data
and the shape keys are matched by hashes of their own. Nothing is named after the character, so a
section is classified by hash and index alone. Every draw slot the library knows for the character
becomes the mod object ``("", "componentN")``, and each entry of :attr:`hashOnlyObjs` becomes
``("", <obj>)``
    )doc");

    py::class_<AGRC::WWMIParserConfig::DrawSplit> drawSplit(parserConfig, "DrawSplit", R"doc(
Some draws of one slot, picked out by the material-code map the mod draws them with -- see
:attr:`WWMIParserConfig.drawSplits`
    )doc");

    py::class_<AGRC::WWMIParserConfig::DrawSplit::FlagMask>(drawSplit, "FlagMask", R"doc(
Where a material-code map carries the split's :attr:`WWMIParserConfig.DrawSplit.flags`, cut into
``width x height`` blocks: a block's bit is set when more than half of its texels carry any of them.
Bits are row-major, the lowest bit of each byte first
    )doc")
        .def(py::init<>())
        .def_readwrite("width", &AGRC::WWMIParserConfig::DrawSplit::FlagMask::width,
                        py::doc(":class:`int`: The number of blocks across"))
        .def_readwrite("height", &AGRC::WWMIParserConfig::DrawSplit::FlagMask::height,
                        py::doc(":class:`int`: The number of blocks down"))
        .def_readwrite("bits", &AGRC::WWMIParserConfig::DrawSplit::FlagMask::bits,
                        py::doc("List[:class:`int`]: The blocks, one bit each"));

    drawSplit
        .def(py::init<>())
        .def(py::init([](std::string name, int component, std::string codeRole, int flags) {
            return AGRC::WWMIParserConfig::DrawSplit{std::move(name), component, std::move(codeRole), flags};
        }), py::arg("name"), py::arg("component"), py::arg("codeRole"), py::arg("flags") = 0)
        .def_readwrite("name", &AGRC::WWMIParserConfig::DrawSplit::name, py::doc(R"doc(
:class:`str`: The split's name: the parser files the draws it picks as the :meth:`IniParseData.drawRanges`
of the mod object ``(name, <the slot's own name>)``
        )doc"))
        .def_readwrite("component", &AGRC::WWMIParserConfig::DrawSplit::component,
                        py::doc(":class:`int`: The source slot whose draws are read"))
        .def_readwrite("codeRole", &AGRC::WWMIParserConfig::DrawSplit::codeRole,
                        py::doc(":class:`str`: The role of the material-code map, among the slot's textures, that marks the draws"))
        .def_readwrite("flag", &AGRC::WWMIParserConfig::DrawSplit::flag, py::doc(R"doc(
:class:`str`: The :class:`MaterialCodes` row naming ``flags``: ``<codeRole>.flags.<flag>`` on the slot, eg. ``film`` for
``jacketDetail.flags.film``; :meth:`WWMIParserConfig.withGameData` reads it into ``flags``. **Default**: ``""``, ``flags``
as given
        )doc"))
        .def_readwrite("flags", &AGRC::WWMIParserConfig::DrawSplit::flags, py::doc(R"doc(
:class:`int`: The flag bits that mark a texel, in the code a shader reads off the map's red channel
(:meth:`TextureFile.shaderChannel`): a draw is picked when more than :attr:`WWMIParserConfig.DrawSplit.share`
of the texels under its vertices carry any of them
        )doc"))
        .def_readwrite("share", &AGRC::WWMIParserConfig::DrawSplit::share, py::doc(R"doc(
:class:`float`: The share of a draw's texels that must carry :attr:`WWMIParserConfig.DrawSplit.flags` for
the draw to be picked, eg. ``0.01`` for a draw that holds any real amount of the flagged material among
others. **Default**: ``0.5``, more than half
        )doc"))
        .def_readwrite("sourceFlags", &AGRC::WWMIParserConfig::DrawSplit::sourceFlags, py::doc(R"doc(
:class:`WWMIParserConfig.DrawSplit.FlagMask`: The character's own game code map of
:attr:`WWMIParserConfig.DrawSplit.codeRole`, for a mod that binds no code map of its own for that role:
such a mod draws with the game's map on its own character, so that map decides which of its draws carry
the flags. **Default**: empty, and such a mod's slot is not split
        )doc"));

    parserConfig
        .def(py::init<>())
        .def_static("slotWithPrefix", &AGRC::WWMIParserConfig::slotWithPrefix, py::arg("obj"), py::arg("prefix"), py::doc(R"doc(
The number a slot's mod object names under ``prefix``: ``("", <prefix><n>)`` is ``n``, for digits only -- what
:meth:`slotOf` reads with :attr:`WWMINames.SlotPrefix`

Parameters
----------
obj: Tuple[:class:`str`, :class:`str`]
    The mod object

prefix: :class:`str`
    The slot prefix, eg. ``component``

Returns
-------
Optional[:class:`int`]
    The number, or ``None`` for any other mod object
        )doc"))
        .def("slotOf", &AGRC::WWMIParserConfig::slotOf, py::arg("obj"), py::doc(R"doc(
The draw slot a slot's mod object names: ``("", <WWMINames.SlotPrefix><n>)`` is slot ``n``

Parameters
----------
obj: Tuple[:class:`str`, :class:`str`]
    The mod object

Returns
-------
Optional[:class:`int`]
    The slot, or ``None`` for any other mod object
        )doc"))
        .def_readwrite("drawSplits", &AGRC::WWMIParserConfig::drawSplits, py::doc(R"doc(
List[:class:`WWMIParserConfig.DrawSplit`]: Draws of a slot to pick out by the mod's own material-code map --
never by its colours -- for a fixer to treat apart (:attr:`WWMIFixerConfig.drawReroutes`). A slot whose map
is not the mod's own is read through :attr:`WWMIParserConfig.DrawSplit.sourceFlags`, or not split without
one; a split no draw carries is not filed. **Default**: none
        )doc"))
        .def_readwrite("version", &AGRC::WWMIParserConfig::version, py::doc(R"doc(
:class:`str`: The game version the library files the character's rows under, used when the ``.ini``
carries no version of its own. A reverse lookup with no version resolves through the newest bucket
holding the value, and ``0`` (component 0's index) is every GI head's index too

**Default**: ``"2.5"``
        )doc"))
        .def_readwrite("skeletonNumberings", &AGRC::WWMIParserConfig::skeletonNumberings, py::doc(R"doc(
List[:class:`SkeletonNumberingProbe.Numbering`]: Every numbering this character's merged skeleton has had,
when a game update renumbered it. The parser asks the mod's own geometry which one its bone ids are in
(:class:`SkeletonNumberingProbe`) and records it as :meth:`IniParseData.blendVersion` -- a mod's ``vb0``
cannot say, since hash-update tools rewrite the hashes and leave the bone ids. Fewer than two (the default)
switches it off
        )doc"))
        .def_readwrite("referenceBoneCentroids", &AGRC::WWMIParserConfig::referenceBoneCentroids, py::doc(R"doc(
Dict[:class:`int`, Tuple[:class:`float`, :class:`float`, :class:`float`]]: The rest-pose centroid of each of
this character's bones, keyed by its id in the numbering :attr:`skeletonNumberings` map onto
        )doc"))
        .def_readwrite("textures", &AGRC::WWMIParserConfig::textures, py::doc(R"doc(
:class:`WWMITextureFacts`: What this character's textures are -- what the parser identifies a mod's
textures WITH. Leaving it empty means every role falls back to downloading the game's own texture
        )doc"))
        .def_readwrite("hashOnlyObjs", &AGRC::WWMIParserConfig::hashOnlyObjs, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: The sections identified by a hash alone, as
``(hash type in HashData, mod object name)``

**Default**: the bone-data override and the two shape-key overrides
        )doc"));

    py::class_<AGRC::WWMIFixerConfig> fixerConfig(m, "WWMIFixerConfig", R"doc(
What one Wuthering Waves remap does differently, for :func:`makeWWMIFixer`

A WWMI character is drawn in SEVERAL COMPONENTS, each a range of one index buffer, all skinned in
ONE merged skeleton -- so a remap between two WuWa characters is a multi-component onto
multi-component remap, and every WuWa pair is. Per ``.ini`` file the fixer retargets every draw slot
the mod has a section for onto the target slot :attr:`plan` names, binds the mod's textures by
register on the target's passes (never by hash: WuWa texture hashes drift with streaming and game
version), binds a zero shape-key offset stream on every remapped draw, remaps the blend over the
8-byte WWMI layout, writes one remapped section per target draw per file (a further ``.ini`` copy
per extra claimant of a slot), and skips the target slots nothing is drawn through
    )doc");

    py::class_<AGRC::WWMIFixerConfig::Binding>(fixerConfig, "Binding", R"doc(
One register of a target slot's draw and the ROLE of the mod texture bound there
    )doc")
        .def(py::init<>())
        .def(py::init([](std::string reg, std::string role, int srcComponent) {
            return AGRC::WWMIFixerConfig::Binding{std::move(reg), std::move(role), srcComponent};
        }), py::arg("reg"), py::arg("role"), py::arg("srcComponent") = -1)
        .def_readwrite("reg", &AGRC::WWMIFixerConfig::Binding::reg,
                        py::doc(":class:`str`: The register, eg. ``\"ps-t0\"``"))
        .def_readwrite("role", &AGRC::WWMIFixerConfig::Binding::role, py::doc(R"doc(
:class:`str`: The role (a value of :attr:`WWMIFixerConfig.roles`,
or the name of a :attr:`WWMIFixerConfig.createdTextures` entry). A role no file of the mod has is
left unbound, so the GAME's texture serves the register
        )doc"))
        .def_readwrite("srcComponent", &AGRC::WWMIFixerConfig::Binding::srcComponent, py::doc(R"doc(
:class:`int`: In :attr:`WWMIFixerConfig.extraPassRegs` only: the SOURCE component this binding is for,
or ``-1`` for every source drawn through the slot. Needed where two sources merge onto one target
slot, since "the diffuse" is then a different file for each. **Default**: ``-1``
        )doc"));

    py::class_<AGRC::WWMIFixerConfig::Shader> shader(fixerConfig, "Shader", R"doc(
A pixel shader a :class:`WWMIFixerConfig.SourceComponent` is drawn with in place of the target slot's.
The fix writes the shader and its constant buffers beside the mod's buffers, declares them in its own
block (so an undo removes them), and turns each ``drawindexed`` of the component into a ``run =`` of a
``CustomShader`` section that binds them and makes that draw. Everything the shader does not bind
itself is the target draw's: its vertex shader, its render targets and the textures the component's
:attr:`WWMIFixerConfig.plan` entry binds
    )doc");

    py::class_<AGRC::WWMIFixerConfig::Shader::Stashed>(shader, "Stashed", R"doc(
A register of a :class:`WWMIFixerConfig.Shader`'s bound to a capture the fix makes off another slot -- see
:attr:`WWMIFixerConfig.Shader.stashed`
    )doc")
        .def(py::init<>())
        .def(py::init([](std::string role, int slot, std::string reg, std::string slotReg) {
                 return AGRC::WWMIFixerConfig::Shader::Stashed{std::move(role), slot, std::move(reg), std::move(slotReg)};
             }), py::arg("role"), py::arg("slot"), py::arg("reg") = "", py::arg("slotReg") = "")
        .def_readwrite("role", &AGRC::WWMIFixerConfig::Shader::Stashed::role, py::doc(":class:`str`: The role captured, eg. ``\"glow\"``"))
        .def_readwrite("slot", &AGRC::WWMIFixerConfig::Shader::Stashed::slot,
                        py::doc(":class:`int`: The target slot whose draw the capture is made off (:attr:`WWMIFixerConfig.stashRegs`)"))
        .def_readwrite("reg", &AGRC::WWMIFixerConfig::Shader::Stashed::reg, py::doc(R"doc(
:class:`str`: The shader's register, eg. ``"ps-t4"``. Empty: the source's :class:`TextureRegisters` row for :attr:`role` on
:attr:`WWMIFixerConfig.Shader.component` (:meth:`WWMIFixerConfig.withGameData`)
        )doc"))
        .def_readwrite("slotReg", &AGRC::WWMIFixerConfig::Shader::Stashed::slotReg, py::doc(R"doc(
:class:`str`: The register of that slot's draw that is captured. Empty: the target's :class:`TextureRegisters` row for
:attr:`role` on :attr:`slot` (:meth:`WWMIFixerConfig.withGameData`)
        )doc"));

    py::class_<AGRC::WWMIFixerConfig::Shader::After>(shader, "After", R"doc(
Which draws must come before a :class:`WWMIFixerConfig.Shader`'s component, on a pass -- see
:attr:`WWMIFixerConfig.Shader.after`
    )doc")
        .def(py::init<>())
        .def_readwrite("component", &AGRC::WWMIFixerConfig::Shader::After::component,
                        py::doc(":class:`int`: The SOURCE component (a key of :attr:`WWMIFixerConfig.plan`) drawn first"))
        .def_readwrite("draws", &AGRC::WWMIFixerConfig::Shader::After::draws, py::doc(R"doc(
:class:`int`: How many draws that component's slot makes in one pass; the deferred draw follows the last
of them. **Default**: ``1``
        )doc"))
        .def_readwrite("target", &AGRC::WWMIFixerConfig::Shader::After::target, py::doc(R"doc(
:class:`str`: The output register that tells one pass from another; a pass leaving it unbound draws the
component as before. **Default**: ``"o3"``
        )doc"))
        .def_readwrite("carried", &AGRC::WWMIFixerConfig::Shader::After::carried, py::doc(R"doc(
List[:class:`str`]: The registers a deferred draw takes from the component's own draw, eg. ``"ps-t0"``
        )doc"))
        .def_readwrite("also", &AGRC::WWMIFixerConfig::Shader::After::also, py::doc(R"doc(
List[Tuple[:class:`int`, :class:`int`]]: Further components the draw waits for as well, each with how many
draws its slot makes in one pass; the draw is made once all of them have drawn there, in whatever order.
A component the mod does not draw is left out of the count. **Default**: empty
        )doc"));

    shader
        .def(py::init<>())
        .def_readwrite("name", &AGRC::WWMIFixerConfig::Shader::name,
                        py::doc(":class:`str`: The name its files and sections are given, eg. ``\"LynaeFilm\"``"))
        .def_static("ofGame", &AGRC::WWMIFixerConfig::Shader::ofGame, py::arg("game"), R"doc(
A shader whose code and constant buffers are the :class:`GameShaders` row 'game' (:attr:`game`), named after it

:param game: :class:`str`: the row, eg. ``"LynaeUpper"``
:returns: :class:`WWMIFixerConfig.Shader`
        )doc")
        .def_readwrite("game", &AGRC::WWMIFixerConfig::Shader::game, py::doc(R"doc(
:class:`str`: The :class:`GameShaders` row its code and constant buffers are read from, eg. ``"LynaeFilm"`` -- the source's
or the target's game shader (:meth:`WWMIFixerConfig.withGameData` fills :attr:`ps` and :attr:`constantBuffers`). Empty:
the config gives them
        )doc"))
        .def_readwrite("ps", &AGRC::WWMIFixerConfig::Shader::ps,
                        py::doc(":class:`str`: The HLSL source, entry point ``main``"))
        .def_readwrite("constantBuffers", &AGRC::WWMIFixerConfig::Shader::constantBuffers, py::doc(R"doc(
Dict[:class:`str`, List[:class:`int`]]: Register -> the constant buffer bound there, as its 32-bit
words (four to a row), eg. ``"ps-cb4"``. Written as a ``R32G32B32A32_FLOAT`` buffer
        )doc"))
        .def_readwrite("lines", &AGRC::WWMIFixerConfig::Shader::lines, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: Further lines of the ``CustomShader`` section, written as they are,
after :attr:`stashed` and :attr:`gameRoles`, eg. ``("ps-t5", "ref ps-t6")`` or a render state
        )doc"))
        .def_readwrite("component", &AGRC::WWMIFixerConfig::Shader::component, py::doc(R"doc(
:class:`int`: The SOURCE component whose own draw the shader was read from: every register of :attr:`registers`,
:attr:`gameRoles` and :attr:`stashed` left empty is the source's :class:`TextureRegisters` row for the role on it
(:meth:`WWMIFixerConfig.withGameData`). **Default**: ``-1``, the component of the :attr:`WWMIFixerConfig.plan` entry the
shader is on (a reroute's: the component it reroutes)
        )doc"))
        .def_readwrite("refRoles", &AGRC::WWMIFixerConfig::Shader::refRoles, py::doc(R"doc(
List[:class:`str`]: Roles the shader reads off the target slot's own draw by ``ref``: :meth:`WWMIFixerConfig.withGameData`
writes each first in :attr:`lines` as ``<the shader's register> = ref <the slot's register>``, both from
:class:`TextureRegisters`. **Default**: empty
        )doc"))
        .def_readwrite("stashed", &AGRC::WWMIFixerConfig::Shader::stashed, py::doc(R"doc(
List[:class:`WWMIFixerConfig.Shader.Stashed`]: Registers bound to a texture the target's OTHER slot holds
while it draws -- captured off that slot's draw (:attr:`WWMIFixerConfig.stashRegs`). Written first.
**Default**: empty
        )doc"))
        .def_readwrite("gameRoles", &AGRC::WWMIFixerConfig::Shader::gameRoles, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: Register -> role: the shader's register bound to the texture of that
role -- the mod's file, or the source's own game texture downloaded (a role with neither binds nothing).
Written after :attr:`stashed`. **Default**: empty
        )doc"))
        .def_readwrite("passes", &AGRC::WWMIFixerConfig::Shader::passes, py::doc(R"doc(
List[List[Tuple[:class:`str`, :class:`str`]]]: Draw the component once per entry, each with
:attr:`lines` and that entry's own lines after them -- a source pass whose draws split by state, eg. a
see-through layer blended where the stencil marks a body behind it and drawn opaque everywhere else.
**Default**: empty, one draw with :attr:`lines`
        )doc"))
        .def_readwrite("passGame", &AGRC::WWMIFixerConfig::Shader::passGame, py::doc(R"doc(
List[:class:`str`]: :attr:`passPs` by :class:`GameShaders` row: per entry of :attr:`passes`, the row whose code that pass
draws with (:meth:`WWMIFixerConfig.withGameData` fills :attr:`passPs`). An empty entry keeps that pass's :attr:`passPs`.
**Default**: empty
        )doc"))
        .def_readwrite("passPs", &AGRC::WWMIFixerConfig::Shader::passPs, py::doc(R"doc(
List[:class:`str`]: Per entry of :attr:`passes`, the HLSL that pass draws with in place of :attr:`ps` -- a
source that draws one component with two shaders, eg. a solid one where the part is opaque and a
see-through one over a body. An empty entry, or none, keeps :attr:`ps`. **Default**: empty
        )doc"))
        .def_readwrite("onlyWith", &AGRC::WWMIFixerConfig::Shader::onlyWith, py::doc(R"doc(
:class:`str`: An output register (eg. ``"o3"``) a pass has to bind for the shader to draw there; a pass
that leaves it unbound (a depth pre-pass) draws the component with the game's own shader. **Default**:
``""``, every pass
        )doc"))
        .def_readwrite("registers", &AGRC::WWMIFixerConfig::Shader::registers, py::doc(R"doc(
List[:class:`WWMIFixerConfig.Binding`]: Bindings the component takes only while this shader draws it, after the
plan entry's own (the source's register layout). **Default**: empty
        )doc"))
        .def_readwrite("yieldToRabbitFX", &AGRC::WWMIFixerConfig::Shader::yieldToRabbitFX, py::doc(R"doc(
:class:`bool`: A mod whose sections for the component hand its maps to RabbitFX keeps the target slot's game
shader there (RabbitFX patches the game's shaders, never this one). **Default**: ``False``
        )doc"))
        .def_readwrite("after", &AGRC::WWMIFixerConfig::Shader::after, py::doc(R"doc(
Optional[:class:`WWMIFixerConfig.Shader.After`]: Draw the component only after another component's draws
in the same pass, whichever order the game draws the two slots in -- a see-through layer drawn first
blends over whatever is behind the body. **Default**: ``None``
        )doc"));

    py::class_<AGRC::WWMIFixerConfig::SourceComponent> sourceComponent(fixerConfig, "SourceComponent", R"doc(
How one SOURCE component is drawn on the target
    )doc");

    py::class_<AGRC::WWMIFixerConfig::SourceComponent::RabbitFX>(sourceComponent, "RabbitFX", R"doc(
RabbitFX on a component, carried across with the SOURCE's routing. RabbitFX patches a character's pixel
shaders to take a mod's maps in place of the shader's first texture samples, and decides which map
replaces which sample from the KIND of texture bound at ``ps-t0`` .. ``ps-t3`` when the mod runs its
``SetTextures``. With this set, the component keeps the mod's RabbitFX lines and right after each
``SetTextures`` the routing flags (``x187`` .. ``w187``) are pinned to what they are on the SOURCE's own
draw: read off the mod's own file for the role at each register, or :attr:`gameTags` where the mod
binds none. Valid only where the target slot samples the same roles in the same order as the source's
    )doc")
        .def(py::init<>())
        .def_readwrite("gameTags", &AGRC::WWMIFixerConfig::SourceComponent::RabbitFX::gameTags, py::doc(R"doc(
Dict[:class:`str`, :class:`int`]: ``ps-t0`` .. ``ps-t3`` -> the tag RabbitFX (8.3.2) gives the
SOURCE's own game texture there (3 for 1718.3, 4 for 1718.4, 5 for 1718.5, 6 for 1718.6, 0 for none)
        )doc"));

    sourceComponent
        .def(py::init<>())
        .def(py::init([](int slot, std::vector<AGRC::WWMIFixerConfig::Binding> bindings) {
            return AGRC::WWMIFixerConfig::SourceComponent{slot, std::move(bindings)};
        }), py::arg("slot"), py::arg("bindings"))
        .def_readwrite("slot", &AGRC::WWMIFixerConfig::SourceComponent::slot,
                        py::doc(":class:`int`: The target draw slot it goes through"))
        .def_readwrite("bindings", &AGRC::WWMIFixerConfig::SourceComponent::bindings,
                        py::doc("List[:class:`WWMIFixerConfig.Binding`]: The registers its command list binds, in this order"))
        .def_readwrite("sameFileSection", &AGRC::WWMIFixerConfig::SourceComponent::sameFileSection, py::doc(R"doc(
:class:`bool`: When another source component also goes through :attr:`slot`, draw this one from the
mod's own ``.ini`` as a second section on that draw instead of from a copy ``.ini``, whose shape-keyed
positions are never computed. **Default**: ``False``
        )doc"))
        .def_readwrite("shader", &AGRC::WWMIFixerConfig::SourceComponent::shader, py::doc(R"doc(
Optional[:class:`WWMIFixerConfig.Shader`]: The SOURCE's own pixel shader, when the target slot's cannot
draw what this component is. With one, the component binds the mod's art as the mod wrote it -- no
texture edit of the config reaches it, since the edits are for the TARGET's shader. It draws with the
shader on the passes the slot's :attr:`WWMIFixerConfig.slotPasses` name, and as before on any other
(an outline pass, :attr:`WWMIFixerConfig.extraPassRegs`). A :class:`WWMIFixerConfig.DrawReroute`'s
``as_`` entry may carry one too. **Default**: ``None``, the target slot's shader
        )doc"))
        .def_readwrite("rabbitFX", &AGRC::WWMIFixerConfig::SourceComponent::rabbitFX, py::doc(R"doc(
Optional[:class:`WWMIFixerConfig.SourceComponent.RabbitFX`]: RabbitFX on this component, carried
across with the SOURCE's routing. **Default**: ``None``, RabbitFX's lines are removed as
:attr:`WWMIFixerConfig.removedRegs` says
        )doc"))
        .def_readwrite("markLines", &AGRC::WWMIFixerConfig::SourceComponent::markLines, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: After each of the component's draws on the passes the slot's
:attr:`WWMIFixerConfig.slotPasses` name, the same draw again with NO pixel shader under these
``CustomShader`` lines -- to mark the pixels the component is the visible surface of in the stencil, eg.
``("depth_func", "equal")`` with a stencil write, for a later draw to test. Nothing is written to the
render targets. **Default**: empty, no such draw
        )doc"))
        .def_readwrite("overlay", &AGRC::WWMIFixerConfig::SourceComponent::overlay, py::doc(R"doc(
Optional[:class:`WWMIFixerConfig.Shader`]: After each of the component's draws on the passes the slot's
:attr:`WWMIFixerConfig.slotPasses` name, the same draw again with this shader -- its
:attr:`WWMIFixerConfig.Shader.registers`, constant buffers and :attr:`WWMIFixerConfig.Shader.lines` --
while the component itself keeps the target slot's shader and its plan bindings. For a part whose surface
the target's shader draws right and only the SOURCE's shader can finish, eg. a glow written into the
scene-colour target alone (``("mask[1]", "0")`` ... with ``("depth_func", "equal")``). Its
:attr:`WWMIFixerConfig.Shader.registers` bind the mod's own files, unedited. Only its ``name``, ``ps``,
``constantBuffers``, ``registers``, ``lines``, ``onlyWith`` and ``yieldToRabbitFX`` are read.
**Default**: ``None``, no such draw
        )doc"))
        .def_readwrite("offPasses", &AGRC::WWMIFixerConfig::SourceComponent::offPasses, py::doc(R"doc(
List[:class:`str`]: Passes of the target slot (pixel shader hashes, as in
:attr:`WWMIFixerConfig.slotPasses`) that the component's draws are kept off -- eg. a slot's outline
pass, whose shell of an under-layer the target's outline shader draws through the layer over it.
**Default**: empty, every pass
        )doc"));

    py::class_<AGRC::WWMIFixerConfig::DrawReroute> drawReroute(fixerConfig, "DrawReroute", R"doc(
Some draws of one source component, sent through a different target slot -- see
:attr:`WWMIFixerConfig.drawReroutes`
    )doc");

    drawReroute
        .def(py::init<>())
        .def(py::init([](int from, int as, std::string split) {
            return AGRC::WWMIFixerConfig::DrawReroute{from, as, std::move(split)};
        }), py::arg("from_"), py::arg("as_"), py::arg("split"))
        .def_readwrite("from_", &AGRC::WWMIFixerConfig::DrawReroute::from,
                        py::doc(":class:`int`: The source component whose draws may go elsewhere"))
        .def_readwrite("as_", &AGRC::WWMIFixerConfig::DrawReroute::as, py::doc(R"doc(
:class:`int`: The :attr:`WWMIFixerConfig.plan` entry the rerouted draws are drawn by -- an id that is
not a source slot. Its slot and bindings say where they go and what they bind
        )doc"))
        .def_readwrite("split", &AGRC::WWMIFixerConfig::DrawReroute::split, py::doc(R"doc(
:class:`str`: The :attr:`WWMIParserConfig.drawSplits` entry whose draws go: its
:attr:`WWMIParserConfig.DrawSplit.name`
        )doc"));

    py::class_<AGRC::WWMIFixerConfig::CreatedTexture>(fixerConfig, "CreatedTexture", R"doc(
A texture the fix INVENTS, eg. the flat material mask a target reads skin off
    )doc")
        .def(py::init<>())
        .def(py::init([](std::string role, std::optional<AGRC::Colour> colour, int size, std::pair<std::string, std::string> material) {
            return AGRC::WWMIFixerConfig::CreatedTexture{std::move(role), colour.value_or(AGRC::Colour()), size, std::move(material)};
        }), py::arg("role"), py::arg("colour") = py::none(), py::arg("size") = 16,
            py::arg("material") = std::pair<std::string, std::string>())
        .def_readwrite("material", &AGRC::WWMIFixerConfig::CreatedTexture::material, py::doc(R"doc(
Tuple[:class:`str`, :class:`str`]: The target's component and :class:`MaterialColours` type its colour is read from, eg.
``("component3", "torsoMask.skin")`` (:meth:`WWMIFixerConfig.withGameData`). Empty: :attr:`colour` is the config's
        )doc"))
        .def_readwrite("role", &AGRC::WWMIFixerConfig::CreatedTexture::role,
                        py::doc(":class:`str`: The role a binding names it by, eg. ``\"SkinMask\"``"))
        .def_readwrite("colour", &AGRC::WWMIFixerConfig::CreatedTexture::colour,
                        py::doc(":class:`Colour`: The solid colour it is filled with"))
        .def_readwrite("size", &AGRC::WWMIFixerConfig::CreatedTexture::size,
                        py::doc(":class:`int`: Its width and height. **Default**: ``16``"));

    py::class_<AGRC::WWMIFixerConfig::RegRemoval>(fixerConfig, "RegRemoval", R"doc(
A register line a remapped section drops -- see :attr:`WWMIFixerConfig.removedRegs`
    )doc")
        .def(py::init<>())
        .def(py::init([](std::string reg, std::string valuePrefix) {
                 return AGRC::WWMIFixerConfig::RegRemoval{std::move(reg), std::move(valuePrefix)};
             }), py::arg("reg"), py::arg("valuePrefix") = "")
        .def_readwrite("reg", &AGRC::WWMIFixerConfig::RegRemoval::reg,
                        py::doc(":class:`str`: The register (or key) to drop, eg. ``ResourceBlendBufferOverride``"))
        .def_readwrite("valuePrefix", &AGRC::WWMIFixerConfig::RegRemoval::valuePrefix, py::doc(R"doc(
:class:`str`: Drop the line only when its value begins with this, ignoring case and leading space.
Empty (the default) drops every value
        )doc"));

    py::class_<AGRC::WWMIFixerConfig::TexEditContext>(fixerConfig, "TexEditContext", R"doc(
What a :class:`WWMIFixerConfig.TexEdit`'s filter factory is told about the mod being fixed
    )doc")
        .def(py::init<>())
        .def_readwrite("iniFolder", &AGRC::WWMIFixerConfig::TexEditContext::iniFolder,
                        py::doc(":class:`str`: The folder of the ``.ini`` being fixed"))
        .def_readwrite("positionFile", &AGRC::WWMIFixerConfig::TexEditContext::positionFile,
                        py::doc(":class:`str`: The mod's position buffer"))
        .def_readwrite("texcoordFile", &AGRC::WWMIFixerConfig::TexEditContext::texcoordFile,
                        py::doc(":class:`str`: The mod's texcoord buffer"))
        .def_readwrite("indexFile", &AGRC::WWMIFixerConfig::TexEditContext::indexFile,
                        py::doc(":class:`str`: The mod's index buffer"))
        .def_readwrite("drawRanges", &AGRC::WWMIFixerConfig::TexEditContext::drawRanges,
                        py::doc("Dict[:class:`int`, List[Tuple[:class:`int`, :class:`int`]]]: Source component -> the (count, start) ranges the mod draws it with"))
        .def_readwrite("fileOfRole", &AGRC::WWMIFixerConfig::TexEditContext::fileOfRole, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: The file each role resolved to -- the mod's own, or the one its
fallback download lands. A filter may need ANOTHER role's texture
        )doc"))
        // settable, so a Python fixer hands its filters the context the compiled one does (the context keeps the
        // ModType alive: it holds a plain pointer)
        .def_property("source", py::cpp_function([](const AGRC::WWMIFixerConfig::TexEditContext& self) { return self.source; },
                                                 py::return_value_policy::reference),
                      py::cpp_function([](AGRC::WWMIFixerConfig::TexEditContext& self, const AGRC::ModType* type) { self.source = type; },
                                       py::keep_alive<1, 2>()),
                      py::doc("Optional[:class:`ModType`]: The character fixed from, or ``None`` outside a fix -- a filter reads the asset tables off it"))
        .def_property("target", py::cpp_function([](const AGRC::WWMIFixerConfig::TexEditContext& self) { return self.target; },
                                                 py::return_value_policy::reference),
                      py::cpp_function([](AGRC::WWMIFixerConfig::TexEditContext& self, const AGRC::ModType* type) { self.target = type; },
                                       py::keep_alive<1, 2>()),
                      py::doc("Optional[:class:`ModType`]: The character fixed to, or ``None`` outside a fix"))
        .def_readwrite("fromVersion", &AGRC::WWMIFixerConfig::TexEditContext::fromVersion,
                       py::doc("Optional[:class:`Version`]: The run's version of the source"))
        .def_readwrite("toVersion", &AGRC::WWMIFixerConfig::TexEditContext::toVersion,
                       py::doc("Optional[:class:`Version`]: The run's version of the target"));

    py::class_<AGRC::WWMIFixerConfig::TexEdit>(fixerConfig, "TexEdit", R"doc(
An edit the fix makes to a role's texture before binding it -- see :attr:`WWMIFixerConfig.texEdits`
    )doc")
        .def(py::init<>())
        .def(py::init([](std::string role, std::string name, const py::object &makeFilter, bool compress) {
                 AGRC::WWMIFixerConfig::TexEdit edit;
                 edit.role = std::move(role);
                 edit.name = std::move(name);
                 edit.makeFilter = toPyRefFunction<AGRC::TexEditor::Filter(const AGRC::WWMIFixerConfig::TexEditContext&)>(makeFilter);
                 edit.compress = compress;
                 return edit;
             }), py::arg("role"), py::arg("name"), py::arg("makeFilter"), py::arg("compress") = false)
        .def_readwrite("role", &AGRC::WWMIFixerConfig::TexEdit::role,
                        py::doc(":class:`str`: The role whose texture is edited"))
        .def_readwrite("name", &AGRC::WWMIFixerConfig::TexEdit::name, py::doc(R"doc(
:class:`str`: A short name for the edit, part of the written file's name. Two edits of one role need
two names, or the second overwrites the first
        )doc"))
        // A property over toPyRefFunction rather than def_readwrite: pybind11's own conversion of a
        // filter hands it a COPY of the texture, so an edit written in Python would save the
        // unedited texture (see GIMIComponentFixerConfig.diffuseEdits)
        .def_property("makeFilter",
            [](const AGRC::WWMIFixerConfig::TexEdit &self) {
                return fromPyRefFunction<AGRC::TexEditor::Filter(const AGRC::WWMIFixerConfig::TexEditContext&),
                                         PyTexFilter(const AGRC::WWMIFixerConfig::TexEditContext&)>(self.makeFilter);
            },
            [](AGRC::WWMIFixerConfig::TexEdit &self, const PyOptionalCallable<PyTexFilter(const AGRC::WWMIFixerConfig::TexEditContext&)> &makeFilter) {
                self.makeFilter = toPyRefFunction<AGRC::TexEditor::Filter(const AGRC::WWMIFixerConfig::TexEditContext&)>(makeFilter);
            }, py::doc(R"doc(
Callable[[:class:`WWMIFixerConfig.TexEditContext`], Callable[[:class:`CppTextureFile`], ``None``]]:
Builds the filter for THIS mod -- a factory rather than a filter, because an edit may depend on the
mod's own geometry. The filter is handed the texture itself and edits it in place
            )doc"))
        .def_readwrite("compress", &AGRC::WWMIFixerConfig::TexEdit::compress, py::doc(R"doc(
:class:`bool`: Whether to re-encode to the source's compressed format. **Default**: ``False``,
because a mask is CODES and BCn would move them
        )doc"));

    fixerConfig
        .def(py::init<>())
        .def_readwrite("targetId", &AGRC::WWMIFixerConfig::targetId,
                        py::doc(":class:`ModTypeId`: The character fixed TO -- whose ``vb0`` hash, slot windows, vertex-group offsets and shape-key checksum the library's WuWa tables hold"))
        .def_readwrite("version", &AGRC::WWMIFixerConfig::version,
                        py::doc(":class:`str`: The TARGET's game version, used when the run asks for none. Empty (the default) is the target's newest model -- the version of its newest ``vb0`` row -- so a fix follows the tables to a new model of the target without a change here"))
        .def_readwrite("slotPasses", &AGRC::WWMIFixerConfig::slotPasses, py::doc(R"doc(
List[List[:class:`str`]]: Per TARGET slot, the pixel shaders that bind that slot's character
textures -- read off a frame dump of the target. A slot's command list binds on every one of them
and no other pass
        )doc"))
        .def_readwrite("plan", &AGRC::WWMIFixerConfig::plan, py::doc(R"doc(
Dict[:class:`int`, :class:`WWMIFixerConfig.SourceComponent`]: Source component -> how it is drawn on
the target. A source component the mod has a section for but this does not name is dropped. Several
sources may name one target slot: that is the merge, and which lands in the mod's own ``.ini`` and
which in a copy follows the source components' numeric order
        )doc"))
        .def_readwrite("drawReroutes", &AGRC::WWMIFixerConfig::drawReroutes, py::doc(R"doc(
List[:class:`WWMIFixerConfig.DrawReroute`]: Draws that go through a different target slot than the
rest of their component: the draws the parser picked out for one of its
:attr:`WWMIParserConfig.drawSplits`. A component the parser picked no draws of is left whole.
**Default**: none
        )doc"))
        .def_readwrite("sourceTextures", &AGRC::WWMIFixerConfig::sourceTextures, py::doc(R"doc(
:class:`WWMITextureFacts`: Every fact about the SOURCE character's own textures -- its hashes by
role, its register layout per component, its pixel thumbprints, and where its game textures are
downloaded from.

The same object :attr:`WWMIParserConfig.textures` takes, so a character states these once
        )doc"))
        .def_readwrite("createdTextures", &AGRC::WWMIFixerConfig::createdTextures,
                        py::doc("List[:class:`WWMIFixerConfig.CreatedTexture`]: The textures the fix invents, each bound wherever a binding names its role"))
        .def_readwrite("zeroShapeKeyStream", &AGRC::WWMIFixerConfig::zeroShapeKeyStream, py::doc(R"doc(
:class:`bool`: Whether every remapped draw binds a zero shape-key offset stream, at
:attr:`IniKeywords.WWMIShapeKeyStreamReg`, :attr:`WWMINames.ShapeKeyStreamStride` bytes a vertex. **Default**: ``False``
        )doc"))
        .def_readwrite("hiddenObjs", &AGRC::WWMIFixerConfig::hiddenObjs, py::doc(R"doc(
List[:class:`str`]: The hash-only mod objects whose sections are commented out of the mod's own text
and copied nowhere, eg. the two shape-key overrides when the mod's keys cannot be retargeted. **Default**: none
        )doc"))
        .def_readwrite("sourceLabels", &AGRC::WWMIFixerConfig::sourceLabels,
                        py::doc("Dict[:class:`int`, :class:`str`]: Source component -> a label for the log"))
        .def_readwrite("targetLabels", &AGRC::WWMIFixerConfig::targetLabels,
                        py::doc("Dict[:class:`int`, :class:`str`]: Target slot -> a label for the log"))
        .def_readwrite("copyPreamble", &AGRC::WWMIFixerConfig::copyPreamble,
                        py::doc(":class:`str`: What every generated ``.ini`` copy opens with"))
        .def_readwrite("sourceVersion", &AGRC::WWMIFixerConfig::sourceVersion, py::doc(R"doc(
:class:`str`: The SOURCE's own game version, when the pair is not filed under one; empty falls back
to :attr:`version`. A reverse-then-forward lookup of a value both characters share (a shape-key
checksum) answers the wrong character when asked at the target's version
        )doc"))
        .def_readwrite("sourceVersionByVb0", &AGRC::WWMIFixerConfig::sourceVersionByVb0, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: The SOURCE's version per ``vb0`` hash (lower case), read off the mod's
own slot sections -- for a character whose skeleton was renumbered between game versions, so each mod
takes the vertex group remap row filed at the version it was exported at. Empty (the default) keeps
:attr:`sourceVersion` for every mod
        )doc"))
        .def_readwrite("sourceVersionFromVb0", &AGRC::WWMIFixerConfig::sourceVersionFromVb0, py::doc(R"doc(
:class:`bool`: Whether :attr:`sourceVersionByVb0` is read off the source's own ``vb0`` rows in the library's hashes
(:meth:`vb0VersionsOf`, by :meth:`withGameData`). **Default**: ``False``
        )doc"))
        .def_readwrite("shapeKeyOrder", &AGRC::WWMIFixerConfig::shapeKeyOrder, py::doc(R"doc(
:class:`list` [:class:`int`]: For each of the target's shape-key slots, the source key it takes (``-1``: none).
The game drives shape keys -- blinks, mouth shapes, expressions -- by slot, and two skins of one character can
number them differently; with this set the fix writes the mod's shape-key buffers again in the target's order
and binds the copies. Slots past the end (a mod's custom keys) are left alone. **Default**: ``[]``
        )doc"))
        .def_readwrite("shapeKeyDispatchSize", &AGRC::WWMIFixerConfig::shapeKeyDispatchSize, py::doc(R"doc(
:class:`str`: The TARGET's original shape-key dispatch height (its ``Metadata.json``'s
``shapekeys.dispatch_y``), written into a retargeted mod's ``shapekey_dispatch_size_y_original_batch0`` /
``_batch1``. WWMI picks the batch by that value, and the source's leaves most of the mod's offsets
unloaded on the target. Empty (the default) leaves them alone
        )doc"))
        .def_readwrite("filterIndices", &AGRC::WWMIFixerConfig::filterIndices, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: Shader hash -> the ``filter_index`` to tag it with, read off ``DrawPasses``' tags by
:meth:`withGameData`; the fix refuses a pass shader with none (:meth:`ShaderFilterTags.addGiven`). Every fix tagging a
shader must agree on its value, since 3dmigoto keys a ``[ShaderOverride]`` by its hash across every loaded ``.ini``
        )doc"))
        .def_readwrite("flatLeftToGame", &AGRC::WWMIFixerConfig::flatLeftToGame, py::doc(R"doc(
Set[:class:`str`]: Roles of :attr:`WWMITextureFacts.regionRoles` whose refused file is left to the GAME
rather than replaced by the source's own download
        )doc"))
        .def_readwrite("removedRegs", &AGRC::WWMIFixerConfig::removedRegs, py::doc(R"doc(
List[:class:`WWMIFixerConfig.RegRemoval`]: Register lines a remapped section drops, on top of what
the template removes anyway -- eg. the three ``Resource...Override = ref ...`` lines of a source past
256 merged bones, which undo the remap, and RabbitFX's resource lines. **Default**: :meth:`gameRemovedRegs`
        )doc"))
        .def_static("gameRemovedRegs", &AGRC::WWMIFixerConfig::gameRemovedRegs, py::doc(R"doc(
The lines every remapped section drops (:attr:`removedRegs`' default): the three ``Resource...Override = ref ...`` of a
source past 256 bones, and RabbitFX's ``run = CommandList\RabbitFX\SetTextures`` and the six maps it reads

Returns
-------
List[:class:`WWMIFixerConfig.RegRemoval`]
    The lines
        )doc"))
        .def_readwrite("remappedSkeletonRemovals", &AGRC::WWMIFixerConfig::remappedSkeletonRemovals, py::doc(R"doc(
List[:class:`WWMIFixerConfig.RegRemoval`]: Register lines a remapped section also drops when the TARGET is past 256
bones: the mod's own merge list, the two other routes to it (``CheckTextureOverride`` on ``vs-cb3`` / ``vs-cb4``)
and its own bindings of the merged skeletons, each matched by value prefix. Each would bind the MOD's skeleton, sized
for the source. **Default**: those six lines, as WWMI Tools writes them
        )doc"))
        .def_readwrite("anchorChains", &AGRC::WWMIFixerConfig::anchorChains, py::doc(R"doc(
Dict[:class:`int`, List[:class:`int`]]: Chains of SOURCE vertex groups pinned to one bone each,
``{root: members}``: every member is remapped to whatever the ROOT maps to. Check a chain with
``Tools/Misc/Diagnostics/anchorSafety.py`` first -- a member another component also weights is
pinned there too
        )doc"))
        .def_readwrite("extraPassRegs", &AGRC::WWMIFixerConfig::extraPassRegs, py::doc(R"doc(
Dict[:class:`int`, Dict[:class:`str`, List[:class:`WWMIFixerConfig.Binding`]]]: Per target slot, per
pass, the bindings that pass takes INSTEAD of the plan's -- a slot's register layout is per shader
        )doc"))
        .def_readwrite("extraPassNoDraw", &AGRC::WWMIFixerConfig::extraPassNoDraw,
                        py::doc("Dict[:class:`int`, Set[:class:`str`]]: Target slot -> the passes of :attr:`extraPassRegs` its draw is not re-issued on"))
        .def_readwrite("passVertexShaders", &AGRC::WWMIFixerConfig::passVertexShaders, py::doc(R"doc(
Dict[:class:`str`, List[:class:`str`]]: Each pass (pixel shader) mapped to the VERTEX shaders it is
drawn with. Set, the fix tags those vertex shaders and guards ``vs == ...`` instead of tagging the
pixel shader -- which RabbitFX also tags, so a ``ps`` tag switches its effects off. A pass left out
of a non-empty map is an error
        )doc"))
        .def_readwrite("sharedMeshes", &AGRC::WWMIFixerConfig::sharedMeshes, py::doc(R"doc(
Dict[:class:`str`, Dict[:class:`str`, List[:class:`WWMIFixerConfig.Binding`]]]: Other meshes the
character draws, by their own ``vb0`` hash -- ``{mesh hash: {pass: bindings}}``
        )doc"))
        .def_readwrite("stashRegs", &AGRC::WWMIFixerConfig::stashRegs, py::doc(R"doc(
Dict[:class:`int`, List[:class:`str`]]: Registers to capture off the GAME's draw of a target slot nothing of
the mod is drawn through, on passes that bind ``o3``; a :attr:`Shader.stashed` entry binds the capture, and
:meth:`withGameData` adds each one's register here. **Default**: empty
        )doc"))
        .def_readwrite("hiddenMeshes", &AGRC::WWMIFixerConfig::hiddenMeshes, py::doc(R"doc(
List[:class:`str`]: Other meshes the TARGET draws that a mod of her replaces, by their own ``vb0``
hash: each is skipped while the mod is enabled (no index window, so wherever it is drawn)
        )doc"))
        .def_readwrite("meshCounterparts", &AGRC::WWMIFixerConfig::meshCounterparts, py::doc(R"doc(
List[:class:`str`]: Other meshes the SOURCE and the TARGET both draw, by the hash type both characters file
them under in the library's hashes -- also the mod object the parser places their sections in
(:attr:`WWMIParserConfig.hashOnlyObjs`). Each such section is copied into the fix's block on the target's hash
of the same type. **Default**: empty
        )doc"))
        .def_readwrite("texEdits", &AGRC::WWMIFixerConfig::texEdits,
                        py::doc("List[:class:`WWMIFixerConfig.TexEdit`]: Edits the fix makes to a role's texture before binding it"))
        .def_readwrite("cleanTexcoords", &AGRC::WWMIFixerConfig::cleanTexcoords, py::doc(R"doc(
:class:`bool`: Bind the remapped sections to a CLEANED copy of the mod's texcoord buffer (a NaN
second UV zeroed, a U in the next tile folded back). **Default**: ``False``
        )doc"))
        .def_readwrite("sourceVgMaps", &AGRC::WWMIFixerConfig::sourceVgMaps, py::doc(R"doc(
Dict[:class:`int`, List[:class:`int`]]: The SOURCE's ``vg_map`` per component, for a mod from before
WWMI's merged skeleton (its blend holds per-component LOCAL ids); without it such a mod is refused
        )doc"))
        .def_readwrite("mergedSkeletonSlots", &AGRC::WWMIFixerConfig::mergedSkeletonSlots,
                        py::doc(":class:`int`: The float4 slots of the merged skeleton buffers declared for a legacy mod. **Default**: ``768``"))
        .def_readwrite("carryByRegisterRole", &AGRC::WWMIFixerConfig::carryByRegisterRole, py::doc(R"doc(
:class:`bool`: Carry a mod's own texture line that puts a texture into a register of a different kind by
the REGISTER's role on the source rather than the texture's -- so a toggle that binds a diffuse into a
ramp or detail slot keeps doing so on the target. **Default**: ``False``
        )doc"))
        .def_readwrite("targetInfluences", &AGRC::WWMIFixerConfig::targetInfluences, py::doc(R"doc(
:class:`int`: How many bone influences a vertex the TARGET's draws read, when it differs from the mod's
own. The fix's remapped ``Blend.buf`` (and, past 256 bones, its ``BlendRemapVertexVG.buf``) is written at
this many -- the mod's own first, the rest bone 0 with weight 0 -- and the generated section's ``stride``
and the blend remap's ``weights_per_vertex_count`` follow. A 4-influence character drawn through an
8-influence skin's shaders otherwise has every vertex read across its neighbour's bytes. When it is
FEWER, each vertex keeps its heaviest slots, their weights scaled to the same total. **Default**:
``0``, the mod's own count
        )doc"))
        .def_readwrite("currentPoseInCb3Only", &AGRC::WWMIFixerConfig::currentPoseInCb3Only, py::doc(R"doc(
:class:`bool`: Treat a draw whose ``vs-cb3`` carries the bone-data marker while its ``vs-cb4`` does not as
one whose CURRENT pose sits in ``vs-cb3`` -- merged into the main skeleton and bound the remapped main
one -- as WWMI Tools' own template does. For a target with passes like that (Lynae's depth passes),
where reading ``vs-cb3`` as the previous pose drops thin layered parts out. **Default**: ``False``
        )doc"));

    m.def("makeWWMIParser", [](const AGRC::WWMIParserConfig &config) {
        return PyIniParseFactory{AGRC::makeWWMIParser(config)};
    }, py::arg("config"), py::doc(R"doc(
Builds the parser for a mod of one Wuthering Waves character -- see :class:`WWMIParserConfig`

:param config: The character's config
:returns: A parser factory, for :meth:`CppStrategyOverrides.setParser` or the builder table
    )doc"));

    m.def("makeWWMIFixer", [](const AGRC::WWMIFixerConfig &config) {
        return PyIniFixFactory{AGRC::makeWWMIFixer(config)};
    }, py::arg("config"), py::doc(R"doc(
Builds the fixer that remaps a mod of one Wuthering Waves character onto another -- see
:class:`WWMIFixerConfig`

The fourth fixer template, next to :func:`makeGIMICharFixer`, :func:`makeGIMIComponentFixer` and
:func:`makeGIMIMergeFixer`, and the first for a game other than Genshin: one fixer for the pair,
every source component of the mod's ``.ini`` handled by it, the target's components being draw
slots rather than mod types of their own

The fix is a LIST of edits, built for each ``.ini`` from 'config' and what the mod draws, each step named
(:attr:`BaseIniGraphGroupEdit.name`): ``copies``, ``toggles``, ``slotRemap``, the edits on the target's objects by
kind (``textureListCalls``, ``sharedBypass``, ``modTexLines``, ``additions``, ``rerouteFilters``, ``removedLines``,
``rabbitFXPins``, ``slotValues``, ``hashes``, ``dispatchSize``, ``shapeKeyLists``, ``afterShaders``, ``markLines``,
``overlays``, ``rerouteShaders``, ``offPasses``), ``blend``, ``rebind``, ``fixSections`` -- the fix's own sections,
into graph ``(0, "", "fixSections")``, one step per kind of section after it (``textureLists``, ``glowLists``,
``linkedState``, ``declaredFiles``, ``skeletonSections``, ``blendRemapSections``, ``bypassSections``,
``hideSections``, ``stashResources``, ``passTags``, ``sharedMeshes``, ``hiddenMeshes``, ``shaderDraws``,
``rabbitFXPinLists``), so one kind is read, removed or replaced as one step -- then
``shaderFiles``, ``fixFiles``, ``latch`` and ``texEdits``. Change a fix by rewriting that list with
:meth:`CppIniFixFactory.withList`:

.. code-block:: python

    factory = FRB.ModTypes["Lynae"].value.iniFixBuilder.factoryOf("Lynae", "LynaePeppermint")
    def rewrite(steps):
        at = max(i for i, step in enumerate(steps) if step.name == "fixSections") + 1
        return steps[:at] + [myEdit] + steps[at:]
    FRB.CppStrategyOverrides.setFixer("Lynae", "LynaePeppermint", factory.withList(rewrite))

:param config: The pair's config
:returns: A fixer factory, for :meth:`CppStrategyOverrides.setFixer` or the builder table
    )doc"));

    fixerConfig.def_readwrite("roleKinds", &AGRC::WWMIFixerConfig::roleKinds, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: Each texture role -> its KIND: ``upperDiffuse``, ``lowerDiffuse`` and ``panelDiffuse`` all
have the kind ``Diffuse``. :meth:`withGameData` fills it from both characters' :class:`TextureKinds` rows, and refuses a
role the config names that has none. **Default**: empty
        )doc"));
    fixerConfig.def_static("mergedSkeletonSlotsFor", &AGRC::WWMIFixerConfig::mergedSkeletonSlotsFor, py::arg("bones"), R"doc(
The size of a merged skeleton for ``bones`` bones, in float4 slots: three a bone, rounded up to WWMI Tools' steps
(:attr:`WWMINames.MergedSkeletonStep`) -- what :meth:`withGameData` fills :attr:`mergedSkeletonSlots` with, for the
target's :meth:`ModType.getMergedBoneCount`

:param bones: The bones
:returns: :class:`int`
    )doc");
    fixerConfig.def("componentOf", &AGRC::WWMIFixerConfig::componentOf, py::arg("key"), R"doc(
The SOURCE component a :attr:`plan` entry draws: for a reroute's entry (:attr:`DrawReroute.as_`), the component it takes
its draws from; else the entry's own key

:param key: The plan entry
:returns: :class:`int`
    )doc");
    fixerConfig.def("kindOf", &AGRC::WWMIFixerConfig::kindOf, py::arg("role"), R"doc(
A role's kind (:attr:`roleKinds`): ``""`` for a role with no kind, or none known

:param role: The role
:returns: Its kind
    )doc");
    fixerConfig.def("registerOf", &AGRC::WWMIFixerConfig::registerOf, py::arg("bindings"), py::arg("role"), R"doc(
The register ``bindings`` bind ``role`` at: the first of its own, else the one they bind its KIND at (:meth:`kindOf`)
-- only where that is one register, since two roles of one kind (``irisDiffuse``, ``eyeDiffuse``) may sit at
different ones. A ``null`` binding binds nothing

:param bindings: A component's bindings, eg. its plan row's
:param role: The role
:returns: The register, or ``None``
    )doc");
    fixerConfig.def_static("ofFactory", [](const PyIniFixFactory& factory) {
        return AGRC::WWMIFixerConfig::ofFactory(factory.factory);
    }, py::arg("factory"), R"doc(
The config a fixer factory from :func:`makeWWMIFixer` was made with, or ``None`` for any other factory. A shipped fix's
factory is its mod type's builder row (:meth:`IniFixBuilder.factoryOf`), so this reads what a shipped fix does -- or
starts a variant of it (change a field, hand it to :func:`makeWWMIFixer`)

:param factory: :class:`CppIniFixFactory`: the factory
:returns: Optional[:class:`WWMIFixerConfig`]
    )doc");
    fixerConfig.def("withGameData", &AGRC::WWMIFixerConfig::withGameData, py::arg("source"), py::arg("target"),
                    py::arg("fromVersion") = py::none(), py::arg("toVersion") = py::none(), R"doc(
This config with its GAME DATA read off the versioned asset tables, at the run's versions -- what a config carries is the
pair's choices, and what the game draws with comes from the tables, so a fix runs at any version the tables have rows for:

* :attr:`slotPasses`, :attr:`passVertexShaders` and :attr:`filterIndices`, from the target's :class:`DrawPasses` (each
  slot's ``main``, ``main2``, ... and ``other``, ``other2``, ... passes, their vertex shaders and ``filter_index`` tags)
* the keys of :attr:`extraPassRegs` and :attr:`extraPassNoDraw` written as a pass NAME (``other``, ``other2``), as the
  pass's pixel shader
* :attr:`shapeKeyDispatchSize` (:class:`ShapeKeyDispatchSizes`) and :attr:`shapeKeyOrder` (:class:`ShapeKeyRemaps`)
* each :class:`WWMIFixerConfig.Shader`'s code and constant buffers, for one naming its :class:`GameShaders` row
  (:attr:`WWMIFixerConfig.Shader.game`, and a pass's own code by :attr:`WWMIFixerConfig.Shader.passGame`)
* the source's game textures' thumbprints (:attr:`sourceTextures`, :class:`TextureThumbprints`)
* a :class:`WWMIFixerConfig.CreatedTexture`'s colour, for one naming its :class:`MaterialColours` row
* :attr:`sourceVgMaps`: the source's :class:`VGRemaps` rows from each component onto her merged skeleton (``""``), at her
  oldest ``vb0``'s version
* each :class:`Shader`'s registers left empty, from :class:`TextureRegisters`: the source's for the draw it was read
  from (:attr:`Shader.component`), the target's for the slot it captures or refs off (:attr:`Shader.stashed`,
  :attr:`Shader.refRoles`)
* :attr:`stashRegs`, from the shaders' :attr:`Shader.stashed`
* :attr:`sourceTextures`' download game folder, folder and prefix, when empty: the source's game and name
* :attr:`roleKinds`, from both characters' :class:`TextureKinds` -- refusing a role the config names with no row
* :attr:`sourceVersionByVb0`, when :attr:`sourceVersionFromVb0` (see :meth:`vb0VersionsOf`)

A field the tables hold nothing for keeps the config's value. The compiled fixer calls this once per fix

:param source: :class:`ModType`: the source's mod type
:param target: :class:`ModType`: the target's mod type
:param fromVersion: Optional[:class:`Version`]: the source's version (the mod's) -- ``None`` reads each table's newest rows
:param toVersion: Optional[:class:`Version`]: the target's version (the fix's)
:returns: :class:`WWMIFixerConfig`
    )doc");
    fixerConfig.def_static("vb0VersionsOf", &AGRC::WWMIFixerConfig::vb0VersionsOf, py::arg("source"), R"doc(
Each ``vb0`` hash the library's hashes file a character's model under (lower case) -> its version

:param source: :class:`ModType`: the character's mod type
:returns: Dict[:class:`str`, :class:`str`]
    )doc");
    fixerConfig.def_static("newestVersionOf", &AGRC::WWMIFixerConfig::newestVersionOf, py::arg("target"), R"doc(
The version a fix onto a character is made at when the run asks for none: the newest of her newest model (the version
of her newest ``vb0`` row in the library's hashes) and the newest game version her :class:`DrawPasses` rows are filed
at. A game update that rehashes her shaders and moves no model of hers (Wuthering Waves 3.8) is a newer
:class:`DrawPasses` row, and a fix made by default reaches it; a lookup of a model table at that version still answers
her newest model's row

:param target: :class:`ModType`: the character's mod type
:returns: Optional[:class:`Version`]
    )doc");
    // ---- what a fix decides from the mod ----
    fixerConfig.def("shaderDraws", &AGRC::WWMIFixerConfig::shaderDraws, py::arg("component"), py::arg("usesRabbitFX"), R"doc(
Whether a component's :attr:`SourceComponent.shader` draws it: it has one, and does not yield to a mod that hands the
component's maps to RabbitFX (:attr:`Shader.yieldToRabbitFX`)

:param component: The source component
:param usesRabbitFX: Whether the mod's sections of the component name RabbitFX
:returns: Whether the shader draws it
    )doc");
    fixerConfig.def("bindingsOf", &AGRC::WWMIFixerConfig::bindingsOf, py::arg("component"), py::arg("shaderDrawn"), R"doc(
A component's bindings: its :attr:`plan` entry's, with its shader's :attr:`Shader.registers` in place of the ones at the
same register while the shader draws it (:meth:`shaderDraws`)

:param component: The source component
:param shaderDrawn: Whether its shader draws it
:returns: List[:class:`WWMIFixerConfig.Binding`]
    )doc");
    fixerConfig.def("shaderLineRoles", &AGRC::WWMIFixerConfig::shaderLineRoles, py::arg("component"), R"doc(
The roles a component's :attr:`SourceComponent.shader` names in a ``role <name>`` line, of its own lines and its passes'

:param component: The source component
:returns: List[:class:`str`]: the roles, in the order the lines are written
    )doc");
    fixerConfig.def("passOnlyRoles", &AGRC::WWMIFixerConfig::passOnlyRoles, py::arg("drawn"), R"doc(
The roles only a slot's OTHER passes (:attr:`extraPassRegs`, of a slot a drawn component goes through), a
:attr:`sharedMeshes` mesh or a drawn component's :attr:`SourceComponent.overlay` bind -- each once, in that order

:param drawn: Set[:class:`int`]: the source components the mod draws
:returns: List[:class:`str`]
    )doc");

    py::class_<AGRC::WWMIFixerConfig::Download>(fixerConfig, "Download", R"doc(
A texture of the source's own game the fix downloads: from its :attr:`sourceTextures` folders, as
``<prefix><kind>RemapDL.dds``, declared under the resource :meth:`IniNamingTools.getRemapDLResourceName` names
    )doc")
        .def(py::init<>())
        .def_readwrite("url", &AGRC::WWMIFixerConfig::Download::url, ":class:`str`: Where it is downloaded from")
        .def_readwrite("file", &AGRC::WWMIFixerConfig::Download::file, ":class:`str`: The file it is saved as, beside the ``.ini``")
        .def_readwrite("resource", &AGRC::WWMIFixerConfig::Download::resource, ":class:`str`: The resource section that declares it");
    fixerConfig.def("downloadOf", &AGRC::WWMIFixerConfig::downloadOf, py::arg("kind"), py::arg("hash"), R"doc(
The source's game texture of ``hash`` as a :class:`WWMIFixerConfig.Download` of the kind ``kind`` (``<Role>``, or
``<Role>Game`` for a default)

:param kind: The kind, eg. ``"UpperDiffuse"``
:param hash: The texture's hash in the source's download folder
:returns: :class:`WWMIFixerConfig.Download`
    )doc");

    py::class_<AGRC::WWMIFixerConfig::DeclaredFile>(fixerConfig, "DeclaredFile", R"doc(
A resource the fix declares for one of the mod's files that no resource of the ``.ini`` names (or that only a previous
fix named): ``[Resource<Role><Target>RemapRef]`` -- RemapRef, since an undo deletes the files a RemapFix section names
    )doc")
        .def(py::init<>())
        .def_readwrite("file", &AGRC::WWMIFixerConfig::DeclaredFile::file, ":class:`str`: The file, as the parser names it")
        .def_readwrite("rel", &AGRC::WWMIFixerConfig::DeclaredFile::rel,
                       ":class:`str`: The file relative to the ``.ini``'s folder, as the ``filename =`` line writes it -- "
                       "empty for the fix to compute it from :attr:`file`")
        .def_readwrite("resource", &AGRC::WWMIFixerConfig::DeclaredFile::resource, ":class:`str`: The resource section's name");

    py::class_<AGRC::WWMIFixerConfig::RoleResources>(fixerConfig, "RoleResources", R"doc(
What each texture role a mod's components bind resolves to -- see :meth:`WWMIFixerConfig.roleResources`
    )doc")
        .def(py::init<>())
        .def_readwrite("ofSlotRole", &AGRC::WWMIFixerConfig::RoleResources::ofSlotRole,
                       "Dict[Tuple[:class:`str`, :class:`int`], :class:`str`]: (role, source component) -> the resource that binds the mod's own file for it")
        .def_readwrite("ofRole", &AGRC::WWMIFixerConfig::RoleResources::ofRole,
                       "Dict[:class:`str`, :class:`str`]: role -> what stands in for it where no file of the mod's serves: a texture the fix invents, else the source's own game texture, downloaded")
        .def_readwrite("fileOfRole", &AGRC::WWMIFixerConfig::RoleResources::fileOfRole,
                       "Dict[:class:`str`, :class:`str`]: role -> the mod's file chosen for it")
        .def_readwrite("declared", &AGRC::WWMIFixerConfig::RoleResources::declared,
                       "List[:class:`WWMIFixerConfig.DeclaredFile`]: the resources the fix declares for the mod's files, in the order they were named")
        .def_readwrite("downloads", &AGRC::WWMIFixerConfig::RoleResources::downloads,
                       "Dict[:class:`str`, :class:`WWMIFixerConfig.Download`]: role -> the download standing in for it")
        .def_readwrite("leftToGame", &AGRC::WWMIFixerConfig::RoleResources::leftToGame,
                       "Set[:class:`str`]: the roles left to the game: a flat texture the parser refused, in :attr:`WWMIFixerConfig.flatLeftToGame`, with nothing else of the mod's")
        .def_readwrite("notes", &AGRC::WWMIFixerConfig::RoleResources::notes,
                       "List[:class:`str`]: what the decision has to say, for the log, in order")
        .def("of", &AGRC::WWMIFixerConfig::RoleResources::of, py::arg("role"), py::arg("component"), py::arg("ownShader") = false, R"doc(
The resource bound for ``role`` on ``component``: its own, else -- on a component its own shader draws -- the download
itself, else the stand-in

:param role: The role
:param component: The source component
:param ownShader: Whether the component's own shader draws it, which reads a download raw rather than a texture edit of it
:returns: Optional[:class:`str`]: the resource, or ``None`` for a role nothing binds
        )doc")
        .def("shared", &AGRC::WWMIFixerConfig::RoleResources::shared, py::arg("role"), py::arg("drawn"),
             py::arg("ownShader") = std::set<int>{}, R"doc(
The resource the first of ``drawn`` to bind ``role`` binds (:meth:`of`)

:param role: The role
:param drawn: Set[:class:`int`]: the source components the mod draws
:param ownShader: Set[:class:`int`]: the drawn components their own shader draws
:returns: Optional[:class:`str`]: the resource, or ``None`` for a role none binds
        )doc");
    fixerConfig.def("roleResources", &AGRC::WWMIFixerConfig::roleResources, py::arg("data"), py::arg("drawn"),
                    py::arg("rabbitFX"), py::arg("sourceName"), py::arg("toModName"), py::arg("iniFolder"), R"doc(
What each role resolves to on each component the mod draws.

Every role a drawn component's bindings name, then every :meth:`passOnlyRoles` role on every drawn component, is offered
the mod's file the parser chose for it on that component (:meth:`IniParseData.roleFile`). The file binds through the
resource the mod uses for it (:meth:`IniParseData.resourceOfFile`), or through one the fix declares
(:class:`WWMIFixerConfig.DeclaredFile`) when the mod names it nowhere -- or only in a previous fix, which the run's undo
removes. Then the roles the fix invents (:attr:`createdTextures`), and a stand-in download for every role of a drawn
component no file serves, every role a drawn component's shader names, and every pass-only role no component owns --
unless the role is left to the game

:param data: :class:`IniParseData`: the parser's data of the mod's ``.ini``
:param drawn: Set[:class:`int`]: the source components the mod draws
:param rabbitFX: Set[:class:`int`]: the drawn components whose sections name RabbitFX
:param sourceName: The source character's name
:param toModName: The target's name
:param iniFolder: The ``.ini``'s folder
:returns: :class:`WWMIFixerConfig.RoleResources`
    )doc");

    py::class_<AGRC::WWMIFixerConfig::GameDefaults>(fixerConfig, "GameDefaults", R"doc(
Where the source's own game texture is the default for a role -- see :meth:`WWMIFixerConfig.gameDefaults`
    )doc")
        .def(py::init<>())
        .def_readwrite("ofRole", &AGRC::WWMIFixerConfig::GameDefaults::ofRole,
                       "Dict[:class:`str`, :class:`str`]: role -> what the draws a section makes before it binds anything bind: the source's own game texture -- the role's stand-in when the mod has no file for it, else a download of its own")
        .def_readwrite("downloads", &AGRC::WWMIFixerConfig::GameDefaults::downloads,
                       "Dict[:class:`str`, :class:`WWMIFixerConfig.Download`]: role -> the source's game texture, downloaded as ``<prefix><Role>GameRemapDL.dds``")
        .def_readwrite("toggled", &AGRC::WWMIFixerConfig::GameDefaults::toggled,
                       "Dict[Tuple[:class:`str`, :class:`int`], :class:`str`]: (role, component) -> what binds the role outside the toggle the mod binds its own file under")
        .def_readwrite("edited", &AGRC::WWMIFixerConfig::GameDefaults::edited,
                       "Set[:class:`str`]: the roles whose download is the default, and so is edited as the mod's own file of the role would be")
        .def_readwrite("notes", &AGRC::WWMIFixerConfig::GameDefaults::notes,
                       "List[:class:`str`]: what the decision has to say, for the log, in order")
        .def("of", &AGRC::WWMIFixerConfig::GameDefaults::of, py::arg("role"), py::arg("component"), py::arg("ownShader"), R"doc(
The source's game texture of ``role`` as ``component`` binds it: the download itself for a component its own shader
draws, else :attr:`ofRole`'s

:param role: The role
:param component: The source component
:param ownShader: Whether the component's own shader draws it
:returns: :class:`str`: the resource
        )doc");
    fixerConfig.def("gameDefaults", &AGRC::WWMIFixerConfig::gameDefaults, py::arg("data"), py::arg("roles"), py::arg("drawn"),
                    py::arg("rerouteFrom"), py::arg("ownShader"), py::arg("sourceName"), R"doc(
THE SOURCE'S OWN GAME TEXTURE AS THE DEFAULT, where the mod's file reaches only some of a component's draws -- each a
place the original drew with the game's texture:

#. the draws a section makes BEFORE it binds anything (:meth:`IniParseData.lateBindings`): a role the mod has no file
   for is already the source's game texture (its stand-in), and a role it does serve takes a download of its own
#. a role bound only under a toggle that reaches none of the section's draws (:meth:`IniParseData.toggledBindings`) --
   unless a ``this`` binds it for every draw anyway
#. a role bound with one ``this`` under a toggle: off, the original draws with the game's texture

Nothing when the source has no download folder

:param data: :class:`IniParseData`: the parser's data of the mod's ``.ini``
:param roles: :class:`WWMIFixerConfig.RoleResources`: what the roles resolve to, with a reroute's entries copied from
    its source component
:param drawn: Set[:class:`int`]: the source components the mod draws, reroutes included
:param rerouteFrom: Dict[:class:`int`, :class:`int`]: a reroute that took draws -> the component it took them from
:param ownShader: Set[:class:`int`]: the drawn components their own shader draws
:param sourceName: The source character's name
:returns: :class:`WWMIFixerConfig.GameDefaults`
    )doc");

    py::class_<AGRC::WWMIFixerConfig::TexEditTargets>(fixerConfig, "TexEditTargets", R"doc(
Which resources each :attr:`WWMIFixerConfig.texEdits` edit applies to -- see :meth:`WWMIFixerConfig.texEditTargets`
    )doc")
        .def(py::init<>())
        .def_readwrite("targets", &AGRC::WWMIFixerConfig::TexEditTargets::targets,
                       "Dict[:class:`str`, Set[:class:`str`]]: role -> the resources (lower case) its edits apply to")
        .def_readwrite("rolesOf", &AGRC::WWMIFixerConfig::TexEditTargets::rolesOf,
                       "Dict[:class:`str`, List[:class:`str`]]: a resource (lower case) -> the roles edited from it, in the order they claimed it")
        .def_readwrite("notes", &AGRC::WWMIFixerConfig::TexEditTargets::notes,
                       "List[:class:`str`]: what the decision has to say, for the log, in order");
    fixerConfig.def("texEditTargets", &AGRC::WWMIFixerConfig::texEditTargets, py::arg("data"), py::arg("roles"),
                    py::arg("drawn"), py::arg("ownShader"), py::arg("iniPath"), R"doc(
Which resources each of :attr:`texEdits` applies to: the one its role resolves to (:meth:`RoleResources.shared`) and
every other variant the mod's toggles bind the role as -- unless another role's edit of another kind claimed that file
first, since a second claim would run that role's filter over it. An edit of a role with neither a file of the mod's nor
a stand-in download applies to nothing

:param data: :class:`IniParseData`: the parser's data of the mod's ``.ini``
:param roles: :class:`WWMIFixerConfig.RoleResources`: what the roles resolve to, a reroute's entries included
:param drawn: Set[:class:`int`]: the source components the mod draws
:param ownShader: Set[:class:`int`]: the drawn components their own shader draws
:param iniPath: The mod's ``.ini``
:returns: :class:`WWMIFixerConfig.TexEditTargets`
    )doc");

    py::class_<AGRC::WWMIFixerConfig::CarriedLine>(fixerConfig, "CarriedLine", R"doc(
One line a linked ``.ini`` sets on this file's draws, as the fix carries it -- see :meth:`WWMIFixerConfig.linkedLines`
    )doc")
        .def(py::init<>())
        .def_readwrite("condition", &AGRC::WWMIFixerConfig::CarriedLine::condition,
                       ":class:`str`: the condition it is set under, reading the fix's copies of the linked file's variables; empty for none")
        .def_readwrite("key", &AGRC::WWMIFixerConfig::CarriedLine::key,
                       ":class:`str`: the key it sets (eg. RabbitFX's glow map); empty for a texture set at the register of :attr:`role`")
        .def_readwrite("role", &AGRC::WWMIFixerConfig::CarriedLine::role,
                       ":class:`str`: the texture role the parser gave the line's file, for a line with no :attr:`key`")
        .def_readwrite("value", &AGRC::WWMIFixerConfig::CarriedLine::value,
                       ":class:`str`: the value: ``ref <resource>``, the resource naming the file in this ``.ini``");
    py::class_<AGRC::WWMIFixerConfig::LinkedState>(fixerConfig, "LinkedState", R"doc(
A linked file whose state the fix copies, so the carried lines' conditions read it -- see :meth:`WWMIFixerConfig.linkedLines`
    )doc")
        .def(py::init<>())
        .def_readwrite("ini", &AGRC::WWMIFixerConfig::LinkedState::ini, ":class:`str`: the linked ``.ini``")
        .def_readwrite("vars", &AGRC::WWMIFixerConfig::LinkedState::vars,
                       "Set[:class:`str`]: the variables of its state the fix copies: the conditions' and every one they depend on")
        .def_readwrite("suffix", &AGRC::WWMIFixerConfig::LinkedState::suffix,
                       ":class:`str`: what each copied variable's name takes on in the fix");
    py::class_<AGRC::WWMIFixerConfig::LinkedLines>(fixerConfig, "LinkedLines", R"doc(
What linked ``.ini`` files set on this file's draws, and how the fix carries it -- see :meth:`WWMIFixerConfig.linkedLines`
    )doc")
        .def(py::init<>())
        .def_readwrite("lines", &AGRC::WWMIFixerConfig::LinkedLines::lines,
                       "Dict[:class:`int`, List[:class:`WWMIFixerConfig.CarriedLine`]]: source component -> the lines carried onto its draws")
        .def_readwrite("states", &AGRC::WWMIFixerConfig::LinkedLines::states,
                       "List[:class:`WWMIFixerConfig.LinkedState`]: the linked files whose state the fix copies, in the order their first line was read")
        .def_readwrite("declared", &AGRC::WWMIFixerConfig::LinkedLines::declared,
                       "List[:class:`WWMIFixerConfig.DeclaredFile`]: the resources the fix declares for the linked files no resource of this ``.ini`` names")
        .def_readwrite("notes", &AGRC::WWMIFixerConfig::LinkedLines::notes,
                       "List[:class:`str`]: what the decision has to say, for the log, in order");
    fixerConfig.def("linkedLines", &AGRC::WWMIFixerConfig::linkedLines, py::arg("data"), py::arg("roles"), py::arg("drawn"),
                    py::arg("linkedInis"), py::arg("sourceName"), py::arg("toModName"), py::arg("iniFolder"), R"doc(
What linked ``.ini`` files set on this file's draws (:meth:`IniParseData.linkedLines`), on the components the mod draws:
each line's value names its file through the resource this ``.ini`` uses for it, or one the fix declares; its condition
reads the fix's copies of the linked file's variables (suffixed ``<Target>RemapRef``, then ``...RemapRef2``, per file),
whose state the fix copies

:param data: :class:`IniParseData`: the parser's data of the mod's ``.ini``
:param roles: :class:`WWMIFixerConfig.RoleResources`: what the roles resolve to; the names it declares are taken
:param drawn: Set[:class:`int`]: the source components the mod draws, reroutes included
:param linkedInis: List[:class:`IniFile`]: the run's ``.ini`` files linked to this one (:meth:`IniFile.linkedInis`)
:param sourceName: The source character's name
:param toModName: The target's name
:param iniFolder: The ``.ini``'s folder
:returns: :class:`WWMIFixerConfig.LinkedLines`
    )doc");

    fixerConfig.def("rabbitFXFlags", &AGRC::WWMIFixerConfig::rabbitFXFlags, py::arg("data"), py::arg("roles"),
                    py::arg("toggled"), py::arg("drawn"), py::arg("rerouteFrom"), py::arg("iniPath"), py::arg("iniFolder"), R"doc(
The routing flags RabbitFX's ``SetTextures`` sets on the SOURCE's own draw of each drawn component that keeps RabbitFX
(:attr:`SourceComponent.rabbitFX`) and whose sections run ``SetTextures`` -- from what that draw has BOUND at ``ps-t0``
.. ``ps-t3``, not from the maps the mod hands RabbitFX. Per register: the mod's own line (unless only a toggle sets it),
else the mod's ``this`` override of the game texture there, else the game texture itself (``RabbitFX.gameTags``)

:param data: :class:`IniParseData`: the parser's data of the mod's ``.ini``
:param roles: :class:`WWMIFixerConfig.RoleResources`: what the roles resolve to
:param toggled: Dict[Tuple[:class:`str`, :class:`int`], :class:`str`]: (role, component) -> what binds the role outside a
    toggle (:attr:`GameDefaults.toggled`)
:param drawn: Dict[:class:`int`, List[:class:`str`]]: each source component the mod draws (reroutes included) -> its sections
:param rerouteFrom: Dict[:class:`int`, :class:`int`]: a reroute that took draws -> the component it took them from
:param iniPath: The mod's ``.ini``
:param iniFolder: The ``.ini``'s folder
:returns: Dict[:class:`int`, List[:class:`int`]]: component -> its four flags (:meth:`RabbitFX.routingFlags`)
    )doc");

    py::class_<AGRC::WWMIFixerConfig::TextureLine>(fixerConfig, "TextureLine", R"doc(
One line of a texture list the fix writes -- see :meth:`WWMIFixerConfig.textureLists`
    )doc")
        .def(py::init<>())
        .def_readwrite("reg", &AGRC::WWMIFixerConfig::TextureLine::reg,
                       ":class:`str`: the register the line binds (``ib`` for a pass list's ``ib = null``)")
        .def_readwrite("value", &AGRC::WWMIFixerConfig::TextureLine::value, ":class:`str`: what it binds: a resource, or ``null``")
        .def_readwrite("role", &AGRC::WWMIFixerConfig::TextureLine::role,
                       ":class:`str`: the role it binds, or empty for a line that binds none (``null``, ``ib = null``)")
        .def_readwrite("raw", &AGRC::WWMIFixerConfig::TextureLine::raw,
                       ":class:`bool`: whether the list keeps the mod's own art unedited: a component its own shader draws, or an overlay")
        .def_readwrite("modToggle", &AGRC::WWMIFixerConfig::TextureLine::modToggle,
                       ":class:`bool`: whether :meth:`WWMIFixerConfig.toggleLists` may put the mod's own toggle of :attr:`value` "
                       "in the line's place; false for a line already binding a role outside its toggle (:attr:`GameDefaults.toggled`)")
        .def_readwrite("run", &AGRC::WWMIFixerConfig::TextureLine::run,
                       ":class:`str`: set by :meth:`WWMIFixerConfig.toggleLists`: the copy of the mod's toggle run in the line's place, "
                       "as ``run = <run>``");
    py::class_<AGRC::WWMIFixerConfig::TextureList>(fixerConfig, "TextureList", R"doc(
One texture list the fix writes, its lines in order -- see :meth:`WWMIFixerConfig.textureLists`
    )doc")
        .def(py::init<>())
        .def_readwrite("kind", &AGRC::WWMIFixerConfig::TextureList::kind,
                       ":class:`str`: ``\"main\"``, ``\"game\"`` and ``\"pass\"`` a drawn component's, ``\"sharedMesh\"`` one pass of a "
                       ":attr:`WWMIFixerConfig.sharedMeshes` mesh, ``\"overlay\"`` a :attr:`SourceComponent.overlay`'s bindings")
        .def_readwrite("component", &AGRC::WWMIFixerConfig::TextureList::component,
                       ":class:`int`: the source component, or for ``\"sharedMesh\"`` the mesh's place in :attr:`WWMIFixerConfig.sharedMeshes`")
        .def_readwrite("onPass", &AGRC::WWMIFixerConfig::TextureList::onPass,
                       ":class:`str`: the pass a ``\"pass\"`` or ``\"sharedMesh\"`` list binds on")
        .def_readwrite("passNum", &AGRC::WWMIFixerConfig::TextureList::passNum,
                       ":class:`int`: a ``\"pass\"`` or ``\"sharedMesh\"`` list's place among its slot's or mesh's passes")
        .def_readwrite("lines", &AGRC::WWMIFixerConfig::TextureList::lines, "List[:class:`WWMIFixerConfig.TextureLine`]: the lines");
    py::class_<AGRC::WWMIFixerConfig::ToggleList>(fixerConfig, "ToggleList", R"doc(
A copy of the mod's own texture override toggling a role's resources, run in a list's place -- see
:meth:`WWMIFixerConfig.toggleLists`
    )doc")
        .def(py::init<>())
        .def_readwrite("name", &AGRC::WWMIFixerConfig::ToggleList::name, ":class:`str`: the list's name")
        .def_readwrite("graph", &AGRC::WWMIFixerConfig::ToggleList::graph,
                       "Tuple[:class:`str`, :class:`str`]: the parser's graph holding the mod's override")
        .def_readwrite("section", &AGRC::WWMIFixerConfig::ToggleList::section, ":class:`str`: the mod's override")
        .def_readwrite("role", &AGRC::WWMIFixerConfig::ToggleList::role, ":class:`str`: the role it binds")
        .def_readwrite("reg", &AGRC::WWMIFixerConfig::ToggleList::reg, ":class:`str`: the register every ``this`` is re-keyed to")
        .def_readwrite("raw", &AGRC::WWMIFixerConfig::ToggleList::raw,
                       ":class:`bool`: whether it is the copy for a list keeping the mod's art unedited")
        .def_readwrite("thisKeys", &AGRC::WWMIFixerConfig::ToggleList::thisKeys,
                       "List[:class:`str`]: the spellings of ``this`` the override writes")
        .def_readwrite("deadValues", &AGRC::WWMIFixerConfig::ToggleList::deadValues,
                       "Set[:class:`str`]: the values of a ``this`` naming nothing, which the copy drops")
        .def_readwrite("offBinding", &AGRC::WWMIFixerConfig::ToggleList::offBinding,
                       "Optional[:class:`str`]: a one-variant toggle's binding while off: the source's game texture of the role");
    py::class_<AGRC::WWMIFixerConfig::TextureLists>(fixerConfig, "TextureLists", R"doc(
Every texture list the fix writes, and the toggles they run -- see :meth:`WWMIFixerConfig.textureLists`
    )doc")
        .def(py::init<>())
        .def_readwrite("lists", &AGRC::WWMIFixerConfig::TextureLists::lists,
                       "List[:class:`WWMIFixerConfig.TextureList`]: the lists, in the order the fix writes them")
        .def_readwrite("toggles", &AGRC::WWMIFixerConfig::TextureLists::toggles,
                       "List[:class:`WWMIFixerConfig.ToggleList`]: the copies of the mod's toggles the lines run, in the order first run")
        .def("find", &AGRC::WWMIFixerConfig::TextureLists::find, py::arg("kind"), py::arg("component"), py::arg("passNum") = 0,
             py::return_value_policy::reference_internal, R"doc(
The list of ``kind`` for ``component`` (and ``passNum``)

:param kind: The kind
:param component: The component (a shared mesh's place)
:param passNum: The pass's place
:returns: Optional[:class:`WWMIFixerConfig.TextureList`]
             )doc");
    fixerConfig.def("textureLists", &AGRC::WWMIFixerConfig::textureLists, py::arg("data"), py::arg("roles"), py::arg("defaults"),
                    py::arg("drawn"), py::arg("rerouteFrom"), py::arg("ownShader"), py::arg("overlaid"), R"doc(
THE FIX'S TEXTURE LISTS, LINE BY LINE. For each drawn component of the plan, in order: its ``"main"`` list (each of its
bindings at its register: ``null`` as nothing, a role the mod toggles against the game's texture by what binds it
outside the toggle, else the role's resource), its ``"game"`` list when its section draws before it binds anything (the
source's game textures), and a ``"pass"`` list for each of its slot's :attr:`extraPassRegs` (``ib = null`` where
:attr:`extraPassNoDraw` names the pass). Then a ``"sharedMesh"`` list for each pass of each :attr:`sharedMeshes` mesh,
and an ``"overlay"`` list for each drawn overlay. Every line binds a resource directly;
:meth:`toggleLists` then puts the mod's own toggles in

:param data: :class:`IniParseData`: the parser's data of the mod's ``.ini``
:param roles: :class:`WWMIFixerConfig.RoleResources`: what the roles resolve to, a reroute's entries included
:param defaults: :class:`WWMIFixerConfig.GameDefaults`: where the source's game texture is the default
:param drawn: Set[:class:`int`]: the source components the mod draws, reroutes included
:param rerouteFrom: Dict[:class:`int`, :class:`int`]: a reroute that took draws -> the component it took them from
:param ownShader: Set[:class:`int`]: the drawn components their own shader draws
:param overlaid: Set[:class:`int`]: the drawn components whose overlay is drawn
:returns: :class:`WWMIFixerConfig.TextureLists`
    )doc");
    fixerConfig.def("toggleLists", &AGRC::WWMIFixerConfig::toggleLists, py::arg("lists"), py::arg("data"), py::arg("defaults"),
                    py::arg("sourceName"), py::arg("toModName"), py::arg("iniPath"), R"doc(
THE MOD'S OWN TOGGLES IN THE LISTS. A line binding a resource the mod chooses among others under a toggle
(:meth:`IniParseData.thisToggleOf`) runs a copy of the mod's texture override instead
(``CommandList<Source><Role><Reg>[Raw]``, one per role, register and rawness, made on its first line), so the toggle
comes across rather than collapsing to its first variant: its ``this`` lines re-keyed to the line's register, the ones
naming nothing dropped, and a one-variant toggle bound to the source's game texture while off. A line stays direct when
the override has no lines, sits in no graph, or binds nothing that resolves

:param lists: :class:`WWMIFixerConfig.TextureLists`: the lists (:meth:`textureLists`)
:param data: :class:`IniParseData`: the parser's data of the mod's ``.ini``
:param defaults: :class:`WWMIFixerConfig.GameDefaults`: where the source's game texture is the default
:param sourceName: The source character's name
:param toModName: The target's name
:param iniPath: The ``.ini``'s path
:returns: :class:`WWMIFixerConfig.TextureLists`: the lists, with the lines that run a toggle, and the toggles
    )doc");

    fixerConfig.def_static("withReroutes", &AGRC::WWMIFixerConfig::withReroutes, py::arg("roles"), py::arg("rerouteFrom"), R"doc(
The roles as a fix resolves them once the reroutes are known : each reroute
(:attr:`drawReroutes`) reads its source component's files, so every ``(role, from)`` entry of
:attr:`RoleResources.ofSlotRole` is also filed under ``(role, as)``, unless that entry is there already

:param roles: :class:`WWMIFixerConfig.RoleResources`: the :meth:`roleResources` decision
:param rerouteFrom: Dict[:class:`int`, :class:`int`]: each reroute that took draws: its ``as`` -> its ``from``
:returns: :class:`WWMIFixerConfig.RoleResources`
    )doc");

    py::class_<AGRC::WWMIFixerConfig::FileGroups>(fixerConfig, "FileGroups", R"doc(
Which file each drawn component's remapped section goes in -- see :meth:`WWMIFixerConfig.fileGroups`
    )doc")
        .def(py::init<>())
        .def_readwrite("groups", &AGRC::WWMIFixerConfig::FileGroups::groups,
                       "List[List[:class:`int`]]: per file, in order -- the mod's own ``.ini`` first, then each copy -- the source "
                       "components it remaps")
        .def_readwrite("dropped", &AGRC::WWMIFixerConfig::FileGroups::dropped,
                       "List[:class:`int`]: the drawn components the plan has no target slot for, which are not remapped")
        .def_readwrite("notes", &AGRC::WWMIFixerConfig::FileGroups::notes,
                       "List[:class:`str`]: what the decision has to say, for the log, in order");
    fixerConfig.def("fileGroups", &AGRC::WWMIFixerConfig::fileGroups, py::arg("drawn"), py::arg("sourceName"), R"doc(
ONE REMAPPED SECTION PER TARGET DRAW PER FILE. In source component order, the first drawn component to claim a target slot
goes in the mod's own ``.ini`` (group 0), the second in the first copy, and so on -- except one whose
:attr:`SourceComponent.sameFileSection` is set, which stays in the mod's own file. A drawn component the plan names no slot
for is dropped. There is always at least one group

:param drawn: Set[:class:`int`]: the source components the mod draws, reroutes included
:param sourceName: The source character's name, for the notes
:returns: :class:`WWMIFixerConfig.FileGroups`
    )doc");

    py::class_<AGRC::WWMIFixerConfig::OwnMesh>(fixerConfig, "OwnMesh", R"doc(
A source component the mod draws from mesh files of its OWN: its section runs a ``<sharedResourcesList>.<suffix>`` list
(:attr:`WWMINames.OverrideSharedResourcesList`) that binds an index, position and blend buffer of its own -- see
:meth:`WWMIFixerConfig.drawnOf`
    )doc")
        .def(py::init<>())
        .def_readwrite("list", &AGRC::WWMIFixerConfig::OwnMesh::list,
                       ":class:`str`: the mod's own shared-resource list, as the section runs it")
        .def_readwrite("blendResource", &AGRC::WWMIFixerConfig::OwnMesh::blendResource,
                       ":class:`str`: the resource the list binds at the blend register (:attr:`IniKeywords.WWMIBlendReg`)")
        .def_readwrite("blendFile", &AGRC::WWMIFixerConfig::OwnMesh::blendFile,
                       ":class:`str`: that resource's file, as the ``.ini`` names it")
        .def_readwrite("positionFile", &AGRC::WWMIFixerConfig::OwnMesh::positionFile,
                       ":class:`str`: the file of the resource the list binds at ``vb0``, as the ``.ini`` names it")
        .def_readwrite("vgOffset", &AGRC::WWMIFixerConfig::OwnMesh::vgOffset,
                       ":class:`int`: the section's ``vg_offset``: where the component's bone ids start (``0`` when it has none "
                       "or it is not a number)");

    py::class_<AGRC::WWMIFixerConfig::Drawn>(fixerConfig, "Drawn", R"doc(
What the mod draws, read off the parser's data -- see :meth:`WWMIFixerConfig.drawnOf`
    )doc")
        .def(py::init<>())
        .def_readwrite("present", &AGRC::WWMIFixerConfig::Drawn::present,
                       "Dict[:class:`int`, List[:class:`str`]]: each source component whose sections draw -> its sections, a "
                       "reroute's ``as`` included (sharing its ``from``'s sections)")
        .def_readwrite("silent", &AGRC::WWMIFixerConfig::Drawn::silent,
                       "List[:class:`int`]: the source components whose sections draw nothing (:meth:`IniParseData.isUndrawn`), "
                       "in the parser's order")
        .def_readwrite("drawRanges", &AGRC::WWMIFixerConfig::Drawn::drawRanges,
                       "Dict[:class:`int`, List[Tuple[:class:`int`, :class:`int`]]]: each drawn source component -> its "
                       "``(index count, first index)`` draws, less the ones a reroute took; a reroute's ``as`` -> the draws it took")
        .def_readwrite("rerouteFrom", &AGRC::WWMIFixerConfig::Drawn::rerouteFrom,
                       "Dict[:class:`int`, :class:`int`]: each reroute (:attr:`WWMIFixerConfig.drawReroutes`) that took draws: "
                       "its ``as`` -> its ``from``")
        .def_readwrite("reroutedDraws", &AGRC::WWMIFixerConfig::Drawn::reroutedDraws,
                       "Dict[:class:`int`, Set[Tuple[:class:`int`, :class:`int`]]]: each reroute that took draws: its ``as`` -> "
                       "the draws it took")
        .def_readwrite("drawn", &AGRC::WWMIFixerConfig::Drawn::drawn,
                       "Set[:class:`int`]: the present components that are not a reroute's ``as``")
        .def_readwrite("rabbitFX", &AGRC::WWMIFixerConfig::Drawn::rabbitFX,
                       "Set[:class:`int`]: of :attr:`drawn`, the ones whose sections name RabbitFX (:meth:`IniParseData.libraries`)")
        .def_readwrite("ownShader", &AGRC::WWMIFixerConfig::Drawn::ownShader,
                       "Set[:class:`int`]: the present components their own :attr:`SourceComponent.shader` draws "
                       "(:meth:`WWMIFixerConfig.shaderDraws`)")
        .def_readwrite("overlaid", &AGRC::WWMIFixerConfig::Drawn::overlaid,
                       "Set[:class:`int`]: the present components their :attr:`SourceComponent.overlay` is drawn over -- not one "
                       "that hands its maps to RabbitFX while the overlay yields to it")
        .def_readwrite("markLines", &AGRC::WWMIFixerConfig::Drawn::markLines,
                       "Set[:class:`int`]: the present components whose :attr:`SourceComponent.markLines` are written -- not one "
                       "whose shader yields to RabbitFX")
        .def_readwrite("legacy", &AGRC::WWMIFixerConfig::Drawn::legacy,
                       ":class:`bool`: whether the mod is from before WWMI's merged skeleton: none of its slot sections carries a "
                       "``vg_offset``, so its blend holds each component's OWN bone indices")
        .def_readwrite("hostResetsDetected", &AGRC::WWMIFixerConfig::Drawn::hostResetsDetected,
                       ":class:`bool`: whether the mod's ``[Present]`` resets ``$object_detected`` each frame "
                       "(``post $object_detected = 0``, anywhere in it; a ``[Present]`` written more than once is one section)")
        .def_readwrite("ownMeshes", &AGRC::WWMIFixerConfig::Drawn::ownMeshes,
                       "Dict[:class:`int`, :class:`WWMIFixerConfig.OwnMesh`]: each drawn component (not a reroute's ``as``) "
                       "drawn from mesh files of its own -> those files")
        .def_readwrite("error", &AGRC::WWMIFixerConfig::Drawn::error,
                       ":class:`str`: why the mod is not remapped, or empty. When set, only :attr:`hostResetsDetected`, "
                       ":attr:`watching` and what was read before the reason are filled")
        .def_readwrite("watching", &AGRC::WWMIFixerConfig::Drawn::watching,
                       "List[:class:`str`]: the sections the parser placed in its watching graph (:attr:`WWMIParseObjs.Watch`), "
                       "in its order")
        .def_readwrite("notes", &AGRC::WWMIFixerConfig::Drawn::notes,
                       "List[:class:`str`]: what the reading has to say, for the log, in order");
    fixerConfig.def("drawnOf", &AGRC::WWMIFixerConfig::drawnOf, py::arg("data"), py::arg("targets"), py::arg("sourceSlots"),
                    py::arg("sourceName"), py::arg("sourceVb0"), py::arg("targetName"), R"doc(
WHAT THE MOD DRAWS, off the parser's data and graphs:

#. each slot graph (:meth:`WWMIParserConfig.slotWithPrefix` with :attr:`WWMINames.SlotPrefix`) of a source slot below
   ``sourceSlots`` with sections is a source component: :attr:`Drawn.silent` when its sections draw nothing, else
   :attr:`Drawn.present`, with its draws. With neither, the mod is not remapped (:attr:`Drawn.error`)
#. whether the mod is :attr:`Drawn.legacy`; a legacy mod is not remapped when :attr:`sourceVgMaps` is empty
#. the components drawn from mesh files of their own (:attr:`Drawn.ownMeshes`); one whose blend or position resource names
   no file is left out, with a note
#. each of :attr:`drawReroutes` whose ``from`` is present with draws, whose ``as`` is planned and not present, and whose
   split picked draws: those draws move from ``from`` to ``as``, which is present from then on with ``from``'s sections
#. which components keep RabbitFX, and which are drawn by their own shader, under an overlay or with mark lines
   (:attr:`SourceComponent.shader`, :attr:`SourceComponent.overlay`, :attr:`SourceComponent.markLines`)

:param data: :class:`IniParseData`: the parser's data of the mod's ``.ini``
:param targets: List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: the parser's graphs and their sections,
    in its order (``list(parser.sectionTargets.items())``)
:param sourceSlots: How many draw slots the source has at the fix's source version; a slot graph at or past it is not a
    component. ``-1`` for no limit
:param sourceName: The source character's name, for the notes
:param sourceVb0: The source's ``vb0`` hash at the fix's source version, for the notes
:param targetName: The target character's name, for the notes
:returns: :class:`WWMIFixerConfig.Drawn`
    )doc");

    py::class_<AGRC::WWMIFixerConfig::BlendWidth>(fixerConfig, "BlendWidth", R"doc(
Whether the target's merged skeleton needs WWMI's blend remap -- see :meth:`WWMIFixerConfig.blendWidth`
    )doc")
        .def(py::init<>())
        .def_readwrite("targetPast256", &AGRC::WWMIFixerConfig::BlendWidth::targetPast256,
                       ":class:`bool`: whether the remapped blend indices pass what an 8-bit blend index names, so the fix "
                       "writes a blend remap (and drops the mod's own skeleton lines: :attr:`WWMIFixerConfig.remappedSkeletonRemovals`)")
        .def_readwrite("bones", &AGRC::WWMIFixerConfig::BlendWidth::bones,
                       ":class:`int`: the distinct target bones the vertex group row names")
        .def_readwrite("notes", &AGRC::WWMIFixerConfig::BlendWidth::notes,
                       "List[:class:`str`]: what the decision has to say, for the log, in order");
    fixerConfig.def("blendWidth", &AGRC::WWMIFixerConfig::blendWidth, py::arg("row"), R"doc(
DOES THE TARGET'S SKELETON PASS AN 8-BIT BLEND INDEX? The vertex group row's largest target bone answers it, a property of
the PAIR, so the ``.ini`` and the buffers cannot disagree about whether there is a blend remap. A row naming more distinct
target bones than one blend remap holds (:attr:`WWMIBlend.RemapSize`) falls back to an 8-bit blend, and says that every bone
past 255 then lands on bone 0

:param row: Optional[:class:`VGRemap`]: the source's vertex group row onto the target (:meth:`ModType.getVGRemap`)
:returns: :class:`WWMIFixerConfig.BlendWidth`
    )doc");
    fixerConfig.def("removedLines", &AGRC::WWMIFixerConfig::removedLines, py::arg("targetPast256"), py::arg("components"),
                    py::arg("keepsRabbitFX"), R"doc(
THE LINES EACH REMAPPED SECTION DROPS. Per remapped source component: :attr:`removedRegs`, plus
:attr:`remappedSkeletonRemovals` when ``targetPast256`` -- without RabbitFX's own lines for a component that keeps RabbitFX.
A component with nothing to drop is left out. Each is matched with or without ``ref``

:param targetPast256: the :meth:`blendWidth` decision
:param components: Set[:class:`int`]: the remapped source components (every :meth:`fileGroups` group's)
:param keepsRabbitFX: Set[:class:`int`]: the components that keep the mod's RabbitFX (the :meth:`rabbitFXFlags` decision's keys)
:returns: Dict[:class:`int`, List[:class:`WWMIFixerConfig.RegRemoval`]]: per component, the lines it drops
    )doc");

    py::class_<AGRC::WWMIFixerConfig::ModTexLine>(fixerConfig, "ModTexLine", R"doc(
One of the mod's own texture-register lines and what the fix makes of it -- see :meth:`WWMIFixerConfig.modTexLines`
    )doc")
        .def(py::init<>())
        .def_readwrite("key", &AGRC::WWMIFixerConfig::ModTexLine::key, ":class:`str`: the register as the mod writes it, lowered")
        .def_readwrite("value", &AGRC::WWMIFixerConfig::ModTexLine::value, ":class:`str`: the value as the mod writes it, stripped")
        .def_readwrite("dropped", &AGRC::WWMIFixerConfig::ModTexLine::dropped, ":class:`bool`: whether the line is dropped")
        .def_readwrite("reg", &AGRC::WWMIFixerConfig::ModTexLine::reg,
                       ":class:`str`: the register the line moves to, unless :attr:`dropped`")
        .def_readwrite("newValue", &AGRC::WWMIFixerConfig::ModTexLine::newValue,
                       ":class:`str`: what it binds there, unless :attr:`dropped`");
    fixerConfig.def("modTexLines", &AGRC::WWMIFixerConfig::modTexLines, py::arg("data"), py::arg("roles"), py::arg("lists"),
                    py::arg("drawn"), py::arg("rerouteFrom"), py::arg("ownShader"), R"doc(
THE MOD'S OWN TEXTURE LINES, RE-KEYED BY ROLE. Each ``ps-tN`` line of the mod's sections, for each drawn
component of the plan, goes to the register that component's bindings read its texture's role at
(:meth:`registerOf`). The role is the parser's (:meth:`IniParseData.roleOfResource`).

- With :attr:`carryByRegisterRole`, a texture put into another kind of slot on purpose goes by the slot, or is left alone.
- A line whose role or register is unknown is left alone, unless the component's ``"main"`` list binds that register:
  then it is dropped -- except a file of no known role where the source reads the same kind as the target.
- A file the parser refused for its role binds what the role resolved to instead, or is dropped where nothing stands
  in -- except on a component its own shader draws.

A line left alone is not listed

:param data: :class:`IniParseData`: the parser's data of the mod's ``.ini``
:param roles: :class:`WWMIFixerConfig.RoleResources`: what the roles resolve to, a reroute's entries included
:param lists: :class:`WWMIFixerConfig.TextureLists`: the texture lists (:meth:`textureLists`, :meth:`toggleLists`)
:param drawn: Set[:class:`int`]: the source components the mod draws, reroutes included
:param rerouteFrom: Dict[:class:`int`, :class:`int`]: a reroute that took draws -> the component it took them from
:param ownShader: Set[:class:`int`]: the drawn components their own shader draws
:returns: Dict[:class:`int`, List[:class:`WWMIFixerConfig.ModTexLine`]]: source component -> its lines the fix moves or
    drops, in the order the mod first writes them
    )doc");

    parserConfig.def("withGameData", &AGRC::WWMIParserConfig::withGameData, py::arg("modType"), py::arg("version") = py::none(), R"doc(
This config with the character's GAME DATA read off the versioned asset tables: her game textures' thumbprints
(:attr:`WWMITextureFacts.textureThumbprints`, :class:`TextureThumbprints`, at ``version``), each
:class:`WWMIParserConfig.DrawSplit`'s ``flags`` named by its ``flag`` (:class:`MaterialCodes`, at ``version``) and, for a character whose skeleton was
renumbered (her ``vb0`` rows in the library's hashes at two versions or more, each with a :class:`VGRemaps` row onto the
newest), :attr:`skeletonNumberings` -- one per version, oldest first, with the bones that row moves -- and
:attr:`referenceBoneCentroids` (:class:`BoneCentroids` at the newest). A field the tables hold nothing for keeps the
config's value. The compiled parser calls this once per parse

:param modType: :class:`ModType`: the character's mod type
:param version: Optional[:class:`Version`]: the run's version of her (the mod's); ``None`` reads at :attr:`version`
:returns: :class:`WWMIParserConfig`
    )doc");

    // WWMI's parser's readings of its rows, each a decision a GraphVarApply of the parser runs
    parserConfig.def_static("slotSections", &AGRC::WWMIParserConfig::slotSections, py::arg("targets"), py::arg("sectionNames"), R"doc(
Each draw slot the mod has sections for -> those sections: the roots of the slot graphs (the mod objects
:meth:`slotWithPrefix` reads with :attr:`WWMINames.SlotPrefix`), in the order the ``.ini`` writes them, each once. A slot
whose sections draw nothing (:meth:`IniParseData.isUndrawn`) is listed too, and a slot graph with no sections maps to an
empty list. A section ``sectionNames`` does not name sorts after every one it does, in the parser's order

:param targets: List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: the parser's graphs and their sections,
    in its order (``list(parser.sectionTargets.items())``)
:param sectionNames: List[:class:`str`]: the ``.ini``'s sections, in the order it writes them; empty keeps the parser's order
:returns: Dict[:class:`int`, List[:class:`str`]]: draw slot -> its sections
    )doc");
    parserConfig.def_static("mergedSkeleton", &AGRC::WWMIParserConfig::mergedSkeleton, py::arg("objRows"), py::arg("targets"),
                            py::arg("slots") = -1, R"doc(
Whether the mod is written for WWMI's merged skeleton: a ``vg_offset`` row of ``objRows`` sits in a section of a slot
graph (:meth:`slotSections`), the section's name matched ignoring case. A mod from before the merged skeleton carries no
``vg_offset`` there

:param objRows: List[:class:`GraphCollect.Row`]: the rows filed under :attr:`IniParseRows.Objs`
:param targets: List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: the parser's graphs and their sections,
    in its order (``list(parser.sectionTargets.items())``)
:param slots: :class:`int`: how many draw slots count; a slot graph at or past it is not read. ``-1`` for no limit
:returns: :class:`bool`
    )doc");
    parserConfig.def("resourceDataRules", &AGRC::WWMIParserConfig::resourceDataRules, py::arg("targets"), py::arg("sectionNames"),
                     py::arg("onlyThisFile"), py::arg("log") = py::none(), R"doc(
The rules :meth:`IniParseData.resourceDataOf` reads the mod's resource data by, as WWMI's parser hands them over: the rows
the parser files under :attr:`IniParseRows.Objs`, :attr:`IniParseRows.Resources` and :attr:`IniParseRows.Sections`; the
slot graphs as the roots; ``ps-t`` registers (:attr:`IniKeywords.TexRegPrefix`); no slot undrawn when every slot would be;
RabbitFX as a library; a ``run =`` drawing when it calls a section ``sectionNames`` does not name outside WWMI's own namespace

Unless ``onlyThisFile``, or this config has no textures, also the TOGGLES: the rows filed under
:attr:`WWMINames.TextureTogglesRows`, read in the ``TextureOverride`` sections that are not a fix's, the texture graphs as
their roots, and a lone ``this`` under ``$object_detected`` not a toggle

:param targets: List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: the parser's graphs and their sections,
    in its order (``list(parser.sectionTargets.items())``)
:param sectionNames: List[:class:`str`]: the ``.ini``'s sections
:param onlyThisFile: :class:`bool`: whether the rules are for a linked ``.ini`` a fix reads only the resource data of
:param log: Optional[Callable[[:class:`str`], None]]: where the reading logs what it has to say; ``None`` for nowhere
:returns: :class:`IniParseData.ResourceDataRules`
    )doc");
    parserConfig.def("replacesTexturesByHash", &AGRC::WWMIParserConfig::replacesTexturesByHash, py::arg("overrideRows"),
                     py::arg("targets"), R"doc(
Whether the mod replaces textures by HASH: a ``TextureOverride`` section that is no slot graph's root has a ``hash`` and a
``this`` among ``overrideRows``. Its art then reaches every draw binding that texture, whatever the draw's own section
binds -- :attr:`IniParseData.ResourceData.replacesTexturesByHash`

:param overrideRows: List[:class:`GraphCollect.Row`]: the rows filed under :attr:`WWMINames.TextureOverridesRows`
:param targets: List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: the parser's graphs and their sections,
    in its order
:returns: :class:`bool`
    )doc");
    parserConfig.def("roleFileRules", &AGRC::WWMIParserConfig::roleFileRules, py::arg("targets"), py::arg("sectionNames"),
                     py::arg("modType"), py::arg("iniPath"), py::arg("iniFolder"), R"doc(
The rules :meth:`IniParseData.roleFilesOf` reads which file each drawn slot takes for each texture role by, as WWMI's parser
hands them over, or ``None`` when this config has no textures:

#. the rows the parser files (:attr:`IniParseRows.Sections`, :attr:`IniParseRows.Declared`, :attr:`IniParseRows.Objs`,
   :attr:`WWMINames.TextureOverridesRows`, :attr:`WWMINames.TextureRolesRows`, :attr:`WWMINames.CarriedRows`)
#. the slots with sections (:meth:`slotSections`) as the objects, each slot's :attr:`WWMITextureFacts.registerRoles`, and
   :attr:`WWMITextureFacts.regionRoles`
#. a lone ``this`` under ``$object_detected`` not a toggle
#. a hash's role: the character's :class:`Hashes` row typed by a role this config knows, else
   :attr:`WWMITextureFacts.roles`; a file's game texture by its thumbprint (:meth:`TexThumbprint.identifyFile`); a hash's
   age (:meth:`Hashes.age` over the library's hashes)
#. a section a previous fix wrote (:meth:`IniNamingTools.looksRemapped` over the character's name and the mods she fixes)
   is not the mod's use of a resource

``linked`` and ``log`` are left for the caller. Every function in the rules holds its own copies of what it reads

:param targets: List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: the parser's graphs and their sections,
    in its order (``list(parser.sectionTargets.items())``)
:param sectionNames: List[:class:`str`]: the ``.ini``'s sections, in the order it writes them
:param modType: Optional[:class:`ModType`]: the character's mod type; ``None`` reads no hash off her hashes
:param iniPath: :class:`str`: the ``.ini``'s path
:param iniFolder: :class:`str`: the ``.ini``'s folder
:returns: Optional[:class:`IniParseData.RoleFileRules`]
    )doc");
    parserConfig.def("skeletonNumbering", &AGRC::WWMIParserConfig::skeletonNumbering, py::arg("objRows"), py::arg("resourceRows"),
                     py::arg("targets"), py::arg("iniFolder"), py::arg("log") = py::none(), R"doc(
Which numbering of :attr:`skeletonNumberings` the mod's blend's bone ids are in, asked of its own geometry
(:class:`SkeletonNumberingProbe`) -- the numbering's version, or ``None``. Not asked with fewer than two numberings or no
:attr:`referenceBoneCentroids`, of a mod from before WWMI's merged skeleton (not :meth:`mergedSkeleton`), or of one
whose resources name no position or blend buffer

:param objRows: List[:class:`GraphCollect.Row`]: the rows filed under :attr:`IniParseRows.Objs`
:param resourceRows: List[:class:`GraphCollect.Row`]: the rows filed under :attr:`IniParseRows.Resources`
:param targets: List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: the parser's graphs and their sections,
    in its order
:param iniFolder: :class:`str`: the ``.ini``'s folder, which the resources' files are relative to
:param log: Optional[Callable[[:class:`str`], None]]: where the probe's answer is logged; ``None`` for nowhere
:returns: Optional[:class:`str`]
    )doc");
    parserConfig.def("splitDraws", &AGRC::WWMIParserConfig::splitDraws, py::arg("data"), py::arg("roles"), py::arg("targets"),
                     py::arg("iniPath"), py::arg("iniFolder"), py::arg("log") = py::none(), R"doc(
The resource data with each of :attr:`drawSplits`' draws added, or ``None`` when there are no splits, no data, or the
resources name no index, texcoord or position buffer. A split reads the draws of its drawn slot (not undrawn, with draws and
sections) off the material-code map the slot takes for :attr:`WWMIParserConfig.DrawSplit.codeRole` -- the mod's own when a
resource of the ``.ini`` whose name is not a fix's names that file, else :attr:`WWMIParserConfig.DrawSplit.sourceFlags` --
at the draws' vertices (:class:`DrawUVCoverage`), and files the draws it picks as the draw ranges of ``(split name, slot)``

:param data: Optional[:class:`IniParseData.ResourceData`]: the resource data (:attr:`IniParseData.ResourceDataVar`)
:param roles: Optional[:class:`IniParseData.RoleFileData`]: the role files (:attr:`IniParseData.RoleFilesVar`)
:param targets: List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: the parser's graphs and their sections,
    in its order
:param iniPath: :class:`str`: the ``.ini``'s path
:param iniFolder: :class:`str`: the ``.ini``'s folder, which the resources' files are relative to
:param log: Optional[Callable[[:class:`str`], None]]: where a map that cannot be read is logged; ``None`` for nowhere
:returns: Optional[:class:`IniParseData.ResourceData`]
    )doc");
    parserConfig.def_static("ofFactory", [](const PyIniParseFactory& factory) {
        return AGRC::WWMIParserConfig::ofFactory(factory.factory);
    }, py::arg("factory"), R"doc(
The config a parser factory from :func:`makeWWMIParser` was made with, or ``None`` for any other factory. A shipped
parser's factory is its mod type's builder row (:meth:`IniParseBuilder.factoryOf`), so this reads a shipped parser -- or
starts a variant of it (change a field, hand it to :func:`makeWWMIParser`)

:param factory: :class:`CppIniParseFactory`: the factory
:returns: Optional[:class:`WWMIParserConfig`]
    )doc");

}
