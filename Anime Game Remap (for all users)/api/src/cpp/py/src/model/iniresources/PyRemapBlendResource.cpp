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

#include "PyRemapBlendResource.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// BufElementType is a small, shareable value (real real-world usage keeps the same cached instance
// reused across several BufElementType-consuming definitions -- see Architecture/CLAUDE.md's own
// warning about this exact type) -- clone each entry via its real copy constructor rather than
// disowning the Python object, so a caller can safely pass the same BufElementType instance to more
// than one RemapBlendResource without either one losing access to it.
std::vector<std::unique_ptr<AGRC::BufElementType>> blendElementsFromList(const py::object &elements) {
    std::vector<std::unique_ptr<AGRC::BufElementType>> result;
    for (auto item : elements) {
        result.push_back(std::make_unique<AGRC::BufElementType>(item.cast<AGRC::BufElementType>()));
    }
    return result;
}


// A Python callable as the core's fixFunc, called with the resource BY REFERENCE.
//
// pybind11's own std::function caster casts an lvalue-reference argument with the 'copy' policy
// whenever no Python wrapper for the object exists yet, and RemapBlendResource is not copyable (it
// owns a vector of unique_ptr). So a fixFunc reached from RemapService's C++ resource loop -- where
// the resource was built by the fixer and never handed to Python -- died with
// "return_value_policy = copy, but type is non-copyable" before the callable ran, logged per
// resource, and the Blend.buf was never written. Calling the same resource's fix() from Python
// worked, because getResources() had already created the wrapper the cast then found; that is why
// every test passed. Found 2026-09-12 by the Yelan -> YelanTranquil prototype, whose per-component
// buffers all come through fixFunc.
//
// A named functor rather than a lambda so the getter can hand back the ORIGINAL Python object
// (std::function::target), which is what the pure-Python attribute always returned.
struct PyFixFunc {
    py::object callable;

    bool operator()(AGRC::RemapBlendResource &resource) const {
        return callable(py::cast(&resource, py::return_value_policy::reference)).cast<bool>();
    }
};

std::function<bool(AGRC::RemapBlendResource&)> toFixFunc(const py::object &fixFunc) {
    if (fixFunc.is_none()) {
        return nullptr;
    }

    return PyFixFunc{fixFunc};
}

py::object fromFixFunc(const std::function<bool(AGRC::RemapBlendResource&)> &fixFunc) {
    if (!fixFunc) {
        return py::none();
    }

    if (const PyFixFunc *held = fixFunc.target<PyFixFunc>()) {
        return held->callable;
    }

    return py::cpp_function(fixFunc);
}
}


void initCppRemapBlendResource(pybind11::module_ &m) {
    // No stats-taking methods rebound here -- srcEncounteredError/srcIsFixed/fixEncounteredError/
    // fixIsFixed/fixExists/hasRequired are all inherited from RemapIniFixResource (and
    // transitively RemapIniResourceMixin) via real Python inheritance, dispatching correctly to
    // this class's own C++ overrides through the vtable -- see PyRemapIniResource.cpp's own note.
    // py::smart_holder -- its base (RemapIniFixResource) uses it, and a holder type must stay
    // consistent throughout one inheritance chain.
    py::class_<AGRC::RemapBlendResource, AGRC::RemapIniFixResource, py::smart_holder>(m, "RemapBlendResource", R"doc(
This class inherits from :class:`RemapIniFixResource`

Class for fixing some ``Blend.buf`` file used by the overall remap process
    )doc")

        .def(py::init([](const std::string &iniFolderPath, const std::string &srcPath, const std::string &fixedPath,
                          AGRC::VGRemap vgRemap, std::string type, const py::object &fixFunc,
                          const py::object &blendElements) {
            std::vector<std::unique_ptr<AGRC::BufElementType>> converted;
            if (!blendElements.is_none()) {
                converted = blendElementsFromList(blendElements);
            }
            // py::init(factory) returning std::unique_ptr<T> by value -- same
            // neither-copyable-nor-movable reasoning as IniGroupedResource's own binding
            // (this class owns a vector<unique_ptr<BufElementType>> member).
            return std::make_unique<AGRC::RemapBlendResource>(iniFolderPath, srcPath, fixedPath, std::move(vgRemap), std::move(type),
                                                                toFixFunc(fixFunc), std::move(converted));
        }), py::arg("iniFolderPath"), py::arg("srcPath"), py::arg("fixedPath"), py::arg("vgRemap"),
            py::arg("type") = "resourceRemapBlend", py::arg("fixFunc") = py::none(), py::arg("blendElements") = py::none(),
            py::doc(R"doc(
Constructs a new blend resource

Parameters
----------
iniFolderPath: :class:`str`
    The path to the folder of the .ini file

srcPath: :class:`str`
    The file path to the resource

fixedPath: :class:`str`
    The file path to the fixed resource

vgRemap: :class:`VGRemap`
    The vertex group remap for the ``Blend.buf`` file

type: :class:`str`
    The name for the type of resource

    **Default**: ``"resourceRemapBlend"``

fixFunc: Optional[Callable[[:class:`RemapBlendResource`], :class:`bool`]]
    Custom function for fixing the resource, overriding the default behavior if given

    **Default**: ``None``

blendElements: Optional[List[:class:`BufElementType`]]
    The sequence of elements for constructing the ``Blend.buf`` file. If this is ``None`` or empty,
    the elements for a GIMI character are used instead

    **Default**: ``None``
        )doc"))

        .def_readwrite("vgRemap", &AGRC::RemapBlendResource::vgRemap, py::doc(R"doc(
:class:`VGRemap`: The vertex group remap for the ``Blend.buf`` file
        )doc"))

        .def_property("fixFunc",
                      [](const AGRC::RemapBlendResource &self) { return fromFixFunc(self.fixFunc); },
                      [](AGRC::RemapBlendResource &self, const py::object &fixFunc) { self.fixFunc = toFixFunc(fixFunc); },
                      py::doc(R"doc(
Optional[Callable[[:class:`RemapBlendResource`], :class:`bool`]]: Custom function for fixing the resource, overriding the default behavior if set
        )doc"))

        .def("createBlend", &AGRC::RemapBlendResource::createBlend, py::doc(R"doc(
Creates the blend file -- a fresh copy of the stored blend elements is cloned into it

Returns
-------
:class:`BlendFile`
    The created blend file
        )doc"))

        .def("fix", &AGRC::RemapBlendResource::fix, py::doc(R"doc(
Fixes the resource -- calls the custom 'fixFunc' if set at construction, otherwise performs a
vertex group remap on the ``Blend.buf`` file

Returns
-------
:class:`bool`
    Whether the resource was fixed
        )doc"));
}
