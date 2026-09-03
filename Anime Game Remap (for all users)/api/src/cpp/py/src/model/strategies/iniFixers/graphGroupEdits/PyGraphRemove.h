#ifndef AGRemapPyBind_PyGraphRemove_H
#define AGRemapPyBind_PyGraphRemove_H

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphRemove`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 A subclass rather than a plain alias (the same reason `PyRegAdd` is one): it holds the **exact**
 `Python`_ object the caller passed as ``graphIds``. The pure-Python original simply did
 ``self.graphIds = graphIds``, so ``someEdit.graphIds is theListYouPassed`` held (a contract
 ``test_GraphRemove.py`` pins with ``assertIs``) and mutating that list afterwards changed what the
 edit did -- both of which a parsed C++ ``std::vector`` copy would silently break. The C++ member
 is re-derived from it at the start of every ``edit`` (see #refresh)
 @endrst
 */
class PyGraphRemove: public AGRC::GraphRemove<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphRemove<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``graphIds`` -- a list of
         ``(iniIndex, componentName, objectName)`` tuples
         @endrst
         */
        py::object graphIdsObj;

        /**
         * @brief Constructs a new graph-removing edit
         *
         * @param graphIdsObj The Python list of graph id tuples to remove
         */
        explicit PyGraphRemove(py::object graphIdsObj);

        /**
         * @brief
         @rst
         Re-derives the inherited C++ ``graphIds`` member from #graphIdsObj -- called at the start
         of every ``edit`` so an in-place mutation of the `Python`_ list is honoured, exactly as it
         was for the pure-Python original
         @endrst
         */
        void refresh();
};


void initCppGraphRemove(pybind11::module_ &m);

#endif
