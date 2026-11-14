#ifndef AGRemapCore_GuidedScaleFilter_H
#define AGRemapCore_GuidedScaleFilter_H

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

#include <array>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/ChannelMapFilter.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Scales the colour of every pixel by a factor another texture -- the GUIDE -- chooses: the guide's channel at
     the same UV (nearest texel, the two textures may differ in size) picks one of 256 RGB factors. What a texel IS
     comes from the guide, never from its own colour: a material-code map says which material a diffuse texel
     paints, a normal map's alpha which shading band it sits in :raw-html:`<br />` :raw-html:`<br />`

     With #linear the factor applies to the colour decoded from sRGB (2.2) when the texture is stored sRGB (one
     :cpp:func:`TextureFile::getGamma` is set for), and the result is encoded back. Results are rounded and clamped
     to 0-255. Alpha is untouched. A guide that is not there (#guide empty, or a file that does not open) leaves the
     texture as it is
     @endrst
     */
    class GuidedScaleFilter: public BaseTexFilter {
        public:
            /**
             * @brief The guide texture's file. **Default**: empty
             */
            std::string guide;

            /**
             * @brief The guide's channel that chooses the factor: 0 (R) to 3 (A). **Default**: 0
             */
            int channel = 0;

            /**
             * @brief How the guide's channel is read -- see :cpp:enum:`ChannelMapFilter::Read`. **Default**: :cpp:enumerator:`ChannelMapFilter::Read::Raw`
             */
            ChannelMapFilter::Read read = ChannelMapFilter::Read::Raw;

            /**
             * @brief The RGB factor for each value of the guide's channel: 256 entries, or empty to leave every texel. **Default**: empty
             */
            std::vector<std::array<double, 3>> factors;

            /**
             * @brief Whether to scale the colour decoded from sRGB, for a texture stored sRGB. **Default**: ``false``
             */
            bool linear = false;

            void transform(TextureFile &texFile) override;
    };
}

#endif
