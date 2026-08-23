#ifndef AGRemapCore_BlendFile_H
#define AGRemapCore_BlendFile_H

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

        private:
            static std::vector<std::unique_ptr<BufElementType>> defaultElements();
    };
}

#endif
