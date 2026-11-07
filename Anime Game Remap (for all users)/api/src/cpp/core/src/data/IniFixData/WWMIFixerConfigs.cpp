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

#include "AGRemapCore/constants/WWMINames.h"
#include "AGRemapCore/tools/StringTools.h"

#include <sstream>
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"

#include <cctype>
#include <map>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoles.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseRows.h"
#include "AGRemapCore/data/RabbitFX.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/ListTools.h"
#include "AGRemapCore/tools/TextTools.h"
#include "AGRemapCore/tools/files/FileService.h"
#include <algorithm>

#include "AGRemapCore/data/IniFixData/Changli/ChangliFixer.h"
#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphFixer.h"
#include "AGRemapCore/data/IniFixData/Chisa/ChisaFixer.h"
#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitFixer.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeFixer.h"
#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintFixer.h"
#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaFixer.h"
#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistFixer.h"


namespace AGRemapCore {
    std::string WWMIFixerConfig::roleKind(const std::string& role) {
        for (std::size_t i = 0; i < role.size(); ++i) {
            if (std::isupper(static_cast<unsigned char>(role[i])) != 0) {
                return role.substr(i);
            }
        }
        return "";
    }

    std::optional<std::string> WWMIFixerConfig::Shader::lineRole(const std::pair<std::string, std::string>& line) {
        std::istringstream words{std::string(StringTools::strip(line.second))};
        std::string word;
        std::string role;
        if (!(words >> word) || word != WWMINames::RoleLine || !(words >> role)) {
            return std::nullopt;
        }
        return role;
    }

    std::optional<std::string> WWMIFixerConfig::registerOf(const std::vector<Binding>& bindings, const std::string& role) {
        std::map<std::string, std::string> regOfRole;
        for (const Binding& binding : bindings) {
            if (binding.role != IniKeywords::Null) {
                regOfRole.emplace(binding.role, binding.reg);
            }
        }
        if (const auto found = regOfRole.find(role); found != regOfRole.end()) {
            return found->second;
        }

        const std::string kind = roleKind(role);
        std::optional<std::string> byKind;
        for (const auto& [rowRole, reg] : regOfRole) {
            if (kind.empty() || roleKind(rowRole) != kind) {
                continue;
            }
            if (byKind.has_value() && *byKind != reg) {
                return std::nullopt;
            }
            byKind = reg;
        }
        return byKind;
    }

    bool WWMIFixerConfig::shaderDraws(int component, bool usesRabbitFX) const {
        const auto planned = plan.find(component);
        if (planned == plan.end() || !planned->second.shader.has_value()) {
            return false;
        }
        return !(planned->second.shader->yieldToRabbitFX && usesRabbitFX);
    }

    std::vector<WWMIFixerConfig::Binding> WWMIFixerConfig::bindingsOf(int component, bool shaderDrawn) const {
        const auto planned = plan.find(component);
        if (planned == plan.end()) {
            return {};
        }
        std::vector<Binding> result = planned->second.bindings;
        if (shaderDrawn && planned->second.shader.has_value()) {
            // a shader's binding takes the register's place in the plan's
            for (const Binding& extra : planned->second.shader->registers) {
                result.erase(std::remove_if(result.begin(), result.end(), [&extra](const Binding& b) {
                                 return StringTools::equalsIgnoreCase(b.reg, extra.reg);
                             }),
                             result.end());
                result.push_back(extra);
            }
        }
        return result;
    }

    std::vector<std::string> WWMIFixerConfig::shaderLineRoles(int component) const {
        std::vector<std::string> roles;
        const auto planned = plan.find(component);
        if (planned == plan.end() || !planned->second.shader.has_value()) {
            return roles;
        }
        const Shader& shader = *planned->second.shader;
        const auto collect = [&roles](const std::vector<std::pair<std::string, std::string>>& lines) {
            for (const auto& line : lines) {
                if (std::optional<std::string> role = Shader::lineRole(line)) {
                    roles.push_back(*role);
                }
            }
        };
        collect(shader.lines);
        for (const auto& pass : shader.passes) {
            collect(pass);
        }
        return roles;
    }

    std::vector<std::string> WWMIFixerConfig::passOnlyRoles(const std::set<int>& drawn) const {
        std::vector<std::string> out;
        for (const auto& [slot, byPass] : extraPassRegs) {
            const bool slotDrawn = std::any_of(plan.begin(), plan.end(), [&drawn, slot = slot](const auto& planned) {
                return planned.second.slot == slot && drawn.count(planned.first) > 0;
            });
            if (!slotDrawn) {
                continue;
            }
            for (const auto& [pass, binds] : byPass) {
                (void)pass;
                for (const Binding& binding : binds) {
                    ListTools::pushDistinct(out, binding.role);
                }
            }
        }
        for (const auto& [mesh, byPass] : sharedMeshes) {
            (void)mesh;
            for (const auto& [pass, binds] : byPass) {
                (void)pass;
                for (const Binding& binding : binds) {
                    ListTools::pushDistinct(out, binding.role);
                }
            }
        }
        // SourceComponent::overlay's registers, on a component the mod draws
        for (const auto& [component, planned] : plan) {
            if (!planned.overlay.has_value() || drawn.count(component) == 0) {
                continue;
            }
            for (const Binding& binding : planned.overlay->registers) {
                if (binding.role != IniKeywords::Null) {
                    ListTools::pushDistinct(out, binding.role);
                }
            }
        }
        return out;
    }

    WWMIFixerConfig::Download WWMIFixerConfig::downloadOf(const std::string& kind, const std::string& hash) const {
        const WWMITextureFacts& source = sourceTextures;
        const std::string fileName = DownloadTools::fixedFileName(source.downloadPrefix, kind, FileExt::DDS);
        return Download{DownloadTools::url(source.downloadGameFolder, source.downloadCharFolder, source.downloadVersionFolder,
                                           source.downloadPrefix, "Texture" + hash, FileExt::DDS),
                        fileName, IniNamingTools::getRemapDLResourceName(source.downloadPrefix + kind)};
    }

