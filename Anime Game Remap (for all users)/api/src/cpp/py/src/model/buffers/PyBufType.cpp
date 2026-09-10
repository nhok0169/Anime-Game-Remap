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

#include "PyBufType.h"

#include "AGRemapCore/model/buffers/BufType.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBufType(pybind11::module_ &m) {
    // Registered under the bare 'BufType' name -- the pure-Python original
    // (api/src/py/FixRaidenBoss2/model/buffers/BufType.py) has been deleted; this is a full
    // replacement, not a wrapper (see Documentation/CLAUDE.md's naming-pitfall section: the
    // pybind11 registration itself must own the bare name, not be imported "as" it, or autodoc
    // collapses to "alias of").
    py::class_<AGRC::BufType, py::smart_holder>(m, "BufType", R"doc(
The common base for any type used to describe the structure of a ``.buf`` file

.. note::
    Unlike the pure-Python original this replaces, this class has no ``decode``/``encode`` methods
    of its own -- see :class:`BufDataType`/:class:`BufElementType` (whose Python originals both
    overrode ``decode``/``encode`` with genuinely incompatible signatures -- a single value vs. a
    list of values -- that only Python's duck typing let share one base method name)
    )doc")

        .def_property("name", &AGRC::BufType::getName, &AGRC::BufType::setName, py::doc(R"doc(
:class:`str`: The name of the type
        )doc"));
}
