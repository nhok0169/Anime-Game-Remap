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

#ifndef AGRemapCore_VGSplitGroupResource_H
#define AGRemapCore_VGSplitGroupResource_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/buffers/BufValue.h"
#include "AGRemapCore/model/buffers/VGComponentSplit.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"

namespace AGRemapCore {
    class BaseLogger;

    /**
     * @brief
     @rst
     What a :cpp:class:`VGSplitGroupResource` needs to know beyond its members: which component of
     the target it writes, every component of the target (the split is joint -- which triangles a
     cut component gets depends on what the negative-index components keep), and which of the mod's
     index buffers take part
     @endrst
     */
    struct VGSplitGroupConfig {
        /**
         * @brief A per-line edit of a vertex buffer, applied before the lines are filtered
         */
        using LineEdit = std::function<ByteVec(const ByteVec&)>;

        /**
         * @brief The component this group's files are written for
         */
        std::string component;

        /**
         * @brief Every component of the target
         */
        std::vector<VGComponentSpec> specs;

        /**
         * @brief
         @rst
         The source ``.ib`` of every drawn object, in draw order. The split needs all of them even
         when this group holds only one: a cut component's vertex set is the union over every
         object's kept triangles, so a group that saw one object would write a blend the other
         object's index buffer cannot index. Empty: the group's own ``ib`` members, in member order
         @endrst
         */
        std::vector<std::string> ibPaths;

        /**
         * @brief Applied to every line of the ``Texcoord.buf`` before filtering, or empty
         */
        LineEdit texcoordLineEdit;

        /**
         * @brief Applied to every line of the ``Position.buf`` before filtering, or empty
         */
        LineEdit positionLineEdit;
    };

    /**
     * @brief
     @rst
     Splits a mod's buffers for one component of a multi-component target, as a fix over a group of
     resources -- the resource-group counterpart of :cpp:class:`RemapBlendResource` :raw-html:`<br />`
     :raw-html:`<br />`

     A mod's ``Blend.buf``, ``Position.buf``, ``Texcoord.buf`` and ``.ib`` files cannot be fixed one
     at a time here: the blend decides which vertices a component keeps, the index buffers decide
     which triangles, and every vertex buffer then has to follow the same vertex set, renumbered the
     same way (issue #190). So the members are fixed together, from the one :cpp:class:`VGComponentSplit`
     :raw-html:`<br />` :raw-html:`<br />`

     Members are told apart by :cpp:member:`IniResource::type`: ``blend`` (exactly one), ``position``
     and ``texcoord`` (at most one each) and ``buf`` (the index buffers, any number, matched to
     :cpp:member:`VGSplitGroupConfig::ibPaths` by source path). Every member is an
     :cpp:class:`IniFixResource`, read from its ``srcPath`` and written to its ``fixedPath``
     @endrst
     *
     * @param group The group of resources
     * @param config What to split and for which component
     * @param logger Where to narrate, or ``nullptr``
     *
     * @throw std::invalid_argument If the group has no blend member, or a vertex buffer's size is not a whole number of lines
     *
     * @return Whether anything was written
     */
    bool fixVGSplitGroup(IniGroupedResource& group, const VGSplitGroupConfig& config, BaseLogger* logger);

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`RemapIniGroupedResource`

     A group of one mod's buffers, fixed by :cpp:func:`fixVGSplitGroup` -- see there
     @endrst
     */
    class VGSplitGroupResource: public RemapIniGroupedResource {
        public:
            /**
             * @brief Constructs a new group
             *
             * @param name The name of the group
             * @param resources The group's members, keyed by resource type
             * @param config What to split and for which component
             * @param fixFunc Custom function for fixing the group, overriding #_fix if given
             * @param isBuilt Whether the group is ready to be fixed
             */
            VGSplitGroupResource(std::string name, std::unordered_map<std::string, std::unique_ptr<IniResource>> resources,
                                  VGSplitGroupConfig config, std::function<bool(IniGroupedResource&)> fixFunc = nullptr,
                                  bool isBuilt = true);

            VGSplitGroupConfig config;

        protected:
            bool _fix() override;
    };
}

#endif
