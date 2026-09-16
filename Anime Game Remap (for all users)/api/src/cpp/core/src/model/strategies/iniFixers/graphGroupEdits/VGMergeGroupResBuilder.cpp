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


    VGMergeGroupResBuilder::VGMergeGroupResBuilder(std::string name, std::vector<VGMergeGroupConfig> configs,
                                                    IniFile* iniFile):
        name_(std::move(name)), configs_(std::move(configs)),
        config_(configs_.empty() ? VGMergeGroupConfig{} : configs_.front()), iniFile_(iniFile) {}


    IniGroupedResource* VGMergeGroupResBuilder::build() {
        // The config for the group being built. ResGroupCollect calls this once per satisfiable
        // combination, in order, so group N is branch N of a $swapvar'd source -- see configs_.
        // The last config repeats, which covers a source that does not branch at all.
        const VGMergeGroupConfig* config = &config_;
        if (!configs_.empty()) {
            const std::size_t index = std::min(groups_.size(), configs_.size() - 1);
            config = &configs_[index];
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
