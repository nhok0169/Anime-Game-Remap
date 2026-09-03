#ifndef AGRemapPyBind_PyRegNewVals_H
#define AGRemapPyBind_PyRegNewVals_H

#include <pybind11/pybind11.h>

#include "PyBaseRegEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegNewVals`\\<py::object, py::object\\> -- holds the
 exact `Python`_ ``dict`` given for ``vals``, for the same reason `PyRegAdd` holds its own list
 (see that class's note)
 @endrst
 */
class PyRegNewVals: public AGRC::RegNewVals<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegNewVals<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``vals`` -- a ``dict`` of register name -> new value
         @endrst
         */
        py::object valsObj;

        /**
         * @brief Constructs a new value-assigning register edit
         *
         * @param valsObj The Python dict of register name -> new value
         * @param addNewKVPs Whether to add new KVPs for register keys that do not exist yet
         */
        PyRegNewVals(py::object valsObj, bool addNewKVPs);

        /**
         * @brief
         @rst
         Re-derives the inherited C++ ``vals`` member from #valsObj -- called at the start of
         every ``edit``, both so an in-place mutation of the `Python`_ ``dict`` is honoured and
         because this is where 'modType' gets closed over :raw-html:`<br />` :raw-html:`<br />`

         A `ReplaceIf` value's predicate is invoked as ``predicate(oldValue, modType)`` here --
         one argument wider than every ``replaceVals`` calls it with. The `Python`_ ``modType``
         has to arrive by capture rather than through the C++ core's own
         :cpp:type:`AGRemapCore::RegNewVals::ModTypePredicate` parameter, because the `Python`_
         API's ``ModType`` is a pure-`Python`_ class with no :cpp:class:`AGRemapCore::ModType` to
         cast to -- so the lambda built here ignores the ``const ModType*`` the core hands it
         (always ``nullptr`` from this layer) and uses the captured object instead
         @endrst
         *
         * @param modType The ``modType`` ``edit`` was called with, passed on to every ReplaceIf predicate
         */
        void refresh(const py::object &modType);
};


void initCppRegNewVals(pybind11::module_ &m);

#endif
