#include "AGRemapCore/model/strategies/iniParsers/parseSteps/ResourceRead.h"

#include "AGRemapCore/constants/IniKeywords.h"

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
    void ResourceRead::read(Context& ctx, const SectionTargets& targets, IniParseData& data) {
        (void)targets;
        for (const std::string& name : ctx.sectionNames()) {
            const Section* section = ctx.getSection(name);
            if (section == nullptr) {
                continue;
            }

            const std::optional<std::string> file = firstVal(*section, IniKeywords::Filename);
            if (file.has_value()) {
                data.setResourceFile(name, *file);
            }
        }
    }
}
