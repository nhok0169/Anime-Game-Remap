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

#ifndef AGRemapCore_GIMIMergeFixer_H
#define AGRemapCore_GIMIMergeFixer_H

#include <array>
#include <functional>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"

namespace AGRemapCore {
    /**
     * @brief
     @rst
     What :cpp:func:`makeGIMIMergeFixer` needs: a mod built for a skin of SEVERAL components, fixed
     onto a target of ONE :raw-html:`<br />` :raw-html:`<br />`

     The inverse of :cpp:struct:`GIMIComponentFixerConfig`, and the third fixer template beside it
     and :cpp:struct:`GIMICharFixerConfig`. Where that one splits a mod's single mesh across a
     skin's components, this joins a skin's components into the target's single set of buffers --
     see :cpp:class:`VGComponentMerge` for why several ``.ini`` files binding one hash cannot do it
     @endrst
     */
    struct GIMIMergeFixerConfig {
        /**
         * @brief One draw slot of one source component, and where it lands
         */
        struct Slot {
            /**
             * @brief The slot's name on the source, eg. ``A`` -- matches the parser's mod object
             */
            std::string name;

            /**
             * @brief
             @rst
             The slot's ``match_first_index`` on the SOURCE, as a literal :raw-html:`<br />`
             :raw-html:`<br />`

             The fixer is built before the parser parses, so it finds the mod's files by hash over
             :cpp:func:`IniFile::getIfTemplates` and needs this to tell one slot's section from
             another's. Carried here rather than read from :cpp:class:`Indices` for the reason
             ``IndexData.cpp``'s own note records -- the components' slot indices are deliberately
             not in that table
             @endrst
             */
            std::string index;

            /**
             * @brief
             @rst
             The TARGET object it is drawn through, eg. ``body``. Two slots naming the same one are
             MERGED into a single draw, their index buffers concatenated
             @endrst
             */
            std::string to;

            /**
             * @brief
             @rst
             ``true`` when this slot's section reads a normal map on ``ps-t0`` :raw-html:`<br />`
             :raw-html:`<br />`

             The target has no slot for one, so it is dropped and the rest shifted down
             (``ps-t1`` -> ``ps-t0``, ``ps-t2`` -> ``ps-t1``) -- the GanyuTwilight -> Ganyu shape
             @endrst
             */
            bool normalMap = false;

            /**
             * @brief
             @rst
             Where a slot with no textures of its own borrows them, as ``component;slot`` -- empty
             when it has its own :raw-html:`<br />` :raw-html:`<br />`

             YelanTranquil's ``Bang`` and ``Eye`` have none: the game draws both with her Body slot
             A's set. A mod may leave those sections with an ``ib`` and nothing else, and the
             remapped draw then binds no texture at all -- which is worse than untextured, because
             ``ORFix`` / ``NNFix`` re-slot whatever is bound whether or not the section bound it. A
             slot that ends up with nothing gets no fix call either
             @endrst
             */
            std::string borrowFrom;

            /**
             * @brief
             @rst
             The GAME model's index count for this slot, used only when the mod does not have the
             slot's ``ib`` on disk :raw-html:`<br />` :raw-html:`<br />`

             The same fallback as :cpp:member:`Component::vertexCount` and for the same reason: a
             slot the mod does not have at all gets its ``ib`` from a download, and downloads are
             fetched in ``fixResources``, AFTER the ``.ini`` file is written. A slot the mod DOES
             have is measured from its own file, so a modded mesh of a different size is unaffected
             -- one YelanTranquil edit draws 10782 indices out of a Bang the game draws 7692 from
             :raw-html:`<br />` :raw-html:`<br />`

             Only ever read for a target object SEVERAL source slots land on, which is the only
             place an index count is needed -- see :cpp:member:`Slot::to`. ``0`` means "measure it"
             @endrst
             */
            long long indexCount = 0;

            /**
             * @brief
             @rst
             Whether this slot is drawn in the TARGET's outline pass :raw-html:`<br />`
             :raw-html:`<br />`

             A skin may outline a slot with a shader of its own -- CitlaliWhisperofStars' dress
             (Body B and C) uses outline shaders nothing else of hers does, made for thin two-sided
             cloth. Merged into a target object, the slot is drawn by the TARGET's outline shader
             instead, and on Citlali the hull of the skirt's far panel covered its lining in black
             (a frame dump: the outline draw wrote 4890 pixels outside the silhouette and turned 7141
             inside it near-black). ``false`` puts the member's block under ``if vs != 037730.0``,
             the ``filter_index`` ORFix gives every outline vertex shader, so it keeps the main
             passes and loses only its outline :raw-html:`<br />` :raw-html:`<br />`

             Honoured for a MERGED member drawn by an appended block; a slot that is its object's
             representative, or whose object's draws go per branch, is logged and drawn as before
             @endrst
             */
            bool outline = true;
        };

