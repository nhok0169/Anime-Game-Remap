#ifndef AGRemapPyBind_PyRegRemove_H
#define AGRemapPyBind_PyRegRemove_H

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
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegRemove`\\<py::object, py::object\\> -- holds the
 exact `Python`_ ``dict`` given for ``removeKeys``, for the same reason `PyRegAdd` holds its own
 list (see that class's note) :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    Keeping the original ``dict`` matters more here than for the other reg edits: this one's
    values are `Python`_ callables, and `pybind11`_ cannot hand a ``std::function`` back to
    `Python`_ as the *same* callable it was built from -- it re-wraps it in a fresh
    ``cpp_function``. Reconstructing the ``dict`` from the parsed C++ member would therefore
    quietly break both ``someEdit.removeKeys is theDictYouPassed`` and
    ``someEdit.removeKeys["a"] is thePredicateYouPassed``
 @endrst
 */
class PyRegRemove: public AGRC::RegRemove<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegRemove<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``removeKeys`` -- a ``dict`` of key -> optional
         check predicate
         @endrst
         */
        py::object removeKeysObj;

        /**
         * @brief Constructs a new bulk key-removing register edit
         *
         * @param removeKeysObj The Python dict of key -> optional check predicate
         */
        explicit PyRegRemove(py::object removeKeysObj);

        /**
         * @copydoc PyRegAdd::refresh
         */
        void refresh(const py::object &modType);
};


void initCppRegRemove(pybind11::module_ &m);

#endif
