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

#include "PyIniParseData.h"

#include <string>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoleChooser.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoles.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTextureRoles(pybind11::module_ &m) {
    py::class_<AGRC::TextureRoles> cls(m, "TextureRoles", R"doc(
What ROLE each of a mod's textures plays -- read off the .ini, never off the folder

A texture is identified by a **hash and a register**, in both games: the hash is either the texture's
own (the `section`_ IS that texture and its ``this =`` names the file), or the mesh's (the `section`_ is
a draw the parser placed on a mod object, and the ``ps-t<N>`` within it says which role the file it
binds plays). A parser builds this and hands it to the fixer through :class:`IniParseData`

A file plays EVERY role the .ini binds it for, since one atlas can serve two components
    )doc");

    py::class_<AGRC::TextureRoles::Role>(cls, "Role", R"doc(
One role a file plays, and how it was decided

Parameters
----------
role: :class:`str`
    The role, eg. ``bodyDiffuse``

how: :class:`str`
    How it was decided, for the log
    )doc")
        .def(py::init([](std::string role, std::string how) {
            return AGRC::TextureRoles::Role{std::move(role), std::move(how)};
        }), py::arg("role"), py::arg("how") = "")
        .def_readwrite("role", &AGRC::TextureRoles::Role::role, py::doc(":class:`str`: The role, eg. ``bodyDiffuse``"))
        .def_readwrite("how", &AGRC::TextureRoles::Role::how, py::doc(":class:`str`: How it was decided, for the log"));

    cls.def(py::init<>())
       .def_property_readonly_static("DeclaredHash", [](py::object) { return AGRC::TextureRoles::DeclaredHash; },
            py::doc(R"doc(:class:`str`: The prefix :meth:`add`'s ``how`` carries when the role was read off the file's OWN ``hash =`` -- the mod naming the game texture its file replaces -- rather than inferred from a register)doc"))
       .def("add", &AGRC::TextureRoles::add, py::arg("file"), py::arg("role"), py::arg("how"), R"doc(
Records that ``file`` plays ``role``, decided by ``how``. The same role twice for one file is kept once

Parameters
----------
file: :class:`str`
    The texture file, as the .ini names it resolved against its folder

role: :class:`str`
    The role it plays

how: :class:`str`
    How that was decided
       )doc")
       .def("addResource", &AGRC::TextureRoles::addResource, py::arg("iniPath"), py::arg("resource"), py::arg("file"), R"doc(
Records the file a .ini's resource `section`_ names

Parameters
----------
iniPath: :class:`str`
    The .ini the resource was declared in

resource: :class:`str`
    The resource `section`_'s name

file: :class:`str`
    The file it names
       )doc")
       .def("rolesOf", &AGRC::TextureRoles::rolesOf, py::return_value_policy::copy, R"doc(
Every file with the roles it plays

Returns
-------
Dict[:class:`str`, List[:class:`TextureRoles.Role`]]
    file -> its roles
       )doc")
       .def("resourcesOf", &AGRC::TextureRoles::resourcesOf, py::arg("iniPath"), py::return_value_policy::copy, R"doc(
The resource `sections`_ one .ini declares

Parameters
----------
iniPath: :class:`str`
    The .ini

Returns
-------
List[Tuple[:class:`str`, :class:`str`]]
    ``(resource, file)`` for each
       )doc");
}


