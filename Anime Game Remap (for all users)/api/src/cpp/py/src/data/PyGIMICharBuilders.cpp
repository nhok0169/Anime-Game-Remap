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

#include "PyGIMICharBuilders.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/data/IniFixData/RegValChecks.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"
#include "../model/strategies/texEditors/PyTexEditor.h"   // toTexFilter

// TexEditor::Filter is std::function<void(TextureFile&)>, and pybind11/functional.h needs the
// COMPLETE type to decide how to convert it -- TexEditor.h only forward-declares it.
#include "AGRemapCore/model/files/TextureFile.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppGIMICharBuilders(pybind11::module_ &m) {
    // ------------------------------------------------------------------- the parser config
    py::class_<AGRC::GIMICharParserConfig>(m, "GIMICharParserConfig", R"doc(
What one character's ``.ini`` file looks like, for the **standard GIMI character shape**

Everything :func:`makeGIMICharParser` needs that is not the same for every character. Deliberately
small: if you find yourself wanting a field for something only one character does, that character
probably wants a parser of its own instead.

Every field is writable, and all but :attr:`drawnObjs` and :attr:`texcoordStride` have a sensible
default, so the usual shape is to construct one and assign what differs.
    )doc")
        .def(py::init<>())

        .def_property("modTypeId",
            [](const AGRC::GIMICharParserConfig &self) { return static_cast<int>(self.modTypeId); },
            [](AGRC::GIMICharParserConfig &self, int modTypeId) {
                self.modTypeId = static_cast<AGRC::ModTypeId>(modTypeId);
            }, py::doc(R"doc(
:class:`int`: Which mod type this parses -- a :attr:`ModType.modTypeId`. Used for the vertex count
lookup the downloaded `blend`_ needs
            )doc"))

        .def_readwrite("downloadCharFolder", &AGRC::GIMICharParserConfig::downloadCharFolder,
            py::doc(":class:`str`: The character's folder under ``Data/Mod Downloads/GI/``, eg. ``\"Amber\"``"))

        .def_readwrite("downloadVersionFolder", &AGRC::GIMICharParserConfig::downloadVersionFolder,
            py::doc(":class:`str`: The version subfolder within it, eg. ``\"4_0\"``"))

        .def_readwrite("downloadPrefix", &AGRC::GIMICharParserConfig::downloadPrefix, py::doc(R"doc(
:class:`str`: The prefix every file in that folder carries, eg. ``"Amber"``

.. note::
    Not always the character's name --- ``Raiden/`` holds ``RaidenShogun``-prefixed files, and one
    character's version subfolders can disagree with each other. Read it off the folder's contents
        )doc"))

        .def_readwrite("drawnObjs", &AGRC::GIMICharParserConfig::drawnObjs, py::doc(R"doc(
List[:class:`str`]: The objects that actually draw, lowercase and **in the order the game draws
them** --- eg. ``["head", "body", "dress"]``

They all share one ``ib`` hash and are told apart by the ``match_first_index`` that follows it
        )doc"))

        .def_readwrite("texcoordStride", &AGRC::GIMICharParserConfig::texcoordStride, py::doc(R"doc(
:class:`int`: The byte size of one texcoord vertex --- **per character**, and worth checking rather
than copying: Amber and Mona are ``12`` where Rosaria is ``20``
        )doc"))

        .def_readwrite("positionStride", &AGRC::GIMICharParserConfig::positionStride,
            py::doc(":class:`int`: The byte size of one position vertex. **Default**: ``40``"))

        .def_readwrite("blendStride", &AGRC::GIMICharParserConfig::blendStride,
            py::doc(":class:`int`: The byte size of one `blend`_ vertex. **Default**: ``32``"));

    // ------------------------------------------------------------------- the fixer config
    py::class_<AGRC::GIMICharFixerConfig> fixerConfig(m, "GIMICharFixerConfig", R"doc(
What one character's fix does differently, for the **standard GIMI character shape**

Everything :func:`makeGIMICharFixer` needs that is not the same for every character. Every field has
a default that means "the ordinary thing", so a config assigns only what its character does
differently.
    )doc");

    // Without these two registered, objRegRemovals / objRegRemaps could be READ from Python but
    // never ASSIGNED -- every list handed to them failed to convert. Both are implicitly
    // convertible from the plain shapes their docs describe, so a bare register name and a
    // (from, [to, ...]) tuple still work.
    py::class_<AGRC::GIMICharFixerConfig::RegRef>(fixerConfig, "RegRef", R"doc(
A register named in a :class:`GIMICharFixerConfig`, optionally conditional on what that register is
bound TO

``"ps-t2"`` on its own means every occurrence of the register. ``RegRef("ps-t2",
GIMICharFixerConfig.RegValChecks.isLightMap)`` means only the ones whose value looks like a
lightmap. Anywhere a :class:`GIMICharFixerConfig.RegRef` is expected, a plain :class:`str` is
accepted too

Parameters
----------
reg: :class:`str`
    The register, eg. ``"ps-t2"``

check: Optional[Callable[[:class:`str`], :class:`bool`]]
    The test over the register's value --- a resource section name. See
    :class:`GIMICharFixerConfig.RegValChecks` for the ready-made ones :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning every occurrence
    )doc")
        .def(py::init<std::string>(), py::arg("reg"))
        .def(py::init([](std::string reg, py::object check) {
            if (check.is_none()) {
                return AGRC::GIMICharFixerConfig::RegRef(std::move(reg));
            }
            return AGRC::GIMICharFixerConfig::RegRef(
                std::move(reg), check.cast<AGRC::GIMICharFixerConfig::RegValCheck>());
        }), py::arg("reg"), py::arg("check"))

        .def_readwrite("reg", &AGRC::GIMICharFixerConfig::RegRef::reg,
            py::doc(":class:`str`: The register"));

    py::implicitly_convertible<py::str, AGRC::GIMICharFixerConfig::RegRef>();

    py::class_<AGRC::GIMICharFixerConfig::RegRemapRule>(fixerConfig, "RegRemapRule", R"doc(
One register rename inside :attr:`GIMICharFixerConfig.objRegRemaps`

Anywhere a :class:`GIMICharFixerConfig.RegRemapRule` is expected, a ``(from, [to, ...])`` or
``(from, [to, ...], keepIfNoneMatch)`` tuple is accepted too

Parameters
----------
from_: :class:`str`
    The register being renamed

to: List[Union[:class:`str`, :class:`GIMICharFixerConfig.RegRef`]]
    What it becomes --- one entry per register it ends up on

keepIfNoneMatch: :class:`bool`
    Whether an occurrence that no conditional target matched keeps its ORIGINAL register, rather than
    being deleted. Only meaningful when ``to`` carries checks :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc")
        .def(py::init<std::string, std::vector<AGRC::GIMICharFixerConfig::RegRef>, bool>(),
            py::arg("from_"), py::arg("to"), py::arg("keepIfNoneMatch") = false)
        .def(py::init([](py::tuple rule) {
            if (rule.size() != 2 && rule.size() != 3) {
                throw py::value_error("A RegRemapRule tuple is (from, [to, ...]) or (from, [to, ...], keepIfNoneMatch)");
            }

            return AGRC::GIMICharFixerConfig::RegRemapRule(
                rule[0].cast<std::string>(),
                rule[1].cast<std::vector<AGRC::GIMICharFixerConfig::RegRef>>(),
                rule.size() == 3 ? rule[2].cast<bool>() : false);
        }), py::arg("rule"))

        .def_readwrite("from_", &AGRC::GIMICharFixerConfig::RegRemapRule::from,
            py::doc(":class:`str`: The register being renamed"))
        .def_readwrite("to", &AGRC::GIMICharFixerConfig::RegRemapRule::to,
            py::doc("List[:class:`GIMICharFixerConfig.RegRef`]: What it becomes"))
        .def_readwrite("keepIfNoneMatch", &AGRC::GIMICharFixerConfig::RegRemapRule::keepIfNoneMatch,
            py::doc(":class:`bool`: Whether an occurrence no conditional target matched keeps its original register"));

    py::implicitly_convertible<py::tuple, AGRC::GIMICharFixerConfig::RegRemapRule>();

    py::class_<AGRC::RegValChecks>(fixerConfig, "RegValChecks", R"doc(
The ready-made tests a :class:`GIMICharFixerConfig.RegRef` can be conditional on --- each one takes
a register's value (a resource section name) and says whether it names that kind of texture

What they are for is a mod that has ALREADY been fixed by hand: a rename conditional on
:meth:`isDiffuse` leaves a register alone when the author already moved the diffuse there
    )doc")
        .def_static("isDiffuse", &AGRC::RegValChecks::isDiffuse, py::arg("val"),
            py::doc(":class:`bool`: Whether 'val' names a diffuse texture"))
        .def_static("isLightMap", &AGRC::RegValChecks::isLightMap, py::arg("val"),
            py::doc(":class:`bool`: Whether 'val' names a lightmap"))
        .def_static("isNormalMap", &AGRC::RegValChecks::isNormalMap, py::arg("val"),
            py::doc(":class:`bool`: Whether 'val' names a normal map"))
        .def_static("isMetalMap", &AGRC::RegValChecks::isMetalMap, py::arg("val"),
            py::doc(":class:`bool`: Whether 'val' names a metal map"))
        .def_static("isShadow", &AGRC::RegValChecks::isShadow, py::arg("val"),
            py::doc(":class:`bool`: Whether 'val' names a shadow ramp"));

    py::class_<AGRC::GIMICharFixerConfig::TexEdit>(fixerConfig, "TexEdit", R"doc(
One texture the fix rewrites, and the register it repoints at the rewritten copy

Parameters
----------
obj: :class:`str`
    The **target** object whose graph holds the register

reg: :class:`str`
    The register the texture hangs off, eg. ``"ps-t1"``

name: :class:`str`
    The name the rewritten texture is filed under

filter: Callable[[:class:`CppTextureFile`], ``None``]
    What the edit does to the texture. It is handed the texture itself, not a copy, so edit it in
    place --- eg. through :meth:`CppTextureFile.getPixels` / :meth:`CppTextureFile.setPixels`, or a
    filter's ``transform``

compress: :class:`bool`
    Whether the written ``.dds`` is compressed :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

srcObj: :class:`str`
    Which **source** object's copy this edit belongs to, or ``""`` for every copy of 'obj'. Only
    meaningful under a **merge**, where several sources land on one target :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

toReg: :class:`str`
    The register to bind the EDITED texture to when it should sit alongside the original rather
    than replace it, or ``""`` to rebind 'reg' itself. A PRE-edit register, exactly like 'reg'
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

check: Optional[Callable[[:class:`str`], :class:`bool`]]
    A test over what 'reg' is bound to, so the edit fires only on the occurrences that match
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning every occurrence
    )doc")
        .def(py::init([](std::string obj, std::string reg, std::string name,
                          const PyTexFilter &filter, bool compress, std::string srcObj,
                          std::string toReg, py::object check) {
            AGRC::GIMICharFixerConfig::TexEdit edit{};
            edit.obj = std::move(obj);
            edit.reg = std::move(reg);
            edit.name = std::move(name);
            edit.filter = toTexFilter(filter);
            edit.compress = compress;
            edit.srcObj = std::move(srcObj);
            edit.toReg = std::move(toReg);
            if (!check.is_none()) {
                edit.check = check.cast<AGRC::GIMICharFixerConfig::RegValCheck>();
            }
            return edit;
        }), py::arg("obj"), py::arg("reg"), py::arg("name"), py::arg("filter"),
            py::arg("compress") = true, py::arg("srcObj") = "", py::arg("toReg") = "",
            py::arg("check") = py::none())

        .def_readwrite("obj", &AGRC::GIMICharFixerConfig::TexEdit::obj,
            py::doc(":class:`str`: The **target** object whose graph holds the register"))
        .def_readwrite("reg", &AGRC::GIMICharFixerConfig::TexEdit::reg,
            py::doc(":class:`str`: The register the texture hangs off, eg. ``\"ps-t1\"``"))
        .def_readwrite("name", &AGRC::GIMICharFixerConfig::TexEdit::name,
            py::doc(":class:`str`: The name the rewritten texture is filed under"))
        .def_readwrite("compress", &AGRC::GIMICharFixerConfig::TexEdit::compress,
            py::doc(":class:`bool`: Whether the written ``.dds`` is compressed. **Default**: ``True``"))
        .def_readwrite("srcObj", &AGRC::GIMICharFixerConfig::TexEdit::srcObj,
            py::doc(":class:`str`: Which **source** object's copy this edit belongs to, or ``\"\"`` for every copy"))
        .def_readwrite("toReg", &AGRC::GIMICharFixerConfig::TexEdit::toReg,
            py::doc(":class:`str`: The register the edited copy is bound to, or ``\"\"`` to rebind :attr:`reg` itself"));

    fixerConfig
        .def(py::init<>())

        .def_readwrite("drawnObjs", &AGRC::GIMICharFixerConfig::drawnObjs, py::doc(R"doc(
List[:class:`str`]: The objects that actually draw, lowercase --- **the SOURCE's**, not the
target's, and it must match the paired :attr:`GIMICharParserConfig.drawnObjs` exactly

Where the target draws a different set, see :attr:`objSplits`
        )doc"))

        .def_readwrite("objSplits", &AGRC::GIMICharFixerConfig::objSplits, py::doc(R"doc(
List[Tuple[:class:`str`, List[:class:`str`]]]: Which of the target's drawn objects each of the
source's becomes --- **empty (the default) means one-to-one**

A split is ``[("head", ["head"]), ("body", ["body", "dress"])]``; a merge names the same target
twice. A source object left out is dropped from the remap entirely
        )doc"))

        .def_readwrite("objRegRemaps", &AGRC::GIMICharFixerConfig::objRegRemaps, py::doc(R"doc(
List[Tuple[:class:`str`, List[Tuple[:class:`str`, List[:class:`str`]]]]]: Registers **renamed** on
one target object's parts --- ``[("head", [("ps-t1", ["ps-t0"]), ("ps-t2", ["ps-t1"])])]``

All of an object's renames are applied in **one pass**, so a shift and a swap are both expressible
and neither re-reads its own output. Naming two targets duplicates the value into both
        )doc"))

        .def_readwrite("objFixCalls", &AGRC::GIMICharFixerConfig::objFixCalls, py::doc(R"doc(
List[Tuple[:class:`str`, List[:class:`str`]]]: Which external library calls one target object
re-issues, in order

An object not named here re-issues ``NNFix``, which is the common case. The mod's own calls to the
three libraries are stripped first, so this is a re-issue rather than an addition
        )doc"))

        .def_readwrite("objRegRemovals", &AGRC::GIMICharFixerConfig::objRegRemovals, py::doc(R"doc(
List[Tuple[:class:`str`, List[Union[:class:`str`, :class:`GIMICharFixerConfig.RegRef`]]]]: Registers
stripped from one **target** object's parts entirely --- ``[("head", ["ps-t3"])]``
        )doc"))

        .def_readwrite("objNewRegVals", &AGRC::GIMICharFixerConfig::objNewRegVals, py::doc(R"doc(
List[Tuple[:class:`str`, List[Tuple[:class:`str`, :class:`str`]]]]: Register values forced onto one
target object

Replaces a value that is already there; a part with no such register does not grow one
        )doc"))

        .def_readwrite("srcObjRegRemovals", &AGRC::GIMICharFixerConfig::srcObjRegRemovals, py::doc(R"doc(
List[Tuple[:class:`str`, List[Union[:class:`str`, :class:`GIMICharFixerConfig.RegRef`]]]]: Registers
stripped from the copies that came from ONE **source** object --- the merge counterpart of
:attr:`objRegRemovals`
        )doc"))

        .def_readwrite("srcObjRegRemaps", &AGRC::GIMICharFixerConfig::srcObjRegRemaps, py::doc(R"doc(
List[Tuple[:class:`str`, List[:class:`GIMICharFixerConfig.RegRemapRule`]]]: Registers renamed on the
copies that came from ONE **source** object --- the merge counterpart of :attr:`objRegRemaps`
        )doc"))

        .def_readwrite("texEdits", &AGRC::GIMICharFixerConfig::texEdits,
            py::doc("List[:class:`GIMICharFixerConfig.TexEdit`]: The textures this fix rewrites"))

        .def_readwrite("copyPreamble", &AGRC::GIMICharFixerConfig::copyPreamble, py::doc(R"doc(
:class:`str`: What the extra ``.ini`` file a **merge** produces says about itself
        )doc"))

        .def_readwrite("moveDrawIndexed", &AGRC::GIMICharFixerConfig::moveDrawIndexed, py::doc(R"doc(
:class:`bool`: Whether the shared ``drawindexed`` is taken off ``("", "ib")`` and re-issued per drawn
object

**Default**: ``False``
        )doc"))

        .def_readwrite("faceDiffuseReg", &AGRC::GIMICharFixerConfig::faceDiffuseReg, py::doc(R"doc(
:class:`str`: The register the face's diffuse hangs off. **Default**: ``"ps-t0"``
        )doc"))

        .def_readwrite("faceLightMapReg", &AGRC::GIMICharFixerConfig::faceLightMapReg, py::doc(R"doc(
:class:`str`: The register the face's lightmap hangs off. **Default**: ``"ps-t1"``

The fix swaps this with :attr:`faceDiffuseReg`, which is what removes the white shiny cheek spots ---
GI 6.x swapped which register the shader reads the two out of
        )doc"))

        .def_readwrite("swapFaceRegs", &AGRC::GIMICharFixerConfig::swapFaceRegs, py::doc(R"doc(
:class:`bool`: Whether to perform that swap at all. **Default**: ``True``

Set it ``False`` only for a fix transcribed from a PRE-6.x row: the swap corrects something GI 6.x
did to the shader, so at 4.0 it moves a correct binding to the wrong register
        )doc"))

        .def_readwrite("removeSrcFixCalls", &AGRC::GIMICharFixerConfig::removeSrcFixCalls, py::doc(R"doc(
:class:`bool`: Whether a remapped section drops the MOD'S OWN ``ORFix``/``NNFix`` calls. **Default**: ``True``

True because the fix re-issues those itself, so a survivor of the mod's own would duplicate. A row
that re-issues nothing --- every pre-6.x one --- must set this ``False``, or the removal deletes the
modder's call and puts nothing back
        )doc"))

        .def_readwrite("removeSrcTexFxCalls", &AGRC::GIMICharFixerConfig::removeSrcTexFxCalls, py::doc(R"doc(
:class:`bool`: Whether to drop EVERY call under the TexFx folder, not just the re-issued ones. **Default**: ``False``

False because a folder match deletes modder content -- a call to a sub-command this fix does not
re-issue duplicates nothing. Several pre-5.0 rows do ask for it
        )doc"));

    // ------------------------------------------------------------------- the factories
    py::class_<PyIniParseFactory>(m, "CppIniParseFactory", R"doc(
A built parser factory, ready to hand to :meth:`CppStrategyOverrides.setParser`

Opaque: there is nothing to read off one. Build it with :func:`makeGIMICharParser`
    )doc");

    py::class_<PyIniFixFactory>(m, "CppIniFixFactory", R"doc(
A built fixer factory, ready to hand to :meth:`CppStrategyOverrides.setFixer`

Opaque: there is nothing to read off one. Build it with :func:`makeGIMICharFixer`
    )doc");

    m.def("makeGIMICharParser", [](const AGRC::GIMICharParserConfig &config) {
        return PyIniParseFactory{AGRC::makeGIMICharParser(config)};
    }, py::arg("config"), py::doc(R"doc(
Builds the parser for a character with the **standard GIMI shape**

That shape is: drawn objects sharing one ``ib`` hash and separated by ``match_first_index``, plus
`blend`_/position/texcoord/ib named outright by their own hashes, plus ``("", "other")`` for the
``VertexLimitRaise`` and ``("", "face")`` for the face diffuse. It also builds every default
download a modder may have left out.

This is the same factory the compiled-in characters use, so a mod type overridden with it is parsed
exactly as one of them would be.

Parameters
----------
config: :class:`GIMICharParserConfig`
    What this character does differently

Returns
-------
:class:`CppIniParseFactory`
    The factory, for :meth:`CppStrategyOverrides.setParser`
    )doc"));

    m.def("makeGIMICharFixer", [](const AGRC::GIMICharFixerConfig &config) {
        return PyIniFixFactory{AGRC::makeGIMICharFixer(config)};
    }, py::arg("config"), py::doc(R"doc(
Builds the fixer for a character with the **standard GIMI shape**

The counterpart of :func:`makeGIMICharParser`, and the same factory the compiled-in characters use.
It builds every graph and register edit the fix needs --- the renames per resource kind, the hash
remap, the ``match_first_index`` rewrite and its `KVP`_ window, the `blend`_ collector, the external
library re-issues and their placement, and the face's register swap --- from the config alone.

Parameters
----------
config: :class:`GIMICharFixerConfig`
    What this character's fix does differently

Returns
-------
:class:`CppIniFixFactory`
    The factory, for :meth:`CppStrategyOverrides.setFixer`
    )doc"));
}
