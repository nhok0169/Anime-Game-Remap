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

#include "PyTextureRegisters.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/TextureRegisters.h"


void initCppTextureRegisters(pybind11::module_ &m) {
    py::class_<AGRC::TextureRegisters, CoreModMappedAssets, py::smart_holder>(m, "TextureRegisters", R"doc(
This class inherits from :class:`ModMappedAssets`

Where a character's own draws bind each texture role, per game version, pre-populated with this project's real data

The slot is the ``component`` (``component0``, ...), the role the ``type`` (``upperDiffuse``), and the value
the register (``ps-t3``). The same role handed to RabbitFX is the type ``<role>@rabbitfx``, valued with
RabbitFX's key (``Resource\\RabbitFX\\Diffuse``), and the register another pass of the slot binds it at (an
outline, named as :class:`DrawPasses` names the pass) is the type ``<role>@<pass>``
(``upperDiffuse@other``) :raw-html:`<br />` :raw-html:`<br />`

A row whose ``name`` is a GAME's (the game's name) and whose ``component`` is ``""`` is
that game's default for every character and slot -- :meth:`ModType.getTextureRegister` falls back to it.
So a game update that moves a role to another register for every character (GI 5.x moved the diffuse from
``ps-t0`` to ``ps-t1``) is one row

:raw-html:`<br />`

.. note::
    Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/``getKeys``/
    ``versionOf``/``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:

    * version (version index)
    * name
    * component
    * type
    )doc")
        .def(py::init([](const py::object &map) {
            return std::make_unique<AGRC::TextureRegisters>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to

    **Default**: ``None``
        )doc"));
}