    std::optional<std::string> WWMIFixerConfig::RoleResources::of(const std::string& role, int component, bool ownShader) const {
        const auto own = ofSlotRole.find({role, component});
        if (own != ofSlotRole.end()) {
            return own->second;
        }
        // a component drawn by its own shader reads the download itself, not a texture edit of it
        if (ownShader) {
            const auto download = downloads.find(role);
            if (download != downloads.end()) {
                return download->second.resource;
            }
        }
        const auto standIn = ofRole.find(role);
        if (standIn != ofRole.end()) {
            return standIn->second;
        }
        return std::nullopt;
    }

    std::optional<std::string> WWMIFixerConfig::RoleResources::shared(const std::string& role, const std::set<int>& drawn,
                                                                      const std::set<int>& ownShader) const {
        for (const int component : drawn) {
            if (std::optional<std::string> resource = of(role, component, ownShader.count(component) > 0)) {
                return resource;
            }
        }
        return std::nullopt;
    }

    WWMIFixerConfig::RoleResources WWMIFixerConfig::roleResources(const IniParseData& data, const std::set<int>& drawn,
                                                                  const std::set<int>& rabbitFX,
                                                                  const std::string& sourceName,
                                                                  const std::string& toModName,
                                                                  const std::string& iniFolder) const {
        RoleResources result;
        const std::vector<std::string> remapNames = {sourceName, toModName};

        // a flat texture the parser refused: left to the game when the config says so and nothing else of the mod's
        // serves the role
        std::set<std::string> refused;
        for (const IniParseData::Rejection& rejection : data.rejections()) {
            const bool toGame = flatLeftToGame.count(rejection.role) > 0;
            refused.insert(rejection.role);
            result.notes.push_back(FileService::getRelPath(rejection.file, iniFolder) + " " + rejection.why + "; "
                                   + (toGame ? "left to the game" : "the source's own is used instead"));
        }
        for (const std::string& role : refused) {
            if (flatLeftToGame.count(role) > 0 && data.roleCandidates(role).empty()) {
                result.leftToGame.insert(role);
            }
        }

        // the mod's file for one role on one component: through the resource the mod uses for it, else one the fix
        // declares (a file only a previous fix named is the mod's, and the run's undo removes that name)
        std::map<std::string, std::string> declaredName;
        std::set<std::string> usedNames;
        std::set<std::string> saidAmbiguous;
        const auto assignRole = [&](const std::string& role, int component) {
            const IniParseData::RoleFile* chosen = data.roleFile(role, {"", slotPrefix + std::to_string(component)});
            if (chosen == nullptr || result.ofSlotRole.count({role, component}) > 0) {
                return;
            }

            const std::string& best = chosen->file;
            if (!chosen->tiedFile.empty() && saidAmbiguous.insert(role + "\n" + best + "\n" + chosen->tiedFile).second) {
                result.notes.push_back("WARNING: " + FileService::getRelPath(chosen->tiedFile, iniFolder) + " also has the role "
                                       + role + " (" + chosen->tiedHow + "), already taken by "
                                       + FileService::getRelPath(best, iniFolder) + " (" + chosen->how
                                       + "); the first one is bound");
            }

            result.fileOfRole[role] = best;
            const std::optional<std::string> own = data.resourceOfFile(best);
            if (own.has_value() && !IniNamingTools::looksRemapped(*own, remapNames)) {
                result.ofSlotRole[{role, component}] = *own;
                return;
            }

            auto declared = declaredName.find(best);
            if (declared == declaredName.end()) {
                std::string name = IniKeywords::Resource
                    + IniNamingTools::getModSuffixedName(TextTools::capitalize(role), IniKeywords::RemapRef, toModName);
                for (std::size_t n = 2; usedNames.count(name) > 0; ++n) {
                    name = IniKeywords::Resource + IniNamingTools::getModSuffixedName(
                        TextTools::capitalize(role) + std::to_string(n), IniKeywords::RemapRef, toModName);
                }
                usedNames.insert(name);
                declared = declaredName.emplace(best, name).first;
                result.declared.push_back(DeclaredFile{
                    best, FileService::pathToIniStr(FileService::strToPath(FileService::getRelPath(best, iniFolder))), name});
            }
            result.ofSlotRole[{role, component}] = declared->second;
        };

        // the source's own game texture standing in for a role no file of the mod's serves
        const auto standIn = [&](const std::string& role) {
            const auto fallback = sourceTextures.fallbackTextures.find(role);
            if (result.ofRole.count(role) == 0 && result.leftToGame.count(role) == 0
                    && fallback != sourceTextures.fallbackTextures.end()) {
                result.downloads[role] = downloadOf(TextTools::capitalize(role), fallback->second);
                result.ofRole[role] = result.downloads[role].resource;
            }
        };

        const auto bindings = [&](int component) {
            return bindingsOf(component, shaderDraws(component, rabbitFX.count(component) > 0));
        };

        // every role offered to every component that could serve it: the plan's, then the ones only a slot's other
        // passes or a shared mesh bind
        for (const auto& planned : plan) {
            if (drawn.count(planned.first) > 0) {
                for (const Binding& binding : bindings(planned.first)) {
                    assignRole(binding.role, planned.first);
                }
            }
        }
        const std::vector<std::string> passOnly = passOnlyRoles(drawn);
        for (const std::string& role : passOnly) {
            for (const int component : drawn) {
                assignRole(role, component);
            }
        }

        // the roles the fix invents
        for (const CreatedTexture& created : createdTextures) {
            result.ofRole[created.role] = IniNamingTools::getRemapFixName(IniKeywords::Resource + created.role, toModName);
        }

        if (sourceTextures.downloadCharFolder.empty()) {
            return result;
        }

        // a stand-in for: a planned role a drawn component has no file for; a role a drawn component's shader names
        // (a reroute's plan entry counts when the component it draws from is drawn); a pass-only role no component owns
        for (const auto& planned : plan) {
            if (drawn.count(planned.first) > 0) {
                for (const Binding& binding : bindings(planned.first)) {
                    if (result.ofSlotRole.count({binding.role, planned.first}) == 0) {
                        standIn(binding.role);
                    }
                }
            }

            bool componentDrawn = drawn.count(planned.first) > 0;
            for (const DrawReroute& reroute : drawReroutes) {
                componentDrawn = componentDrawn || (reroute.as == planned.first && drawn.count(reroute.from) > 0);
            }
            for (const std::string& role : componentDrawn ? shaderLineRoles(planned.first) : std::vector<std::string>{}) {
                standIn(role);
            }
        }
        for (const std::string& role : passOnly) {
            const bool owned = std::any_of(result.ofSlotRole.begin(), result.ofSlotRole.end(),
                                           [&role](const auto& entry) { return entry.first.first == role; });
            if (!owned) {
                standIn(role);
            }
        }
        return result;
    }

