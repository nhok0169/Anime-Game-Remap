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

#ifndef AGRemapCore_FileAddEdit_TPP
#define AGRemapCore_FileAddEdit_TPP

#include <functional>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"

#include "FileAddEdit.h"


namespace AGRemapCore {
    // FileAddCreate
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    FileAddCreate<K, V, KeyHash, KeyEqual>::FileAddCreate(GraphId resModObj, std::unordered_map<std::string, File> files,
                                                          ResEditConfig config, std::string resType):
        Base(std::move(resType), std::move(resModObj), std::move(config)), files(std::move(files)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> FileAddCreate<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                           const std::string& modName) const {
        (void)resource;
        (void)modName;
        return std::nullopt;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename FileAddCreate<K, V, KeyHash, KeyEqual>::Section*
            FileAddCreate<K, V, KeyHash, KeyEqual>::buildSection(const std::string& sectionName, const std::string& modName) {
        (void)modName;

        auto found = files.find(sectionName);
        if (found == files.end()) {
            return nullptr;
        }

        std::vector<std::pair<K, V>> src = found->second.keys;
        src.emplace_back(this->config.filenameKey, this->config.valOfFile(found->second.file));

        std::vector<std::unique_ptr<IfTemplatePart>> parts;
        parts.push_back(std::make_unique<IfContentPart<K, V, KeyHash, KeyEqual>>(src, 0));

        IfTemplateRunConfig<K, V> runConfig{
            IniKeywords::Run,
            [](const V& val) { return val; },
            [](const std::string& name) { return name; }
        };

        sections_.push_back(std::make_unique<Section>(std::move(parts), std::move(runConfig), sectionName));
        return sections_.back().get();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void FileAddCreate<K, V, KeyHash, KeyEqual>::buildResModel(const std::string& resType, const std::string& srcPath,
                                                               const std::string& fixedPath, const std::string& modName,
                                                               const std::string& fileKey, Context& ctx) {
        (void)fixedPath;
        (void)modName;

        for (const auto& entry : files) {
            if (entry.second.file != srcPath) {
                continue;
            }

            auto resource = std::make_unique<RemapFileAddResource>(ctx.iniFolder(), srcPath, entry.second.bytes, resType);
            resource->makeBytes = entry.second.makeBytes;
            resource->logger = ctx.logger();
            ctx.storeResource(fileKey, std::move(resource));
            return;
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void FileAddCreate<K, V, KeyHash, KeyEqual>::clear() {
        Base::clear();

        sections_.clear();
    }


    // FileRefAdd
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    FileRefAdd<K, V, KeyHash, KeyEqual>::FileRefAdd(std::unordered_map<std::string, std::vector<std::uint8_t>> files,
                                                    ResEditConfig config, std::string resType):
        Base(std::move(resType), GraphId(), std::move(config)), files(std::move(files)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> FileRefAdd<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                        const std::string& modName) const {
        (void)resource;
        (void)modName;
        return std::nullopt;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename FileRefAdd<K, V, KeyHash, KeyEqual>::GraphGroups& FileRefAdd<K, V, KeyHash, KeyEqual>::buildResources(
            const CollectedSections& collectedSections, Context& ctx, GraphGroups& graphGroups, const std::string& modName,
            const ResourceFilter& resourceFilter, bool copySections) {
        (void)modName;
        (void)copySections;

        for (const auto& entry : collectedSections) {
            const std::string& file = entry.second;
            auto found = files.find(file);
            if (found == files.end() || (resourceFilter && !resourceFilter(file, file))) {
                continue;
            }

            auto resource = std::make_unique<RemapFileAddResource>(ctx.iniFolder(), file, found->second, this->resType);
            resource->logger = ctx.logger();
            ctx.storeResource(file, std::move(resource));
        }

        return graphGroups;
    }
}

#endif
