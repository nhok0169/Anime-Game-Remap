#ifndef AGRemapCore_VGRemap_H
#define AGRemapCore_VGRemap_H

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
#include <optional>
#include <unordered_map>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class for handling the vertex group remaps for mods
     @endrst
     */
    class VGRemap {
        public:

            /**
             * @brief Constructs a new vertex group remap
             *
             * @param remap The vertex group remap from one type of mod to another
             */
            explicit VGRemap(std::unordered_map<long long, long long> remap = {});

            /**
             * @brief The vertex group remap
             */
            const std::unordered_map<long long, long long>& getRemap() const;

            /**
             * @brief Sets the vertex group remap (recomputes #getMaxIndex)
             *
             * @param remap The new remap
             */
            void setRemap(std::unordered_map<long long, long long> remap);

            /**
             * @brief The maximum index in the vertex group remap, or ``std::nullopt`` if #getRemap is empty
             */
            std::optional<long long> getMaxIndex() const;

            /**
             * @brief
             @rst
             The row renumbered into a table of 'size' entries: every distinct target bone below 'size' it names gets a
             LOCAL id, in ascending order of the bone -- so a buffer can hold local ids no wider than the table, with the
             table to turn them back
             @endrst
             */
            struct Compact {
                /**
                 * @brief The row in local ids: source bone -> the local id of its target (a target past the table is left out)
                 */
                std::unordered_map<long long, long long> localRow;

                /**
                 * @brief 'size' entries: local id -> target bone; ``0`` past the last
                 */
                std::vector<std::uint16_t> forward;

                /**
                 * @brief 'size' entries: target bone -> local id; ``0`` for a bone the row does not name
                 */
                std::vector<std::uint16_t> reverse;
            };

            /**
             * @brief
             @rst
             This row renumbered into a table of 'size' entries (:cpp:struct:`Compact`)
             @endrst
             *
             * @param size The entries of the table
             *
             * @return The local row and its two tables
             */
            Compact compact(std::size_t size) const;

        private:
            std::unordered_map<long long, long long> remap_;
            std::optional<long long> maxIndex_;
    };
}

#endif
