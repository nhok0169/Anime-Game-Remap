#ifndef AGRemapCore_BaseIniGraphPartEdit_H
#define AGRemapCore_BaseIniGraphPartEdit_H

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

#include "AGRemapCore/model/strategies/iniFixers/BaseIniPartEdit.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class for a filter that edits some part of a caller/callee graph
     (:cpp:class:`IniSectionGraph`) within a ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     Adds nothing of its own over :cpp:class:`BaseIniPartEdit` -- exactly like the pure-Python
     original, this exists purely to mark the graph-editing half of the edit hierarchy apart from
     the rest (:cpp:class:`BaseIniGraphGroupEdit`, for instance, deliberately derives from
     :cpp:class:`BaseIniPartEdit` directly instead)
     @endrst
     */
    class BaseIniGraphPartEdit: public BaseIniPartEdit {
    };
}

#endif
