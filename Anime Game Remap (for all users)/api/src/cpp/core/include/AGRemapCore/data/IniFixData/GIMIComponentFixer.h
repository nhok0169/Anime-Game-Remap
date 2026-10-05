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

#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iniresources/VGSplitGroupResource.h"
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

     The library's answer (issue #190): one fixer PER TARGET COMPONENT, each a row in :cpp:class:`IniFixBuilderData` keyed
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
     @endrst
     */
    struct GIMIComponentFixerConfig {
        /**
         * @brief
         @rst
         Which texture layout the SOURCE mod's object sections bind, which decides whether a
         normal-map slot needs the register shift and the created normal map
         :raw-html:`<br />` :raw-html:`<br />`

         * ``Plain`` (the default): ``ps-t0``
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
             Per SOURCE object, the ``match_first_index`` of the slot that object is drawn through
             instead of :cpp:member:`slotIndex` -- eg. ``{{"body", "53529"}}``. An object not listed
             uses :cpp:member:`slotIndex` :raw-html:`<br />` :raw-html:`<br />`

             A skin's slots draw on DIFFERENT pixel shaders, and a source object shaded by the wrong
             one renders wrong in ways the textures cannot explain. Eg. CharlotteHurlock draws its
             Body slot A (hair and skin) on a hair shader and its slot B (the outfit) on
             ``6546504e`` -- Charlotte's own -- so a Charlotte mod's body belongs in slot B: drawn
             through slot A it gets black shards over dark clothing. Each source object is its
             own ``.ini`` group already, so each can take its own slot. **Default**: empty
             @endrst
             */
            std::vector<std::pair<std::string, std::string>> objSlotIndices;

            /**
             * @brief
             @rst
             EVERY draw slot of this TARGET component, by ``match_first_index`` -- eg. ``{"0", "46620",
             "71025"}`` -- or empty to declare none :raw-html:`<br />` :raw-html:`<br />`

             What lets the fixer work out, per mod, which of the skin's slots nothing is drawn through:
             every slot here that no DRAWN source object is routed to (:cpp:member:`slotIndex` /
             :cpp:member:`objSlotIndices`), and every slot of a component the mod draws nothing onto at
             all, gets the TexFx guard :cpp:member:`GIMIComponentFixerConfig::unremappedSlots` writes --
             worked out from the routing itself, so unlike that hand-kept list it cannot go stale when
             the routing changes. A slot left unguarded can spend a mod's TexFx transparency request
             (eg. a see-through shirt vanishes). Added to, never replacing, that field. **Default**:
             empty
             @endrst
             */
            std::vector<std::string> slotIndices;

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
             For a cut component: the least share of a vertex's weight on this component's groups for it
             to claim the vertex -- see :cpp:member:`VGComponentSpec::claimShare`. **Default**: ``0``,
             the plain majority
             @endrst
             */
            double claimShare = 0.0;

            /**
             * @brief
             @rst
             For a cut component: the source groups this component does NOT own, each to the bone of
             this component that stands in for it -- see :cpp:member:`VGComponentSpec::secondary`. A
             group the component's own row already maps is ignored here. **Default**: empty, a foreign
             weight is dropped
             @endrst
             */
            std::unordered_map<long long, long long> standIns;

            /**
             * @brief
             @rst
             For a cut component: how many rings of its neighbours' triangles it draws as well, so a seam
             that opens when the skin poses is covered -- see :cpp:member:`VGComponentSpec::overlapRings`.
             **Default**: ``0``, no overlap
             @endrst
             */
            std::size_t overlapRings = 0;

            /**
             * @brief
             @rst
             Whether this component's remapped sections drop the mod's `TexFx`_ transparency: its
             ``ps-t69`` / ``ps-t70`` bindings and every ``run = CommandList\\TexFx\\...`` :raw-html:`<br />` :raw-html:`<br />`

             TexFx replaces a draw's pixel shader with its own, recognised by shader pattern, and a
             skin's slot may use a shader it does not recognise. There the request is not served and the
             transparency texture bound at ``ps-t69`` blanks the part out entirely (eg. a sheer shirt
             on NeuvilletteMelusent's main mesh vanishes although its triangles are skinned and drawn).
             Dropped, the part draws opaque -- a texture fault, where a missing
             part is a geometry one. **Default**: ``false``, the mod's TexFx lines are kept
             @endrst
             */
            bool dropTexFx = false;

            /**
             * @brief
             @rst
             For a component whose mod's TexFx transparency is dropped (:cpp:member:`dropTexFx`): the
             opacity its SEE-THROUGH draws are blended at instead, 0 to 1; ``0`` leaves them opaque
             :raw-html:`<br />` :raw-html:`<br />`

             Which draws are see-through is the mod's own answer: TexFx reads opacity off the RED
             channel of the mask it binds at ``ps-t69`` (0 opaque, 1-254 see-through, 255 not drawn),
             so each ``drawindexed = <count>, <start>, ...`` range of an object with a mask is judged by
             the mask under its triangles' UVs, and a range most of whose vertices sit on 1-254 is drawn
             through a ``CustomShader`` of its own. That keeps the game's shaders and blends only the
             G-buffer's COLOUR target (``o1``), at this factor, leaving the normals and material ids of
             whatever is under it: blending all of them, or the colour alone at a low factor, washes a
             dark garment out to white, since a mix is dominated by the brighter surface. As a guide,
             ``0.9`` looks close to TexFx's own transparency and ``0.45`` is nearly invisible. An ``auto`` draw, and a mod that
             branches, keep their draw as it is. **Default**: ``0``
             @endrst
             */
            float texFxBlend = 0.0f;

            /**
             * @brief
             @rst
             The SOURCE objects (lowercase, eg. ``"dress"``) whose triangles get a MIRRORED INNER LAYER
             on this component -- see :cpp:member:`VGComponentSpec::mirroredIbs` :raw-html:`<br />`
             :raw-html:`<br />`

             For single-layer cloth whose inside the target's shader does not shade as cloth: eg. on
             NeuvilletteMelusent the back faces of a Neuvillette dress are lit like rim light (an inner
             skirt comes out flat light blue), where his own shader shades them like the outside. Each
             triangle gets a twin wound the other way, its normal turned round and moved
             :cpp:member:`mirrorOffset` inward, so the inside is a front face with a normal that faces
             the viewer. Doubles those objects' triangles. Cut components only. **Default**: empty
             @endrst
             */
            std::vector<std::string> mirroredObjs;

            /**
             * @brief
             @rst
             How far inside the surface the mirrored layer sits, in model units -- enough that the depth
             buffer always puts it BEHIND the surface seen from outside, small enough not to show as a gap
             :raw-html:`<br />` :raw-html:`<br />`

             The game's depth buffer cannot separate surfaces a millimetre apart at the outfit preview's
             distance: at ``0.001`` the twins, their normals turned round, fight the surface and a
             mirrored garment comes out dark and speckled, "metallic". ``0.004`` to ``0.008`` render as
             cleanly as no layer at all. **Default**: ``0.005`` (5 mm on a GI character)
             @endrst
             */
            float mirrorOffset = 0.005f;

            /**
             * @brief
             @rst
             How far behind a :cpp:member:`mirroredObjs` triangle a layer of the mod facing the other way makes its
             twin unneeded, in model units -- see :cpp:member:`VGComponentSpec::mirrorBackedReach` :raw-html:`<br />`
             :raw-html:`<br />`

             A coat modelled with its own lining is two surfaces a few millimetres apart, and a twin moved
             :cpp:member:`mirrorOffset` inward from one lands in front of the other, as flat grey polygons over the
             coat. A triangle backed within this distance gets no twin. **Default**: ``0``, every triangle is mirrored
             @endrst
             */
            float mirrorBackedReach = 0.0f;

            /**
             * @brief
             @rst
             Whether the :cpp:member:`mirroredObjs` layer reads the source's back-face UVs (``TEXCOORD1``) where it has
             them -- see :cpp:member:`VGSplitGroupConfig::mirrorBackUV`. A source whose cloth shader is two-sided textures
             its back faces that way, and the layer stands in for them. **Default**: ``false``
             @endrst
             */
            bool mirrorBackUV = false;

            /**
             * @brief
             @rst
             Source groups whose weight this component SHARES among several of its bones, as
             ``{source group: [(bone, share), ...]}`` -- see :cpp:member:`VGComponentSpec::splitGroups`. For a
             cloth part the target has no counterpart for, between a bone it clips on and one it folds on.
             **Default**: empty
             @endrst
             */
            std::unordered_map<long long, std::vector<std::pair<long long, double>>> splitGroups;

            /**
             * @brief
             @rst
             Cloth pushed HORIZONTALLY away from a point on this component, by its weight share on the push's
             source groups -- see :cpp:member:`VGSplitGroupConfig::pushAway`. For cloth that clips a limb the
             target moves differently. Cut components only. **Default**: empty
             @endrst
             */
            std::vector<VGPushAway> pushAway;

            /**
             * @brief
             @rst
             The SOURCE objects (lowercase, eg. ``"head"``) whose INNER layers draw no outline on this component --
             see :cpp:class:`InnerLayerOutline` :raw-html:`<br />` :raw-html:`<br />`

             For hair of close two-sided sheets: on a skin whose outline sits further out than the character's own,
             the inner face's outline shell comes out in front of the outer face as small dark shards (eg. long layered
             hair on YaoyaoBamboo). The outer faces keep theirs, so the silhouette outline stays. Needs
             the component's ``Position.buf`` (not a :cpp:member:`negativeIndex` one). **Default**: empty
             @endrst
             */
            std::vector<std::string> innerOutlineObjs;

            /**
             * @brief How far along its normal a vertex looks for a covering layer -- :cpp:member:`InnerLayerOutline::reach`. **Default**: ``0.1``
             */
            float innerOutlineReach = 0.1f;

            /**
             * @brief Whether a face turned in towards the head's vertical axis is inner too -- :cpp:member:`InnerLayerOutline::facingAxis`. **Default**: ``true``
             */
            bool innerOutlineFacingAxis = true;

            /**
             * @brief
             @rst
             Whether the slot's shader reads the normal-map layout -- ``ps-t0`` normal map,
             ``ps-t1`` diffuse, ``ps-t2`` lightmap, re-slotted by ``ORFix`` -- in which case the
             mod's ``ps-t0`` / ``ps-t1`` are shifted up, a flat normal map is created on ``ps-t0``
             and ``ORFix`` is re-issued. ``false``: ``ps-t0`` diffuse / ``ps-t1`` lightmap under
             ``NNFix`` -- a mod object on the plain layout is left as it is, and one already on the
             normal-map layout (see :cpp:member:`GIMIComponentFixerConfig::sourceLayout`) has its
             normal map dropped and its diffuse and lightmap moved down to ``ps-t0`` / ``ps-t1``,
             since a plain slot has nowhere to put the normal map
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
             A translation, in model space, added to every vertex position written for this
             component, or all zeros to write the mod's own positions :raw-html:`<br />` :raw-html:`<br />`

             A mod's vertices are in its SOURCE's bind pose, and a bone of the target moves them
             relative to the TARGET's bind pose. For most parts the difference is invisible -- hair or
             a coat a centimetre higher still reads as the same outfit -- but a part that has to sit
             inside something the GAME draws cannot be off at all. The eyes sit in the sockets of a
             face mesh neither mod carries: eg. NeuvilletteMelusent's eye mesh is Neuvillette's own
             moved 1.24 cm lower, so unshifted, his irises sit behind the skin's upper lid and the
             eyes look white with no pupils.

             Measure it rather than guess it: take the target component's own Position buffer and the
             component this fix writes for the character's identity mod, and difference them vertex
             for vertex (the ib and UVs of both must agree first). A translation is right only when
             the residual is small against the part's size; anything else is not a shift and needs
             its own edit.

             Only the position is moved: a translation leaves normals and tangents as they are
             @endrst
             */
            std::array<float, 3> positionOffset = {0.0f, 0.0f, 0.0f};

            /**
             * @brief
             @rst
             The ``y`` range over which :cpp:member:`positionOffset` FADES to nothing -- full at or
             below ``[0]``, none at or above ``[1]``, linear between. ``{0, 0}`` (the default) is no
             fade at all: every vertex takes the whole offset :raw-html:`<br />` :raw-html:`<br />`

             **A character and a skin of different HEIGHT cannot be matched by a rigid translation.**
             Lifting a mod so its soles meet the target's ground also lifts its head -- and the GAME
             draws the FACE, at its own fixed place, for any mod that does not carry a face mesh of its
             own. Citlali's sole sits 4.5 cm below CitlaliWhisperofStars' and her head only 1 cm above:
             lift the whole model and the feet come right while the face is left 4.5 cm below the head
             it belongs to, which is what a user reported (2026-10-04) :raw-html:`<br />`
             :raw-html:`<br />`

             Fading the lift out by the height of the face anchors the head where the game's face is and
             takes the difference up through the body, which is where two models of one character
             actually differ -- the skin's foot and knee groups sit 2 to 6 cm higher than Citlali's while
             her head group sits 1 cm lower. Set ``[0]`` to the source's sole and ``[1]`` just above its
             eyes

             .. note::
                This is a DEFORMATION, not a translation, so it is only right where the two models
                genuinely differ in height. Where they differ by an ORIGIN instead -- Xiangling and
                XianglingCheer, 0.78 units apart -- the whole model must move and the fade must stay off
             @endrst
             */
            std::array<float, 2> positionOffsetFade = {0.0f, 0.0f};

            /**
             * @brief
             @rst
             Whether :cpp:member:`positionOffset` applies only while the mod draws with the GAME's face
             :raw-html:`<br />` :raw-html:`<br />`

             The offset fits the source's part into the face the GAME draws. A mod can hide that face --
             ``handling = skip`` on the source's face diffuse hash -- and draw its own inside its head
             mesh, which reaches the target unshifted; its eyes are placed for THAT face, and shifting
             them drops them below it: such a mod's eyes look down on the skin unless they are left
             exactly where the mod put them.

             Read per ``.ini`` file: a file whose face-diffuse section skips the draw keeps the mod's own
             positions for this component. **Default**: ``false``, the offset always applies
             @endrst
             */
            bool offsetOnlyWithGameFace = false;

            /**
             * @brief
             @rst
             Every ``ps-t`` register the TARGET's own slot binds, or empty to leave the registers
             alone :raw-html:`<br />` :raw-html:`<br />`

             A remapped section inherits its ``ps-t`` lines from the MOD's section, and a mod binds
             whatever its SOURCE slot reads -- Bennett's body binds four (diffuse, light map, metal
             map, shadow ramp). A register beyond this list is not a harmless extra: the target's
             shader reads that slot as something else, and the target's own mod leaves it unbound
             so the GAME's texture serves it -- an eye slot given one renders as blank white eyes.

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
         shiny triangles between the arms and the hair (eg. Citlali onto CitlaliWhisperofStars,
         whose slot B outline draws before slot A's) :raw-html:`<br />` :raw-html:`<br />`

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
         The hash types of the source's SIDE MESHES -- its own draws that are no mod object, such as
         the face and the head-upper, eg. ``{"ib_face", "ib_headupper"}`` :raw-html:`<br />` :raw-html:`<br />`

         A mod may hide one of them by hash to put something of its own in its place (eg. a mask:
         ``ib = null`` on Neuvillette's face, head-upper and eyebrow meshes). The skin draws its
         OWN side meshes, under different hashes, so a section left on the source's hash hides
         nothing and the skin's face would show through the mask in pieces. Each such section of the mod
         is written again on the target's hash of the same type -- filed in ``HashData`` under
         :cpp:member:`targetSkin` -- by the same owner as :cpp:member:`hiddenComponents`, the rest of
         its body copied as the mod wrote it. A mesh the two characters SHARE (the same hash on both
         sides, Neuvillette's eyebrows) is left to the mod's own section :raw-html:`<br />` :raw-html:`<br />`

         **Default**: empty
         @endrst
         */
        std::vector<std::string> sideMeshes;

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
         (skin-coloured or not)
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
         body. The diffuse gate does not save it on its own -- on his own assets a bright-and-neutral
         gate takes the whole hair on his head lightmap, and still a few percent of his BODY's, pixels
         that mean something else there.

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

         ON by default here, where :cpp:member:`GIMIMergeFixerConfig::compressTextures` is off.
         Turning it off for a character is safe.

         Turn it OFF for any config with a band table. The thing being edited IS the alpha and the
         alpha IS a band selector, so the encoder is free to move a value that has to match exactly
         -- eg. BC7 can move an UNEDITED band from 255 to 254, and a band legend reads that as a
         different material
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
         Whether :cpp:enumerator:`SourceLayout::Detect` asks the section's OWN fix call before it asks whether
         ``ps-t2`` is bound :raw-html:`<br />` :raw-html:`<br />`

         ``NNFix`` reads the diffuse out of ``ps-t0`` and the light map out of ``ps-t1`` and nothing else, so a
         section that renders through its own ``NNFix`` (and no ``ORFix``) is the PLAIN layout on its own
         character, whatever else it binds. Eg. a mod binding a third texture it calls a normal map at ``ps-t2``
         beside ``run = CommandList\global\ORFix\NNFix`` reads, by ``ps-t2`` alone, as the normal-map layout:
         its light map becomes the skin's DIFFUSE and the whole outfit comes out vivid green. With this on, such
         a section is plain, and on a normal-map slot its extra ``ps-t2`` is DROPPED before the shift puts the
         light map there -- kept, the section would bind ``ps-t2`` twice and whichever line its author wrote
         first would win, which may be the fake normal map :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``
         @endrst
         */
        bool layoutFromOwnFixCall = false;

        /**
         * @brief
         @rst
         Whether the face's two-way ``ps-t0`` <-> ``ps-t1`` swap runs ONLY when the mod binds its
         face diffuse at ``ps-t0`` :raw-html:`<br />` :raw-html:`<br />`

         GI 6.x swapped which register the face shader reads the diffuse and the light map from. A
         mod authored before that binds its face diffuse at ``ps-t0`` and needs the swap; one built
         from a 6.x dump -- every identity mod -- already binds ``ps-t1``, and the swap moves it the
         WRONG way. ``false`` (the default) swaps unconditionally
         @endrst
         */
        bool faceSwapOnlyFromDiffuseReg = false;

        /**
         * @brief
         @rst
         Whether a remapped slot section gets the template's ``drawindexed = auto`` only when the mod's own
         section draws on NO path -- see :cpp:member:`RegFillMissing::onlyWhenAbsent` :raw-html:`<br />` :raw-html:`<br />`

         The fill is a bottom cover, added unconditionally whenever some path lacks a draw. A mod that draws
         one object as variants on an ``if`` / ``else if`` chain with no ``else`` (eg. three hairstyles on
         ``$Hair``) then draws EVERY variant at once on the skin, on top of its own remapped ranges. With this
         on, a section that draws somewhere keeps exactly its author's draws, as on the mod's own character,
         and a section that draws nowhere (an identity mod's) is still filled :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``
         @endrst
         */
        bool fillDrawOnlyWhenUndrawn = false;

        /**
         * @brief
         @rst
         Whether a remapped part's `TexFx`_ calls are moved onto the TARGET's layout variant -- ``.0`` for a slot with
         no normal map, ``.1`` for one with it at ``ps-t0`` -- when the part's SOURCE layout differs. See
         :cpp:class:`TexFxLayout`: a mod's call names its own character's layout, so eg. a Lumine mod's ``T.0`` on
         LumineHeaven's normal-map slots barely glows. A call already naming the target's layout is kept, so an
         author's deliberate choice survives wherever the layout does not change. **Default**: ``true``
         @endrst
         */
        bool texFxLayoutSwitch = true;

        /**
         * @brief
         @rst
         Whether each drawn object's texture bindings go to the register their resource NAME's
         role belongs on (``ps-t0`` diffuse / ``ps-t1`` light map, or ``ps-t0`` normal map /
         ``ps-t1`` diffuse / ``ps-t2`` light map when a normal map is among them) BEFORE any other
         texture edit reads a register -- :cpp:member:`GIMIMergeFixerConfig::texRegsByName`'s rule,
         through the same :cpp:class:`TexRegLayout` :raw-html:`<br />` :raw-html:`<br />`

         A mod may be written in the GAME's register order rather than GIMI's: eg. a Neuvillette mod
         binding its Dress ``ps-t0 = <light map>`` / ``ps-t1 = <diffuse>`` with no fix call, which is
         right on his own plain shader; read positionally, the light map takes the diffuse role on
         the skin and draws flat green. The layout (normal map or not)
         and the diffuse / light map files an object's edits read are decided by name as well. A
         binding naming no role stays where it is, so a mod already in GIMI's order is untouched.

         Names are an author's labels, so an object's are believed only when its section calls no
         fix library itself (``NNFix`` / ``ORFix`` read fixed registers, so such a section is in
         GIMI's order whatever its files are called) and every texture it binds at ``ps-t0`` ..
         ``ps-t2`` names exactly one role, no two alike. Anything else is read positionally: a mod
         may name its normal map "Diffuse", its diffuse "LightMap" and its light map "Shadow", which
         read by name would draw its whole outfit flat yellow.
         **Default**: ``false``, the positional reading
         @endrst
         */
        bool texRegsByName = false;

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
