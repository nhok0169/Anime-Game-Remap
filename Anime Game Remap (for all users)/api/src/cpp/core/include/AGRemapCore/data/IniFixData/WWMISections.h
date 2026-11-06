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
#include "AGRemapCore/model/strategies/iniFixers/CustomShaderBackups.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/tools/z3/Z3Context.h"

namespace AGRemapCore {
    /**
     * @brief
     @rst
     The sections a fix of a Wuthering Waves mod writes to drive WWMI, the mod loader in the game: the merged
     skeleton a mod from before WWMI's merged skeleton has none of, a remapped copy of it for a target past 256 bones,
     one merge list per target draw slot, the blend remap that runs WWMI's ``BlendRemapper``, the section of a
     target draw slot nothing of the mod is drawn through, and the sections of a draw made with a pixel shader of the fix's own :raw-html:`<br />` :raw-html:`<br />`

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

                /**
                 * @brief Only merge the slot's window (#mergeWindowList), for a file whose skeleton needs the bones of a slot
                 * another file draws: no `$object_detected` and no skip
                 */
                bool mergeOnly = false;
            };

            /**
             * @brief The `TextureOverride` of a target draw slot nothing of the mod is drawn through: it skips the skin's own
             * geometry and still merges the slot's bones, which other slots' vertices may be weighted to
             * @param z3 The Z3 context the `if` lines are parsed in
             * @param hide What to write
             * @return The section
             */
            static std::unique_ptr<Section> hideSection(Z3Context& z3, const Hide& hide);

            /**
             * @brief register -> the resource its previous binding is parked in -- see #bypasses
             */
            using Bypasses = std::vector<std::pair<std::string, std::string>>;

            /**
             * @brief
             @rst
             The mesh buffers the fix has to put back after a remapped draw, each with the resource it is parked in:
             every one the mod's shared-resource OVERRIDE list binds that its CLEANUP list does not already restore
             (``vb0``, which WWMI's own pair handles). On the mod's own character each draw is matched by one of its
             sections, so a buffer left bound is rebound before anything reads it; remapped, the first draw of a target
             slot the mod does not cover inherits the mod's buffers. Empty for a mod without both lists
             @endrst
             *
             * @param data The mod's parse data
             * @param sections The collect of every key of every `section`_ (:cpp:member:`IniParseRows::Sections`)
             * @param overrideList The mod's override list (``CommandListOverrideSharedResources``)
             * @param cleanupList The mod's cleanup list (``CommandListCleanupSharedResources``)
             * @param modName The mod the fix is for, which names the resources (``ResourceBypass<Reg><Mod>RemapFix``)
             * @return The bypasses
             */
            static Bypasses bypasses(const IniParseData& data, const std::string& sections, const std::string& overrideList,
                                     const std::string& cleanupList, const std::string& modName);

            /**
             * @brief The list that binds every bypassed buffer back (``CommandListRestoreSharedResources<Mod>RemapFix``)
             * @param modName The mod the fix is for
             * @return The name
             */
            static std::string restoreListName(const std::string& modName);

            /**
             * @brief
             @rst
             The bypass pair's sections: an empty resource for each bypass -- a handle ``= ref <reg>`` parks a binding in,
             like the mod's own ``[ResourceBypassVB0]`` -- and the list that binds each register back from it. The
             capture before the override list rebinds a register, and the ``run =`` of the restore list at the end of
             the cleanup list, are edits on the mod's own lists (a :cpp:class:`RegAnchorAdd` and a :cpp:class:`RegAdd`)
             @endrst
             *
             * @param z3 The Z3 context the sections are built in
             * @param bypasses The bypasses (#bypasses)
             * @param restoreList The restore list's name (#restoreListName)
             * @return The sections, empty when there are no bypasses
             */
            static std::vector<std::unique_ptr<Section>> bypassSections(Z3Context& z3, const Bypasses& bypasses,
                                                                         const std::string& restoreList);

            /**
             * @brief `KVPs`_, in order
             */
            using KVPs = std::vector<std::pair<std::string, std::string>>;

            /**
             * @brief What shaderDrawSections writes
             */
            struct ShaderDraw {
                /**
                 * @brief
                 @rst
                 The fix's name of one of the draw's sections, from its kind (``CommandList``, ``CustomShader``,
                 ``Resource``, or ``""`` for a variable) and a suffix (``""`` for the draw's own list and shader)
                 @endrst
                 */
                std::function<std::string(const std::string& kind, const std::string& suffix)> name;

                /**
                 * @brief The `drawindexed` value each pass draws
                 */
                std::string draw;

                /**
                 * @brief The `.ini` condition true on the passes the shader draws on
                 */
                std::string onPlan;

                /**
                 * @brief A resource the pass must bind for the shader to draw (`o3`), or empty to draw on every pass of #onPlan
                 */
                std::string onlyWith;

                /**
                 * @brief The line that draws the range everywhere the shader does not, or an empty key to draw nothing there
                 * (a draw made after the mod's own, which still draws the range)
                 */
                std::pair<std::string, std::string> fallback;

                /**
                 * @brief
                 @rst
                 Each pass's ``CustomShader`` lines, before its ``drawindexed``: the first pass's section is
                 ``name("CustomShader", "")``, pass ``p``'s that name followed by ``Pass<p>``
                 @endrst
                 */
                std::vector<KVPs> passes;

                /**
                 * @brief The lines the shaders assign whose registers are saved before the runs and restored after them
                 * (CustomShaderBackups::wrap)
                 */
                KVPs assigned;

                /**
                 * @brief
                 @rst
                 For a draw held back until other draws of the same pass are made: the resource that tells one pass from
                 another (a render target), or empty to draw at once
                 @endrst
                 */
                std::string afterTarget;

                /**
                 * @brief How many game draws a pass makes before the held-back draw, for #afterTarget
                 */
                int afterDraws = 0;

                /**
                 * @brief The registers the held-back draw takes from the pass it was held in, for #afterTarget
                 */
                std::vector<std::string> afterCarried;
            };

            /**
             * @brief
             @rst
             One range of a mod drawn with a pixel shader of the fix's own, on the passes :cpp:member:`ShaderDraw::onPlan`
             names and as :cpp:member:`ShaderDraw::fallback` everywhere else: the command list a ``run =`` in place of the
             mod's ``drawindexed`` calls, and one ``CustomShader`` per pass. A draw held back
             (:cpp:member:`ShaderDraw::afterTarget`) adds the list each awaited game draw runs (``name("CommandList",
             "After")``) and the resources that count them; its variables are #shaderDrawConstants
             @endrst
             *
             * @param z3 The Z3 context the `if` lines are parsed in
             * @param draw What to write
             * @param backups What the runs save and restore
             * @return The sections, in order
             */
            static std::vector<std::unique_ptr<Section>> shaderDrawSections(Z3Context& z3, const ShaderDraw& draw,
                                                                             CustomShaderBackups& backups);

            /**
             * @brief The `global` variables a held-back draw keeps (empty for one drawn at once), for a `[Constants]`
             * @param draw The draw
             * @return The lines
             */
            static KVPs shaderDrawConstants(const ShaderDraw& draw);
    };
}

#endif
