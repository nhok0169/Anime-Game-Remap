#ifndef PY_REG_RESTRICT_H
#define PY_REG_RESTRICT_H

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
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRestrict.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegRestrict`\\<std::string, std::string\\> -- holds the
 exact `Python`_ objects given for ``allowedKeys`` and ``keyFilter``, for the reason `PyRegRemove`
 holds its ``dict``: a ``std::function`` cannot be handed back to `Python`_ as the callable it was
 built from, so ``someEdit.keyFilter is theCallableYouPassed`` would otherwise break
 @endrst
 */
class PyRegRestrict: public AGRC::RegRestrict<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegRestrict<std::string, std::string>;

        /**
         * @brief The exact `Python`_ object given for ``allowedKeys`` -- a list of keys, or ``None``
         */
        py::object allowedKeysObj;

        /**
         * @brief The exact `Python`_ object given for ``keyFilter`` -- a callable, or ``None``
         */
        py::object keyFilterObj;

        /**
         * @brief Constructs a new register-restricting edit
         *
         * @param allowedKeysObj The Python list of keys a part may keep, or ``None``
         * @param keyFilterObj The Python callable deciding which keys are governed, or ``None``
         * @param keepFirstOnly Whether a repeated governed key keeps only its first binding
         */
        PyRegRestrict(py::object allowedKeysObj, py::object keyFilterObj, bool keepFirstOnly);

        /**
         * @copydoc PyRegAdd::refresh
         */
        void refresh(const py::object &modType);
};


/**
 * @brief Registers the Python-facing ``RegRestrict``
 *
 * @param m The module to register into
 */
void initCppRegRestrict(pybind11::module_ &m);

#endif
