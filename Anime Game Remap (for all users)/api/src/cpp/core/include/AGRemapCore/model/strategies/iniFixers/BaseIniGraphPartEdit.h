#ifndef AGRemapCore_BaseIniGraphPartEdit_H
#define AGRemapCore_BaseIniGraphPartEdit_H

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
