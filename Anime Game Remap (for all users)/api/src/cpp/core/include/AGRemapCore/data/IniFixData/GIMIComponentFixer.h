#ifndef AGRemapCore_GIMIComponentFixer_H
#define AGRemapCore_GIMIComponentFixer_H

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
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What one remap onto a skin of SEVERAL COMPONENTS does differently -- everything
     :cpp:func:`makeGIMIComponentFixer` needs that is not the same for every such pair
     :raw-html:`<br />` :raw-html:`<br />`

     A GIMI character of the classic shape is one mesh: one ``Blend.buf``, one set of draw calls,
     one vertex-group numbering. The newer skins are not -- YelanTranquil (5.7) is a ``Body``, a
     ``Bang`` and an ``Eye``, each with its own buffers, hashes and vertex-group index space, and
     every GI character from Bennett on is built the same way. Remapping a classic-shape mod onto
     one is therefore not a rename of hashes: the mod's ONE set of buffers has to be SPLIT into
     one set per component, each skinned in that component's bones, and each component's draw
     slot has to be told to draw the part of the mod that lands in it :raw-html:`<br />`
     :raw-html:`<br />`

     The library's answer (issue #190, confirmed in game on four Yelan mods on 2026-09-12): one
     fixer PER TARGET COMPONENT, each a row in :cpp:class:`IniFixBuilderData` keyed
     ``(source, <skin><component>)`` -- the component is a :cpp:enum:`ModTypeId` of its own, so
     the naming, the hash remap and the merge all work unchanged -- and each fixer:

     #. splits the mod's geometry ONCE (:cpp:class:`VGComponentSplit`, every component at once,
        since which triangles a cut component gets depends on what the negative-index ones keep)
        to learn which of the mod's objects its component draws and how many vertices it keeps
     #. copies every drawn object's graph onto the component's draw slot
        (:cpp:class:`GraphGroupRemap`). Two objects on one slot is the API's **merge**: the second
        claimant lands in a further ``.ini`` group, written as ``<name>RemapFix1.ini``
     #. edits the textures the slot reads (a diffuse edit, a lightmap edit built per object from
        the object's diffuse, a flat normal map created where the slot's layout has one)
     #. collects the blend / position / texcoord / ib registers as ONE resource group
        (:cpp:class:`ResGroupCollect` + :cpp:class:`BufReplace` + :cpp:class:`VGSplitGroupResource`),
        because the buffers cannot be split one at a time
     #. rewrites the ``match_first_index`` (windowed to the object's own KVPs), drops the mod's fix
        calls, moves the draw call onto the slot (``BottomCover``, after everything the section sets
        up), re-issues ``ORFix`` or ``NNFix``, remaps the hashes, writes the vertex-limit
        overrides the target's own buffer size makes necessary, and swaps the face registers

     The prototype every field here was read off is ``Tools/Misc/Prototypes/yelanTranquilFix.py``,
     and the guide is Creating Remaps' "Porting the Yelan prototype into C++"
     @endrst
     */
    struct GIMIComponentFixerConfig {
        /**
         * @brief
         @rst
         Which texture layout the SOURCE mod's object sections bind, which decides whether a
         normal-map slot needs the register shift and the created normal map
         :raw-html:`<br />` :raw-html:`<br />`

         * ``Plain`` (the default, and what every config before Citlali assumed): ``ps-t0``
           diffuse, ``ps-t1`` light map -- the shift and the flat normal map are applied
         * ``NormalMap``: ``ps-t0`` normal map, ``ps-t1`` diffuse, ``ps-t2`` light map, already the
           layout a normal-map slot reads -- the mod's own three textures pass through
         * ``Detect``: per OBJECT, the normal-map layout when the object's section binds ``ps-t2``,
           the plain one otherwise. Right for a character whose own shader is the normal-map family
           (Citlali), where a mod binding ``ps-t2`` is binding a light map. NOT safe as a default:
           some Bennett mods bind a metal map at ``ps-t2`` on the plain layout
         @endrst
         */
        enum class SourceLayout { Plain, NormalMap, Detect };


        /**
         * @brief One component of the target skin, and how the mod is drawn through it
         */
        struct Component {
            /**
             * @brief The component's name, as the vertex-group table's component column spells it, eg. ``"Body"``
             */
            std::string name;

            /**
             * @brief
             @rst
             The :cpp:enum:`ModTypeId` that stands for this component as a fix TARGET -- the name
             the :cpp:class:`IniFixBuilderData` row, the hash rows and the index rows are keyed by,
             eg. ``YelanTranquilBody``
             @endrst
             */
            std::string modTypeName;

            /**
             * @brief
             @rst
             The target's draw slot the mod is drawn through -- the object name of the component's
             :cpp:class:`Indices` row that holds the slot's ``match_first_index``, eg. ``"C"``
             @endrst
             */
            std::string slot;

            /**
             * @brief
             @rst
             The slot's ``match_first_index`` on the target, eg. ``"67374"`` -- written onto every
             copied object's section :raw-html:`<br />` :raw-html:`<br />`

             Held here rather than in :cpp:class:`IndexData`, deliberately: a reverse lookup there
             resolves through the newest version bucket holding a value, so a slot at index ``0``
             filed under a 5.x skin would shadow every classic character's head (index ``0`` at
             4.0). See the note in ``IndexData.cpp``. Empty falls back to the table, for a target
             whose rows are safe to file
             @endrst
             */
            std::string slotIndex;

            /**
             * @brief
             @rst
             ``true``: the **negative-index** strategy (the component draws the whole mod, every
             bone of another component becomes a sentinel, and the ib is trimmed); ``false``: the
             **graph cut** (the component takes the triangles the negative-index components leave,
             and every buffer is filtered to the vertices it uses). See :cpp:class:`VGComponentSpec`
             @endrst
             */
            bool negativeIndex = false;

            /**
             * @brief
             @rst
             Whether the slot's shader reads the normal-map layout -- ``ps-t0`` normal map,
             ``ps-t1`` diffuse, ``ps-t2`` lightmap, re-slotted by ``ORFix`` -- in which case the
             mod's ``ps-t0`` / ``ps-t1`` are shifted up, a flat normal map is created on ``ps-t0``
             and ``ORFix`` is re-issued. ``false``: ``ps-t0`` diffuse / ``ps-t1`` lightmap under
             ``NNFix``, textures untouched
             @endrst
             */
            bool normalMap = true;

            /**
             * @brief Whether this component's fix carries the face graph (and swaps its registers) -- exactly one component should
             */
            bool face = false;

            /**
             * @brief
             @rst
             The TARGET component's Texcoord stride, or ``0`` to write the mod's own
             :raw-html:`<br />` :raw-html:`<br />`

             A mod's Texcoord is whatever its author made it -- vanilla Bennett's is 12 (``COLOR`` +
             ``TEXCOORD``) and a mod that carries a second UV set is 20 -- while the SLOT it is being
             drawn through reads a fixed layout. Hand a slot whose shader reads ``TEXCOORD1`` at
             offset 12 a 12-byte buffer, and every such read lands in the NEXT vertex's ``COLOR``.

             So the written buffer is brought to this width: zero-padded at the END, which is exactly
             where ``TEXCOORD1`` sits, or truncated there when the mod carries one and the target
             does not. Both directions are real, and which one a mod needs cannot be told from the
             character -- only from its own file.

             ``0`` leaves the mod's own stride alone, which is what YelanTranquil's config relies on:
             Yelan's Texcoord is already 20, the width all three of her slots read
             @endrst
             */
            std::size_t texcoordStride = 0;

            /**
             * @brief
             @rst
             Every ``ps-t`` register the TARGET's own slot binds, or empty to leave the registers
             alone :raw-html:`<br />` :raw-html:`<br />`

             A remapped section inherits its ``ps-t`` lines from the MOD's section, and a mod binds
             whatever its SOURCE slot reads -- Bennett's body binds four (diffuse, light map, metal
             map, shadow ramp). A register beyond this list is not a harmless extra: the target's
             shader reads that slot as something else, and the target's own mod leaves it unbound
             so the GAME's texture serves it. Blank white eyes were exactly that.

             A register bound TWICE in one part keeps its first binding, for the same reason: the
             later one silently discards the earlier, and on a normal-map slot the shifted light map
             lands on ``ps-t2`` ahead of the mod's own metal map there.

             Read these off the target's identity mod, never off a mod of it. Only a literal
             ``ps-t<number>`` is considered, so the fix's own scratch register is untouched
             @endrst
             */
            std::vector<std::string> slotRegisters;
        };

        /**
         * @brief
         @rst
         The target skin's name in the vertex-group table -- the ``toName`` column of every
         ``(source, "") -> (skin, component)`` row, eg. ``"YelanTranquil"``
         @endrst
         */
        std::string targetSkin;

        /**
         * @brief The source's drawn objects, lowercase, in the order the game draws them -- must match the parser's
         */
        std::vector<std::string> drawnObjs;

        /**
         * @brief Every component of the target, whatever component this particular fixer is for -- the split is joint
         */
        std::vector<Component> components;

        /**
         * @brief
         @rst
         The TARGET components nothing is remapped onto, by their ``ModTypeId`` names -- their own
         draw is suppressed :raw-html:`<br />` :raw-html:`<br />`

         A component absent from :cpp:member:`components` is not thereby absent from the GAME: it
         still draws the skin's own geometry, on top of whatever the mod put there. Bennett has no
         hair bone, so his forward Bang vertex-group row is empty and a Bang fixer would draw
         nothing -- but BennettAdventure's own fringe then sits over the hair his Body component
         draws, which in game reads as two different whites.

         Each name here becomes one ``TextureOverride`` on that component's ib hash carrying
         ``handling = skip`` and no draw, written once into the mod's own ``.ini`` by the fixer for
         the LAST entry of :cpp:member:`components` -- one owner, so several fixers over one file
         cannot write the same section twice, and the last, because each fixer's output replaces
         the ``.ini`` rather than adding to it.

         Empty for a skin every component of which receives geometry, which is YelanTranquil
         @endrst
         */
        std::vector<std::string> hiddenComponents;

        /**
         * @brief
         @rst
         The skin's draw SLOTS nothing is remapped onto, as ``{target component ModTypeId name,
         {match_first_index, ...}}`` -- every slot of a remapped component other than its
         :cpp:member:`Component::slotIndex` :raw-html:`<br />` :raw-html:`<br />`

         Those draws are already skipped (the component's ib-hash section), and that is not enough
         when the mod calls `TexFx`_. ``run = CommandList\TexFx\TN.0`` does not draw: it sets
         ``$use_default_shader = 2``, a request that TexFx's outline shader regex serves on the
         NEXT outline draw it sees with ``drawindexed = auto`` -- the mod's own, on the character
         the mod was made for. On a skin whose slots draw in a different order, the next outline
         draw may be a slot nothing is remapped onto: TexFx then draws the skin's WHOLE index
         buffer over the mod's remapped vertex buffers, and in game that is strands of stretched,
         shiny triangles between the arms and the hair (Citlali onto CitlaliWhisperofStars,
         2026-09-22: slot B's outline draws before slot A's, and 124851 indices went through the
         mod's buffers) :raw-html:`<br />` :raw-html:`<br />`

         Each slot here becomes one ``TextureOverride`` on the component's ib hash and that
         ``match_first_index``, withdrawing the request (``$\TexFx\use_default_shader = -1``).
         Written by the same owner as :cpp:member:`hiddenComponents`, and only when the mod's
         ``.ini`` calls TexFx at all -- the variable is TexFx's, and naming it with the library
         absent is a load-time warning
         @endrst
         */
        std::vector<std::pair<std::string, std::vector<std::string>>> unremappedSlots;

        /**
         * @brief
         @rst
         A diffuse edit per SOURCE object, applied on normal-map slots only, eg. the head diffuse
         at alpha 1 (the target's shader darkens by diffuse alpha, the source's ignores it). An
         object not named here keeps its diffuse as is
         @endrst
         */
        std::vector<std::pair<std::string, TexEditor::Filter>> diffuseEdits;

        /**
         * @brief
         @rst
         Builds the lightmap edit for one object from the path of that object's DIFFUSE, or empty
         for no lightmap edit. Applied on normal-map slots, to every object :raw-html:`<br />`
         :raw-html:`<br />`

         Why a factory over the diffuse: a lightmap's alpha is a material band and the legend is
         the mod AUTHOR's, not the skin's -- a port keeps its source character's bands -- so a band
         cannot be moved by number alone. The edit reads the diffuse under each pixel to decide
         (skin-coloured or not). See Creating Remaps' "The Yelan lessons"
         @endrst
         */
        std::function<TexEditor::Filter(const std::string& diffusePath)> lightMapEdit;

        /**
         * @brief
         @rst
         The SOURCE objects :cpp:member:`lightMapEdit` applies to, or EMPTY for every object
         :raw-html:`<br />` :raw-html:`<br />`

         A band legend is per OBJECT, not per character, so a move that is right on one object is
         wrong on another: band ``0`` is Bennett's silver HAIR on his head and his dark CLOTH on his
         body. The diffuse gate does not save it on its own -- measured on his shipped 4.0 assets, a
         bright-and-neutral gate takes 84.7% of his head lightmap, which is the whole hair, and still
         6.0% of his BODY's: 62462 pixels that mean something else there.

         Empty means every object, which is what YelanTranquil relies on -- both of her moves are
         gated on the diffuse and her legend does not change between her objects
         @endrst
         */
        std::vector<std::string> lightMapObjs;

        /**
         * @brief
         @rst
         The flat normal map created for a normal-map slot, pre-corrected for the sRGB header the
         game reads it under (``127 -> 55``). **Default**: ``(55, 55, 255, 255)``, 1024 x 1024
         @endrst
         */
        Colour flatNormal = Colour(55, 55, 255, 255);

        /**
         * @brief The created normal map's size
         */
        int flatNormalSize = 1024;

        /**
         * @brief Whether every texture written carries its mip chain (every texture the game ships does). **Default**: ``true``
         */
        bool mipmaps = true;

        /**
         * @brief
         @rst
         Whether the edited diffuses and light maps are BC7-compressed :raw-html:`<br />`
         :raw-html:`<br />`

         ON by default here, where :cpp:member:`GIMIMergeFixerConfig::compressTextures` is off, and
         the difference is history rather than judgement: this template hardcoded compression before
         that one existed, and YelanTranquil is confirmed in game with it on. Turning it off for a
         character is safe; turning it off for everyone would move her output.

         Turn it OFF for any config with a band table. The thing being edited IS the alpha and the
         alpha IS a band selector, so the encoder is free to move a value that has to match exactly
         -- measured on Bennett, BC7 moved an UNEDITED body map's band from 255 to 254, and a band
         legend reads that as a different material
         @endrst
         */
        bool compressTextures = true;

        /**
         * @brief
         @rst
         Whether the vertex colour's G and B are set to 128 in the split texcoord -- what both game
         models say, where a mod may carry anything. **Default**: ``true``
         @endrst
         */
        bool normaliseVertexColour = true;

        /**
         * @brief
         @rst
         Whether a 20-byte texcoord's second UV set is zeroed -- the target's opaque vertices
         carry none. **Default**: ``true``
         @endrst
         */
        bool zeroSecondUV = true;

        /**
         * @brief The SOURCE mod's texture layout -- see :cpp:enum:`SourceLayout`. **Default**: ``Plain``
         */
        SourceLayout sourceLayout = SourceLayout::Plain;

        /**
         * @brief
         @rst
         Whether the face's two-way ``ps-t0`` <-> ``ps-t1`` swap runs ONLY when the mod binds its
         face diffuse at ``ps-t0`` :raw-html:`<br />` :raw-html:`<br />`

         GI 6.x swapped which register the face shader reads the diffuse and the light map from. A
         mod authored before that binds its face diffuse at ``ps-t0`` and needs the swap; one built
         from a 6.x dump -- every identity mod -- already binds ``ps-t1``, and the swap moves it the
         WRONG way. ``false`` (the default) swaps unconditionally, which is what every config before
         Citlali's was confirmed in game with
         @endrst
         */
        bool faceSwapOnlyFromDiffuseReg = false;

        /**
         * @brief What every generated ``.ini`` file opens with -- see :cpp:member:`GIMIFixer::copyPreamble`
         */
        std::string copyPreamble;
    };


    /**
     * @brief
     @rst
     Builds the fixer that remaps a classic-shape GIMI mod onto ONE component of a
     multi-component skin, from the config -- see :cpp:class:`GIMIComponentFixerConfig`
     :raw-html:`<br />` :raw-html:`<br />`

     Called once per component of the target, each result a row of :cpp:class:`IniFixBuilderData`
     keyed by that component's own :cpp:enum:`ModTypeId`
     @endrst
     *
     * @param config The pair's config -- the same one for every component
     * @param component Which of ``config.components`` this fixer is for, by name
     */
    IniFixBuilder::Factory makeGIMIComponentFixer(GIMIComponentFixerConfig config, std::string component);
}

#endif
