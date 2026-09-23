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


#include "AGRemapCore/data/IniFixData/TexRegLayout.h"

#include <functional>

#include "AGRemapCore/data/IniFixData/RegValChecks.h"


namespace AGRemapCore {
    namespace {
        // Mutually exclusive by construction -- see TexRegLayout::byName. Precedence: normal map,
        // then light map, then diffuse.
        bool normalMapNamed(const std::string& val) {
            return RegValChecks::isNormalMap(val);
        }

        bool lightMapNamed(const std::string& val) {
            return RegValChecks::isLightMap(val) && !RegValChecks::isNormalMap(val);
        }

        bool diffuseNamed(const std::string& val) {
            return RegValChecks::isDiffuse(val) && !RegValChecks::isLightMap(val)
                    && !RegValChecks::isNormalMap(val);
        }
    }


    TexRegLayout::Roles TexRegLayout::fixLibraryRoles(bool normalMap) {
        // ORFix's CommandListReference, and NNFix's ReferenceNoNormal below it.
        if (normalMap) {
            return Roles{"ps-t0", "ps-t1", "ps-t2"};
        }

        return Roles{"", "ps-t0", "ps-t1"};
    }


    std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>> TexRegLayout::byName(
            const Roles& roles, const std::vector<std::string>& fromRegs) {
        using Remapped = RemappedKeyData<std::string, std::string>;

        std::vector<std::pair<std::string, std::function<bool(const std::string&)>>> destinations;
        if (!roles.normalMap.empty()) {
            destinations.emplace_back(roles.normalMap, &normalMapNamed);
        }
        if (!roles.diffuse.empty()) {
            destinations.emplace_back(roles.diffuse, &diffuseNamed);
        }
        if (!roles.lightMap.empty()) {
            destinations.emplace_back(roles.lightMap, &lightMapNamed);
        }

        // One rule per register a mod may have bound, each listing every role's destination:
        // IfContentPart::remapKeys consults the rules once per ORIGINAL key in a single pass, so a
        // rotation cannot re-read its own output (the property GIMICharFixer's two-way face swap
        // relies on as well).
        std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>> remaps;
        remaps.reserve(fromRegs.size());

        for (const std::string& from : fromRegs) {
            RemapList<std::string, std::string> targets;
            for (const auto& destination : destinations) {
                const auto check = destination.second;
                targets.push_back(Remapped(destination.first, Remapped::CheckPredicate(
                    [check](const std::string&, const std::string& val) { return check(val); })));
            }

            // keep: a register bound to something that names no role at all stays put
            remaps.emplace_back(from, RegRemap<>::KeyRemapValue(
                KeyRemapData<std::string, std::string>(std::move(targets), true)));
        }

        return remaps;
    }


    std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> TexRegLayout::removeNormalMap(
            const std::vector<std::string>& fromRegs) {
        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> keys;
        keys.reserve(fromRegs.size());

        for (const std::string& reg : fromRegs) {
            keys.emplace_back(reg, RegRemove<>::RemoveKeyCheck(
                [](long long, const std::string& val) { return normalMapNamed(val); }));
        }

        return keys;
    }
}
