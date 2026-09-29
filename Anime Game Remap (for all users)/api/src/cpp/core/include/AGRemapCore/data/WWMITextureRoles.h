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


#ifndef WWMI_TEXTURE_ROLES_H
#define WWMI_TEXTURE_ROLES_H

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/IniSectionGraph.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What ROLE each of a mod's textures plays -- read off the ``.ini``, never off the folder
     :raw-html:`<br />` :raw-html:`<br />`

     A texture is identified by a **hash and a register**, in both games, and the hash is either

     * the **texture's own**, so the `section`_ IS that texture and its ``this =`` names the file --
       what nearly every Wuthering Waves mod is written with; or
     * the **mesh's**, so the `section`_ is a draw that :cpp:class:`GIMISectionClassifier` places on
       a component, and the ``ps-t<N>`` within it says which role the file it binds plays -- the
       usual GI form, which works in WuWa too and which few mods use (issue #188)

     Either way the question is *which section is this* followed by *which register*, which is
     :cpp:class:`GIMISectionClassifier`'s job, so this is built by the PARSER and handed to the fixer
     through :cpp:class:`WWMIParseFacts` :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        **It does not look at the file system at all**, and that is the point. What replaced it used
        to climb up to three parent folders, walk everything under them for ``.dds``, and then try to
        work out what each file was from its name, a hash embedded in its name, or its pixels. A file
        the mod never binds was therefore a candidate: Chisa7 keeps spare colourways beside the
        installed one, the scan offered a spare for ``upperDiffuse``, and the kimono rendered red
        (2026-09-29). A texture the ``.ini`` does not name is not the mod's texture.
     @endrst
     */
    class WWMITextureRoles {
        public:

            /**
             * @brief
             @rst
             The roles one file plays, and how each was decided -- a file plays EVERY role the
             ``.ini`` binds it for, since one atlas can serve two components
             @endrst
             */
            struct Role {
                /**
                 * @brief The role, eg. ``bodyDiffuse``
                 */
                std::string role;

                /**
                 * @brief How it was decided, for the log
                 */
                std::string how;
            };

            WWMITextureRoles() = default;

            /**
             * @brief Records that 'file' plays 'role', decided by 'how'
             *
             * @param file The texture file, as the ``.ini`` names it resolved against its folder
             * @param role The role it plays
             * @param how How that was decided
             */
            void add(const std::string& file, const std::string& role, const std::string& how);

            /**
             * @brief Records the file a ``.ini``'s resource `section`_ names
             *
             * @param iniPath The ``.ini`` the resource was declared in
             * @param resource The resource `section`_'s name
             * @param file The file it names
             */
            void addResource(const std::string& iniPath, const std::string& resource, const std::string& file);

            /**
             * @brief Every file with the roles it plays
             *
             * @return file -> its roles
             */
            const std::unordered_map<std::string, std::vector<Role>>& rolesOf() const;

            /**
             * @brief The resource `sections`_ one ``.ini`` declares
             *
             * @param iniPath The ``.ini``
             * @return ``(resource, file)`` for each
             */
            const std::vector<std::pair<std::string, std::string>>& resourcesOf(const std::string& iniPath) const;

        private:
            std::unordered_map<std::string, std::vector<Role>> rolesOf_;
            std::map<std::string, std::vector<std::pair<std::string, std::string>>> resourcesByIni_;
    };
}

#endif
