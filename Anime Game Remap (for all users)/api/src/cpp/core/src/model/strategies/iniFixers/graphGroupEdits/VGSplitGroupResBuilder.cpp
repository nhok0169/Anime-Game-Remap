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

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/VGSplitGroupResBuilder.h"

#include <utility>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iniresources/IniResource.h"


namespace AGRemapCore {

    VGSplitGroupResBuilder::VGSplitGroupResBuilder(std::string name, VGSplitGroupConfig config, IniFile* iniFile):
        name_(std::move(name)), config_(std::move(config)), iniFile_(iniFile) {}


    IniGroupedResource* VGSplitGroupResBuilder::build() {
        auto group = std::make_shared<VGSplitGroupResource>(
            name_, std::unordered_map<std::string, std::unique_ptr<IniResource>>{}, config_, nullptr, /*isBuilt*/ false);
        groups_.push_back(group);
        return group.get();
    }


    void VGSplitGroupResBuilder::store(IniGroupedResource& resource) {
        if (iniFile_ == nullptr) {
            return;
        }

        for (const std::shared_ptr<VGSplitGroupResource>& group : groups_) {
            if (group.get() == &resource) {
                iniFile_->getGroupedResources().push_back(group);
                return;
            }
        }
    }


    void VGSplitGroupResBuilder::addResource(IniGroupedResource& group, const GraphId& resType, IniResource& resource) {
        // A copy -- see the class's own note. The paths are already absolute, so the folder the
        // constructor resolves against is irrelevant; the type is what fixVGSplitGroup tells the
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


    const std::vector<std::shared_ptr<VGSplitGroupResource>>& VGSplitGroupResBuilder::groups() const {
        return groups_;
    }
}
