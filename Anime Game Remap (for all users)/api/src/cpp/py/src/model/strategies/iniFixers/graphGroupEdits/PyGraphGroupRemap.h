#ifndef AGRemapPyBind_PyGraphGroupRemap_H
#define AGRemapPyBind_PyGraphGroupRemap_H

#include <unordered_map>

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphGroupRemap`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 A subclass rather than a plain alias, for the same reason `PyGraphInherit` is one: it keeps the
 **exact** `Python`_ ``dict`` given for ``remap``, so ``someEdit.remap is theDictYouPassed`` holds
 and an in-place mutation of it changes what the edit does. The C++ ``remap`` member is re-derived
 from it at the start of every ``edit``/``remapGraphs`` (see #refresh)
 @endrst
 */
class PyGraphGroupRemap: public AGRC::GraphGroupRemap<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphGroupRemap<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ ``dict`` given for ``remap`` -- source graph id tuples mapped to lists
         of ``(iniIndex, component, object)`` (optionally plus a rename callable) target tuples
         @endrst
         */
        py::object remapObj;

        /**
         * @brief Constructs a new graph-remapping edit
         *
         * @param remapObj The Python remap dict
         */
        explicit PyGraphGroupRemap(py::object remapObj);

        /**
         * @brief
         @rst
         Re-derives the inherited C++ ``remap`` member from #remapObj -- called at the start of
         every ``edit``/``remapGraphs`` so an in-place mutation of the `Python`_ dict is honoured,
         exactly as it was for the pure-Python original
         @endrst
         */
        void refresh();

        /**
         * @brief
         @rst
         The original `Python`_ callable a parsed rename function was built from, or ``None``
         :raw-html:`<br />` :raw-html:`<br />`

         Needed by :meth:`remapGraphs`, whose user-supplied ``createToGraph`` is handed the rename
         function as its fourth argument and must receive the caller's *own* callable back -- not a
         fresh ``cpp_function`` re-wrapping of it, which is all a ``std::function`` can be cast
         into. Keyed by the address of the stored ``std::function``, which is stable for as long as
         the parsed ``remap`` member lives
         @endrst
         *
         * @param renameFunc The parsed rename function to look up
         */
        py::object renameFuncToPy(const RenameFunc &renameFunc) const;

    private:
        std::unordered_map<const void*, py::object> renameFuncObjs_;
};


void initCppGraphGroupRemap(pybind11::module_ &m);

#endif
