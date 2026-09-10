#ifndef AGRemapCore_InvertAlphaFilter_H
#define AGRemapCore_InvertAlphaFilter_H

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
