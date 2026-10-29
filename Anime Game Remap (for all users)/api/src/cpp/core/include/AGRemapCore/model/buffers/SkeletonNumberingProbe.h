#ifndef AGRemapCore_SkeletonNumberingProbe_H
#define AGRemapCore_SkeletonNumberingProbe_H

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
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace AGRemapCore {

    /**
     * @brief
     @rst
     Tells which of several numberings a mod's blend gives its bone ids in, from the mod's own geometry
     :raw-html:`<br />` :raw-html:`<br />`

     A character whose skeleton was renumbered between game versions has mods whose blends name bones
     by an older numbering, while the community hash tools rewrite a mod's hashes and keep its bone
     ids -- so the hashes cannot say which numbering a blend is in. The geometry can: for every vertex
     whose heaviest bone (over half its weight) is an id the numberings disagree on, each numbering is
     scored by the vertex's distance to that bone's rest-pose centroid on the character's own model, and
     the numbering with the smallest median distance wins :raw-html:`<br />` :raw-html:`<br />`

     The buffers are a Wuthering Waves (WWMI) mod's: ``Position.buf`` of three 32-bit floats a vertex,
     ``Blend.buf`` of the bone ids then the weights of each vertex, 8 bits each, and optionally the
     16-bit ids of a mod past 256 bones (``BlendRemapVertexVG.buf``), read in place of the blend's own
     @endrst
     */
    class SkeletonNumberingProbe {
        public:
            /**
             * @brief One way a character's bone ids have been numbered
             */
            struct Numbering {
                /**
                 * @brief The version whose bone ids follow this numbering
                 */
                std::string version;

                /**
                 * @brief Id in this numbering -> the same bone's id in the numbering the centroids are keyed by. An id not listed is the same in both
                 */
                std::unordered_map<long long, long long> toReference;
            };

            /**
             * @brief What #probe found
             */
            struct Result {
                /**
                 * @brief The index of the numbering with the smallest median distance
                 */
                std::size_t best = 0;

                /**
                 * @brief Each numbering's median distance, in the order the numberings were given
                 */
                std::vector<double> medians;

                /**
                 * @brief How many vertices sit on a bone the numberings disagree on -- what the medians are taken over
                 */
                std::size_t vertices = 0;
            };

            /**
             * @brief The fewest vertices on disputed bones #probe gives an answer from
             */
            static constexpr std::size_t MinVertices = 20;

            /**
             * @brief Which numbering the mod's blend is in
             *
             * @param positionFile The mod's ``Position.buf``
             * @param blendFile The mod's ``Blend.buf``
             * @param vertexVGFile The mod's 16-bit bone ids, read in place of the blend's own. **Default**: none
             * @param numberings The candidate numberings -- at least two
             * @param centroids Each bone's rest-pose centroid on the character's own model, keyed by its reference id
             *
             * @return The answer, or ``std::nullopt`` when fewer than two numberings are given, a buffer cannot be
             *      read or its size does not fit the others, or fewer than #MinVertices vertices sit on disputed bones
             */
            static std::optional<Result> probe(const std::string& positionFile, const std::string& blendFile,
                                               const std::optional<std::string>& vertexVGFile,
                                               const std::vector<Numbering>& numberings,
                                               const std::unordered_map<long long, std::array<double, 3>>& centroids);
    };
}

#endif
