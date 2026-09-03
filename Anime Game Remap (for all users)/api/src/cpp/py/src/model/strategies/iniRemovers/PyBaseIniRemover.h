#ifndef AGRemapPyBind_PyBaseIniRemover_H
#define AGRemapPyBind_PyBaseIniRemover_H

#include <pybind11/pybind11.h>

#include "../../iftemplate/PyIfContentPart.h"  // reuses PyObjectHash/PyObjectEqual
#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The core :cpp:class:`AGRemapCore::BaseIniRemover` specialization every `pybind11`_-facing remover
 is built on
 @endrst
 */
using PyBaseIniRemoverCore = AGRC::BaseIniRemover<std::string, std::string>;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``BaseIniRemover`` -- :cpp:class:`AGRemapCore::BaseIniRemover` plus the one
 piece of `Python`_ state the pure-Python original carries :raw-html:`<br />` :raw-html:`<br />`

 A real subclass rather than a plain alias, for the same reason ``PyBaseIniParser`` is one: that
 state has no core equivalent and could not have. ``iniFile`` is the *`Python`_* ``IniFile``
 (``model/files/IniFile.py``), an unrelated class to :cpp:class:`AGRemapCore::IniFile` -- see
 :cpp:class:`AGRemapCore::IniRemoveContext`'s own note. The original exposes it as a plain public
 attribute (``self.iniFile``, not ``_iniFile``), and this keeps that spelling

 :raw-html:`<br />`

 .. note::
    Because it is a real subclass, it is also what :cpp:class:`AGRemapCore::RemapIniRemover`'s
    ``RemoverBase`` template parameter is instantiated with, so ``RemapIniRemover`` inherits it and
    ``py::class_<PyRemapIniRemover, PyBaseIniRemover>`` is genuine C++ inheritance. See that parameter's
    own documentation
 @endrst
 */
class PyBaseIniRemover: public PyBaseIniRemoverCore {
    public:
        /**
         * @brief Constructs a new remover
         *
         * @param iniFile The Python ``IniFile`` to remove the fix from, or ``None``
         */
        explicit PyBaseIniRemover(py::object iniFile = py::none());

        /**
         * @brief The Python ``IniFile`` the fix will be removed from -- Python-visible as ``iniFile``
         */
        py::object iniFileObj;
};


/**
 * @brief
 @rst
 Binds the ``iniFile``/``remove`` surface every remover shares onto an already-constructed
 ``py::class_`` :raw-html:`<br />` :raw-html:`<br />`

 ``remove`` is re-bound per class rather than inherited from the ``BaseIniRemover`` registration so
 that it dispatches to *that* class's C++ override -- the same reason
 ``bindBaseIniParserCommonMethods`` exists for the parser family
 @endrst
 *
 * @tparam T The remover class being bound
 * @tparam PyClass The ``py::class_`` to chain onto
 *
 * @param cls The class to chain onto
 * @param removeDoc The docstring for ``remove``
 */
template <typename T, typename PyClass>
void bindBaseIniRemoverCommonMethods(PyClass &cls, const char *removeDoc) {
    cls.def_readwrite("iniFile", &T::iniFileObj,
        py::doc(R"doc(:class:`IniFile`: The .ini file that the fix will be removed from)doc"))

       .def("remove", [](T &self, bool parse, bool writeBack, py::object context) {
           // Taken as a py::object defaulting to None, and NOT as
           // py::arg("context") = AGRC::IniRemovalContext(): a default argument is built once at
           // binding time and shared by every call that omits it, so a mutable one is pybind's
           // version of Python's mutable-default-argument bug -- a caller that flipped
           // 'ignoreModType' on the object it was handed would silently change what every later
           // defaulted call does. A fresh one per call, here.
           AGRC::IniRemovalContext removalContext;
           if (!context.is_none()) {
               removalContext = context.cast<AGRC::IniRemovalContext>();
           }

           return self.remove(parse, writeBack, removalContext);
       }, py::arg("parse") = false, py::arg("writeBack") = true, py::arg("context") = py::none(),
          py::doc(removeDoc));
}


void initCppBaseIniRemover(pybind11::module_ &m);

#endif
