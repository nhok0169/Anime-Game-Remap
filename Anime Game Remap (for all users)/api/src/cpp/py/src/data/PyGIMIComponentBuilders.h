#ifndef PyGIMIComponentBuilders_H
#define PyGIMIComponentBuilders_H

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
 Registers the builders for a skin of SEVERAL components -- ``makeGIMIComponentParser`` and
 ``makeGIMIMergeFixer``, with their configs :raw-html:`<br />` :raw-html:`<br />`

 The counterpart of ``PyGIMICharBuilders`` for the shape where a character's mesh is a ``Body``, a
 ``Bang`` and an ``Eye`` with separate buffers, rather than one mesh with several draw ranges
 @endrst
 *
 * @param m The module to register into
 */
void initCppGIMIComponentBuilders(pybind11::module_ &m);

#endif
