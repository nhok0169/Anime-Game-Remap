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

#include "AGRemapCore/constants/ZZMIBuilder.h"

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
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"


namespace AGRemapCore {
    namespace {
        // The three table-backed builders every ZZZ ModType shares -- the same tables GI and WuWa read
        // (GIBuilder.cpp has the reasoning). Function-local statics for the same init-order reason.
        const std::shared_ptr<IniParseBuilder>& zzmiIniParseBuilder() {
            static const std::shared_ptr<IniParseBuilder> builder =
                std::make_shared<IniParseBuilder>(IniParseBuilderData::repo());
            return builder;
        }

        const std::shared_ptr<IniFixBuilder>& zzmiIniFixBuilder() {
            static const std::shared_ptr<IniFixBuilder> builder =
                std::make_shared<IniFixBuilder>(IniFixBuilderData::repo());
            return builder;
        }

        const std::shared_ptr<IniRemoveBuilder>& zzmiIniRemoveBuilder() {
            static const std::shared_ptr<IniRemoveBuilder> builder =
                std::make_shared<IniRemoveBuilder>(IniRemoveBuilderData::repo());
            return builder;
        }

        // The remap graph, as GIBuilder.cpp's makeRemapMap builds it: {ownName -> [targetNames]}, and
        // the same targets under every component name, because HashData files a ZZZ character's rows
        // under its COMPONENTS' names -- without them the reverse-then-forward lookup of
        // ModMappedAssets::replace resolves a source hash to "MiyabiBody", finds no such key and writes
        // HashNotFound.
        std::unordered_map<std::string, std::vector<std::string>> makeRemapMap(ModTypeId modTypeId, const std::string& name,
                                                                               const std::vector<ModTypeId>& targets) {
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
            for (ModTypeId component : ModTypeIdTools::getComponentIds(modTypeId)) {
                result.emplace(ModTypeIdTools::getName(component), targetNames);
            }
            return result;
        }

        ModType makeZZMIModType(ModTypeId modTypeId, std::vector<std::string> aliases = {}) {
            const std::string name = ModTypeIdTools::getName(modTypeId);

            return ModType(static_cast<int>(GameTypeId::ZZZ), static_cast<int>(modTypeId),
                           name, std::move(aliases),
                           std::make_shared<Hashes>(makeRemapMap(modTypeId, name, ModTypeIdTools::getHashRemapTargets(modTypeId))),
                           std::make_shared<Indices>(makeRemapMap(modTypeId, name, ModTypeIdTools::getIndexRemapTargets(modTypeId))),
                           // nullptr vertexCounts / vgRemaps: the shared tables, exactly as GIBuilder does
                           nullptr, nullptr,
                           zzmiIniParseBuilder(), zzmiIniFixBuilder(), zzmiIniRemoveBuilder());
        }
    }


    ModType ZZMIBuilder::miyabi() {
        return makeZZMIModType(ModTypeId::Miyabi, {"HoshimiMiyabi", "MiyabiHoshimi"});
    }

    ModType ZZMIBuilder::miyabiBlossom() {
        // Her mods name the skin 'MiyabiSkin'; the outfit is "Hoshimi Miyabi: Dignified Blossom" in game.
        return makeZZMIModType(ModTypeId::MiyabiBlossom, {"MiyabiSkin", "MiyabiDignifiedBlossom", "DignifiedBlossomMiyabi", "BlossomMiyabi", "HoshimiMiyabiBlossom"});
    }

    std::vector<ModType> ZZMIBuilder::all() {
        // NOT the component ids: they are asset-table names, built by nobody -- see ModTypeId::MiyabiBody.
        return {
            miyabi(),
            miyabiBlossom(),
        };
    }
}
