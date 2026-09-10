#ifndef AGRemapPyBind_PyRegSurroundedAdd_H
#define AGRemapPyBind_PyRegSurroundedAdd_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

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
         * @brief The same mirror as #beforeRegsObj, for ``optBeforeRegs``
         */
        py::dict optBeforeRegsObj;

        /**
         * @brief The same mirror as #beforeRegsObj, for ``optAfterRegs``
         */
        py::dict optAfterRegsObj;

        /**
         * @brief Constructs a new `surrounded`_-window-adding edit
         *
         * @param additionsObj The `KVP`_ tuple(s) to add -- one ``(key, value)`` tuple, or a list of them
         * @param beforeRegsObj The registers that must come before 'additionsObj', or ``None`` for none
         * @param afterRegsObj The registers that must come after 'additionsObj', or ``None`` for none
         * @param latest Whether to add 'additionsObj' at the latest valid location instead of the earliest
         * @param optBeforeRegsObj Registers of which at least one must come before 'additionsObj', or ``None`` for none
         * @param optAfterRegsObj Registers of which at least one must come after 'additionsObj', or ``None`` for none
         */
        PyRegSurroundedAdd(py::object additionsObj, py::object beforeRegsObj, py::object afterRegsObj, bool latest,
                           py::object optBeforeRegsObj, py::object optAfterRegsObj);
};


/**
 * @brief
 @rst
 Parses the `Python`_ value given for an edit's ``additions`` into the core's own
 :cpp:type:`AGRemapCore::RegSurroundedAdd::Additions` :raw-html:`<br />` :raw-html:`<br />`

 Accepts either a single ``(key, value)`` tuple of two strings (normalized to a one-entry list) or
 any iterable of such tuples, in order; ``None`` means no entries
 @endrst
 *
 * @param additionsObj The Python value to parse
 *
 * @throw pybind11::type_error If 'additionsObj' is neither a 2-tuple of strings nor an iterable of them
 */
PyRegSurroundedAdd::Core::Additions parseAdditions(const py::object &additionsObj);


/**
 * @brief Converts the core's ``additions`` back into the `Python`_ list of ``(key, value)`` tuples a reader expects
 *
 * @param additions The entries to convert
 */
py::list additionsToPy(const PyRegSurroundedAdd::Core::Additions &additions);


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
