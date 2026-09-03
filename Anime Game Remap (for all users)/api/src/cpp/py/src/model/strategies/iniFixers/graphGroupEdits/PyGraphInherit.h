#ifndef AGRemapPyBind_PyGraphInherit_H
#define AGRemapPyBind_PyGraphInherit_H

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphInherit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphInherit`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 A subclass rather than a plain alias, for the two members whose *`Python`_ object identity* is
 observable: ``src``/``dst`` (kept as the exact tuples given, so the getters round-trip without
 rebuilding) and ``partFilter``. The last one is the load-bearing case -- ``pybind11``'s
 ``std::function`` caster cannot hand a callable back to `Python`_ as the *same* callable it was
 built from (it re-wraps it in a fresh ``cpp_function``), so a getter reconstructing from a parsed
 ``std::function`` member would break ``someEdit.partFilter is theFilterYouPassed``, a contract
 ``test_GraphInherit.py`` pins with ``assertIs``. The C++ members are re-derived from these at the
 start of every ``edit`` (see #refresh)
 @endrst
 */
class PyGraphInherit: public AGRC::GraphInherit<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphInherit<std::string, std::string>;

        /**
         * @brief The exact Python tuple given for ``src``
         */
        py::object srcObj;

        /**
         * @brief The exact Python tuple given for ``dst``
         */
        py::object dstObj;

        /**
         * @brief
         @rst
         The exact `Python`_ callable given for ``partFilter``, or ``None``
         @endrst
         */
        py::object partFilterObj;

        /**
         * @brief Constructs a new graph-inheriting edit
         *
         * @param srcObj The Python tuple id of the source graph
         * @param dstObj The Python tuple id of the graph to merge into 'src'
         * @param reg The name of the register used to reference the root `sections`_ of the graph at 'dst'
         * @param latest Whether to insert the KVPs at the back instead of the front
         * @param partFilterObj The Python filter callable, or ``None``
         */
        PyGraphInherit(py::object srcObj, py::object dstObj, std::string reg, bool latest, py::object partFilterObj);

        /**
         * @brief
         @rst
         Re-derives the inherited C++ ``src``/``dst``/``partFilter`` members from their `Python`_
         counterparts -- called at the start of every ``edit`` so an in-place reassignment of any of
         them is honoured, exactly as it was for the pure-Python original
         @endrst
         *
         * @param modType The ``modType`` ``edit`` was called with -- captured by the rebuilt filter, since the C++ side has no ``ModType`` to hand over
         */
        void refresh(const py::object &modType);
};


void initCppGraphInherit(pybind11::module_ &m);

#endif
