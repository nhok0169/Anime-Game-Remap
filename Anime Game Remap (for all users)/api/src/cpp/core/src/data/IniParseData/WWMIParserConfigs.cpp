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

#include "AGRemapCore/tools/StringTools.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/WWMINames.h"
#include "AGRemapCore/data/RabbitFX.h"
#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/buffers/DrawUVCoverage.h"
#include "AGRemapCore/model/buffers/SkeletonNumberingProbe.h"
#include "AGRemapCore/model/buffers/WWMIBlend.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseRows.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoles.h"
#include "AGRemapCore/model/textures/TexThumbprint.h"
#include "AGRemapCore/tools/files/FileService.h"



namespace AGRemapCore {
    std::optional<int> WWMIParserConfig::slotOf(const std::pair<std::string, std::string>& obj) const {
        return slotWithPrefix(obj, WWMINames::SlotPrefix);
    }

    std::optional<int> WWMIParserConfig::slotWithPrefix(const std::pair<std::string, std::string>& obj, const std::string& prefix) {
        if (!obj.first.empty() || !StringTools::startsWith(obj.second, prefix)) {
            return std::nullopt;
        }
        const std::string digits = obj.second.substr(prefix.size());
        if (digits.empty() || digits.size() > 9 || digits.find_first_not_of("0123456789") != std::string::npos) {
            return std::nullopt;
        }
        return std::stoi(digits);
    }

    namespace {
        using ModObj = IniParseData::ModObj;

        ModObj slotObjOf(int slot) {
            return ModObj("", WWMINames::SlotPrefix + std::to_string(slot));
        }

        // a lone `this` under `$object_detected` is not a toggle (every Sanhua mod wraps its one `this` that way)
        std::vector<std::string> notTogglesOf(bool lower) {
            const std::string detected = lower ? StringTools::toLower(IniKeywords::WWMIObjectDetected) : IniKeywords::WWMIObjectDetected;
            return {detected, detected + "==1"};
        }
    }

    std::map<int, std::vector<std::string>> WWMIParserConfig::slotSections(const SectionTargets& targets, const std::vector<std::string>& sectionNames) {
        std::unordered_map<std::string, std::size_t> at;
        std::size_t next = 0;
        for (const std::string& each : sectionNames) {
            at.emplace(each, next++);
        }
        const auto sectionAt = [&at](const std::string& name) {
            const auto found = at.find(name);
            return found == at.end() ? at.size() : found->second;
        };

        std::map<int, std::vector<std::string>> result;
        for (const auto& [obj, sections] : targets) {
            if (const std::optional<int> slot = slotWithPrefix(obj, WWMINames::SlotPrefix)) {
                std::vector<std::string>& into = result[*slot];
                into.insert(into.end(), sections.begin(), sections.end());
            }
        }

        for (auto& [slot, sections] : result) {
            (void)slot;
            std::stable_sort(sections.begin(), sections.end(),
                             [&sectionAt](const std::string& a, const std::string& b) { return sectionAt(a) < sectionAt(b); });
            sections.erase(std::unique(sections.begin(), sections.end()), sections.end());
        }
        return result;
    }

    bool WWMIParserConfig::mergedSkeleton(const std::vector<Row>& objRows, const SectionTargets& targets, int slots) {
        std::unordered_set<std::string> sections;
        for (const auto& [slot, names] : slotSections(targets, {})) {
            if (slots >= 0 && slot >= slots) {
                continue;
            }
            for (const std::string& name : names) {
                sections.insert(StringTools::toLower(name));
            }
        }
        return std::any_of(objRows.begin(), objRows.end(), [&sections](const Row& row) {
            return row.key == IniKeywords::WWMIVgOffset && sections.count(StringTools::toLower(row.section)) > 0;
        });
    }

