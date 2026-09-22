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

#include "AGRemapCore/constants/WWMIBuilder.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniRemoveBuilderData.h"
#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/assets/Indices.h"
#include "AGRemapCore/model/assets/IndexCounts.h"
#include "AGRemapCore/model/assets/ShapeKeyChecksums.h"
#include "AGRemapCore/model/assets/VGCounts.h"
#include "AGRemapCore/model/assets/VGOffsets.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"


namespace AGRemapCore {
    namespace {
        // The three table-backed builders every WuWa ModType shares -- the same three tables the GI
        // mod types read (GIBuilder.cpp has the reasoning; the WuWa rows there are stubs today), so
        // one row per character per table is all a new WuWa character adds. Function-local statics
        // for the same init-order reason as GIBuilder's.
        const std::shared_ptr<IniParseBuilder>& wwmiIniParseBuilder() {
            static const std::shared_ptr<IniParseBuilder> builder =
                std::make_shared<IniParseBuilder>(IniParseBuilderData::repo());
            return builder;
        }

        const std::shared_ptr<IniFixBuilder>& wwmiIniFixBuilder() {
            static const std::shared_ptr<IniFixBuilder> builder =
                std::make_shared<IniFixBuilder>(IniFixBuilderData::repo());
            return builder;
        }

        const std::shared_ptr<IniRemoveBuilder>& wwmiIniRemoveBuilder() {
            static const std::shared_ptr<IniRemoveBuilder> builder =
                std::make_shared<IniRemoveBuilder>(IniRemoveBuilderData::repo());
            return builder;
        }

        // The remap graph, as GIBuilder.cpp's makeRemapMap builds it: {ownName -> [targetNames]},
        // where an absent key means "no targets". A WuWa character is one merged skeleton, so there
        // are no component names to file it under as well.
        std::unordered_map<std::string, std::vector<std::string>> makeRemapMap(const std::string& name, const std::vector<ModTypeId>& targets) {
            if (targets.empty()) {
                return {};
            }

            std::vector<std::string> targetNames;
            targetNames.reserve(targets.size());
            for (ModTypeId target : targets) {
                targetNames.push_back(ModTypeIdTools::getName(target));
            }

            std::unordered_map<std::string, std::vector<std::string>> result;
            result.emplace(name, targetNames);
            return result;
        }

        // Builds one WuWa ModType. Every asset table gets the same remap graph: a WWMI fixer
        // replaces the hash, the four per-slot values and the shape-key checksum the same way,
        // reverse-then-forward through ModMappedAssets::replace.
        ModType makeWWMIModType(ModTypeId modTypeId, std::vector<std::string> aliases = {}) {
            const std::string name = ModTypeIdTools::getName(modTypeId);
            const std::unordered_map<std::string, std::vector<std::string>> hashMap = makeRemapMap(name, ModTypeIdTools::getHashRemapTargets(modTypeId));
            const std::unordered_map<std::string, std::vector<std::string>> indexMap = makeRemapMap(name, ModTypeIdTools::getIndexRemapTargets(modTypeId));

            return ModType(static_cast<int>(GameTypeId::WuWa), static_cast<int>(modTypeId),
                           name, std::move(aliases),
                           std::make_shared<Hashes>(hashMap),
                           std::make_shared<Indices>(indexMap),
                           // nullptr vertexCounts -> its own fully-populated table; nullptr vgRemaps
                           // -> the shared ModDataAssets::vgRemaps, exactly as GIBuilder does.
                           nullptr, nullptr,
                           wwmiIniParseBuilder(), wwmiIniFixBuilder(), wwmiIniRemoveBuilder(),
                           std::make_shared<IndexCounts>(indexMap),
                           std::make_shared<VGOffsets>(indexMap),
                           std::make_shared<VGCounts>(indexMap),
                           std::make_shared<ShapeKeyChecksums>(indexMap));
        }
    }


    ModType WWMIBuilder::sanhua() {
        return makeWWMIModType(ModTypeId::Sanhua, {"JinhsiBodyguard"});
    }

    ModType WWMIBuilder::sanhuaExorcist() {
        // "SanhuaSkin1" is what WWMI-Assets' PlayerCharacterData calls the skin's folder.
        return makeWWMIModType(ModTypeId::SanhuaExorcist, {"SanhuaSkin1", "ExorcistSanhua", "SanhuaMoonChasing", "MoonChasingSanhua", "JinhsiBodyguardExorcist", "ExorcistJinhsiBodyguard", "JinhsiBodyguardMoonChasing", "MoonChasingJinhsiBodyguard"});
    }

    ModType WWMIBuilder::chisa() {
        return makeWWMIModType(ModTypeId::Chisa);
    }

    ModType WWMIBuilder::chisaParfait() {
        // WWMI-Assets has no folder for either of them, so there is no asset-repo name to alias.
        return makeWWMIModType(ModTypeId::ChisaParfait, {"ChisaSkin1", "ParfaitChisa"});
    }

    std::vector<ModType> WWMIBuilder::all() {
        return {
            sanhua(),
            sanhuaExorcist(),
            chisa(),
            chisaParfait(),
        };
    }
}
