#ifndef PY_REG_ANCHOR_ADD_H
#define PY_REG_ANCHOR_ADD_H

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
#include <vector>

#include <pybind11/pybind11.h>

#include "PyBaseRegEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAnchorAdd.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegAnchorAdd`\\<std::string, std::string\\> -- holds the
 exact `Python`_ object given for ``adder``, for the reason `PyRegRestrict` holds its ``keyFilter``: a
 ``std::function`` cannot be handed back to `Python`_ as the callable it was built from
 @endrst
 */
class PyRegAnchorAdd: public AGRC::RegAnchorAdd<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegAnchorAdd<std::string, std::string>;

        /**
         * @brief The exact `Python`_ object given for ``adder`` -- a callable, or ``None``
         */
        py::object adderObj;

        /**
         * @brief The exact `Python`_ object given for ``anchorIf`` -- a callable, or ``None``
         */
        py::object anchorIfObj;

        /**
         * @brief Constructs a new anchored add
         *
         * @param anchorKeys The keys whose lines are anchors
         * @param adderObj The Python callable giving each anchor's lines, or ``None``
         * @param placement Where the added lines go
         * @param anchorIfObj The Python callable deciding which keys make a line an anchor, or ``None``
         * @param firstOnly Whether only the part's first anchor gets lines
         * @param skipExisting Whether a line the part already holds is left out
         */
        PyRegAnchorAdd(std::vector<std::string> anchorKeys, py::object adderObj, Core::Placement placement,
                       py::object anchorIfObj, bool firstOnly, bool skipExisting);

        /**
         * @copydoc PyRegAdd::refresh
         */
        void refresh(const py::object &modType);
};


/**
 * @brief Registers the Python-facing ``RegAnchorAdd``
 *
 * @param m The module to register into
 */
void initCppRegAnchorAdd(pybind11::module_ &m);

#endif
