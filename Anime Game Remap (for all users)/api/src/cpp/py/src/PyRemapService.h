#ifndef AGRemapPyBind_PyRemapService_H
#define AGRemapPyBind_PyRemapService_H

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
