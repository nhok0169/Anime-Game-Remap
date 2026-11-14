#ifndef AGRemapPy_PyGraphCollect_H
#define AGRemapPy_PyGraphCollect_H

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

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollect.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphCollect`\\<std::string, std::string\\> :raw-html:`<br />`
 :raw-html:`<br />`

 Keeps the exact `Python`_ objects given for the per-graph settings and re-derives the C++ members from them at
 the start of every ``edit`` (see #refresh), as :cpp:class:`PyResRegCollect` does -- the predicates are
 `Python`_ callables, which a ``std::function`` round trip cannot hand back as the same callable
 @endrst
 */
class PyGraphCollect: public AGRC::GraphCollect<std::string, std::string> {
    public:
        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphCollect<std::string, std::string>;

        /**
         * @brief The exact Python object given for ``keys``
         */
        py::object keysObj;

        /**
         * @brief The exact Python object given for ``contextKeys``
         */
        py::object contextKeysObj;

        /**
         * @brief The exact Python object given for ``partPredicates``
         */
        py::object partPredicatesObj;

        /**
         * @brief The exact Python object given for ``valPredicates``
         */
        py::object valPredicatesObj;

        /**
         * @brief The exact Python object given for ``trackKeys`` -- a ``bool`` or the per-graph dict
         */
        py::object trackKeysObj;

        /**
         * @brief The exact Python object given for ``keysToTrack``
         */
        py::object keysToTrackObj;

        /**
         * @brief Constructs a new collect -- see the `Python`_ class's own description for each argument
         */
        PyGraphCollect(py::object keys, py::object contextKeys, py::object partPredicates, py::object valPredicates,
                       py::object trackKeys, py::object keysToTrack);

        /**
         * @brief Re-derives every per-graph C++ member from its `Python`_ counterpart
         */
        void refresh();
};


void initCppGraphCollect(pybind11::module_ &m);

#endif