    IniParseData::ResourceDataRules WWMIParserConfig::resourceDataRules(const SectionTargets& targets, const std::vector<std::string>& sectionNames,
                                                                        bool onlyThisFile, std::function<void(const std::string&)> log) const {
        IniParseData::ResourceDataRules rules;
        rules.rows = IniParseRows::Objs;
        rules.resources = IniParseRows::Resources;
        rules.sections = IniParseRows::Sections;
        rules.texRegPrefix = IniKeywords::TexRegPrefix;
        rules.noneUndrawnWhenAll = true;
        rules.libraries = {{RabbitFX::Name, [](const std::string& text) { return RabbitFX::names(text); }}};

        // a `run =` draws when it calls a section the file does not have, unless that is WWMI's own library
        const auto names = std::make_shared<const std::unordered_set<std::string>>(sectionNames.begin(), sectionNames.end());
        const std::string wwmi = StringTools::toLower(IniKeywords::WWMINamespace.substr(1)) + "\\";
        rules.runDraws = [names, wwmi](const std::string& called) {
            return names->count(called) == 0 && StringTools::toLower(called).find(wwmi) == std::string::npos;
        };
        rules.log = std::move(log);
        for (const auto& [obj, sections] : targets) {
            if (slotOf(obj).has_value()) {
                rules.roots.emplace_back(obj, sections);
            }
        }

        // THE TOGGLES: which of the mod's own resources a texture override binds in SEVERAL VARIANTS -- only for the
        // file a fix reads its textures from. One `this` under `$object_detected` alone is not a toggle; under any other
        // condition it is a toggle of one variant
        if (!onlyThisFile && !(textures.roles.empty() && textures.registerRoles.empty())) {
            rules.toggles = WWMINames::TextureTogglesRows;
            rules.togglesIn = [](const std::string& section) {
                return StringTools::startsWith(section, IniKeywords::TextureOverride)
                    && !StringTools::endsWithIgnoreCase(section, IniKeywords::RemapFix);
            };
            rules.notToggles = notTogglesOf(false);
            for (const auto& [obj, sections] : targets) {
                if (obj.first == IniGraphModObjKeywords::Texture) {
                    rules.textureRoots.emplace_back(obj, sections);
                }
            }
        }
        return rules;
    }

    bool WWMIParserConfig::replacesTexturesByHash(const std::vector<Row>& overrideRows, const SectionTargets& targets) const {
        std::unordered_set<std::string> slotRoots;
        for (const auto& [obj, sections] : targets) {
            if (slotOf(obj).has_value()) {
                slotRoots.insert(sections.begin(), sections.end());
            }
        }

        std::map<std::string, std::pair<bool, bool>> hashAndThis;
        for (const Row& row : overrideRows) {
            if (!StringTools::startsWith(row.section, IniKeywords::TextureOverride) || slotRoots.count(row.section) > 0) {
                continue;
            }

            auto& has = hashAndThis[row.section];
            has.first = has.first || row.key == IniKeywords::Hash;
            has.second = has.second || row.key == IniKeywords::This;
            if (has.first && has.second) {
                return true;
            }
        }
        return false;
    }

    std::optional<IniParseData::RoleFileRules> WWMIParserConfig::roleFileRules(const SectionTargets& targets, const std::vector<std::string>& sectionNames,
                                                                               const ModType* self, const std::string& iniPath,
                                                                               const std::string& iniFolder) const {
        if (textures.roles.empty() && textures.registerRoles.empty()) {
            return std::nullopt;
        }

        IniParseData::RoleFileRules rules;
        rules.iniPath = iniPath;
        rules.folder = iniFolder;
        rules.sections = IniParseRows::Sections;
        rules.declared = IniParseRows::Declared;
        rules.overrides = WWMINames::TextureOverridesRows;
        rules.roleRows = WWMINames::TextureRolesRows;
        rules.carriedRows = WWMINames::CarriedRows;
        rules.objRows = IniParseRows::Objs;
        for (const auto& [slot, sections] : slotSections(targets, sectionNames)) {
            rules.objs.emplace_back(slotObjOf(slot), sections);
        }
        for (const auto& [slot, layout] : textures.registerRoles) {
            rules.registerRoles[slotObjOf(slot)] = layout;
        }
        rules.regionRoles = textures.regionRoles;
        rules.notToggles = notTogglesOf(true);

        // the role a hash of the character plays: her Hashes row typed by a role this config knows, the config's table after it
        const auto facts = std::make_shared<const WWMITextureFacts>(textures);
        const std::shared_ptr<Hashes> hashes = self == nullptr ? nullptr : self->hashes;
        const std::string ownName = self == nullptr ? std::string() : self->name;
        std::unordered_set<std::string> knownRoles;
        for (const auto& entry : textures.roles) {
            knownRoles.insert(entry.second);
        }
        for (const auto& comp : textures.registerRoles) {
            for (const auto& reg : comp.second) {
                knownRoles.insert(reg.second);
            }
        }
        rules.roleOfHash = [facts, hashes, ownName, knownRoles](const std::string& hash) -> std::optional<std::string> {
            if (hashes != nullptr && !ownName.empty()) {
                const std::optional<std::vector<std::string>> key = hashes->getKey(hash, std::nullopt, {ownName, std::nullopt}, false);
                if (key.has_value() && key->size() > 1 && knownRoles.count((*key)[1]) > 0) {
                    return (*key)[1];
                }
            }
            const auto it = facts->roles.find(hash);
            return it == facts->roles.end() ? std::optional<std::string>() : std::optional<std::string>(it->second);
        };

        // A FILE NO HASH AND NO REGISTER NAMED MAY STILL BE THE GAME'S OWN TEXTURE, AND ITS PIXELS SAY SO
        rules.identify = [facts](const std::string& file) -> std::optional<std::string> {
            return TexThumbprint::identifyFile(file, facts->textureThumbprints, facts->thumbprintSize,
                                               facts->identityMin, facts->identityGap);
        };

        // HOW OLD A HASH IS: the versions the library's hashes file it at, newest first, and within one version the current
        // hash before its `Live` and `Old`, `Old2`, ... generations
        rules.hashAge = [ownName, ages = std::make_shared<Hashes>()](const std::string& hash) {
            return ages->age(ownName, hash);
        };

        // a section a previous fix wrote is not the mod's use of a resource
        std::vector<std::string> fixNames{ownName};
        if (self != nullptr) {
            for (const std::string& name : self->getModsToFix()) {
                fixNames.push_back(name);
            }
        }
        rules.fixSection = [fixNames](const std::string& section) { return IniNamingTools::looksRemapped(section, fixNames); };
        return rules;
    }