        /**
         * @brief One component of the SOURCE
         */
        struct Component {
            /**
             * @brief The component's name, eg. ``Body`` -- matches the parser's mod objects
             */
            std::string name;

            /**
             * @brief
             @rst
             The component's own mod type name, whose hashes find its `sections`_ -- empty for the
             skin's name followed by :cpp:member:`name`, which is how :cpp:enum:`ModTypeId` names
             every component so far :raw-html:`<br />` :raw-html:`<br />`

             The same field as ``GIMIComponentParserConfig::Component::modTypeName``, for a component
             that is not named that way: NeuvilletteMelusent's main mesh is the UNNAMED component
             ``""`` (its files are ``NeuvilletteMelusentHead.ib``, ``NeuvilletteMelusentBlend.buf``)
             and is filed as ``NeuvilletteMelusentMain``. Derived, the name was the skin's own, which
             has no buffer hashes, and every buffer of the main mesh fell back to a download
             (2026-09-25). **Default**: empty
             @endrst
             */
            std::string modTypeName;

            /**
             * @brief The component's draw slots
             */
            std::vector<Slot> slots;

            /**
             * @brief
             @rst
             The GAME model's vertex count for this component, used only when the mod does not have
             the component at all :raw-html:`<br />` :raw-html:`<br />`

             A mod may simply be missing one -- an NSFW YelanTranquil edit has no ``Eye`` -- and the
             parser then hangs that component's downloads off `sections`_ it invents for it. Those
             downloads are not on disk yet when the ``.ini`` file is written: the service fetches
             them in ``fixResources``, AFTER ``IniFile::fix``. So the count that goes into ``draw``
             and ``override_vertex_count``, and the vertex offsets every later component's index
             buffers are shifted by, cannot be measured from the file here :raw-html:`<br />`
             :raw-html:`<br />`

             It does not have to be: a downloaded buffer is by definition the game's own, so its
             length is this number. A component the mod DOES have is still measured, so a modded
             mesh of a different size is unaffected :raw-html:`<br />` :raw-html:`<br />`

             ``0`` means "measure it or drop the component", which is the right behaviour for a
             component that is missing AND has no download to fall back on

             .. note::
                The same number as the parse-side ``GIMIComponentParserConfig::Component::vertexCount``,
                and for the same reason -- see that field for why it is not in :cpp:class:`VertexCounts`
             @endrst
             */
            long long vertexCount = 0;

            /**
             * @brief
             @rst
             Added to every vertex position of this component as it is merged, in model units -- ``{0, 0, 0}``
             (the default) writes the mod's own :raw-html:`<br />` :raw-html:`<br />`

             Two skins of one character may put the same part at different heights: NeuvilletteMelusent's
             Eye is Neuvillette's 168 eye vertices 1.24 cm lower, so the forward fix moves his eyes DOWN
             (``GIMIComponentFixerConfig::Component::positionOffset``), and the reverse, without this, put
             the skin's eyes 1.24 cm low in his face -- NeuvilletteMelusent1's "eyes looking down"
             (2026-09-26). The reverse of the forward offset
             @endrst
             */
            std::array<float, 3> positionOffset{0.0f, 0.0f, 0.0f};

            /**
             * @brief
             @rst
             Whether :cpp:member:`positionOffset` applies only while the mod keeps the GAME's face -- not when it
             hides it (``handling = skip`` on the source's face diffuse) and brings its own, whose eyes already
             sit where its own face has them. The forward direction's rule, see
             ``GIMIComponentFixerConfig::Component::offsetOnlyWithGameFace``. **Default**: ``false``
             @endrst
             */
            bool offsetOnlyWithGameFace = false;
        };

        /**
         * @brief
         @rst
         Every component of the source, in MERGE order. The first takes vertex offset 0, so its
         index buffers pass through untouched -- worth putting the biggest one there
         @endrst
         */
        std::vector<Component> components;

        /**
         * @brief The TARGET's drawn objects, lowercase, in draw order
         */
        std::vector<std::string> targetObjs;