void initCppIniParseData(pybind11::module_ &m) {
    py::class_<AGRC::IniParseData> cls(m, "IniParseData", R"doc(
What a parser learned about a .ini file that a fixer needs and cannot read off the `sections`_ it is
handed -- the classification data a parser keeps, whatever the game

Every parser owns one (:attr:`CppBaseIniParser.parseData`): the parser fills it while it parses, and a
fixer reads it through the parser it is given. A parser that fills nothing leaves the defaults, which
say "everything is there": the file has something to remap, no mod object is undrawn, and no texture
has a known role

A mod object is ``(component, object)``, as the parser's graphs are keyed -- eg. ``("Body", "C")`` for a
slot of a GI component, ``("", "component3")`` for a Wuthering Waves draw slot
    )doc");

    cls.def(py::init<>())
       .def("clear", &AGRC::IniParseData::clear, "Resets everything to the defaults")
       .def("hasRemappableContent", &AGRC::IniParseData::hasRemappableContent, R"doc(
Whether the .ini file carries anything to remap. ``False`` for a file that only WATCHES the character
-- eg. a help overlay whose one section matches the position hash to know she is on screen, which
remapped like a mod would draw a second copy of her :raw-html:`<br />` :raw-html:`<br />`

**Default**: ``True``

Returns
-------
:class:`bool`
    Whether there is anything to remap
       )doc")
       .def("setRemappableContent", &AGRC::IniParseData::setRemappableContent, py::arg("remappable"), R"doc(
Sets :meth:`hasRemappableContent`

Parameters
----------
remappable: :class:`bool`
    Whether the file carries anything to remap
       )doc")
       .def("isRecolourOnly", &AGRC::IniParseData::isRecolourOnly, R"doc(
Whether the .ini file is a RECOLOUR and nothing else: it draws none of the character's mesh, no other
.ini file beside it does, and it overrides a texture of the character that the target does not draw.
Its whole model then comes from downloads, with the mod's textures bound in place of the downloaded
ones :raw-html:`<br />` :raw-html:`<br />`

**Default**: ``False``

Returns
-------
:class:`bool`
    Whether the file is a recolour and nothing else
       )doc")
       .def("setRecolourOnly", &AGRC::IniParseData::setRecolourOnly, py::arg("recolourOnly"), R"doc(
Sets :meth:`isRecolourOnly`

Parameters
----------
recolourOnly: :class:`bool`
    Whether the file is a recolour and nothing else
       )doc")
       .def("carriedSections", &AGRC::IniParseData::carriedSections, R"doc(
`Sections`_ a fix writes into every .ini file it generates, so a SIBLING's toggle keeps working there:
a ``[Constants]`` declaring a copy of each variable a carried recolour's condition reads, and a copy of
each sibling ``[Key...]`` section that cycles one, both under the copy's name

**Default**: empty -- nothing to carry

Returns
-------
:class:`str`
    The `sections`_, as .ini text
       )doc")
       .def("setCarriedSections", &AGRC::IniParseData::setCarriedSections, py::arg("sections"), R"doc(
Sets :meth:`carriedSections`

Parameters
----------
sections: :class:`str`
    The `sections`_, as .ini text
       )doc")
       .def("isUndrawn", &AGRC::IniParseData::isUndrawn, py::arg("modObj"), R"doc(
Whether the mod leaves a mod object UNDRAWN -- on its own character the game never draws it, so a fix
must not draw a download of it over the mod's own buffers

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

Returns
-------
:class:`bool`
    Whether it is undrawn
       )doc")
       .def("addUndrawn", &AGRC::IniParseData::addUndrawn, py::arg("modObj"), R"doc(
Records that the mod leaves ``modObj`` undrawn

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``
       )doc")
       .def("clearUndrawn", &AGRC::IniParseData::clearUndrawn, "Forgets every mod object :meth:`addUndrawn` recorded")
       .def("undrawnObjs", &AGRC::IniParseData::undrawnObjs, py::return_value_policy::copy, R"doc(
Every mod object the mod leaves undrawn

Returns
-------
Set[Tuple[:class:`str`, :class:`str`]]
    The mod objects
       )doc")
       .def("drawRanges", &AGRC::IniParseData::drawRanges, py::arg("modObj"), py::return_value_policy::copy, R"doc(
The draws the mod's own `sections`_ of a mod object make with numbers, in the order they are written -- not
``drawindexed = auto``, which draws the whole of whatever is bound

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

Returns
-------
List[Tuple[:class:`int`, :class:`int`]]
    Each draw's ``(index count, first index)``
       )doc")
       .def("addDrawRange", &AGRC::IniParseData::addDrawRange, py::arg("modObj"), py::arg("range"), R"doc(
Records one of :meth:`drawRanges`

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

range: Tuple[:class:`int`, :class:`int`]
    The draw's ``(index count, first index)``
       )doc")
       .def("drawParts", &AGRC::IniParseData::drawParts, py::arg("modObj"), py::return_value_policy::copy, R"doc(
Which parts of the mod object's `sections`_ hold a ``drawindexed``, counted from 1 over every part of a
`section`_ (an ``if`` line is a part too). Draws all in one part are draws under one condition

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

Returns
-------
Set[:class:`int`]
    The parts
       )doc")
       .def("addDrawPart", &AGRC::IniParseData::addDrawPart, py::arg("modObj"), py::arg("part"), R"doc(
Records one of :meth:`drawParts`

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

part: :class:`int`
    The part
       )doc")
       .def("lateBindings", &AGRC::IniParseData::lateBindings, py::arg("modObj"), py::return_value_policy::copy, R"doc(
The texture registers a mod object's `sections`_ bind only AFTER they have drawn, in the order first bound.
A draw before them renders with whatever the game had bound

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

Returns
-------
List[:class:`str`]
    The registers
       )doc")
       .def("setLateBindings", &AGRC::IniParseData::setLateBindings, py::arg("modObj"), py::arg("regs"), R"doc(
Sets :meth:`lateBindings`

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

regs: List[:class:`str`]
    The registers
       )doc")
       .def("replacesTexturesByHash", &AGRC::IniParseData::replacesTexturesByHash, R"doc(
Whether the mod replaces a game texture BY HASH anywhere -- a texture override with a ``hash`` and a
``this =`` -- so its art reaches every draw that binds that texture, whatever the draw's own `section`_ binds

Returns
-------
:class:`bool`
    Whether it does
       )doc")
       .def("setReplacesTexturesByHash", &AGRC::IniParseData::setReplacesTexturesByHash, py::arg("replaces"), R"doc(
Sets :meth:`replacesTexturesByHash`

Parameters
----------
replaces: :class:`bool`
    Whether it does
       )doc")
       .def("resourceFile", &AGRC::IniParseData::resourceFile, py::arg("section"), R"doc(
The ``filename =`` of one of the mod's resource `sections`_, found without regard to the case of its name,
or ``None``. The value is the .ini's own (a Windows path, relative to the .ini)

Parameters
----------
section: :class:`str`
    The resource `section`_

Returns
-------
Optional[:class:`str`]
    The file
       )doc")
       .def("resourceFiles", &AGRC::IniParseData::resourceFiles, py::return_value_policy::copy, R"doc(
Every resource `section`_ of the mod that names a file

Returns
-------
Dict[:class:`str`, :class:`str`]
    lowercased `section`_ name -> its ``filename =``
       )doc")
       .def("setResourceFile", &AGRC::IniParseData::setResourceFile, py::arg("section"), py::arg("file"), R"doc(
Records one of :meth:`resourceFiles`

Parameters
----------
section: :class:`str`
    The resource `section`_

file: :class:`str`
    Its ``filename =``
       )doc")
       .def("textureRoles", &AGRC::IniParseData::textureRoles, py::return_value_policy::reference_internal, R"doc(
What role each texture of the .ini plays, or ``None`` when the parser records none -- every role the
.ini binds a file for, since one atlas can serve two objects

Returns
-------
Optional[:class:`TextureRoles`]
    The roles
       )doc")
       .def("setTextureRoles", &AGRC::IniParseData::setTextureRoles, py::arg("roles"), R"doc(
Sets the texture roles

Parameters
----------
roles: :class:`TextureRoles`
    The roles
       )doc")
       .def("defer", [](AGRC::IniParseData& self, py::object fill) {
           if (fill.is_none()) {
               self.defer(AGRC::IniParseData::Filler());
           } else {
               self.defer(fill.cast<AGRC::IniParseData::Filler>());
           }
       }, py::arg("fill"), R"doc(
Fills the texture data -- :meth:`textureRoles`, :meth:`roleCandidates`, :meth:`roleFile`,
:meth:`rejections` -- on the first ask of any of them, rather than now. Sorting a mod's textures may
open every one of them, and a parser is built for every .ini a run classifies, most of which no fixer
asks about

Parameters
----------
fill: Optional[Callable[[:class:`IniParseData`], None]]
    What fills them, or ``None`` for nothing
       )doc")
       .def("roleCandidates", &AGRC::IniParseData::roleCandidates, py::arg("role"), py::return_value_policy::copy, R"doc(
The files left for a role once the ones that cannot serve it are refused, in the order they were found

Parameters
----------
role: :class:`str`
    The role

Returns
-------
List[Tuple[:class:`str`, :class:`str`]]
    ``(file, how it was found)`` for each; empty when nothing serves the role
       )doc")
       .def("setRoleCandidates", &AGRC::IniParseData::setRoleCandidates, py::arg("role"), py::arg("candidates"), R"doc(
Sets :meth:`roleCandidates` for a role

Parameters
----------
role: :class:`str`
    The role

candidates: List[Tuple[:class:`str`, :class:`str`]]
    ``(file, how)`` for each
       )doc")
       .def("roleFile", &AGRC::IniParseData::roleFile, py::arg("role"), py::arg("modObj"), py::return_value_policy::reference_internal, R"doc(
The file a mod object takes for a role, or ``None`` when no file of the mod serves the role. The choice
among :meth:`roleCandidates` is per mod object, since a file a mod binds on one object is the better
answer for that object

Parameters
----------
role: :class:`str`
    The role

modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

Returns
-------
Optional[:class:`IniParseData.RoleFile`]
    The file it takes
       )doc")
       .def("setRoleFile", &AGRC::IniParseData::setRoleFile, py::arg("role"), py::arg("modObj"), py::arg("file"), R"doc(
Sets :meth:`roleFile`

Parameters
----------
role: :class:`str`
    The role

modObj: Tuple[:class:`str`, :class:`str`]
    The mod object

file: :class:`IniParseData.RoleFile`
    The file it takes
       )doc")
       .def("thisToggleOf", &AGRC::IniParseData::thisToggleOf, py::arg("resource"), py::return_value_policy::reference_internal, R"doc(
The ``this =`` toggle that chooses a resource, or ``None`` -- a texture override
``hash = <a texture> / if ... this = A / else this = B`` the mod writes to swap one texture for another

Parameters
----------
resource: :class:`str`
    The resource, lowercased

Returns
-------
Optional[:class:`IniParseData.TextureToggle`]
    The toggle
       )doc")
       .def("setThisToggle", &AGRC::IniParseData::setThisToggle, py::arg("resource"), py::arg("toggle"), R"doc(
Records the ``this =`` toggle that chooses ``resource``

Parameters
----------
resource: :class:`str`
    The resource, lowercased

toggle: :class:`IniParseData.TextureToggle`
    The toggle
       )doc")
       .def("registerToggleOf", &AGRC::IniParseData::registerToggleOf, py::arg("resource"), py::return_value_policy::reference_internal, R"doc(
The register toggle that chooses a resource, or ``None`` -- one `section`_ binding one register to several
resources on its branches

Parameters
----------
resource: :class:`str`
    The resource, lowercased

Returns
-------
Optional[:class:`IniParseData.TextureToggle`]
    The toggle
       )doc")
       .def("setRegisterToggle", &AGRC::IniParseData::setRegisterToggle, py::arg("resource"), py::arg("toggle"), R"doc(
Records the register toggle that chooses ``resource``

Parameters
----------
resource: :class:`str`
    The resource, lowercased

toggle: :class:`IniParseData.TextureToggle`
    The toggle
       )doc")
       .def("rejections", &AGRC::IniParseData::rejections, py::return_value_policy::copy, R"doc(
Every file refused for a role, in the order they were refused

Returns
-------
List[:class:`IniParseData.Rejection`]
    The refusals
       )doc")
       .def("blendVersion", &AGRC::IniParseData::blendVersion, py::return_value_policy::copy, R"doc(
The game version whose skeleton numbering the mod's blend gives its bone ids in, when the parser could tell
from the mod's geometry -- a mod's hashes may name one version while its bone ids are another's. ``None``
when the parser did not ask or could not tell

Returns
-------
Optional[:class:`str`]
    The version
       )doc")
       .def("setBlendVersion", &AGRC::IniParseData::setBlendVersion, py::arg("version"), R"doc(
Sets :meth:`blendVersion`

Parameters
----------
version: :class:`str`
    The version
       )doc")
       .def("setTextureResource", &AGRC::IniParseData::setTextureResource, py::arg("resource"), py::arg("file"), R"doc(
Records that the .ini's resource `section`_ ``resource`` names the texture ``file``

Parameters
----------
resource: :class:`str`
    The resource `section`_

file: :class:`str`
    The file, as :meth:`roleFile` and :meth:`rejections` name it
       )doc")
       .def("rolesOfResource", &AGRC::IniParseData::rolesOfResource, py::arg("resource"), R"doc(
What a texture line binding ``resource`` binds: every ``(role, mod object)`` the parser chose the resource's
file for (:meth:`roleFile`), by role and then by mod object. Empty for a resource naming no texture, or one
chosen for nothing

Parameters
----------
resource: :class:`str`
    The resource `section`_, in any case

Returns
-------
List[:class:`IniParseData.ResourceRole`]
    The roles
       )doc")
       .def("refusedRoleOf", &AGRC::IniParseData::refusedRoleOf, py::arg("resource"), R"doc(
The role the parser REFUSED the resource's file for (the first of :meth:`rejections`), or ``None``

Parameters
----------
resource: :class:`str`
    The resource `section`_, in any case

Returns
-------
Optional[:class:`str`]
    The role
       )doc")
       .def("reject", &AGRC::IniParseData::reject, py::arg("file"), py::arg("role"), py::arg("why"), R"doc(
Records that ``file`` was refused for ``role``

Parameters
----------
file: :class:`str`
    The file

role: :class:`str`
    The role it was refused FOR -- it may still serve another

why: :class:`str`
    Why, as a sentence about the file, eg. ``is a flat hairMask, which marks no regions``
       )doc");

    py::class_<AGRC::IniParseData::RoleFile>(cls, "RoleFile", R"doc(
The file a mod object takes for a role -- see :meth:`IniParseData.roleFile`

Parameters
----------
file: :class:`str`
    The file

how: :class:`str`
    How it was found, for the log

tiedFile: :class:`str`
    Another file ranked EQUALLY good for the role, or empty: only a measurement can say which is right,
    so a fixer should say so

tiedHow: :class:`str`
    How :attr:`tiedFile` was found
    )doc")
        .def(py::init([](std::string file, std::string how, std::string tiedFile, std::string tiedHow) {
            return AGRC::IniParseData::RoleFile{std::move(file), std::move(how), std::move(tiedFile), std::move(tiedHow)};
        }), py::arg("file"), py::arg("how") = "", py::arg("tiedFile") = "", py::arg("tiedHow") = "")
        .def_readwrite("file", &AGRC::IniParseData::RoleFile::file, py::doc(":class:`str`: The file"))
        .def_readwrite("how", &AGRC::IniParseData::RoleFile::how, py::doc(":class:`str`: How it was found, for the log"))
        .def_readwrite("tiedFile", &AGRC::IniParseData::RoleFile::tiedFile, py::doc(":class:`str`: Another file ranked equally good, or empty"))
        .def_readwrite("tiedHow", &AGRC::IniParseData::RoleFile::tiedHow, py::doc(":class:`str`: How :attr:`tiedFile` was found"));

    py::class_<AGRC::IniParseData::TextureToggle>(cls, "TextureToggle", R"doc(
A toggle that chooses among several of the mod's texture resources

Parameters
----------
section: :class:`str`
    The `section`_ that chooses (for a register toggle, ``<section> <register>``)

variants: List[:class:`str`]
    Every resource it chooses among, in the order the `section`_ names them

single: :class:`bool`
    Whether it binds ONE resource under a real condition -- off, the original draws with the game's own
    texture

graph: Tuple[:class:`str`, :class:`str`]
    The parser's graph holding the `section`_, as ``(component, mod object)`` -- see
    :attr:`IniGraphModObjKeywords.Texture`. Empty for a register toggle
    )doc")
        .def(py::init([](std::string section, std::vector<std::string> variants, bool single,
                         std::pair<std::string, std::string> graph) {
            return AGRC::IniParseData::TextureToggle{std::move(section), std::move(variants), single, std::move(graph)};
        }), py::arg("section"), py::arg("variants"), py::arg("single") = false,
            py::arg("graph") = std::pair<std::string, std::string>{})
        .def_readwrite("section", &AGRC::IniParseData::TextureToggle::section, py::doc(":class:`str`: The section that chooses"))
        .def_readwrite("variants", &AGRC::IniParseData::TextureToggle::variants, py::doc("List[:class:`str`]: Every resource it chooses among"))
        .def_readwrite("single", &AGRC::IniParseData::TextureToggle::single, py::doc(":class:`bool`: Whether it binds one resource under a real condition"))
        .def_readwrite("graph", &AGRC::IniParseData::TextureToggle::graph, py::doc("Tuple[:class:`str`, :class:`str`]: The parser's graph holding the section"));

    py::class_<AGRC::IniParseData::ResourceRole>(cls, "ResourceRole", R"doc(
One role the parser chose a file for, on one mod object -- see :meth:`IniParseData.rolesOfResource`
    )doc")
        .def_readonly("role", &AGRC::IniParseData::ResourceRole::role, py::doc(":class:`str`: The role"))
        .def_readonly("modObj", &AGRC::IniParseData::ResourceRole::modObj,
                      py::doc("Tuple[:class:`str`, :class:`str`]: The mod object it was chosen for"));

    py::class_<AGRC::IniParseData::Rejection>(cls, "Rejection", R"doc(
A file refused for a role -- see :meth:`IniParseData.rejections`
    )doc")
        .def_readonly("file", &AGRC::IniParseData::Rejection::file, py::doc(":class:`str`: The file"))
        .def_readonly("role", &AGRC::IniParseData::Rejection::role, py::doc(":class:`str`: The role it was refused FOR"))
        .def_readonly("why", &AGRC::IniParseData::Rejection::why, py::doc(":class:`str`: Why, as a sentence about the file"));
}


