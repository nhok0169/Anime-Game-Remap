#ifndef PY_REG_DRAW_RANGE_REMAP_H
#define PY_REG_DRAW_RANGE_REMAP_H

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
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegDrawRangeRemap.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegDrawRangeRemap`\\<std::string, std::string\\> -- holds the
 exact `Python`_ object given for ``reroute``, for the reason `PyRegRestrict` holds its ``keyFilter``: a
 ``std::function`` cannot be handed back to `Python`_ as the callable it was built from
 @endrst
 */
class PyRegDrawRangeRemap: public AGRC::RegDrawRangeRemap<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegDrawRangeRemap<std::string, std::string>;

        /**
         * @brief The exact `Python`_ object given for ``reroute`` -- a callable, or ``None``
         */
        py::object rerouteObj;

        /**
         * @brief Constructs a new draw-range remap
         *
         * @param keptTriangleIds The source index of every kept triangle, ascending
         * @param rerouteObj The Python callable deciding which remapped ranges are drawn by a ``run =``, or ``None``
         */
        PyRegDrawRangeRemap(std::vector<std::size_t> keptTriangleIds, py::object rerouteObj);

        /**
         * @copydoc PyRegAdd::refresh
         */
        void refresh(const py::object &modType);
};


/**
 * @brief Registers the Python-facing ``RegDrawRangeRemap``
 *
 * @param m The module to register into
 */
void initCppRegDrawRangeRemap(pybind11::module_ &m);

#endif
