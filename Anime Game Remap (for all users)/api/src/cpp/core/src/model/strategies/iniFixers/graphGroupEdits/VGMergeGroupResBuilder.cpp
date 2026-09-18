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

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/VGMergeGroupResBuilder.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iniresources/IniResource.h"


namespace AGRemapCore {

    VGMergeGroupResBuilder::VGMergeGroupResBuilder(std::string name, VGMergeGroupConfig config, IniFile* iniFile):
        name_(std::move(name)), config_(std::move(config)), iniFile_(iniFile) {}


    VGMergeGroupResBuilder::VGMergeGroupResBuilder(std::string name, ConfigResolver configFor, IniFile* iniFile):
        name_(std::move(name)), configFor_(std::move(configFor)), iniFile_(iniFile) {}


    void VGMergeGroupResBuilder::beginGroup(const std::optional<Z3Predicate>& query) {
        query_ = query;
    }


    IniGroupedResource* VGMergeGroupResBuilder::build() {
        // The config for the group being built. ResGroupCollect calls this once per satisfiable
        // combination of the resources it found, and beginGroup has just said which one -- so the
        // resolver answers with the buffers that can be selected at the same time as this group,
        // rather than this being the n-th call and n indexing a list of variants.
        VGMergeGroupConfig resolved;
        const VGMergeGroupConfig* config = &config_;
        if (configFor_) {
            resolved = configFor_(query_.has_value() ? &(*query_) : nullptr);
            config = &resolved;
        }

        auto group = std::make_shared<VGMergeGroupResource>(
            name_, std::unordered_map<std::string, std::unique_ptr<IniResource>>{}, *config, nullptr, /*isBuilt*/ false);
        groups_.push_back(group);
        return group.get();
    }


    void VGMergeGroupResBuilder::store(IniGroupedResource& resource) {
        if (iniFile_ == nullptr) {
            return;
        }

        for (const std::shared_ptr<VGMergeGroupResource>& group : groups_) {
            if (group.get() == &resource) {
                iniFile_->getGroupedResources().push_back(group);
                return;
            }
        }
    }


    void VGMergeGroupResBuilder::addResource(IniGroupedResource& group, const GraphId& resType, IniResource& resource) {
        // A copy -- see the class's own note. The paths are already absolute, so the folder the
        // constructor resolves against is irrelevant; the type is what fixVGMergeGroup tells the
        // members apart by, and BufReplace typed it by kind.
        std::string fixedPath;
        if (const auto* fixResource = dynamic_cast<const IniFixResource*>(&resource)) {
            fixedPath = fixResource->fixedPath;
        }

        auto member = std::make_unique<IniFixResource>(resource.type, iniFile_ == nullptr ? "" : iniFile_->getFolder(),
                                                        resource.srcPath, fixedPath);
        member->logger = resource.logger;

        // Keyed by the resource's whole mod object -- one entry per buffer of the group, and the
        // string form is only a map key nothing reads back.
        const std::string key = std::to_string(resType.iniIndex) + ";" + resType.modObj.first + ";" + resType.modObj.second;
        group.addResource(key, std::move(member));
    }


    const std::vector<std::shared_ptr<VGMergeGroupResource>>& VGMergeGroupResBuilder::groups() const {
        return groups_;
    }
}
