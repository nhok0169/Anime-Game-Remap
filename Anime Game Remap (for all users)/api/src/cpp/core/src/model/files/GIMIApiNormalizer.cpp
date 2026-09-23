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


#include "AGRemapCore/model/files/GIMIApiNormalizer.h"

#include <set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    namespace {
        using Section = IfTemplate<std::string, std::string>;
        using ContentPart = IfContentPart<std::string, std::string>;

        // GIMI's names -> the traditional register each is read from. Two layouts, as the
        // traditional API has: with a normal map, ORFix's own Reference (normal map / diffuse /
        // light map at ps-t0/1/2); without one, NNFix's (diffuse / light map at ps-t0/1) --
        // SetTextures leaves the normal map null when none is set, which is NNFix's reading, and a
        // plain-shader slot (CitlaliWhisperofStars' Body D) is written that way.
        const std::vector<std::pair<std::string, std::string>>& gimiRegisters(bool normalMap = true) {
            static const std::vector<std::pair<std::string, std::string>> normal = {
                {IniKeywords::GIMINormalMap, "ps-t0"},
                {IniKeywords::GIMIDiffuse, "ps-t1"},
                {IniKeywords::GIMILightMap, "ps-t2"}};
            static const std::vector<std::pair<std::string, std::string>> plain = {
                {IniKeywords::GIMIDiffuse, "ps-t0"},
                {IniKeywords::GIMILightMap, "ps-t1"}};
            return normalMap ? normal : plain;
        }

        bool isSetTexturesCall(const std::string& value) {
            return StringTools::equalsIgnoreCase(StringTools::strip(value), IniKeywords::GIMISetTexturesPath);
        }

        // The register a key names, or empty -- ignoring case, as 3DMigoto does.
        std::string registerOf(const std::string& key, bool normalMap = true) {
            for (const auto& entry : gimiRegisters(normalMap)) {
                if (StringTools::equalsIgnoreCase(StringTools::strip(key), entry.first)) {
                    return entry.second;
                }
            }
            return "";
        }

        // `ref X` -> `X`: a texture register is bound by reference anyway, and every reader in the
        // library looks the value up as a section name.
        std::string withoutRef(const std::string& value) {
            std::string stripped(StringTools::strip(value));
            if (StringTools::startsWith(StringTools::toLower(stripped), "ref ")) {
                return std::string(StringTools::strip(std::string_view(stripped).substr(4)));
            }
            return stripped;
        }
    }


    bool GIMIApiNormalizer::normalize(Section& section, const std::string& sectionName) {
        bool callsSetTextures = false;
        for (const auto& part : section.parts()) {
            auto* contentPart = dynamic_cast<ContentPart*>(part.get());
            if (contentPart == nullptr) {
                continue;
            }
            for (const std::string& value : contentPart->getVals(IniKeywords::Run)) {
                if (isSetTexturesCall(value)) {
                    callsSetTextures = true;
                    break;
                }
            }
            if (callsSetTextures) {
                break;
            }
        }

        if (!callsSetTextures) {
            return false;
        }

        // The layout, per section: a normal map set anywhere in it means ORFix's.
        bool normalMap = false;
        for (const auto& part : section.parts()) {
            auto* contentPart = dynamic_cast<ContentPart*>(part.get());
            if (contentPart == nullptr) {
                continue;
            }
            for (const auto& kvp : contentPart->entries()) {
                if (StringTools::equalsIgnoreCase(StringTools::strip(kvp.first), IniKeywords::GIMINormalMap)) {
                    normalMap = true;
                }
            }
        }
        const std::string fixPath = normalMap ? IniKeywords::ORFixPath : IniKeywords::NNFixPath;

        using NewVals = RegNewVals<>;

        for (const auto& part : section.parts()) {
            auto* contentPart = dynamic_cast<ContentPart*>(part.get());
            if (contentPart == nullptr) {
                continue;
            }

            // Every spelling of every GIMI key this part carries, and each distinct `ref` value.
            std::set<std::string> keys;
            std::set<std::pair<std::string, std::string>> refValues;
            for (const auto& kvp : contentPart->entries()) {
                if (registerOf(kvp.first, normalMap).empty()) {
                    continue;
                }
                keys.insert(kvp.first);
                if (withoutRef(kvp.second) != StringTools::strip(kvp.second)) {
                    refValues.insert(kvp);
                }
            }

            // 1. the values: one conditional replacement per distinct `ref` value.
            for (const auto& kvp : refValues) {
                const std::string old = kvp.second;
                NewVals dropRef({{kvp.first, NewVals::NewValSpec(std::pair<NewVals::NewVal, NewVals::ModTypePredicate>(
                    NewVals::NewVal(withoutRef(old)),
                    [old](const std::string& value, const ModType*) { return value == old; }))}});
                dropRef.edit(*contentPart, sectionName);
            }

            // 2. the keys.
            if (!keys.empty()) {
                std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>> remap;
                for (const std::string& key : keys) {
                    RemapList<std::string, std::string> to;
                    to.push_back(registerOf(key, normalMap));
                    remap.emplace_back(key, RegRemap<>::KeyRemapValue(std::move(to)));
                }
                RegRemap<>(std::move(remap)).edit(*contentPart, sectionName);
            }

            // 3. the call.
            NewVals toORFix({{IniKeywords::Run, NewVals::NewValSpec(std::pair<NewVals::NewVal, NewVals::ModTypePredicate>(
                NewVals::NewVal(fixPath),
                [](const std::string& value, const ModType*) { return isSetTexturesCall(value); }))}});
            toORFix.edit(*contentPart, sectionName);
        }

        section.rebuild();
        return true;
    }


    std::size_t GIMIApiNormalizer::normalize(Sections& sections) {
        std::size_t count = 0;
        for (auto it = sections.begin(); it != sections.end(); ++it) {
            if (it->second != nullptr && normalize(*it.value(), it->first)) {
                ++count;
            }
        }
        return count;
    }
}
