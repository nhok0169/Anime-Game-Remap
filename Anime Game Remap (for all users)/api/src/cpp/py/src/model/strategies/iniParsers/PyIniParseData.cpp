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

#include <optional>
#include <string>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseRows.h"
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
       .def("libraries", &AGRC::IniParseData::libraries, py::arg("modObj"), py::return_value_policy::copy, R"doc(
The external libraries a mod object's `sections`_ name on any of their lines (eg. ``RabbitFX``), by the
name the parser gives each

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

Returns
-------
Set[:class:`str`]
    The libraries
       )doc")
       .def("addLibrary", &AGRC::IniParseData::addLibrary, py::arg("modObj"), py::arg("library"), R"doc(
Records one of :meth:`libraries`

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

library: :class:`str`
    The library's name
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
       .def("toggledBindings", &AGRC::IniParseData::toggledBindings, py::arg("modObj"), py::return_value_policy::copy, R"doc(
The texture resources a mod object's own `sections`_ bind ONLY under a toggle, lowercased: every line binding one sits
in a branch that no draw of the object sits in, at a register no line reaching a draw binds. The game's own texture is
what those draws render with whenever the toggle is off

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

Returns
-------
Set[:class:`str`]
    The resources
       )doc")
       .def("setToggledBindings", &AGRC::IniParseData::setToggledBindings, py::arg("modObj"), py::arg("resources"), R"doc(
Sets :meth:`toggledBindings`

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The mod object, ``(component, object)``

resources: Set[:class:`str`]
    The resources, lowercased
       )doc")
       .def("thisOverrideOf", &AGRC::IniParseData::thisOverrideOf, py::arg("hash"), R"doc(
The resource a ``TextureOverride`` `section`_ with ``hash`` 'hash' puts in the texture's place with ``this =``, as
written -- the first such section's -- or ``None``

Parameters
----------
hash: :class:`str`
    The texture's hash, in any case

Returns
-------
Optional[:class:`str`]
    The resource
       )doc")
       .def("setThisOverride", &AGRC::IniParseData::setThisOverride, py::arg("hash"), py::arg("resource"), R"doc(
Sets :meth:`thisOverrideOf`, unless a resource is already recorded for 'hash'

Parameters
----------
hash: :class:`str`
    The texture's hash, in any case

resource: :class:`str`
    The resource
       )doc")
       .def("isThisBound", &AGRC::IniParseData::isThisBound, py::arg("resource"), R"doc(
Whether any `section`_ binds 'resource' with ``this =``

Parameters
----------
resource: :class:`str`
    The resource, in any case

Returns
-------
:class:`bool`
    Whether it is
       )doc")
       .def("addThisBound", &AGRC::IniParseData::addThisBound, py::arg("resource"), R"doc(
Records that a `section`_ binds 'resource' with ``this =`` -- see :meth:`isThisBound`

Parameters
----------
resource: :class:`str`
    The resource, in any case
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
       .def("collected", &AGRC::IniParseData::collected, py::arg("name"), py::return_value_policy::copy, R"doc(
The rows of the :class:`GraphCollect` named 'name' among the parser's :attr:`GIMIParser.parseSteps`, in the order
it collected them. Empty for a name nothing filed

.. warning::
    The rows describe the graphs as they were when the collect ran (see :class:`GraphCollect`). A collect among
    :attr:`GIMIParser.deferredParseSteps` runs on the FIRST ask of this data, so that first ask has to come
    before any edit to the graphs it reads. Asked first after an edit, the rows are read from the edited graphs

Parameters
----------
name: :class:`str`
    The collect's :attr:`GraphCollect.name`

Returns
-------
List[:class:`GraphCollect.Row`]
    The rows
       )doc")
       .def("collectedSections", &AGRC::IniParseData::collectedSections, py::arg("name"), py::return_value_policy::copy, R"doc(
The `sections`_ the :class:`GraphCollect` named 'name' has rows in, in the order they are written in the .ini file

Parameters
----------
name: :class:`str`
    The collect's :attr:`GraphCollect.name`

Returns
-------
List[:class:`str`]
    The sections
       )doc")
       .def("sectionRows", [](const AGRC::IniParseData& self, const std::string& name, const std::string& section) {
                std::vector<AGRC::IniParseData::CollectedRow> rows;
                for (const AGRC::IniParseData::CollectedRow* row : self.sectionRows(name, section)) {
                    rows.push_back(*row);
                }
                return rows;
            }, py::arg("name"), py::arg("section"), R"doc(
The rows of the :class:`GraphCollect` named 'name' in 'section', in the order they are written: each line once however
many graphs reached it, and a key's rows among the section's others rather than grouped by key as a collect files
them. Empty for a section with none

Parameters
----------
name: :class:`str`
    The collect's :attr:`GraphCollect.name`

section: :class:`str`
    The `section`_, as the file names it

Returns
-------
List[:class:`GraphCollect.Row`]
    The rows
       )doc")
       .def("firstCollected", [](const AGRC::IniParseData& self, const std::string& name, const std::string& section,
                                 const std::string& key) -> std::optional<AGRC::IniParseData::CollectedRow> {
                const AGRC::IniParseData::CollectedRow* row = self.firstCollected(name, section, key);
                return row == nullptr ? std::nullopt : std::optional<AGRC::IniParseData::CollectedRow>(*row);
            }, py::arg("name"), py::arg("section"), py::arg("key"), R"doc(
The first row of the :class:`GraphCollect` named 'name' that holds 'key' in 'section' itself -- not in a section it
``run =``\s -- in the order the collect found them. 'section' is matched regardless of case, as 3DMigoto matches a
section's name; 'key' exactly

Parameters
----------
name: :class:`str`
    The collect's :attr:`GraphCollect.name`

section: :class:`str`
    The `section`_

key: :class:`str`
    The key

Returns
-------
Optional[:class:`GraphCollect.Row`]
    The row, or ``None`` when there is none
       )doc")
       .def("setCollected", &AGRC::IniParseData::setCollected, py::arg("name"), py::arg("rows"), R"doc(
Files 'rows' under 'name', replacing what was filed there -- see :meth:`collected`

Parameters
----------
name: :class:`str`
    The name

rows: List[:class:`GraphCollect.Row`]
    The rows
       )doc")
       .def("resourceData", &AGRC::IniParseData::resourceData, py::arg("rules"), R"doc(
Reads the mod's resource data off collected rows and records it: each resource's file (:meth:`resourceFiles`), and for each
object its numbered draws (:meth:`drawRanges`) and the parts they sit in (:meth:`drawParts`), the texture registers it
binds only after it has drawn (:meth:`lateBindings`, unless :meth:`replacesTexturesByHash`), the libraries it names
(:meth:`libraries`), and whether it is hidden -- its `sections`_ draw nothing and skip the game's draw
(:meth:`isUndrawn`). Only the rows in an object's own root `sections`_ count, except that a draw or a ``run =``
anywhere it reaches counts as drawing

Given :attr:`ResourceDataRules.toggles`, also the toggles that choose among the mod's textures:

* **a ``this =`` toggle** (:meth:`thisToggleOf`): a texture override binding several resources on its branches, or
  ONE under a condition that is not in :attr:`ResourceDataRules.notToggles` (off, the original draws with the game's
  texture). When two `sections`_ bind one resource, the one that declares more of its own variants as `sections`_
  keeps it
* **a register toggle** (:meth:`registerToggleOf`): a texture override binding one texture register to several
  resources on its branches, per `section`_ and register

Parameters
----------
rules: :class:`IniParseData.ResourceDataRules`
    What to read, and how
       )doc")
       .def("roleFiles", &AGRC::IniParseData::roleFiles, py::arg("rules"), R"doc(
Reads which file each mod object takes for each texture role, and records it: :meth:`textureRoles`, :meth:`roleCandidates`,
:meth:`rejections`, :meth:`roleFile`, and each texture resource's file (:meth:`rolesOfResource`, :meth:`resourceOfFile`)

A file is a candidate for a role when the .ini states it: a texture override's own ``hash =`` names a game texture of the
role, an object's `section`_ binds it at a register of the role, a linked .ini recolours that texture or binds it on the
same draw, or -- for a ``.dds`` nothing states -- its pixels are the game texture's (:attr:`RoleFileRules.identify`). A
file the folder does not hold, or one a previous fix wrote (``RemapDL`` / ``RemapTex``), is none. A flat file, or one an
object binds for a second role too, is refused for a region role (:attr:`RoleFileRules.regionRoles`)

Each object then takes, in this order:

#. a file not declared by its hash as ANOTHER role
#. the file bound only on this object, then on it and others, then on none, then only on others
#. a file declared by its hash, then one bound at a register, then one known by its pixels
#. of two hashes of one role, the newest (:attr:`RoleFileRules.hashAge`)
#. of two `sections`_ of one hash, the one 3DMigoto applies last: the higher ``match_priority``, then the `section`_
   name later in order
#. the file whose resource is declared first -- reported as a tie (:attr:`RoleFile.tiedFile`)

The variants of one toggle are not ranked against each other: the toggle's first-named variant among the candidates is
chosen, and all of them recorded (:attr:`RoleFile.variants`). A toggle is a ``this =`` or register toggle
(:meth:`thisToggleOf`, :meth:`registerToggleOf`), or one register an object's graph -- through ``run =``, or in a linked
.ini -- binds to several resources under conditions

Parameters
----------
rules: :class:`IniParseData.RoleFileRules`
    What to read, and how
       )doc")
       .def("startingValues", &AGRC::IniParseData::startingValues, py::arg("sections"), py::arg("z3Ctx"), R"doc(
The values the .ini's variables start at, as one condition: ``$x == <value>`` for every variable its ``[Constants]``
declares (``global``, ``local``, ``persist`` alike; one declared with no value starts at ``0``) -- or ``None`` when it
declares none

Which of several values a toggle shows before anyone presses a key is the one whose condition can hold together with
this (:meth:`GraphCollect.valueUnder`). A variable another file declares is left free

Parameters
----------
sections: :class:`str`
    The collect of every key of every `section`_ (:attr:`IniParseRows.Sections`)

z3Ctx: :class:`Z3Context`
    The context the condition is built in

Returns
-------
Optional[:class:`Z3Predicate`]
    The condition
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
       .def("resourceOfFile", &AGRC::IniParseData::resourceOfFile, py::arg("file"), R"doc(
The resource `section`_ that names ``file`` as the mod USES it, or ``None`` -- one file may be declared by several
resources, and the one a section of the mod binds is the one that reaches the draws. Recorded by the parser with
:meth:`setFileResource`

Parameters
----------
file: :class:`str`
    The file, as :meth:`roleFile` names it

Returns
-------
Optional[:class:`str`]
    The resource `section`_
       )doc")
       .def("setFileResource", &AGRC::IniParseData::setFileResource, py::arg("file"), py::arg("resource"), R"doc(
Records that the mod uses the resource `section`_ ``resource`` for ``file`` -- see :meth:`resourceOfFile`

Parameters
----------
file: :class:`str`
    The file

resource: :class:`str`
    The resource `section`_
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

    py::class_<AGRC::IniParseData::RoleFileRules> roleFileRules(cls, "RoleFileRules", R"doc(
How :meth:`IniParseData.roleFiles` reads which file each mod object takes for each role
    )doc");
    py::class_<AGRC::IniParseData::RoleFileRules::Linked>(roleFileRules, "Linked", R"doc(
Another .ini of the mod, linked to this one, whose parse is read too -- see :attr:`IniParseData.RoleFileRules.linked`
    )doc")
        .def(py::init([](const AGRC::IniParseData& data, std::string folder, std::string name) {
            return AGRC::IniParseData::RoleFileRules::Linked{&data, std::move(folder), std::move(name)};
        }), py::arg("data"), py::arg("folder"), py::arg("name") = "", py::keep_alive<1, 2>())
        .def_readwrite("folder", &AGRC::IniParseData::RoleFileRules::Linked::folder,
                       py::doc(":class:`str`: Its folder, which its resources' files are relative to"))
        .def_readwrite("name", &AGRC::IniParseData::RoleFileRules::Linked::name, py::doc(":class:`str`: Its file name, for the log"));
    roleFileRules
        .def(py::init<>())
        .def_readwrite("iniPath", &AGRC::IniParseData::RoleFileRules::iniPath,
                       py::doc(":class:`str`: The .ini's path, which :meth:`IniParseData.textureRoles` files its resources under"))
        .def_readwrite("folder", &AGRC::IniParseData::RoleFileRules::folder,
                       py::doc(":class:`str`: The .ini's folder, which its resources' files are relative to"))
        .def_readwrite("sections", &AGRC::IniParseData::RoleFileRules::sections, py::doc(R"doc(
:class:`str`: The collect of every key of every `section`_ (:attr:`IniParseRows.Sections`), which orders the `sections`_ as written
        )doc"))
        .def_readwrite("declared", &AGRC::IniParseData::RoleFileRules::declared, py::doc(R"doc(
:class:`str`: The collect of every `section`_'s declaration (:attr:`IniParseRows.Declared`), whose ``filename`` names a resource's file
        )doc"))
        .def_readwrite("overrides", &AGRC::IniParseData::RoleFileRules::overrides,
                       py::doc(":class:`str`: The collect of each texture override's ``hash``, ``this`` and ``match_priority``"))
        .def_readwrite("roleRows", &AGRC::IniParseData::RoleFileRules::roleRows, py::doc(R"doc(
:class:`str`: The collect, with conditions, of the rows that STATE a role: a ``this`` in a role's texture graph (mod object
``(IniGraphModObjKeywords.Texture, <role>)``), and a register of :attr:`registerRoles` in an object's graph, through ``run =``
        )doc"))
        .def_readwrite("objRows", &AGRC::IniParseData::RoleFileRules::objRows,
                       py::doc(":class:`str`: The collect of every key of each object's graph, whose OWN `sections`_ say what the object binds"))
        .def_readwrite("objs", &AGRC::IniParseData::RoleFileRules::objs, py::doc(R"doc(
List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: The objects a file is chosen for, each with its root `sections`_
        )doc"))
        .def_readwrite("registerRoles", &AGRC::IniParseData::RoleFileRules::registerRoles, py::doc(R"doc(
Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`str`]]: Which role each register an object binds plays,
as ``object -> {register -> role}``
        )doc"))
        .def_readwrite("roleOfHash", &AGRC::IniParseData::RoleFileRules::roleOfHash, py::doc(R"doc(
Optional[Callable[[:class:`str`], Optional[:class:`str`]]]: The role a game texture hash of the character plays, or ``None``
        )doc"))
        .def_readwrite("identify", &AGRC::IniParseData::RoleFileRules::identify, py::doc(R"doc(
Optional[Callable[[:class:`str`], Optional[:class:`str`]]]: Which game texture a file IS, by its pixels, or ``None`` -- asked only
of a ``.dds`` no hash and no register places. ``None``: no file is identified that way
        )doc"))
        .def_readwrite("hashAge", &AGRC::IniParseData::RoleFileRules::hashAge, py::doc(R"doc(
Optional[Callable[[:class:`str`], :class:`int`]]: How old a hash of the character is, lower being newer -- when the mod declares
a role by two different hashes, the newest is the one the game binds now. ``None``: every hash is as new as any other
        )doc"))
        .def_readwrite("regionRoles", &AGRC::IniParseData::RoleFileRules::regionRoles, py::doc(R"doc(
Set[:class:`str`]: The roles whose texture marks regions, refused when flat or bound as a second role of one object
        )doc"))
        .def_readwrite("isFlat", &AGRC::IniParseData::RoleFileRules::isFlat, py::doc(R"doc(
Optional[Callable[[:class:`str`], :class:`bool`]]: Whether a file is flat -- see :attr:`regionRoles`. ``None``: its channel 0 is constant
        )doc"))
        .def_readwrite("notToggles", &AGRC::IniParseData::RoleFileRules::notToggles, py::doc(R"doc(
List[:class:`str`]: A condition that is not a toggle, lowercased with no spaces -- see :attr:`ResourceDataRules.notToggles`
        )doc"))
        .def_readwrite("fixSection", &AGRC::IniParseData::RoleFileRules::fixSection, py::doc(R"doc(
Optional[Callable[[:class:`str`], :class:`bool`]]: Whether a `section`_ was written by a previous fix, so what it binds is not the mod's use
        )doc"))
        .def_readwrite("linked", &AGRC::IniParseData::RoleFileRules::linked, py::doc(R"doc(
List[:class:`IniParseData.RoleFileRules.Linked`]: The other .ini files of the mod linked to this one -- see :meth:`IniFile.linkedInis`
        )doc"))
        .def_readwrite("log", &AGRC::IniParseData::RoleFileRules::log,
                       py::doc("Optional[Callable[[:class:`str`], None]]: Where what was found is reported, or ``None``"));

    py::class_<AGRC::IniParseData::ResourceDataRules>(cls, "ResourceDataRules", R"doc(
What :meth:`IniParseData.resourceData` reads, and the rules a game reads it by
    )doc")
        .def(py::init<>())
        .def_readwrite("rows", &AGRC::IniParseData::ResourceDataRules::rows, py::doc(R"doc(
:class:`str`: The collect whose rows it reads: every key each object's `sections`_ reach -- see :attr:`IniParseRows.Objs`
        )doc"))
        .def_readwrite("resources", &AGRC::IniParseData::ResourceDataRules::resources, py::doc(R"doc(
:class:`str`: The collect whose rows name each bound resource's ``filename`` -- see :attr:`IniParseRows.Resources` -- or
empty for none
        )doc"))
        .def_readwrite("roots", &AGRC::IniParseData::ResourceDataRules::roots, py::doc(R"doc(
List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: The objects to read, each with its root `sections`_ in
the order they are written
        )doc"))
        .def_readwrite("texRegPrefix", &AGRC::IniParseData::ResourceDataRules::texRegPrefix,
                        py::doc(":class:`str`: The register family a mod binds its textures with. **Default**: ``\"ps-t\"``"))
        .def_readwrite("runDraws", &AGRC::IniParseData::ResourceDataRules::runDraws, py::doc(R"doc(
Optional[Callable[[:class:`str`], :class:`bool`]]: Whether a ``run =`` of a `section`_ counts as a draw -- eg. one of a
library this file does not hold. **Default**: none does
        )doc"))
        .def_readwrite("libraries", &AGRC::IniParseData::ResourceDataRules::libraries, py::doc(R"doc(
List[Tuple[:class:`str`, Callable[[:class:`str`], :class:`bool`]]]: The libraries to record (:meth:`IniParseData.libraries`),
each by whether a key or value names it
        )doc"))
        .def_readwrite("noneUndrawnWhenAll", &AGRC::IniParseData::ResourceDataRules::noneUndrawnWhenAll, py::doc(R"doc(
:class:`bool`: When EVERY object read looks hidden (draws nothing and skips the game's draw), record none of them as
undrawn: a mod the reading does not understand. **Default**: ``False``
        )doc"))
        .def_readwrite("sections", &AGRC::IniParseData::ResourceDataRules::sections, py::doc(R"doc(
:class:`str`: The collect whose rows hold every key of every `section`_ -- see :attr:`IniParseRows.Sections` -- for
:meth:`IniParseData.thisOverrideOf` and :meth:`IniParseData.isThisBound`, or empty for none
        )doc"))
        .def_readwrite("toggles", &AGRC::IniParseData::ResourceDataRules::toggles, py::doc(R"doc(
:class:`str`: The collect whose rows hold each texture override's ``this =`` and texture register lines, with the
conditions they sit under, for :meth:`IniParseData.thisToggleOf` and :meth:`IniParseData.registerToggleOf` -- or
empty for none
        )doc"))
        .def_readwrite("textureRoots", &AGRC::IniParseData::ResourceDataRules::textureRoots, py::doc(R"doc(
List[Tuple[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]]: The texture objects whose `sections`_ may choose
among the mod's textures, each with its root `sections`_: a ``this =`` toggle is read in these, and a register toggle
in these and in :attr:`roots`
        )doc"))
        .def_readwrite("togglesIn", &AGRC::IniParseData::ResourceDataRules::togglesIn, py::doc(R"doc(
Optional[Callable[[:class:`str`], :class:`bool`]]: Which `sections`_ may hold a toggle. **Default**: ``None``, every
`section`_ named ``TextureOverride...``
        )doc"))
        .def_readwrite("notToggles", &AGRC::IniParseData::ResourceDataRules::notToggles, py::doc(R"doc(
List[:class:`str`]: The conditions that do not make a toggle, lowercased and without spaces -- eg. a condition every
section of a game's mods is wrapped in. **Default**: none
        )doc"))
        .def_readwrite("log", &AGRC::IniParseData::ResourceDataRules::log, py::doc(R"doc(
Optional[Callable[[:class:`str`], None]]: Where a draw range that cannot be read is reported, or ``None``
        )doc"));

    py::class_<AGRC::IniParseRows>(m, "IniParseRows", R"doc(
The names the standard row sets of a parser are filed under (:meth:`IniParseData.collected`)
    )doc")
        .def_readonly_static("Resources", &AGRC::IniParseRows::Resources,
                             ":class:`str`: Every resource `section`_ the mod binds anywhere, by its ``filename``")
        .def_readonly_static("Declared", &AGRC::IniParseRows::Declared, R"doc(
:class:`str`: Every `section`_'s ``filename``, ``type``, ``format`` and ``stride`` (and any key the parser adds), over the
whole file
        )doc")
        .def_readonly_static("Objs", &AGRC::IniParseRows::Objs,
                             ":class:`str`: Every key the `sections`_ of the mod objects the parser chose reach, through ``run =``")
        .def_readonly_static("Sections", &AGRC::IniParseRows::Sections, R"doc(
:class:`str`: Every key of every `section`_ over the whole file. A section ``run =`` reaches is met again under its
caller, so a reader of one section's own lines keeps each line once
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
        .def_readwrite("variants", &AGRC::IniParseData::RoleFile::variants, py::doc(R"doc(
List[:class:`str`]: The files of the toggle :attr:`file` is a variant of, in the order the toggle names them
(:attr:`file` among them), or empty when it is no toggle's. Each plays the same role on the same object
        )doc"))
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
