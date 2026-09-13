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
