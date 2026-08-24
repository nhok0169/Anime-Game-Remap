#ifndef AGRemapCore_InvertAlphaFilter_H
#define AGRemapCore_InvertAlphaFilter_H

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Inverts the alpha channel of an image (``255 - alpha`` for every pixel) :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        Unlike its sibling per-pixel :cpp:class:`InvertAlpha`, this class uses ``255 - alpha`` (a
        "true" invert) -- see that class's own doc comment for the discrepancy
     @endrst
     */
    class InvertAlphaFilter: public BaseTexFilter {
        public:
            void transform(TextureFile &texFile) override;
    };
}

#endif
