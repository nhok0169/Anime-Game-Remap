#include "AGRemapCore/model/iniresources/IniResource.h"

#include <utility>

#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    IniResource::IniResource(std::string type, const std::string& iniFolderPath, const std::string& srcPath):
        type(std::move(type)), srcPath(FileService::absPathOfRelPath(srcPath, iniFolderPath)) {}

    IniFixResource::IniFixResource(std::string type, const std::string& iniFolderPath, const std::string& srcPath, const std::string& fixedPath):
        IniResource(std::move(type), iniFolderPath, srcPath), fixedPath(FileService::absPathOfRelPath(fixedPath, iniFolderPath)) {}

    IniGroupedResource::IniGroupedResource(std::string name, std::unordered_map<std::string, std::unique_ptr<IniResource>> resources,
                                            std::function<bool(IniGroupedResource&)> fixFunc, bool isBuilt):
        name(std::move(name)), resources(std::move(resources)), fixFunc(std::move(fixFunc)), isBuilt(isBuilt) {}

    bool IniGroupedResource::_fix() {
        return false;
    }

    bool IniGroupedResource::fix() {
        if (fixFunc) {
            return fixFunc(*this);
        }

        return _fix();
    }

    bool IniGroupedResource::isMissing(const std::unordered_set<std::string>& collected) const {
        for (const std::string& resType : collected) {
            if (!resources.contains(resType)) {
                return true;
            }
        }

        return false;
    }

    void IniGroupedResource::addResource(const std::string& resType, std::unique_ptr<IniResource> resource) {
        resources[resType] = std::move(resource);
    }
}
