#ifndef AGRemapPyBind_PyRegRemap_H
#define AGRemapPyBind_PyRegRemap_H

#include <pybind11/pybind11.h>

#include "PyBaseRegEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegRemap`\\<py::object, py::object\\> -- holds the
 exact `Python`_ ``dict`` given for ``keyRemap``, for the same reason `PyRegAdd` holds its own
 list (see that class's note)
 @endrst
 */
class PyRegRemap: public AGRC::RegRemap<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegRemap<py::object, py::object, PyObjectHash, PyObjectEqual>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``keyRemap`` -- a ``dict`` of old key -> remap rules
         @endrst
         */
        py::object keyRemapObj;

        /**
         * @brief Constructs a new bulk key-renaming register edit
         *
         * @param keyRemapObj The Python dict of old key -> remap rules
         */
        explicit PyRegRemap(py::object keyRemapObj);

        /**
         * @copydoc PyRegAdd::refresh
         */
        void refresh(const py::object &modType);
};


void initCppRegRemap(pybind11::module_ &m);

#endif
