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

             Kept here so that every reader of a `blend`_ buffer shares one spelling of them
             @endrst
             */
            static inline const std::string BlendWeightKey = "BLENDWEIGHT";
            static inline const std::string BlendIndicesKey = "BLENDINDICES";

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

            /**
             * @brief
             @rst
             For each slot of a vertex after :cpp:func:`setInfluences`, the slot it came from -- or ``std::nullopt`` for
             a new, empty slot. One list per vertex
             @endrst
             */
            using KeptSlots = std::vector<std::vector<std::optional<std::size_t>>>;

            /**
             * @brief How many (weight, index) slots each vertex carries: the shorter of the two elements, or 0 when the
             * file has neither
             */
            std::size_t influences() const;

            /**
             * @brief
             @rst
             'elements' with the weight and index elements made 'influences' wide, each slot of the same data type as
             the element's first. Every other element is kept as it is, in its place
             @endrst
             *
             * @param elements The elements of a `blend`_ line
             * @param influences How many (weight, index) slots each vertex should carry
             *
             * @return The elements
             */
            static std::vector<std::unique_ptr<BufElementType>> elementsWithInfluences(
                const std::vector<std::unique_ptr<BufElementType>>& elements, std::size_t influences);

            /**
             * @brief
             @rst
             Makes every vertex carry 'influences' (weight, index) slots, in place :raw-html:`<br />` :raw-html:`<br />`

             * **Wider**: each vertex keeps its slots and gains new ones at the end: bone 0 at weight 0, which no
               shader weighs
             * **Narrower**: slots of one vertex naming the same bone are merged first, their weights summed onto the
               first of them; then the vertex keeps its 'influences' heaviest slots (a tie goes to the lower slot),
               their weights scaled so the vertex's total is what it was. Integer weights are scaled with rounding,
               the remainder put on the heaviest slot, and a sum is capped at the largest value the weight's type
               holds; float weights are scaled exactly
             * **The same width**: nothing changes

             Which bone a slot names is its index -- or, given 'bones', the value at the same slot there: a buffer of
             ids beside the blend that names the bones for it (a remapped skeleton's own ids, say). The slots kept are
             returned, so such a buffer can keep the same ones (:cpp:func:`keepSlots`)
             @endrst
             *
             * @param influences How many (weight, index) slots each vertex should carry
             * @param bones For each vertex, the bone each slot names, or ``nullptr`` to read it from the indices.
             *      **Default**: ``nullptr``
             *
             * @return For each vertex, the slot each new slot came from
             *
             * @throws std::runtime_error if the file has no weight or index element, or 'bones' does not have a list
             *      of the file's width for every vertex
             */
            KeptSlots setInfluences(std::size_t influences, const std::vector<std::vector<long long>>* bones = nullptr);

            /**
             * @brief
             @rst
             A buffer of ids beside a `blend`_ -- each vertex 'fromSlots' unsigned little-endian ids of 'idBytes'
             bytes -- rewritten to the slots :cpp:func:`setInfluences` kept. A new slot is id 0
             @endrst
             *
             * @param ids The buffer
             * @param idBytes The bytes of one id
             * @param fromSlots How many ids each vertex has in 'ids'
             * @param kept What :cpp:func:`setInfluences` returned
             *
             * @return The rewritten buffer
             *
             * @throws std::runtime_error if 'ids' does not hold one line for each vertex of 'kept'
             */
            static ByteVec keepSlots(const ByteVec& ids, std::size_t idBytes, std::size_t fromSlots, const KeptSlots& kept);
    };
}

#endif