    std::string WWMIFixerConfig::GameDefaults::of(const std::string& role, int component, bool ownShader) const {
        if (ownShader) {
            const auto raw = downloads.find(role);
            if (raw != downloads.end()) {
                return raw->second.resource;
            }
        }
        return ofRole.at(role);
    }

    WWMIFixerConfig::GameDefaults WWMIFixerConfig::gameDefaults(const IniParseData& data, const RoleResources& roles,
                                                                const std::set<int>& drawn,
                                                                const std::map<int, int>& rerouteFrom,
                                                                const std::set<int>& ownShader,
                                                                const std::string& sourceName) const {
        GameDefaults result;
        if (sourceTextures.downloadCharFolder.empty()) {
            return result;
        }

        const auto sourceOf = [&rerouteFrom](int component) {
            const auto alias = rerouteFrom.find(component);
            return alias != rerouteFrom.end() ? alias->second : component;
        };
        const auto slotObj = [this, &sourceOf](int component) {
            return std::pair<std::string, std::string>("", slotPrefix + std::to_string(sourceOf(component)));
        };
        const auto bindings = [this, &ownShader](int component) {
            return bindingsOf(component, ownShader.count(component) > 0);
        };

        const auto resourceFor = [&](const std::string& role, int component) {
            return roles.of(role, component, ownShader.count(component) > 0);
        };
        const auto sharedResourceFor = [&](const std::string& role) { return roles.shared(role, drawn, ownShader); };

        // the source's own game texture of a role, downloaded and bound as the default
        const auto planGameResource = [&](const std::string& role) {
            if (result.ofRole.count(role) > 0) {
                return true;
            }
            if (result.downloads.count(role) == 0) {
                const auto hash = sourceTextures.fallbackTextures.find(role);
                if (hash == sourceTextures.fallbackTextures.end()) {
                    return false;
                }
                result.downloads[role] = downloadOf(TextTools::capitalize(role) + WWMINames::GameSuffix, hash->second);
            }
            result.ofRole[role] = result.downloads[role].resource;
            result.edited.insert(role);
            return true;
        };

        // 1. the draws before the first binding
        std::set<std::string> lateRoles;
        std::size_t lateComponents = 0;
        for (const int component : drawn) {
            if (data.lateBindings(slotObj(component)).empty()) {
                continue;
            }
            ++lateComponents;
            for (const Binding& binding : bindings(component)) {
                if (binding.role != IniKeywords::Null && roles.leftToGame.count(binding.role) == 0) {
                    lateRoles.insert(binding.role);
                }
            }
        }

        std::vector<std::string> downloaded;
        for (const std::string& role : lateRoles) {
            if (roles.fileOfRole.count(role) == 0) {
                if (std::optional<std::string> resolved = sharedResourceFor(role)) {
                    result.ofRole[role] = *resolved;       // already the source's, edits and all
                }
            } else if (planGameResource(role)) {
                downloaded.push_back(role);
            } else {
                result.notes.push_back("the draws " + slotPrefix + " sections make before binding anything need " + sourceName
                                       + "'s own " + role + ", and fallbackTextures has no hash for it -- those draws keep"
                                       " the target's texture at the mod's UVs");
            }
        }
        if (!downloaded.empty()) {
            std::string names;
            for (const std::string& role : downloaded) {
                names += (names.empty() ? "" : ", ") + role;
            }
            result.notes.push_back(std::to_string(lateComponents) + " component section(s) draw before binding a texture"
                                   " register, so those draws take " + sourceName + "'s own " + names + " rather than the mod's");
        }

        // 2. a role bound only under a toggle, and 3. a one-variant toggle
        for (int pass = 2; pass <= 3; ++pass) {
            for (const int component : drawn) {
                if (plan.count(component) == 0) {
                    continue;
                }
                const std::set<std::string>& toggledOnly = data.toggledBindings(slotObj(component));
                if (pass == 2 && toggledOnly.empty()) {
                    continue;
                }

                for (const Binding& binding : bindings(component)) {
                    const std::optional<std::string> resource = resourceFor(binding.role, component);
                    if (binding.role == IniKeywords::Null || roles.leftToGame.count(binding.role) > 0 || !resource.has_value()
                            || (pass == 2 && roles.fileOfRole.count(binding.role) == 0)) {
                        continue;
                    }

                    const std::string base = StringTools::toLower(*resource);
                    if (pass == 3) {
                        const IniParseData::TextureToggle* toggle = data.thisToggleOf(base);
                        if (toggle != nullptr && toggle->single) {
                            planGameResource(binding.role);
                        }
                        continue;
                    }

                    if (toggledOnly.count(base) == 0 || data.isThisBound(base)) {
                        continue;               // bound unconditionally, or some other way (a `this` override)
                    }
                    if (!planGameResource(binding.role)) {
                        result.notes.push_back(slotPrefix + std::to_string(component) + " binds its " + binding.role
                                               + " only under a toggle, and fallbackTextures has no hash for " + sourceName
                                               + "'s own -- the mod's file stays bound in every branch");
                        continue;
                    }
                    result.toggled[{binding.role, component}] = result.of(binding.role, component, ownShader.count(component) > 0);
                    result.notes.push_back(slotPrefix + std::to_string(component) + " binds its " + binding.role
                                           + " only under a toggle; elsewhere " + sourceName + "'s own is bound, as the game does");
                }
            }
        }
        return result;
    }

