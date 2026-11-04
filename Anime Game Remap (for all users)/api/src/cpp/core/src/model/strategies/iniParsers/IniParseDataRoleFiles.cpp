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

// IniParseData::roleFiles: which file each mod object takes for each texture role, read off what the .ini states.

#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <map>
#include <set>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollect.h"
#include "AGRemapCore/model/textures/TexThumbprint.h"
#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    namespace {
        using Row = IniParseData::CollectedRow;
        using ModObj = IniParseData::ModObj;

        std::string stripped(const std::string& text) {
            return std::string(StringTools::strip(text));
        }

        std::string lowered(const std::string& text) {
            return StringTools::toLower(text);
        }

        bool exists(const std::string& file) {
            std::error_code error;
            return std::filesystem::is_regular_file(FileService::strToPath(file), error);
        }

        // A file a previous run of a fix wrote: `RemapDL` / `RemapTex` right before the extension -- the
        // same marker the undo deletes by. A download is a byte copy of the game's texture, so its pixels
        // ARE that texture; read as the mod's, a second fix binds the first one's downloads.
        bool fixWrote(const std::string& file) {
            const std::string stem = lowered(FileService::stem(file));
            return StringTools::endsWith(stem, lowered(IniKeywords::RemapDL))
                   || StringTools::endsWith(stem, lowered(IniKeywords::RemapTex));
        }

        // the file a resource of a .ini names, from that .ini's folder
        std::optional<std::string> fileOfResource(const IniParseData& data, const std::string& folder, const std::string& value) {
            const std::optional<std::string> file = data.resourceFile(IniNamingTools::removeRefPrefix(stripped(value)));
            if (!file.has_value()) {
                return std::nullopt;
            }
            return FileService::absPathOfRelPath(FileService::iniPathToRel(*file), folder);
        }

        // a section's position in the file: the order the Sections collect gives them, as written
        std::unordered_map<std::string, std::size_t> sectionOrder(const IniParseData& data, const std::string& sections) {
            std::unordered_map<std::string, std::size_t> at;
            if (!sections.empty()) {
                for (const std::string& section : data.collectedSections(sections)) {
                    at.emplace(section, at.size());
                }
            }
            return at;
        }

        std::size_t positionOf(const std::unordered_map<std::string, std::size_t>& at, const std::string& section) {
            const auto found = at.find(section);
            return found == at.end() ? at.size() : found->second;
        }

        // A toggle written as one register an object's graph binds to several resources, each under its own
        // condition -- through `run =` (a CommandList of `if`s), in this file or a linked one. Per root section
        // and register, the resources as written; only one that holds under a real condition is a toggle.
        // a toggle the reader finds itself: its files in the order written, and the one that shows at the starting values
        struct BindToggle {
            std::vector<std::string> files;
            std::string atStart;
        };

        void registerBindToggles(const IniParseData& data, const std::string& rows,
                                 const std::function<std::optional<std::string>(const std::string& value)>& fileOf,
                                 const std::vector<std::string>& notToggles, const std::optional<Z3Predicate>& start,
                                 Z3Context* z3Ctx, std::map<std::string, BindToggle>& into,
                                 std::vector<std::pair<std::string, std::vector<std::string>>>* resourceToggles = nullptr) {
            std::map<std::pair<std::string, std::string>, std::vector<const Row*>> byBinding;
            std::set<std::tuple<std::string, std::string, std::size_t, long long, std::string>> seen;
            for (const Row& row : data.collected(rows)) {
                if (row.graphId.modObj.first == IniGraphModObjKeywords::Texture
                        || !seen.emplace(row.rootSection, row.section, row.partId, row.line, row.key).second) {
                    continue;
                }
                byBinding[{row.rootSection, lowered(stripped(row.key))}].push_back(&row);
            }

            for (auto& [binding, bound] : byBinding) {
                std::stable_sort(bound.begin(), bound.end(), [](const Row* a, const Row* b) {
                    return std::make_pair(a->partId, a->line) < std::make_pair(b->partId, b->line);
                });

                bool real = false;
                std::vector<std::string> files;
                std::vector<std::string> resources;
                for (const Row* row : bound) {
                    for (const std::string& guard : row->guards) {
                        std::string test = lowered(guard);
                        test.erase(std::remove_if(test.begin(), test.end(),
                                                  [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }),
                                   test.end());
                        real = real || std::find(notToggles.begin(), notToggles.end(), test) == notToggles.end();
                    }
                    const std::optional<std::string> file = fileOf(row->value);
                    if (file.has_value() && std::find(files.begin(), files.end(), *file) == files.end()) {
                        files.push_back(*file);
                        resources.push_back(IniNamingTools::removeRefPrefix(stripped(row->value)));
                    }
                }

                if (!real || files.size() < 2) {
                    continue;
                }

                // the value that can hold at the starting values: what the mod shows before a key is pressed
                std::vector<Row> held;
                for (const Row* row : bound) {
                    held.push_back(*row);
                }
                const std::optional<std::string> atStart = fileOf(GraphCollect<>::valueUnder(held, std::string(), start, z3Ctx));
                for (const std::string& file : files) {
                    into.emplace(file, BindToggle{files, atStart.value_or(std::string())});
                }
                if (resourceToggles != nullptr) {
                    resourceToggles->emplace_back(binding.first + " " + binding.second, resources);
                }
            }
        }

        // what a candidate's statement is, for the ranking: how it was stated, and by which hash and section
        struct Statement {
            int tier = 2;                     // 0 its own hash, 1 a register, 2 its pixels
            int age = 0;                      // of its hash, lower newer
            std::string hash;
            std::string section;              // the texture override stating it, in THIS file
            int priority = 0;                 // that section's match_priority
        };
    }


    void IniParseData::roleFiles(const RoleFileRules& rules) {
        const auto log = [&rules](const std::string& message) {
            if (rules.log) {
                rules.log(message);
            }
        };
        const auto ageOf = [&rules](const std::string& hash) {
            return rules.hashAge ? rules.hashAge(hash) : 0;
        };
        const std::unordered_map<std::string, std::size_t> sectionAt = sectionOrder(*this, rules.sections);
        const auto bySection = [&sectionAt](std::vector<std::string>& sections) {
            std::sort(sections.begin(), sections.end(), [&sectionAt](const std::string& a, const std::string& b) {
                return positionOf(sectionAt, a) < positionOf(sectionAt, b);
            });
        };

        // ---- (1) WHAT EACH FILE THE .ini NAMES IS: the roles the .ini states for it ----
        TextureRoles roles;

        // resource -> the file it names: each section's first `filename`. A section a PREVIOUS fix declared counts
        // like any other: `[Resource<Role><Target>RemapRef]` names one of the mod's own textures
        std::unordered_map<std::string, std::string> fileOf;
        std::unordered_set<std::string> declaredSections;
        for (const Row& row : collected(rules.declared)) {
            if (row.key != IniKeywords::Filename || !declaredSections.insert(row.section).second) {
                continue;
            }
            const std::string file = FileService::absPathOfRelPath(FileService::iniPathToRel(stripped(row.value)), rules.folder);
            fileOf[lowered(row.section)] = file;
            roles.addResource(rules.iniPath, row.section, file);
        }

        // each texture override's first `hash` and its match_priority
        std::unordered_map<std::string, std::string> hashOf;
        std::unordered_map<std::string, int> priorityOf;
        for (const Row& row : collected(rules.overrides)) {
            if (row.key == IniKeywords::Hash) {
                hashOf.emplace(row.section, lowered(stripped(row.value)));
            } else if (row.key == IniKeywords::MatchPriority) {
                try {
                    priorityOf.emplace(row.section, std::stoi(stripped(row.value)));
                } catch (const std::exception&) {
                }
            }
        }

        std::map<std::pair<std::string, std::string>, Statement> statements;        // (role, file) -> its best statement
        const auto state = [&statements](const std::string& role, const std::string& file, Statement statement) {
            const auto placed = statements.emplace(std::make_pair(role, file), statement);
            Statement& had = placed.first->second;
            if (!placed.second && std::make_tuple(statement.tier, statement.age) < std::make_tuple(had.tier, had.age)) {
                had = std::move(statement);
            }
        };

        std::unordered_map<std::string, std::vector<const Row*>> byRoot;
        for (const Row& row : collected(rules.roleRows)) {
            byRoot[row.rootSection].push_back(&row);
        }
        std::vector<std::string> rootSections;
        for (const auto& entry : byRoot) {
            rootSections.push_back(entry.first);
        }
        bySection(rootSections);

        for (const std::string& name : rootSections) {
            const std::vector<const Row*>& rows = byRoot[name];

            // the section IS a texture: placed in its role's graph by its hash, and `this` names the file
            for (const Row* row : rows) {
                if (row->graphId.modObj.first != IniGraphModObjKeywords::Texture || row->graphId.modObj.second.empty()
                        || row->key != IniKeywords::This) {
                    continue;
                }
                const auto file = fileOf.find(lowered(stripped(row->value)));
                if (file == fileOf.end()) {
                    continue;
                }
                const auto hash = hashOf.find(name);
                const std::string hashText = hash == hashOf.end() ? std::string() : hash->second;
                const std::string& role = row->graphId.modObj.second;
                roles.add(file->second, role, TextureRoles::DeclaredHash + hashText);

                Statement statement;
                statement.tier = 0;
                statement.age = hashText.empty() ? 0 : ageOf(hashText);
                statement.hash = hashText;
                statement.section = name;
                const auto priority = priorityOf.find(name);
                statement.priority = priority == priorityOf.end() ? 0 : priority->second;
                state(role, file->second, statement);
            }

            // the section is an object's DRAW: its registers say what they bind, through `run =`
            for (const Row* row : rows) {
                const auto table = rules.registerRoles.find(row->graphId.modObj);
                if (table == rules.registerRoles.end()) {
                    continue;
                }
                const auto role = table->second.find(row->key);
                if (role == table->second.end()) {
                    continue;
                }
                const auto file = fileOf.find(lowered(IniNamingTools::removeRefPrefix(stripped(row->value))));
                if (file != fileOf.end()) {
                    roles.add(file->second, role->second, "bound at " + row->key + " of " + row->graphId.modObj.second);
                    Statement statement;
                    statement.tier = 1;
                    state(role->second, file->second, statement);
                }
            }
        }

        // A .dds no hash and no register placed may still be the game's own texture, and its pixels say so -- only
        // the files this .ini names, never the folder's other files (a spare colourway is not the mod's texture)
        if (rules.identify) {
            for (const auto& entry : fileOf) {
                const std::string& file = entry.second;
                if (!StringTools::endsWithIgnoreCase(file, FileExt::DDS) || roles.rolesOf().count(file) > 0 || fixWrote(file)) {
                    continue;
                }
                const std::optional<std::string> hash = rules.identify(file);
                const std::optional<std::string> role = hash.has_value() && rules.roleOfHash
                                                            ? rules.roleOfHash(lowered(*hash)) : std::nullopt;
                if (role.has_value()) {
                    roles.add(file, *role, "the game's own " + *hash + " by its pixels");
                    state(*role, file, Statement{});
                }
            }
        }

        setTextureRoles(std::move(roles));
        const TextureRoles& found = *textureRoles();
        const std::vector<std::pair<std::string, std::string>>& resources = found.resourcesOf(rules.iniPath);

        // ---- (2) THE CANDIDATES: every file stated for a role that the folder holds ----
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> candidates;
        std::map<std::string, std::map<ModObj, std::set<std::string>>> bindings;       // file -> object -> roles it binds
        std::map<std::string, std::set<std::string>> declaredAs;                       // file -> roles its own hash states
        std::set<std::string> missing;
        const auto isMissing = [&missing](const std::string& file) {
            if (exists(file)) {
                return false;
            }
            missing.insert(file);
            return true;
        };

        for (const auto& entry : found.rolesOf()) {
            if (isMissing(entry.first)) {
                continue;
            }
            for (const TextureRoles::Role& role : entry.second) {
                candidates[role.role].emplace_back(entry.first, role.how);
                if (StringTools::startsWith(role.how, TextureRoles::DeclaredHash)) {
                    declaredAs[entry.first].insert(role.role);
                }
            }
        }

        // what each object's OWN sections bind: the first binding of each register of its table, as written
        std::unordered_map<std::string, std::vector<ModObj>> objRoots;
        for (const auto& [obj, sections] : rules.objs) {
            for (const std::string& section : sections) {
                objRoots[section].push_back(obj);
            }
        }
        if (!rules.registerRoles.empty()) {
            std::unordered_map<std::string, std::string> fileByResource;
            for (const auto& entry : resources) {
                fileByResource.emplace(lowered(entry.first), entry.second);
            }

            std::unordered_map<std::string, std::vector<const Row*>> own;
            for (const Row& row : collected(rules.objRows)) {
                const auto root = objRoots.find(row.section);
                if (root != objRoots.end() && std::find(root->second.begin(), root->second.end(), row.graphId.modObj) != root->second.end()) {
                    own[row.section].push_back(&row);
                }
            }
            for (auto& entry : own) {
                std::stable_sort(entry.second.begin(), entry.second.end(), [](const Row* a, const Row* b) {
                    return std::make_tuple(a->position, a->line) < std::make_tuple(b->position, b->line);
                });
            }

            for (const auto& [obj, sections] : rules.objs) {
                const auto layout = rules.registerRoles.find(obj);
                if (layout == rules.registerRoles.end()) {
                    continue;
                }
                for (const std::string& section : sections) {
                    const auto rows = own.find(section);
                    if (rows == own.end()) {
                        continue;
                    }
                    for (const auto& [reg, role] : layout->second) {
                        std::optional<std::string> bound;
                        for (const Row* row : rows->second) {
                            if (IniNamingTools::sameKey(row->key, reg)) {
                                bound = stripped(row->value);
                                break;
                            }
                        }
                        if (!bound.has_value()) {
                            continue;
                        }
                        const auto file = fileByResource.find(lowered(IniNamingTools::removeRefPrefix(*bound)));
                        if (file == fileByResource.end() || isMissing(file->second)) {
                            continue;
                        }
                        candidates[role].emplace_back(file->second, "the " + reg + " its own section binds it at");
                        bindings[file->second][obj].insert(role);
                        Statement statement;
                        statement.tier = 1;
                        state(role, file->second, statement);
                    }
                }
            }
        }

        for (const std::string& file : missing) {
            log(FileService::pathToStr(FileService::strToPath(file).filename())
                + " is declared but not in the mod's folder; its role is treated as one the mod has no file for");
        }

        // A LINKED .ini: its texture-only recolours of this character's textures (the role is stated by the hash),
        // then the textures it binds on one of this file's draws -- on the mod's own character both fire, and
        // remapped neither does, so every role they serve would be downloaded from the game over the mod's art
        // ...and in this file each such toggle is a register toggle (#registerToggleOf), so a fixer editing one variant
        // edits them all, when the resource holds none yet
        Z3Context z3;
        std::map<std::string, BindToggle> bindToggles;
        std::vector<std::pair<std::string, std::vector<std::string>>> ownToggles;
        registerBindToggles(*this, rules.roleRows, [&fileOf](const std::string& value) -> std::optional<std::string> {
            const auto file = fileOf.find(lowered(IniNamingTools::removeRefPrefix(stripped(value))));
            return file == fileOf.end() ? std::nullopt : std::optional<std::string>(file->second);
        }, rules.notToggles, startingValues(rules.sections, z3), &z3, bindToggles, &ownToggles);
        for (const auto& [section, resources] : ownToggles) {
            for (const std::string& resource : resources) {
                if (registerToggleOf(lowered(resource)) == nullptr) {
                    setRegisterToggle(lowered(resource), TextureToggle{section, resources, false});
                }
            }
        }
        if (!rules.objs.empty() && !rules.iniPath.empty()) {
            std::size_t recoloured = 0;
            for (const RoleFileRules::Linked& linked : rules.linked) {
                if (linked.data == nullptr) {
                    continue;
                }
                const IniParseData& other = *linked.data;
                std::unordered_map<std::string, std::string> otherHash;
                for (const Row& row : other.collected(rules.overrides)) {
                    if (row.key == IniKeywords::Hash) {
                        otherHash.emplace(row.section, lowered(stripped(row.value)));
                    }
                }

                // each texture override's FIRST `this`, in the order the sections are written
                const std::unordered_map<std::string, std::size_t> otherAt = sectionOrder(other, rules.sections);
                std::map<std::string, const Row*> firstThis;
                for (const Row& row : other.collected(rules.roleRows)) {
                    if (row.graphId.modObj.first != IniGraphModObjKeywords::Texture || row.graphId.modObj.second.empty()
                            || row.key != IniKeywords::This || !StringTools::startsWith(row.section, IniKeywords::TextureOverride)
                            || otherHash.count(row.section) == 0) {
                        continue;
                    }
                    const auto placed = firstThis.emplace(row.section, &row);
                    if (!placed.second && std::make_pair(row.partId, row.line)
                                              < std::make_pair(placed.first->second->partId, placed.first->second->line)) {
                        placed.first->second = &row;
                    }
                }
                std::vector<std::string> sections;
                for (const auto& entry : firstThis) {
                    sections.push_back(entry.first);
                }
                std::stable_sort(sections.begin(), sections.end(), [&otherAt](const std::string& a, const std::string& b) {
                    return positionOf(otherAt, a) < positionOf(otherAt, b);
                });

                for (const std::string& section : sections) {
                    const Row* row = firstThis[section];
                    const std::optional<std::string> file = fileOfResource(other, linked.folder, row->value);
                    if (!file.has_value() || !exists(*file)) {
                        continue;
                    }
                    const std::string& hash = otherHash[section];
                    const std::string& role = row->graphId.modObj.second;
                    candidates[role].emplace_back(*file, "a sibling .ini overrides " + hash + ", the game texture it replaces");
                    Statement statement;
                    statement.tier = 0;
                    statement.age = ageOf(hash);
                    statement.hash = hash;
                    state(role, *file, statement);
                    ++recoloured;
                }
            }
            if (recoloured > 0) {
                log("a sibling .ini recolours " + std::to_string(recoloured)
                    + " of the game's textures; carried over to the target, which binds its own and would never see those hashes");
            }

            std::size_t bound = 0;
            for (const RoleFileRules::Linked& linked : rules.linked) {
                if (linked.data == nullptr) {
                    continue;
                }
                const IniParseData& other = *linked.data;
                registerBindToggles(other, rules.roleRows, [&other, &linked](const std::string& value) {
                    return fileOfResource(other, linked.folder, value);
                }, rules.notToggles, other.startingValues(rules.sections, z3), &z3, bindToggles);

                // every value an object's sections bind, through `run =`, at a register of its table -- the sections
                // as written, each object in order, each register in the table's order, each part counted once
                std::unordered_map<std::string, std::vector<const Row*>> otherRoots;
                std::set<std::tuple<std::string, std::string, std::size_t, long long, std::string>> seen;
                for (const Row& row : other.collected(rules.roleRows)) {
                    if (rules.registerRoles.count(row.graphId.modObj) > 0
                            && seen.emplace(row.rootSection, row.section, row.partId, row.line, row.key).second) {
                        otherRoots[row.rootSection].push_back(&row);
                    }
                }
                const std::unordered_map<std::string, std::size_t> otherAt = sectionOrder(other, rules.sections);
                std::vector<std::string> sections;
                for (const auto& entry : otherRoots) {
                    sections.push_back(entry.first);
                }
                std::sort(sections.begin(), sections.end(), [&otherAt](const std::string& a, const std::string& b) {
                    return positionOf(otherAt, a) < positionOf(otherAt, b);
                });

                for (const std::string& section : sections) {
                    const std::vector<const Row*>& rows = otherRoots[section];
                    std::set<ModObj> objs;
                    for (const Row* row : rows) {
                        objs.insert(row->graphId.modObj);
                    }
                    for (const auto& entry : rules.objs) {
                        const ModObj& obj = entry.first;
                        const auto layout = rules.registerRoles.find(obj);
                        if (objs.count(obj) == 0 || layout == rules.registerRoles.end()) {
                            continue;
                        }
                        for (const auto& [reg, role] : layout->second) {
                            for (const Row* row : rows) {
                                if (row->graphId.modObj != obj || !IniNamingTools::sameKey(row->key, reg)) {
                                    continue;
                                }
                                const std::optional<std::string> file = fileOfResource(other, linked.folder, row->value);
                                if (!file.has_value() || !exists(*file)) {
                                    continue;
                                }
                                candidates[role].emplace_back(*file, "the " + reg + " " + linked.name + " binds on the same draw");
                                bindings[*file][obj].insert(role);
                                Statement statement;
                                statement.tier = 1;
                                state(role, *file, statement);
                                ++bound;
                            }
                        }
                    }
                }
            }
            if (bound > 0) {
                log("another .ini of the mod binds " + std::to_string(bound)
                    + " textures on this file's draws; carried over to the target, where that .ini's overrides never fire");
            }
        }

        // ---- (3) THE TOGGLES: a file's variants, in the order its toggle names them ----
        std::unordered_map<std::string, std::vector<std::string>> resourcesOfFile;
        std::unordered_map<std::string, std::string> fileByResource;
        for (const auto& [resource, file] : resources) {
            fileByResource.emplace(lowered(resource), file);
            resourcesOfFile[file].push_back(lowered(resource));
        }
        const auto variantsOf = [&](const std::string& file) {
            std::vector<std::string> files;
            const auto ownResources = resourcesOfFile.find(file);
            for (const std::string& resource : ownResources != resourcesOfFile.end() ? ownResources->second : std::vector<std::string>{}) {
                for (const TextureToggle* toggle : {thisToggleOf(resource), registerToggleOf(resource)}) {
                    if (toggle == nullptr || toggle->variants.size() < 2) {
                        continue;
                    }
                    for (const std::string& variant : toggle->variants) {
                        const auto variantFile = fileByResource.find(lowered(variant));
                        if (variantFile != fileByResource.end()
                                && std::find(files.begin(), files.end(), variantFile->second) == files.end()) {
                            files.push_back(variantFile->second);
                        }
                    }
                }
            }
            if (files.size() >= 2) {
                return files;
            }
            const auto bound = bindToggles.find(file);
            return bound != bindToggles.end() ? bound->second.files : std::vector<std::string>{};
        };

        // the variant that shows at the starting values, for a toggle the reader found itself -- through `run =` or in a
        // linked .ini. A toggle IniParseData already held keeps its first-named variant: the fixer carries its lines
        const auto atStartOf = [&](const std::string& file) {
            const auto ownResources = resourcesOfFile.find(file);
            for (const std::string& resource : ownResources != resourcesOfFile.end() ? ownResources->second : std::vector<std::string>{}) {
                for (const TextureToggle* toggle : {thisToggleOf(resource), registerToggleOf(resource)}) {
                    if (toggle != nullptr && toggle->variants.size() >= 2
                            && std::none_of(ownToggles.begin(), ownToggles.end(), [&toggle](const auto& own) { return own.first == toggle->section; })) {
                        return std::string();
                    }
                }
            }
            const auto bound = bindToggles.find(file);
            return bound != bindToggles.end() ? bound->second.atStart : std::string();
        };

        // ---- (4) THE CHOICE ----
        // one entry per file: a file found more than one way keeps the first way, the more specific
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> byRole;
        for (const auto& [role, list] : candidates) {
            std::vector<std::pair<std::string, std::string>>& unique = byRole[role];
            for (const auto& candidate : list) {
                if (std::none_of(unique.begin(), unique.end(), [&](const auto& kept) { return kept.first == candidate.first; })) {
                    unique.push_back(candidate);
                }
            }
        }

        // a region role is refused a file one object also binds for another role (a mask pointed at the slot's
        // normal map), and a flat file (it marks no regions)
        for (auto& [role, list] : byRole) {
            if (rules.regionRoles.count(role) == 0) {
                continue;
            }
            std::vector<std::pair<std::string, std::string>> kept;
            for (const auto& candidate : list) {
                const auto perObj = bindings.find(candidate.first);
                const bool aliased = perObj != bindings.end()
                    && std::any_of(perObj->second.begin(), perObj->second.end(),
                                   [&role](const auto& obj) { return obj.second.count(role) > 0 && obj.second.size() > 1; });
                if (aliased) {
                    reject(candidate.first, role, "is bound for another role of its own slot too, so it is not the mod's " + role);
                } else {
                    kept.push_back(candidate);
                }
            }
            list = std::move(kept);
        }
        for (auto& [role, list] : byRole) {
            if (rules.regionRoles.count(role) == 0) {
                continue;
            }
            std::vector<std::pair<std::string, std::string>> varying;
            for (const auto& candidate : list) {
                const bool flat = rules.isFlat ? rules.isFlat(candidate.first) : TexThumbprint::channelIsConstant(candidate.first, 0);
                if (flat) {
                    reject(candidate.first, role, "is a flat " + role + ", which marks no regions");
                } else {
                    varying.push_back(candidate);
                }
            }
            list = std::move(varying);
        }

        // the order a file's resource is declared in this .ini; a file of a linked .ini comes after
        std::unordered_map<std::string, std::size_t> declaredAt;
        for (const auto& entry : resources) {
            declaredAt.emplace(entry.second, declaredAt.size());
        }

        using Rank = std::tuple<int, int, int, int, int, std::size_t>;
        for (auto& [role, list] : byRole) {
            // of the sections stating one hash for this role in this file, the one 3DMigoto applies LAST (it runs
            // them all, sorted by match_priority and then section name) ranks first
            std::map<std::string, int> lastApplied;                    // file -> how many of its hash's sections run after it
            for (const auto& candidate : list) {
                const auto mine = statements.find({role, candidate.first});
                if (mine == statements.end() || mine->second.tier != 0 || mine->second.section.empty()) {
                    continue;
                }
                int after = 0;
                for (const auto& other : list) {
                    const auto theirs = statements.find({role, other.first});
                    if (other.first == candidate.first || theirs == statements.end() || theirs->second.tier != 0
                            || theirs->second.section.empty() || theirs->second.hash != mine->second.hash) {
                        continue;
                    }
                    if (std::make_pair(mine->second.priority, mine->second.section)
                            < std::make_pair(theirs->second.priority, theirs->second.section)) {
                        ++after;
                    }
                }
                lastApplied[candidate.first] = after;
            }

            const auto rank = [&](const std::string& file, const ModObj& obj) -> Rank {
                // a file whose own hash declares it as ANOTHER role is not this one
                const auto declared = declaredAs.find(file);
                const int contradicted = declared != declaredAs.end() && !declared->second.empty() && declared->second.count(role) == 0 ? 1 : 0;

                // how specifically the mod binds it on this object: only here, here and elsewhere, nowhere, only elsewhere
                int specificity = 2;
                const auto bound = bindings.find(file);
                if (bound != bindings.end() && !bound->second.empty()) {
                    const auto here = bound->second.find(obj);
                    specificity = here == bound->second.end() ? 3 : (bound->second.size() == 1 ? 0 : 1);
                }

                const auto statement = statements.find({role, file});
                const int tier = statement == statements.end() ? 2 : statement->second.tier;
                const int age = statement == statements.end() || tier != 0 ? 0 : statement->second.age;
                const auto last = lastApplied.find(file);
                const auto at = declaredAt.find(file);
                return {contradicted, specificity, tier, age, last == lastApplied.end() ? 0 : last->second,
                        at == declaredAt.end() ? declaredAt.size() : at->second};
            };

            for (const auto& entry : rules.objs) {
                const ModObj& obj = entry.first;
                if (list.empty()) {
                    break;
                }

                std::vector<std::pair<std::string, std::string>> ranked = list;
                std::stable_sort(ranked.begin(), ranked.end(), [&](const auto& a, const auto& b) {
                    return rank(a.first, obj) < rank(b.first, obj);
                });

                RoleFile chosen{ranked.front().first, ranked.front().second, "", ""};
                std::vector<std::string> variants = variantsOf(ranked.front().first);
                if (!variants.empty()) {
                    // the variants of one toggle play one role: of the candidates the mod does not declare as another
                    // role, the variant that shows at the starting values (a toggle the reader found itself), else the
                    // toggle's first-named
                    const int contradicted = std::get<0>(rank(ranked.front().first, obj));
                    std::vector<std::string> order = variants;
                    const std::string atStart = atStartOf(ranked.front().first);
                    if (!atStart.empty()) {
                        order.erase(std::remove(order.begin(), order.end(), atStart), order.end());
                        order.insert(order.begin(), atStart);
                    }
                    for (const std::string& file : order) {
                        const auto candidate = std::find_if(ranked.begin(), ranked.end(), [&file](const auto& c) { return c.first == file; });
                        if (candidate != ranked.end() && std::get<0>(rank(candidate->first, obj)) == contradicted) {
                            chosen.file = candidate->first;
                            chosen.how = candidate->second;
                            break;
                        }
                    }
                } else if (ranked.size() > 1) {
                    const Rank x = rank(ranked[0].first, obj);
                    const Rank y = rank(ranked[1].first, obj);
                    if (std::get<0>(x) == std::get<0>(y) && std::get<1>(x) == std::get<1>(y) && std::get<2>(x) == std::get<2>(y)
                            && std::get<3>(x) == std::get<3>(y) && std::get<4>(x) == std::get<4>(y)) {
                        chosen.tiedFile = ranked[1].first;
                        chosen.tiedHow = ranked[1].second;
                    }
                }

                if (std::find(variants.begin(), variants.end(), chosen.file) != variants.end()) {
                    chosen.variants = std::move(variants);
                }
                setRoleFile(role, obj, std::move(chosen));
            }
            setRoleCandidates(role, std::move(list));
        }

        // ---- (5) EACH TEXTURE RESOURCE'S FILE, and the resource the mod USES for each file: one file may be declared
        // by several resources, and the one a section of the mod binds reaches the draws ----
        for (const auto& [resource, file] : resources) {
            setTextureResource(resource, file);
        }
        std::unordered_set<std::string> used;
        if (!rules.sections.empty()) {
            for (const std::string& section : collectedSections(rules.sections)) {
                if (rules.fixSection && rules.fixSection(section)) {
                    continue;
                }
                for (const Row* row : sectionRows(rules.sections, section)) {
                    used.insert(lowered(stripped(IniNamingTools::removeRefPrefix(row->value))));
                }
            }
        }
        std::unordered_map<std::string, std::string> usedBy;
        for (const auto& [resource, file] : resources) {
            const auto placed = usedBy.emplace(file, resource);
            if (!placed.second && used.count(lowered(placed.first->second)) == 0 && used.count(lowered(resource)) > 0) {
                placed.first->second = resource;
            }
        }
        for (const auto& [file, resource] : usedBy) {
            setFileResource(file, resource);
        }
    }

    std::optional<Z3Predicate> IniParseData::startingValues(const std::string& sections, Z3Context& z3Ctx) const {
        std::optional<Z3Predicate> start;
        if (sections.empty()) {
            return start;
        }

        std::set<std::string> seen;
        for (const std::string& section : collectedSections(sections)) {
            if (!StringTools::equalsIgnoreCase(section, IniKeywords::Constants)) {
                continue;
            }
            for (const Row* row : sectionRows(sections, section)) {
                // `global $x = 1`, `global persist $x = 1`, `local $x`: the variable is the key's last word
                const std::string key = stripped(row->key);
                const std::size_t at = key.find_last_of(" \t");
                const std::string var = at == std::string::npos ? key : key.substr(at + 1);
                const std::string kind = lowered(key.substr(0, key.find_first_of(" \t")));
                if (var.size() < 2 || var.front() != '$' || (kind != "global" && kind != "local") || !seen.insert(lowered(var)).second) {
                    continue;
                }

                std::string value = stripped(row->value);
                if (value.empty()) {
                    value = "0";
                }
                try {
                    std::size_t used = 0;
                    std::stod(value, &used);
                    if (used != value.size()) {
                        continue;
                    }
                } catch (const std::exception&) {
                    continue;
                }

                const IfPredPart part("if " + var + " == " + value, IfPredPartType::If, z3Ctx);
                if (part.query.has_value()) {
                    start = start.has_value() ? std::optional<Z3Predicate>(*start & *part.query) : part.query;
                }
            }
        }
        return start;
    }
}
