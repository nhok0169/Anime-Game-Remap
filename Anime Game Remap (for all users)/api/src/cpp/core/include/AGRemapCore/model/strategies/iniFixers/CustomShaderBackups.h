#ifndef AGRemapCore_CustomShaderBackups_H
#define AGRemapCore_CustomShaderBackups_H
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

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     Saves the resource registers a ``[CustomShader...]`` assigns before it runs, and puts them back after
     :raw-html:`<br />` :raw-html:`<br />`

     3DMigoto restores a CustomShader's shaders and render state when it returns, but NOT the resources its section
     assigns. A later draw of the same game draw -- another part on the same slot, drawn by the game's own shader --
     then reads them: a body shaded with another character's material constants renders black. :cpp:func:`wrap` puts a
     save line (``<backup> = ref <reg>``) before the ``run =`` lines of a CustomShader for each register its lines
     assign, and a restore line (``<reg> = ref <backup>``) after them. Each backup is a resource `section`_ of its own,
     which :cpp:func:`section` writes
     @endrst
     */
    class CustomShaderBackups {
        public:
            /**
             * @brief `KVPs`_, in order
             */
            using KVPs = std::vector<std::pair<std::string, std::string>>;

            /**
             * @brief The name of the resource `section`_ that backs up a register
             */
            using BackupName = std::function<std::string(const std::string& reg)>;

            /**
             * @brief Constructs the backups
             *
             * @param backupName The name of the resource `section`_ that backs up a register
             */
            explicit CustomShaderBackups(BackupName backupName);

            /**
             * @brief The name of the resource `section`_ that backs up a register
             */
            BackupName backupName;

            /**
             * @brief Whether a key binds a resource register: ``ps-t<n>``, ``ps-cb<n>``, ``vs-t<n>`` or ``vs-cb<n>``
             *
             * @param key The key
             *
             * @return Whether it binds a resource register
             */
            static bool bindsRegister(const std::string& key);

            /**
             * @brief
             @rst
             'runs' with a save line for each register 'lines' assign before them, and a restore line for each after them,
             the registers in their sorted order. The registers are remembered for :cpp:func:`registers`
             @endrst
             *
             * @param runs The ``run =`` lines of the CustomShaders
             * @param lines The lines the CustomShaders' sections write
             *
             * @return The lines, saving and restoring
             */
            KVPs wrap(const KVPs& runs, const KVPs& lines);

            /**
             * @brief Every register :cpp:func:`wrap` has saved, in their sorted order
             */
            std::vector<std::string> registers() const;

            /**
             * @brief The resource `section`_ that backs up 'reg': empty, named by \ref backupName
             *
             * @param reg The register
             *
             * @return The section
             */
            std::unique_ptr<IfTemplate<std::string, std::string>> section(const std::string& reg) const;

            /**
             * @brief Forgets every register saved so far
             */
            void clear();

        private:
            std::set<std::string> registers_;
    };
}

#endif
