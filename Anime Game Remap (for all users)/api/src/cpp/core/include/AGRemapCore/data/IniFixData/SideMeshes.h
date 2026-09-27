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

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/Version.h"

namespace AGRemapCore {
    /**
     * @brief
     @rst
     A mod's sections on the SOURCE character's side meshes, written again on the TARGET's
     :raw-html:`<br />` :raw-html:`<br />`

     A side mesh is one of a character's own draws that is no mod object -- the face, the head-upper --
     and a mod may hide one by hash to put something of its own in its place (Neuvillette9's mask:
     ``ib = null`` on Neuvillette's face meshes). The other character draws its OWN side meshes under
     different hashes, so the mod's section reaches nothing there, and the face showed through the mask
     in pieces (2026-09-26). Shared by both multi-component templates: the component one
     (:cpp:member:`GIMIComponentFixerConfig::sideMeshes`, a character onto a skin) and the merge one
     (:cpp:member:`GIMIMergeFixerConfig::sideMeshes`, a skin onto a character) -- the same trap, both ways
     @endrst
     */
    class SideMeshes {
        public:
            SideMeshes() = delete;

            /**
             * @brief
             @rst
             The re-issued sections: each section of 'fileTxt' whose ``hash`` is one of the SOURCE's
             side meshes of a type in 'types', with its body copied and its ``hash`` replaced by the
             TARGET's hash of the same type. The section is renamed with the target's name and the Remap
             keyword appended, so an undo takes it with the rest of the fix. A mesh both characters SHARE
             (the same hash on both sides) is left to the mod's own section. Empty when there is nothing
             to write
             @endrst
             *
             * @param fileTxt The mod's ``.ini`` text
             * @param hashes The hash table both characters' side meshes are filed in
             * @param srcName The source's mod type name
             * @param fromVersion The version the mod is written for
             * @param types The side-mesh hash types, eg. ``{"ib_face", "ib_headupper"}``
             * @param targetName The name the target's side-mesh rows are filed under
             * @param toVersion The version the fix is for
             *
             * @return The sections, with a leading comment, or empty
             */
            static std::string build(const std::string& fileTxt, Hashes& hashes, const std::string& srcName,
                                     const std::optional<Version>& fromVersion, const std::vector<std::string>& types,
                                     const std::string& targetName, const std::optional<Version>& toVersion);
    };
}
