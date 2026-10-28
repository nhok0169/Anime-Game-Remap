#ifndef AGRemapCore_WWMIShapeKeys_H
#define AGRemapCore_WWMIShapeKeys_H

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
#include <string>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The three shape-key buffers of a Wuthering Waves (WWMI) mod, as WWMI Tools writes them
     :raw-html:`<br />` :raw-html:`<br />`

     * ``ShapeKeyOffset`` holds the FIRST entry of each key slot, one ``uint32`` a slot
     * ``ShapeKeyVertexId`` holds one ``uint32`` vertex id per entry
     * ``ShapeKeyVertexOffset`` holds one vertex offset of a fixed width per entry

     A slot's entries run to the next slot's first, the last slot's to the end :raw-html:`<br />`
     :raw-html:`<br />`

     The game drives a character's shape keys -- blinks, mouth shapes -- by SLOT, and two skins of
     one character can number the same keys differently, so a mod remapped between them has its
     keys put in the target's order by :cpp:func:`reorder`
     @endrst
     */
    class WWMIShapeKeys {
        public:
            /**
             * @brief The three buffers of one set of shape keys
             */
            struct Files {
                /**
                 * @brief ``ShapeKeyOffset``: each slot's first entry
                 */
                std::string offsets;

                /**
                 * @brief ``ShapeKeyVertexId``: each entry's vertex
                 */
                std::string vertexIds;

                /**
                 * @brief ``ShapeKeyVertexOffset``: each entry's offset
                 */
                std::string vertexOffsets;
            };

            /**
             * @brief What a set of shape-key buffers holds
             */
            struct Layout {
                /**
                 * @brief Each slot's first entry. Empty when the buffers are not this layout
                 */
                std::vector<std::uint32_t> firsts;

                /**
                 * @brief How many entries there are
                 */
                std::size_t entries = 0;

                /**
                 * @brief The bytes of one vertex offset. Zero when the buffers are not this layout
                 */
                std::size_t offsetWidth = 0;
            };

            /**
             * @brief Reads the layout off the three buffers' bytes and sizes
             *
             * @param files The buffers
             *
             * @return The layout, with no slots when the sizes do not divide as this layout's do
             *
             * @throws std::runtime_error if the offsets cannot be read
             */
            static Layout layoutOf(const Files& files);

            /**
             * @brief Why the buffers cannot be put in 'order' -- empty when they can
             *
             * @param files The buffers
             * @param order The order \ref reorder would be asked for
             *
             * @return The reason, or ``""``
             */
            static std::string checkReorder(const Files& files, const std::vector<long long>& order);

            /**
             * @brief
             @rst
             Writes the buffers again with their slots in another order :raw-html:`<br />`
             :raw-html:`<br />`

             ``order[j]`` is the key whose entries go in slot ``j``, ``-1`` for a slot that takes
             none. Slots past the end of 'order' keep their entries and position. A key of the first
             ``order.size()`` that no slot takes is kept, its offsets zeroed, after the last ordered
             slot, so the number of entries does not change
             @endrst
             *
             * @param files The buffers to read
             * @param order The source key for each slot
             * @param out Where the reordered buffers are written
             *
             * @throws std::runtime_error if a buffer cannot be read or written, or the reorder would
             *      change the number of entries
             */
            static void reorder(const Files& files, const std::vector<long long>& order, const Files& out);
    };
}

#endif
