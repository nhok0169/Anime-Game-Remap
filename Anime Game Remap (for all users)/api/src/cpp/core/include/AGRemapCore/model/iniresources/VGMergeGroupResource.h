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

#ifndef AGRemapCore_VGMergeGroupResource_H
#define AGRemapCore_VGMergeGroupResource_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/buffers/BufValue.h"
#include "AGRemapCore/model/buffers/VGComponentMerge.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"

namespace AGRemapCore {
    class BaseLogger;

    /**
     * @brief
     @rst
     One source component's files, as a :cpp:class:`VGMergeGroupResource` finds them on disk
     @endrst
     */
    struct VGMergeComponentFiles {
        /**
         * @brief The component and its own vertex group row
         */
        VGMergeComponentSpec spec;

        /**
         * @brief This component's ``Blend.buf``
         */
        std::string blendPath;

        /**
         * @brief This component's ``Position.buf``
         */
        std::string positionPath;

        /**
         * @brief This component's ``Texcoord.buf``
         */
        std::string texcoordPath;
    };

    /**
     * @brief
     @rst
     One object of the TARGET, and the source objects drawn through it
     @endrst
     */
    struct VGMergeObject {
        /**
         * @brief
         @rst
         The source ``.ib`` this object's group member was collected from -- the representative,
         which is :cpp:member:`members`' first entry. The member resource is matched by this path
         @endrst
         */
        std::string srcPath;

        /**
         * @brief
         @rst
         The source objects this target object draws, as ``(component, .ib path)``, in draw order.
         More than one means a MERGE into a single draw -- YelanTranquil's ``Bang`` and ``Eye`` both
         land on Yelan's head -- which is what saves the second ``.ini`` file a collision would
         otherwise cost
         @endrst
         */
        std::vector<std::pair<std::string, std::string>> members;
    };

    /**
     * @brief
     @rst
     What a :cpp:class:`VGMergeGroupResource` needs beyond its members: every component of the
     SOURCE (the merge is joint -- an index buffer's offset is the sum of the vertices before its
     component) and which source objects each target object draws
     @endrst
     */
    struct VGMergeGroupConfig {
        /**
         * @brief
         @rst
         Every component of the source, in merge order. The first takes offset 0, so its index
         buffers pass through unchanged -- worth putting the biggest there
         @endrst
         */
        std::vector<VGMergeComponentFiles> components;

        /**
         * @brief The target's drawn objects
         */
        std::vector<VGMergeObject> objects;

        /**
         * @brief
         @rst
         How many bytes one index takes, keyed by the source ``.ib`` path -- 2 for a buffer its
         ``.ini`` declares ``DXGI_FORMAT_R16_UINT``. A path that is not here is 4, which is every
         mod that does not say otherwise.

         The width is carried rather than inferred because it CANNOT be inferred: a 16-bit buffer
         whose byte count divides by 12 reads as 32-bit without complaint, at half the index count
         and with values past the end of the mesh. See :cpp:class:`IbFile`
         @endrst
         */
        std::unordered_map<std::string, std::size_t> ibBytesPerIndex;
    };

    /**
     * @brief
     @rst
     Merges a multi-component mod's buffers onto a single-component target, as a fix over a group of
     resources -- the inverse of :cpp:func:`fixVGSplitGroup` :raw-html:`<br />` :raw-html:`<br />`

     The buffers cannot be fixed one at a time: every component's index buffer shifts by the vertices
     of the components before it, so an index buffer written without knowing the others is wrong. So
     the members are fixed together, from the one :cpp:class:`VGComponentMerge` :raw-html:`<br />`
     :raw-html:`<br />`

     Members are told apart by :cpp:member:`IniResource::type`, exactly as the split does it:
     ``blend``, ``position`` and ``texcoord`` (at most one each -- the merged buffer is bound to the
     target's single hash) and ``buf`` (the index buffers, matched to
     :cpp:member:`VGMergeObject::srcPath`). A target object whose component sits at offset 0 and
     draws one source object needs no member at all: its index buffer is already right
     @endrst
     *
     * @param group The group of resources
     * @param config The source's components and the target's objects
     * @param logger Where to narrate, or ``nullptr``
     *
     * @throw std::invalid_argument If the group has no blend member, or a member's source path is not one of the objects'
     *
     * @return Whether anything was written
     */
    bool fixVGMergeGroup(IniGroupedResource& group, const VGMergeGroupConfig& config, BaseLogger* logger);

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`RemapIniGroupedResource`

     A group of one mod's buffers, fixed by :cpp:func:`fixVGMergeGroup` -- see there
     @endrst
     */
    class VGMergeGroupResource: public RemapIniGroupedResource {
        public:
            /**
             * @brief Constructs a new group
             *
             * @param name The name of the group
             * @param resources The group's members, keyed by resource type
             * @param config The source's components and the target's objects
             * @param fixFunc Custom function for fixing the group, overriding #_fix if given
             * @param isBuilt Whether the group is ready to be fixed
             */
            VGMergeGroupResource(std::string name, std::unordered_map<std::string, std::unique_ptr<IniResource>> resources,
                                  VGMergeGroupConfig config, std::function<bool(IniGroupedResource&)> fixFunc = nullptr,
                                  bool isBuilt = true);

            VGMergeGroupConfig config;

        protected:
            bool _fix() override;
    };
}

#endif
