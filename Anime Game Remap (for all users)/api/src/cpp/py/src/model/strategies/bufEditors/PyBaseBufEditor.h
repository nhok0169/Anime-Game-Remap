#ifndef AGRemapPyBind_PyBaseBufEditor_H
#define AGRemapPyBind_PyBaseBufEditor_H

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

#include <optional>
#include <string>

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/strategies/bufEditors/BaseBufEditor.h"


/**
 * @brief
 @rst
 The `pybind11`_ trampoline for :cpp:class:`AGRemapCore::BaseBufEditor` -- lets a pure-`Python`_
 subclass override ``fix`` and still be reached through C++ virtual dispatch (eg. a mixed
 C++/`Python`_ list of editors calling through a ``BaseBufEditor*``) :raw-html:`<br />`
 :raw-html:`<br />`

 ``fix``'s return value (:cpp:type:`AGRemapCore::BufFile::FixResult`) is converted from the
 `Python`_ override's result via :cpp:func:`pyToFixResult` rather than `pybind11`_'s automatic
 ``std::variant`` casting -- see that function's own doc comment for why
 @endrst
 */
class PyBaseBufEditor: public AGRemapCore::BaseBufEditor, public pybind11::trampoline_self_life_support {
    public:
        using AGRemapCore::BaseBufEditor::BaseBufEditor;

        AGRemapCore::BufFile::FixResult fix(AGRemapCore::BufFile &bufFile,
                                             const std::optional<std::string> &fixedBufFile = std::nullopt) override;
};


void initCppBaseBufEditor(pybind11::module_ &m);

#endif
