#ifndef AGRemapPyBind_PyRemapServiceCLI_H
#define AGRemapPyBind_PyRemapServiceCLI_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/RemapServiceCLI.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_ trampoline for ``RemapServiceCLI`` :raw-html:`<br />` :raw-html:`<br />`

 This class exists for one method in particular. :cpp:func:`AGRemapCore::RemapServiceCLI::addTips`
 is an empty hook in core on purpose -- a tip names a command-line option (``--undo`` and friends)
 and those belong to the argument parser, which lives in `Python`_. The `Python`_ subclass
 (``remapServiceCLI.py``) is where they are actually written, and this is what lets
 :cpp:func:`AGRemapCore::RemapServiceCLI::fix` call down into it :raw-html:`<br />`
 :raw-html:`<br />`

 #fix and #createLog are overridable for the same reason, so a subclass can wrap either without
 having to reimplement the other
 @endrst
 */
// trampoline_self_life_support is required alongside py::smart_holder: it is what keeps the
// Python half of a subclass alive when only C++ still holds the object. pybind11 refuses to
// compile the combination without it.
class PyBindRemapServiceCLI: public AGRC::RemapServiceCLI, public py::trampoline_self_life_support {
    public:
        using AGRC::RemapServiceCLI::RemapServiceCLI;

        void fix() override {
            PYBIND11_OVERRIDE(void, AGRC::RemapServiceCLI, fix);
        }

        void createLog() override {
            PYBIND11_OVERRIDE(void, AGRC::RemapServiceCLI, createLog);
        }

        void addTips() override {
            PYBIND11_OVERRIDE(void, AGRC::RemapServiceCLI, addTips);
        }

        void printModsToFix() override {
            PYBIND11_OVERRIDE(void, AGRC::RemapServiceCLI, printModsToFix);
        }
};


/**
 * @brief
 @rst
 Registers ``CppRemapServiceCLI`` -- the C++ half of the command-line front end :raw-html:`<br />`
 :raw-html:`<br />`

 Bound under a ``Cpp`` name rather than the bare one because the class a caller actually wants is
 the `Python`_ ``RemapServiceCLI`` that subclasses it, following the same convention as
 ``CppVersion``/``CppGlobalModTypes``
 @endrst
 *
 * @param m The module to register into
 */
void initCppRemapServiceCLI(pybind11::module_ &m);

#endif
