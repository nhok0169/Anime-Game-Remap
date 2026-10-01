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


#include "AGRemapCore/data/IniFixData/TexFxLayout.h"

#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    namespace {
        // Every per-layout family TexFx defines, with the layout its unsuffixed name aliases (Main.ini and
        // ComponentFriendlyBuiltinShaders.ini: `T` runs `.0`, `TN` runs `.1`)
        struct Family {
            const char* name;
            bool bareIsNormalMap;
        };

        const Family Families[] = {
            {"T", false}, {"Transparency", false}, {"C", false}, {"Component", false},
            {"TN", true}, {"TNat", true}, {"TransparencyNatlan", true},
            {"CN", true}, {"CNat", true}, {"ComponentNatlan", true}};

        std::string callOf(const char* family, bool normalMap) {
            return IniKeywords::TexFxFolder + "\\" + family + (normalMap ? ".1" : ".0");
        }

        // The family a call names and the layout it asks for, or nullptr
        const Family* parse(const std::string& value, bool& normalMap) {
            const std::string path = StringTools::toLower(std::string(StringTools::strip(value)));
            const std::string folder = StringTools::toLower(IniKeywords::TexFxFolder + "\\");
            if (path.size() <= folder.size() || path.compare(0, folder.size(), folder) != 0) {
                return nullptr;
            }

            std::string name = path.substr(folder.size());
            std::optional<bool> suffix;
            if (name.size() > 2 && name[name.size() - 2] == '.' && (name.back() == '0' || name.back() == '1')) {
                suffix = (name.back() == '1');
                name = name.substr(0, name.size() - 2);
            }

            for (const Family& family : Families) {
                if (name == StringTools::toLower(family.name)) {
                    normalMap = suffix.value_or(family.bareIsNormalMap);
                    return &family;
                }
            }
            return nullptr;
        }
    }


    std::optional<std::string> TexFxLayout::retarget(const std::string& value, bool normalMap) {
        bool asked = false;
        const Family* family = parse(value, asked);
        if (family == nullptr || asked == normalMap) {
            return std::nullopt;
        }
        return callOf(family->name, normalMap);
    }


    std::vector<std::unique_ptr<RegNewVals<>>> TexFxLayout::switches(bool normalMap) {
        std::vector<std::unique_ptr<RegNewVals<>>> edits;
        for (const Family& family : Families) {
            const std::string target = callOf(family.name, normalMap);
            RegNewVals<>::ModTypePredicate matches = [target, normalMap](const std::string& value, const ModType*) {
                const std::optional<std::string> retargeted = TexFxLayout::retarget(value, normalMap);
                return retargeted.has_value() && *retargeted == target;
            };
            edits.push_back(std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                {IniKeywords::Run, RegNewVals<>::NewValSpec(std::make_pair(RegNewVals<>::NewVal(target), std::move(matches)))}}));
        }
        return edits;
    }
}