        /**
         * @brief
         @rst
         The source character's download prefix -- the same string the parse row gives
         ``GIMIComponentParserConfig::downloadPrefix``, eg. ``"BennettAdventure"``
         :raw-html:`<br />` :raw-html:`<br />`

         A mod may carry none of a component at all: an NSFW body edit has no reason to touch a
         character's eyes and ships no ``Eye`` sections whatsoever. The parser registers that
         component's buffers as DOWNLOADS, and the merge has to read them -- so it needs the name
         they land under, which is :cpp:func:`DownloadTools::fixedFileName` of this prefix.

         Without it the component's paths stay EMPTY and go into the merge as they are, which reads
         as ``Unable to open file:`` with nothing after the colon and loses that whole ``.ini``
         file's buffers. Only ever used for a path the mod itself does not supply

         Empty disables the fallback, which is the old behaviour
         @endrst
         */
        std::string downloadPrefix;

        /**
         * @brief
         @rst
         The register the target binds its face diffuse to, or empty to leave the face graph alone
         :raw-html:`<br />` :raw-html:`<br />`

         GI 6.x swapped the face's diffuse and light map, so the diffuse is bound at ``ps-t1`` on the
         main pass -- but a MOD may still write the pre-6.x ``ps-t0``, and passing that through
         replaces the target's face LIGHT MAP with it. Normalising is a no-op when the mod already
         agrees
         @endrst
         */
        std::string faceReg;

        /**
         * @brief
         @rst
         Whether the mod's face section is copied onto the target ONLY when its diffuse has to move onto
         \ref faceReg -- the mod binds it at the other register :raw-html:`<br />` :raw-html:`<br />`

         For two skins of one character that draw the SAME face meshes with the SAME face diffuse hash
         (Yaoyao and YaoyaoBamboo: ``c70ae897``), the mod's own section already fires on the target, and a
         copy is a second `TextureOverride`_ on that hash -- 3DMigoto reports it as a mod conflict on every
         reload. A binding by ``this =`` names no register and never moves. The forward direction's rule
         is ``GIMIComponentFixerConfig::faceSwapOnlyFromDiffuseReg``. **Default**: ``false``, always copied
         @endrst
         */
        bool faceOnlyWhenMoved = false;

        /**
         * @brief How the TARGET's shader reads its textures
         */
        enum class TargetLayout {
            /**
             * @brief
             @rst
             ``ps-t0`` diffuse, ``ps-t1`` light map, under ``NNFix``: a source slot's normal map is
             dropped and the rest shifted down. Every target before Citlali -- Yelan, Bennett, Ganyu
             @endrst
             */
            Plain,

            /**
             * @brief
             @rst
             ``ps-t0`` normal map, ``ps-t1`` diffuse, ``ps-t2`` light map, under ``ORFix`` -- the
             layout a GI 6.x skin's slots already use, so a source slot on it passes through with no
             register moved, and one on the plain layout is shifted UP (with no normal map of its
             own, the draw reads whatever ``ps-t0`` holds) :raw-html:`<br />` :raw-html:`<br />`

             CitlaliWhisperofStars -> Citlali (2026-09-22): both sides read the normal-map layout,
             and the plain handling dropped every normal map the skin's mods carry, then issued
             ``NNFix`` on a shader that wants ``ORFix``
             @endrst
             */
            NormalMap
        };

        /**
         * @brief
         @rst
         How the TARGET's shader reads its textures -- see :cpp:enum:`TargetLayout`.
         **Default**: :cpp:enumerator:`TargetLayout::Plain`, the behaviour before this field existed
         @endrst
         */
        TargetLayout targetLayout = TargetLayout::Plain;

        /**
         * @brief
         @rst
         Whether a carried binding goes to the register its resource NAME says, rather than staying
         where the mod put it :raw-html:`<br />` :raw-html:`<br />`

         ``NNFix`` and ``ORFix`` do not re-slot a REGISTER, they read a ROLE out of a fixed one
         (``CommandListReference``: the normal map from ``ps-t0``, the diffuse from ``ps-t1``, the
         light map from ``ps-t2``), so a remapped `section`_ that keeps the mod's own bindings and
         then calls one has to put them there first. **A mod dumped straight from the game does not
         have them there**: CitlaliWhisperofStars mods bind the light map, normal map and diffuse at
         ``ps-t0/1/2``, which is what the game's own draw of that `ib`_ binds (frame dump, ``ib``
         ``f117984b``), and ``ORFix`` over that reads every role out of the wrong slot -- the shoes,
         eyes and sleeping mask of one mod, while every slot the fix had downloaded and bound itself
         was right :raw-html:`<br />` :raw-html:`<br />`

         Which texture is which comes from the resource NAME (:cpp:class:`RegValChecks`, whose
         header records why the pixels are deliberately not consulted). A binding naming no role is
         left where it is, and a mod already in the fix's layout is unchanged -- so this SUBSUMES
         \ref GIMIMergeFixerConfig::targetLayout's positional shift of a plain slot rather than
         running beside it :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``, the positional shift every compiled character was written against
         @endrst
         */
        bool texRegsByName = false;

