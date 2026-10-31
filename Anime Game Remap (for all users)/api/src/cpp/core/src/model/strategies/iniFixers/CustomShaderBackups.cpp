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

#include "AGRemapCore/model/strategies/iniFixers/CustomShaderBackups.h"

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    CustomShaderBackups::CustomShaderBackups(BackupName backupName): backupName(std::move(backupName)) {}

    bool CustomShaderBackups::bindsRegister(const std::string& key) {
        const std::string k = StringTools::toLower(StringTools::strip(key));
        if (k.size() < 4 || k[2] != '-' || (k.compare(0, 2, "ps") != 0 && k.compare(0, 2, "vs") != 0)) {
            return false;
        }

        std::size_t digits = 3;
        if (k.compare(3, 2, "cb") == 0) {
            digits = 5;
        } else if (k[3] == 't') {
            digits = 4;
        } else {
            return false;
        }
        return digits < k.size() && k.find_first_not_of("0123456789", digits) == std::string::npos;
    }

    CustomShaderBackups::KVPs CustomShaderBackups::wrap(const KVPs& runs, const KVPs& lines) {
        std::set<std::string> regs;
        for (const auto& line : lines) {
            if (bindsRegister(line.first)) {
                regs.insert(StringTools::toLower(StringTools::strip(line.first)));
            }
        }

        KVPs result;
        for (const std::string& reg : regs) {
            result.emplace_back(backupName(reg), IniKeywords::Ref + " " + reg);
            registers_.insert(reg);
        }
        result.insert(result.end(), runs.begin(), runs.end());
        for (const std::string& reg : regs) {
            result.emplace_back(reg, IniKeywords::Ref + " " + backupName(reg));
        }
        return result;
    }

    std::vector<std::string> CustomShaderBackups::registers() const {
        return std::vector<std::string>(registers_.begin(), registers_.end());
    }

    std::unique_ptr<IfTemplate<std::string, std::string>> CustomShaderBackups::section(const std::string& reg) const {
        // `run =` names the section it calls, as every section a fix writes of its own
        IfTemplateRunConfig<std::string, std::string> runConfig{
            IniKeywords::Run,
            [](const std::string& val) { return val; },
            [](const std::string& name) { return name; }
        };
        return std::make_unique<IfTemplate<std::string, std::string>>(std::vector<std::unique_ptr<IfTemplatePart>>(),
                                                                      std::move(runConfig), backupName(reg));
    }

    void CustomShaderBackups::clear() {
        registers_.clear();
    }
}
