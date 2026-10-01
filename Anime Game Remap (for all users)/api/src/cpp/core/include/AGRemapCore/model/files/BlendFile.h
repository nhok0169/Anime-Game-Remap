#ifndef AGRemapCore_BlendFile_H
#define AGRemapCore_BlendFile_H

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

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufValue.h"
#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/model/files/BufFile.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufFile` :raw-html:`<br />` :raw-html:`<br />`

     Used for handling ``Blend.buf`` files :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        We observe that a ``Blend.buf`` file is a binary file defined as:

        * a line corresponds to the data for a particular vertex in the mod
        * each line contains 32 bytes (256 bits)
        * each line uses little-endian mode (MSB is to the right while LSB is to the left)
        * the first 16 bytes of a line are for the blend weights, each weight is 4 bytes or 32 bits (4 weights/line)
        * the last 16 bytes of a line are for the corresponding indices for the blend weights, each index is 4 bytes or 32 bits (4 indices/line)
        * the blend weights are floating points while the blend indices are unsigned integers
     @endrst
     */
    class BlendFile: public BufFile {
        public:

            /**
             * @brief Constructs a new blend file and immediately reads it
             *
             * @param src The source file or bytes for the blend file
             * @param elements The sequence of elements within the ``.buf`` file. If this is empty,
             *      uses the elements specified for some GIMI character: the blend weights (as
             *      4 32-bit floating-point numbers) followed by the blend indices (as 4 32-bit
             *      signed integers)
             */
            explicit BlendFile(BinarySrc src, std::vector<std::unique_ptr<BufElementType>> elements = {});

            /**
             * @brief Retrieves the temporary remap for any missing blend indices not included in 'vgRemap'
             *
             * @param src The data for the blend weights and the blend indices for a particular vertex
             * @param vgRemap The vertex group remap for correcting the Blend.buf file
             *
             * @return The temporary remap for the missing indices. The keys are the missing indices
             *      found and the values are the temporary remapped values for these missing indices
             */
            static std::unordered_map<long long, long long> getMissingIndicesRemap(const BufLineData& src, const VGRemap& vgRemap);

            /**
             * @brief Remaps the vertex group indices for a particular line (vertex)
             *
             * @param src The data for the blend weights and the blend indices for a particular vertex
             * @param vgRemap The vertex group remap for correcting the Blend.buf file
             * @param remapMissingIndices Whether to deactivate any missing blend indices that cannot be identified
             *
             * @return The new data for the blend weights/blend indices, with the blend indices remapped
             */
            static BufLineData remapIndices(BufLineData src, const VGRemap& vgRemap, bool remapMissingIndices = true);

            /**
             * @brief Remaps the blend indices in a ``Blend.buf`` file
             *
             * @param vgRemap The vertex group remap for correcting the Blend.buf file
             * @param fixedBlendFile The file path for the fixed ``Blend.buf`` file. If this is
             *      ``std::nullopt``, the fixed bytes are returned directly instead of being written
             *      to a file
             * @param remapMissingIndices Whether to deactivate any missing blend indices that cannot be identified
             *
             * @return ``std::monostate`` if no correction was needed and #getSrc is a file path (nothing
             *      was written); a raw copy of #getData if no correction was needed and #getSrc is
             *      already raw bytes; otherwise the same result shape as #fix
             */
            std::variant<std::monostate, std::string, ByteVec> remap(const VGRemap& vgRemap, const std::optional<std::string>& fixedBlendFile = std::nullopt, bool remapMissingIndices = true);

            /**
             * @brief
             @rst
             The two element names every `blend`_ buffer uses, in both games :raw-html:`<br />`
             :raw-html:`<br />`

             Here rather than in each reader because three files had their own file-local copies of
             the pair -- this one, ``VGComponentSplit.cpp`` and ``WWMIFixer.cpp`` -- and a spelling
             repeated per file is a spelling that can be wrong in one of them. One such copy was
             silently emptied by a patch script and every Sanhua `blend`_ in a corpus was skipped
             from a clean build (2026-09-29)
             @endrst
             */
            static inline const std::string BlendWeightKey = "BLENDWEIGHT";
            static inline const std::string BlendIndicesKey = "BLENDINDICES";

        private:
            /**
             * @brief
             @rst
             The ordinary `blend`_ layout: 'influences' 32-bit float weights then 'influences' 32-bit
             signed indices :raw-html:`<br />` :raw-html:`<br />`

             A width other than four is not exotic -- a Wuthering Waves `blend`_ carries as many as
             eight, at one byte each -- and nothing in this class assumes the number: #remapIndices,
             #getMissingIndicesRemap and #remap all run to the SHORTER of the two elements the file
             actually declares. A caller whose types differ as well hands its own elements to the
             constructor, which is what the WuWa fixer does
             @endrst
             *
             * @param influences How many (weight, index) slots each vertex carries. **Default**: ``4``
             *
             * @return The elements
             */
            static std::vector<std::unique_ptr<BufElementType>> defaultElements(std::size_t influences = 4);
    };
}

#endif