    WWMIFixerConfig::TexEditTargets WWMIFixerConfig::texEditTargets(const IniParseData& data, const RoleResources& roles,
                                                                    const std::set<int>& drawn, const std::set<int>& ownShader,
                                                                    const std::string& iniPath,
                                                                    const std::string& sourceName) const {
        TexEditTargets result;

        // which file each of the mod's resources names, so an edit reaches every variant a toggle binds
        std::map<std::string, std::string> fileOfResource;
        if (const TextureRoles* textureRoles = data.textureRoles()) {
            for (const auto& entry : textureRoles->resourcesOf(iniPath)) {
                fileOfResource.emplace(StringTools::toLower(entry.first), entry.second);
            }
        }

        const auto noteEdited = [&result](const std::string& resource, const std::string& role) {
            const std::string key = StringTools::toLower(resource);
            result.targets[role].insert(key);
            ListTools::pushDistinct(result.rolesOf[key], role);
        };

        for (const TexEdit& edit : texEdits) {
            const std::optional<std::string> was = roles.shared(edit.role, drawn, ownShader);
            if (!edit.makeFilter || !was.has_value()) {
                continue;
            }

            // the source's own textures the edit compares against -- downloaded, never bound
            for (const std::string& role : edit.sourceGameRoles) {
                const auto hash = sourceTextures.fallbackTextures.find(role);
                if (!sourceTextures.downloadCharFolder.empty() && hash != sourceTextures.fallbackTextures.end()) {
                    result.references.emplace(role, downloadOf(TextTools::capitalize(role) + WWMINames::GameSuffix, hash->second));
                } else {
                    result.notes.push_back("the " + edit.name + " edit of " + edit.role + " reads " + sourceName + "'s own "
                                           + role + ", and fallbackTextures has no hash for it");
                }
            }

            // the mod's own file, or -- for a role it has none for -- the one the stand-in download lands
            if (roles.fileOfRole.count(edit.role) == 0 && roles.downloads.count(edit.role) == 0) {
                continue;
            }

            noteEdited(*was, edit.role);

            // EVERY variant of the role, not just the one it resolved to: an edit reads one file, so a mod that binds
            // its mask two ways would carry the second through unedited. The resolved variant keeps its name; a
            // variant another role of another kind claimed first is left to that role
            std::size_t n = 1;
            std::string groupLabel;
            const auto emitVariants = [&](const IniParseData::TextureToggle* toggle) {
                if (toggle == nullptr) {
                    return;
                }
                for (const std::string& variant : toggle->variants) {
                    if (StringTools::equalsIgnoreCase(variant, *was)) {
                        continue;
                    }
                    const auto claimed = result.rolesOf.find(StringTools::toLower(variant));
                    if (claimed != result.rolesOf.end()
                            && std::any_of(claimed->second.begin(), claimed->second.end(), [&edit](const std::string& by) {
                                   return by == edit.role || roleKind(by) != roleKind(edit.role);
                               })) {
                        continue;
                    }
                    if (fileOfResource.count(StringTools::toLower(variant)) == 0) {
                        continue;
                    }
                    ++n;
                    noteEdited(variant, edit.role);
                    if (groupLabel.empty()) {
                        groupLabel = toggle->section;
                    }
                }
            };
            const std::string wasKey = StringTools::toLower(*was);
            emitVariants(data.thisToggleOf(wasKey));
            emitVariants(data.registerToggleOf(wasKey));
            if (n > 1) {
                result.notes.push_back(edit.role + ": " + std::to_string(n) + " variants bound by " + groupLabel
                                       + ", each given its own " + edit.name + " edit");
            }
        }
        return result;
    }

    const WWMIFixerConfig::TextureList* WWMIFixerConfig::TextureLists::find(const std::string& kind, int component,
                                                                           int passNum) const {
        for (const TextureList& list : lists) {
            if (list.kind == kind && list.component == component && list.passNum == passNum) {
                return &list;
            }
        }
        return nullptr;
    }

