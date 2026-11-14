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

#include "AGRemapCore/model/files/IniSharedState.h"

#include <algorithm>
#include <filesystem>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    struct IniSharedState::Entry {
        std::unique_ptr<IniFile> ini;
        std::optional<std::string> ns;

        // every key, value and `if` test of the file, lowercased: where a reference to a namespace is written
        std::string refText;

        // the resource files, as resolved, and their keys (#keyOf) to compare
        std::vector<std::string> files;
        std::set<std::string> fileKeys;
    };

    IniSharedState::IniSharedState() = default;
    IniSharedState::~IniSharedState() = default;

    std::string IniSharedState::keyOf(const std::string& path) {
        std::filesystem::path normal = FileService::strToPath(path).lexically_normal();
        normal.make_preferred();
        return StringTools::toLower(FileService::pathToStr(normal));
    }

    void IniSharedState::add(const std::string& path) {
        auto entry = std::make_unique<Entry>();
        entry->ini = std::make_unique<IniFile>(path);
        entry->ini->readFileLines();
        entry->ns = entry->ini->getNamespace();

        const std::string folder = FileService::parentOf(path);
        for (const auto& [name, section] : entry->ini->getIfTemplates()) {
            (void)name;
            if (section == nullptr) {
                continue;
            }

            const std::optional<std::string> file = section->firstVal(IniKeywords::Filename);
            if (file.has_value() && !file->empty()) {
                std::string resolved = FileService::absPathOfRelPath(*file, folder);
                if (entry->fileKeys.insert(keyOf(resolved)).second) {
                    entry->files.push_back(std::move(resolved));
                }
            }

            for (const auto& part : section->parts()) {
                if (const auto* pred = dynamic_cast<const IfPredPart*>(part.get())) {
                    entry->refText += StringTools::toLower(pred->src) + "\n";
                    continue;
                }
                if (const auto* content = dynamic_cast<const IfTemplate<std::string, std::string>::ContentPart*>(part.get())) {
                    for (const auto& [key, value] : content->entries()) {
                        entry->refText += StringTools::toLower(key) + " = " + StringTools::toLower(value) + "\n";
                    }
                }
            }
        }

        const std::string key = keyOf(path);
        if (entries_.count(key) == 0) {
            files_.push_back(path);
        }
        entries_[key] = std::move(entry);
    }

    void IniSharedState::clear() {
        files_.clear();
        entries_.clear();
    }

    const std::vector<std::string>& IniSharedState::files() const {
        return files_;
    }

    const IniSharedState::Entry* IniSharedState::entryOf(const std::string& path) const {
        auto found = entries_.find(keyOf(path));
        return found == entries_.end() ? nullptr : found->second.get();
    }

    IniFile* IniSharedState::file(const std::string& path) const {
        const Entry* entry = entryOf(path);
        return entry == nullptr ? nullptr : entry->ini.get();
    }

    std::optional<std::string> IniSharedState::namespaceOf(const std::string& path) const {
        const Entry* entry = entryOf(path);
        return entry == nullptr ? std::nullopt : entry->ns;
    }

    const std::vector<std::string>& IniSharedState::resourceFiles(const std::string& path) const {
        static const std::vector<std::string> none;
        const Entry* entry = entryOf(path);
        return entry == nullptr ? none : entry->files;
    }

    std::vector<std::string> IniSharedState::linked(const std::string& path) const {
        std::vector<std::string> result;
        const Entry* self = entryOf(path);
        if (self == nullptr) {
            return result;
        }

        // `$\X\var`, `CommandList\X\name`, `Resource\X\name`: the namespace between backslashes
        const auto references = [](const Entry& from, const Entry& to) {
            return to.ns.has_value() && !to.ns->empty()
                && from.refText.find("\\" + StringTools::toLower(*to.ns) + "\\") != std::string::npos;
        };
        const auto shareFiles = [](const Entry& a, const Entry& b) {
            return std::any_of(a.fileKeys.begin(), a.fileKeys.end(), [&b](const std::string& file) { return b.fileKeys.count(file) > 0; });
        };

        for (const std::string& other : files_) {
            const Entry* entry = entryOf(other);
            if (entry == nullptr || entry == self) {
                continue;
            }
            if (references(*self, *entry) || references(*entry, *self) || shareFiles(*self, *entry)) {
                result.push_back(other);
            }
        }
        return result;
    }
}
