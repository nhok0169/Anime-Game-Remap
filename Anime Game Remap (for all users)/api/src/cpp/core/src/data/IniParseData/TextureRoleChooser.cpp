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

#include "AGRemapCore/data/IniParseData/TextureRoleChooser.h"

#include <algorithm>
#include <tuple>

#include "AGRemapCore/model/textures/TexThumbprint.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    void TextureRoleChooser::choose(IniParseData& data, const std::vector<ModObj>& modObjs) const {
        // ONE ENTRY PER FILE. A file is routinely found for a role more than one way -- by its hash
        // AND by the register its own section binds it at -- and two entries naming one path rank
        // identically, which reported a tie with the SAME file on both sides of it. The first way it
        // was found is kept, which is the more specific one.
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> byRole;
        for (const auto& [role, found] : candidates) {
            std::vector<std::pair<std::string, std::string>>& unique = byRole[role];
            for (const auto& candidate : found) {
                const bool seen = std::any_of(unique.begin(), unique.end(),
                                              [&](const auto& kept) { return kept.first == candidate.first; });
                if (!seen) {
                    unique.push_back(candidate);
                }
            }
        }

        // A REGION ROLE SATISFIED BY A FILE ONE OBJECT ALSO BINDS FOR ANOTHER ROLE IS NOT THAT ROLE
        // (2026-09-27). A mod may point its mask and its normal map at ONE resource -- Chisa13 does,
        // for both hair slots -- and the target's shader then reads slope data as material codes.
        // Narrow on purpose: only when the SAME object binds the file for both. A file legitimately
        // plays every role its hashes name, across objects, and that is untouched here.
        for (auto& [role, found] : byRole) {
            if (regionRoles.count(role) == 0) {
                continue;
            }

            std::vector<std::pair<std::string, std::string>> kept;
            for (const auto& candidate : found) {
                const auto perObj = bindings.find(candidate.first);
                bool aliased = false;
                if (perObj != bindings.end()) {
                    for (const auto& obj : perObj->second) {
                        if (obj.second.count(role) > 0 && obj.second.size() > 1) {
                            aliased = true;
                            break;
                        }
                    }
                }

                if (!aliased) {
                    kept.push_back(candidate);
                    continue;
                }

                data.reject(candidate.first, role, "is bound for another role of its own slot too, so it is not the mod's " + role);
            }

            found = std::move(kept);
        }

        // A FLAT file for a region role marks no regions, and is refused the same way.
        for (auto& [role, found] : byRole) {
            if (regionRoles.count(role) == 0) {
                continue;
            }

            std::vector<std::pair<std::string, std::string>> varying;
            for (const auto& candidate : found) {
                const bool flat = isFlat ? isFlat(candidate.first) : TexThumbprint::channelIsConstant(candidate.first, 0);
                if (!flat) {
                    varying.push_back(candidate);
                    continue;
                }

                data.reject(candidate.first, role, "is a flat " + role + ", which marks no regions");
            }

            found = std::move(varying);
        }

        for (auto& [role, found] : byRole) {
            for (const ModObj& modObj : modObjs) {
                if (found.empty()) {
                    break;
                }

                std::vector<std::pair<std::string, std::string>> ranked = found;
                std::sort(ranked.begin(), ranked.end(), [&](const auto& a, const auto& b) {
                    return rank(role, a.first, modObj) < rank(role, b.first, modObj);
                });

                const auto equal = [&](const std::string& a, const std::string& b) {
                    const Rank x = rank(role, a, modObj);
                    const Rank y = rank(role, b, modObj);
                    return std::get<0>(x) == std::get<0>(y) && std::get<1>(x) == std::get<1>(y)
                           && std::get<2>(x) == std::get<2>(y) && std::get<3>(x) == std::get<3>(y);
                };

                IniParseData::RoleFile chosen{ranked.front().first, ranked.front().second, "", ""};
                std::vector<std::string> variants = variantsOf ? variantsOf(ranked.front().first) : std::vector<std::string>{};
                const auto isVariant = [&variants](const std::string& file) {
                    return std::find(variants.begin(), variants.end(), file) != variants.end();
                };

                if (!variants.empty()) {
                    // VARIANTS OF ONE TOGGLE ARE NOT RANKED AGAINST EACH OTHER: they play one role, so how
                    // specifically each is bound says nothing (a toggle repeated on one object's section binds
                    // every variant there). The toggle's first-named variant the mod does not declare as
                    // another role is the one chosen
                    const int declaredElsewhere = std::get<0>(rank(role, ranked.front().first, modObj));
                    for (const std::string& file : variants) {
                        const auto candidate = std::find_if(ranked.begin(), ranked.end(),
                                                            [&file](const auto& entry) { return entry.first == file; });
                        if (candidate != ranked.end() && std::get<0>(rank(role, candidate->first, modObj)) == declaredElsewhere) {
                            chosen.file = candidate->first;
                            chosen.how = candidate->second;
                            break;
                        }
                    }
                } else if (ranked.size() > 1 && equal(ranked[0].first, ranked[1].first)) {
                    chosen.tiedFile = ranked[1].first;
                    chosen.tiedHow = ranked[1].second;
                }

                if (isVariant(chosen.file)) {
                    chosen.variants = std::move(variants);
                }

                data.setRoleFile(role, modObj, std::move(chosen));
            }

            data.setRoleCandidates(role, std::move(found));
        }
    }


    TextureRoleChooser::Rank TextureRoleChooser::rank(const std::string& role, const std::string& file,
                                                      const ModObj& modObj) const {
        // A FILE THE MOD DECLARES AS ANOTHER ROLE IS NOT THIS ONE (2026-09-29). Chisa3 binds its front
        // hair NORMAL at the register its character reads as the mask, so the normal map won the mask
        // role. Only a file whose declared roles EXIST and exclude this one is demoted: a mod may alias
        // one file onto several hashes (Chisa2 puts seven on one resource).
        int contradicted = 0;
        const auto declaredRoles = declared.find(file);
        if (declaredRoles != declared.end() && !declaredRoles->second.empty() && declaredRoles->second.count(role) == 0) {
            contradicted = 1;
        }

        std::string rel = FileService::pathKey(FileService::getRelPath(file, folder));
        std::size_t ups = 0;
        std::size_t pos = 0;
        while ((pos = rel.find("../", pos)) != std::string::npos) {
            ++ups;
            pos += 3;
        }

        // WHAT THE MOD SAYS, NOT WHAT THE FILE IS CALLED (2026-09-29): a mod author may call a texture
        // anything, so a file's name is a guess about a fact the bindings state outright. A file no
        // section binds makes no claim, which is not evidence against it.
        int specificity = 2;
        const auto bound = bindings.find(file);
        if (bound != bindings.end() && !bound->second.empty()) {
            const auto here = bound->second.find(modObj);
            if (here != bound->second.end()) {
                specificity = bound->second.size() == 1 ? 0 : 1;
            } else {
                specificity = 3;                           // bound, and NOT by this one
            }
        }

        return std::make_tuple(contradicted, specificity, declaredFiles.count(file) > 0 ? 0 : 1, ups, rel.size(), rel);
    }
}
