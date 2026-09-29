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

#include "AGRemapCore/data/WWMITextureIndex.h"

#include <algorithm>
#include <regex>


namespace AGRemapCore {

    namespace {

    // WWMI Tools' own export names: "... t=<hash>.dds" and "Component<N>_<Type>.dds".
    const std::regex FileHashPattern(R"(t=([0-9a-fA-F]{8})\.dds$)", std::regex::icase);
    const std::regex ComponentFilePattern(R"(component[\s_-]*(\d+)[\s_-]+([a-z]+)\.dds$)", std::regex::icase);

    // How far above the .ini file its textures may live -- a mod with LOD folders declares them in
    // a parent.
    const int MaxFolderClimb = 3;
    }

    WWMITextureIndex::WWMITextureIndex(const std::string& iniFolder, const WWMITextureFacts& facts, const std::string& remapTexKeyword,
                         const Hashes* libraryHashes, const std::string& sourceName) {
                root_ = findRoot(iniFolder);

                // The roles this config knows what to do with -- the gate on the library
                // lookup below, so a `vb0` or `cb4` row can never be read as a texture role.
                std::unordered_set<std::string> knownRoles;
                for (const auto& entry : facts.roles) {
                    knownRoles.insert(entry.second);
                }

                // A ROLE IS ASKED OF THE LIBRARY FIRST. `facts.roles` is written from one
                // generation of the character's textures and a mod carries whatever hash its
                // author dumped, so a mod a version or two old matched almost nothing and
                // downloaded the GAME's texture for role after role -- someone's painted outfit
                // rendering as the vanilla one, with "downloaded 13 files" in the summary.
                // HashData files them typed by role at every generation.
                //
                // One call, no version: ModMappedAssets keys its buckets by the ASSET, so a
                // hash's buckets are its own generations and a hash that only ever existed at
                // 3.0 has exactly one.
                auto roleOfHash = [&facts, &knownRoles, libraryHashes, &sourceName]
                                  (const std::string& hash) -> std::optional<TextureRole> {
                    if (libraryHashes != nullptr && !sourceName.empty()) {
                        std::optional<std::vector<std::string>> key =
                            libraryHashes->getKey(hash, std::nullopt, {sourceName, std::nullopt}, false);
                        if (key.has_value() && key->size() > 1 && knownRoles.count((*key)[1]) > 0) {
                            return TextureRole{(*key)[1], "hash " + hash + " (the library's history)"};
                        }
                    }

                    auto role = facts.roles.find(hash);
                    if (role != facts.roles.end()) {
                        return TextureRole{role->second, "hash " + hash};
                    }

                    return std::nullopt;
                };

                std::unordered_map<std::string, std::vector<std::string>> hashesOfFile;
                std::vector<std::string> ddsFiles;
                const std::string remapTex = StringTools::toLower(remapTexKeyword);
                const std::string remapFix = StringTools::toLower(IniKeywords::RemapFix);
                for (const std::string& file : FileService::getFilesAndDirs(root_, true).first) {
                    if (isDisabled(file)) {
                        continue;
                    }

                    const std::string name = StringTools::toLower(FileService::baseName(file));
                    if (StringTools::endsWith(name, FileExt::DDS)) {
                        if (name.find(remapTex) == std::string::npos) {
                            const std::string key = FileService::pathKey(file);
                            ddsFiles.push_back(key);
                            real_[key] = file;
                        }

                        continue;
                    }

                    if (!StringTools::endsWith(name, FileExt::Ini) || name.find(remapFix) != std::string::npos) {
                        continue;
                    }

                    const std::string folder = FileService::parentOf(file);
                    std::vector<std::pair<std::string, std::string>> resources;     // in declaration order
                    std::unordered_map<std::string, std::string> fileOfResource;
                    const std::vector<IniScanSection> sections = IniScan::scan(file);
                    for (const IniScanSection& section : sections) {
                        if (!StringTools::startsWith(section.name, IniKeywords::Resource)
                            || StringTools::toLower(section.name).find(remapFix) != std::string::npos) {
                            continue;
                        }

                        std::optional<std::string> fileName = IniScan::firstVal(section, IniKeywords::Filename);
                        if (fileName.has_value() && StringTools::endsWith(StringTools::toLower(*fileName), FileExt::DDS)) {
                            const std::string key = FileService::pathKey(FileService::absPathOfRelPath(*fileName, folder));
                            resources.emplace_back(section.name, key);
                            fileOfResource[section.name] = key;
                        }
                    }

                    for (const IniScanSection& section : sections) {
                        if (!StringTools::startsWith(section.name, IniKeywords::TextureOverride + "Texture")) {
                            continue;
                        }

                        std::optional<std::string> hash = IniScan::firstVal(section, IniKeywords::Hash);
                        std::optional<std::string> resource = IniScan::firstVal(section, IniKeywords::This);
                        if (hash.has_value() && resource.has_value() && fileOfResource.count(*resource) > 0) {
                            hashesOfFile[fileOfResource[*resource]].push_back(StringTools::toLower(*hash));
                        }
                    }

                    resourcesByIni_[FileService::pathKey(file)] = std::move(resources);
                }

                std::vector<std::string> pending;
                for (const std::string& file : ddsFiles) {
                    std::vector<std::string> hashes = hashesOfFile[file];
                    std::smatch match;
                    if (std::regex_search(file, match, FileHashPattern)) {
                        hashes.push_back(StringTools::toLower(match[1].str()));
                    }

                    std::vector<TextureRole> roles;
                    for (const std::string& hash : hashes) {
                        std::optional<TextureRole> role = roleOfHash(hash);
                        if (!role.has_value()) {
                            continue;
                        }

                        const bool known = std::any_of(roles.begin(), roles.end(),
                                                       [&](const TextureRole& r) { return r.role == role->role; });
                        if (!known) {
                            roles.push_back(*role);
                        }
                    }

                    if (!roles.empty()) {
                        rolesOf_[file] = std::move(roles);
                        ++byHash_;
                    } else {
                        pending.push_back(file);
                    }
                }

                for (const std::string& file : pending) {
                    std::optional<std::string> hash;
                    if (facts.identifyTexture) {
                        hash = facts.identifyTexture(real_[file]);
                    }

                    if (!hash.has_value()) {
                        hash = TexThumbprint::identifyFile(real_[file], facts.textureThumbprints, facts.thumbprintSize,
                                                  facts.identityMin, facts.identityGap);
                    }

                    if (hash.has_value()) {
                        std::optional<TextureRole> role = roleOfHash(StringTools::toLower(*hash));
                        if (role.has_value()) {
                            rolesOf_[file] = {TextureRole{role->role, "the game's own " + *hash + " by its pixels"}};
                            ++byPixels_;
                            continue;
                        }
                    }

                    std::smatch match;
                    const std::string name = FileService::baseName(file);
                    if (std::regex_search(name, match, ComponentFilePattern)) {
                        const int component = std::stoi(match[1].str());
                        auto type = TypeOfSuffix.find(StringTools::toLower(match[2].str()));
                        auto typeRoles = facts.typeRoles.find(component);
                        if (type != TypeOfSuffix.end() && typeRoles != facts.typeRoles.end()) {
                            auto role = typeRoles->second.find(type->second);
                            if (role != typeRoles->second.end()) {
                                rolesOf_[file] = {TextureRole{role->second, "its name"}};
                                ++byName_;
                                continue;
                            }
                        }
                    }

                    unresolved_.push_back(file);
                }
            }

