#ifndef AGRemapPyBind_PyTexEditor_H
#define AGRemapPyBind_PyTexEditor_H

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

#include <pybind11/pybind11.h>
#include <pybind11/typing.h>

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


/**
 * @brief
 @rst
 Wraps a `Python`_ callable as a :cpp:type:`AGRemapCore::TexEditor::Filter` that edits the texture it
 is given IN PLACE :raw-html:`<br />` :raw-html:`<br />`

 Any binding that takes a texture filter from `Python`_ goes through this (or through
 :cpp:func:`toPyRefFunction` for a signature that returns one), never through
 ``pybind11/functional.h``'s own conversion, which hands the filter a COPY of the ``TextureFile`` so
 the written texture is the unedited one
 @endrst
 *
 * @param fn The filter, taking the :class:`CppTextureFile` to edit
 * @return The filter
 * @throws py::type_error If 'fn' is not callable -- ``None`` included, since an edit with no filter
 *    has nothing to do
 */
AGRemapCore::TexEditor::Filter toTexFilter(const pybind11::object &fn);


/**
 * @brief The parameter type a binding declares for a texture filter, so the stub names its argument
 */
using PyTexFilter = pybind11::typing::Callable<void(AGRemapCore::TextureFile&)>;

void initCppTexEditor(pybind11::module_ &m);

#endif
