#ifndef AGRemapCore_TexcoordFile_H
#define AGRemapCore_TexcoordFile_H

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

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/model/files/BufFile.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufFile` :raw-html:`<br />` :raw-html:`<br />`

     Used for handling ``Texcoord.buf`` files: one element of UV values a vertex (:cpp:member:`uvElement`), whose
     first two values are the vertex's U and V, beside any others the layout declares :raw-html:`<br />`
     :raw-html:`<br />`

     A mod's UVs can carry two things a draw does not expect: a value that is not a number, and a U past ``1`` that
     relies on the sampler WRAPPING -- a part UV'd into the ``[1, 2)`` tile. #clean sets the first to ``0`` and folds
     the second back into ``[0, 1)``, which selects the same texel wherever the pass wraps and keeps the part right
     where it does not
     @endrst
     */
    class TexcoordFile: public BufFile {
        public:
            /**
             * @brief The name #halfElements gives the UV element
             */
            static inline const std::string UVKey = "TEXCOORD";

            /**
             * @brief What #clean changed
             */
            struct CleanStats {
                /**
                 * @brief How many values that were not a number it set to ``0``
                 */
                std::size_t cleared = 0;

                /**
                 * @brief How many U values it folded into ``[0, 1)``
                 */
                std::size_t folded = 0;
            };

            /**
             * @brief Constructs a new texcoord file and immediately reads it
             *
             * @param src The source file or bytes for the ``.buf`` file
             * @param elements The sequence of elements within the ``.buf`` file. If this is empty, uses #halfElements of a 4-byte stride
             * @param uvElement The element holding the UVs, U first. **Default**: :cpp:member:`UVKey`
             */
            explicit TexcoordFile(BinarySrc src, std::vector<std::unique_ptr<BufElementType>> elements = {},
                                  std::string uvElement = UVKey);

            /**
             * @brief
             @rst
             One element, :cpp:member:`UVKey`, of 'stride' / 2 16-bit halves, encoded to the nearest half (ties to
             even) -- the rounding a folded U needs
             @endrst
             *
             * @param stride The bytes of one vertex
             *
             * @return The elements
             */
            static std::vector<std::unique_ptr<BufElementType>> halfElements(std::size_t stride);

            /**
             * @brief The element holding the UVs
             */
            const std::string& uvElement() const;

            /**
             * @brief Each vertex's U, with a value that is not a number read as the ``0`` #clean makes it
             */
            std::vector<float> us() const;

            /**
             * @brief
             @rst
             For each vertex, whether it is a corner of a triangle whose corners sit in DIFFERENT U tiles. Folding one
             corner of such a triangle would widen its U span from a few hundredths to nearly ``1`` and interpolate it
             backwards across the atlas, so #clean leaves those vertices alone -- which also protects deliberate tiling
             @endrst
             *
             * @param indices The mesh's index list, three to a triangle -- see :cpp:func:`IbFile::getIndices`. An index
             *      past the last vertex skips its triangle
             */
            std::vector<bool> straddling(const std::vector<long long>& indices) const;

            /**
             * @brief
             @rst
             In place: sets every value of the UV element that is not a number to ``0``, and folds each U at or above
             ``1`` into ``[0, 1)`` (its fractional part), except at a vertex 'keep' marks. A U below ``0`` is left alone:
             that is the edge bleed an authoring tool leaves around a UV island, which a clamping sampler is meant to
             extend, and folded it would sample the far side of the atlas
             @endrst
             *
             * @param keep For each vertex, whether to leave its U alone -- eg. #straddling. A vertex past the list's end
             *      is folded. **Default**: none is kept
             *
             * @return What it changed
             */
            CleanStats clean(const std::vector<bool>& keep = {});

        private:
            std::string uvElement_;
    };
}

#endif
