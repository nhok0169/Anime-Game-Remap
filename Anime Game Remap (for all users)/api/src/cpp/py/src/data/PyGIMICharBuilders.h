#ifndef AGRemapPyBind_PyGIMICharBuilders_H
#define AGRemapPyBind_PyGIMICharBuilders_H

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

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 A built :cpp:type:`AGRemapCore::IniParseBuilder::Factory`, wrapped so it can be handed straight to
 ``CppStrategyOverrides.setParser`` :raw-html:`<br />` :raw-html:`<br />`

 The alternative -- a `Python`_ callable that returns a strategy -- would have a C++-constructed
 parser cross into `Python`_ and back out through ``holdPyStrategy``, for no reason: nothing in
 `Python`_ ever touches it. ``setParser`` recognises this type and uses the factory as-is
 @endrst
 */
struct PyIniParseFactory {
    AGRC::IniParseBuilder::Factory factory;
};


/**
 * @brief The fixer counterpart of `PyIniParseFactory`
 */
struct PyIniFixFactory {
    AGRC::IniFixBuilder::Factory factory;
};


void initCppGIMICharBuilders(pybind11::module_ &m);

#endif
