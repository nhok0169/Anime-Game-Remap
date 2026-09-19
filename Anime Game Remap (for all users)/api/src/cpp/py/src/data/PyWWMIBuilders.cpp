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

#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyGIMICharBuilders.h"         // PyIniParseFactory / PyIniFixFactory, the same wrappers
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppWWMIBuilders(pybind11::module_ &m) {
    py::class_<AGRC::WWMIParserConfig>(m, "WWMIParserConfig", R"doc(
What one Wuthering Waves character looks like to its parser, for :func:`makeWWMIParser`

A WWMI mod is ONE mesh drawn in several ranges: every ``[TextureOverrideComponentN]`` matches the
character's ``vb0`` hash and one draw slot's ``match_first_index``, and the buffers, the bone data
and the shape keys are matched by hashes of their own. Nothing is named after the character, so a
section is classified by hash and index alone. Every draw slot the library knows for the character
becomes the mod object ``("", "componentN")``, and each entry of :attr:`hashOnlyObjs` becomes
``("", <obj>)``
    )doc")
        .def(py::init<>())
        .def_readwrite("modTypeId", &AGRC::WWMIParserConfig::modTypeId,
                        py::doc(":class:`ModTypeId`: The mod type a ``.ini`` of this character classifies as"))
        .def_readwrite("version", &AGRC::WWMIParserConfig::version, py::doc(R"doc(
:class:`str`: The game version the library files the character's rows under, used when the ``.ini``
carries no version of its own. A reverse lookup with no version resolves through the newest bucket
holding the value, and ``0`` (component 0's index) is every GI head's index too

**Default**: ``"2.5"``
        )doc"))
        .def_readwrite("slotHashType", &AGRC::WWMIParserConfig::slotHashType,
                        py::doc(":class:`str`: The hash type every draw slot section matches. **Default**: ``\"vb0\"``"))
        .def_readwrite("slotPrefix", &AGRC::WWMIParserConfig::slotPrefix,
                        py::doc(":class:`str`: The ``type`` a draw slot's index rows are filed under, followed by the slot number. **Default**: ``\"component\"``"))
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
        .def(py::init([](std::string reg, std::string role) {
            return AGRC::WWMIFixerConfig::Binding{std::move(reg), std::move(role)};
        }), py::arg("reg"), py::arg("role"))
        .def_readwrite("reg", &AGRC::WWMIFixerConfig::Binding::reg,
                        py::doc(":class:`str`: The register, eg. ``\"ps-t0\"``"))
        .def_readwrite("role", &AGRC::WWMIFixerConfig::Binding::role, py::doc(R"doc(
:class:`str`: The role (a value of :attr:`WWMIFixerConfig.roles` or :attr:`WWMIFixerConfig.typeRoles`,
or the name of a :attr:`WWMIFixerConfig.createdTextures` entry). A role no file of the mod has is
left unbound, so the GAME's texture serves the register
        )doc"));

    py::class_<AGRC::WWMIFixerConfig::SourceComponent>(fixerConfig, "SourceComponent", R"doc(
How one SOURCE component is drawn on the target
    )doc")
        .def(py::init<>())
        .def(py::init([](int slot, std::vector<AGRC::WWMIFixerConfig::Binding> bindings) {
            return AGRC::WWMIFixerConfig::SourceComponent{slot, std::move(bindings)};
        }), py::arg("slot"), py::arg("bindings"))
        .def_readwrite("slot", &AGRC::WWMIFixerConfig::SourceComponent::slot,
                        py::doc(":class:`int`: The target draw slot it goes through"))
        .def_readwrite("bindings", &AGRC::WWMIFixerConfig::SourceComponent::bindings,
                        py::doc("List[:class:`WWMIFixerConfig.Binding`]: The registers its command list binds, in this order"));

    py::class_<AGRC::WWMIFixerConfig::CreatedTexture>(fixerConfig, "CreatedTexture", R"doc(
A texture the fix INVENTS, eg. the flat material mask a target reads skin off
    )doc")
        .def(py::init<>())
        .def(py::init([](std::string role, AGRC::Colour colour, int size) {
            return AGRC::WWMIFixerConfig::CreatedTexture{std::move(role), colour, size};
        }), py::arg("role"), py::arg("colour"), py::arg("size") = 16)
        .def_readwrite("role", &AGRC::WWMIFixerConfig::CreatedTexture::role,
                        py::doc(":class:`str`: The role a binding names it by, eg. ``\"SkinMask\"``"))
        .def_readwrite("colour", &AGRC::WWMIFixerConfig::CreatedTexture::colour,
                        py::doc(":class:`Colour`: The solid colour it is filled with"))
        .def_readwrite("size", &AGRC::WWMIFixerConfig::CreatedTexture::size,
                        py::doc(":class:`int`: Its width and height. **Default**: ``16``"));

    fixerConfig
        .def(py::init<>())
        .def_readwrite("targetId", &AGRC::WWMIFixerConfig::targetId,
                        py::doc(":class:`ModTypeId`: The character fixed TO -- whose ``vb0`` hash, slot windows, vertex-group offsets and shape-key checksum the library's WuWa tables hold"))
        .def_readwrite("version", &AGRC::WWMIFixerConfig::version,
                        py::doc(":class:`str`: The game version the library files both characters under, used when the ``.ini`` carries none. **Default**: ``\"2.5\"``"))
        .def_readwrite("slotPasses", &AGRC::WWMIFixerConfig::slotPasses, py::doc(R"doc(
List[List[:class:`str`]]: Per TARGET slot, the pixel shaders that bind that slot's character
textures -- read off a frame dump of the target. A slot's command list binds on every one of them
and no other pass
        )doc"))
        .def_readwrite("filterBase", &AGRC::WWMIFixerConfig::filterBase,
                        py::doc(":class:`float`: The ``filter_index`` the first distinct shader of :attr:`slotPasses` is tagged with. **Default**: ``3381.91``"))
        .def_readwrite("filterStep", &AGRC::WWMIFixerConfig::filterStep,
                        py::doc(":class:`float`: How much higher each further shader's ``filter_index`` is. **Default**: ``0.01``"))
        .def_readwrite("plan", &AGRC::WWMIFixerConfig::plan, py::doc(R"doc(
Dict[:class:`int`, :class:`WWMIFixerConfig.SourceComponent`]: Source component -> how it is drawn on
the target. A source component the mod has a section for but this does not name is dropped. Several
sources may name one target slot: that is the merge, and which lands in the mod's own ``.ini`` and
which in a copy follows the source components' numeric order
        )doc"))
        .def_readwrite("roles", &AGRC::WWMIFixerConfig::roles, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: Texture hash -> role, for every version of the source's textures a
mod may carry. A file's role is decided in this order: the hash an override section of ANY ``.ini``
of the mod matches for it; the ``t=<hash>`` in its file name; :attr:`identifyTexture`; and last
the ``Component<N>_<Diffuse|LM|NM>`` name convention through :attr:`typeRoles`
        )doc"))
        .def_readwrite("typeRoles", &AGRC::WWMIFixerConfig::typeRoles, py::doc(R"doc(
Dict[:class:`int`, Dict[:class:`str`, :class:`str`]]: Source component ->
``{"diffuse" | "mask" | "normal" -> role}``, for a file named by component and type and by nothing
else
        )doc"))
        .def_readwrite("createdTextures", &AGRC::WWMIFixerConfig::createdTextures,
                        py::doc("List[:class:`WWMIFixerConfig.CreatedTexture`]: The textures the fix invents, each bound wherever a binding names its role"))
        .def_readwrite("downloadGameFolder", &AGRC::WWMIFixerConfig::downloadGameFolder,
                        py::doc(":class:`str`: The game folder under ``Data/Mod Downloads`` the source's textures are fetched from. **Default**: ``\"WuWa\"``"))
        .def_readwrite("downloadCharFolder", &AGRC::WWMIFixerConfig::downloadCharFolder,
                        py::doc(":class:`str`: The source character's download folder under :attr:`downloadGameFolder`. Empty registers no fallback download. **Default**: ``\"\"``"))
        .def_readwrite("downloadVersionFolder", &AGRC::WWMIFixerConfig::downloadVersionFolder,
                        py::doc(":class:`str`: The version folder under :attr:`downloadCharFolder`, eg. ``\"2_5\"``. **Default**: ``\"\"``"))
        .def_readwrite("downloadPrefix", &AGRC::WWMIFixerConfig::downloadPrefix,
                        py::doc(":class:`str`: The file prefix of the source's downloads, eg. ``\"Sanhua\"`` for ``SanhuaTexture<hash>.dds``. **Default**: ``\"\"``"))
        .def_readwrite("fallbackTextures", &AGRC::WWMIFixerConfig::fallbackTextures, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: Role -> the source's texture hash of that role, for a planned role
the mod has NO file for: the register is bound to the SOURCE's own game texture, downloaded as
``<downloadPrefix><Role>RemapDL.dds`` into the mod's texture folder. The mod's UVs are the source's,
so the target's texture, which an unbound register samples on the target's draw, is wrong by
construction. **Default**: empty
        )doc"))
        .def_readwrite("identifyTexture", &AGRC::WWMIFixerConfig::identifyTexture, py::doc(R"doc(
Optional[Callable[[:class:`str`], Optional[:class:`str`]]]: A hook that names the texture hash a mod
file IS -- pixel identity with one of the game's own textures -- for a file no hash names and whose
name says something else. ``None`` skips the step
        )doc"))
        .def_readwrite("textureThumbprints", &AGRC::WWMIFixerConfig::textureThumbprints, py::doc(R"doc(
Dict[:class:`str`, List[:class:`int`]]: The game's own textures by hash, each a :attr:`thumbprintSize`
square grayscale box average of the decoded file (``Tools/Misc/Diagnostics/wwmiTextureThumbs.py``
generates them). A mod file no hash names is thumbprinted the same way and correlated against every
entry; it IS the texture it correlates at least :attr:`identityMin` with while every other stays
under :attr:`identityGap`. Consulted after :attr:`identifyTexture`; empty skips the step
        )doc"))
        .def_readwrite("thumbprintSize", &AGRC::WWMIFixerConfig::thumbprintSize,
                        py::doc(":class:`int`: The side of a thumbprint. **Default**: ``16``"))
        .def_readwrite("identityMin", &AGRC::WWMIFixerConfig::identityMin,
                        py::doc(":class:`float`: The correlation a file needs with ONE thumbprint to be that texture. **Default**: ``0.97``"))
        .def_readwrite("identityGap", &AGRC::WWMIFixerConfig::identityGap,
                        py::doc(":class:`float`: The correlation every OTHER thumbprint must stay under. **Default**: ``0.90``"))
        .def_readwrite("zeroShapeKeyStream", &AGRC::WWMIFixerConfig::zeroShapeKeyStream,
                        py::doc(":class:`bool`: Whether every remapped draw binds a zero shape-key offset stream. **Default**: ``True``"))
        .def_readwrite("shapeKeyStreamReg", &AGRC::WWMIFixerConfig::shapeKeyStreamReg,
                        py::doc(":class:`str`: The register the game reads that stream from. **Default**: ``\"vb6\"``"))
        .def_readwrite("shapeKeyStride", &AGRC::WWMIFixerConfig::shapeKeyStride,
                        py::doc(":class:`int`: Bytes per vertex of that stream. **Default**: ``24``"))
        .def_readwrite("hiddenObjs", &AGRC::WWMIFixerConfig::hiddenObjs, py::doc(R"doc(
List[:class:`str`]: The hash-only mod objects whose sections are commented out of the mod's own text
and copied nowhere. **Default**: the two shape-key overrides
        )doc"))
        .def_readwrite("blendReg", &AGRC::WWMIFixerConfig::blendReg,
                        py::doc(":class:`str`: The register the mod's blend buffer is bound at. **Default**: ``\"vb4\"``"))
        .def_readwrite("sharedResourcesList", &AGRC::WWMIFixerConfig::sharedResourcesList,
                        py::doc(":class:`str`: The command list every slot section runs to bind the mod's buffers; the texture command list and the zero stream are added right after it. **Default**: ``\"CommandListOverrideSharedResources\"``"))
        .def_readwrite("slotPrefix", &AGRC::WWMIFixerConfig::slotPrefix,
                        py::doc(":class:`str`: The mod object prefix of a draw slot. **Default**: ``\"component\"``"))
        .def_readwrite("sourceLabels", &AGRC::WWMIFixerConfig::sourceLabels,
                        py::doc("Dict[:class:`int`, :class:`str`]: Source component -> a label for the log"))
        .def_readwrite("targetLabels", &AGRC::WWMIFixerConfig::targetLabels,
                        py::doc("Dict[:class:`int`, :class:`str`]: Target slot -> a label for the log"))
        .def_readwrite("copyPreamble", &AGRC::WWMIFixerConfig::copyPreamble,
                        py::doc(":class:`str`: What every generated ``.ini`` copy opens with"));

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

:param config: The pair's config
:returns: A fixer factory, for :meth:`CppStrategyOverrides.setFixer` or the builder table
    )doc"));
}
