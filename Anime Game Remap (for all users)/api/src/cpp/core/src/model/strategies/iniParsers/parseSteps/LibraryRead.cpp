#include "AGRemapCore/model/strategies/iniParsers/parseSteps/LibraryRead.h"

#include <utility>

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
    LibraryRead::LibraryRead(std::vector<Library> libraries, ObjFilter objs):
        libraries(std::move(libraries)), objs(std::move(objs)) {}


    void LibraryRead::read(Context& ctx, const SectionTargets& targets, IniParseData& data) {
        for (const auto& [obj, sections] : targets) {
            if (!selects(objs, obj)) {
                continue;
            }

            for (const std::string& name : sections) {
                const Section* section = ctx.getSection(name);
                if (section == nullptr) {
                    continue;
                }

                for (const ContentPart* content : contentParts(*section)) {
                    for (const auto& item : content->items()) {
                        const std::string key = StringTools::toLower(item.key);
                        const std::string value = StringTools::toLower(item.value);
                        for (const Library& library : libraries) {
                            const std::string text = StringTools::toLower(library.second);
                            if (key.find(text) != std::string::npos || value.find(text) != std::string::npos) {
                                data.addLibrary(obj, library.first);
                            }
                        }
                    }
                }
            }
        }
    }
}
