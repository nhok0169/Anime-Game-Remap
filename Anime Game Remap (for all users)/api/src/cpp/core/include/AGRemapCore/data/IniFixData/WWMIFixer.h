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
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/WWMITextureFacts.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
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
     ``.ini`` file:

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
        collide, and a file holding two remapped sections on one draw renders the whole body wavy.
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
             so the GAME's texture serves the register.

             ``"null"`` (:cpp:member:`IniKeywords::Null`) is reserved and means **bind nothing** --
             ``<reg> = null`` is written, and the target's own texture does NOT serve the register.

             .. note::
                 Leaving a register to the game is only safe when what it holds is not indexed by
                 UV: a lookup, a ramp, a matcap. For a UV-MAPPED texture it is wrong by
                 construction, because the geometry under it is the SOURCE's, so the target's art
                 is sampled at UVs it was never authored for -- which renders as irregular blotches
                 that follow the target's layout rather than the mod's. Chisa's hair ``ps-t5``, a
                 2048 x 2048 map, is an example.
             @endrst
             */
            std::string role;

            /**
             * @brief
             @rst
             In :cpp:member:`extraPassRegs` only: the SOURCE component this binding is for, or
             ``-1`` (the default) for every source that reaches the slot.

             Everything else in that table is keyed by the TARGET slot, which is right until two
             sources MERGE onto one -- and then "the diffuse at ``ps-t0``" is a different file per
             source, while the role lookup falls back to a component-agnostic one and so resolves
             either role for either section. Two bindings on one register in one list means the last
             one wins, silently, for whichever source it does not belong to.

             The same shape as GI's ``TexEdit::srcObj``, and for the same reason: a merge is the one
             case a target-keyed table cannot express. ChisaParfait -> Chisa has two merged slots
             (her frilled panel joins the upper body, her hip prop the lower), so both of its merged
             rows carry this
             @endrst
             */
            int srcComponent = -1;
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
             * @brief The role a :cpp:struct:`AGRemapCore::WWMIFixerConfig::Binding` names it by, eg. ``"SkinMask"``
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
         The SOURCE's own game version, when the pair is not filed under one.
         :cpp:member:`version` serves both characters while they share a version, as Sanhua's pair
         does at 2.5; Chisa is 2.8 and her skin 3.5, and the difference matters to any lookup that
         is REVERSE-then-forward.

         ``ModMappedAssets::getKey`` buckets every row holding a value by version and searches only
         the newest bucket at or below the version asked. Chisa and ChisaParfait have the SAME
         shape-key checksum, 2610, so asked at 3.5 the reverse half answers ChisaParfait, the
         forward half asks what ChisaParfait remaps to in a Chisa -> ChisaParfait fix, finds
         nothing, and writes the literal ``ChecksumNotFound`` -- after which ``ShapeKeyOverrider``
         cannot set up and every shape key silently stops being applied. Asked at 2.8 the same
         lookup answers Chisa.

         Empty (the default) falls back to :cpp:member:`version`
         @endrst
         */
        std::string sourceVersion;

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
         The textures the fix invents, each bound wherever a :cpp:struct:`AGRemapCore::WWMIFixerConfig::Binding` names its role.
         Written into the mod's texture folder as ``<role><target>RemapTex.dds``
         @endrst
         */
        std::vector<CreatedTexture> createdTextures;

        /**
         * @brief
         @rst
         Roles whose texture says WHERE something is, so a CONSTANT one from the mod is not usable
         :raw-html:`<br />` :raw-html:`<br />`

         A material mask marks regions. A mod that ships one holding a single value everywhere is not
         saying "no preference": ``R = 255`` everywhere says "all of this is bare skin", and carried
         across faithfully that is what the target's shader is told -- a jacket shaded as skin, or
         bare thighs shaded as cloth :raw-html:`<br />` :raw-html:`<br />`

         A role named here whose only candidate file is flat is treated as a role the mod has NO file
         for, so it takes \ref fallbackTextures like any other: the SOURCE's own texture, which has
         real regions and is authored for the UVs this mesh actually carries. The target's would be
         authored for the target's UV layout, which this mesh does not use :raw-html:`<br />`
         :raw-html:`<br />`

         .. note::
            Only for roles that mark regions. A DIFFUSE may legitimately be one flat colour, and a
            normal map is nearly flat by construction, so naming either here would throw away art the
            mod meant :raw-html:`<br />` :raw-html:`<br />`

            Empty (the default) keeps every candidate whatever its pixels
         @endrst
         */
        std::set<std::string> flatFallsBackToSource;

        /**
         * @brief
         @rst
         Roles like \ref flatFallsBackToSource, except that a flat one is left to the GAME rather
         than replaced by a download :raw-html:`<br />` :raw-html:`<br />`

         Both drop the mod's flat file; they differ in what stands in for it. A body mask takes the
         source's, because the mod's UVs are the source's and its regions land where they belong. A
         HAIR mask is left alone: a flat hair mask such as ``(255, 0, 126, 0)`` shades the crown of
         the hair red, and the register is better left to the game than filled in :raw-html:`<br />`
         :raw-html:`<br />`

         .. note::
            This suppresses the fallback for that role even though \ref fallbackTextures names it.
            A hair mask the mod never ships still downloads -- only a FLAT one is left
            alone, because "the mod gave us nothing" and "the mod gave us something meaningless"
            deserve different answers here
         @endrst
         */
        std::set<std::string> flatLeftToGame;

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
         The SOURCE components drawn a SECOND time, wound the other way with their normals flipped.
         **Default**: empty -- nothing is mirrored, so no character's output moves
         :raw-html:`<br />` :raw-html:`<br />`

         For single-layer cloth whose INSIDE the target's shader lights differently from the
         source's. A skirt is one sheet -- measured, 0 triangles in any of ChisaParfait's 8
         components share three positions with opposite winding -- so where the gaps between a hem's
         scallops show its inside, the target's shader is lighting a back face, and on
         ChisaParfait -> Chisa that drew dark quadrilaterals the skin's own model does not have.

         The twin presents those pixels as a front face with an outward normal: the component's
         index window wound the other way, over a copy of the mod's own vector buffer with every
         normal negated. It is the WuWa counterpart of GI's
         :cpp:member:`GIMIComponentFixerConfig::Component::mirroredObjs`, and unlike that one it
         adds no vertices -- the twin indexes the same ones.

         .. note::
            A component the mod draws as more than one RANGE is skipped and said so in the log. One
            twin after one draw is right only while the component has one draw; a toggled range
            would have its twin drawn whatever the toggle says
         @endrst
         */
        std::set<int> mirroredComponents;

        /**
         * @brief
         @rst
         The register family a mod binds its textures with, which a remapped section's CARRIED
         bindings are re-keyed within -- see :cpp:member:`plan`
         @endrst
         *
         * A mod that binds per draw rather than once per section writes these lines inside the
         * component section, in ITS OWN layout, and they land after the fix's texture list and
         * override it. Matched by prefix, case-insensitively.
         */
        std::string texRegPrefix = "ps-t";

        /**
         * @brief The register the mod's vector buffer (its normals) is bound at. **Default**: ``"vb1"``
         */
        std::string vectorReg = "vb1";

        /**
         * @brief
         @rst
         The mod objects (of :cpp:struct:`WWMIParserConfig`'s hash-only ones) whose sections are
         commented out of the mod's own text and copied nowhere. **Default**: the two shape-key
         overrides, since the mod's keys are sized for the source's shape-key vertex count, not the
         target's
         @endrst
         */
        std::vector<std::string> hiddenObjs = {"shapekeyOffsets", "shapekeyScale"};

        /**
         * @brief The register the mod's blend buffer is bound at. **Default**: ``"vb4"``
         */
        std::string blendReg = "vb4";
        /**
         * @brief A register line a remapped section DROPS, optionally only for certain values
         */
        struct RegRemoval {
            /**
             * @brief The register or key, eg. ``"ResourceBlendBufferOverride"`` or ``"run"``
             */
            std::string reg;
            /**
             * @brief
             @rst
             Drop it only when its value begins with this, ignoring case and leading space. Empty
             (the default) drops every value
             @endrst
             */
            std::string valuePrefix;
        };
        /**
         * @brief
         @rst
         Register lines a remapped section drops, on top of everything the template removes anyway.

         Two kinds, both needed for Chisa:

         * the ``Resource{BlendBuffer,MergedSkeleton,ExtraMergedSkeleton}Override = ref ...`` lines a
           mod of a character past 256 merged bones carries. They point the draw at WWMI's blend
           remap of the SOURCE, and the two shaders are exact inverses, so copied into a remapped
           section they feed the draw the source's own merged index against the TARGET's skeleton --
           the components that have a blend remap collapse into a drape under an intact
           head. Match ``"ref"`` and not the bare name: the shared cleanup list sets the same three
           to ``null``, which is a safety net worth keeping
         * the ``run`` of RabbitFX's ``SetTextures`` and the maps it names, which otherwise override
           the fix's own texture lists positionally
         @endrst
         */
        std::vector<RegRemoval> removedRegs;
        /**
         * @brief
         @rst
         Chains of SOURCE vertex groups pinned to one bone each: ``{root: members}``, where every
         member is remapped to whatever the ROOT maps to.

         A part the target has no counterpart for wants one rigid anchor rather than the finder's
         per-bone nearest. Two of Chisa's need it:

         * the fox mask and hairpins of the KIMONO MOD (Chisa2, Hanabi Night) -- neither Chisa
           nor her skin has one; these are her component 5 ACCESSORY bones, which her base model
           uses for a hair ribbon and which mods hang props off. A rigid prop whose bones the
           finder matched one at a time and scattered from her head to her waist, which reads in
           game as the prop being GONE rather
           than as anything misplaced, because it is smeared through the torso it is buried in
         * her back skirt panel, which flew out behind her on the skin. Its bones are not mapped to
           the wrong PLACE -- they land 0.8 to 6.1 units from where they live on her, and the panel
           hangs correctly on Chisa herself -- they are mapped to bones that MOVE differently, the
           skin's own frill bones for the garment she wears instead. Retargeting by proximity is
           worse by that same measure, which is the sign that rigidity is the criterion

         The key is a SOURCE bone and the chain takes whatever it maps to, so writing a target id
         here is a silent no-op-shaped error. Pick the root by skinning the part with each candidate
         and keeping the ones whose RMS radius from the centroid is unchanged
         (``Tools/Misc/Diagnostics/wwmiAnchorSearch.py``) -- never off a bone's ``vs-cb4``
         translation column, which is a skinning matrix and not a pose.

         .. warning::
             **Both examples above are parts with bones of their OWN, and that is a precondition
             this does not check.** The key space is the source skeleton GLOBALLY, not per
             component, so a member another component also weights is pinned in that component too
             -- and for a body bone that is a welded torso, silently and totally.

             Eg. on ChisaParfait -> Chisa, ``wwmiAnchorSearch.py`` names a good bone for both of her
             extra parts, but **most of the hip prop's and the frilled panel's weight sits on bones
             the upper or lower body also uses**, so neither may be anchored at all and that pair's
             row is deliberately empty. Check with
             ``Tools/Misc/Diagnostics/anchorSafety.py`` before adding a row
         @endrst
         */
        std::map<long long, std::vector<long long>> anchorChains;
        /**
         * @brief
         @rst
         Per target slot, per pass, the bindings that pass takes INSTEAD of the plan's --
         ``{slot: {pass: bindings}}``.

         :cpp:member:`slotPasses` names the passes one command list guards, all taking the plan's
         bindings. That holds while every pass of a slot binds the same art at the same registers,
         which is true of Sanhua and false of Chisa: a slot's register layout is per SHADER, so the
         same role sits at different registers on different passes of one slot. Her outline and
         shadow passes take each slot's diffuse at ``ps-t0`` where the main pass takes it at
         ``ps-t1`` (hair) or ``ps-t3`` (clothing).

         A pass named in neither table still DRAWS -- with the GAME's textures, which is the "one
         part wearing another's art" symptom -- so this table is also what records that a pass was
         considered. ``Tools/Misc/Diagnostics/wwmiPassCoverage.py`` lists every pass per slot
         against both
         @endrst
         */
        std::map<int, std::map<std::string, std::vector<Binding>>> extraPassRegs;

        /**
         * @brief
         @rst
         Target slot -> the passes of :cpp:member:`extraPassRegs` its draw must NOT be re-issued on
         @endrst
         *
         * A remapped section matches by hash and index window, which name no pass, so it runs on
         * EVERY pass that draws its slot and re-issues `drawindexed` on each. That is right where
         * the target really draws the slot on that pass, and wrong where the pass belongs to the
         * SOURCE's shader set: the second write is the same geometry under a different vertex
         * shader, offset from the first, which reads in game as a ghost limb.
         *
         * Suppressed with `ib = null` inside the pass's own gated list, so it reaches that pass and
         * nothing else, and the textures stay bound for a slot that does draw there. Empty by
         * default, so no earlier pair's output moves.
         */
        std::map<int, std::set<std::string>> extraPassNoDraw;
        /**
         * @brief
         @rst
         Each pass mapped to the VERTEX shaders it is drawn with. Empty (the default) tags the pass's
         own pixel shader and guards ``ps == ...``; set, the fix tags those vertex shaders instead
         and guards ``vs == ...``, an OR when a pass has several.

         Chisa needs it and Sanhua does not. RabbitFX patches PIXEL shaders and marks each
         ``filter_index = 1718.1``, and a shader carries one filter_index -- so tagging the same
         pixel shader makes every RabbitFX ``SetTextures`` read ``if ps == 1718.1`` as false, and
         because a ``[ShaderOverride]`` is keyed by shader hash GLOBALLY it does that for any mod
         drawing with those shaders, not only this one. Tagging just the passes RabbitFX leaves alone
         is not a readable list: six of its regexes have their dump lines commented out, so a pass
         can be RabbitFX's with no trace in any dump, and a ``ps`` tag on such a pass switches its
         FX-map discard off (eg. a backless sweater's see-through panels render red).

         A pass left out of a non-empty map is an error rather than a fallback to its pixel shader:
         the fallback would be silent and would reintroduce exactly that. Read the pairs off every
         frame dump's draw table (``Tools/Misc/Diagnostics/wwmiDrawTable.py``) -- a pass can run on a
         different vertex shader per component, or on different ones in different dumps
         @endrst
         */
        std::map<std::string, std::vector<std::string>> passVertexShaders;
        /**
         * @brief
         @rst
         Every fact about the SOURCE character's own textures -- its hashes by role, its register
         layout per component, its pixel thumbprints, and where its game textures are downloaded
         from. The same object the parser is configured with
         (:cpp:member:`WWMIParserConfig::textures`), so a character states these once, in
         ``<Name>Textures.cpp``, rather than once per fix row :raw-html:`<br />` :raw-html:`<br />`

         :cpp:member:`WWMITextureFacts::registerRoles` is the one the fix reads most:
         ``{component: {register: role}}``.

         A mod that REPAINTS a texture is identified by none of the other paths: its hash is its own,
         its pixels are its own art, and its exporter may name the file anything. What still
         identifies it is where the mod's own section binds it -- a file at the register the source's
         layout calls the light map IS the light map. This is the primary path for most mods that do
         more than swap a mesh.

         Read off the source's own draws at max LOD (``Tools/Misc/Diagnostics/wwmiPassLayout.py``),
         and remember that only a ``sets`` line is evidence about a character: a register a draw
         INHERITED may have been written by an unrelated NPC standing in the same frame
         @endrst
         */
        WWMITextureFacts sourceTextures;
        /**
         * @brief
         @rst
         Other meshes the character draws, by their own ``vb0`` hash --
         ``{mesh hash: {pass: bindings}}``.

         A character is not only her ``vb0`` mesh. Chisa and ChisaParfait both draw ``b00403dc``,
         the same hash on both and so the same geometry, and the game textures it PER CHARACTER:
         Chisa's draws bind her hair diffuse, the skin's bind her own. It is her hair ribbon.

         Nothing else the fix writes reaches it -- the remapped sections match the MAIN mesh's
         ``vb0`` hash, and a mod's ``[TextureOverrideTexture]`` overrides by the hash the GAME binds,
         which on the skin is never the source's. So such a mesh keeps the target's art however much
         texture work is done elsewhere -- painting every slot a flat colour and seeing which part
         stays unpainted is how to find such a mesh.

         The geometry is shared, so there is nothing to remap: only the textures to rebind, on the
         passes where the two characters differ.

         .. warning::
             The section matches the hash with NO index window, so it applies wherever that mesh is
             drawn. It is assumed to be this character's own accessory, shared between her skins --
             if another character draws it too, this repaints theirs as well
         @endrst
         */
        std::map<std::string, std::map<std::string, std::vector<Binding>>> sharedMeshes;
        /**
         * @brief What a texture-edit filter is allowed to know about the mod it is editing
         */
        struct TexEditContext {
            /**
             * @brief The folder the mod's ``.ini`` sits in
             */
            std::string iniFolder;
            /**
             * @brief The mod's ``Position.buf``, or empty when it has none
             */
            std::string positionFile;
            /**
             * @brief The mod's ``Texcoord.buf``, or empty
             */
            std::string texcoordFile;
            /**
             * @brief The mod's ``Index.buf``, or empty
             */
            std::string indexFile;
            /**
             * @brief Each SOURCE component's own draws, as ``(index count, first index)``
             */
            std::map<int, std::vector<std::pair<long long, long long>>> drawRanges;
            /**
             * @brief
             @rst
             The file each role resolved to -- the mod's own, or the one its fallback download
             lands. A filter may need ANOTHER role's texture: repacking a material mask asks the
             DIFFUSE under each texel how flesh-coloured it is, because a code in the matte band is
             skin only there
             @endrst
             */
            std::map<std::string, std::string> fileOfRole;
        };
        /**
         * @brief An edit the fix makes to one role's texture before binding it
         */
        struct TexEdit {
            /**
             * @brief The role whose texture is edited
             */
            std::string role;
            /**
             * @brief
             @rst
             A short name for the edit, part of the written file's name. Two edits of one role need
             two names, or the second overwrites the first
             @endrst
             */
            std::string name;
            /**
             * @brief
             @rst
             Builds the filter for THIS mod. A factory rather than a filter, because an edit may
             depend on the mod's own geometry -- the accessory grade applies to a UV island, and the
             island is rasterised from the MOD's texcoords, since the source character's do not fit
             a mod that remeshes
             @endrst
             */
            std::function<TexEditor::Filter(const TexEditContext&)> makeFilter;
            /**
             * @brief
             @rst
             Whether to re-encode to the source's compressed format. **Default**: ``false``, because
             a mask is CODES and BCn would move them
             @endrst
             */
            bool compress = false;
        };
        /**
         * @brief
         @rst
         Edits the fix makes to a role's texture before binding it. Chisa needs four: her material
         mask repacked into the target's layout, her packed four-profile sheen matcap translated into
         the skin's holographic foil, a colour grade on her accessory diffuse -- a shader family is a
         colour grade, and the texture is the only place to put it back -- and her hair's ps-t5 map
         repacked so a UV-mapped register need not be left to the game
         @endrst
         */
        std::vector<TexEdit> texEdits;
        /**
         * @brief
         @rst
         Bind the remapped sections to a CLEANED copy of the mod's texcoord buffer.

         Two faults live in that buffer and neither is the mod's bug -- both are bytes that are
         harmless on the source and not on the target:

         * a **NaN** in the second UV. Chisa's fox mask, hairpins and bells are drawn, placed and
           textured correctly and are INVISIBLE, because the skin's upper-body shader reads that
           second UV where hers does not. Every other vertex of the mesh carries ~(0, 0) there,
           which is what a NaN becomes in the copy
         * a **U outside [0, 1)**, where a mod has UV'd a part into the next tile and relies on the
           sampler wrapping. Neither character's own model ever leaves [0, 1), so the game never
           exercises its address mode there and the two passes are free to differ -- one side of a
           body rendering with its texture detail and the other flat and pale. U and U - 1 select
           the same texel under wrap, so folding cannot regress a mod that already renders

         A vertex on a triangle whose vertices straddle a tile boundary keeps what it had: folding
         would widen that triangle's U span from a few hundredths to nearly 1 and interpolate it
         backwards across the atlas.

         The mod's own buffer and its own sections are untouched. **Default**: ``false``
         @endrst
         */
        bool cleanTexcoords = false;
        /**
         * @brief
         @rst
         The register the texcoord buffer is bound at, for :cpp:member:`cleanTexcoords`.
         **Default**: ``"vb2"``
         @endrst
         */
        std::string texcoordReg = "vb2";

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
         merged, which would scramble every bone of the body
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
         Bind the remapped PREVIOUS pose on every pass, not only where the second skeleton
         carries ``boneDataFilter`` -- see :cpp:member:`boneDataFilter`. **Default**: ``true``
         @endrst
         *
         * `vs-cb4` is the pose a draw is skinned with and `vs-cb3` is the previous frame's,
         * which the shader turns into motion vectors. The game does not mark cb3 on every
         * pass, so a guarded replacement leaves some draws skinned with OUR pose and
         * reprojected from the TARGET's -- a large bogus motion, which TAA smears into a
         * second body over the whole character.
         *
         * Defaulted ON because the alternative is never right. It moves the output of every
         * WuWa pair, so a pair confirmed in game before 2026-10-03 wants another look.
         */
        bool bindPrevPoseAlways = true;

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
         * @brief
         @rst
         The command list every slot section runs after its draw to put the game's own buffers
         back -- see :cpp:member:`sharedResourcesList`. **Default**:
         ``"CommandListCleanupSharedResources"``
         @endrst
         *
         * WWMI's own pair captures `vb0` and restores only that; the fix extends both so every
         * buffer the override list binds is put back, because a REMAP has draws of the target
         * that no section of it matches, and those inherit whatever is still bound.
         */
        std::string cleanupResourcesList = "CommandListCleanupSharedResources";

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
