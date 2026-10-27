#ifndef PY_REG_IF_TEMPLATE_ADD_H
#define PY_REG_IF_TEMPLATE_ADD_H

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

#include "PyBaseIniGraphEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegIfTemplateAdd.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegIfTemplateAdd`\\<std::string, std::string\\>. It holds
 the exact `Python`_ ``IfTemplate`` passed (which is what keeps the borrowed template alive), and
 re-derives the core pointer from it at the start of every ``edit`` (see #refresh)
 @endrst
 */
class PyRegIfTemplateAdd: public AGRC::RegIfTemplateAdd<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegIfTemplateAdd<std::string, std::string>;

        /**
         * @brief The exact `Python`_ template, or ``None``
         */
        py::object ifTemplateObj;

        /**
         * @brief Constructs a new template-adding edit
         *
         * @param ifTemplateObj The Python ``IfTemplate``, or None
         * @param position Where in each root the parts go
         * @param key The key ``AfterKey`` inserts after
         */
        PyRegIfTemplateAdd(py::object ifTemplateObj, Position position, std::string key);

        /**
         * @brief Re-derives the core template pointer from #ifTemplateObj
         */
        void refresh();
};


void initCppRegIfTemplateAdd(pybind11::module_ &m);

#endif
