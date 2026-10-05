#ifndef AGRemapCore_VGSplitGroupResBuilder_H
#define AGRemapCore_VGSplitGroupResBuilder_H

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

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/iniresources/VGSplitGroupResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResGroupCollect.h"


namespace AGRemapCore {
    class IniFile;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResGroupCollect::GroupedResBuilder`

     The plain-C++ builder of a :cpp:class:`VGSplitGroupResource` for a :cpp:class:`ResGroupCollect`
     -- the core-side counterpart of the `pybind11`_ layer's ``PyGroupedResBuilder``, which wraps a
     `Python`_ ``IniGroupedResBuilder`` :raw-html:`<br />` :raw-html:`<br />`

     Every grouped resource a C++ fixer builds goes through one of these: :cpp:func:`build` makes a
     fresh, not-yet-built group carrying the split's configuration, :cpp:func:`addResource` files
     each member the collect built (a :cpp:class:`BufReplace`'s :cpp:class:`IniFixResource`) into
     it, and :cpp:func:`store` hands the finished group to the ``.ini`` file's
     :cpp:func:`IniFile::getGroupedResources` :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        **The member is COPIED into the group, not moved.** The collect hands over a reference to a
        model whose owner is the resource-edit context's capture buffer (see
        :cpp:func:`IniFileResEditContext::storeResource`), which outlives nothing but the edit --
        and a :cpp:class:`IniGroupedResource` owns its members outright. An
        :cpp:class:`IniFixResource` is four strings, so a fresh one built from the same type and
        paths is the same resource; the captured original is never fixed on its own, since a
        captured model never reaches :cpp:func:`IniFile::getResources`
     @endrst
     */
    class VGSplitGroupResBuilder: public ResGroupCollect<>::GroupedResBuilder {
        public:
            using GraphId = ResGroupCollect<>::GraphId;

            /**
             * @brief Constructs a builder for one component's groups
             *
             * @param name What every built group is called, eg. ``"YelanYelanTranquilBodyBuffers"``
             * @param config The split -- which component, every component's spec, the ib paths, the line edits
             * @param iniFile The ``.ini`` file the built groups are stored into -- borrowed, may be ``nullptr`` (then nothing is stored)
             */
            VGSplitGroupResBuilder(std::string name, VGSplitGroupConfig config, IniFile* iniFile);

            /**
             * @brief
             @rst
             Builds the config for one group, from the query that group's resources co-occur under
             (see :cpp:func:`ResGroupCollect::GroupedResBuilder::beginGroup`) -- ``nullptr`` for a
             group with no query at all
             @endrst
             */
            using ConfigResolver = std::function<VGSplitGroupConfig(const Z3Predicate*)>;

            /**
             * @brief
             @rst
             A config built PER GROUP, for a source whose resources branch -- the split's side of
             :cpp:func:`VGMergeGroupResBuilder::VGMergeGroupResBuilder`'s resolver :raw-html:`<br />`
             :raw-html:`<br />`

             A merged master names one variant's index buffers per ``$swapvar`` branch, and the split
             needs EVERY drawn object's index buffer of the variant it is splitting (see
             :cpp:member:`VGSplitGroupConfig::ibPaths`). One config for every group hands each
             variant the first variant's, which the group's own ``ib`` member is then not one of
             @endrst
             *
             * @param name The resource group's name
             * @param configFor Builds the config for one group -- see #ConfigResolver
             * @param iniFile The ``.ini`` file the groups are stored on
             */
            VGSplitGroupResBuilder(std::string name, ConfigResolver configFor, IniFile* iniFile);

            void beginGroup(const std::optional<Z3Predicate>& query) override;
            IniGroupedResource* build() override;
            void store(IniGroupedResource& resource) override;
            void addResource(IniGroupedResource& group, const GraphId& resType, IniResource& resource) override;

            /**
             * @brief The groups built so far -- owned here, shared with the ``.ini`` file once stored
             */
            const std::vector<std::shared_ptr<VGSplitGroupResource>>& groups() const;

        private:
            std::string name_;
            ConfigResolver configFor_;
            VGSplitGroupConfig config_;
            std::optional<Z3Predicate> query_;
            IniFile* iniFile_;
            std::vector<std::shared_ptr<VGSplitGroupResource>> groups_;
    };
}

#endif
