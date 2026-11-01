#ifndef AGRemapPyBind_PyTexEditorTrampoline_H
#define AGRemapPyBind_PyTexEditorTrampoline_H

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

#include <string>

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/files/TextureFile.h"


/**
 * @brief
 @rst
 The `pybind11`_ trampoline for :cpp:class:`AGRemapCore::BaseTexEditor` and the two editors derived
 from it -- lets a pure-`Python`_ subclass override ``fix`` and still be reached through C++ virtual
 dispatch :raw-html:`<br />` :raw-html:`<br />`

 A texture resource holds its editor as a ``std::shared_ptr<BaseTexEditor>``, so without this the
 `Python`_ :class:`TexEditor` handed to a :class:`TexReplace` ran as a plain :class:`CppTexEditor`:
 no filter, no file :raw-html:`<br />` :raw-html:`<br />`

 The texture file is handed over **by reference** rather than copied, so what the override does to it
 is what the caller sees
 @endrst
 *
 * @tparam Base The editor class this stands in for
 */
template <typename Base>
class PyTexEditorTrampoline: public Base, public pybind11::trampoline_self_life_support {
    public:
        using Base::Base;

        void fix(AGRemapCore::TextureFile &texFile, const std::string &fixedTexFile) override {
            pybind11::gil_scoped_acquire gil;
            pybind11::function override = pybind11::get_override(static_cast<const Base*>(this), "fix");
            if (override) {
                override(pybind11::cast(&texFile, pybind11::return_value_policy::reference), fixedTexFile);
                return;
            }

            Base::fix(texFile, fixedTexFile);
        }
};

#endif
