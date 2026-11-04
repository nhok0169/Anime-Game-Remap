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

#include "AGRemapCore/model/strategies/iniFixers/ShaderFilterTags.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/tools/NumTools.h"


namespace AGRemapCore {
    bool ShaderFilterTags::add(const std::string& shaderHash, const std::string& filterIndex) {
        if (filterIndexOf(shaderHash).has_value()) {
            return false;
        }

        tags_.push_back(Tag{shaderHash, filterIndex});
        return true;
    }

    void ShaderFilterTags::addNumbered(const std::vector<std::string>& shaderHashes, const std::map<std::string, std::string>& given,
                                       double base, double step) {
        std::size_t next = 0;
        for (const std::string& shader : shaderHashes) {
            if (filterIndexOf(shader).has_value()) {
                continue;
            }
            const auto value = given.find(shader);
            if (value != given.end()) {
                add(shader, value->second);
            } else {
                add(shader, NumTools::formatDouble(base + step * static_cast<double>(next)));
                ++next;
            }
        }
    }

    std::optional<std::string> ShaderFilterTags::filterIndexOf(const std::string& shaderHash) const {
        const auto found = std::find_if(tags_.begin(), tags_.end(), [&shaderHash](const Tag& tag) { return tag.shaderHash == shaderHash; });
        return found == tags_.end() ? std::nullopt : std::optional<std::string>(found->filterIndex);
    }

    const std::vector<ShaderFilterTags::Tag>& ShaderFilterTags::tags() const {
        return tags_;
    }

    std::string ShaderFilterTags::condition(const std::string& reg, const std::vector<std::string>& shaderHashes, bool none) const {
        std::vector<std::string> named;
        std::string out;
        for (const std::string& hash : shaderHashes) {
            const std::optional<std::string> index = filterIndexOf(hash);
            if (!index.has_value() || std::find(named.begin(), named.end(), hash) != named.end()) {
                continue;
            }

            named.push_back(hash);
            out += (out.empty() ? "" : (none ? " && " : " || ")) + reg + (none ? " != " : " == ") + *index;
        }
        return out;
    }

    std::unique_ptr<IfTemplate<std::string, std::string>> ShaderFilterTags::section(std::size_t index, const std::string& name) const {
        if (index >= tags_.size()) {
            throw std::out_of_range("ShaderFilterTags: no tagged shader number " + std::to_string(index));
        }

        const Tag& tag = tags_[index];
        std::vector<std::unique_ptr<IfTemplatePart>> parts;
        parts.push_back(std::make_unique<IfContentPart<std::string, std::string>>(
            std::vector<std::pair<std::string, std::string>>{{IniKeywords::Hash, tag.shaderHash}, {IniKeywords::FilterIndex, tag.filterIndex}}, 0));

        // `run =` names the section it calls, as every section a fix writes of its own
        IfTemplateRunConfig<std::string, std::string> runConfig{
            IniKeywords::Run,
            [](const std::string& val) { return val; },
            [](const std::string& section) { return section; }
        };
        return std::make_unique<IfTemplate<std::string, std::string>>(std::move(parts), std::move(runConfig), name);
    }
}
