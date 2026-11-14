#ifndef AGRemapCore_WWMIBlend_H
#define AGRemapCore_WWMIBlend_H

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
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufValue.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Remaps the vertex groups of a Wuthering Waves (WWMI) mod's `blend`_ buffer :raw-html:`<br />`
     :raw-html:`<br />`

     A WWMI ``Blend.buf`` line is N one-byte bone ids followed by N one-byte weights, where N --
     the influences per vertex -- is a property of the character (four for Sanhua, eight for Chisa)
     and is read off the buffer's size against its ``Position.buf`` (:cpp:func:`layoutOf`) rather
     than assumed. Three shapes of mod need more than :cpp:func:`BlendFile::remap`:

     * a character past 256 merged bones keeps each vertex's true 16-bit ids in a
       ``BlendRemapVertexVG.buf`` beside the blend, whose 8-bit ids are then only an index into
       WWMI's blend remap -- :cpp:func:`remapThroughVertexVG`
     * a mod from before WWMI's merged skeleton writes each component's OWN bone ids, which have to
       be lifted into the merged skeleton through that component's ``vg_map`` before the row applies
       -- :cpp:func:`liftLegacy`
     * a remap ONTO a character past 256 bones has to write WWMI's own blend remap -- the remapped
       blend plus its ``VertexVG``, ``Forward`` and ``Reverse`` buffers -- exactly as WWMI Tools
       writes one for that character's own mods -- :cpp:func:`writeRemap`

     A target whose draws read a different number of influences than the mod carries is met by
     :cpp:func:`BlendFile::setInfluences` :raw-html:`<br />` :raw-html:`<br />`

     Every function throws ``std::runtime_error`` with the reason when a buffer cannot be read or
     written in the shape asked, so a caller can say which file failed and why
     @endrst
     */
    class WWMIBlend {
        public:
            /**
             * @brief What a mod's ``Position.buf`` and ``Blend.buf`` sizes say its blend layout is
             */
            struct Layout {
                /**
                 * @brief How many vertices the mesh has
                 */
                std::size_t vertices = 0;

                /**
                 * @brief How many (id, weight) slots each vertex carries
                 */
                std::size_t influences = 0;
            };

            /**
             * @brief
             @rst
             A mod from before WWMI's merged skeleton, as :cpp:func:`liftLegacy` and
             :cpp:func:`writeRemap` read one
             @endrst
             */
            struct LegacyLift {
                /**
                 * @brief The mod's index buffer
                 */
                std::string indexPath;

                /**
                 * @brief
                 @rst
                 source component -> the (index count, first index) of each of its draws, which is
                 what says which component owns each vertex
                 @endrst
                 */
                std::map<int, std::vector<std::pair<long long, long long>>> drawRanges;

                /**
                 * @brief source component -> its ``vg_map``: component-local bone id -> merged bone id
                 */
                std::map<int, std::vector<int>> vgMaps;
            };

            /**
             * @brief The three buffers of a WWMI blend remap, besides the remapped blend itself
             */
            struct RemapFiles {
                /**
                 * @brief Every vertex's remapped ids at their full 16 bits
                 */
                std::string vertexVG;

                /**
                 * @brief :cpp:member:`RemapSize` uint16, local id -> merged bone
                 */
                std::string forward;

                /**
                 * @brief :cpp:member:`RemapSize` uint16, merged bone -> local id
                 */
                std::string reverse;
            };

            /**
             * @brief The bytes per vertex of a WWMI ``Position.buf``: three 32-bit floats
             */
            static constexpr std::uintmax_t PositionStride = 12;

            /**
             * @brief The highest bone a one-byte blend id can name
             */
            static constexpr long long MaxByteBone = 255;

            /**
             * @brief The entries in one WWMI blend remap
             */
            static constexpr std::size_t RemapSize = 512;

            /**
             * @brief
             @rst
             The elements of a WWMI `blend`_ line: 'influences' one-byte ids under
             :cpp:member:`BlendFile::BlendIndicesKey`, then 'influences' one-byte weights under
             :cpp:member:`BlendFile::BlendWeightKey`
             @endrst
             *
             * @param influences How many (id, weight) slots each vertex carries
             *
             * @return The elements
             */
            static std::vector<std::unique_ptr<BufElementType>> blendElements(std::size_t influences);

            /**
             * @brief
             @rst
             The elements of a ``BlendRemapVertexVG.buf`` line: 'influences' 16-bit ids under
             :cpp:member:`BlendFile::BlendIndicesKey`, and no weights
             @endrst
             *
             * @param influences How many ids each vertex carries
             *
             * @return The elements
             */
            static std::vector<std::unique_ptr<BufElementType>> vertexVGElements(std::size_t influences);

            /**
             * @brief
             @rst
             The vertex count and influences per vertex the two buffers' SIZES imply
             @endrst
             *
             * @param blendPath The mod's ``Blend.buf``
             * @param positionPath Its ``Position.buf``, which gives the vertex count
             *
             * @return The layout, or ``std::nullopt`` when the sizes imply none: no vertices, a blend
             *      that does not divide by them, or an odd number of bytes per vertex
             */
            static std::optional<Layout> layoutOf(const std::string& blendPath, const std::string& positionPath);

            /**
             * @brief
             @rst
             How many influences a vertex of the mod's blend carries -- :cpp:func:`layoutOf`'s
             answer, else the blend's size over 'vertices', else 'fallback'
             @endrst
             *
             * @param blendPath The mod's ``Blend.buf``
             * @param vertices The vertex count the mod declares, used only when the ``Position.buf``
             *      does not answer. Zero or less for none
             * @param positionPath Its ``Position.buf``
             * @param fallback The answer when nothing else gives one. **Default**: ``4``
             *
             * @return The influences per vertex
             */
            static std::size_t influencesOf(const std::string& blendPath, long long vertices,
                                            const std::string& positionPath, std::size_t fallback = 4);

            /**
             * @brief
             @rst
             Which source component draws each vertex, off the mod's own draw ranges over its index
             buffer -- ``-1`` for a vertex no range draws. Where two ranges reach one vertex, the
             later component in 'drawRanges' wins
             @endrst
             *
             * @param indexPath The mod's index buffer
             * @param drawRanges source component -> the (index count, first index) of each of its draws
             * @param vertices How many vertices the mesh has
             *
             * @return One component per vertex
             */
            static std::vector<int> componentOfVertex(const std::string& indexPath,
                                                      const std::map<int, std::vector<std::pair<long long, long long>>>& drawRanges,
                                                      std::size_t vertices);

            /**
             * @brief
             @rst
             Remaps the blend of a mod whose skeleton passes 256 bones: each weighted slot's id is
             read from its ``BlendRemapVertexVG.buf`` (the one-byte id in the blend is only an index
             into the mod's blend remap) and sent through 'row'. The weights are kept
             @endrst
             *
             * @param blendPath The mod's ``Blend.buf``
             * @param fixedPath Where the remapped blend is written
             * @param vertexVGPath The mod's ``BlendRemapVertexVG.buf``
             * @param positionPath Its ``Position.buf``
             * @param row The vertex group remap
             */
            static void remapThroughVertexVG(const std::string& blendPath, const std::string& fixedPath,
                                             const std::string& vertexVGPath, const std::string& positionPath,
                                             const VGRemap& row);

            /**
             * @brief
             @rst
             Remaps the blend of a mod from before WWMI's merged skeleton: each weighted slot's id is
             local to the component that draws the vertex (:cpp:func:`componentOfVertex`), so it is
             lifted through that component's ``vg_map`` and only then sent through 'row'. A vertex no
             component with a ``vg_map`` draws is left alone
             @endrst
             *
             * @param blendPath The mod's ``Blend.buf``
             * @param fixedPath Where the remapped blend is written
             * @param positionPath Its ``Position.buf``
             * @param legacy The mod's index buffer, draw ranges and ``vg_map``\\s
             * @param row The vertex group remap
             */
            static void liftLegacy(const std::string& blendPath, const std::string& fixedPath,
                                   const std::string& positionPath, const LegacyLift& legacy, const VGRemap& row);

            /**
             * @brief
             @rst
             Remaps a blend ONTO a character whose merged skeleton passes 256 bones, writing WWMI's
             blend remap for it :raw-html:`<br />` :raw-html:`<br />`

             One remap serves the whole mesh, over the distinct targets 'row' names (an entry no
             vertex reaches costs two bytes), so a caller can declare the remap's bone count from the
             row alone before this runs. The blend gets the LOCAL id each mapped bone has in that
             remap, which is what WWMI's BlendRemapper writes there at run time, and the weights are
             kept :raw-html:`<br />` :raw-html:`<br />`

             The true ids are the mod's own ``BlendRemapVertexVG.buf`` when it has one, else the
             blend's, lifted as :cpp:func:`liftLegacy` lifts them when 'legacy' is given
             @endrst
             *
             * @param blendPath The mod's ``Blend.buf``
             * @param fixedPath Where the remapped blend is written
             * @param srcVertexVGPath The mod's own ``BlendRemapVertexVG.buf``, or ``""`` for none
             * @param positionPath Its ``Position.buf``
             * @param row The vertex group remap
             * @param out Where the remap's three buffers are written
             * @param legacy The mod's legacy lift, for a mod from before WWMI's merged skeleton
             *
             * @throws std::runtime_error also when 'row' maps to a bone a remap of
             *      :cpp:member:`RemapSize` entries cannot name
             */
            static void writeRemap(const std::string& blendPath, const std::string& fixedPath,
                                   const std::string& srcVertexVGPath, const std::string& positionPath,
                                   const VGRemap& row, const RemapFiles& out,
                                   const std::optional<LegacyLift>& legacy = std::nullopt);

            /**
             * @brief
             @rst
             The reverse map of a blend remap over a row -- merged target bone -> local id -- as
             :cpp:func:`writeRemap` writes it: every distinct target the row names that a remap of
             :cpp:member:`RemapSize` entries can hold, numbered in ascending order. Every other entry is ``0``
             @endrst
             *
             * @param row The vertex group remap
             *
             * @return :cpp:member:`RemapSize` local ids
             */
            static std::vector<std::uint16_t> reverseOf(const VGRemap& row);

            /**
             * @brief
             @rst
             The blend of ONE source component drawn from mesh files of its own -- a ``Blend.buf`` whose ids
             start at the component's ``vg_offset`` -- remapped into the TARGET's local ids, as WWMI's
             BlendRemapper would write them for the main mesh under :cpp:func:`writeRemap`'s remap: each id
             lifted out of the component's window (``id - vgOffset``) into the source's merged skeleton through
             'vgMap', then through 'row', then through :cpp:func:`reverseOf` 'row'. So the bytes need no
             compute pass to be right :raw-html:`<br />` :raw-html:`<br />`

             Weights are kept. A slot of weight ``0``, and an id the vg map or the row does not place, reads
             local ``0``. The output has ``max(influences, outInfluences)`` slots a vertex -- the extra ones
             empty -- so a narrower source is widened to a target that reads more
             @endrst
             *
             * @param blendPath The component's ``Blend.buf``
             * @param positionPath Its ``Position.buf``, which gives the vertex count (:cpp:func:`layoutOf`)
             * @param vgOffset The first merged id the component's window covers
             * @param vgMap The component's ``vg_map``: component-local bone id -> merged source bone id
             * @param row The vertex group remap
             * @param outInfluences The influences a vertex of the target reads
             * @param unmapped Filled with the component-local ids that could not be placed, when given
             *
             * @throws std::runtime_error When the two buffers' sizes imply no layout
             *
             * @return The remapped blend
             */
            static ByteVec componentBlend(const std::string& blendPath, const std::string& positionPath, long long vgOffset,
                                          const std::vector<int>& vgMap, const VGRemap& row, std::size_t outInfluences,
                                          std::set<long long>* unmapped = nullptr);
    };
}

#endif
