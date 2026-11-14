#ifndef AGRemapCore_ChannelMapFilter_H
#define AGRemapCore_ChannelMapFilter_H

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
#include <cstdint>
#include <vector>

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Writes each channel of every pixel from a channel of the same pixel through a 256-entry table: a
     material-code map swapped between two characters' codes, a channel set to a constant, a channel copied from
     another, alpha clamped, a colour gain. Every channel is read before any is written :raw-html:`<br />`
     :raw-html:`<br />`

     How a channel is read is #read: the stored bytes, or the values a shader reads
     (:cpp:func:`TextureFile::shaderChannel`) -- a code map stored sRGB reaches its shader decoded. With #data the
     written bytes are data rather than colour, and the texture's gamma is cleared so they are saved as they are
     @endrst
     */
    class ChannelMapFilter: public BaseTexFilter {
        public:
            /**
             * @brief How a source channel is read
             */
            enum class Read {
                /**
                 * @brief The stored bytes
                 */
                Raw,

                /**
                 * @brief As a shader reads a code: decoded, truncated (:cpp:func:`TextureFile::shaderChannel`)
                 */
                ShaderCode,

                /**
                 * @brief As a shader reads a value: decoded, rounded
                 */
                ShaderValue
            };

            /**
             * @brief What one output channel is written from
             */
            struct Channel {
                /**
                 * @brief The channel read (0 to 3), or -1 to leave this channel as it is. **Default**: -1
                 */
                int source = -1;

                /**
                 * @brief
                 @rst
                 The value written for each value read: 256 entries, or empty to write the value read.
                 **Default**: empty
                 @endrst
                 */
                std::vector<std::uint8_t> table;
            };

            /**
             * @brief The output channels R, G, B and A, in that order
             */
            std::array<Channel, 4> channels;

            /**
             * @brief How every source channel is read. **Default**: :cpp:enumerator:`Read::Raw`
             */
            Read read = Read::Raw;

            /**
             * @brief Whether the written bytes are data (codes, flags) rather than colour: the gamma is cleared. **Default**: ``false``
             */
            bool data = false;

            /**
             * @brief A table that writes 'value' for every value read
             *
             * @param value The value
             *
             * @return The table
             */
            static std::vector<std::uint8_t> constant(std::uint8_t value);

            /**
             * @brief
             @rst
             A filter for a material-code map: each texel's code -- the red channel as a shader reads it
             (:cpp:enumerator:`Read::ShaderCode`) -- through 'table', written to R, G and B as data. A single-channel
             map comes in as ``(v, v, v, 255)`` and is written the same way
             @endrst
             *
             * @param table The code written for each code read: 256 entries
             *
             * @return The filter
             */
            static ChannelMapFilter codeMap(std::vector<std::uint8_t> table);

            void transform(TextureFile &texFile) override;
    };
}

#endif
