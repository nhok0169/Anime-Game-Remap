#ifndef AGRemapCore_WWMIRuntime_H
#define AGRemapCore_WWMIRuntime_H

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
     The `sections`_ a fix of a Wuthering Waves mod writes to drive WWMI itself: the merged skeleton a mod from
     before WWMI's merged skeleton has none of, a remapped copy of it for a target past 256 bones, one merge list per
     target draw slot, and the blend remap that rewrites the remapped blend's ids into that remap's local space
     :raw-html:`<br />` :raw-html:`<br />`

     Every name goes through :cpp:member:`Skeleton::name` / :cpp:member:`BlendRemap::name`, so the sections carry the
     fix's own names and an undo takes them
     @endrst
     */
    class WWMIRuntime {
        public:
            /**
             * @brief One `section`_
             */
            using Section = IfTemplate<std::string, std::string>;

            /**
             * @brief What #skeletonSections writes
             */
            struct Skeleton {
                /**
                 * @brief The fix's name for a `section`_ of the given name
                 */
                std::function<std::string(const std::string&)> name;

                /**
                 * @brief The hash of the game's bone-data constant buffer, which the merges are gated on
                 */
                std::string cb4Hash;

                /**
                 * @brief The ``filter_index`` the fix marks that buffer with
                 */
                std::string boneDataFilter;

                /**
                 * @brief How many bones the merged skeleton holds
                 */
                int mergedSkeletonSlots = 0;

                /**
                 * @brief
                 @rst
                 Whether the target is past 256 bones: the merge then feeds a private REMAPPED skeleton
                 (:cpp:member:`blendRemapForward`, WWMI's ``SkeletonRemapper``), bound in place of the merged one
                 @endrst
                 */
                bool remapped = false;

                /**
                 * @brief Whether the remap runs once a frame from a latch elsewhere; otherwise each merge list runs it first
                 */
                bool latched = false;

                /**
                 * @brief Whether a draw with its current pose in ``vs-cb3`` alone merges into the MAIN skeleton
                 */
                bool currentPoseInCb3Only = false;

                /**
                 * @brief Whether ``vs-cb3`` gets the previous pose after every merge, whatever the marker says
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
                 * @brief Each target draw slot's bone window, as ``(vg_offset, vg_count)``
                 */
                std::vector<std::pair<std::string, std::string>> slotWindows;
            };

            /**
             * @brief The merge list of one target draw slot
             *
             * @param skeleton What the sections are named by
             * @param slot The slot
             */
            static std::string mergeListName(const Skeleton& skeleton, int slot);

            /**
             * @brief
             @rst
             The skeleton's `sections`_: the merged skeleton and its read-write copies, the marker on the game's
             bone-data constant buffer, the remapped skeleton with the list that remaps it and the merge a hidden slot
             runs (for :cpp:member:`Skeleton::remapped`), and one merge list per slot (#mergeListName)
             @endrst
             *
             * @param z3 The Z3 context the `if` lines are parsed in
             * @param skeleton What to write
             *
             * @return The sections, in order
             */
            static std::vector<std::unique_ptr<Section>> skeletonSections(Z3Context& z3, const Skeleton& skeleton);

            /**
             * @brief What #blendRemapSections writes
             */
            struct BlendRemap {
                /**
                 * @brief The fix's name for a `section`_ of the given name
                 */
                std::function<std::string(const std::string&)> name;

                /**
                 * @brief The remapped blend file
                 */
                std::string blendFile;

                /**
                 * @brief The files of the remap's three id buffers: ``VertexVG``, ``Forward`` and ``Reverse``
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
             * @brief The list #blendRemapSections runs WWMI's ``BlendRemapper`` from
             *
             * @param blendRemap What the sections are named by
             */
            static std::string blendRemapInitList(const BlendRemap& blendRemap);

            /**
             * @brief
             @rst
             The blend remap's resources and the one-time run of WWMI's ``BlendRemapper`` (#blendRemapInitList), guarded
             by a ``local`` so it costs one dispatch on the first drawn frame and binds the remapped blend after
             @endrst
             *
             * @param z3 The Z3 context the `if` lines are parsed in
             * @param blendRemap What to write
             *
             * @return The sections, in order
             */
            static std::vector<std::unique_ptr<Section>> blendRemapSections(Z3Context& z3, const BlendRemap& blendRemap);

            /**
             * @brief
             @rst
             The lines a remapped `section`_ drops when the target is past 256 bones, as ``(key, value prefix)`` -- each
             value matched with :cpp:func:`RegRemove::valueStartsWith`. They are the mod's own merge list and the two
             other routes to it (``CheckTextureOverride`` on ``vs-cb3`` / ``vs-cb4``), and its own bindings of the merged
             skeletons: each would write or bind the MOD's skeleton, sized for the source, and consume the bone-data
             marker the fix's own merge list is gated on -- which leaves the draw skinning target-space blend indices
             against the source's skeleton, or smears a second body through the motion vectors
             @endrst
             */
            static std::vector<std::pair<std::string, std::string>> remappedSkeletonRemovals();

            /**
             * @brief What #hideSection writes
             */
            struct Hide {
                /**
                 * @brief The `section`_'s name
                 */
                std::string name;

                /**
                 * @brief The comment written above it
                 */
                std::string comment;

                /**
                 * @brief The target's ``vb0`` hash
                 */
                std::string vb0Hash;

                /**
                 * @brief The slot's ``match_first_index`` and ``match_index_count``
                 */
                std::string indexOffset, indexCount;

                /**
                 * @brief The slot's bone window, ``vg_offset`` and ``vg_count``
                 */
                std::string vgOffset, vgCount;

                /**
                 * @brief A name of the slot for its frame latch (``$state_id_<slot>``)
                 */
                std::string slot;

                /**
                 * @brief
                 @rst
                 For a mod from before WWMI's merged skeleton: the list that merges the slot's bones
                 (:cpp:func:`mergeListName`), run before the draw is skipped -- or empty for a mod with a merged skeleton
                 @endrst
                 */
                std::string legacyMergeList;

                /**
                 * @brief
                 @rst
                 For a target past 256 bones: the list that merges the slot's window into the fix's own skeleton, run on
                 every draw -- or empty, which runs ``CommandListMergeSkeleton`` once a frame through the slot's latch
                 @endrst
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
             * @brief
             @rst
             The ``TextureOverride`` of a target draw slot nothing of the mod is drawn through: it skips the skin's own
             geometry and still merges the slot's bones, which other slots' vertices may be weighted to
             @endrst
             *
             * @param z3 The Z3 context the `if` lines are parsed in
             * @param hide What to write
             *
             * @return The section
             */
            static std::unique_ptr<Section> hideSection(Z3Context& z3, const Hide& hide);
    };
}

#endif
