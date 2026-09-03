#ifndef AGRemapPyBind_PyGraphRename_H
#define AGRemapPyBind_PyGraphRename_H

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphRename`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 A subclass rather than a plain alias (the same reason `PyGraphRemove` is one): it holds the
 **exact** `Python`_ object the caller passed as ``renameFunc``. The pure-Python original simply did
 ``self.renameFunc = renameFunc``, so ``someEdit.renameFunc is theFunctionYouPassed`` held (a
 contract ``test_GraphRename.py`` pins with ``assertIs``) -- and `pybind11`_'s ``std::function``
 caster cannot hand a callable back as the *same* callable anyway, since it re-wraps it in a fresh
 ``cpp_function``. The C++ member is re-derived from it at the start of every ``edit`` (see
 #refresh)
 @endrst
 */
class PyGraphRename: public AGRC::GraphRename<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphRename<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``renameFunc`` -- a callable taking the old
         `section`_ name and returning the new one
         @endrst
         */
        py::object renameFuncObj;

        /**
         * @brief Constructs a new `section`_-renaming edit
         *
         * @param renameFuncObj The Python callable used to rename a `section`_
         */
        explicit PyGraphRename(py::object renameFuncObj);

        /**
         * @brief
         @rst
         Re-derives the inherited C++ ``renameFunc`` member from #renameFuncObj -- called at the
         start of every ``edit`` so reassigning ``renameFunc`` after construction is honoured,
         exactly as it was for the pure-Python original :raw-html:`<br />` :raw-html:`<br />`

         A non-callable (including ``None``) leaves the C++ member empty, which
         `AGRC::GraphRename::edit` treats as "rename nothing"
         @endrst
         */
        void refresh();
};


/**
 * @brief Registers the Python-facing ``GraphRename``
 *
 * @param m The module to register into
 */
void initCppGraphRename(pybind11::module_ &m);

#endif
