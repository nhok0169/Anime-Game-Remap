#ifndef AGRemapCore_RegionFilter_H
#define AGRemapCore_RegionFilter_H

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

#include <functional>
#include <memory>
#include <vector>

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Runs another filter on one region of a texture and leaves every other texel as it was: a channel set on the
     part of an atlas some triangles paint (:cpp:func:`DrawUVCoverage::region`), a colour grade on one UV island
     :raw-html:`<br />` :raw-html:`<br />`

     The region is asked for at the texture's size when the filter runs. An empty region, or one of the wrong size,
     leaves the texture as it is -- no region found is better than an edit of the whole texture. A change the inner
     filter makes to the texture's gamma applies to the whole texture
     @endrst
     */
    class RegionFilter: public BaseTexFilter {
        public:
            /**
             * @brief The region of a texture of 'width' x 'height': for each texel, row by row, whether it is edited
             */
            using Region = std::function<std::vector<bool>(long long width, long long height)>;

            /**
             * @brief The filter run on the region
             */
            std::shared_ptr<BaseTexFilter> filter;

            /**
             * @brief The region
             */
            Region region;

            /**
             * @brief Constructs the filter
             *
             * @param filter The filter run on the region
             * @param region The region
             */
            RegionFilter(std::shared_ptr<BaseTexFilter> filter = nullptr, Region region = nullptr);

            void transform(TextureFile &texFile) override;
    };
}

#endif
