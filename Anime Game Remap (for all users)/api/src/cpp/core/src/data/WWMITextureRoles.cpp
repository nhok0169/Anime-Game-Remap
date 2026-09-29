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

#include "AGRemapCore/data/WWMITextureRoles.h"

#include <algorithm>

#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {

    void WWMITextureRoles::add(const std::string& file, const std::string& role, const std::string& how) {
        if (file.empty() || role.empty()) {
            return;
        }

        std::vector<Role>& roles = rolesOf_[file];

        // A FILE PLAYS EVERY ROLE THE .INI BINDS IT FOR. One atlas can serve two components --
        // Upper_D.dds is both the arm skin's and the bodice's diffuse -- and taking only the first
        // left the second component unbound, drawing with the TARGET's own textures (2026-09-19).
        // The same role twice is just the mod binding it in more than one place.
        const bool known = std::any_of(roles.begin(), roles.end(),
                                       [&role](const Role& r) { return r.role == role; });
        if (known) {
            return;
        }

        roles.push_back(Role{role, how});
    }

    void WWMITextureRoles::addResource(const std::string& iniPath, const std::string& resource,
                                        const std::string& file) {
        if (resource.empty() || file.empty()) {
            return;
        }

        resourcesByIni_[FileService::pathKey(iniPath)].emplace_back(resource, file);
    }

    const std::unordered_map<std::string, std::vector<WWMITextureRoles::Role>>& WWMITextureRoles::rolesOf() const {
        return rolesOf_;
    }

    const std::vector<std::pair<std::string, std::string>>& WWMITextureRoles::resourcesOf(
            const std::string& iniPath) const {
        static const std::vector<std::pair<std::string, std::string>> none;
        auto it = resourcesByIni_.find(FileService::pathKey(iniPath));
        return it == resourcesByIni_.end() ? none : it->second;
    }

}
