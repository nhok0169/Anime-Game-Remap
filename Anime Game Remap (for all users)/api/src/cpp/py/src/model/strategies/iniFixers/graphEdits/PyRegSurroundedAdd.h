#ifndef AGRemapPyBind_PyRegSurroundedAdd_H
#define AGRemapPyBind_PyRegSurroundedAdd_H

#include <utility>

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegSurroundedAdd`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 Unlike `PyRegFillMissing`/`PyGraphRename`, this holds no ``refresh()``-rebuilt member: the pure-Python
 original parses ``beforeRegs``/``afterRegs`` into their filter/tracked-key form exactly once, inside
 ``__init__``, never again -- reassigning :attr:`beforeRegs`/:attr:`afterRegs` after construction
 already goes stale for the original too (its own ``_beforeFilters``/``_afterFilters``/
 ``_trackedKeys`` are ``__init__``-only derived state). This subclass mirrors that exactly: the
 raw `Python`_ dicts are kept at #beforeRegsObj/#afterRegsObj (so a read gives back a real
 ``dict``, and no ``assertIs``-style test on either exists to break), while the inherited
 ``beforeRegs``/``afterRegs`` (and the base's own private filter/tracked-key state, computed once by
 its constructor) are re-derived only when explicitly reassigned through this class's own property
 setters -- never automatically on every ``edit()`` call
 @endrst
 */
class PyRegSurroundedAdd: public AGRC::RegSurroundedAdd<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegSurroundedAdd<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ ``dict`` given for ``beforeRegs`` at construction (or the most recent
         reassignment) -- ``None`` is materialized into a real empty ``dict``, matching the pure-Python
         original's own ``{} if beforeRegs is None else beforeRegs``
         @endrst
         */
        py::dict beforeRegsObj;

        /**
         * @brief The same mirror as #beforeRegsObj, for ``afterRegs``
         */
        py::dict afterRegsObj;

        /**
         * @brief Constructs a new `surrounded`_-window-adding edit
         *
         * @param additionObj The `KVP`_ tuple to add
         * @param beforeRegsObj The registers that must come before 'additionObj', or ``None`` for none
         * @param afterRegsObj The registers that must come after 'additionObj', or ``None`` for none
         * @param latest Whether to add 'additionObj' at the latest valid location instead of the earliest
         */
        PyRegSurroundedAdd(py::object additionObj, py::object beforeRegsObj, py::object afterRegsObj, bool latest);
};


/**
 * @brief Parses a `Python`_ 2-tuple into the core's own ``(K, V)`` addition pair
 *
 * @param additionObj The Python value to parse
 *
 * @throw pybind11::type_error If 'additionObj' isn't a 2-length sequence
 */
std::pair<std::string, std::string> parseAddition(const py::object &additionObj);


/**
 * @brief
 @rst
 Parses a `Python`_ ``Optional[Dict[Any, Optional[Callable[[Any], bool]]]]`` into the core's own
 :cpp:type:`AGRemapCore::RegSurroundedAdd::RegMap` :raw-html:`<br />` :raw-html:`<br />`

 ``None`` (for the whole dict, or for any individual value) is preserved as "accept any value" --
 an empty :cpp:type:`AGRemapCore::RegSurroundedAdd::Predicate`
 @endrst
 *
 * @param regsObj The Python value to parse
 */
PyRegSurroundedAdd::Core::RegMap parseRegMap(const py::object &regsObj);


/**
 * @brief Registers the Python-facing ``RegSurroundedAdd``
 *
 * @param m The module to register into
 */
void initCppRegSurroundedAdd(pybind11::module_ &m);

#endif