    std::string WWMITextureIndex::real(const std::string& key) const {
                auto it = real_.find(key);
                return it == real_.end() ? key : it->second;
            }

    const std::vector<std::pair<std::string, std::string>>& WWMITextureIndex::resourcesOf(const std::string& iniPath) const {
                static const std::vector<std::pair<std::string, std::string>> none;
                auto it = resourcesByIni_.find(FileService::pathKey(iniPath));
                return it == resourcesByIni_.end() ? none : it->second;
            }

    std::string WWMITextureIndex::findRoot(const std::string& iniFolder) {
                std::string folder = iniFolder;
                for (int i = 0; i < MaxFolderClimb; ++i) {
                    const std::string parent = FileService::parentOf(folder);
                    if (parent.empty() || parent == folder) {
                        break;
                    }

                    bool holdsIni = false;
                    for (const std::string& file : FileService::getFilesAndDirs(parent, false).first) {
                        const std::string name = FileService::baseName(file);
                        if (StringTools::endsWith(StringTools::toLower(name), FileExt::Ini) && !IniNamingTools::isDisabled(name)) {
                            holdsIni = true;
                            break;
                        }
                    }

                    if (!holdsIni) {
                        break;
                    }

                    folder = parent;
                }

                return folder;
            }

    bool WWMITextureIndex::isDisabled(const std::string& file) const {
                const std::string rel = FileService::pathKey(FileService::getRelPath(file, root_));
                std::size_t start = 0;
                while (start <= rel.size()) {
                    const std::size_t end = rel.find('/', start);
                    const std::string part = rel.substr(start, end == std::string::npos ? std::string::npos : end - start);
                    if (IniNamingTools::isDisabled(part)) {
                        return true;
                    }

                    if (end == std::string::npos) {
                        break;
                    }

                    start = end + 1;
                }

                return false;
            }
}
