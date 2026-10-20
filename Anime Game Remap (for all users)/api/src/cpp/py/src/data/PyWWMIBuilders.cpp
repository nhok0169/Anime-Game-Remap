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
#include "../model/strategies/texEditors/PyTexEditor.h"   // PyTexFilter
#include "../tools/PyRefFunction.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppWWMIBuilders(pybind11::module_ &m) {
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
        .def_readwrite("identifyTexture", &AGRC::WWMITextureFacts::identifyTexture, py::doc(R"doc(
Optional[Callable[[:class:`str`], Optional[:class:`str`]]]: A hook that names the texture hash a mod
file IS -- pixel identity with one of the game's own textures -- for a declared file no hash and no
register names. ``None`` skips the step
        )doc"))
        .def_readwrite("textureThumbprints", &AGRC::WWMITextureFacts::textureThumbprints, py::doc(R"doc(
Dict[:class:`str`, List[:class:`int`]]: The game's own textures by hash, each a
:attr:`thumbprintSize` square grayscale box average of the decoded file
(``Tools/Misc/Diagnostics/wwmiTextureThumbs.py`` generates them). A declared file nothing else names
is thumbprinted the same way and correlated against every entry; it IS the texture it correlates at
least :attr:`identityMin` with while every other stays under :attr:`identityGap`. Consulted after
:attr:`identifyTexture`; empty skips the step

.. note::
    The thumbprints are read from here only, not from the fixer's config; leaving this empty makes
    the pass silently identify nothing
        )doc"))
        .def_readwrite("thumbprintSize", &AGRC::WWMITextureFacts::thumbprintSize,
                        py::doc(":class:`int`: The side of a thumbprint. **Default**: ``16``"))
        .def_readwrite("identityMin", &AGRC::WWMITextureFacts::identityMin,
                        py::doc(":class:`float`: The correlation a file needs with ONE thumbprint to be that texture. **Default**: ``0.97``"))
        .def_readwrite("identityGap", &AGRC::WWMITextureFacts::identityGap,
                        py::doc(":class:`float`: The correlation every OTHER thumbprint must stay under. **Default**: ``0.90``"))
        .def_readwrite("downloadGameFolder", &AGRC::WWMITextureFacts::downloadGameFolder,
                        py::doc(":class:`str`: The game folder under ``Data/Mod Downloads`` this character's textures are fetched from. **Default**: ``\"WuWa\"``"))
        .def_readwrite("downloadCharFolder", &AGRC::WWMITextureFacts::downloadCharFolder,
                        py::doc(":class:`str`: This character's download folder under :attr:`downloadGameFolder`. Empty registers no fallback download. **Default**: ``\"\"``"))
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
        )doc"));

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
        .def_readwrite("version", &AGRC::WWMIParserConfig::version, py::doc(R"doc(
:class:`str`: The game version the library files the character's rows under, used when the ``.ini``
carries no version of its own. A reverse lookup with no version resolves through the newest bucket
holding the value, and ``0`` (component 0's index) is every GI head's index too

**Default**: ``"2.5"``
        )doc"))
        .def_readwrite("slotHashType", &AGRC::WWMIParserConfig::slotHashType,
                        py::doc(":class:`str`: The hash type every draw slot section matches. **Default**: ``\"vb0\"``"))
        .def_readwrite("textures", &AGRC::WWMIParserConfig::textures, py::doc(R"doc(
:class:`WWMITextureFacts`: What this character's textures are -- what the parser identifies a mod's
textures WITH. Leaving it empty means every role falls back to downloading the game's own texture
        )doc"))
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
        .def(py::init([](std::string reg, std::string role, int srcComponent) {
            return AGRC::WWMIFixerConfig::Binding{std::move(reg), std::move(role), srcComponent};
        }), py::arg("reg"), py::arg("role"), py::arg("srcComponent") = -1)
        .def_readwrite("reg", &AGRC::WWMIFixerConfig::Binding::reg,
                        py::doc(":class:`str`: The register, eg. ``\"ps-t0\"``"))
        .def_readwrite("role", &AGRC::WWMIFixerConfig::Binding::role, py::doc(R"doc(
:class:`str`: The role (a value of :attr:`WWMIFixerConfig.roles` or :attr:`WWMIFixerConfig.typeRoles`,
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
        .def_readwrite("ps", &AGRC::WWMIFixerConfig::Shader::ps,
                        py::doc(":class:`str`: The HLSL source, entry point ``main``"))
        .def_readwrite("constantBuffers", &AGRC::WWMIFixerConfig::Shader::constantBuffers, py::doc(R"doc(
Dict[:class:`str`, List[:class:`int`]]: Register -> the constant buffer bound there, as its 32-bit
words (four to a row), eg. ``"ps-cb4"``. Written as a ``R32G32B32A32_FLOAT`` buffer
        )doc"))
        .def_readwrite("lines", &AGRC::WWMIFixerConfig::Shader::lines, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: Further lines of the ``CustomShader`` section, as they are
written, eg. ``("ps-t5", "ref ps-t6")``
        )doc"))
        .def_readwrite("passes", &AGRC::WWMIFixerConfig::Shader::passes, py::doc(R"doc(
List[List[Tuple[:class:`str`, :class:`str`]]]: Draw the component once per entry, each with
:attr:`lines` and that entry's own lines after them -- a source pass whose draws split by state, eg. a
see-through layer blended where the stencil marks a body behind it and drawn opaque everywhere else.
**Default**: empty, one draw with :attr:`lines`
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

    py::class_<AGRC::WWMIFixerConfig::DrawReroute::FlagMask>(drawReroute, "FlagMask", R"doc(
Where a material-code map carries the reroute's :attr:`WWMIFixerConfig.DrawReroute.flags`, cut into
``width x height`` blocks: a block's bit is set when more than half of its texels carry any of them.
Bits are row-major, the lowest bit of each byte first
    )doc")
        .def(py::init<>())
        .def_readwrite("width", &AGRC::WWMIFixerConfig::DrawReroute::FlagMask::width,
                        py::doc(":class:`int`: The number of blocks across"))
        .def_readwrite("height", &AGRC::WWMIFixerConfig::DrawReroute::FlagMask::height,
                        py::doc(":class:`int`: The number of blocks down"))
        .def_readwrite("bits", &AGRC::WWMIFixerConfig::DrawReroute::FlagMask::bits,
                        py::doc("List[:class:`int`]: The blocks, one bit each"));

    drawReroute
        .def(py::init<>())
        .def(py::init([](int from, int as, std::string codeRole, int flags) {
            return AGRC::WWMIFixerConfig::DrawReroute{from, as, std::move(codeRole), flags};
        }), py::arg("from_"), py::arg("as_"), py::arg("codeRole"), py::arg("flags"))
        .def_readwrite("from_", &AGRC::WWMIFixerConfig::DrawReroute::from,
                        py::doc(":class:`int`: The source component whose draws may go elsewhere"))
        .def_readwrite("as_", &AGRC::WWMIFixerConfig::DrawReroute::as, py::doc(R"doc(
:class:`int`: The :attr:`WWMIFixerConfig.plan` entry the rerouted draws are drawn by -- an id that is
not a source slot. Its slot and bindings say where they go and what they bind
        )doc"))
        .def_readwrite("codeRole", &AGRC::WWMIFixerConfig::DrawReroute::codeRole,
                        py::doc(":class:`str`: The role of the material-code map that marks the draws"))
        .def_readwrite("flags", &AGRC::WWMIFixerConfig::DrawReroute::flags, py::doc(R"doc(
:class:`int`: The flag bits that mark a texel: a draw goes when more than
:attr:`WWMIFixerConfig.DrawReroute.share` of the texels under its vertices carry any of them
        )doc"))
        .def_readwrite("share", &AGRC::WWMIFixerConfig::DrawReroute::share, py::doc(R"doc(
:class:`float`: The share of a draw's texels that must carry :attr:`WWMIFixerConfig.DrawReroute.flags`
for the draw to go, eg. ``0.01`` to take a draw that holds any real amount of the flagged material
among others. **Default**: ``0.5``, more than half
        )doc"))
        .def_readwrite("sourceFlags",&AGRC::WWMIFixerConfig::DrawReroute::sourceFlags, py::doc(R"doc(
:class:`WWMIFixerConfig.DrawReroute.FlagMask`: The SOURCE's own game code map of
:attr:`WWMIFixerConfig.DrawReroute.codeRole`, for a mod that binds no code map of its own for that
role: such a mod draws with the game's map on its own character, so that map decides which of its
draws carry the flags. **Default**: empty, and such a mod's component is left whole
        )doc"));

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
        .def_readwrite("gameFileOfRole", &AGRC::WWMIFixerConfig::TexEditContext::gameFileOfRole, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: The SOURCE character's own game texture for each role a
:attr:`WWMIFixerConfig.TexEdit.sourceGameRoles` names, downloaded for the edit
        )doc"));

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
        )doc"))
        .def_readwrite("sourceGameRoles", &AGRC::WWMIFixerConfig::TexEdit::sourceGameRoles, py::doc(R"doc(
List[:class:`str`]: Roles whose SOURCE game texture this edit reads, each downloaded and handed to the
filter in :attr:`WWMIFixerConfig.TexEditContext.gameFileOfRole`. Nothing binds the download.
**Default**: empty
        )doc"));

    py::class_<AGRC::WWMIFixerConfig::SkeletonNumbering>(fixerConfig, "SkeletonNumbering", R"doc(
One way the source's merged bone ids have been numbered -- see :attr:`WWMIFixerConfig.skeletonNumberings`
    )doc")
        .def(py::init<>())
        .def(py::init([](std::string version, std::unordered_map<long long, long long> toReference) {
                 return AGRC::WWMIFixerConfig::SkeletonNumbering{std::move(version), std::move(toReference)};
             }), py::arg("version"), py::arg("toReference") = std::unordered_map<long long, long long>{})
        .def_readwrite("version", &AGRC::WWMIFixerConfig::SkeletonNumbering::version,
                        py::doc(":class:`str`: The version whose vertex group remap row reads ids in this numbering"))
        .def_readwrite("toReference", &AGRC::WWMIFixerConfig::SkeletonNumbering::toReference, py::doc(R"doc(
Dict[:class:`int`, :class:`int`]: Id in this numbering -> the same bone's id in the numbering
:attr:`WWMIFixerConfig.referenceBoneCentroids` is keyed by. An id not listed is the same in both
        )doc"));

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
        .def_readwrite("drawReroutes", &AGRC::WWMIFixerConfig::drawReroutes, py::doc(R"doc(
List[:class:`WWMIFixerConfig.DrawReroute`]: Draws that go through a different target slot than the
rest of their component, chosen by the mod's own material-code map -- never by its colours. A
component whose map is not the mod's own is read through
:attr:`WWMIFixerConfig.DrawReroute.sourceFlags`, or left whole without one; a component whose draws
carry no such texel is left whole.
**Default**: none
        )doc"))
        .def_readwrite("typeRoles", &AGRC::WWMIFixerConfig::typeRoles, py::doc(R"doc(
Dict[:class:`int`, Dict[:class:`str`, :class:`str`]]: Source component ->
``{"diffuse" | "mask" | "normal" -> role}``. Read for which components a role belongs to; what
IDENTIFIES a mod's textures is :class:`WWMITextureFacts`, on the parser
        )doc"))
        .def_readwrite("sourceTextures", &AGRC::WWMIFixerConfig::sourceTextures, py::doc(R"doc(
:class:`WWMITextureFacts`: Every fact about the SOURCE character's own textures -- its hashes by
role, its register layout per component, its pixel thumbprints, and where its game textures are
downloaded from.

The same object :attr:`WWMIParserConfig.textures` takes, so a character states these once
        )doc"))
        .def_readwrite("createdTextures", &AGRC::WWMIFixerConfig::createdTextures,
                        py::doc("List[:class:`WWMIFixerConfig.CreatedTexture`]: The textures the fix invents, each bound wherever a binding names its role"))
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
        .def_readwrite("skeletonNumberings", &AGRC::WWMIFixerConfig::skeletonNumberings, py::doc(R"doc(
List[:class:`WWMIFixerConfig.SkeletonNumbering`]: Every numbering the source's merged skeleton has had,
when a game update renumbered it. The fix scores each against the mod's own geometry (its vertices'
distance to their heaviest bone's centroid, :attr:`referenceBoneCentroids`) and reads the blend with
the vertex group remap row of the closest -- a mod's ``vb0`` cannot say, since hash-update tools
rewrite the hashes and leave the bone ids. Fewer than two (the default) switches it off
        )doc"))
        .def_readwrite("referenceBoneCentroids", &AGRC::WWMIFixerConfig::referenceBoneCentroids, py::doc(R"doc(
Dict[:class:`int`, Tuple[:class:`float`, :class:`float`, :class:`float`]]: The rest-pose centroid of
each source bone, keyed by its id in the reference numbering
        )doc"))
        .def_readwrite("filterIndices", &AGRC::WWMIFixerConfig::filterIndices, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: Shader hash -> the ``filter_index`` to tag it with, overriding
:attr:`filterBase` / :attr:`filterStep`. Both directions of a pair must agree on every shader they
both tag, since 3dmigoto keys a ``[ShaderOverride]`` by its hash across every loaded ``.ini``
        )doc"))
        .def_readwrite("flatFallsBackToSource", &AGRC::WWMIFixerConfig::flatFallsBackToSource, py::doc(R"doc(
Set[:class:`str`]: Roles whose texture marks REGIONS, so a constant one from the mod is replaced by
the source's own (its fallback download)
        )doc"))
        .def_readwrite("flatLeftToGame", &AGRC::WWMIFixerConfig::flatLeftToGame, py::doc(R"doc(
Set[:class:`str`]: Like :attr:`flatFallsBackToSource`, except that a flat one is left to the GAME
        )doc"))
        .def_readwrite("mirroredComponents", &AGRC::WWMIFixerConfig::mirroredComponents, py::doc(R"doc(
Set[:class:`int`]: Source components drawn a second time, wound the other way with their normals
flipped, for single-layer cloth whose inside the target's shader lights differently
        )doc"))
        .def_readwrite("texRegPrefix", &AGRC::WWMIFixerConfig::texRegPrefix,
                        py::doc(":class:`str`: The register family a mod binds its textures with. **Default**: ``ps-t``"))
        .def_readwrite("vectorReg", &AGRC::WWMIFixerConfig::vectorReg,
                        py::doc(":class:`str`: The register the vector (normal) buffer is bound at. **Default**: ``vb1``"))
        .def_readwrite("removedRegs", &AGRC::WWMIFixerConfig::removedRegs, py::doc(R"doc(
List[:class:`WWMIFixerConfig.RegRemoval`]: Register lines a remapped section drops, on top of what
the template removes anyway -- eg. the three ``Resource...Override = ref ...`` lines of a source past
256 merged bones, which undo the remap, and RabbitFX's resource lines
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
the mod is drawn through, on passes that bind ``o3``; a :attr:`Shader.lines` value ``stash <slot> <register>``
binds the capture. **Default**: empty
        )doc"))
        .def_readwrite("hiddenMeshes", &AGRC::WWMIFixerConfig::hiddenMeshes, py::doc(R"doc(
List[:class:`str`]: Other meshes the TARGET draws that a mod of her replaces, by their own ``vb0``
hash: each is skipped while the mod is enabled (no index window, so wherever it is drawn)
        )doc"))
        .def_readwrite("texEdits", &AGRC::WWMIFixerConfig::texEdits,
                        py::doc("List[:class:`WWMIFixerConfig.TexEdit`]: Edits the fix makes to a role's texture before binding it"))
        .def_readwrite("cleanTexcoords", &AGRC::WWMIFixerConfig::cleanTexcoords, py::doc(R"doc(
:class:`bool`: Bind the remapped sections to a CLEANED copy of the mod's texcoord buffer (a NaN
second UV zeroed, a U in the next tile folded back). **Default**: ``False``
        )doc"))
        .def_readwrite("texcoordReg", &AGRC::WWMIFixerConfig::texcoordReg,
                        py::doc(":class:`str`: The register the texcoord buffer is bound at. **Default**: ``vb2``"))
        .def_readwrite("sourceVgMaps", &AGRC::WWMIFixerConfig::sourceVgMaps, py::doc(R"doc(
Dict[:class:`int`, List[:class:`int`]]: The SOURCE's ``vg_map`` per component, for a mod from before
WWMI's merged skeleton (its blend holds per-component LOCAL ids); without it such a mod is refused
        )doc"))
        .def_readwrite("mergedSkeletonSlots", &AGRC::WWMIFixerConfig::mergedSkeletonSlots,
                        py::doc(":class:`int`: The float4 slots of the merged skeleton buffers declared for a legacy mod. **Default**: ``768``"))
        .def_readwrite("boneDataFilter", &AGRC::WWMIFixerConfig::boneDataFilter,
                        py::doc(":class:`str`: WWMI's marker on the game's bone-data constant buffer. **Default**: ``3381.7777``"))
        .def_readwrite("carryByRegisterRole", &AGRC::WWMIFixerConfig::carryByRegisterRole, py::doc(R"doc(
:class:`bool`: Carry a mod's own texture line that puts a texture into a register of a different kind by
the REGISTER's role on the source rather than the texture's -- so a toggle that binds a diffuse into a
ramp or detail slot keeps doing so on the target. **Default**: ``False``
        )doc"))
        .def_readwrite("copiesShareSkeleton", &AGRC::WWMIFixerConfig::copiesShareSkeleton, py::doc(R"doc(
:class:`bool`: On a target past 256 bones, make every generated copy ``.ini`` share the mod's skeleton
state: merge-only sections for the slots other files draw, and the bone-data marker and shape-key
overrides in the mod's own file only. **Default**: ``False``
        )doc"))
        .def_readwrite("currentPoseInCb3Only", &AGRC::WWMIFixerConfig::currentPoseInCb3Only, py::doc(R"doc(
:class:`bool`: Treat a draw whose ``vs-cb3`` carries the bone-data marker while its ``vs-cb4`` does not as
one whose CURRENT pose sits in ``vs-cb3`` -- merged into the main skeleton and bound the remapped main
one -- as WWMI Tools' own template does. For a target with passes like that (Lynae's depth passes),
where reading ``vs-cb3`` as the previous pose drops thin layered parts out. **Default**: ``False``
        )doc"))
        .def_readwrite("bindPrevPoseAlways", &AGRC::WWMIFixerConfig::bindPrevPoseAlways,
                        py::doc(":class:`bool`: Bind the remapped PREVIOUS pose on every pass, not only where the second skeleton carries :attr:`boneDataFilter`. **Default**: ``True``"))
        .def_readwrite("cleanupResourcesList", &AGRC::WWMIFixerConfig::cleanupResourcesList,
                        py::doc(":class:`str`: The command list every slot section runs after its draw. **Default**: ``CommandListCleanupSharedResources``"));

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