    WWMIFixerConfig::TextureLists WWMIFixerConfig::textureLists(const IniParseData& data, const RoleResources& roles,
                                                                const GameDefaults& defaults, const std::set<int>& drawn,
                                                                const std::map<int, int>& rerouteFrom,
                                                                const std::set<int>& ownShader,
                                                                const std::set<int>& overlaid) const {
        TextureLists result;
        const auto line = [](const Binding& binding, const std::string& value, bool raw, bool modToggle = true) {
            return TextureLine{binding.reg, value, binding.role, raw, modToggle, ""};
        };

        for (const auto& [component, planned] : plan) {
            if (drawn.count(component) == 0) {
                continue;
            }
            const bool raw = ownShader.count(component) > 0;
            const std::vector<Binding> bindings = bindingsOf(component, raw);

            TextureList main{"main", component, "", 0, {}};
            for (const Binding& binding : bindings) {
                if (binding.role == IniKeywords::Null) {
                    main.lines.push_back(TextureLine{binding.reg, std::string(IniKeywords::Null), "", false, false, ""});
                } else if (const auto toggled = defaults.toggled.find({binding.role, component}); toggled != defaults.toggled.end()) {
                    main.lines.push_back(line(binding, toggled->second, raw, false));
                } else if (const std::optional<std::string> resource = roles.of(binding.role, component, raw)) {
                    main.lines.push_back(line(binding, *resource, raw));
                }
            }
            result.lists.push_back(std::move(main));

            // the draws its section makes before it binds anything (a reroute reads its source component's)
            const auto alias = rerouteFrom.find(component);
            const int own = alias != rerouteFrom.end() ? alias->second : component;
            if (!data.lateBindings(std::pair<std::string, std::string>("", slotPrefix + std::to_string(own))).empty() && !defaults.ofRole.empty()) {
                TextureList game{"game", component, "", 0, {}};
                for (const Binding& binding : bindings) {
                    if (binding.role != IniKeywords::Null && defaults.ofRole.count(binding.role) > 0) {
                        game.lines.push_back(line(binding, defaults.of(binding.role, component, raw), raw));
                    }
                }
                result.lists.push_back(std::move(game));
            }

            const auto extra = extraPassRegs.find(planned.slot);
            if (extra != extraPassRegs.end()) {
                int n = 0;
                for (const auto& [pass, regs] : extra->second) {
                    TextureList passList{"pass", component, pass, n, {}};
                    for (const Binding& binding : regs) {
                        if (binding.srcComponent >= 0 && binding.srcComponent != component) {
                            continue;
                        }
                        if (const std::optional<std::string> resource = roles.of(binding.role, component, raw)) {
                            passList.lines.push_back(line(binding, *resource, raw));
                        }
                    }

                    const auto noDraw = extraPassNoDraw.find(planned.slot);
                    if (noDraw != extraPassNoDraw.end() && noDraw->second.count(pass) > 0) {
                        passList.lines.push_back(TextureLine{std::string(IniKeywords::Ib), std::string(IniKeywords::Null), "", false, false, ""});
                    }
                    result.lists.push_back(std::move(passList));
                    ++n;
                }
            }
        }

        int meshNum = 0;
        for (const auto& [meshHash, byPass] : sharedMeshes) {
            (void)meshHash;
            int n = 0;
            for (const auto& [pass, bindings] : byPass) {
                TextureList mesh{"sharedMesh", meshNum, pass, n++, {}};
                for (const Binding& binding : bindings) {
                    if (const std::optional<std::string> resource = roles.shared(binding.role, drawn, ownShader)) {
                        mesh.lines.push_back(line(binding, *resource, false));
                    }
                }
                result.lists.push_back(std::move(mesh));
            }
            ++meshNum;
        }

        for (const int component : overlaid) {
            TextureList overlay{"overlay", component, "", 0, {}};
            for (const Binding& binding : plan.at(component).overlay->registers) {
                if (binding.role == IniKeywords::Null) {
                    overlay.lines.push_back(TextureLine{binding.reg, std::string(IniKeywords::Null), "", false, false, ""});
                } else if (const std::optional<std::string> resource = roles.of(binding.role, component, ownShader.count(component) > 0)) {
                    overlay.lines.push_back(line(binding, *resource, true));
                }
            }
            result.lists.push_back(std::move(overlay));
        }
        return result;
    }

    WWMIFixerConfig::TextureLists WWMIFixerConfig::toggleLists(TextureLists lists, const IniParseData& data,
                                                               const GameDefaults& defaults, const std::string& sourceName,
                                                               const std::string& toModName, const std::string& iniPath) const {
        std::set<std::string> fileResources;
        if (const TextureRoles* textureRoles = data.textureRoles()) {
            for (const auto& entry : textureRoles->resourcesOf(iniPath)) {
                fileResources.insert(StringTools::toLower(entry.first));
            }
        }
        const auto rowsOf = [&data](const std::string& section) -> const std::vector<const IniParseData::CollectedRow*>& {
            return data.sectionRows(IniParseRows::Sections, section);
        };

        std::map<std::tuple<std::string, std::string, bool>, std::string> made;     // (role, register, raw) -> list
        for (TextureList& list : lists.lists) {
            for (TextureLine& line : list.lines) {
                if (line.role.empty() || !line.modToggle) {
                    continue;
                }
                const IniParseData::TextureToggle* toggle = data.thisToggleOf(StringTools::toLower(line.value));
                if (toggle == nullptr) {
                    continue;
                }
                if (const auto found = made.find({line.role, line.reg, line.raw}); found != made.end()) {
                    line.run = found->second;
                    continue;
                }
                if (rowsOf(toggle->section).empty() || toggle->graph.first.empty()) {
                    continue;
                }

                // a `this` naming a resource the mod neither files nor declares is the author's typo: copying it can
                // only replace a good binding of the mod's own with nothing
                ToggleList toggled;
                std::size_t live = 0;
                for (const IniParseData::CollectedRow* row : rowsOf(toggle->section)) {
                    if (StringTools::toLower(row->key) != IniKeywords::This) {
                        continue;
                    }
                    ListTools::pushDistinct(toggled.thisKeys, row->key);
                    if (fileResources.count(StringTools::toLower(row->value)) == 0 && rowsOf(row->value).empty()) {
                        toggled.deadValues.insert(row->value);
                    } else {
                        ++live;
                    }
                }
                if (live == 0) {
                    continue;
                }

                toggled.name = IniNamingTools::getRemapFixName(IniKeywords::CommandList + sourceName + TextTools::capitalize(line.role)
                                                               + IniNamingTools::getRegTag(line.reg) + (line.raw ? "Raw" : ""), toModName);
                toggled.graph = toggle->graph;
                toggled.section = toggle->section;
                toggled.role = line.role;
                toggled.reg = line.reg;
                toggled.raw = line.raw;
                if (toggle->single) {
                    const auto game = defaults.ofRole.find(line.role);
                    if (game != defaults.ofRole.end()) {
                        toggled.offBinding = game->second;
                    }
                }

                made.emplace(std::make_tuple(line.role, line.reg, line.raw), toggled.name);
                line.run = toggled.name;
                lists.toggles.push_back(std::move(toggled));
            }
        }
        return lists;
    }

