#ifndef AGRemapCore_WWMIFixer_H
#define AGRemapCore_WWMIFixer_H

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

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     What one Wuthering Waves remap does differently -- everything :cpp:func:`makeWWMIFixer` needs
     that is not the same for every pair of WWMI characters :raw-html:`<br />` :raw-html:`<br />`

     A WWMI character is drawn in SEVERAL COMPONENTS (bangs, hair, face, torso, ...), each a range of
     one index buffer matched by the character's ``vb0`` hash plus a ``match_first_index``, and all
     of them skinned in ONE merged skeleton that the mod's ``[TextureOverrideComponentN]`` sections
     assemble at draw time from ``$\\WWMIv1\\vg_offset`` / ``vg_count``. So a remap between two WuWa
     characters is a **multi-component onto multi-component** remap, and every WuWa pair is: it
     never splits or merges buffers the way the GIMI templates do. What it does instead, per
     ``.ini`` file (read off ``Tools/Misc/Prototypes/sanhuaExorcistFix.py``, confirmed in game on
     four Sanhua mods on 2026-09-19):

     #. **retargets every draw slot the mod has a section for** onto the target slot
        :cpp:member:`plan` names -- the target's ``vb0`` hash, ``match_first_index``,
        ``match_index_count``, ``vg_offset`` and ``vg_count`` written over the source's (the last
        four out of the library's four ``Indices``-shaped WuWa tables), the shape-key checksum
        remapped through :cpp:class:`ShapeKeyChecksums`, and the section and everything it calls
        renamed with the fix suffix. A source component the plan does not name is DROPPED (its graph
        removed), the shape-key sections are hidden (see :cpp:member:`hiddenObjs`)
     #. **binds the mod's textures by register on the target's draws**, never by hash: WuWa
        texture hashes drift with both streaming and game version, so a hash override written today
        is dead tomorrow. Each drawn source component gets a command list that binds the mod's
        texture of each role the plan asks for, gated on the target slot's pixel shaders
        (:cpp:member:`slotPasses`, each tagged by a ``[ShaderOverride]`` of its own), and run right
        after the mod's shared-resource override. Which of the mod's files plays which role is
        decided by a run-level index of every ``.dds`` under the mod (see :cpp:member:`roles`)
     #. **binds a zero shape-key offset stream** on every remapped draw: the torso, face and eye
        shaders read the game's live per-vertex shape-key offsets out of a sixth vertex stream BY
        VERTEX ID, which WWMI never rebinds, so a mod vertex through those slots would take
        whatever offset the target's buffer holds at the same index
     #. **remaps the blend** (:cpp:member:`blendReg`, collected out of the shared-resource override)
        through the library's vertex-group row for the pair -- an 8-byte-a-vertex WWMI layout,
        not GIMI's 32
     #. **writes one remapped section per target draw per file**: several source components landing
        on one target slot (a skin with one torso slot for the source's arm skin, bodice and skirt)
        collide, and a file holding two remapped sections on one draw rendered the whole body wavy.
        The second claimant of a slot lands in a further ``.ini`` group, written as
        ``<name>RemapFix1.ini`` -- the API's merge, exactly as a GIMI merge does it
     #. **skips the target slots nothing is drawn through**, bones still merged, so the skin's own
        geometry does not sit on top of the mod's

     What is deliberately NOT here yet: downloads of the target's own components (a mod missing a
     component draws nothing there), and retargeting the shape keys rather than hiding them
     @endrst
     */
    struct WWMIFixerConfig {
        /**
         * @brief One register of a target slot's draw and the ROLE of the mod texture bound there
         */
        struct Binding {
            /**
             * @brief The register, eg. ``"ps-t0"``
             */
            std::string reg;

            /**
             * @brief
             @rst
             The role (a value of :cpp:member:`roles` or :cpp:member:`typeRoles`, or the name of a
             :cpp:member:`createdTextures` entry). A role no file of the mod has is left unbound,
             so the GAME's texture serves the register
             @endrst
             */
            std::string role;
        };

        /**
         * @brief How one SOURCE component is drawn on the target
         */
        struct SourceComponent {
            /**
             * @brief The target draw slot it goes through
             */
            int slot = 0;

            /**
             * @brief The registers its command list binds, in this order
             */
            std::vector<Binding> bindings;
        };

        /**
         * @brief A texture the fix INVENTS, eg. the flat material mask a target reads skin off
         */
        struct CreatedTexture {
            /**
             * @brief The role a :cpp:member:`Binding` names it by, eg. ``"SkinMask"``
             */
            std::string role;

            /**
             * @brief The solid colour it is filled with
             */
            Colour colour = Colour();

            /**
             * @brief Its width and height. **Default**: ``16``
             */
            int size = 16;
        };

        /**
         * @brief
         @rst
         The character fixed TO -- whose ``vb0`` hash, slot windows, vertex-group offsets and
         shape-key checksum the library's WuWa tables hold
         @endrst
         */
        ModTypeId targetId = ModTypeId::SanhuaExorcist;

        /**
         * @brief
         @rst
         The game version the library files both characters under, used when the ``.ini`` carries
         no version of its own. **Default**: ``"2.5"``
         @endrst
         */
        std::string version = "2.5";

        /**
         * @brief
         @rst
         Per TARGET slot, the pixel shaders that bind that slot's character textures -- read off a
         frame dump of the target (``Tools/Misc/Diagnostics/wwmiDrawTable.py``). A slot's command
         list binds on every one of them and no other pass; a pass binding only globals is not
         listed. Index = slot number
         @endrst
         */
        std::vector<std::vector<std::string>> slotPasses;

        /**
         * @brief
         @rst
         The ``filter_index`` the first distinct shader of :cpp:member:`slotPasses` is tagged with;
         every further one is :cpp:member:`filterStep` higher. **Default**: ``3381.91``, beside
         WWMI's own ``3381.3333`` / ``3381.4444`` / ``3381.7777``
         @endrst
         */
        double filterBase = 3381.91;

        /**
         * @brief See :cpp:member:`filterBase`. **Default**: ``0.01``
         */
        double filterStep = 0.01;

        /**
         * @brief
         @rst
         The ``filter_index`` to tag a particular shader with, overriding the value
         :cpp:member:`filterBase` and :cpp:member:`filterStep` would compute for it -- as a string, so
         the value in the ``.ini`` is exactly what is written here :raw-html:`<br />`
         :raw-html:`<br />`

         3dmigoto keys a ``[ShaderOverride]`` by its shader hash across EVERY loaded ``.ini``, not per
         file or per namespace. Both directions of one pair tag the same shaders -- a skin and its
         character share the hair, face and eye ones -- so a mod fixed one way and a mod fixed the
         other way, both installed, declare overrides on those hashes twice. If the two disagree on
         the value, whichever file 3dmigoto reads last wins and the other mod's ``if ps == <filter>``
         never matches: its textures are silently not bound. So the SECOND pair to be written names
         the first's values here, and gives its own shaders values the first never uses (Sanhua's
         direction computes ``3381.91`` up; the Exorcist's names those and takes ``3381.81`` up for
         the shaders only it tags)
         @endrst
         */
        std::unordered_map<std::string, std::string> filterIndices;

        /**
         * @brief
         @rst
         Source component -> how it is drawn on the target. A source component the mod has a
         section for but this does not name is dropped. Several sources may name one target slot
         (that is the merge, item 5 above); which lands in the mod's own ``.ini`` and which in a
         copy follows the source components' numeric order
         @endrst
         */
        std::map<int, SourceComponent> plan;

        /**
         * @brief
         @rst
         Texture hash -> role, for every version of the source's textures a mod may carry: the
         current hashes, and the older ones the community's hash maps and
         ``Data/Mod Downloads/WuWa/<Name>/<Name>HashLineage.json`` know :raw-html:`<br />`
         :raw-html:`<br />`
         A file's roles are decided in this order: the hashes the override sections (``hash =``
         plus ``this = Resource``) of ANY ``.ini`` of the mod match for it, plus the ``t=<hash>``
         in its file name (how WWMI Tools names an export) -- and a file plays EVERY role those
         hashes name, because a mod declares one file under two hashes when one atlas serves two
         components; then :cpp:member:`identifyTexture`; and last the
         ``Component<N>_<Diffuse|LM|NM>`` name convention through :cpp:member:`typeRoles`
         @endrst
         */
        std::unordered_map<std::string, std::string> roles;

        /**
         * @brief
         @rst
         Source component -> ``{"diffuse" | "mask" | "normal" -> role}``, for a file named by
         component and type and by nothing else (``Component4_NM.dds``: the RabbitFX / WWMI-Tools
         export names). The suffixes accepted: ``diffuse``, ``albedo``, ``base``, ``color``,
         ``colour``, ``d`` (diffuse); ``lm``, ``lightmap``, ``mask``, ``m`` (mask); ``nm``,
         ``normal``, ``normalmap``, ``n`` (normal)
         @endrst
         */
        std::map<int, std::unordered_map<std::string, std::string>> typeRoles;

        /**
         * @brief
         @rst
         The textures the fix invents, each bound wherever a :cpp:member:`Binding` names its role.
         Written into the mod's texture folder as ``<role><target>RemapTex.dds``
         @endrst
         */
        std::vector<CreatedTexture> createdTextures;

        /**
         * @brief
         @rst
         The game folder under ``Data/Mod Downloads`` the source's textures are fetched from.
         **Default**: ``"WuWa"``
         @endrst
         */
        std::string downloadGameFolder = "WuWa";

        /**
         * @brief
         @rst
         The source character's download folder, version folder and file prefix under
         :cpp:member:`downloadGameFolder` -- ``Sanhua`` / ``2_5`` / ``Sanhua`` for
         ``WuWa/Sanhua/2_5/SanhuaTexture<hash>.dds``. Empty (the default) registers no fallback
         download at all
         @endrst
         */
        std::string downloadCharFolder;
        std::string downloadVersionFolder;
        std::string downloadPrefix;

        /**
         * @brief
         @rst
         Role -> the source's texture hash of that role, for a planned role the mod has NO file
         for: the register is bound to the SOURCE's own game texture, downloaded as
         ``<downloadPrefix><Role>RemapDL.dds`` into the mod's texture folder :raw-html:`<br />`
         :raw-html:`<br />`

         The mod's UVs are the source's, so what an unbound register samples on the target's draw
         -- the TARGET's texture -- is wrong by construction: the red-camellia mod ships no bodice
         or skirt mask, and the Exorcist's mask at its UVs put skin codes over cloth, a reddish hue
         over the whole body while every diffuse was right (2026-09-19). The same reasoning as the
         GI templates' texture donor. A role whose hash BOTH skins bind needs no entry: the target's
         texture is the source's. Empty (the default) binds nothing for a missing role
         @endrst
         */
        std::map<std::string, std::string> fallbackTextures;

        /**
         * @brief
         @rst
         An optional hook that names the texture hash a mod file IS -- pixel identity with one of
         the game's own textures -- for a file no hash names and whose name says something else.
         The prototype measures this by image correlation against the download folder
         (``Component6_Diffuse.dds`` of one mod is the ``ps-t5`` ramp by its pixels, not the iris
         its name says). Empty skips the step
         @endrst
         */
        std::function<std::optional<std::string>(const std::string& file)> identifyTexture;

        /**
         * @brief
         @rst
         The game's own textures by hash, each as a THUMBPRINT -- a :cpp:member:`thumbprintSize`
         square grayscale box average of the decoded file, generated from the character's download
         folder by ``Tools/Misc/Diagnostics/wwmiTextureThumbs.py``. A mod file no hash names is
         decoded, thumbprinted the same way and correlated against every entry: it IS the texture
         it correlates at least :cpp:member:`identityMin` with when every other entry stays under
         :cpp:member:`identityGap`. Measured on the cloak mod's 19 files, 16 x 16 grayscale makes
         the same decision as a full-size colour correlation on every one, and needs no download.
         Consulted after :cpp:member:`identifyTexture`; empty skips the step
         @endrst
         */
        std::unordered_map<std::string, std::vector<std::uint8_t>> textureThumbprints;

        /**
         * @brief The side of a thumbprint. **Default**: ``16``
         */
        int thumbprintSize = 16;

        /**
         * @brief The correlation a file needs with ONE thumbprint to be that texture. **Default**: ``0.97``
         */
        double identityMin = 0.97;

        /**
         * @brief The correlation every OTHER thumbprint must stay under. **Default**: ``0.90``
         */
        double identityGap = 0.90;

        /**
         * @brief Whether every remapped draw binds a zero shape-key offset stream (item 3 above). **Default**: ``true``
         */
        bool zeroShapeKeyStream = true;

        /**
         * @brief The register the game reads that stream from. **Default**: ``"vb6"``
         */
        std::string shapeKeyStreamReg = "vb6";

        /**
         * @brief Bytes per vertex of that stream, off the frame dump's ``vb6`` layout. **Default**: ``24``
         */
        int shapeKeyStride = 24;

        /**
         * @brief
         @rst
         The mod objects (of :cpp:struct:`WWMIParserConfig`'s hash-only ones) whose sections are
         commented out of the mod's own text and copied nowhere. **Default**: the two shape-key
         overrides -- the maintainer's working hand remap has them off, and the mod's keys are
         sized for the source's shape-key vertex count, not the target's
         @endrst
         */
        std::vector<std::string> hiddenObjs = {"shapekeyOffsets", "shapekeyScale"};

        /**
         * @brief The register the mod's blend buffer is bound at. **Default**: ``"vb4"``
         */
        std::string blendReg = "vb4";

        /**
         * @brief
         @rst
         The SOURCE character's ``vg_map`` per component -- that component's own bone indices to the
         merged skeleton's, as ``WWMI-Assets``' ``Metadata.json`` holds it. Needed only for a mod
         from before WWMI grew the merged skeleton (a ``WWMI ALPHA-2 INI``, ``required_wwmi_version``
         0.7): such a mod's sections carry no ``vg_offset``, no merge and none of the merged-skeleton
         resources, because a draw could then only address the bones the GAME hands it for that
         component -- so its ``Blend.buf`` holds each component's OWN indices, not the merged ones
         :raw-html:`<br />` :raw-html:`<br />`

         With this, such a mod's blend is read per component and lifted through the map before the
         library's row is applied, and the fix supplies the merged skeleton the mod lacks. Without
         it, the fix REFUSES a mod of that shape rather than remapping local indices as if they were
         merged, which scrambles every bone of the body (SanhuaExorcist3 in game: "the body became a
         noodle mess", 2026-09-19)
         @endrst
         */
        std::map<int, std::vector<int>> sourceVgMaps;

        /**
         * @brief
         @rst
         The float4 slots of the merged skeleton buffers the fix declares for a legacy mod -- WWMI
         Tools' own template's size, whatever the character's bone count. **Default**: ``768``
         @endrst
         */
        int mergedSkeletonSlots = 768;

        /**
         * @brief
         @rst
         WWMI's marker on the game's bone-data constant buffer, which a legacy mod's supplied merge
         is gated on so a pass with something else in that slot cannot merge junk.
         **Default**: ``"3381.7777"``
         @endrst
         */
        std::string boneDataFilter = "3381.7777";

        /**
         * @brief
         @rst
         The command list every slot section runs to bind the mod's buffers; the texture command
         list and the zero stream are added right after it. **Default**:
         ``"CommandListOverrideSharedResources"``
         @endrst
         */
        std::string sharedResourcesList = "CommandListOverrideSharedResources";

        /**
         * @brief The mod object prefix of a draw slot. **Default**: ``"component"``
         */
        std::string slotPrefix = "component";

        /**
         * @brief Source component -> a label for the log, eg. ``"bangs"``
         */
        std::map<int, std::string> sourceLabels;

        /**
         * @brief Target slot -> a label for the log, eg. ``"torso, arms, ribbons"``
         */
        std::map<int, std::string> targetLabels;

        /**
         * @brief What every generated ``.ini`` copy opens with -- see :cpp:member:`GIMIFixer::copyPreamble`
         */
        std::string copyPreamble;
    };


    /**
     * @brief
     @rst
     Builds the fixer that remaps a mod of one Wuthering Waves character onto another -- see
     :cpp:struct:`WWMIFixerConfig`. One fixer for the pair: every source component of the mod's
     ``.ini`` is handled by it, and the target's components are draw slots rather than mod types
     of their own
     @endrst
     * @param config The pair's config
     */
    IniFixBuilder::Factory makeWWMIFixer(WWMIFixerConfig config);
}

#endif