        /**
         * @brief
         @rst
         The TARGET's texcoord stride: the merged ``Texcoord.buf`` is at least this wide (zero-padded
         at the end of each line, where a missing ``TEXCOORD1`` sits) and the copied resource section
         declares it :raw-html:`<br />` :raw-html:`<br />`

         Unset, the merged stride is the widest source component's, which is right while some
         component is as wide as the target. NeuvilletteMelusent's four components all carry 12 bytes
         and Neuvillette reads 20 (2026-09-25). **Default**: ``0``, the widest component's
         @endrst
         */
        std::size_t texcoordStride = 0;

        /**
         * @brief
         @rst
         Built per drawn object from that object's own diffuse path: the target's light map band
         table. A GIMI light map's alpha is a material band, and the legend is per skin
         @endrst
         */
        std::function<TexEditor::Filter(const std::string& diffusePath)> lightMapEdit;

        /**
         * @brief
         @rst
         A diffuse edit per TARGET object, eg. ``{"body", <alpha to 0>}`` -- applied to the diffuse every
         slot landing on that object draws with :raw-html:`<br />` :raw-html:`<br />`

         A diffuse's ALPHA is read by the target's own shader, and two skins of one character may mean
         different things by it. YaoyaoBamboo's body diffuse is alpha 255 all over and Yaoyao's is ~0: on
         her body shader 255 is a glow, and the skin's identity mod came out lit up white from the collar
         down (2026-09-27). The component template's ``GIMIComponentFixerConfig::diffuseEdits`` is the same
         idea keyed by the source object. **Default**: empty, no edit
         @endrst
         */
        std::vector<std::pair<std::string, TexEditor::Filter>> diffuseEdits;

        /**
         * @brief Whether written textures carry a mip chain -- without one they speckle at distance
         */
        bool mipmaps = true;

        /**
         * @brief
         @rst
         Whether the edited light maps are BC7-compressed :raw-html:`<br />` :raw-html:`<br />`

         Off by default here, unlike everywhere else: the thing being edited IS the alpha and the
         alpha IS a band selector, and a band table that moves several bands at once puts widely
         separated values in one 4x4 block, which the encoder splits the difference on
         @endrst
         */
        bool compressTextures = false;

        /**
         * @brief What generated ``.ini`` files open with
         */
        std::string copyPreamble;

        /**
         * @brief
         @rst
         The hash types of the SOURCE skin's side meshes (its own draws that are no mod object, filed in
         ``HashData`` under the skin's name), eg. ``{"ib_face", "ib_headupper"}`` -- a mod's section hiding
         one is written again on the target's hash of the same type. See :cpp:class:`SideMeshes`.
         **Default**: empty
         @endrst
         */
        std::vector<std::string> sideMeshes;

        /**
         * @brief
         @rst
         Whether a target object NO slot is drawn through gets a section withdrawing a pending TexFx request
         (``$\TexFx\use_default_shader = -1``), when the mod calls TexFx at all :raw-html:`<br />` :raw-html:`<br />`

         A mod's ``run = CommandList\TexFx\TN.0`` leaves a request the NEXT outline draw serves with
         ``drawindexed = auto`` -- and an object the merge draws nothing through still draws its own outline
         over the merged buffers, so TexFx would draw its whole index buffer there (the component
         template's ``unremappedSlots`` guards the same thing the other way). **Default**: ``false``, so no
         config before it writes anything new
         @endrst
         */
        bool texFxGuardUnreached = false;
    };

    /**
     * @brief
     @rst
     Builds the fixer for a mod of a skin of SEVERAL components onto a single-component target
     :raw-html:`<br />` :raw-html:`<br />`

     One fixer, not one per component: the target draws through ONE set of buffer hashes, so all of
     the source's components end up in one ``.ini`` file over one merged set of buffers
     (:cpp:class:`VGMergeGroupResource`). Several source slots landing on one target object become a
     single draw rather than a second file, which is what the merge's concatenated index buffers are
     for :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        A target of several components -- a WuWa skin onto a WuWa skin -- is NOT this function's
        shape and is deliberately left open. The pieces that would serve it are here though: the
        slots name their target object by string, and :cpp:class:`VGComponentMerge` knows nothing
        about the target at all
     @endrst
     */
    IniFixBuilder::Factory makeGIMIMergeFixer(GIMIMergeFixerConfig config);
}

#endif
