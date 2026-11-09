#ifndef AGRemapCore_WWMINames_H
#define AGRemapCore_WWMINames_H

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

#include <string>

#include "AGRemapCore/model/assets/Hashes.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     The names a Wuthering Waves fix reads and writes, beside :cpp:struct:`IniKeywords`: the rows its parser files,
     the parts of the sections and files its fixer names, and the words of its own small syntaxes. One place, so a
     parser or fixer built from `Python`_ spells them as the compiled ones do
     @endrst
     */
    struct WWMINames {
        // ---- the rows a WWMI parser files (IniParseData::collected), besides IniParseRows ----

        /**
         * @brief Each texture override's ``hash``, ``this`` and ``match_priority``
         */
        static inline const std::string TextureOverridesRows = "WWMIParser.textureOverrides";

        /**
         * @brief The texture roles the ``.ini`` states, with their conditions
         */
        static inline const std::string TextureRolesRows = "WWMIParser.textureRoles";

        /**
         * @brief Each texture override's own ``this`` and texture registers, with the branches they sit in
         */
        static inline const std::string TextureTogglesRows = "WWMIParser.textureToggles";

        /**
         * @brief What a linked ``.ini`` sets on a slot's draw that a fix carries as it is (RabbitFX's glow map)
         */
        static inline const std::string CarriedRows = "WWMIParser.carried";

        // ---- the parts of the names a WWMI fixer gives its sections, lists and files ----

        /**
         * @brief A draw's number in a section or list named after it
         */
        static inline const std::string Draw = "Draw";

        /**
         * @brief The list run after a draw, and the variable that latches it
         */
        static inline const std::string DrawAfter = "DrawAfter";

        /**
         * @brief A component drawn by another of the target's passes too
         */
        static inline const std::string Pass = "Pass";

        /**
         * @brief A draw kept off some of a slot's passes
         */
        static inline const std::string OffPass = "OffPass";

        /**
         * @brief A draw sent to another slot's pass
         */
        static inline const std::string Reroute = "Reroute";

        /**
         * @brief A component's overlay draw (:cpp:member:`WWMIFixerConfig::SourceComponent::overlay`)
         */
        static inline const std::string Overlay = "Overlay";

        /**
         * @brief A component's mark lines (:cpp:member:`WWMIFixerConfig::SourceComponent::markLines`)
         */
        static inline const std::string Mark = "Mark";

        /**
         * @brief The glow list a linked ``.ini``'s lines are carried into
         */
        static inline const std::string Glow = "Glow";

        /**
         * @brief A file a linked ``.ini`` binds, carried into a resource of the fix's own
         */
        static inline const std::string Carried = "Carried";

        /**
         * @brief The suffix of the variable that runs something once a frame
         */
        static inline const std::string Latch = "Latch";

        /**
         * @brief The list that merges the target's skeleton windows (a target past 256 bones)
         */
        static inline const std::string MergeWindowList = "CommandListMergeWindow";

        /**
         * @brief The resource of zero shape-key offsets a fix binds
         */
        static inline const std::string ShapeKeyZero = "ShapeKeyZero";

        /**
         * @brief The suffix of a resource that binds the source's game texture
         */
        static inline const std::string GameSuffix = "Game";

        /**
         * @brief The texcoord buffer a fix writes with no ``NaN``
         */
        static inline const std::string TexcoordNoNaN = "TexcoordNoNaN";

        /**
         * @brief The blend remap buffers a fix writes
         */
        static inline const std::string BlendRemap = "BlendRemap";

        /**
         * @brief What a shape-key checksum the library cannot look up is written as
         */
        static inline const std::string ChecksumNotFound = "ChecksumNotFound";

        // ---- the hash types HashData files a WuWa texture under ----

        /**
         * @brief A role's hash the game binds once the texture's mips have streamed in: ``<role>Live``
         *        (:cpp:member:`Hashes::LiveSuffix`)
         */
        static inline const std::string& LiveSuffix = Hashes::LiveSuffix;

        /**
         * @brief A role's older hash: ``<role>Old``, ``<role>Old2``, ... (:cpp:member:`Hashes::OldSuffix`)
         */
        static inline const std::string& OldSuffix = Hashes::OldSuffix;

        // ---- the words of a WWMIFixerConfig::Shader line's value ----

        /**
         * @brief ``role <name>``: the file the component's texture list binds for that role
         */
        static inline const std::string RoleLine = "role";

        /**
         * @brief ``stash <slot> <reg>``: what a register held on that slot's own draw (WWMIFixerConfig::stashRegs)
         */
        static inline const std::string StashLine = "stash";
    };
}

#endif
