#ifndef AGRemapPyBind_PyRegAdd_H
#define AGRemapPyBind_PyRegAdd_H

#include <pybind11/pybind11.h>

#include "PyBaseRegEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAdd.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegAdd`\\<py::object, py::object\\> :raw-html:`<br />` :raw-html:`<br />`

 A subclass rather than a plain alias (unlike `PyBaseRegEdit`) because it holds one extra,
 `Python`_-only member: the **exact** `Python`_ object the caller passed as ``vals``. The
 pure-Python original simply did ``self.vals = vals``, so ``someEdit.vals is theListYouPassed``
 held, and mutating that list afterwards changed what the edit did -- both of which a parsed C++
 ``std::vector`` copy would silently break. Keeping the `Python`_ object as the source of truth
 and re-deriving the C++ member from it at the start of every ``edit`` (see #refresh) preserves
 both, at the cost of re-parsing a handful of small tuples per call
 @endrst
 */
class PyRegAdd: public AGRC::RegAdd<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegAdd<py::object, py::object, PyObjectHash, PyObjectEqual>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``vals`` -- a list of ``(key, value)`` tuples
         @endrst
         */
        py::object valsObj;

        /**
         * @brief Constructs a new bulk-add register edit
         *
         * @param valsObj The Python list of (key, value) tuples to add, in the order given
         * @param latest Whether to add the KVPs at the end instead of at the beginning
         */
        PyRegAdd(py::object valsObj, bool latest);

        /**
         * @brief
         @rst
         Re-derives the inherited C++ ``vals`` member from #valsObj -- called at the start of
         every ``edit`` so an in-place mutation of the `Python`_ list is honoured, exactly as it
         was for the pure-Python original
         @endrst
         *
         * @param modType The ``modType`` ``edit`` was called with. Unused here -- only `PyRegNewVals` reads it
         */
        void refresh(const py::object &modType);
};


void initCppRegAdd(pybind11::module_ &m);

#endif
