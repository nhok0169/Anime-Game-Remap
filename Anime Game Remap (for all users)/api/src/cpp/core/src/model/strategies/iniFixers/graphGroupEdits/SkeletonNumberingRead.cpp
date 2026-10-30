#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/SkeletonNumberingRead.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/view/BaseLogger.h"

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


namespace AGRemapCore {
    namespace {
        // The first value of 'key' in the section's own parts, stripped, without following `run =`
        std::optional<std::string> firstVal(const IfTemplate<std::string, std::string>& section, const std::string& key) {
            for (const auto& part : section.parts()) {
                const auto* content = dynamic_cast<const IfTemplate<std::string, std::string>::ContentPart*>(part.get());
                if (content == nullptr) {
                    continue;
                }

                std::vector<std::string> vals = content->getVals(key);
                if (!vals.empty()) {
                    return std::string(StringTools::strip(vals.front()));
                }
            }

            return std::nullopt;
        }
    }


    SkeletonNumberingRead::SkeletonNumberingRead(std::vector<SkeletonNumberingProbe::Numbering> numberings,
                                                 std::unordered_map<long long, std::array<double, 3>> centroids,
                                                 std::string positionResource, std::string blendResource,
                                                 std::string vertexVGResource, std::string requiredKey, ObjFilter objs):
        numberings(std::move(numberings)), centroids(std::move(centroids)), positionResource(std::move(positionResource)),
        blendResource(std::move(blendResource)), vertexVGResource(std::move(vertexVGResource)), requiredKey(std::move(requiredKey)),
        objs(std::move(objs)) {}


    const std::optional<std::string>& SkeletonNumberingRead::version() const {
        return version_;
    }


    void SkeletonNumberingRead::clear() {
        version_.reset();
    }


    SkeletonNumberingRead::GraphGroups& SkeletonNumberingRead::edit(GraphGroups& graphGroups, const ModType* modType,
                                                                    const std::string& modName) {
        (void)modType;
        (void)modName;
        clear();
        return graphGroups;
    }


    SkeletonNumberingRead::GraphGroups& SkeletonNumberingRead::editFromIni(GraphGroups& graphGroups, IniFile* ini, const ModType* modType,
                                                                           const std::string& modName) {
        (void)modType;
        (void)modName;
        clear();
        if (ini == nullptr || numberings.size() < 2 || centroids.empty()) {
            return graphGroups;
        }

        bool carries = requiredKey.empty();
        for (const auto& obj : graphGroups.size() > 0 ? graphGroups.modObjs(0) : std::vector<ModObj>{}) {
            const Graph* graph = (carries || (objs && !objs(obj))) ? nullptr : graphGroups.getGraph(0, obj);
            for (const std::string& root : graph != nullptr ? graph->roots() : std::vector<std::string>{}) {
                const auto* section = ini->getSection(root);
                carries = carries || (section != nullptr && firstVal(*section, requiredKey).has_value());
            }
        }

        // every resource section's file, by its lowercased name -- the first declaration of a name wins
        std::map<std::string, std::string> fileOf;
        for (const auto& entry : ini->getIfTemplates()) {
            if (entry.second == nullptr) {
                continue;
            }

            const std::optional<std::string> file = firstVal(*entry.second, IniKeywords::Filename);
            if (file.has_value()) {
                fileOf.emplace(StringTools::toLower(entry.first), *file);
            }
        }

        const auto position = fileOf.find(StringTools::toLower(positionResource));
        const auto blend = fileOf.find(StringTools::toLower(blendResource));
        if (!carries || position == fileOf.end() || blend == fileOf.end()) {
            return graphGroups;
        }

        std::optional<std::string> vertexVG;
        if (!vertexVGResource.empty()) {
            std::string wanted = StringTools::toLower(vertexVGResource);
            wanted.erase(std::remove(wanted.begin(), wanted.end(), '_'), wanted.end());
            for (const auto& [section, file] : fileOf) {
                std::string name = section;
                name.erase(std::remove(name.begin(), name.end(), '_'), name.end());
                if (name.find(wanted) != std::string::npos) {
                    vertexVG = file;
                    break;
                }
            }
        }

        const std::string folder = ini->getFolder();
        const auto pathOf = [&folder](const std::string& rel) {
            return FileService::absPathOfRelPath(FileService::iniPathToRel(rel), folder);
        };
        const std::optional<SkeletonNumberingProbe::Result> found = SkeletonNumberingProbe::probe(
            pathOf(position->second), pathOf(blend->second),
            vertexVG.has_value() ? std::optional<std::string>(pathOf(*vertexVG)) : std::nullopt, numberings, centroids);
        if (!found.has_value()) {
            return graphGroups;
        }

        std::ostringstream how;
        for (std::size_t n = 0; n < found->medians.size(); ++n) {
            how << (n == 0 ? "" : ", ") << numberings[n].version << " "
                << static_cast<long long>(std::lround(found->medians[n] * 100)) / 100.0;
        }

        version_ = numberings[found->best].version;
        if (ini->logger != nullptr) {
            ini->logger->log("its bone ids are " + *version_ + "'s numbering (" + std::to_string(found->vertices)
                             + " vertices on renumbered bones; median distance to the bone by version: " + how.str() + ")");
        }
        return graphGroups;
    }


    void SkeletonNumberingRead::fileInto(IniParseData& data) const {
        if (version_.has_value()) {
            data.setBlendVersion(*version_);
        }
    }
}