    WWMIFixerConfig::FileGroups WWMIFixerConfig::fileGroups(const std::set<int>& drawn, const std::string& sourceName) const {
        FileGroups result;
        std::map<int, std::size_t> claimants;
        for (const int component : drawn) {
            const auto planned = plan.find(component);
            if (planned == plan.end()) {
                result.dropped.push_back(component);
                continue;
            }

            const std::size_t group = planned->second.sameFileSection ? 0 : claimants[planned->second.slot]++;
            if (result.groups.size() <= group) {
                result.groups.resize(group + 1);
            }
            result.groups[group].push_back(component);
        }
        if (result.groups.empty()) {
            result.groups.resize(1);
        }

        if (!result.dropped.empty()) {
            std::string names;
            for (const int component : result.dropped) {
                names += (names.empty() ? "" : ", ") + slotPrefix + std::to_string(component);
                const auto label = sourceLabels.find(component);
                if (label != sourceLabels.end() && !label->second.empty()) {
                    names += " (" + label->second + ")";
                }
            }
            const bool one = result.dropped.size() == 1;
            result.notes.push_back("this mod draws " + std::to_string(result.dropped.size()) + (one ? " component " : " components ")
                                   + names + " that " + sourceName + "'s plan has no target slot for -- "
                                   + (one ? "it is" : "they are") + " not remapped");
        }
        return result;
    }

    std::map<int, std::vector<WWMIFixerConfig::ModTexLine>> WWMIFixerConfig::modTexLines(
            const IniParseData& data, const RoleResources& roles, const TextureLists& lists, const std::set<int>& drawn,
            const std::map<int, int>& rerouteFrom, const std::set<int>& ownShader) const {
        std::map<int, std::vector<ModTexLine>> result;

        // the source component of one of the parser's slot objects
        const auto componentOf = [this](const std::pair<std::string, std::string>& obj) -> std::optional<int> {
            if (!obj.first.empty() || !StringTools::startsWith(obj.second, slotPrefix)) {
                return std::nullopt;
            }
            try {
                const int component = std::stoi(obj.second.substr(slotPrefix.size()));
                return component >= 0 ? std::optional<int>(component) : std::nullopt;
            } catch (const std::exception&) {
                return std::nullopt;
            }
        };
        const std::vector<std::string> passOnly = passOnlyRoles(drawn);
        const auto bindsRole = [&](int component, const std::string& role) {
            for (const Binding& binding : bindingsOf(component, ownShader.count(component) > 0)) {
                if (binding.role == role) {
                    return true;
                }
            }
            return std::find(passOnly.begin(), passOnly.end(), role) != passOnly.end();
        };
        const std::string refPrefix = StringTools::toLower(IniKeywords::Ref) + " ";
        const std::string prefix = StringTools::toLower(texRegPrefix);

        for (const auto& [component, planned] : plan) {
            (void)planned;
            if (drawn.count(component) == 0) {
                continue;
            }
            const bool raw = ownShader.count(component) > 0;
            const std::vector<Binding> rowBindings = bindingsOf(component, raw);
            const auto alias = rerouteFrom.find(component);
            const int ownSource = alias != rerouteFrom.end() ? alias->second : component;

            // the role the SOURCE reads at each register of its own layout, and the one the TARGET's row binds there
            std::map<std::string, std::string> sourceRoleAt, targetRoleAt;
            if (const auto own = sourceTextures.registerRoles.find(ownSource); own != sourceTextures.registerRoles.end()) {
                for (const auto& [reg, role] : own->second) {
                    sourceRoleAt.emplace(StringTools::toLower(reg), role);
                }
            }
            for (const Binding& binding : rowBindings) {
                targetRoleAt.emplace(StringTools::toLower(binding.reg), binding.role);
            }
            const auto sameKindAt = [&](const std::string& reg) {
                const auto source = sourceRoleAt.find(reg);
                const auto target = targetRoleAt.find(reg);
                return source != sourceRoleAt.end() && target != targetRoleAt.end()
                    && roleKind(source->second) == roleKind(target->second);
            };

            // the registers the component's own list binds (a line running a toggle binds its register too)
            std::set<std::string> listRegs;
            if (const TextureList* main = lists.find("main", component)) {
                for (const TextureLine& line : main->lines) {
                    listRegs.insert(StringTools::toLower(line.reg));
                    if (!line.run.empty()) {
                        listRegs.insert(std::string(IniKeywords::Run));
                    }
                }
            }

            const std::pair<std::string, std::string> ownObj("", slotPrefix + std::to_string(ownSource));
            // the line in this one's place: no answer leaves it alone, an empty line drops it
            const auto fateOf = [&](const std::string& key, const std::string& value) -> std::optional<std::optional<std::pair<std::string, std::string>>> {
                using Line = std::optional<std::pair<std::string, std::string>>;
                std::string resource = value;
                if (StringTools::startsWith(StringTools::toLower(resource), refPrefix)) {
                    resource = std::string(StringTools::lstrip(resource.substr(refPrefix.size() - 1)));
                }

                const std::optional<std::string> role = data.roleOfResource(
                    StringTools::toLower(resource), ownObj, [&](const IniParseData::ResourceRole& chosen) {
                        const std::optional<int> of = componentOf(chosen.modObj);
                        return of.has_value() && drawn.count(*of) > 0 && bindsRole(*of, chosen.role);
                    });

                // a texture put into another kind of slot on purpose goes by the slot
                if (role.has_value() && carryByRegisterRole) {
                    const auto slotRole = sourceRoleAt.find(key);
                    if (slotRole == sourceRoleAt.end() || roleKind(slotRole->second) != roleKind(*role)) {
                        const std::optional<std::string> slotReg =
                            slotRole != sourceRoleAt.end() ? registerOf(rowBindings, slotRole->second) : std::nullopt;
                        if (slotReg.has_value() && !StringTools::equalsIgnoreCase(*slotReg, key)) {
                            return Line(std::make_pair(*slotReg, value));
                        }
                        return std::nullopt;
                    }
                }

                const std::optional<std::string> reg = role.has_value() ? registerOf(rowBindings, *role) : std::nullopt;
                if (!reg.has_value()) {
                    if (listRegs.count(key) == 0 || (!role.has_value() && sameKindAt(key))) {
                        return std::nullopt;
                    }
                    return Line();
                }

                if (!raw && data.refusedRoleOf(resource) == role) {
                    const std::optional<std::string> resolved = roles.of(*role, component, raw);
                    return resolved.has_value() ? Line(std::make_pair(*reg, *resolved)) : Line();
                }
                return Line(std::make_pair(*reg, resource));
            };

            std::vector<ModTexLine>& lines = result[component];
            std::set<std::pair<std::string, std::string>> seen;
            for (const std::string& section : data.collectedSections(IniParseRows::Sections)) {
                for (const IniParseData::CollectedRow* row : data.sectionRows(IniParseRows::Sections, section)) {
                    const std::string key = StringTools::toLower(std::string(StringTools::strip(row->key)));
                    const std::string value(StringTools::strip(row->value));
                    if (!StringTools::startsWith(key, prefix) || seen.count({key, value}) > 0) {
                        continue;
                    }
                    if (auto fate = fateOf(key, value)) {
                        seen.insert({key, value});
                        lines.push_back(fate->has_value() ? ModTexLine{key, value, false, (*fate)->first, (*fate)->second}
                                                          : ModTexLine{key, value, true, "", ""});
                    }
                }
            }
        }
        return result;
    }