    std::optional<std::string> WWMIParserConfig::skeletonNumbering(const std::vector<Row>& objRows, const std::vector<Row>& resourceRows,
                                                                   const SectionTargets& targets, const std::string& iniFolder,
                                                                   std::function<void(const std::string&)> log) const {
        if (skeletonNumberings.size() < 2 || referenceBoneCentroids.empty()) {
            return std::nullopt;
        }

        // a merged-skeleton mod: a `vg_offset` in a slot graph's own root section
        const bool merged = mergedSkeleton(objRows, targets);

        const std::map<std::string, std::string> files = IniParseData::resourceFilesOf(resourceRows);
        const auto fileOf = [&files](const std::string& section) -> std::optional<std::string> {
            const auto found = files.find(StringTools::toLower(section));
            return found == files.end() ? std::nullopt : std::optional<std::string>(found->second);
        };
        const std::optional<std::string> position = fileOf(IniKeywords::WWMIPositionBuffer);
        const std::optional<std::string> blend = fileOf(IniKeywords::WWMIBlendBuffer);
        if (!merged || !position.has_value() || !blend.has_value()) {
            return std::nullopt;
        }

        // the remapped skeleton's own 16-bit ids
        const std::optional<std::string> vertexVG = fileOf(IniKeywords::WWMIBlendRemapVertexVGBuffer);
        const auto pathOf = [&iniFolder](const std::string& rel) {
            return FileService::absPathOfRelPath(FileService::iniPathToRel(rel), iniFolder);
        };
        const std::optional<SkeletonNumberingProbe::Result> found = SkeletonNumberingProbe::probe(
            pathOf(*position), pathOf(*blend), vertexVG.has_value() ? std::optional<std::string>(pathOf(*vertexVG)) : std::nullopt,
            skeletonNumberings, referenceBoneCentroids);
        if (!found.has_value()) {
            return std::nullopt;
        }

        const std::string& numbering = skeletonNumberings[found->best].version;
        if (log) {
            std::ostringstream how;
            for (std::size_t n = 0; n < found->medians.size(); ++n) {
                how << (n == 0 ? "" : ", ") << skeletonNumberings[n].version << " "
                    << static_cast<long long>(std::lround(found->medians[n] * 100)) / 100.0;
            }
            log("its bone ids are " + numbering + "'s numbering (" + std::to_string(found->vertices)
                + " vertices on renumbered bones; median distance to the bone by version: " + how.str() + ")");
        }
        return numbering;
    }

