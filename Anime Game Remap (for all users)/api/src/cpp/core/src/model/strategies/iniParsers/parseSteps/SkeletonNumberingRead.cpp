#include "AGRemapCore/model/strategies/iniParsers/parseSteps/SkeletonNumberingRead.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>

#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"

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
    SkeletonNumberingRead::SkeletonNumberingRead(std::vector<SkeletonNumberingProbe::Numbering> numberings,
                                                 std::unordered_map<long long, std::array<double, 3>> centroids,
                                                 std::string positionResource, std::string blendResource,
                                                 std::string vertexVGResource, std::string requiredKey, ObjFilter objs):
        numberings(std::move(numberings)), centroids(std::move(centroids)), positionResource(std::move(positionResource)),
        blendResource(std::move(blendResource)), vertexVGResource(std::move(vertexVGResource)), requiredKey(std::move(requiredKey)),
        objs(std::move(objs)) {}


    void SkeletonNumberingRead::read(Context& ctx, const SectionTargets& targets, IniParseData& data) {
        if (!ctx.hasIni() || numberings.size() < 2 || centroids.empty()) {
            return;
        }

        bool carries = requiredKey.empty();
        for (const auto& [obj, sections] : targets) {
            if (carries || !selects(objs, obj)) {
                continue;
            }

            for (const std::string& name : sections) {
                const Section* section = ctx.getSection(name);
                carries = carries || (section != nullptr && firstVal(*section, requiredKey).has_value());
            }
        }

        const std::optional<std::string> position = data.resourceFile(positionResource);
        const std::optional<std::string> blend = data.resourceFile(blendResource);
        if (!carries || !position.has_value() || !blend.has_value()) {
            return;
        }

        std::optional<std::string> vertexVG;
        if (!vertexVGResource.empty()) {
            std::string wanted = StringTools::toLower(vertexVGResource);
            wanted.erase(std::remove(wanted.begin(), wanted.end(), '_'), wanted.end());
            for (const auto& [section, file] : data.resourceFiles()) {
                std::string name = StringTools::toLower(section);
                name.erase(std::remove(name.begin(), name.end(), '_'), name.end());
                if (name.find(wanted) != std::string::npos) {
                    vertexVG = file;
                    break;
                }
            }
        }

        const std::string folder = ctx.iniFolder();
        const auto pathOf = [&folder](const std::string& rel) {
            return FileService::absPathOfRelPath(FileService::iniPathToRel(rel), folder);
        };
        const std::optional<SkeletonNumberingProbe::Result> found = SkeletonNumberingProbe::probe(
            pathOf(*position), pathOf(*blend), vertexVG.has_value() ? std::optional<std::string>(pathOf(*vertexVG)) : std::nullopt,
            numberings, centroids);
        if (!found.has_value()) {
            return;
        }

        std::ostringstream how;
        for (std::size_t n = 0; n < found->medians.size(); ++n) {
            how << (n == 0 ? "" : ", ") << numberings[n].version << " "
                << static_cast<long long>(std::lround(found->medians[n] * 100)) / 100.0;
        }

        const std::string& version = numberings[found->best].version;
        ctx.log("its bone ids are " + version + "'s numbering (" + std::to_string(found->vertices)
                + " vertices on renumbered bones; median distance to the bone by version: " + how.str() + ")");
        data.setBlendVersion(version);
    }
}
