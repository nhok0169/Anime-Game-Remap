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

#ifndef AGRemapPyBind_PyBufEdit_H
#define AGRemapPyBind_PyBufEdit_H

#include <string>

#include <pybind11/pybind11.h>

#include "PyResEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BufEdit.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;

/**
 * @brief
 @rst
 The `pybind11`_-facing ``BufReplace`` -- names one of a mod's buffers by its kind and builds a
 :class:`RemapIniFixResource` of that kind for it, for a grouped fix to write
 @endrst
 */
class PyBufReplace: public PyResEditMixin<AGRC::BufReplace<std::string, std::string>> {
    public:
        PyBufReplace(py::object resModObj, std::string kind, const py::object &resSubType);

        py::object pySelf() const override;
        void buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                            const std::string &modName, const std::string &fileKey, Context &ctx) override;
};

void initCppBufReplace(pybind11::module_ &m);

#endif