    std::optional<IniParseData::ResourceData> WWMIParserConfig::splitDraws(const IniParseData::ResourceData* data,
                                                                           const IniParseData::RoleFileData* roleData,
                                                                           const SectionTargets& targets, const std::string& iniPath,
                                                                           const std::string& iniFolder,
                                                                           std::function<void(const std::string&)> log) const {
        if (drawSplits.empty() || data == nullptr) {
            return std::nullopt;
        }
        const auto fileOf = [data](const std::string& section) -> std::optional<std::string> {
            const auto found = data->resourceFiles.find(StringTools::toLower(section));
            return found == data->resourceFiles.end() ? std::nullopt : std::optional<std::string>(found->second);
        };
        const std::optional<std::string> index = fileOf(IniKeywords::WWMIIndexBuffer);
        const std::optional<std::string> texcoord = fileOf(IniKeywords::WWMITexcoordBuffer);
        const std::optional<std::string> position = fileOf(IniKeywords::WWMIPositionBuffer);
        if (!index.has_value() || !texcoord.has_value() || !position.has_value()) {
            return std::nullopt;
        }

        IniParseData::ResourceData result = *data;
        // only whether a slot has sections is asked, so their order does not matter
        const std::map<int, std::vector<std::string>> drawn = slotSections(targets, {});
        for (const DrawSplit& split : drawSplits) {
            const ModObj obj = slotObjOf(split.component);
            const auto drawRanges = data->drawRanges.find(obj);
            const auto sections = drawn.find(split.component);
            if (drawRanges == data->drawRanges.end() || drawRanges->second.empty() || data->undrawn.count(obj) > 0
                    || sections == drawn.end() || sections->second.empty()) {
                continue;
            }
            const std::vector<IniParseData::DrawRange>& ranges = drawRanges->second;

            // the MOD's own map, when a resource of this .ini names the file chosen for the role (one a previous fix
            // declared is no evidence about the mod) -- else the character's own game map, which DrawSplit::sourceFlags
            // stands in for
            std::optional<std::string> codesPath;
            const IniParseData::RoleFile* chosen = nullptr;
            if (roleData != nullptr) {
                const auto chosenAt = roleData->roleFiles.find({split.codeRole, obj});
                chosen = chosenAt == roleData->roleFiles.end() ? nullptr : &chosenAt->second;
            }
            const TextureRoles* roles = roleData == nullptr || !roleData->roles.has_value() ? nullptr : &*roleData->roles;
            if (chosen != nullptr && roles != nullptr) {
                for (const auto& [resource, file] : roles->resourcesOf(iniPath)) {
                    if (file == chosen->file) {
                        if (!IniNamingTools::looksRemapped(resource, {})) {
                            codesPath = FileService::absPathOfRelPath(file, iniFolder);
                        }
                        break;
                    }
                }
            }

            const DrawSplit::FlagMask& mask = split.sourceFlags;
            const bool useMask = !codesPath.has_value();
            if (useMask && (mask.width <= 0 || mask.height <= 0
                            || mask.bits.size() * 8 < static_cast<std::size_t>(mask.width) * static_cast<std::size_t>(mask.height))) {
                continue;
            }

            std::vector<IniParseData::DrawRange> taken;
            try {
                std::vector<std::uint8_t> codes;
                long long w = mask.width;
                long long h = mask.height;
                if (!useMask) {
                    TextureFile map{*codesPath};
                    map.open();
                    if (!map.hasImage()) {
                        continue;
                    }
                    codes = map.shaderChannel(0);
                    w = map.getWidth();
                    h = map.getHeight();
                }

                // the mesh's vertices are its Position.buf's lines
                const std::size_t vertices = static_cast<std::size_t>(std::filesystem::file_size(
                    FileService::strToPath(FileService::absPathOfRelPath(*position, iniFolder))) / WWMIBlend::PositionStride);
                const DrawUVCoverage coverage = DrawUVCoverage::ofMesh(FileService::absPathOfRelPath(*index, iniFolder),
                                                                       FileService::absPathOfRelPath(*texcoord, iniFolder), vertices);

                const DrawUVCoverage::TexelTest marked = [&](long long x, long long y) {
                    const auto at = static_cast<std::size_t>(y * w + x);
                    if (useMask) {
                        return ((mask.bits[at / 8] >> (at % 8)) & 1) != 0;
                    }
                    return at < codes.size() && (codes[at] & split.flags) != 0;
                };

                taken = coverage.coveredRanges(ranges, w, h, marked, split.share);
            } catch (const std::exception& e) {
                if (log) {
                    log("could not read which draws of " + obj.second + " carry the " + split.codeRole + " flags: " + e.what());
                }
                continue;
            }

            for (const IniParseData::DrawRange& range : taken) {
                result.drawRanges[ModObj(split.name, obj.second)].push_back(range);
            }
        }
        return result;
    }

}
