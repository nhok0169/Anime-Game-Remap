#include "PyGIMICharBuilders.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"

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

    py::class_<AGRC::GIMICharFixerConfig::TexEdit>(fixerConfig, "TexEdit", R"doc(
One texture the fix rewrites, and the register it repoints at the rewritten copy
    )doc")
        .def(py::init([](std::string obj, std::string reg, std::string name,
                          AGRC::TexEditor::Filter filter, bool compress) {
            AGRC::GIMICharFixerConfig::TexEdit edit{};
            edit.obj = std::move(obj);
            edit.reg = std::move(reg);
            edit.name = std::move(name);
            edit.filter = std::move(filter);
            edit.compress = compress;
            return edit;
        }), py::arg("obj"), py::arg("reg"), py::arg("name"), py::arg("filter"),
            py::arg("compress") = true)

        .def_readwrite("obj", &AGRC::GIMICharFixerConfig::TexEdit::obj,
            py::doc(":class:`str`: The **target** object whose graph holds the register"))
        .def_readwrite("reg", &AGRC::GIMICharFixerConfig::TexEdit::reg,
            py::doc(":class:`str`: The register the texture hangs off, eg. ``\"ps-t1\"``"))
        .def_readwrite("name", &AGRC::GIMICharFixerConfig::TexEdit::name,
            py::doc(":class:`str`: The name the rewritten texture is filed under"))
        .def_readwrite("compress", &AGRC::GIMICharFixerConfig::TexEdit::compress,
            py::doc(":class:`bool`: Whether the written ``.dds`` is compressed. **Default**: ``True``"));

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
List[Tuple[:class:`str`, List[:class:`str`]]]: Registers stripped from one **target** object's parts
entirely --- ``[("head", ["ps-t3"])]``
        )doc"))

        .def_readwrite("objNewRegVals", &AGRC::GIMICharFixerConfig::objNewRegVals, py::doc(R"doc(
List[Tuple[:class:`str`, List[Tuple[:class:`str`, :class:`str`]]]]: Register values forced onto one
target object

Replaces a value that is already there; a part with no such register does not grow one
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
