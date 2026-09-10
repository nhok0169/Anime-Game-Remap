#ifndef AGRemapPyBind_PyRegRemap_H
#define AGRemapPyBind_PyRegRemap_H

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

#include <pybind11/pybind11.h>

#include "PyBaseRegEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegRemap`\\<py::object, py::object\\> -- holds the
 exact `Python`_ ``dict`` given for ``keyRemap``, for the same reason `PyRegAdd` holds its own
 list (see that class's note)
 @endrst
 */
class PyRegRemap: public AGRC::RegRemap<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegRemap<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``keyRemap`` -- a ``dict`` of old key -> remap rules
         @endrst
         */
        py::object keyRemapObj;

        /**
         * @brief Constructs a new bulk key-renaming register edit
         *
         * @param keyRemapObj The Python dict of old key -> remap rules
         */
        explicit PyRegRemap(py::object keyRemapObj);

        /**
         * @copydoc PyRegAdd::refresh
         */
        void refresh(const py::object &modType);
};


void initCppRegRemap(pybind11::module_ &m);

#endif
