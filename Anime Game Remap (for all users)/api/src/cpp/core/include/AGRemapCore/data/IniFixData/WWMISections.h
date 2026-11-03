#ifndef AGRemapCore_WWMISections_H
#define AGRemapCore_WWMISections_H

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
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/tools/z3/Z3Context.h"

namespace AGRemapCore {
    /**
     * @brief
     @rst
     The sections a fix of a Wuthering Waves mod writes to drive WWMI, the mod loader in the game: the merged
     skeleton a mod from before WWMI's merged skeleton has none of, a remapped copy of it for a target past 256 bones,
     one merge list per target draw slot, the blend remap that runs WWMI's ``BlendRemapper``, and the section of a
     target draw slot nothing of the mod is drawn through :raw-html:`<br />` :raw-html:`<br />`

     Each builder fills a fixed template with the values it is given. Every name goes through
     :cpp:member:`Skeleton::name` / :cpp:member:`BlendRemap::name`, so the sections carry the fix's own names and an
     undo removes them
     @endrst
     */
    class WWMISections {
        public:
            /**
             * @brief One section
             */
            using Section = IfTemplate<std::string, std::string>;

            /**
             * @brief What skeletonSections writes
             */
            struct Skeleton {
                /**
                 * @brief The fix's name for a section of the given name
                 */
                std::function<std::string(const std::string&)> name;

                /**
                 * @brief The hash of the game's bone-data constant buffer, which the merges are gated on
                 */
                std::string cb4Hash;

                /**
                 * @brief The `filter_index` the fix marks that buffer with
                 */
                std::string boneDataFilter;

                /**
                 * @brief How many bones the merged skeleton holds
                 */
                int mergedSkeletonSlots = 0;

                /**
                 * @brief Whether the target is past 256 bones: the merge then feeds a private REMAPPED skeleton
                 * (blendRemapForward, WWMI's `SkeletonRemapper`), bound in place of the merged one
                 */
                bool remapped = false;

                /**
                 * @brief Whether the remap runs once a frame from a latch elsewhere; otherwise each merge list runs it first
                 */
                bool latched = false;

                /**
                 * @brief Whether a draw with its current pose in `vs-cb3` alone merges into the MAIN skeleton
                 */
                bool currentPoseInCb3Only = false;

                /**
                 * @brief Whether `vs-cb3` gets the previous pose after every merge, whatever the marker says
                 */
                bool bindPrevPoseAlways = false;

                /**
                 * @brief The blend remap's bone count, for the remap
                 */
                std::size_t blendRemapBones = 0;

                /**
                 * @brief The resource of the blend remap's forward ids, for the remap
                 */
                std::string blendRemapForward;

                /**
                 * @brief Each target draw slot's bone window, as `(vg_offset, vg_count)`
                 */
                std::vector<std::pair<std::string, std::string>> slotWindows;
            };

            /**
             * @brief The merge list of one target draw slot
             * @param skeleton What the sections are named by
             * @param slot The slot
             */
            static std::string mergeListName(const Skeleton& skeleton, int slot);

            /**
             * @brief The skeleton's sections: the merged skeleton and its read-write copies, the marker on the game's
             * bone-data constant buffer, the remapped skeleton with the list that remaps it and the merge a hidden slot
             * runs (for Skeleton::remapped), and one merge list per slot (mergeListName)
             * @param z3 The Z3 context the `if` lines are parsed in
             * @param skeleton What to write
             * @return The sections, in order
             */
            static std::vector<std::unique_ptr<Section>> skeletonSections(Z3Context& z3, const Skeleton& skeleton);

            /**
             * @brief What blendRemapSections writes
             */
            struct BlendRemap {
                /**
                 * @brief The fix's name for a section of the given name
                 */
                std::function<std::string(const std::string&)> name;

                /**
                 * @brief The remapped blend file
                 */
                std::string blendFile;

                /**
                 * @brief The files of the remap's three id buffers: `VertexVG`, `Forward` and `Reverse`
                 */
                std::function<std::string(const std::string& which)> remapFile;

                /**
                 * @brief The blend's stride
                 */
                std::size_t blendStride = 0;

                /**
                 * @brief How many influences each vertex of the blend carries
                 */
                std::size_t influences = 0;

                /**
                 * @brief The register the remapped blend is bound at
                 */
                std::string blendReg;
            };

            /**
             * @brief The list blendRemapSections runs WWMI's `BlendRemapper` from
             * @param blendRemap What the sections are named by
             */
            static std::string blendRemapInitList(const BlendRemap& blendRemap);

            /**
             * @brief The blend remap's resources and the one-time run of WWMI's `BlendRemapper` (#blendRemapInitList), guarded
             * by a `local` so it costs one dispatch on the first drawn frame and binds the remapped blend after
             * @param z3 The Z3 context the `if` lines are parsed in
             * @param blendRemap What to write
             * @return The sections, in order
             */
            static std::vector<std::unique_ptr<Section>> blendRemapSections(Z3Context& z3, const BlendRemap& blendRemap);

            /**
             * @brief What hideSection writes
             */
            struct Hide {
                /**
                 * @brief The section's name
                 */
                std::string name;

                /**
                 * @brief The comment written above it
                 */
                std::string comment;

                /**
                 * @brief The target's `vb0` hash
                 */
                std::string vb0Hash;

                /**
                 * @brief The slot's `match_first_index` and `match_index_count`
                 */
                std::string indexOffset, indexCount;

                /**
                 * @brief The slot's bone window, `vg_offset` and `vg_count`
                 */
                std::string vgOffset, vgCount;

                /**
                 * @brief A name of the slot for its frame latch (`$state_id_<slot>`)
                 */
                std::string slot;

                /**
                 * @brief For a mod from before WWMI's merged skeleton: the list that merges the slot's bones
                 * (mergeListName), run before the draw is skipped -- or empty for a mod with a merged skeleton
                 */
                std::string legacyMergeList;

                /**
                 * @brief For a target past 256 bones: the list that merges the slot's window into the fix's own skeleton, run on
                 * every draw -- or empty, which runs `CommandListMergeSkeleton` once a frame through the slot's latch
                 */
                std::string mergeWindowList;

                /**
                 * @brief The list run once a frame without #mergeWindowList
                 */
                std::string mergeSkeletonList;

                /**
                 * @brief Lines that capture the game's own bindings on the slot, run while a pixel shader is bound
                 */
                std::vector<std::pair<std::string, std::string>> stashed;
            };

            /**
             * @brief The `TextureOverride` of a target draw slot nothing of the mod is drawn through: it skips the skin's own
             * geometry and still merges the slot's bones, which other slots' vertices may be weighted to
             * @param z3 The Z3 context the `if` lines are parsed in
             * @param hide What to write
             * @return The section
             */
            static std::unique_ptr<Section> hideSection(Z3Context& z3, const Hide& hide);
    };
}

#endif
