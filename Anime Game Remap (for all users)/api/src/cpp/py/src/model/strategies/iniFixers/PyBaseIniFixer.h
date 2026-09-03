#ifndef AGRemapPyBind_PyBaseIniFixer_H
#define AGRemapPyBind_PyBaseIniFixer_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniFixers/IniFixingContext.h"

#include "../../iftemplate/PyIfContentPart.h"  // reuses PyObjectHash/PyObjectEqual
#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The core :cpp:class:`AGRemapCore::BaseIniFixer` specialization every `pybind11`_-facing fixer is
 built on
 @endrst
 */
using PyBaseIniFixerCore = AGRC::BaseIniFixer<std::string, std::string>;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``BaseIniFixer`` -- :cpp:class:`AGRemapCore::BaseIniFixer` plus the two
 `Python`_ handles the (now deleted) pure-Python original carried :raw-html:`<br />`
 :raw-html:`<br />`

 The same arrangement, and for the same reasons, as ``PyBaseIniParser`` -- see that class's own
 note. ``_parser`` and ``_iniFile`` are `Python`_ objects with no C++ counterpart to point at
 (``AGRemapCore::IniFile`` is an unrelated class to the `Python`_ ``IniFile``), and real callers
 read both straight off a fixer

 :raw-html:`<br />`

 .. note::
    Because it is a real subclass, it is also what :cpp:class:`AGRemapCore::GIMIFixer`'s
    ``FixerBase`` template parameter is instantiated with, so ``py::class_<PyGIMIFixer,
    PyBaseIniFixer>`` is genuine C++ inheritance
 @endrst
 */
class PyBaseIniFixer: public PyBaseIniFixerCore {
    public:
        /**
         * @brief Constructs a new fixer
         *
         * @param parser The Python parser to retrieve data for the fix, or ``None``
         */
        explicit PyBaseIniFixer(py::object parser = py::none());

        /**
         * @brief The Python parser this fixer retrieves data from -- Python-visible as ``_parser``
         */
        py::object parserObj;

        /**
         * @brief
         @rst
         The Python ``IniFile`` that will be fixed -- Python-visible as ``_iniFile``, and read from
         ``parser._iniFile`` at construction, matching the pure-Python original's own
         ``self._iniFile = parser._iniFile``
         @endrst
         */
        py::object iniFileObj;

        /**
         * @brief
         @rst
         Fixes the ``.ini`` file and hands the result back as `Python`_ objects -- what the ``fix``
         binding actually calls :raw-html:`<br />` :raw-html:`<br />`

         Separate from :cpp:func:`AGRemapCore::BaseIniFixer::fix` for two reasons. The core takes
         the parse data as a parameter, while the `Python`_ ``fix`` signature every caller uses
         (``IniFile._fix``'s own ``fixer.fix(keepBackup = ..., fixOnly = ..., hideOrig = ...)``)
         does not -- a `Python`_ fixer sources it from its own parser instead. And the core result
         is keyed by file path only, while the original also keys a pathless ``.ini`` file's fix by
         its integer group index :raw-html:`<br />` :raw-html:`<br />`

         Returns ``None`` here, matching the base fixer's own empty result
         @endrst
         *
         * @param keepBackup Whether to keep backups for the .ini file
         * @param fixOnly Whether to only fix the .ini file without undoing any fixes
         * @param hideOrig Whether to hide the mod for the original character
         * @param fixingCtx The per-call options for this fix -- see :cpp:class:`AGRemapCore::IniFixingContext`
         *
         * @return A ``Dict[Union[str, int], str]``, or ``None``
         */
        virtual py::object fixToPy(bool keepBackup, bool fixOnly, bool hideOrig, AGRemapCore::IniFixingContext fixingCtx);
};


/**
 * @brief
 @rst
 Binds the ``_parser``/``_iniFile``/``clear``/``fix`` surface every fixer shares onto an
 already-constructed ``py::class_`` -- the fixer counterpart of ``bindBaseIniParserCommonMethods``,
 and re-bound per class for the same reason
 @endrst
 *
 * @tparam T The fixer class being bound
 * @tparam PyClass The ``py::class_`` to chain onto
 *
 * @param cls The class to chain onto
 * @param fixDoc The docstring for ``fix`` -- what a fixer hands back is the one thing that differs
 */
template <typename T, typename PyClass>
void bindBaseIniFixerCommonMethods(PyClass &cls, const char *fixDoc) {
    cls.def_readwrite("_parser", &T::parserObj,
        py::doc(R"doc(:class:`BaseIniParser`: The associated parser to retrieve data for the fix)doc"))

       .def_readwrite("_iniFile", &T::iniFileObj,
        py::doc(R"doc(:class:`IniFile`: The .ini file that will be fixed)doc"))

       .def("clear", [](T &self) {
           self.clear();
       }, py::doc(R"doc(Resets any saved states within the fixer)doc"))

       .def("fix", [](T &self, bool keepBackup, bool fixOnly, bool hideOrig, py::object context) {
           // Taken as a py::object defaulting to None, and NOT as
           // py::arg("context") = AGRemapCore::IniFixingContext(): a default argument is built once
           // at binding time and shared by every call that omits it, so a mutable one is pybind's
           // version of Python's mutable-default-argument bug -- a caller that flipped
           // 'isLastModType' on the object it was handed would silently change what every later
           // defaulted call does. A fresh one per call, here.
           AGRemapCore::IniFixingContext fixingCtx;
           if (!context.is_none()) {
               fixingCtx = context.cast<AGRemapCore::IniFixingContext>();
           }

           return self.fixToPy(keepBackup, fixOnly, hideOrig, fixingCtx);
       }, py::arg("keepBackup") = true, py::arg("fixOnly") = false, py::arg("hideOrig") = false,
          py::arg("context") = py::none(), py::doc(fixDoc));
}


void initCppBaseIniFixer(pybind11::module_ &m);

#endif
