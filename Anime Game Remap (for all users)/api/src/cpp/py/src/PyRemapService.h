#ifndef AGRemapPyBind_PyRemapService_H
#define AGRemapPyBind_PyRemapService_H

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
 Registers ``RemapService`` -- the **model** half of the remap, with no UI of its own
 :raw-html:`<br />` :raw-html:`<br />`

 The `Python`_-facing ``RemapServiceCLI`` (``remapServiceCLI.py``) wraps one of these and supplies
 everything on the other side of that line: the log file, the tips, and turning what a user typed
 into the ids/versions/enums this class takes
 @endrst
 *
 * @param m The module to register into
 */
void initCppRemapService(pybind11::module_ &m);

#endif
