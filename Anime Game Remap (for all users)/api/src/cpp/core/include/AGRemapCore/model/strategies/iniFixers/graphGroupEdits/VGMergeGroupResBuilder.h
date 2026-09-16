#ifndef AGRemapCore_VGMergeGroupResBuilder_H
#define AGRemapCore_VGMergeGroupResBuilder_H

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

#include "AGRemapCore/model/iniresources/VGMergeGroupResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResGroupCollect.h"


namespace AGRemapCore {
    class IniFile;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResGroupCollect::GroupedResBuilder`

     The plain-C++ builder of a :cpp:class:`VGMergeGroupResource` for a :cpp:class:`ResGroupCollect`
     -- the merge's counterpart of :cpp:class:`VGSplitGroupResBuilder`, and the same three steps:
     :cpp:func:`build` makes a fresh, not-yet-built group carrying the merge's configuration,
     :cpp:func:`addResource` files each member the collect built into it, and :cpp:func:`store` hands
     the finished group to the ``.ini`` file's :cpp:func:`IniFile::getGroupedResources`
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        **The member is COPIED into the group, not moved**, for the same reason the split's builder
        copies -- see :cpp:class:`VGSplitGroupResBuilder`'s own note
     @endrst
     */
    class VGMergeGroupResBuilder: public ResGroupCollect<>::GroupedResBuilder {
        public:
            using GraphId = ResGroupCollect<>::GraphId;

            /**
             * @brief Constructs a builder for a source's merged groups
             *
             * @param name What every built group is called, eg. ``"YelanTranquilYelanBuffers"``
             * @param config The merge -- every source component's files and row, and the target's objects
             * @param iniFile The ``.ini`` file the built groups are stored into -- borrowed, may be ``nullptr`` (then nothing is stored)
             */
            VGMergeGroupResBuilder(std::string name, VGMergeGroupConfig config, IniFile* iniFile);

            /**
             * @brief
             @rst
             Builds the config for one group, from the query that group's resources co-occur under
             (see :cpp:func:`ResGroupCollect::GroupedResBuilder::beginGroup`) -- ``nullptr`` for a
             group with no query at all
             @endrst
             */
            using ConfigResolver = std::function<VGMergeGroupConfig(const Z3Predicate*)>;

            /**
             * @brief
             @rst
             A config built PER GROUP, for a source whose resources branch :raw-html:`<br />`
             :raw-html:`<br />`

             A merged mod's master is several mods behind one ``.ini``: each ``$swapvar`` branch of a
             ``CommandList`` names a different variant's buffers, and a component that does not
             branch keeps its single one whichever variant is selected.
             :cpp:class:`ResGroupCollect` separates the variants by SATISFIABILITY -- each becomes
             its own group, and #build is called once per group -- so the resolver is handed that
             group's query and answers with the buffers that can be selected at the same time as it.

             Nothing here is positional, which is the point: a hand-made mod with independent toggles
             or nested conditions has no ``i``-th variant to index, and pairing one component's
             ``i``-th branch with another's is wrong there in a way that produces a plausible model
             and reports nothing
             @endrst
             *
             * @param name The resource group's name
             * @param configFor Builds the config for one group -- see #ConfigResolver
             * @param iniFile The ``.ini`` file the groups are stored on
             */
            VGMergeGroupResBuilder(std::string name, ConfigResolver configFor, IniFile* iniFile);

            void beginGroup(const std::optional<Z3Predicate>& query) override;
            IniGroupedResource* build() override;
            void store(IniGroupedResource& resource) override;
            void addResource(IniGroupedResource& group, const GraphId& resType, IniResource& resource) override;

            /**
             * @brief The groups built so far -- owned here, shared with the ``.ini`` file once stored
             */
            const std::vector<std::shared_ptr<VGMergeGroupResource>>& groups() const;

        private:
            std::string name_;
            ConfigResolver configFor_;
            VGMergeGroupConfig config_;
            std::optional<Z3Predicate> query_;
            IniFile* iniFile_;
            std::vector<std::shared_ptr<VGMergeGroupResource>> groups_;
    };
}

#endif
