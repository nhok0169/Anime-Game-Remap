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

#include "PyGraphVarRename.h"

#include <map>
#include <memory>
#include <string>
#include <utility>

#include <pybind11/stl.h>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"


void initCppGraphVarRename(pybind11::module_ &m) {
    using Core = AGRC::GraphVarRename<std::string, std::string>;
    py::class_<Core, PyBaseIniGraphEdit, py::smart_holder> cls(m, "GraphVarRename", R"doc(
This class inherits from :class:`BaseIniGraphEdit`

Renames `3DMigoto`_ variables everywhere in the `sections`_ of some caller/callee graph of :class:`IniSectionGraph`:
in each key (``global persist $x``, ``post $x``), each value and each ``if`` / ``else if`` condition. Which
``$name`` count is :meth:`IniNamingTools.varsIn`'s answer -- a namespaced reference names another file's variable
and is left alone

For a `section`_ copied out of another ``.ini`` whose conditions read that file's own variables: a file with no
``namespace`` names its variables after its PATH, so another file cannot read them, and the copy reads a variable
of its own under a name no other file uses

Parameters
----------
renames: Dict[:class:`str`, :class:`str`]
    A variable's lower-cased name -> its new name, both without the ``$`` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``{}``

z3: Optional[:class:`Z3Context`]
    The `Z3`_ context every condition of an edited `section`_ is parsed again in, so a renamed one reads its new names
    and a `section`_ copied from another ``.ini`` shares the context of the `sections`_ it now sits beside; ``None`` to
    rename only the conditions' text. Kept alive as long as this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](std::map<std::string, std::string> renames, AGRC::Z3Context* z3) {
        return std::make_unique<Core>(std::move(renames), z3);
    }), py::arg("renames") = std::map<std::string, std::string>{}, py::arg("z3") = nullptr, py::keep_alive<1, 3>());

    cls.def_readwrite("renames", &Core::renames, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: A variable's lower-cased name -> its new name, both without the ``$``
    )doc"));

    cls.def_static("renameIn", [](AGRC::IfTemplate<std::string, std::string>& section,
                                  const std::map<std::string, std::string>& renames, AGRC::Z3Context* z3) {
        Core::renameIn(section, renames, z3);
    }, py::arg("section"), py::arg("renames"), py::arg("z3") = nullptr, py::doc(R"doc(
Renames the variables of one `section`_, as :meth:`edit` does each `section`_ of a graph

Parameters
----------
section: :class:`IfTemplate`
    The `section`_, modified in place

renames: Dict[:class:`str`, :class:`str`]
    A variable's lower-cased name -> its new name, both without the ``$``

z3: Optional[:class:`Z3Context`]
    The `Z3`_ context every condition is parsed again in, or ``None`` to rename only their text :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc"));

    cls.def("edit", [](Core &self, py::object graph, const py::object &modType, const std::string &modName,
                       const py::object &partFilter, bool trackKeys, const py::object &keysToTrack) {
        (void)modType;
        (void)partFilter;
        (void)trackKeys;
        (void)keysToTrack;
        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);
        self.Core::edit(parsedGraph, nullptr, modName);
        return graph;
    }, py::arg("graph"), py::arg("modType"), py::arg("modName") = "", py::arg("partFilter") = py::none(),
       py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(), py::doc(R"doc(
Renames the variables of every `section`_ of 'graph' by :attr:`renames`

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to edit

modType: Optional[:class:`ModType`]
    The type of mod to fix. Unused by this edit

modName: :class:`str`
    The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
    Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

trackKeys: :class:`bool`
    Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[Set[:class:`str`]]
    Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IniSectionGraph`
    The same graph that was passed in
    )doc"));
}
