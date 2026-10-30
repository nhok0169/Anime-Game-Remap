#include "AGRemapCore/model/strategies/iniParsers/parseSteps/DrawRead.h"

#include <algorithm>
#include <functional>
#include <unordered_set>
#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegDrawRangeRemap.h"
#include "AGRemapCore/tools/ListTools.h"
#include "AGRemapCore/tools/StringTools.h"

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
    DrawRead::DrawRead(ObjFilter objs, std::string texRegPrefix, std::vector<std::string> silentLibraries):
        objs(std::move(objs)), texRegPrefix(std::move(texRegPrefix)), silentLibraries(std::move(silentLibraries)) {}


    void DrawRead::read(Context& ctx, const SectionTargets& targets, IniParseData& data) {
        const auto isSilentLibrary = [this](const std::string& section) {
            const std::string lower = StringTools::toLower(section);
            return std::any_of(silentLibraries.begin(), silentLibraries.end(), [&lower](const std::string& library) {
                return lower.find(StringTools::toLower(library)) != std::string::npos;
            });
        };

        // A `run =` this file cannot resolve counts as drawing, unless it names a library that draws nothing
        std::function<bool(const std::string&, std::unordered_set<std::string>&)> mayDraw =
            [&ctx, &mayDraw, &isSilentLibrary](const std::string& name, std::unordered_set<std::string>& seen) -> bool {
                if (!seen.insert(StringTools::toLower(name)).second) {
                    return false;
                }

                const Section* section = ctx.getSection(name);
                if (section == nullptr) {
                    return !isSilentLibrary(name);
                }

                for (const ContentPart* content : contentParts(*section)) {
                    for (const auto& item : content->items()) {
                        const std::string key = StringTools::toLower(std::string(StringTools::strip(item.key)));
                        if (StringTools::startsWith(key, "draw")) {
                            return true;
                        }

                        if (key == StringTools::toLower(IniKeywords::Run)
                                && mayDraw(std::string(StringTools::strip(item.value)), seen)) {
                            return true;
                        }
                    }
                }

                return false;
            };

        const auto skips = [](const Section& section) {
            for (const ContentPart* content : contentParts(section)) {
                for (const std::string& value : content->getVals(IniKeywords::Handling)) {
                    if (StringTools::equalsIgnoreCase(std::string(StringTools::strip(value)), IniKeywords::Skip)) {
                        return true;
                    }
                }
            }
            return false;
        };

        const std::string drawKey = StringTools::toLower(IniKeywords::DrawIndexed);
        const std::string regPrefix = StringTools::toLower(texRegPrefix);
        std::size_t read = 0;
        std::vector<ModObj> silent;
        for (const auto& [obj, sections] : targets) {
            if (!selects(objs, obj) || sections.empty()) {
                continue;
            }

            ++read;
            bool drew = false;
            bool draws = false;
            bool skipped = false;
            std::vector<std::string> late;
            for (const std::string& name : sections) {
                const Section* section = ctx.getSection(name);
                if (section == nullptr) {
                    continue;
                }

                // its draws, and which part each sits in (every part counted, `if` lines too)
                std::size_t partIndex = 0;
                for (const auto& part : section->parts()) {
                    ++partIndex;
                    const auto* content = dynamic_cast<const ContentPart*>(part.get());
                    if (content == nullptr) {
                        continue;
                    }

                    for (const std::string& draw : content->getVals(IniKeywords::DrawIndexed)) {
                        data.addDrawPart(obj, partIndex);
                        if (std::count(draw.begin(), draw.end(), ',') < 2) {
                            continue;
                        }

                        const std::optional<std::vector<long long>> numbers = RegDrawRangeRemap<>::parseRange(draw);
                        if (numbers.has_value() && numbers->size() >= 3) {
                            data.addDrawRange(obj, {(*numbers)[0], (*numbers)[1]});
                        } else {
                            // a dropped draw range is a piece of the mesh that stops being remapped
                            ctx.log("could not read the draw range `" + std::string(StringTools::strip(draw)) + "` in ["
                                    + name + "], so it is not remapped");
                        }
                    }
                }

                // the texture registers it binds only after it has drawn
                for (const ContentPart* content : contentParts(*section)) {
                    for (const auto& item : content->items()) {
                        const std::string key = StringTools::toLower(std::string(StringTools::strip(item.key)));
                        if (key == drawKey) {
                            drew = true;
                        } else if (drew && StringTools::startsWith(key, regPrefix)) {
                            ListTools::pushDistinct(late, key);
                        }
                    }
                }

                std::unordered_set<std::string> seen;
                draws = draws || mayDraw(name, seen);
                skipped = skipped || skips(*section);
            }

            if (!late.empty() && !data.replacesTexturesByHash()) {
                data.setLateBindings(obj, std::move(late));
            }

            if (!draws && skipped) {
                silent.push_back(obj);
            }
        }

        if (silent.size() < read) {
            for (const ModObj& obj : silent) {
                data.addUndrawn(obj);
            }
        }
    }
}