void initCppTextureRoleChooser(pybind11::module_ &m) {
    py::class_<AGRC::TextureRoleChooser>(m, "TextureRoleChooser", R"doc(
Chooses, from what a parser found out about a mod's textures, the one file each mod object takes for
each role -- and records the answer in an :class:`IniParseData`

A parser collects the evidence (:attr:`candidates`, :attr:`bindings`, :attr:`declared`,
:attr:`declaredFiles`) and :meth:`choose` does the rest, in three steps:

#. **One entry per file**: a file found for a role more than one way keeps the first way it was found
#. **Refusals**, for a :attr:`regionRoles` role only -- a role whose texture marks REGIONS, such as a
   material mask: a file one mod object binds for that role AND another role is not that role's file,
   and a flat file (:attr:`isFlat`) marks no regions. A refused role takes the ordinary "the mod has no
   file for this role" path
#. **Ranking, per mod object**, lower first: a file the mod DECLARES as another role by the ``hash =``
   over it comes after one it does not; then how specifically the mod binds it on THIS object (only
   here, here and elsewhere, nowhere, only elsewhere); then whether the .ini declares a resource for it;
   then how far it is from the .ini. Two files equal on the first four are a tie, recorded in
   :attr:`IniParseData.RoleFile.tiedFile`
    )doc")
        .def(py::init<>())
        .def_readwrite("candidates", &AGRC::TextureRoleChooser::candidates, py::doc(R"doc(
Dict[:class:`str`, List[Tuple[:class:`str`, :class:`str`]]]: Every file found for each role, as
``role -> (file, how it was found)``, in the order found
        )doc"))
        .def_readwrite("bindings", &AGRC::TextureRoleChooser::bindings, py::doc(R"doc(
Dict[:class:`str`, Dict[Tuple[:class:`str`, :class:`str`], Set[:class:`str`]]]: What the mod's own
`sections`_ bind each file for, as ``file -> mod object -> roles``
        )doc"))
        .def_readwrite("declared", &AGRC::TextureRoleChooser::declared, py::doc(R"doc(
Dict[:class:`str`, Set[:class:`str`]]: The roles the mod DECLARES each file as, by writing the game
texture's ``hash =`` over it
        )doc"))
        .def_readwrite("declaredFiles", &AGRC::TextureRoleChooser::declaredFiles,
                        py::doc("Set[:class:`str`]: The files the .ini declares a resource `section`_ for"))
        .def_readwrite("regionRoles", &AGRC::TextureRoleChooser::regionRoles,
                        py::doc("Set[:class:`str`]: The roles whose textures mark regions -- the only ones a file is refused for"))
        .def_readwrite("folder", &AGRC::TextureRoleChooser::folder,
                        py::doc(":class:`str`: The folder of the .ini, which distance is measured from"))
        .def_readwrite("isFlat", &AGRC::TextureRoleChooser::isFlat, py::doc(R"doc(
Optional[Callable[[:class:`str`], :class:`bool`]]: Whether a file is flat, for a :attr:`regionRoles`
role. **Default**: its channel 0 -- the code that says where the regions are -- is constant
        )doc"))
        .def("choose", &AGRC::TextureRoleChooser::choose, py::arg("data"), py::arg("modObjs"), R"doc(
Narrows the candidates and chooses a file for every role and mod object, into ``data``

Parameters
----------
data: :class:`IniParseData`
    Where the answers go

modObjs: List[Tuple[:class:`str`, :class:`str`]]
    The mod objects to choose for
       )doc");
}
