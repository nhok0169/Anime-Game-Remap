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

#include "AGRemapCore/data/WWMIRuntime.h"

#include <tuple>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniSectionText.h"


namespace AGRemapCore {
    std::string WWMIRuntime::mergeListName(const Skeleton& skeleton, int slot) {
        return skeleton.name("CommandListMergeSlot" + std::to_string(slot));
    }

    std::string WWMIRuntime::blendRemapInitList(const BlendRemap& blendRemap) {
        return blendRemap.name("CommandListBlendRemap");
    }

    // What a mod from before WWMI's merged skeleton has none of: the two skeleton buffers and their read-only copies,
    // the marker on the game's bone-data constant buffer, and one merge list per target slot -- each gated on the
    // marker, so a pass with something else in that slot cannot merge junk into the skeleton.
    std::vector<std::unique_ptr<WWMIRuntime::Section>> WWMIRuntime::skeletonSections(Z3Context& z3, const Skeleton& skeleton) {
        std::vector<std::unique_ptr<Section>> sections;
        const std::string merged = skeleton.name(IniKeywords::Resource + std::string("MergedSkeleton"));
        const std::string mergedRW = skeleton.name(IniKeywords::Resource + std::string("MergedSkeletonRW"));
        const std::string extra = skeleton.name(IniKeywords::Resource + std::string("ExtraMergedSkeleton"));
        const std::string extraRW = skeleton.name(IniKeywords::Resource + std::string("ExtraMergedSkeletonRW"));
        sections.push_back(SectionText(z3, merged).build());
        sections.push_back(SectionText(z3, extra).build());
        for (const std::string& name : {mergedRW, extraRW}) {
            sections.push_back(SectionText(z3, name)
                       .keys({{IniKeywords::Type, "RWBuffer"},
                              {IniKeywords::Format, "R32G32B32A32_FLOAT"},
                              {IniKeywords::Array, std::to_string(skeleton.mergedSkeletonSlots)}})
                       .build());
        }

        sections.push_back(SectionText(z3, skeleton.name("TextureOverrideMarkBoneDataCB"))
                   .keys({{IniKeywords::Hash, skeleton.cb4Hash},
                          {IniKeywords::MatchPriority, "0"},
                          {IniKeywords::FilterIndex, skeleton.boneDataFilter}})
                   .build());

        // The private skeleton a blend remap needs, gathered right after the merge:
        // SkeletonRemapper writes remapped[i] = merged[forward[i]], so a vertex whose
        // blend names LOCAL i reaches the bone forward[i] -- which is how an 8-bit index
        // addresses a 420-slot skeleton at all. Bound in place of the merged one.
        const std::string remappedRW = skeleton.name(IniKeywords::Resource + std::string("RemappedSkeletonRW"));
        const std::string remapped = skeleton.name(IniKeywords::Resource + std::string("RemappedSkeleton"));
        const std::string extraRemappedRW = skeleton.name(IniKeywords::Resource + std::string("ExtraRemappedSkeletonRW"));
        const std::string extraRemapped = skeleton.name(IniKeywords::Resource + std::string("ExtraRemappedSkeleton"));
        if (skeleton.remapped) {
            for (const std::string& name : {remapped, extraRemapped, remappedRW, extraRemappedRW}) {
                sections.push_back(SectionText(z3, name).build());
            }
        }

        // The remap, run ONCE PER FRAME from the COMPLETE merged skeleton rather than per draw
        // from a partly filled one (2026-10-02).
        //
        // WWMI's own design: each component's draw merges its window into a persistent RW
        // buffer, and `[Present]` snapshots the result and remaps it, so the NEXT frame's draws
        // all bind one complete skeleton. Remapping inside each draw instead reads a buffer
        // holding only the windows merged so far this frame -- which differs per slot and per
        // frame, so the render comes out UNSTABLE rather than wrong: five reloads of one mod,
        // same files and same build, gave four different pictures.
        //
        // Anchored on the frame's first remapped draw rather than on `[Present]`, which is
        // equivalent -- nothing but these merge lists writes the RW buffer, so between
        // `[Present]` and the first draw it still holds exactly the previous frame's complete
        // merge. It also keeps the hook inside the fix's OWN sections: the mod's `[Present]` is
        // the host's, and no .ini in the corpus carries two of them, so appending one would bet
        // on 3dmigoto merging duplicate sections within a file, which nothing here establishes.
        //
        // `$state_id` is the host's frame counter, flipped once a frame at `[Present]`.
        if (skeleton.remapped) {
            // NO FRAME LATCH (2026-10-02). It was latched on `$state_id`, which the identity mod
            // maintains and a real WWMI Tools mod of the current generation does NOT -- it declares
            // `global $state_id = 0`, never assigns it, and keeps its own `$merge_status_id`. The
            // latch then read 0 != 0 forever: `if $remapped_state != $state_id: false` 118841 times
            // and true ZERO in 3dmigoto's log, so the remap never ran, the remapped skeleton was
            // never created, and binding that null UNBOUND vs-cb4 -- an invisible model.
            //
            // Unconditional costs a few dispatches a frame and needs no frame counter, because the
            // property that matters never came from the latch: the merged RW buffer is PERSISTENT
            // and accumulates, so at any draw it holds every window from this frame or the last, and
            // a remap taken at any point reads a COMPLETE skeleton. That is what moving it out of
            // the per-draw path was for, and it survives a mod that keeps no frame counter.
            SectionText remap(z3, skeleton.name("CommandListRemapMergedSkeleton"));
            for (const auto& cb : {std::make_tuple(mergedRW, merged, remappedRW, remapped),
                                   std::make_tuple(extraRW, extra, extraRemappedRW, extraRemapped)}) {
                remap.keys({{std::get<1>(cb), "copy " + std::get<0>(cb)},
                                {"cs-t37", skeleton.blendRemapForward},
                                {IniKeywords::WWMIBlendRemapId, "0"},
                                {IniKeywords::WWMIVgCount, std::to_string(skeleton.blendRemapBones)},
                                {"cs-t38", std::get<1>(cb)},
                                // Seeded from the merged RW, which is declared with a size. An empty
                                // resource bound to `cs-u5` has no backing buffer: the dispatch writes
                                // nowhere, the `copy` below then finds no source, and binding that null
                                // UNBINDS the slot -- invisible rather than wrong.
                                {std::get<2>(cb), "copy " + std::get<0>(cb)},
                                {"cs-u5", std::get<2>(cb)},
                                {IniKeywords::Run, IniKeywords::WWMISkeletonRemapper},
                                {std::get<3>(cb), "copy " + std::get<2>(cb)}});
            }

            sections.push_back(remap.build());
        }

        // The merge a HIDDEN slot runs. A target slot nothing is remapped onto still
        // contributes bones -- Chisa's [391, 419) carries bone 409, which 1185 of the identity
        // mod's weighted influence slots name -- and its `...RemapHide` section used to run the
        // host's copied `CommandListMergeSkeleton`, which writes the MOD's merged skeleton,
        // sized for the source. The bone was then never written into the buffer the remap
        // reads, so every vertex on it skinned against a zero matrix and collapsed to the
        // origin -- and a triangle with one corner there is a plane across the scene.
        //
        // Merge only, no bind: the caller sets vg_offset / vg_count and the draw is skipped.
        if (skeleton.remapped) {
            SectionText window(z3, skeleton.name("CommandListMergeWindow"));
            window.key(IniKeywords::WWMICustomMeshScale, "1.00");
            for (const auto& cb : {std::make_pair(std::string("vs-cb4"), mergedRW),
                                   std::make_pair(std::string("vs-cb3"), extraRW)}) {
                window.open(cb.first + " == " + skeleton.boneDataFilter)
                      .keys({{"cs-cb8", IniKeywords::Ref + " " + cb.first},
                             {"cs-u6", cb.second},
                             {IniKeywords::Run, IniKeywords::WWMISkeletonMerger}})
                      .close();
            }

            sections.push_back(window.build());
        }

        for (std::size_t slot = 0; slot < skeleton.slotWindows.size(); ++slot) {
            const auto& s = skeleton.slotWindows[slot];
            SectionText mergeList(z3, mergeListName(skeleton, static_cast<int>(slot)));
            if (skeleton.remapped && !skeleton.latched) {
                // Only without a frame latch (see latchEdits): a remap here runs before
                // EVERY slot's merge and so reads a buffer whose windows differ in age.
                mergeList.key(IniKeywords::Run, skeleton.name("CommandListRemapMergedSkeleton"));
            }

            // see WWMIFixerConfig::currentPoseInCb3Only. Decided BEFORE anything is rebound: a
            // `vs-cbN == filter` test reads the buffer bound at that moment, and once vs-cb4
            // holds the fix's own skeleton it no longer carries the marker.
            const std::string Cb3Only = "$cb3_only";
            if (skeleton.currentPoseInCb3Only) {
                mergeList.key(IniKeywords::Local + " " + Cb3Only)
                         .key(Cb3Only, "0")
                         .open("vs-cb3 == " + skeleton.boneDataFilter + " && vs-cb4 != " + skeleton.boneDataFilter)
                         .key(Cb3Only, "1")
                         .close();
            }

            for (const auto& cb : {std::make_tuple(std::string("vs-cb4"), mergedRW, merged, remappedRW, remapped),
                                   std::make_tuple(std::string("vs-cb3"), extraRW, extra, extraRemappedRW, extraRemapped)}) {
                const bool cb3 = std::get<0>(cb) == "vs-cb3";
                mergeList.open(std::get<0>(cb) + " == " + skeleton.boneDataFilter
                               + (cb3 && skeleton.currentPoseInCb3Only ? " && " + Cb3Only + " == 0" : ""))
                         .keys({{IniKeywords::WWMIVgOffset, s.first},
                                {IniKeywords::WWMIVgCount, s.second},
                                {IniKeywords::WWMICustomMeshScale, "1.00"},
                                {"cs-cb8", IniKeywords::Ref + " " + std::get<0>(cb)},
                                {"cs-u6", std::get<1>(cb)},
                                {IniKeywords::Run, IniKeywords::WWMISkeletonMerger}});

                if (skeleton.remapped) {
                    // Bind what the frame's remap produced. The snapshot and the SkeletonRemapper
                    // dispatch that used to sit here moved into CommandListRemapMergedSkeleton,
                    // which runs once a frame from the complete merge.
                    mergeList.key(std::get<0>(cb), std::get<4>(cb));
                } else {
                    // The snapshot the two branches used to share, kept here so a legacy mod writes
                    // exactly the lines it wrote before.
                    mergeList.keys({{std::get<2>(cb), "copy " + std::get<1>(cb)},
                                    {std::get<0>(cb), std::get<2>(cb)}});
                }

                mergeList.close();
            }

            // ...and a draw whose current pose is in vs-cb3 alone: into the MAIN skeleton
            if (skeleton.currentPoseInCb3Only) {
                mergeList.open(Cb3Only + " == 1")
                         .keys({{IniKeywords::WWMIVgOffset, s.first},
                                {IniKeywords::WWMIVgCount, s.second},
                                {IniKeywords::WWMICustomMeshScale, "1.00"},
                                {"cs-cb8", IniKeywords::Ref + " vs-cb3"},
                                {"cs-u6", mergedRW},
                                {IniKeywords::Run, IniKeywords::WWMISkeletonMerger}});
                if (skeleton.remapped) {
                    mergeList.key("vs-cb3", remapped);
                } else {
                    mergeList.keys({{merged, "copy " + mergedRW}, {"vs-cb3", merged}});
                }

                mergeList.close();
            }

            // AND THE PREVIOUS POSE AGAIN, OUTSIDE THE GUARD (2026-10-03).
            //
            // The game does not set `boneDataFilter` on cb3 for every pass. Measured in a
            // frame dump of a remapped mod: five draws -- one for EVERY main body slot --
            // had cb4 replaced and cb3 still holding the TARGET's own skeleton, so the
            // shader reprojected our pose from hers. The motion is nonsense and TAA smears
            // a second body across the character: the "double ghost body" reported on every
            // mod of the pair AND on the identity mod, which is the target skin's own model
            // and so proves the fault is the remap rather than any mod's content.
            //
            // Binding the remapped PREVIOUS pose rather than the current one keeps genuine
            // motion vectors; both clear the smear in game, and this one does not flatten
            // the character's motion blur to zero. On a pass that does carry the marker the
            // guarded block above has already bound the same resource, so this is a no-op.
            if (skeleton.bindPrevPoseAlways) {
                if (skeleton.currentPoseInCb3Only) {
                    mergeList.open(Cb3Only + " == 0")
                             .key("vs-cb3", skeleton.remapped ? extraRemapped : extra)
                             .close();
                } else {
                    mergeList.key("vs-cb3", skeleton.remapped ? extraRemapped : extra);
                }
            }

            sections.push_back(mergeList.build());
        }
        return sections;
    }