    WWMIFixerConfig::LinkedLines WWMIFixerConfig::linkedLines(const IniParseData& data, const RoleResources& roles,
                                                              const std::set<int>& drawn,
                                                              const std::vector<IniFile*>& linkedInis,
                                                              const std::string& sourceName, const std::string& toModName,
                                                              const std::string& iniFolder) const {
        LinkedLines result;
        const std::vector<IniParseData::LinkedLine>& lines = data.linkedLines();
        const std::vector<std::string> remapNames = {sourceName, toModName};

        // the source component a line's graph is a slot of
        const auto componentOf = [this, &drawn](const std::pair<std::string, std::string>& obj) -> std::optional<int> {
            if (!obj.first.empty() || !StringTools::startsWith(obj.second, slotPrefix)) {
                return std::nullopt;
            }
            try {
                const int component = std::stoi(obj.second.substr(slotPrefix.size()));
                return drawn.count(component) > 0 ? std::optional<int>(component) : std::nullopt;
            } catch (const std::exception&) {
                return std::nullopt;
            }
        };

        // the linked files, in the order their first line was read, and the variables their conditions read
        std::vector<std::string> files;
        std::map<std::string, std::set<std::string>> varsOf;
        for (const IniParseData::LinkedLine& line : lines) {
            if (!componentOf(line.obj).has_value()) {
                continue;
            }
            if (std::find(files.begin(), files.end(), line.ini) == files.end()) {
                files.push_back(line.ini);
            }
            for (const std::string& var : IniNamingTools::varsIn(line.condition)) {
                varsOf[line.ini].insert(var);
            }
        }

        // each file's state the fix copies, under the variables' new names
        std::map<std::string, std::map<std::string, std::string>> renamesOf;
        for (const std::string& ini : files) {
            const std::set<std::string>& vars = varsOf[ini];
            const auto linked = std::find_if(linkedInis.begin(), linkedInis.end(), [&ini](IniFile* file) {
                return file != nullptr && file->getFile().value_or("") == ini;
            });
            if (vars.empty() || linked == linkedInis.end()) {
                continue;
            }
            LinkedState state;
            state.ini = ini;
            state.suffix = toModName + IniKeywords::RemapRef + (result.states.empty() ? std::string()
                                                                                      : std::to_string(result.states.size() + 1));
            state.vars = (*linked)->varState(vars).vars;
            for (const std::string& var : state.vars) {
                renamesOf[ini][var] = var + state.suffix;
            }
            result.states.push_back(std::move(state));
        }

        // each line: its value through a resource of this .ini, its condition on the copies
        std::map<std::string, std::string> declaredName;
        std::set<std::string> usedNames;
        for (const DeclaredFile& declared : roles.declared) {
            declaredName.emplace(declared.file, declared.resource);
            usedNames.insert(declared.resource);
        }
        std::size_t carried = 0;
        for (const IniParseData::LinkedLine& line : lines) {
            const std::optional<int> component = componentOf(line.obj);
            if (!component.has_value()) {
                continue;
            }

            std::string resource;
            if (const std::optional<std::string> own = data.resourceOfFile(line.file);
                    own.has_value() && !IniNamingTools::looksRemapped(*own, remapNames)) {
                resource = *own;
            } else {
                auto declared = declaredName.find(line.file);
                if (declared == declaredName.end()) {
                    std::string base = IniKeywords::Resource + (line.key.empty() ? WWMINames::Carried : WWMINames::Glow)
                        + TextTools::capitalize(FileService::pathToStr(FileService::strToPath(line.file).stem()));
                    base.erase(std::remove_if(base.begin(), base.end(),
                                              [](char c) { return !std::isalnum(static_cast<unsigned char>(c)); }),
                               base.end());
                    std::string name = base + toModName + IniKeywords::RemapRef;
                    for (std::size_t n = 2; usedNames.count(name) > 0; ++n) {
                        name = base + std::to_string(n) + toModName + IniKeywords::RemapRef;
                    }
                    usedNames.insert(name);
                    declared = declaredName.emplace(line.file, name).first;
                    result.declared.push_back(DeclaredFile{
                        line.file, FileService::pathToIniStr(FileService::strToPath(FileService::getRelPath(line.file, iniFolder))),
                        name});
                }
                resource = declared->second;
            }

            const auto renames = renamesOf.find(line.ini);
            const std::string condition = renames == renamesOf.end()
                ? line.condition : IniNamingTools::renameVars(line.condition, renames->second);
            result.lines[*component].push_back(CarriedLine{condition, line.key, line.role, IniKeywords::Ref + " " + resource});
            ++carried;
        }
        if (carried > 0) {
            result.notes.push_back("another .ini of the mod sets " + std::to_string(carried) + " line(s) on this file's draws"
                                   " that the target never reaches; carried into lists of the fix's own");
        }
        return result;
    }

