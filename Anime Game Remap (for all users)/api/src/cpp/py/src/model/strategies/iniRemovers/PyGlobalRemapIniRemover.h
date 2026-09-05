#ifndef AGRemapPyBind_PyGlobalRemapIniRemover_H
#define AGRemapPyBind_PyGlobalRemapIniRemover_H

#include <pybind11/pybind11.h>

#include "PyRemapIniRemover.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``GlobalRemapIniRemover`` :raw-html:`<br />` :raw-html:`<br />`

 The same relationship to :cpp:class:`PyRemapIniRemover` that
 :cpp:class:`AGRemapCore::GlobalRemapIniRemover` has to :cpp:class:`AGRemapCore::RemapIniRemover`: it
 keeps the caller's own `Python`_ ``IniFile`` on :cpp:member:`PyBaseIniRemover::iniFileObj` and
 re-points its context at it on every #remove (see :cpp:func:`PyRemapIniRemover::refresh`), and it
 forces :cpp:member:`AGRemapCore::IniRemovalContext::ignoreModType` on :raw-html:`<br />`
 :raw-html:`<br />`

 .. note::
    It derives from :cpp:class:`PyRemapIniRemover` rather than from a
    ``GlobalRemapIniRemover<..., PyBaseIniRemover>`` instantiation, so that
    ``py::class_<PyGlobalRemapIniRemover, PyRemapIniRemover>`` is genuine C++ inheritance and the whole
    ``RemapIniRemover`` surface (``getTargetSectionNames``/``getRemovedSectionNames``/
    ``getRemovedResources``, and the `Python`_ ``IniFile`` handling in
    :cpp:func:`PyRemapIniRemover::refresh`) is inherited rather than duplicated. The core
    :cpp:class:`AGRemapCore::GlobalRemapIniRemover` is therefore **not** in this object's own bases --
    splicing it in would put ``RemapIniRemover`` into the chain twice. There is nothing in it to
    inherit but the one-line #remove, which is spelled out here instead
 @endrst
 */
class PyGlobalRemapIniRemover: public PyRemapIniRemover {
    public:

        /**
         * @brief Constructs a new remover
         *
         * @param iniFile The Python ``IniFile`` to remove the fix from
         */
        explicit PyGlobalRemapIniRemover(py::object iniFile = py::none());

        /**
         * @brief
         @rst
         :cpp:func:`PyRemapIniRemover::remove`, with
         :cpp:member:`AGRemapCore::IniRemovalContext::ignoreModType` forced on
         @endrst
         *
         * @param parse Ignored -- see :cpp:func:`AGRemapCore::RemapIniRemover::remove`. **Default**: ``false``
         * @param writeBack Whether to write back the new text content of the .ini file. **Default**: ``true``
         * @param context The per-call options, with ``ignoreModType`` ignored. **Default**: a default-constructed one
         *
         * @return The new text content of the .ini file
         */
        std::string remove(bool parse = false, bool writeBack = true,
                           AGRC::IniRemovalContext context = AGRC::IniRemovalContext()) override;
};


void initCppGlobalRemapIniRemover(pybind11::module_ &m);

#endif