    // The blend remap's resources, and the one-time run of WWMI's BlendRemapper that rewrites the remapped blend's ids
    // into the remap's LOCAL space. The strided view is the fix's own declaration of the same file: `copy_desc` needs a
    // buffer with the blend's stride.
    std::vector<std::unique_ptr<WWMIRuntime::Section>> WWMIRuntime::blendRemapSections(Z3Context& z3, const BlendRemap& blendRemap) {
        std::vector<std::unique_ptr<Section>> sections;
        const std::string vertexVG = blendRemap.name(IniKeywords::Resource + std::string("BlendRemapVertexVGBuffer"));
        const std::string forward = blendRemap.name(IniKeywords::Resource + std::string("BlendRemapForwardBuffer"));
        const std::string reverse = blendRemap.name(IniKeywords::Resource + std::string("BlendRemapReverseBuffer"));
        const std::string noStride = blendRemap.name(IniKeywords::Resource + std::string("BlendNoStride"));
        const std::string strided = blendRemap.name(IniKeywords::Resource + std::string("BlendStrided"));
        const std::string blendRW = blendRemap.name(IniKeywords::Resource + std::string("RemappedBlendBufferRW"));
        const std::string blendOut = blendRemap.name(IniKeywords::Resource + std::string("RemappedBlendBuffer"));
        const std::string blendPath = blendRemap.blendFile;

        // no stride on the three the compute shader reads: a Buffer declared with one
        // cannot be addressed by a compute shader (wwmiIdentityMod.py's own note)
        for (const auto& entry : {std::make_pair(vertexVG, blendRemap.remapFile("VertexVG")),
                                  std::make_pair(forward, blendRemap.remapFile("Forward")),
                                  std::make_pair(reverse, blendRemap.remapFile("Reverse")),
                                  std::make_pair(noStride, blendPath)}) {
            sections.push_back(SectionText(z3, entry.first)
                       .keys({{IniKeywords::Type, IniKeywords::Buffer},
                              {IniKeywords::Format, entry.first == noStride ? "DXGI_FORMAT_R8_UINT"
                                                                 : "DXGI_FORMAT_R16_UINT"},
                              {IniKeywords::Filename, entry.second}})
                       .build());
        }

        sections.push_back(SectionText(z3, strided)
                   .keys({{IniKeywords::Type, IniKeywords::Buffer},
                          {IniKeywords::Format, "DXGI_FORMAT_R8_UINT"},
                          {IniKeywords::Stride, std::to_string(blendRemap.blendStride)},
                          {IniKeywords::Filename, blendPath}})
                   .build());
        sections.push_back(SectionText(z3, blendRW).build());
        sections.push_back(SectionText(z3, blendOut).build());

        sections.push_back(SectionText(z3, blendRemapInitList(blendRemap))
                   .key(IniKeywords::Local + " $blendRemapReady")
                   .open("!$blendRemapReady")
                   .keys({{IniKeywords::WWMICustomVertexCount, IniKeywords::WWMIMeshVertexCount},
                          {IniKeywords::WWMIWeightsPerVertexCount, std::to_string(blendRemap.influences)},
                          {IniKeywords::WWMIBlendRemapId, "0"},
                          {"cs-t34", IniKeywords::Ref + " " + reverse},
                          {"cs-t35", IniKeywords::Ref + " " + vertexVG},
                          {blendRW, "copy " + noStride},
                          {"cs-u4", IniKeywords::Ref + " " + blendRW},
                          {IniKeywords::Run, IniKeywords::WWMIBlendRemapper},
                          {blendOut, "copy " + blendRW},
                          {blendOut, "copy_desc " + strided},
                          {"$blendRemapReady", "1"}})
                   .close()
                   .key(blendRemap.blendReg, blendOut)
                   .build());
        return sections;
    }
}
