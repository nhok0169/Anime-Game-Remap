#ifndef AGRemapPyBind_PyBufEditor_H
#define AGRemapPyBind_PyBufEditor_H

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
#include "AGRemapCore/model/strategies/bufEditors/BufEditor.h"


/**
 * @brief
 @rst
 The `pybind11`_-facing ``BufEditor`` -- both the `pybind11`_ trampoline for
 :cpp:class:`AGRemapCore::BufEditor` (lets a pure-`Python`_ subclass override ``fix`` instead of
 only setting :cpp:member:`AGRemapCore::BufEditor::filters`, the same reason
 :cpp:class:`PyBaseBufEditor` exists for :cpp:class:`AGRemapCore::BaseBufEditor`) and the holder for
 the exact `Python`_ callables ``filters`` was constructed with :raw-html:`<br />` :raw-html:`<br />`

 The base's own :cpp:member:`~AGRemapCore::BufEditor::filters` stores each filter already wrapped
 into an opaque ``std::function`` (see :cpp:func:`AGRemapCore::BufFile::Filter`), which
 :cpp:func:`~AGRemapCore::BufEditor::fix` needs but which cannot be unwrapped back into the original
 `Python`_ object -- #filtersObj is what the ``filters`` property actually reads and writes, kept in
 sync with the base's parsed copy on every assignment
 @endrst
 */
class PyBufEditor: public AGRemapCore::BufEditor {
    public:
        using AGRemapCore::BufEditor::BufEditor;

        /**
         * @brief The exact `Python`_ callables ``filters`` was constructed/assigned with, in order
         */
        pybind11::list filtersObj;

        AGRemapCore::BufFile::FixResult fix(AGRemapCore::BufFile &bufFile,
                                             const std::optional<std::string> &fixedBufFile = std::nullopt) override;
};


void initCppBufEditor(pybind11::module_ &m);

#endif
