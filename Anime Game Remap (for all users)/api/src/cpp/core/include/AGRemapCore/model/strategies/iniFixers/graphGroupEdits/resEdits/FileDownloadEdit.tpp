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

#ifndef AGRemapCore_FileDownloadEdit_TPP
#define AGRemapCore_FileDownloadEdit_TPP

#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/tools/files/FileDownload.h"
#include "FileDownloadEdit.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    FileDownloadCreate<K, V, KeyHash, KeyEqual>::FileDownloadCreate(GraphId resModObj, std::unordered_map<std::string, File> files,
                                                                    ResEditConfig config, std::string resType):
        Base(std::move(resType), std::move(resModObj), std::move(config)), files(std::move(files)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> FileDownloadCreate<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                                const std::string& modName) const {
        (void)resource;
        (void)modName;
        return std::nullopt;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename FileDownloadCreate<K, V, KeyHash, KeyEqual>::Section*
            FileDownloadCreate<K, V, KeyHash, KeyEqual>::buildSection(const std::string& sectionName, const std::string& modName) {
        (void)modName;
        auto found = files.find(sectionName);
        if (found == files.end()) {
            return nullptr;
        }

        // the same section FileAddCreate builds: the caller's keys, then the filename
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
    void FileDownloadCreate<K, V, KeyHash, KeyEqual>::buildResModel(const std::string& resType, const std::string& srcPath,
                                                                    const std::string& fixedPath, const std::string& modName,
                                                                    const std::string& fileKey, Context& ctx) {
        (void)resType;
        (void)fixedPath;
        (void)modName;
        for (const auto& entry : files) {
            if (entry.second.file != srcPath) {
                continue;
            }
            // with an edit, the download goes to the raw file and the edit writes the named one from it --
            // both under the file's key, in that order, so the download is fetched first
            const File& file = entry.second;
            const std::string downloaded = file.texEdit.has_value() ? file.rawFile : srcPath;
            auto resource = std::make_unique<RemapIniDownload>(ctx.iniFolder(), downloaded,
                                                               std::make_unique<FileDownload>(file.url, downloaded));
            resource->logger = ctx.logger();
            ctx.storeResource(fileKey, std::move(resource));

            if (file.texEdit.has_value()) {
                auto edit = std::make_unique<RemapTexEditResource>(ctx.iniFolder(), downloaded, srcPath, *file.texEdit);
                edit->logger = ctx.logger();
                ctx.storeResource(fileKey, std::move(edit));
            }
            return;
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void FileDownloadCreate<K, V, KeyHash, KeyEqual>::clear() {
        Base::clear();
        sections_.clear();
    }
}

#endif
