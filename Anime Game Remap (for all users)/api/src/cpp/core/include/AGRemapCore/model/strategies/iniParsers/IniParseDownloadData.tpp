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

#ifndef AGRemapCore_IniParseDownloadData_TPP
#define AGRemapCore_IniParseDownloadData_TPP

#include "IniParseDownloadData.h"

#include "AGRemapCore/constants/IniKeywords.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    DownloadData<K, V, KeyHash, KeyEqual>::DownloadData(std::string name, std::unique_ptr<FileDownload> download, DownloadConfig config,
                                                          bool refToSection, std::vector<std::pair<K, V>> downloadRefKVPs,
                                                          std::vector<std::pair<K, V>> resourceKVPs):
        name_(std::move(name)), download_(std::move(download)), config_(std::move(config)), refToSection_(refToSection),
        downloadRefKVPs_(std::move(downloadRefKVPs)), resourceKVPs_(std::move(resourceKVPs)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string DownloadData<K, V, KeyHash, KeyEqual>::name() const {
        return name_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool DownloadData<K, V, KeyHash, KeyEqual>::refToSection() const {
        return refToSection_;
    }



    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const FileDownload* DownloadData<K, V, KeyHash, KeyEqual>::download() const {
        return download_.get();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void DownloadData<K, V, KeyHash, KeyEqual>::addToPart(ContentPart& part, const K& key, const V& val) {
        part.addKVP(key, val);

        if (!downloadRefKVPs_.empty()) {
            part.addKVPs(downloadRefKVPs_);
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void DownloadData<K, V, KeyHash, KeyEqual>::addToSection(Section& section, const K& key, const V& val) {
        section.addKVPToFront(key, val);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<std::pair<K, V>>& DownloadData<K, V, KeyHash, KeyEqual>::downloadRefKVPs() const {
        return downloadRefKVPs_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<std::pair<K, V>>& DownloadData<K, V, KeyHash, KeyEqual>::resourceKVPs() const {
        return resourceKVPs_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::pair<K, V>> DownloadData<K, V, KeyHash, KeyEqual>::blendRefKVPs(long long vertexCount) {
        return {{K(IniKeywords::Handling), V("skip")}, {K(IniKeywords::Draw), V(std::to_string(vertexCount) + ",0")}};
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<typename DownloadData<K, V, KeyHash, KeyEqual>::Section> DownloadData<K, V, KeyHash, KeyEqual>::createResSection(
            const std::string& sectionName) const {
        std::vector<std::pair<K, V>> src;
        src.emplace_back(config_.filenameKey, config_.valOfPath(download_ == nullptr ? std::string() : download_->filename));

        auto contentPart = std::make_unique<ContentPart>(src, 0);
        if (!resourceKVPs_.empty()) {
            contentPart->addKVPsToFront(resourceKVPs_);
        }

        std::vector<std::unique_ptr<IfTemplatePart>> parts;
        parts.push_back(std::move(contentPart));

        return std::make_unique<Section>(std::move(parts), config_.runConfig, sectionName);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<IniResource> DownloadData<K, V, KeyHash, KeyEqual>::createFileDownload(const std::string& iniFolder) const {
        if (download_ == nullptr) {
            return nullptr;
        }

        // A fresh FileDownload rather than a transfer of this object's own: RemapIniDownload takes
        // ownership of what it's given, and one DownloadData is reused across several .ini files (one
        // per mod object/register row of a ModType's download table). Rebuilt field-by-field rather
        // than copy-constructed so a FileDownload subclass isn't silently sliced -- what
        // RemapIniDownload needs from it is exactly these three values.
        auto download = std::make_unique<FileDownload>(download_->url, download_->filename, download_->cache);
        return std::make_unique<RemapIniDownload>(iniFolder, download_->filename, std::move(download));
    }
}

#endif
