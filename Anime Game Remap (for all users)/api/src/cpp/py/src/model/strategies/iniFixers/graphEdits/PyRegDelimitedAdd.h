#ifndef AGRemapPyBind_PyRegDelimitedAdd_H
#define AGRemapPyBind_PyRegDelimitedAdd_H

#include <utility>

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegDelimitedAdd.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegDelimitedAdd`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 Same shape as `PyRegSurroundedAdd`, and for the same reasons: the raw `Python`_ ``dict`` given for
 ``delimiterRegs`` is kept at #delimiterRegsObj so a read gives back a real ``dict``, while the
 inherited ``delimiterRegs`` (the core's predicate map) is re-derived only when explicitly
 reassigned through this class's own property setter. There is no other derived state to keep in
 sync -- the core reads its delimiters straight off each part at ``edit()`` time
 @endrst
 */
class PyRegDelimitedAdd: public AGRC::RegDelimitedAdd<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegDelimitedAdd<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ ``dict`` given for ``delimiterRegs`` at construction (or the most recent
         reassignment) -- ``None`` is materialized into a real empty ``dict``
         @endrst
         */
        py::dict delimiterRegsObj;

        /**
         * @brief Constructs a new per-segment adding edit
         *
         * @param additionsObj The `KVP`_ tuple(s) to add -- one ``(key, value)`` tuple, or a list of them
         * @param delimiterRegsObj The registers that delimit the segments, or ``None`` for none
         */
        PyRegDelimitedAdd(py::object additionsObj, py::object delimiterRegsObj,
                           bool pathEndOnlyWhenUndelimited = false);
};


/**
 * @brief Registers the Python-facing ``RegDelimitedAdd``
 *
 * @param m The module to register into
 */
void initCppRegDelimitedAdd(pybind11::module_ &m);

#endif
