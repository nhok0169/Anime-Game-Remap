#ifndef AGRemapPyBind_PyBlendEdit_H
#define AGRemapPyBind_PyBlendEdit_H

#include <string>

#include <pybind11/pybind11.h>

#include "PyResEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BlendEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``RemapBlendReplace`` -- builds a real :class:`RemapBlendResource` per
 referenced `Blend`_ file :raw-html:`<br />` :raw-html:`<br />`

 The vertex-group remap it needs comes from ``modType.getVGRemap(...)``, and the ``ModType`` in
 question is still a pure-`Python`_ class -- which is why the resource model is built here rather
 than in the core class (see :cpp:class:`AGRemapCore::RemapBlendReplace`'s own note)
 @endrst
 */
class PyRemapBlendReplace: public PyResEditMixin<AGRC::RemapBlendReplace<std::string, std::string>> {
    public:

        /**
         * @brief
         @rst
         A custom `Python`_ function for fixing the `Blend`_ file, or ``None`` :raw-html:`<br />`
         :raw-html:`<br />`

         Kept as the exact `Python`_ object given, so ``someEdit.fixFunc is theFunctionYouPassed``
         holds -- ``pybind11``'s ``std::function`` caster cannot hand a callable back as the same
         callable it was built from
         @endrst
         */
        py::object fixFunc;

        /**
         * @brief Constructs a new `Blend`_-replacing resource edit
         *
         * @param resModObj The Python tuple id of the mod object holding the resource's graph
         * @param resType The name of the type of resource
         * @param fixFunc A custom Python function for fixing the `Blend`_ file, or ``None``
         * @param resSubType The name of the subtype of the resource, or ``None``
         * @param fromComp The specific component to remap from, or ``None``
         * @param toComp The specific component to remap to, or ``None``
         */
        PyRemapBlendReplace(py::object resModObj, std::string resType, py::object fixFunc, const py::object &resSubType,
                             const py::object &fromComp, const py::object &toComp);

        py::object pySelf() const override;
        void buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                            const std::string &modName, const std::string &fileKey, Context &ctx) override;
};


void initCppRemapBlendReplace(pybind11::module_ &m);

#endif
