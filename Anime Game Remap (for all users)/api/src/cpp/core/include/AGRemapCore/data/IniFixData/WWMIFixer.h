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

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/WWMINames.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/WWMITextureFacts.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {
    class IniParseData;
    class ModType;
    class IniFile;

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
     #. **remaps the blend** (:cpp:member:`IniKeywords::WWMIBlendReg`, collected out of the shared-resource override)
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
         * @brief
         @rst
         The config a fixer factory from :cpp:func:`makeWWMIFixer` was made with, or ``std::nullopt`` for any other
         factory. A shipped fix's factory is its mod type's builder row (:cpp:func:`IniFixBuilder::factoryOf`), so this
         reads what a shipped fix does -- or starts a variant of it (change a field, hand it to :cpp:func:`makeWWMIFixer`)
         @endrst
         *
         * @param factory The factory
         *
         * @return The config
         */
        static std::optional<WWMIFixerConfig> ofFactory(const IniFixBuilder::Factory& factory);

        /**
         * @brief
         @rst
         This config with its GAME DATA read off the versioned asset tables, at the run's versions -- what a config
         carries is the pair's choices, and what the game draws with comes from the tables, so a fix runs at any
         version the tables have rows for:

         * :cpp:member:`slotPasses`, :cpp:member:`passVertexShaders` and :cpp:member:`filterIndices`, from the
           target's :cpp:class:`DrawPasses` (each slot's ``main``, ``main2``, ... and ``other``, ``other2``, ... passes,
           their vertex shaders and ``filter_index`` tags)
         * the keys of :cpp:member:`extraPassRegs` and :cpp:member:`extraPassNoDraw` written as a pass NAME
           (``other``, ``other2``), as the pass's pixel shader
         * :cpp:member:`shapeKeyDispatchSize` (:cpp:class:`ShapeKeyDispatchSizes`) and :cpp:member:`shapeKeyOrder`
           (:cpp:class:`ShapeKeyRemaps`)
         * each :cpp:struct:`Shader`'s code and constant buffers, for one naming its :cpp:class:`GameShaders` row
           (:cpp:member:`Shader::game`, and a pass's own code by :cpp:member:`Shader::passGame`)
         * the source's game textures' thumbprints (:cpp:member:`sourceTextures`, :cpp:class:`TextureThumbprints`)
         * a :cpp:struct:`CreatedTexture`'s colour, for one naming its :cpp:class:`MaterialColours` row
         * :cpp:member:`sourceVgMaps`: the source's :cpp:class:`VGRemaps` rows from each component onto her merged
           skeleton (``""``), at her oldest ``vb0``'s version
         * each :cpp:struct:`Shader`'s registers left empty, from :cpp:class:`TextureRegisters`: the source's for the
           draw it was read from (:cpp:member:`Shader::component`), the target's for the slot it captures or refs off
           (:cpp:member:`Shader::stashed`, :cpp:member:`Shader::refRoles`)
         * :cpp:member:`stashRegs`, from the shaders' :cpp:member:`Shader::stashed`
         * :cpp:member:`sourceTextures`' download game folder, folder and prefix, when empty: the source's game and name
         * :cpp:member:`roleKinds`, from both characters' :cpp:class:`TextureKinds` -- refusing a role the config names
           (a binding, a shader's, an edit's, a created texture's, :cpp:member:`sourceTextures`') with no row
         * :cpp:member:`sourceVersionByVb0`, when :cpp:member:`sourceVersionFromVb0` (see :cpp:func:`vb0VersionsOf`)

         A field the tables hold nothing for keeps the config's value. The compiled fixer calls this once per fix
         @endrst
         *
         * @param source The source's mod type
         * @param target The target's mod type
         * @param fromVersion The source's version (the mod's) -- ``std::nullopt`` reads each table's newest rows
         * @param toVersion The target's version (the fix's)
         *
         * @return The filled config
         */
        WWMIFixerConfig withGameData(const ModType& source, const ModType& target, const std::optional<Version>& fromVersion,
                                     const std::optional<Version>& toVersion) const;

        /**
         * @brief Each ``vb0`` hash :cpp:class:`HashData` files a character's model under (lower case) -> its version
         *     (``Version::toString``)
         *
         * @param source The character's mod type
         *
         * @return The versions
         */
        static std::map<std::string, std::string> vb0VersionsOf(const ModType& source);

        /**
         * @brief
         @rst
         The version a fix onto a character is made at when the run asks for none: the newest of her newest model (the
         version of her newest ``vb0`` row in :cpp:class:`Hashes`) and the newest game version her :cpp:class:`DrawPasses`
         rows are filed at. A game update that rehashes her shaders and moves no model of hers (Wuthering Waves 3.8) is a
         newer :cpp:class:`DrawPasses` row, and a fix made by default reaches it; a lookup of a model table at that
         version still answers her newest model's row
         @endrst
         *
         * @param target The character's mod type
         *
         * @return The version, or ``std::nullopt`` when her tables hold neither
         */
        static std::optional<Version> newestVersionOf(const ModType& target);

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
             The role (a value of :cpp:member:`roles`, or the name of a
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
         * @brief
         @rst
         Each texture role -> its KIND: ``upperDiffuse``, ``lowerDiffuse`` and ``panelDiffuse`` all have the kind
         ``Diffuse``. :cpp:func:`withGameData` fills it from both characters' :cpp:class:`TextureKinds` rows, and refuses
         a role the config names that has none. **Default**: empty
         @endrst
         */
        std::map<std::string, std::string> roleKinds;

        /**
         * @brief A role's kind (#roleKinds): ``""`` for a role with no kind, or none known
         *
         * @param role The role
         *
         * @return Its kind
         */
        std::string kindOf(const std::string& role) const;

        /**
         * @brief
         @rst
         The register 'bindings' bind 'role' at: the first of its own, else the one they bind its KIND at (#kindOf)
         -- only where that is one register, since two roles of one kind (``irisDiffuse``, ``eyeDiffuse``) may sit at
         different ones. A ``null`` binding binds nothing
         @endrst
         *
         * @param bindings A component's bindings, eg. its plan row's
         * @param role The role
         *
         * @return The register, or ``std::nullopt``
         */
        std::optional<std::string> registerOf(const std::vector<Binding>& bindings, const std::string& role) const;

        /**
         * @brief
         @rst
         A pixel shader a :cpp:struct:`SourceComponent` is drawn with in place of the target slot's
         :raw-html:`<br />` :raw-html:`<br />`

         The fix writes the shader and its constant buffers beside the mod's buffers, declares
         them in its own block (so an undo removes them), and turns each rerouted
         ``drawindexed`` of the component into a ``run =`` of a ``CustomShader`` section that binds
         them and makes that draw. Everything the shader does not bind itself is the target
         draw's: its vertex shader, its render targets and the textures the component's
         :cpp:member:`plan` entry binds
         @endrst
         */
        struct Shader {
            /**
             * @brief The name its files and sections are given, eg. ``LynaeFilm``
             */
            std::string name;

            /**
             * @brief
             @rst
             The :cpp:class:`GameShaders` row its code and constant buffers are read from, eg. ``LynaeFilm`` -- the
             source's or the target's game shader (:cpp:func:`WWMIFixerConfig::withGameData` fills :cpp:member:`ps`
             and :cpp:member:`constantBuffers`). Empty: the config gives them
             @endrst
             */
            std::string game;

            /**
             * @brief A shader whose code and constant buffers are the :cpp:class:`GameShaders` row 'game' (#game), named after it
             *
             * @param game The row, eg. ``LynaeUpper``
             *
             * @return The shader
             */
            static Shader ofGame(const std::string& game);

            /**
             * @brief The HLSL source, entry point ``main``
             */
            std::string ps;

            /**
             * @brief
             @rst
             Register -> the constant buffer bound there, as its 32-bit words (four to a
             row), eg. ``ps-cb4``. Written as a ``R32G32B32A32_FLOAT`` buffer
             @endrst
             */
            std::map<std::string, std::vector<std::uint32_t>> constantBuffers;

            /**
             * @brief
             @rst
             Further lines of the ``CustomShader`` section, written as they are, after :cpp:member:`stashed` and
             :cpp:member:`gameRoles`, eg. a render state
             @endrst
             */
            std::vector<std::pair<std::string, std::string>> lines;

            /**
             * @brief
             @rst
             The SOURCE component whose own draw the shader was read from: the roles it binds sit at the registers
             that draw binds them (the source's :cpp:class:`TextureRegisters` for that component), so every register
             below left empty is read off that table by :cpp:func:`WWMIFixerConfig::withGameData`. **Default**:
             ``-1``, the component of the :cpp:member:`plan` entry the shader is on (a reroute's: the component it
             reroutes)
             @endrst
             */
            int component = -1;

            /**
             * @brief
             @rst
             Roles the shader reads off the target slot's own draw by ``ref``: each is written first in
             :cpp:member:`lines` as ``<the shader's register> = ref <the slot's register>`` (the two from the source's
             and the target's :cpp:class:`TextureRegisters`) -- a texture both characters share, eg. Lynae's matcap,
             which the skin's slot holds at ``ps-t8`` and her shader reads at ``ps-t4``. **Default**: empty
             @endrst
             */
            std::vector<std::string> refRoles;

            /**
             * @brief A register of the shader's bound to a capture the fix makes off another slot -- see #stashed
             */
            struct Stashed {
                /**
                 * @brief The role captured, eg. ``"glow"``
                 */
                std::string role;

                /**
                 * @brief The target slot whose draw the capture is made off (:cpp:member:`WWMIFixerConfig::stashRegs`)
                 */
                int slot = -1;

                /**
                 * @brief
                 @rst
                 The shader's register, eg. ``"ps-t4"``. Empty: the source's :cpp:class:`TextureRegisters` row for
                 #role on :cpp:member:`Shader::component` (:cpp:func:`WWMIFixerConfig::withGameData`)
                 @endrst
                 */
                std::string reg;

                /**
                 * @brief
                 @rst
                 The register of that slot's draw that is captured. Empty: the target's :cpp:class:`TextureRegisters`
                 row for #role on #slot (:cpp:func:`WWMIFixerConfig::withGameData`)
                 @endrst
                 */
                std::string slotReg;
            };

            /**
             * @brief
             @rst
             Registers bound to a texture the target's OTHER slot holds while it draws -- captured off that slot's draw
             (:cpp:member:`WWMIFixerConfig::stashRegs`, filled from these) -- eg. Lynae's glow, flow and noise maps,
             which her lower body reads and the skin's upper-body slot lacks. Written first. **Default**: empty
             @endrst
             */
            std::vector<Stashed> stashed;

            /**
             * @brief
             @rst
             Register -> role: the shader's register bound to the texture of that role -- the mod's file, or the
             source's own game texture downloaded (a role with neither binds nothing). Written after
             :cpp:member:`stashed`. Eg. Lynae's lighting ramp, which the skin's slot holds under the same name but the
             game refills with the ramp of whoever is on screen. A register left empty is the source's
             :cpp:class:`TextureRegisters` row for the role on :cpp:member:`component`. **Default**: empty
             @endrst
             */
            std::vector<std::pair<std::string, std::string>> gameRoles;

            /**
             * @brief
             @rst
             Draw the component once per entry, each with :cpp:member:`lines` and that entry's own lines
             after them -- a source pass whose draws split by state, eg. a see-through layer blended where
             the stencil marks a body behind it and drawn opaque everywhere else. **Default**: empty, one
             draw with :cpp:member:`lines`
             @endrst
             */
            std::vector<std::vector<std::pair<std::string, std::string>>> passes;

            /**
             * @brief
             @rst
             Per entry of :cpp:member:`passes`, the HLSL that pass draws with in place of :cpp:member:`ps`
             -- a source that draws one component with two shaders, eg. a solid one where the part is opaque
             and a see-through one over a body. An empty entry, or none, keeps :cpp:member:`ps`. Each is
             written beside :cpp:member:`ps` as a file of its own. **Default**: empty
             @endrst
             */
            std::vector<std::string> passPs;

            /**
             * @brief
             @rst
             :cpp:member:`passPs` by :cpp:class:`GameShaders` row: per entry of :cpp:member:`passes`, the row whose code
             that pass draws with (:cpp:func:`WWMIFixerConfig::withGameData` fills :cpp:member:`passPs`). An empty entry
             keeps that pass's :cpp:member:`passPs`. **Default**: empty
             @endrst
             */
            std::vector<std::string> passGame;

            /**
             * @brief
             @rst
             Which draws must come before the component's, on a pass -- see :cpp:member:`after`
             @endrst
             */
            struct After {
                /**
                 * @brief The SOURCE component (a key of :cpp:member:`WWMIFixerConfig::plan`) drawn first
                 */
                int component = -1;

                /**
                 * @brief
                 @rst
                 How many draws that component's slot makes in one pass; the deferred draw follows the
                 last of them. **Default**: ``1``
                 @endrst
                 */
                int draws = 1;

                /**
                 * @brief
                 @rst
                 The output register that tells one pass from another: two draws into the same
                 resource here are one pass. Pick one a pass that should NOT wait (a depth pre-pass)
                 leaves unbound -- such a pass draws the component as before. **Default**: ``"o3"``
                 @endrst
                 */
                std::string target = IniKeywords::WWMIColourTarget;

                /**
                 * @brief
                 @rst
                 The registers a deferred draw takes from the component's own draw (the textures its
                 plan entry binds, a register the shader's lines read by ``ref``), eg. ``"ps-t0"``.
                 Everything else is the later draw's
                 @endrst
                 */
                std::vector<std::string> carried;

                /**
                 * @brief
                 @rst
                 Further components the draw waits for as well, each with how many draws its slot makes
                 in one pass: the draw is made once ALL of them (and :cpp:member:`component`) have drawn
                 there, in whatever order the scene draws them. A component the mod does not draw is left
                 out of the count
                 @endrst
                 */
                std::vector<std::pair<int, int>> also;
            };

            /**
             * @brief
             @rst
             An output register (eg. ``"o3"``) a pass has to bind for the shader to draw there; a pass
             that leaves it unbound (a depth pre-pass) draws the component with the game's own shader, as
             any other component. Empty, the shader draws on every pass of the slot. **Default**: ``""``
             @endrst
             */
            std::string onlyWith;

            /**
             * @brief
             @rst
             A mod whose sections for the component hand its maps to RabbitFX keeps the target slot's GAME
             shader there: RabbitFX works by patching the game's shaders, never this one, so a RabbitFX mod
             drawn with it loses its effect (a mod's robotic legs drawn as skin). **Default**: ``false``
             @endrst
             */
            bool yieldToRabbitFX = false;

            /**
             * @brief
             @rst
             Bindings the component takes only while this shader draws it, after the plan entry's own --
             a register (or a register's role) the shader reads in the SOURCE's layout; a register left empty is the
             source's :cpp:class:`TextureRegisters` row for the role on :cpp:member:`component`. With
             :cpp:member:`yieldToRabbitFX`, a mod that keeps the target's shader keeps the plan's
             bindings alone. **Default**: empty
             @endrst
             */
            std::vector<Binding> registers;

            /**
             * @brief
             @rst
             Draw the component only AFTER another component's draws in the same pass, whichever
             order the game draws the two slots in.

             The game does not keep one order: Lynae's jacket slot draws after her upper body in the
             character menu and BEFORE it in the overworld. A see-through layer blends over whatever
             is under it when it draws, so drawn first it blends over her hair and the scene behind
             her -- and her body, drawn after, loses the depth test under it. Set, the component's
             draw on a pass binding :cpp:member:`After::target` is held back when the other
             component has not drawn yet, and made after that component's last draw instead.
             **Default**: none, drawn where its slot draws
             @endrst
             */
            std::optional<After> after;
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

            /**
             * @brief
             @rst
             When another source component also goes through :cpp:member:`slot`, draw this one from the
             mod's own ``.ini`` as a second section on that draw, instead of from a copy ``.ini``.
             A copy computes none of the mod's shape-keyed positions -- the shape-key pipeline runs
             once, in the mod's own file -- so a component of a mod with shape keys drawn from a copy
             reads an empty position buffer and draws nothing. **Default**: ``false``
             @endrst
             */
            bool sameFileSection = false;

            /**
             * @brief
             @rst
             The SOURCE's own pixel shader, when the target slot's cannot draw what this component
             is (a material the target has no counterpart for). With one, the component binds the
             mod's art as the mod wrote it -- no texture edit of the config reaches it, since the
             edits are for the TARGET's shader. It draws with the shader on the passes the slot's
             :cpp:member:`slotPasses` name, and as before on any other (an outline pass,
             :cpp:member:`extraPassRegs`). A :cpp:struct:`DrawReroute`'s ``as`` entry may carry one
             too. **Default**: none, the target slot's shader
             @endrst
             */
            std::optional<Shader> shader;

            /**
             * @brief
             @rst
             RabbitFX on this component, carried across with the SOURCE's routing
             :raw-html:`<br />` :raw-html:`<br />`

             RabbitFX patches a character's pixel shaders to take a mod's maps
             (``Resource\\RabbitFX\\Diffuse``, ``...Lightmap``, ``...Normalmap``, ...) in place of
             the shader's first texture samples. Which map replaces which sample is decided per draw
             from the KIND of texture bound at ``ps-t0`` .. ``ps-t3`` when the mod runs
             ``CommandList\\RabbitFX\\SetTextures`` -- as of RabbitFX 8.3.2, its ``filter_index``
             1718.3 (a square BC7 of more than 10 mips, or a small square B8G8R8A8 with mips), 1718.4 (a
             square R8 with mips, or a large square BC7 of one mip), 1718.5 (a 1 x 1 B8G8R8A8) or 1718.6
             (a square BC7 of at most 512 and 10 mips). On the target those registers hold other files (the fix's edits, other
             formats), so the same mod lines route the maps differently. With this set, the
             component keeps the mod's RabbitFX lines (:cpp:member:`removedRegs` stops removing them
             here), and right after each ``SetTextures`` the routing flags (``x187`` .. ``w187``) are
             pinned to what they are on the SOURCE's own draw: read off the mod's own file for the
             role at each register, or :cpp:member:`gameTags` where the mod binds none. Valid only
             where the target slot samples the same roles in the same order as the source's pass.
             **Default**: none, RabbitFX's lines are removed as :cpp:member:`removedRegs` says
             @endrst
             */
            struct RabbitFX {
                /**
                 * @brief
                 @rst
                 ``ps-t0`` .. ``ps-t3`` -> the tag RabbitFX gives the SOURCE's own game texture there
                 (3 for 1718.3, 4 for 1718.4, 5 for 1718.5, 6 for 1718.6, 0 for none). Empty: :cpp:func:`withGameData`
                 fills it at the run's version, from what her draw of the component binds (:cpp:class:`DrawPasses`'
                 ``main.ps-tN``) and how the game creates that texture (:cpp:class:`TextureDescs`, :cpp:func:`RabbitFX::tagOf`)
                 @endrst
                 */
                std::map<std::string, int> gameTags;
            };

            /**
             * @brief RabbitFX on this component, with the source's routing -- see :cpp:struct:`RabbitFX`
             */
            std::optional<RabbitFX> rabbitFX;

            /**
             * @brief
             @rst
             After each of the component's draws on the passes the slot's :cpp:member:`slotPasses` name, the
             same draw again with NO pixel shader under these ``CustomShader`` lines -- to mark the pixels
             the component is the visible surface of in the stencil, eg. ``depth_func = equal`` with a stencil
             write, for a later draw to test. Nothing is written to the render targets. **Default**: empty,
             no such draw
             @endrst
             */
            std::vector<std::pair<std::string, std::string>> markLines;

            /**
             * @brief
             @rst
             After each of the component's draws on the passes the slot's :cpp:member:`slotPasses` name, the
             same draw again with this shader -- its :cpp:member:`Shader::registers`, constant buffers and
             :cpp:member:`Shader::lines` -- while the component itself keeps the target slot's shader and its
             plan bindings. For a part whose surface the target's shader draws right and only the SOURCE's
             shader can finish, eg. a glow written into the scene-colour target alone (``mask[1] = 0`` ...
             with ``depth_func = equal``). Its :cpp:member:`Shader::registers` bind the mod's own files,
             unedited. Only :cpp:member:`Shader::name`, :cpp:member:`Shader::ps`,
             :cpp:member:`Shader::constantBuffers`, :cpp:member:`Shader::registers`,
             :cpp:member:`Shader::lines`, :cpp:member:`Shader::onlyWith` and
             :cpp:member:`Shader::yieldToRabbitFX` are read. **Default**: none, no such draw
             @endrst
             */
            std::optional<Shader> overlay;

            /**
             * @brief
             @rst
             Passes of the target slot (pixel shader hashes, as in :cpp:member:`slotPasses`) that the
             component's draws are kept off -- eg. a slot's outline pass, whose shell of an under-layer
             the target's outline shader draws through the layer over it. **Default**: empty, every pass
             @endrst
             */
            std::vector<std::string> offPasses;
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

            /**
             * @brief
             @rst
             The target's component and :cpp:class:`MaterialColours` type its colour is read from, eg.
             ``("component3", "torsoMask.skin")`` (:cpp:func:`WWMIFixerConfig::withGameData`). Empty: :cpp:member:`colour`
             is the config's
             @endrst
             */
            std::pair<std::string, std::string> material;
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
         The TARGET's game version, used when the run asks for none. Empty (the default) is
         :cpp:func:`newestVersionOf` the target -- her newest model, or a newer game version her draw passes are filed
         at -- so a fix follows the tables to a new model of the target, or to a game update, without a change here
         @endrst
         */
        std::string version;
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
         The SOURCE's version per ``vb0`` hash, for a character whose skeleton was RENUMBERED
         between two game versions -- read off the mod's own slot sections and used wherever
         :cpp:member:`sourceVersion` would be, when the ``.ini`` carries no version of its own
         :raw-html:`<br />` :raw-html:`<br />`

         A mod carries the bone ids of the version its author exported at, and its ``vb0`` says
         which version that was. Lynae's 3.7 update moved her ``vb0`` (``0c33d628`` ->
         ``7e400733``) AND inserted bones into her skeleton, so 71 of her 360 merged ids moved
         (mostly by one or two); a 3.6 mod read with the 3.7 row puts those vertices on the
         neighbouring bone. With this, each mod takes the :cpp:class:`VGRemaps` row filed at its own
         version. Keys are lower case. Empty (the default) keeps :cpp:member:`sourceVersion` for
         every mod
         @endrst
         */
        std::map<std::string, std::string> sourceVersionByVb0;

        /**
         * @brief
         @rst
         Whether :cpp:member:`sourceVersionByVb0` is read off the source's own ``vb0`` rows in :cpp:class:`HashData`
         (:cpp:func:`vb0VersionsOf`, by :cpp:func:`withGameData`). **Default**: ``false``
         @endrst
         */
        bool sourceVersionFromVb0 = false;


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
         The ``filter_index`` each pass shader is tagged with -- as a string, so the value in the ``.ini`` is
         exactly what is written here. :cpp:func:`withGameData` reads them off the target's (and the source's)
         ``DrawPasses`` tags, and the fix REFUSES a pass shader with none (:cpp:func:`ShaderFilterTags::addGiven`)
         :raw-html:`<br />` :raw-html:`<br />`

         3dmigoto keys a ``[ShaderOverride]`` by its shader hash across EVERY loaded ``.ini``, not per
         file or per namespace. Both directions of one pair tag the same shaders -- a skin and its
         character share the hair, face and eye ones -- so a mod fixed one way and a mod fixed the
         other way, both installed, declare overrides on those hashes twice. If the two disagree on
         the value, whichever file 3dmigoto reads last wins and the other mod's ``if ps == <filter>``
         never matches: its textures are silently not bound. So one shader has one value in the table,
         whichever pair tags it
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
         * @brief Some draws of one source component, sent through a different target slot
         */
        struct DrawReroute {
            /**
             * @brief The source component whose draws may go elsewhere
             */
            int from = -1;
            /**
             * @brief
             @rst
             The :cpp:member:`plan` entry the rerouted draws are drawn by -- an id that is not a
             source slot, eg. ``105``. Its slot and bindings say where they go and what they bind;
             it is drawn from the mod's own ``.ini`` (give it ``sameFileSection``)
             @endrst
             */
            int as = -1;

            /**
             * @brief
             @rst
             The :cpp:member:`WWMIParserConfig::drawSplits` entry whose draws go: its
             :cpp:member:`WWMIParserConfig::DrawSplit::name`
             @endrst
             */
            std::string split;
        };

        /**
         * @brief
         @rst
         Draws that go through a different target slot than the rest of their component: the draws
         the parser picked out for one of its :cpp:member:`WWMIParserConfig::drawSplits`. Lynae paints a
         see-through film with flag ``0x10`` and her jacket's shader draws it as one; LynaePeppermint's
         jacket slot has no film, so those draws go through her hip pouch, whose foil is. A component
         the parser picked no draws of is left whole. The rerouted draws are
         a second copy of the component's sections, renamed, holding only those draws; the
         component's own copy drops them. **Default**: none
         @endrst
         */
        std::vector<DrawReroute> drawReroutes;

        /**
         * @brief
         @rst
         The SOURCE component a :cpp:member:`plan` entry draws: for a reroute's entry (:cpp:member:`DrawReroute::as`), the
         component it takes its draws from (:cpp:member:`DrawReroute::from`); else the entry's own key
         @endrst
         *
         * @param key The plan entry
         *
         * @return The component
         */
        int componentOf(int key) const;

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
         Roles of :cpp:member:`WWMITextureFacts::regionRoles` whose refused file is left to the GAME
         rather than replaced by a download :raw-html:`<br />` :raw-html:`<br />`

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
         * @brief
         @rst
         Whether every remapped draw binds a zero shape-key offset stream (item 3 above), at
         :cpp:member:`IniKeywords::WWMIShapeKeyStreamReg`, :cpp:member:`WWMINames::ShapeKeyStreamStride` bytes a
         vertex. **Default**: ``false``
         @endrst
         */
        bool zeroShapeKeyStream = false;


        /**
         * @brief
         @rst
         The TARGET's original shape-key dispatch height -- its ``Metadata.json``'s
         ``shapekeys.dispatch_y``, which is its shape-key vertex count / 32 rounded up -- written into a
         retargeted mod's ``$\WWMIv1\shapekey_dispatch_size_y_original_batch0`` / ``_batch1``
         :raw-html:`<br />` :raw-html:`<br />`

         A newer WWMI Tools export loads its keys in batches, and WWMI's
         ``CommandListLoadShapeKeysBatch`` picks a batch by comparing that value with the draw's
         ``THREAD_GROUP_COUNT_Y``. Left at the SOURCE's value, nothing matches on the target and the
         fallback sets ``$shapekey_vertex_count`` to the dispatch size itself, so most of the mod's
         offsets are never loaded and the body is drawn as spikes (Lynae9 onto LynaePeppermint: 1631
         where the target dispatches 1854). Only keys the mod already has are rewritten. Empty (the
         default) leaves them alone
         @endrst
         */
        std::string shapeKeyDispatchSize;

        /**
         * @brief
         @rst
         The order the TARGET numbers its game shape keys in, as the SOURCE key each target slot
         takes: ``shapeKeyOrder[j]`` is the source key whose data goes in slot ``j``, ``-1`` for a slot
         that takes none :raw-html:`<br />` :raw-html:`<br />`

         The game drives a character's shape keys -- blinks, mouth shapes, expressions -- by SLOT,
         and two skins of one character can number the same face keys differently. A mod's shape-key
         buffers are in the source's numbering, so on the target every expression weight lands on
         another key: a blink can open the mouth. With this set, the fix writes the mod's
         ``ResourceShapeKeyOffsetBuffer`` / ``ResourceShapeKeyVertexIdBuffer`` /
         ``ResourceShapeKeyVertexOffsetBuffer`` again in the target's order and binds the copies in
         its shape-key lists. Slots past the end of the list -- a mod's own custom keys, which only
         its ``.ini`` drives -- keep their data and position. A source key no slot takes is kept with
         its offsets zeroed, so the entry count the mod's ``.ini`` declares still holds. Empty (the
         default) leaves the shape keys in the source's order
         @endrst
         */
        std::vector<long long> shapeKeyOrder;

        /**
         * @brief
         @rst
         The mod objects (of :cpp:struct:`WWMIParserConfig`'s hash-only ones) whose sections are
         commented out of the mod's own text and copied nowhere, eg. the two shape-key overrides
         (``{"shapekeyOffsets", "shapekeyScale"}``) when the mod's keys cannot be retargeted. **Default**: none
         @endrst
         */
        std::vector<std::string> hiddenObjs;

        /**
         * @brief A register line a remapped section DROPS, optionally only for certain values
         */
        struct RegRemoval;

        /**
         * @brief
         @rst
         The lines every remapped section drops (:cpp:member:`removedRegs`' default): the three
         ``Resource{BlendBuffer,MergedSkeleton,ExtraMergedSkeleton}Override = ref ...`` of a source past 256 bones, and
         RabbitFX's ``run = CommandList\RabbitFX\SetTextures`` and the six maps it reads
         (:cpp:member:`RabbitFX::TextureKeys`)
         @endrst
         *
         * @return The lines
         */
        static std::vector<RegRemoval> gameRemovedRegs();

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

         **Default**: those ten lines (:cpp:func:`gameRemovedRegs`), a game rule rather than a pair's choice. A line the
         mod does not carry is not there to remove
         @endrst
         */
        std::vector<RegRemoval> removedRegs = gameRemovedRegs();

        /**
         * @brief
         @rst
         Register lines a remapped section also drops when the TARGET is past 256 bones: the mod's own merge list and
         the two other routes to it (``CheckTextureOverride`` on ``vs-cb3`` / ``vs-cb4``), and its own bindings of the
         merged skeletons. Each would write or bind the MOD's skeleton, sized for the source, and consume the bone-data
         marker the fix's own merge list is gated on -- which skins target-space blend indices against the source's
         skeleton, or smears a second body through the motion vectors. A value is matched by prefix, which covers a
         list's name before and after the group remap renames it. **Default**: those six lines, as WWMI Tools writes them
         @endrst
         */
        std::vector<RegRemoval> remappedSkeletonRemovals{
            {IniKeywords::Run, WWMINames::MergeSkeletonList},
            {IniKeywords::CheckTextureOverride, IniKeywords::WWMIExtraBoneDataCb},
            {IniKeywords::CheckTextureOverride, IniKeywords::WWMIBoneDataCb},
            {IniKeywords::WWMIExtraBoneDataCb, IniKeywords::WWMIExtraMergedSkeleton},
            {IniKeywords::WWMIBoneDataCb, IniKeywords::WWMIMergedSkeleton},
            {IniKeywords::WWMIExtraBoneDataCb, IniKeywords::WWMIMergedSkeleton}};
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

         **Every WuWa pair needs this.** The Sanhua pair was the last one without it: of the ten
         pixel shaders its two configs tagged, NINE run RabbitFX's ``[ShaderRegexMain]`` command
         list in the 3.7 frame dumps on disk (measured 2026-10-13 --
         ``Tools/Misc/Diagnostics/wwmiRabbitPatched.py``), so every fixed Sanhua mod had been
         switching RabbitFX off on its own shaders -- including the face shader it shares with
         ChisaParfait. RabbitFX patches PIXEL shaders and marks each
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
         * @brief
         @rst
         Other meshes the TARGET draws that a mod of her replaces, by their :cpp:class:`Hashes` mesh type (eg.
         ``blackShirt``; :cpp:func:`withGameData` reads the hash at the run's version) or their own ``vb0`` hash: each
         is skipped while the mod is enabled.

         A character is not only her ``vb0`` mesh, and some of her other meshes belong to the
         outfit the mod replaces. Lynae draws ``34c0d81d``, a black long-sleeved top under her
         shirt, in the overworld only -- never in the character menu -- and it pokes through any
         slimmer body at the neck and arms. Mods made for her skip it themselves; a remapped mod
         was made for another character and does not.

         Not :cpp:member:`hiddenObjs`, which comments out the MOD's own sections.

         .. warning::
             The section matches the hash with NO index window, so the mesh is skipped wherever it
             is drawn while the mod is enabled
         @endrst
         */
        std::vector<std::string> hiddenMeshes;

        /**
         * @brief
         @rst
         Other meshes the SOURCE and the TARGET both draw, by the hash type both characters file them
         under in :cpp:class:`HashData` -- which is also the mod object the parser places their
         sections in (:cpp:member:`WWMIParserConfig::hashOnlyObjs`, ``{type, type}``).

         Every section of the mod the parser placed in such an object is copied into the fix's block
         with its hash swapped for the target's of the same type, so what the mod does to that part of
         the outfit -- skip it, or mark when it is drawn -- happens on the target too. Changli draws
         her blindfold as its own mesh (``blindfoldMesh``: ``baaa6262``), the Laurel Nymph skin draws hers
         as another (``646aa163``), and several Changli mods hide the blindfold with a
         ``handling = skip`` section on the first hash; without the copy that blindfold showed on every
         remap of them.

         Only the hash is changed: the copy keeps the section's every other line, so a condition or
         a variable it sets means what it meant on the source
         @endrst
         */
        std::vector<std::string> meshCounterparts;

        /**
         * @brief
         @rst
         Registers to capture off the GAME's draw of a target slot nothing of the mod is drawn through
         (slot -> registers, eg. ``{4, {"ps-t5", "ps-t6"}}``), on passes that bind ``o3``. A
         :cpp:member:`Shader::stashed` entry binds the capture: a game texture the target binds on another slot,
         for a shader the source draws with it, without a download. The captured slot has to be drawn before the
         shader's in every scene. :cpp:func:`withGameData` adds each :cpp:member:`Shader::stashed` register here
         @endrst
         */
        std::map<int, std::vector<std::string>> stashRegs;

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

            /**
             * @brief
             @rst
             The characters fixed from and to, and the run's versions of each -- so a filter reads the versioned asset
             tables (:cpp:func:`ModType::getMaterialCode`, ...) at the run's versions rather than carrying their values.
             ``nullptr`` / ``std::nullopt`` outside a fix
             @endrst
             */
            const ModType* source = nullptr;
            const ModType* target = nullptr;
            std::optional<Version> fromVersion;
            std::optional<Version> toVersion;
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
         Edits the fix makes to a role's texture before binding it -- eg. Chisa's material mask repacked
         into the target's layout, or a colour grade on a diffuse a different shader family draws: a shader
         family is a colour grade, and the texture is the only place to put it back
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
         The float4 slots of the merged skeleton buffers the fix declares for a legacy mod: three per bone of the
         TARGET, in steps of WWMI Tools' own template size (:cpp:member:`WWMINames::MergedSkeletonStep`, 768).
         **Default**: ``0`` -- :cpp:func:`withGameData` derives it from the target's bones (:cpp:class:`VGOffsets` +
         :cpp:class:`VGCounts`) at the run's version
         @endrst
         */
        int mergedSkeletonSlots = 0;

        /**
         * @brief
         @rst
         The size of a merged skeleton for 'bones' bones, in float4 slots: three a bone, rounded up to WWMI Tools' steps
         (:cpp:member:`WWMINames::MergedSkeletonStep`) -- what :cpp:func:`withGameData` fills :cpp:member:`mergedSkeletonSlots`
         with, for the target's :cpp:func:`ModType::getMergedBoneCount`
         @endrst
         *
         * @param bones The bones
         *
         * @return The slots
         */
        static int mergedSkeletonSlotsFor(long long bones);

        /**
         * @brief
         @rst
         Treat a draw whose ``vs-cb3`` carries :cpp:member:`WWMINames::BoneDataFilter` while its ``vs-cb4`` does
         not as one whose skeleton -- the CURRENT pose -- sits in ``vs-cb3``: merge it into the main
         skeleton and bind the remapped main one there, where the default reads ``vs-cb3`` as the
         previous pose on every draw :raw-html:`<br />` :raw-html:`<br />`

         WWMI Tools' own template has the same branch (``elif vs-cb3 == marker`` -> ``vs-cb3 = ref
         ResourceMergedSkeleton``). Some characters draw passes with their skeleton in ``vs-cb3``
         alone -- Lynae's early depth passes, 11 draws of her dump -- and reading it as the previous
         pose wrote their depth for last frame's pose: every thin layered part then failed the later
         passes' equal-depth test and dropped out (her ID card, chest pin and ear cups, first seen on
         her identity mod). Such a draw also takes no remapped previous pose (which every other draw is bound,
         the merge list binds it on every pass). **Default**: ``false``, which writes exactly the lines every
         earlier pair writes
         @endrst
         */
        bool currentPoseInCb3Only = false;

        /**
         * @brief
         @rst
         How many bone influences a vertex the TARGET's draws read, when it differs from the mod's
         own :raw-html:`<br />` :raw-html:`<br />`

         A WWMI blend line is N bone ids then N weights, and the game reads it through the TARGET
         draw's input layout. Changli's is four ``R8G8B8A8`` (8 bytes a vertex) and her Laurel Nymph
         skin's eight ``R8`` of each (16), so a 4-influence blend drawn through the skin's shaders
         had every vertex read across its neighbour's bytes and drew the whole model as giant
         polygons. With this set, the fix's remapped ``Blend.buf`` (and, past 256 bones, its
         ``BlendRemapVertexVG.buf``) is written at this many influences -- the mod's own first, the
         rest bone 0 with weight 0 -- and the generated section's ``stride`` and the blend remap's
         ``weights_per_vertex_count`` say so. When it is FEWER than the mod's (an 8-influence skin
         drawn through Changli's 4-influence shaders), each vertex keeps its heaviest slots, their
         weights scaled so the vertex's total is unchanged, and ``BlendRemapVertexVG.buf`` keeps the
         same slots. **Default**: ``0``, the mod's own count, which every earlier pair uses
         @endrst
         */
        std::size_t targetInfluences = 0;

        /**
         * @brief
         @rst
         Carry a mod's own texture line that puts a texture into a register of a DIFFERENT kind by the
         REGISTER's role rather than the texture's :raw-html:`<br />` :raw-html:`<br />`

         A mod's ``ps-tN = Resource...`` line is normally moved to the register where the target reads
         that texture's role. That is right when the texture plays the role its register holds -- a
         diffuse at the diffuse register -- and wrong when the author bound it somewhere else on
         purpose: a toggle that puts each part's diffuse into the slot the source reads its hair ramp
         or detail map from, to change the shading. Moved by the texture's role, such a line lands on
         the target's diffuse register, where the same diffuse is already bound, and the toggle does
         nothing. With this on, a line whose texture kind (``Diffuse``, ``Detail``, ``Ramp``...)
         differs from its register's role on the source (:cpp:member:`WWMITextureFacts::registerRoles`)
         goes to the target's register for the REGISTER's role, its file unchanged; a register with no
         role there keeps its line where it is. **Default**: ``false``
         @endrst
         */
        bool carryByRegisterRole = false;

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

        // ---- WHAT A FIX DECIDES FROM THE MOD: functions of this config and the parser's data, shared by the compiled
        // fixer and a fixer written from the bound API (each a value a GraphEditVariables can hold) ----

        /**
         * @brief
         @rst
         Whether a component's :cpp:member:`SourceComponent::shader` draws it: it has one, and does not yield to a
         mod that hands the component's maps to RabbitFX (:cpp:member:`Shader::yieldToRabbitFX`)
         @endrst
         *
         * @param component The source component
         * @param usesRabbitFX Whether the mod's sections of the component name RabbitFX
         *
         * @return Whether the shader draws it
         */
        bool shaderDraws(int component, bool usesRabbitFX) const;

        /**
         * @brief
         @rst
         A component's bindings: its :cpp:member:`plan` entry's, with its shader's :cpp:member:`Shader::registers`
         in place of the ones at the same register while the shader draws it (#shaderDraws)
         @endrst
         *
         * @param component The source component
         * @param shaderDrawn Whether its shader draws it
         *
         * @return The bindings
         */
        std::vector<Binding> bindingsOf(int component, bool shaderDrawn) const;

        /**
         * @brief
         @rst
         The roles a component's :cpp:member:`SourceComponent::shader` names in a ``role <name>`` line, of its own
         lines and its passes'
         @endrst
         *
         * @param component The source component
         *
         * @return The roles, in the order the lines are written
         */
        std::vector<std::string> shaderLineRoles(int component) const;

        /**
         * @brief
         @rst
         The roles only a slot's OTHER passes (:cpp:member:`extraPassRegs`, of a slot a drawn component goes
         through), a :cpp:member:`sharedMeshes` mesh or a drawn component's :cpp:member:`SourceComponent::overlay`
         bind -- each once, in that order
         @endrst
         *
         * @param drawn The source components the mod draws
         *
         * @return The roles
         */
        std::vector<std::string> passOnlyRoles(const std::set<int>& drawn) const;

        /**
         * @brief
         @rst
         A texture of the source's own game the fix downloads: from its :cpp:member:`sourceTextures` folders, as
         ``<prefix><kind>RemapDL.dds``, declared under the resource :cpp:func:`IniNamingTools::getRemapDLResourceName`
         names
         @endrst
         */
        struct Download {
            /**
             * @brief Where it is downloaded from
             */
            std::string url;

            /**
             * @brief The file it is saved as, beside the ``.ini``
             */
            std::string file;

            /**
             * @brief The resource section that declares it
             */
            std::string resource;
        };

        /**
         * @brief
         @rst
         The source's game texture of 'hash' as a :cpp:struct:`Download` of the kind 'kind' (``<Role>``, or
         ``<Role>Game`` for a default)
         @endrst
         *
         * @param kind The kind, eg. ``"UpperDiffuse"``
         * @param hash The texture's hash in the source's download folder
         *
         * @return The download
         */
        Download downloadOf(const std::string& kind, const std::string& hash) const;

        /**
         * @brief
         @rst
         A resource the fix declares for one of the mod's files that no resource of the ``.ini`` names (or that
         only a previous fix named): ``[Resource<Role><Target>RemapRef]`` -- RemapRef, since an undo deletes the
         files a RemapFix section names
         @endrst
         */
        struct DeclaredFile {
            /**
             * @brief The file, as the parser names it
             */
            std::string file;

            /**
             * @brief
             @rst
             The file relative to the ``.ini``'s folder, as the ``filename =`` line writes it -- empty for the fix to
             compute it from #file
             @endrst
             */
            std::string rel;

            /**
             * @brief The resource section's name
             */
            std::string resource;
        };

        /**
         * @brief
         @rst
         What each texture role a mod's components bind resolves to -- see #roleResources
         @endrst
         */
        struct RoleResources {
            /**
             * @brief (role, source component) -> the resource that binds the mod's own file for it
             */
            std::map<std::pair<std::string, int>, std::string> ofSlotRole;

            /**
             * @brief Role -> what stands in for it where no file of the mod's serves: a texture the fix invents
             *        (:cpp:member:`createdTextures`), else the source's own game texture, downloaded
             */
            std::map<std::string, std::string> ofRole;

            /**
             * @brief Role -> the mod's file chosen for it
             */
            std::map<std::string, std::string> fileOfRole;

            /**
             * @brief The resources the fix declares for the mod's files, in the order they were named
             */
            std::vector<DeclaredFile> declared;

            /**
             * @brief Role -> the download standing in for it
             */
            std::map<std::string, Download> downloads;

            /**
             * @brief The roles left to the game: a flat texture the parser refused, in
             *        :cpp:member:`flatLeftToGame`, with nothing else of the mod's
             */
            std::set<std::string> leftToGame;

            /**
             * @brief What the decision has to say, for the log, in order
             */
            std::vector<std::string> notes;

            /**
             * @brief
             @rst
             The resource bound for 'role' on 'component': its own, else -- on a component its own shader draws -- the
             download itself, else the stand-in
             @endrst
             * @param role The role
             * @param component The source component
             * @param ownShader Whether the component's own :cpp:member:`SourceComponent::shader` draws it, which reads
             *      a download raw rather than a texture edit of it
             * @return The resource, or ``std::nullopt`` for a role nothing binds
             */
            std::optional<std::string> of(const std::string& role, int component, bool ownShader = false) const;

            /**
             * @brief
             @rst
             The resource the first of 'drawn' to bind 'role' binds (#of)
             @endrst
             * @param role The role
             * @param drawn The source components the mod draws
             * @param ownShader The drawn components their own shader draws
             * @return The resource, or ``std::nullopt`` for a role none binds
             */
            std::optional<std::string> shared(const std::string& role, const std::set<int>& drawn,
                                              const std::set<int>& ownShader = {}) const;
        };

        /**
         * @brief
         @rst
         What each role resolves to on each component the mod draws.

         Every role a drawn component's bindings name, then every #passOnlyRoles role on every drawn component, is
         offered the mod's file the parser chose for it on that component (:cpp:func:`IniParseData::roleFile`). The
         file binds through the resource the mod uses for it (:cpp:func:`IniParseData::resourceOfFile`), or through one
         the fix declares (:cpp:struct:`DeclaredFile`) when the mod names it nowhere -- or only in a previous fix,
         which the run's undo removes. Then the roles the fix invents (:cpp:member:`createdTextures`), and a stand-in
         download for every role of a drawn component no file serves, every role a drawn component's shader names, and
         every pass-only role no component owns -- unless the role is left to the game
         @endrst
         *
         * @param data The parser's data of the mod's ``.ini``
         * @param drawn The source components the mod draws
         * @param rabbitFX The drawn components whose sections name RabbitFX
         * @param sourceName The source character's name
         * @param toModName The target's name
         * @param iniFolder The ``.ini``'s folder
         *
         * @return The roles' resources
         */
        RoleResources roleResources(const IniParseData& data, const std::set<int>& drawn, const std::set<int>& rabbitFX,
                                    const std::string& sourceName, const std::string& toModName,
                                    const std::string& iniFolder) const;

        /**
         * @brief
         @rst
         Where the source's own game texture is the default for a role -- see #gameDefaults
         @endrst
         */
        struct GameDefaults {
            /**
             * @brief Role -> what the draws a section makes before it binds anything bind: the source's own game
             *        texture -- the role's stand-in when the mod has no file for it, else a download of its own
             */
            std::map<std::string, std::string> ofRole;

            /**
             * @brief Role -> the source's game texture, downloaded as ``<prefix><Role>GameRemapDL.dds``
             */
            std::map<std::string, Download> downloads;

            /**
             * @brief (role, component) -> what binds the role outside the toggle the mod binds its own file under
             */
            std::map<std::pair<std::string, int>, std::string> toggled;

            /**
             * @brief The roles whose download is the default, and so is edited as the mod's own file of the role
             *        would be (:cpp:member:`texEdits`)
             */
            std::set<std::string> edited;

            /**
             * @brief What the decision has to say, for the log, in order
             */
            std::vector<std::string> notes;

            /**
             * @brief
             @rst
             The source's game texture of 'role' as 'component' binds it: the download itself for a component its
             own shader draws, else #ofRole's
             @endrst
             * @param role The role
             * @param component The source component
             * @param ownShader Whether the component's own shader draws it
             * @return The resource
             */
            std::string of(const std::string& role, int component, bool ownShader) const;
        };

        /**
         * @brief
         @rst
         THE SOURCE'S OWN GAME TEXTURE AS THE DEFAULT, where the mod's file reaches only some of a component's draws
         -- each a place the original drew with the game's texture:

         #. the draws a section makes BEFORE it binds anything (:cpp:func:`IniParseData::lateBindings`): a role
            the mod has no file for is already the source's game texture (its stand-in), and a role it does serve
            takes a download of its own
         #. a role bound only under a toggle that reaches none of the section's draws
            (:cpp:func:`IniParseData::toggledBindings`) -- unless a ``this`` binds it for every draw anyway
         #. a role bound with one ``this`` under a toggle (:cpp:member:`IniParseData::TextureToggle::single`): off,
            the original draws with the game's texture

         Nothing when the source has no download folder
         @endrst
         *
         * @param data The parser's data of the mod's ``.ini``
         * @param roles What the roles resolve to (#roleResources), with a reroute's entries copied from its source
         *      component
         * @param drawn The source components the mod draws, reroutes included
         * @param rerouteFrom A reroute that took draws -> the component it took them from
         *      (:cpp:struct:`DrawReroute`)
         * @param ownShader The drawn components their own :cpp:member:`SourceComponent::shader` draws
         * @param sourceName The source character's name
         *
         * @return The defaults
         */
        GameDefaults gameDefaults(const IniParseData& data, const RoleResources& roles, const std::set<int>& drawn,
                                  const std::map<int, int>& rerouteFrom, const std::set<int>& ownShader,
                                  const std::string& sourceName) const;

        /**
         * @brief
         @rst
         Which resources each :cpp:member:`texEdits` edit applies to -- see #texEditTargets
         @endrst
         */
        struct TexEditTargets {
            /**
             * @brief Role -> the resources (lower case) its edits apply to
             */
            std::map<std::string, std::set<std::string>> targets;

            /**
             * @brief A resource (lower case) -> the roles edited from it, in the order they claimed it
             */
            std::map<std::string, std::vector<std::string>> rolesOf;

            /**
             * @brief What the decision has to say, for the log, in order
             */
            std::vector<std::string> notes;
        };

        /**
         * @brief
         @rst
         Which resources each of :cpp:member:`texEdits` applies to: the one its role resolves to (#RoleResources::shared)
         and every other variant the mod's toggles bind the role as (:cpp:func:`IniParseData::thisToggleOf`,
         :cpp:func:`IniParseData::registerToggleOf`) -- unless another role's edit of another kind (#roleKind)
         claimed that file first, since a second claim would run that role's filter over it. An edit of a role with
         neither a file of the mod's nor a stand-in download applies to nothing
         @endrst
         *
         * @param data The parser's data of the mod's ``.ini``
         * @param roles What the roles resolve to (#roleResources), a reroute's entries included
         * @param drawn The source components the mod draws
         * @param ownShader The drawn components their own shader draws
         * @param iniPath The mod's ``.ini``
         *
         * @return The targets
         */
        TexEditTargets texEditTargets(const IniParseData& data, const RoleResources& roles, const std::set<int>& drawn,
                                      const std::set<int>& ownShader, const std::string& iniPath) const;

        /**
         * @brief
         @rst
         The routing flags RabbitFX's ``SetTextures`` sets on the SOURCE's own draw of each drawn component that keeps
         RabbitFX (:cpp:member:`SourceComponent::rabbitFX`) and whose sections run ``SetTextures`` -- from what that
         draw has BOUND at ``ps-t0`` .. ``ps-t3``, not from the maps the mod hands RabbitFX. Per register: the mod's own
         line (unless only a toggle sets it), else the mod's ``this`` override of the game texture there, else the game
         texture itself (:cpp:member:`SourceComponent::RabbitFX::gameTags`)
         @endrst
         *
         * @param data The parser's data of the mod's ``.ini``
         * @param roles What the roles resolve to (#roleResources)
         * @param toggled (role, component) -> what binds the role outside a toggle (:cpp:member:`GameDefaults::toggled`)
         * @param drawn Each source component the mod draws (reroutes included) -> its sections
         * @param rerouteFrom A reroute that took draws -> the component it took them from
         * @param iniPath The mod's ``.ini``
         * @param iniFolder The ``.ini``'s folder
         *
         * @return Component -> its four flags (:cpp:func:`RabbitFX::routingFlags`)
         */
        /**
         * @brief
         @rst
         One line a linked ``.ini`` sets on this file's draws, as the fix carries it -- see #linkedLines
         @endrst
         */
        struct CarriedLine {
            /**
             * @brief The condition it is set under, reading the fix's copies of the linked file's variables; empty for none
             */
            std::string condition;

            /**
             * @brief The key it sets (eg. RabbitFX's glow map); empty for a texture set at the register of #role
             */
            std::string key;

            /**
             * @brief The texture role the parser gave the line's file, for a line with no #key
             */
            std::string role;

            /**
             * @brief The value: ``ref <resource>``, the resource naming the file in this ``.ini``
             */
            std::string value;
        };

        /**
         * @brief
         @rst
         A linked file whose state the fix copies, so the carried lines' conditions read it -- see #linkedLines
         @endrst
         */
        struct LinkedState {
            /**
             * @brief The linked ``.ini``
             */
            std::string ini;

            /**
             * @brief The variables of its state the fix copies: the conditions' and every one they depend on
             *        (:cpp:func:`IniFile::varState`)
             */
            std::set<std::string> vars;

            /**
             * @brief What each copied variable's name takes on in the fix
             */
            std::string suffix;
        };

        /**
         * @brief
         @rst
         What linked ``.ini`` files set on this file's draws, and how the fix carries it -- see #linkedLines
         @endrst
         */
        struct LinkedLines {
            /**
             * @brief Source component -> the lines carried onto its draws, in the order the parser read them
             */
            std::map<int, std::vector<CarriedLine>> lines;

            /**
             * @brief The linked files whose state the fix copies, in the order their first line was read
             */
            std::vector<LinkedState> states;

            /**
             * @brief The resources the fix declares for the linked files no resource of this ``.ini`` names, after
             *        :cpp:member:`RoleResources::declared`
             */
            std::vector<DeclaredFile> declared;

            /**
             * @brief What the decision has to say, for the log, in order
             */
            std::vector<std::string> notes;
        };

        /**
         * @brief
         @rst
         What linked ``.ini`` files set on this file's draws (:cpp:func:`IniParseData::linkedLines`), on the components
         the mod draws: each line's value names its file through the resource this ``.ini`` uses for it, or one the fix
         declares (``Resource<Carried|Glow><File><Target>RemapRef``); its condition reads the fix's copies of the linked
         file's variables (suffixed ``<Target>RemapRef``, then ``...RemapRef2``, per file), whose state the fix copies
         (:cpp:struct:`LinkedState`)
         @endrst
         *
         * @param data The parser's data of the mod's ``.ini``
         * @param roles What the roles resolve to (#roleResources): the names it declares are taken
         * @param drawn The source components the mod draws, reroutes included
         * @param linkedInis The run's ``.ini`` files linked to this one (:cpp:func:`IniFile::linkedInis`)
         * @param sourceName The source character's name
         * @param toModName The target's name
         * @param iniFolder The ``.ini``'s folder
         *
         * @return The lines
         */
        LinkedLines linkedLines(const IniParseData& data, const RoleResources& roles, const std::set<int>& drawn,
                                const std::vector<IniFile*>& linkedInis, const std::string& sourceName,
                                const std::string& toModName, const std::string& iniFolder) const;

        std::map<int, std::array<int, 4>> rabbitFXFlags(const IniParseData& data, const RoleResources& roles,
                                                        const std::map<std::pair<std::string, int>, std::string>& toggled,
                                                        const std::map<int, std::vector<std::string>>& drawn,
                                                        const std::map<int, int>& rerouteFrom, const std::string& iniPath,
                                                        const std::string& iniFolder) const;

        /**
         * @brief One line of a texture list the fix writes -- see #textureLists
         */
        struct TextureLine {
            /**
             * @brief The register the line binds (``ib`` for a pass list's ``ib = null``)
             */
            std::string reg;

            /**
             * @brief What it binds: a resource, or ``null``
             */
            std::string value;

            /**
             * @brief The role it binds, or empty for a line that binds none (``null``, ``ib = null``)
             */
            std::string role;

            /**
             * @brief Whether the list keeps the mod's own art unedited: a component its own shader draws, or an
             *        overlay
             */
            bool raw = false;

            /**
             * @brief Whether #toggleLists may put the mod's own toggle of #value in the line's place; false for a
             *        line that is already what binds a role outside its toggle (:cpp:member:`GameDefaults::toggled`)
             */
            bool modToggle = true;

            /**
             * @brief Set by #toggleLists: the copy of the mod's toggle run in the line's place, as ``run = <run>``
             */
            std::string run;
        };

        /**
         * @brief One texture list the fix writes, its lines in order -- see #textureLists
         */
        struct TextureList {
            /**
             * @brief Which list: ``"main"``, ``"game"`` and ``"pass"`` a drawn component's, ``"sharedMesh"`` one pass
             *        of a :cpp:member:`sharedMeshes` mesh, ``"overlay"`` a :cpp:member:`SourceComponent::overlay`'s
             *        bindings
             */
            std::string kind;

            /**
             * @brief The source component, or for ``"sharedMesh"`` the mesh's place in :cpp:member:`sharedMeshes`
             */
            int component = -1;

            /**
             * @brief The pass a ``"pass"`` or ``"sharedMesh"`` list binds on
             */
            std::string onPass;

            /**
             * @brief A ``"pass"`` or ``"sharedMesh"`` list's place among its slot's :cpp:member:`extraPassRegs` or its
             *        mesh's passes
             */
            int passNum = 0;

            /**
             * @brief The lines
             */
            std::vector<TextureLine> lines;
        };

        /**
         * @brief A copy of the mod's own texture override toggling a role's resources, run in a list's place -- see
         *        #toggleLists
         */
        struct ToggleList {
            /**
             * @brief The list's name
             */
            std::string name;

            /**
             * @brief The parser's graph holding the mod's override
             */
            std::pair<std::string, std::string> graph;

            /**
             * @brief The mod's override
             */
            std::string section;

            /**
             * @brief The role it binds
             */
            std::string role;

            /**
             * @brief The register the list binds (every ``this`` is re-keyed to it)
             */
            std::string reg;

            /**
             * @brief Whether it is the copy for a list that keeps the mod's art unedited (:cpp:member:`TextureLine::raw`)
             */
            bool raw = false;

            /**
             * @brief The spellings of ``this`` the override writes
             */
            std::vector<std::string> thisKeys;

            /**
             * @brief The values of a ``this`` naming nothing -- a resource the mod neither declares nor files -- which
             *        the copy drops
             */
            std::set<std::string> deadValues;

            /**
             * @brief A one-variant toggle's binding while it is off: the source's game texture of the role
             */
            std::optional<std::string> offBinding;
        };

        /**
         * @brief Every texture list the fix writes, and the toggles they run -- see #textureLists
         */
        struct TextureLists {
            /**
             * @brief The lists, in the order the fix writes them
             */
            std::vector<TextureList> lists;

            /**
             * @brief The copies of the mod's toggles the lines run (#toggleLists), in the order first run
             */
            std::vector<ToggleList> toggles;

            /**
             * @brief The list of 'kind' for 'component' (and 'passNum'), or ``nullptr``
             */
            const TextureList* find(const std::string& kind, int component, int passNum = 0) const;
        };

        /**
         * @brief
         @rst
         THE FIX'S TEXTURE LISTS, LINE BY LINE. For each drawn component of the plan, in order: its ``"main"`` list,
         each of its bindings (:cpp:func:`bindingsOf`) at its register -- ``null`` as nothing, a role the mod toggles
         against the game's texture by what binds it outside the toggle (:cpp:member:`GameDefaults::toggled`), else the
         role's resource (:cpp:func:`RoleResources::of`); its ``"game"`` list when its section draws before it binds
         anything (:cpp:func:`IniParseData::lateBindings`), each role the source's game texture
         (:cpp:func:`GameDefaults::of`); and a ``"pass"`` list for each of its slot's :cpp:member:`extraPassRegs`, with
         ``ib = null`` where :cpp:member:`extraPassNoDraw` names the pass. Then a ``"sharedMesh"`` list for each pass of
         each :cpp:member:`sharedMeshes` mesh (:cpp:func:`RoleResources::shared`), and an ``"overlay"`` list for each
         drawn :cpp:member:`SourceComponent::overlay`.

         Every line binds a resource directly; #toggleLists then puts the mod's own toggles in
         @endrst
         *
         * @param data The parser's data of the mod's ``.ini``
         * @param roles What the roles resolve to (#roleResources), with a reroute's entries copied from its source
         *      component
         * @param defaults Where the source's game texture is the default (#gameDefaults)
         * @param drawn The source components the mod draws, reroutes included
         * @param rerouteFrom A reroute that took draws -> the component it took them from
         * @param ownShader The drawn components their own :cpp:member:`SourceComponent::shader` draws
         * @param overlaid The drawn components whose overlay is drawn
         *
         * @return The lists
         */
        TextureLists textureLists(const IniParseData& data, const RoleResources& roles, const GameDefaults& defaults,
                                  const std::set<int>& drawn, const std::map<int, int>& rerouteFrom,
                                  const std::set<int>& ownShader, const std::set<int>& overlaid) const;

        /**
         * @brief
         @rst
         THE MOD'S OWN TOGGLES IN THE LISTS. A line binding a resource the mod chooses among others under a toggle
         (:cpp:func:`IniParseData::thisToggleOf`) runs a copy of the mod's texture override instead
         (``CommandList<Source><Role><Reg>[Raw]``, one per role, register and rawness, made on its first line), so the
         toggle comes across rather than collapsing to its first variant: its ``this`` lines re-keyed to the line's
         register, the ones naming nothing dropped, and a one-variant toggle bound to the source's game texture while
         off. A line stays direct when the override has no lines, sits in no graph, or binds nothing that resolves
         @endrst
         *
         * @param lists The lists (#textureLists)
         * @param data The parser's data of the mod's ``.ini``
         * @param defaults Where the source's game texture is the default (#gameDefaults)
         * @param sourceName The source character's name
         * @param toModName The target's name
         * @param iniPath The ``.ini``'s path
         *
         * @return The lists, with the lines that run a toggle and the toggles
         */
        TextureLists toggleLists(TextureLists lists, const IniParseData& data, const GameDefaults& defaults,
                                 const std::string& sourceName, const std::string& toModName,
                                 const std::string& iniPath) const;

        /**
         * @brief One of the mod's own texture-register lines and what the fix makes of it -- see #modTexLines
         */
        struct ModTexLine {
            /**
             * @brief The register as the mod writes it, lowered
             */
            std::string key;

            /**
             * @brief The value as the mod writes it, stripped
             */
            std::string value;

            /**
             * @brief Whether the line is dropped
             */
            bool dropped = false;

            /**
             * @brief The register the line moves to, unless #dropped
             */
            std::string reg;

            /**
             * @brief What it binds there, unless #dropped
             */
            std::string newValue;
        };

        /**
         * @brief
         @rst
         THE MOD'S OWN TEXTURE LINES, RE-KEYED BY ROLE. Each ``ps-tN`` line of the mod's sections, for each
         drawn component of the plan, goes to the register that component's bindings read its texture's role at
         (:cpp:func:`registerOf`) -- the layout of the fix's own list, which the lines sit after. The role is the
         parser's (:cpp:func:`IniParseData::roleOfResource`: one it chose the file for among the roles a drawn component
         binds, this component's first, or the one it refused the file for).

         - With :cpp:member:`carryByRegisterRole`, a texture put into another KIND of slot on purpose goes by the slot:
           to the register for the role the source reads there, its file unchanged -- or is left alone.
         - A line whose role or register is unknown is left alone, unless the component's ``"main"`` list binds that
           register: then it is dropped -- except a file of no known role where the source reads the same kind as the
           target.
         - A file the parser refused for its role does not go back: the line binds what the role resolved to, or is
           dropped where nothing stands in -- except on a component its own shader draws, which keeps the mod's art.

         A line left alone is not listed
         @endrst
         *
         * @param data The parser's data of the mod's ``.ini``
         * @param roles What the roles resolve to (#roleResources), with a reroute's entries copied from its source
         *      component
         * @param lists The texture lists (#textureLists, #toggleLists)
         * @param drawn The source components the mod draws, reroutes included
         * @param rerouteFrom A reroute that took draws -> the component it took them from
         * @param ownShader The drawn components their own :cpp:member:`SourceComponent::shader` draws
         *
         * @return Source component -> its lines the fix moves or drops, in the order the mod first writes them
         */
        /**
         * @brief Which file each drawn component's remapped section goes in -- see #fileGroups
         */
        struct FileGroups {
            /**
             * @brief Per file, in order -- the mod's own ``.ini`` first, then each copy -- the source components it remaps
             */
            std::vector<std::vector<int>> groups;

            /**
             * @brief The drawn components the plan has no target slot for, which are not remapped
             */
            std::vector<int> dropped;

            /**
             * @brief What the decision has to say, for the log, in order
             */
            std::vector<std::string> notes;
        };

        /**
         * @brief
         @rst
         ONE REMAPPED SECTION PER TARGET DRAW PER FILE. In source component order, the first drawn component to claim a
         target slot goes in the mod's own ``.ini`` (group 0), the second in the first copy, and so on -- except one whose
         :cpp:member:`SourceComponent::sameFileSection` is set, which stays in the mod's own file. A drawn component the
         plan names no slot for is dropped. There is always at least one group
         @endrst
         *
         * @param drawn The source components the mod draws, reroutes included
         * @param sourceName The source character's name, for the notes
         *
         * @return The groups
         */
        FileGroups fileGroups(const std::set<int>& drawn, const std::string& sourceName) const;

        /**
         * @brief
         @rst
         The parser's graphs and the sections it placed in each, in the parser's order
         (:cpp:type:`IniParseData::SectionTargets`), as #drawnOf reads it
         @endrst
         */
        using SectionTargets = IniParseData::SectionTargets;

        /**
         * @brief One draw of a component: its ``(index count, first index)``, as :cpp:func:`IniParseData::drawRanges` gives it
         */
        using DrawRange = std::pair<long long, long long>;

        /**
         * @brief
         @rst
         A source component the mod draws from mesh files of its OWN: its section runs a
         ``<sharedResourcesList>.<suffix>`` list (:cpp:member:`WWMINames::OverrideSharedResourcesList`) that binds an index,
         position and blend buffer of its own -- see #drawnOf
         @endrst
         */
        struct OwnMesh {
            /**
             * @brief The mod's own shared-resource list, as the section runs it
             */
            std::string list;

            /**
             * @brief The resource the list binds at the blend register (:cpp:member:`IniKeywords::WWMIBlendReg`)
             */
            std::string blendResource;

            /**
             * @brief That resource's file, as the ``.ini`` names it
             */
            std::string blendFile;

            /**
             * @brief The file of the resource the list binds at ``vb0``, as the ``.ini`` names it
             */
            std::string positionFile;

            /**
             * @brief The section's ``vg_offset``: where the component's bone ids start (``0`` when it has none or it is not a number)
             */
            long long vgOffset = 0;
        };

        /**
         * @brief What the mod draws, read off the parser's data -- see #drawnOf
         */
        struct Drawn {
            /**
             * @brief Each source component whose sections draw -> its sections, a reroute's ``as`` included (sharing its ``from``'s sections)
             */
            std::map<int, std::vector<std::string>> present;

            /**
             * @brief The source components whose sections draw nothing (:cpp:func:`IniParseData::isUndrawn`), in the parser's order
             */
            std::vector<int> silent;

            /**
             * @brief Each drawn source component -> its draws, less the ones a reroute took; a reroute's ``as`` -> the draws it took
             */
            std::map<int, std::vector<DrawRange>> drawRanges;

            /**
             * @brief Each reroute (:cpp:member:`drawReroutes`) that took draws: its ``as`` -> its ``from``
             */
            std::map<int, int> rerouteFrom;

            /**
             * @brief Each reroute that took draws: its ``as`` -> the draws it took
             */
            std::map<int, std::set<DrawRange>> reroutedDraws;

            /**
             * @brief The present components that are not a reroute's ``as``
             */
            std::set<int> drawn;

            /**
             * @brief Of #drawn, the ones whose sections name RabbitFX (:cpp:func:`IniParseData::libraries`)
             */
            std::set<int> rabbitFX;

            /**
             * @brief
             @rst
             The present components their own :cpp:member:`SourceComponent::shader` draws (:cpp:func:`shaderDraws`)
             @endrst
             */
            std::set<int> ownShader;

            /**
             * @brief
             @rst
             The present components their :cpp:member:`SourceComponent::overlay` is drawn over -- not one that hands its
             maps to RabbitFX while the overlay yields to it
             @endrst
             */
            std::set<int> overlaid;

            /**
             * @brief
             @rst
             The present components whose :cpp:member:`SourceComponent::markLines` are written -- not one whose shader
             yields to RabbitFX
             @endrst
             */
            std::set<int> markLines;

            /**
             * @brief
             @rst
             Whether the mod is from before WWMI's merged skeleton: none of its slot sections carries a
             ``vg_offset`` (:cpp:member:`IniKeywords::WWMIVgOffset`), so its blend holds each component's OWN bone indices
             @endrst
             */
            bool legacy = false;

            /**
             * @brief
             @rst
             Whether the mod's ``[Present]`` resets ``$object_detected`` each frame (``post $object_detected = 0``,
             anywhere in it; a ``[Present]`` written more than once is one section)
             @endrst
             */
            bool hostResetsDetected = false;

            /**
             * @brief Each drawn component (not a reroute's ``as``) drawn from mesh files of its own -> those files
             */
            std::map<int, OwnMesh> ownMeshes;

            /**
             * @brief
             @rst
             Why the mod is not remapped, or empty. When set, only #hostResetsDetected, #watching and what was read before
             the reason are filled
             @endrst
             */
            std::string error;

            /**
             * @brief The sections the parser placed in its watching graph (:cpp:member:`WWMIParseObjs::Watch`), in its order
             */
            std::vector<std::string> watching;

            /**
             * @brief What the reading has to say, for the log, in order
             */
            std::vector<std::string> notes;
        };

        /**
         * @brief
         @rst
         WHAT THE MOD DRAWS, off the parser's data and graphs:

         #. each slot graph (:cpp:func:`WWMIParserConfig::slotWithPrefix` with :cpp:member:`WWMINames::SlotPrefix`) of a
            source slot below 'sourceSlots' with sections is a source component: :cpp:member:`Drawn::silent` when its
            sections draw nothing, else :cpp:member:`Drawn::present`, with its draws. With neither, the mod is not
            remapped (:cpp:member:`Drawn::error`)
         #. whether the mod is :cpp:member:`Drawn::legacy`; a legacy mod is not remapped when :cpp:member:`sourceVgMaps`
            is empty
         #. the components drawn from mesh files of their own (:cpp:member:`Drawn::ownMeshes`); one whose blend or
            position resource names no file is left out, with a note
         #. each of :cpp:member:`drawReroutes` whose ``from`` is present with draws, whose ``as`` is planned and not
            present, and whose split (:cpp:member:`DrawReroute::split`) picked draws: those draws move from ``from`` to
            ``as``, which is present from then on with ``from``'s sections
         #. which components keep RabbitFX, and which are drawn by their own shader, under an overlay or with mark lines
            (:cpp:member:`SourceComponent::shader`, :cpp:member:`SourceComponent::overlay`,
            :cpp:member:`SourceComponent::markLines`)
         @endrst
         *
         * @param data The parser's data of the mod's ``.ini``
         * @param targets The parser's graphs and their sections, in its order (:cpp:func:`GIMIParser::sectionTargets`)
         * @param sourceSlots How many draw slots the source has at the fix's source version; a slot graph at or past it
         *      is not a component. ``-1`` for no limit
         * @param sourceName The source character's name, for the notes
         * @param sourceVb0 The source's ``vb0`` hash at the fix's source version, for the notes
         * @param targetName The target character's name, for the notes
         *
         * @return What the mod draws
         */
        Drawn drawnOf(const IniParseData& data, const SectionTargets& targets, int sourceSlots, const std::string& sourceName,
                      const std::string& sourceVb0, const std::string& targetName) const;

        /**
         * @brief
         @rst
         The roles as a fix resolves them once the reroutes are known: each reroute (:cpp:member:`drawReroutes`) reads
         its source component's files, so every ``(role, from)`` entry of :cpp:member:`RoleResources::ofSlotRole` is
         also filed under ``(role, as)``, unless that entry is there already
         @endrst
         *
         * @param roles The #roleResources decision
         * @param rerouteFrom Each reroute that took draws: its ``as`` -> its ``from``
         *
         * @return The roles
         */
        static RoleResources withReroutes(RoleResources roles, const std::map<int, int>& rerouteFrom);

        /**
         * @brief Whether the target's merged skeleton needs WWMI's blend remap -- see #blendWidth
         */
        struct BlendWidth {
            /**
             * @brief
             @rst
             Whether the remapped blend indices pass what an 8-bit blend index names, so the fix writes a blend remap
             (and drops the mod's own skeleton lines: :cpp:member:`remappedSkeletonRemovals`)
             @endrst
             */
            bool targetPast256 = false;

            /**
             * @brief The distinct target bones the vertex group row names
             */
            std::size_t bones = 0;

            /**
             * @brief What the decision has to say, for the log, in order
             */
            std::vector<std::string> notes;
        };

        /**
         * @brief
         @rst
         DOES THE TARGET'S SKELETON PASS AN 8-BIT BLEND INDEX? The vertex group row's largest target bone answers it, a
         property of the PAIR, so the ``.ini`` and the buffers cannot disagree about whether there is a blend remap. A
         row naming more distinct target bones than one blend remap holds (:cpp:member:`WWMIBlend::RemapSize`) falls back
         to an 8-bit blend, and says that every bone past 255 then lands on bone 0
         @endrst
         *
         * @param row The source's vertex group row onto the target, if it has one
         *
         * @return The decision
         */
        BlendWidth blendWidth(const std::optional<VGRemap>& row) const;

        /**
         * @brief
         @rst
         THE LINES EACH REMAPPED SECTION DROPS. Per remapped source component: :cpp:member:`removedRegs`, plus
         :cpp:member:`remappedSkeletonRemovals` when ``targetPast256`` -- without RabbitFX's own lines for a component
         that keeps RabbitFX. A component with nothing to drop is left out. Each is matched with or without ``ref``
         @endrst
         *
         * @param targetPast256 The #blendWidth decision
         * @param components The remapped source components (every #fileGroups group's)
         * @param keepsRabbitFX The components that keep the mod's RabbitFX (the #rabbitFXFlags decision's keys)
         *
         * @return Per component, the lines it drops
         */
        std::map<int, std::vector<RegRemoval>> removedLines(bool targetPast256, const std::set<int>& components,
                                                           const std::set<int>& keepsRabbitFX) const;

        std::map<int, std::vector<ModTexLine>> modTexLines(const IniParseData& data, const RoleResources& roles,
                                                           const TextureLists& lists, const std::set<int>& drawn,
                                                           const std::map<int, int>& rerouteFrom,
                                                           const std::set<int>& ownShader) const;
    };


    /**
     * @brief
     @rst
     Builds the fixer that remaps a mod of one Wuthering Waves character onto another -- see
     :cpp:struct:`WWMIFixerConfig`. One fixer for the pair: every source component of the mod's
     ``.ini`` is handled by it, and the target's components are draw slots rather than mod types
     of their own :raw-html:`<br />` :raw-html:`<br />`

     The fix is a LIST of edits, built for each ``.ini`` from 'config' and what the mod draws, each step named
     (:cpp:member:`BaseIniGraphGroupEdit::name`): ``copies``, ``toggles``, ``slotRemap``, the edits on the target's
     objects by kind (``textureListCalls``, ``sharedBypass``, ``modTexLines``, ``additions``, ``rerouteFilters``,
     ``removedLines``, ``rabbitFXPins``, ``slotValues``, ``hashes``, ``dispatchSize``, ``shapeKeyLists``,
     ``afterShaders``, ``markLines``, ``overlays``, ``rerouteShaders``, ``offPasses``), ``blend``, ``rebind``,
     ``fixSections`` -- the fix's own sections, into graph ``(0, "", "fixSections")``, one step per kind of section
     after it (``textureLists``, ``glowLists``, ``linkedState``, ``declaredFiles``, ``skeletonSections``,
     ``blendRemapSections``, ``bypassSections``, ``hideSections``, ``stashResources``, ``passTags``, ``sharedMeshes``,
     ``hiddenMeshes``, ``shaderDraws``, ``rabbitFXPinLists``), so one kind is read, removed or replaced
     as one step -- then ``shaderFiles``, ``fixFiles``, ``latch`` and ``texEdits``. Change a fix by rewriting that list:
     :cpp:func:`TemplateFixer::withList`
     @endrst
     * @param config The pair's config
     */
    IniFixBuilder::Factory makeWWMIFixer(WWMIFixerConfig config);

    /**
     * @brief
     @rst
     What :cpp:func:`makeWWMIFixer` returns, as an :cpp:type:`IniFixBuilder::Factory`: the pair's config and the call that
     builds a fixer from it -- so the config can be read back
     (:cpp:func:`WWMIFixerConfig::ofFactory`)
     @endrst
     */
    struct WWMIFixFactory {
        /**
         * @brief The pair's config
         */
        WWMIFixerConfig config;

        /**
         * @brief Builds the fixer -- see :cpp:type:`IniFixBuilder::Factory`
         */
        std::shared_ptr<BaseIniFixer<>> operator()(BaseIniParser<>* parser, const std::string& toModName,
                                                   std::optional<int> modTypeId) const;
    };
}

#endif
