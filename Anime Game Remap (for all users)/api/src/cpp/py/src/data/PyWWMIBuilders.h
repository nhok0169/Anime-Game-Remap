#ifndef PyWWMIBuilders_H
#define PyWWMIBuilders_H

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


/**
 * @brief
 @rst
 Registers the builders for a Wuthering Waves character -- ``makeWWMIParser`` and
 ``makeWWMIFixer``, with their configs :raw-html:`<br />` :raw-html:`<br />`

 The counterpart of ``PyGIMICharBuilders`` for the WWMI shape: one mesh drawn in several
 components, all skinned in one merged skeleton, remapped onto another character of the same shape
 @endrst
 *
 * @param m The module to register into
 */
void initCppWWMIBuilders(pybind11::module_ &m);

#endif
