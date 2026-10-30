#include "AGRemapCore/model/strategies/iniParsers/parseSteps/TextureOverrideRead.h"

#include <unordered_set>
#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
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
    TextureOverrideRead::TextureOverrideRead(ObjFilter draws): draws(std::move(draws)) {}


    void TextureOverrideRead::read(Context& ctx, const SectionTargets& targets, IniParseData& data) {
        std::unordered_set<std::string> drawSections;
        if (draws) {
            for (const auto& [obj, sections] : targets) {
                if (draws(obj)) {
                    drawSections.insert(sections.begin(), sections.end());
                }
            }
        }

        for (const std::string& name : ctx.sectionNames()) {
            const Section* section = ctx.getSection(name);
            if (section == nullptr || !StringTools::startsWith(name, IniKeywords::TextureOverride) || drawSections.count(name) > 0) {
                continue;
            }

            bool hasHash = false;
            bool hasThis = false;
            for (const ContentPart* content : contentParts(*section)) {
                hasHash = hasHash || content->count(IniKeywords::Hash) > 0;
                hasThis = hasThis || content->count(IniKeywords::This) > 0;
            }

            if (hasHash && hasThis) {
                data.setReplacesTexturesByHash(true);
                return;
            }
        }
    }
}
