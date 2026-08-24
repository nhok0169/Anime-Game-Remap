#include "PyPixelFilter.h"

#include <vector>

#include "AGRemapCore/model/strategies/texEditors/texFilters/PixelFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"
#include "PyTexFilterCommon.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    // Classifies one 'transforms' list entry once (not per-pixel): a real CppBasePixelTransform
    // instance gets called directly in C++ for every pixel (the fast path this whole class exists
    // for); anything else (a plain function, a bound classmethod, ...) falls back to a genuine
    // Python call per pixel, exactly like the pure-Python original.
    struct TransformEntry {
        AGRC::BasePixelTransform *native;
        py::object pyObj;
    };

    // A plain 'py::isinstance<AGRC::BasePixelTransform>(t)' check is not enough to decide the fast
    // path is safe: a *pure-Python* BasePixelTransform subclass (eg. 'class Foo(BasePixelTransform):
    // def transform(self, ...): ...') still passes that isinstance check (it really does carry a
    // real bound BasePixelTransform C++ subobject), but has no C++ vtable entry for its Python-level
    // override -- there is no trampoline here (see PyBaseTexFilter.cpp's '__call__' for why one
    // isn't otherwise needed). Calling entry.native->transform(...) directly for such an instance
    // would silently invoke the no-op base implementation instead of the real Python override.
    //
    // The reliable discriminator: ordinary Python attribute lookup on a *class* returns the exact
    // same descriptor object for every class in the MRO that doesn't itself define the attribute --
    // so type(t).transform 'is' CppBasePixelTransform.transform for every *native* C++ leaf class
    // (CppCorrectGamma, CppTempControl, ...), none of which re-.def("transform", ...) their own
    // copy, but is a genuinely different (real Python function) object for any pure-Python override.
    std::vector<TransformEntry> classifyTransforms(const py::object &transformsObj, const py::object &baseTransformDescriptor) {
        std::vector<TransformEntry> entries;
        for (auto item : transformsObj) {
            py::object t = py::reinterpret_borrow<py::object>(item);
            bool isNativeOverride = false;
            if (py::isinstance<AGRC::BasePixelTransform>(t)) {
                py::object classTransform = t.attr("__class__").attr("transform");
                isNativeOverride = classTransform.is(baseTransformDescriptor);
            }
            AGRC::BasePixelTransform *native = isNativeOverride ? &t.cast<AGRC::BasePixelTransform&>() : nullptr;
            entries.push_back({native, t});
        }
        return entries;
    }

}


void initCppPixelFilter(pybind11::module_ &m) {
    py::class_<AGRC::PixelFilter, AGRC::BaseTexFilter, py::smart_holder>(m, "CppPixelFilter", R"doc(
This class inherits from :class:`CppBaseTexFilter`

Manipulates each pixel within an image

.. note::
    Every whole-image filter in this codebase (eg. :class:`ColourReplaceFilter`) is, under the
    hood, also just a C++ loop over every pixel -- `Compressonator`_ has no vectorized whole-image
    pixel-remap API the way `Pillow`_ did for the pure-Python original, so there's no "whole image
    at once" fast path left to prefer instead. A :class:`CppBasePixelTransform` placed in
    :attr:`transforms` runs directly in C++ for every pixel, at the same cost as a dedicated
    filter's own inlined loop body -- only a plain Python callable placed in :attr:`transforms`
    still pays a real per-pixel Python call
    )doc")

        .def(py::init<>())

        // 'transforms' is a plain Python list (like TexEditor.filters) -- read fresh every call
        // rather than converted once at construction, so in-place mutation (eg. .append(...))
        // takes effect on the next transform() call, matching the pure-Python original's own
        // 'self.transforms' list semantics.
        .def("transform", [](py::object self, py::object texFileObj) {
            py::object transformsObj = self.attr("transforms");
            py::object baseTransformDescriptor = py::type::of<AGRC::BasePixelTransform>().attr("transform");
            std::vector<TransformEntry> entries = classifyTransforms(transformsObj, baseTransformDescriptor);
            if (entries.empty()) {
                return;
            }

            AGRC::TextureFile &texFile = syncTextureFileFromImg(texFileObj);
            int width = texFile.getWidth();
            int height = texFile.getHeight();

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    AGRC::Colour pixel = texFile.getPixel(x, y);

                    for (auto &entry : entries) {
                        if (entry.native != nullptr) {
                            entry.native->transform(pixel, x, y);
                        } else {
                            entry.pyObj(py::cast(&pixel, py::return_value_policy::reference), x, y);
                        }
                    }

                    texFile.setPixel(x, y, pixel);
                }
            }

            syncTextureFileToImg(texFileObj);
        }, py::arg("texFile"), py::doc(R"doc(
Changes each individual pixel in the image

Parameters
----------
texFile: :class:`TextureFile`
    The texture to be edited
        )doc"));
}