    std::map<int, std::array<int, 4>> WWMIFixerConfig::rabbitFXFlags(
            const IniParseData& data, const RoleResources& roles, const std::map<std::pair<std::string, int>, std::string>& toggled,
            const std::map<int, std::vector<std::string>>& drawn, const std::map<int, int>& rerouteFrom,
            const std::string& iniPath, const std::string& iniFolder) const {
        std::map<int, std::array<int, 4>> result;

        std::map<std::string, std::string> fileOfResource;
        if (const TextureRoles* textureRoles = data.textureRoles()) {
            for (const auto& entry : textureRoles->resourcesOf(iniPath)) {
                fileOfResource.emplace(StringTools::toLower(entry.first), entry.second);
            }
        }
        const auto tagOfResource = [&](const std::string& resource) -> std::optional<int> {
            const auto file = fileOfResource.find(StringTools::toLower(IniNamingTools::removeRefPrefix(resource)));
            if (file == fileOfResource.end()) {
                return std::nullopt;
            }
            return RabbitFX::textureTag(FileService::absPathOfRelPath(file->second, iniFolder));
        };

        // whether a component's own sections bind 'reg' themselves
        const auto bindsRegister = [&](int component, const std::string& reg) {
            const auto sections = drawn.find(component);
            return sections != drawn.end()
                   && std::any_of(sections->second.begin(), sections->second.end(), [&](const std::string& name) {
                          const auto& rows = data.sectionRows(IniParseRows::Sections, name);
                          return std::any_of(rows.begin(), rows.end(), [&reg](const IniParseData::CollectedRow* row) {
                              return IniNamingTools::sameKey(row->key, reg);
                          });
                      });
        };
        // whether a component's sections run RabbitFX's SetTextures, directly or through a command list they call
        const auto callsSetTextures = [&](int component) {
            if (drawn.count(component) == 0) {
                return false;
            }
            const std::pair<std::string, std::string> obj("", slotPrefix + std::to_string(component));
            const auto& rows = data.collected(IniParseRows::Objs);
            return std::any_of(rows.begin(), rows.end(), [&obj](const IniParseData::CollectedRow& row) {
                return row.graphId.modObj == obj && row.key == IniKeywords::Run
                       && StringTools::equalsIgnoreCase(StringTools::strip(row.value), IniKeywords::RabbitFXSetTexturesPath);
            });
        };

        for (const auto& [component, planned] : plan) {
            if (!planned.rabbitFX.has_value() || drawn.count(component) == 0) {
                continue;
            }
            const auto alias = rerouteFrom.find(component);
            const int source = alias != rerouteFrom.end() ? alias->second : component;
            if (!callsSetTextures(source)) {
                continue;                           // nothing of RabbitFX's to keep or pin
            }

            const auto regs = sourceTextures.registerRoles.find(source);
            std::array<int, 4> tags{0, 0, 0, 0};
            for (int k = 0; k < 4; ++k) {
                const std::string reg = "ps-t" + std::to_string(k);
                const auto game = planned.rabbitFX->gameTags.find(reg);
                tags[static_cast<std::size_t>(k)] = game != planned.rabbitFX->gameTags.end() ? game->second : 0;
                if (regs == sourceTextures.registerRoles.end()) {
                    continue;
                }
                const auto role = regs->second.find(reg);
                if (role == regs->second.end()) {
                    continue;
                }

                // the mod's own line at this register, in its base state
                if (bindsRegister(source, reg) && toggled.count({role->second, source}) == 0) {
                    const auto current = roles.ofSlotRole.find({role->second, source});
                    if (current != roles.ofSlotRole.end()) {
                        if (const std::optional<int> tag = tagOfResource(current->second)) {
                            tags[static_cast<std::size_t>(k)] = *tag;
                            continue;
                        }
                    }
                }

                // ...else the game's texture there, or the mod's `this` in its place
                const auto gameHash = sourceTextures.fallbackTextures.find(role->second);
                if (gameHash != sourceTextures.fallbackTextures.end()) {
                    if (const std::optional<std::string> over = data.thisOverrideOf(gameHash->second)) {
                        if (const std::optional<int> tag = tagOfResource(*over)) {
                            tags[static_cast<std::size_t>(k)] = *tag;
                        }
                    }
                }
            }
            result[component] = RabbitFX::routingFlags(tags);
        }
        return result;
    }

    std::optional<WWMIFixerConfig> WWMIFixerConfig::of(ModTypeId source, ModTypeId target) {
        if (source == ModTypeId::Changli && target == ModTypeId::ChangliNymph) {
            return ChangliFixer::nymph2_4Config();
        }
        if (source == ModTypeId::ChangliNymph && target == ModTypeId::Changli) {
            return ChangliNymphFixer::changli1_4Config();
        }
        if (source == ModTypeId::Chisa && target == ModTypeId::ChisaParfait) {
            return ChisaFixer::parfait3_7Config();
        }
        if (source == ModTypeId::ChisaParfait && target == ModTypeId::Chisa) {
            return ChisaParfaitFixer::chisa2_8Config();
        }
        if (source == ModTypeId::Lynae && target == ModTypeId::LynaePeppermint) {
            return LynaeFixer::peppermint3_7Config();
        }
        if (source == ModTypeId::LynaePeppermint && target == ModTypeId::Lynae) {
            return LynaePeppermintFixer::lynae3_7Config();
        }
        if (source == ModTypeId::Sanhua && target == ModTypeId::SanhuaExorcist) {
            return SanhuaFixer::exorcist2_5Config();
        }
        if (source == ModTypeId::SanhuaExorcist && target == ModTypeId::Sanhua) {
            return SanhuaExorcistFixer::sanhua2_5Config();
        }
        return std::nullopt;
    }
}
