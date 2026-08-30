#ifndef AGRemapPyBind_PyTexEdit_H
#define AGRemapPyBind_PyTexEdit_H

#include <string>

#include <pybind11/pybind11.h>

#include "PyResEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``TexCreate`` -- builds a real :class:`RemapTexAddResource` per created
 texture, and the brand-new `section`_ that references it :raw-html:`<br />` :raw-html:`<br />`

 Both live here rather than in the core class: the resource carries the caller's own `Python`_
 :class:`TexCreator`, and the `section`_ has to be a `Python`_-owned :class:`IfTemplate` (see
 :cpp:class:`AGRemapCore::TexCreate`'s own note)
 @endrst
 */
class PyTexCreate: public PyResCreateMixin<AGRC::TexCreate<py::object, py::object, PyObjectHash, PyObjectEqual>> {
    public:

        /**
         * @brief The exact Python :class:`TexCreator` given -- the editor for the texture file
         */
        py::object texCreator;

        /**
         * @brief
         @rst
         A custom `Python`_ function for creating the texture, or ``None``. Kept as the exact object
         given, for the same identity reason ``PyRemapBlendReplace::fixFunc`` is
         @endrst
         */
        py::object fixFunc;

        /**
         * @brief Constructs a new texture-creating resource edit
         *
         * @param resModObj The Python tuple id of the mod object holding the resource's graph
         * @param texName The name for the type of texture
         * @param texCreator The Python :class:`TexCreator` for the texture file
         * @param resType The name of the type of resource
         * @param fixFunc A custom Python function for creating the texture, or ``None``
         */
        PyTexCreate(py::object resModObj, std::string texName, py::object texCreator, std::string resType, py::object fixFunc);

        py::object pySelf() const override;
        void buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                            const std::string &modName, const std::string &fileKey, Context &ctx) override;
};


void initCppTexCreate(pybind11::module_ &m);

#endif
