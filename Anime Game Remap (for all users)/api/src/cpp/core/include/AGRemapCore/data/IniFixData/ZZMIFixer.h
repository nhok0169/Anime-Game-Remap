#ifndef AGRemapCore_ZZMIFixer_H
#define AGRemapCore_ZZMIFixer_H

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
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What one Zenless Zone Zero (ZZMI) remap of a character onto a skin does -- everything
     :cpp:func:`makeZZMIParser` and :cpp:func:`makeZZMIFixer` need that is not the same for every pair
     :raw-html:`<br />` :raw-html:`<br />`

     A ZZMI mod is GIMI-shaped (sections per hash, objects selected by ``match_first_index``), but a ZZZ
     character is several COMPONENTS, each with its own ``Position.buf`` / ``Texcoord.buf`` / ``Blend.buf``
     and its own bone palette, and so is its skin. Each source component is remapped WHOLE onto one
     target component (:cpp:member:`Component::target`): the pair is a set of one-mesh remaps, one
     fixer per target component, all built from this one config.

     Each component's mod objects are named ``<component>_<object>`` under an empty component:
     the drawn objects (:cpp:member:`Component::slots`), ``ib``, ``blend``, ``position``, ``texcoord``,
     ``vertexlimit``, and one ``tex_<role>`` per texture the component carries
     (:cpp:member:`Component::textures`)
     @endrst
     */
    struct ZZMIFixerConfig {
        /**
         * @brief One drawn object of a component -- its letter and the ``match_first_index`` each side draws it at
         */
        struct Slot {
            /**
             * @brief The object's name, eg. ``"a"``
             */
            std::string name;

            /**
             * @brief The ``match_first_index`` the SOURCE draws it at
             */
            std::string srcIndex;

            /**
             * @brief The ``match_first_index`` the TARGET draws it at
             */
            std::string targetIndex;

            /**
             * @brief The target object's index count, written over a section's ``match_index_count``
             */
            std::size_t targetIndexCount = 0;
        };

        /**
         * @brief A texture a component's sections may recolour by hash (``hash = <texture>`` / ``this = ...``)
         */
        struct Texture {
            /**
             * @brief The hash type the texture is filed under in :cpp:class:`HashData`, eg. ``"tex_a_diffuse"``
             */
            std::string hashType;

            /**
             * @brief The role, eg. ``"diffuse"``: names the mod object (``tex_<role>``) and the fallback file
             */
            std::string role;

            /**
             * @brief
             @rst
             Whether a mod that recolours NO texture of this hash gets the source's own game texture on the
             target's hash instead (a download). A mod's UVs are the source's, so the target's texture at
             them is garbage. ``false`` for a texture only carried (the weapon's)
             @endrst
             */
            bool fallback = true;
        };

        /**
         * @brief One source component and the target component it is remapped onto
         */
        struct Component {
            /**
             * @brief The source component, eg. ``"Body"`` -- its mod type is the source's name followed by this
             */
            std::string name;

            /**
             * @brief The target component, eg. ``"Dress"`` -- likewise
             */
            std::string target;

            /**
             * @brief The drawn objects
             */
            std::vector<Slot> slots;

            /**
             * @brief The source component's ``Texcoord.buf`` stride
             */
            int srcTexcoordStride = 20;

            /**
             * @brief
             @rst
             The target component's ``Texcoord.buf`` stride. A texcoord of another stride is rewritten at
             this one: ZZZ's 20-byte layout is ``COLOR, TEXCOORD0, TEXCOORD1, TEXCOORD2`` and the 24-byte
             one appends a ``TEXCOORD3``, so narrowing drops it and widening copies ``TEXCOORD0`` into it
             @endrst
             */
            int targetTexcoordStride = 20;

            /**
             * @brief
             @rst
             The textures this component's draws read that the TARGET draws under other hashes: each
             texture-hash section of the mod on one of them is carried onto the target's hash of the same
             type. Empty when both sides draw the same textures (a hair)
             @endrst
             */
            std::vector<Texture> textures;

            /**
             * @brief
             @rst
             THE BAND EDIT: source light map band -> target band. A ZZZ light map's RED channel is a
             material band, and the band picks a tint pair out of the draw's ``ps-cb3`` per OUTFIT -- the
             source's band 0 may be the target's skin tone. Every light map the component binds is
             rewritten. **Default**: empty -- no light map edit
             @endrst
             */
            std::map<int, int> bandMoves;
        };

        /**
         * @brief The source character, eg. ``"Miyabi"``
         */
        std::string source;

        /**
         * @brief The target skin, eg. ``"MiyabiBlossom"``
         */
        std::string target;

        /**
         * @brief The components remapped, one fixer each
         */
        std::vector<Component> components;

        /**
         * @brief The red levels a light map band sits at
         */
        std::array<int, 5> bandLevels = {0, 77, 128, 179, 255};

        /**
         * @brief How far off a level a light map texel may be and still count as on its band
         */
        int bandTolerance = 8;

        /**
         * @brief
         @rst
         The registers a role binds on the main shader, for a texture bound by register instead of by
         ``Resource\ZZMI\<Role>``
         @endrst
         */
        std::map<std::string, std::string> roleRegs = {
            {"diffuse", "ps-t3"}, {"normalmap", "ps-t4"}, {"lightmap", "ps-t5"}, {"materialmap", "ps-t6"}};

        /**
         * @brief
         @rst
         A mesh drawn from another component's slot, moved back to its own: a mod that pre-skins its
         ``first`` component (``ResourceX = copy so0`` in a section of it) and draws that mesh from a
         section of its ``second`` component (``vb0 = ResourceX``) has those draws, with their toggles and
         texture lines, moved into the copy of ``first``'s object section, and zeroed where they were.
         On the target the two components' palettes need not agree the way the source's do
         (MiyabiBlossom's Dress palette sits ~4 cm off its Body's), so the mesh only lands where it
         belongs in the slot that pre-skinned it. **Default**: none
         @endrst
         */
        std::vector<std::pair<std::string, std::string>> crossSlotDraws;

        /**
         * @brief The download folders of the source's game textures (the fallbacks): game, character and version
         */
        std::string downloadGameFolder = "ZZZ";

        /**
         * @brief See :cpp:member:`downloadGameFolder`
         */
        std::string downloadCharFolder;

        /**
         * @brief See :cpp:member:`downloadGameFolder`
         */
        std::string downloadVersionFolder;

        /**
         * @brief What every generated ``.ini`` file opens with -- see :cpp:member:`GIMIFixer::copyPreamble`
         */
        std::string copyPreamble;
    };


    /**
     * @brief
     @rst
     Builds the parser of a ZZMI character's mods, from its pair's config -- one
     :cpp:class:`GIMISectionClassifier` per component, filtered to that component's own hash rows, and
     the drawn objects resolved from :cpp:member:`ZZMIFixerConfig::Component::slots`
     @endrst
     *
     * @param config The pair's config
     */
    IniParseBuilder::Factory makeZZMIParser(ZZMIFixerConfig config);

    /**
     * @brief
     @rst
     Builds the fixer that remaps one source component of a ZZMI mod onto its target component --
     see :cpp:class:`ZZMIFixerConfig` :raw-html:`<br />` :raw-html:`<br />`

     Called once per component, each result a row of :cpp:class:`IniFixBuilderData` keyed by the
     target component's own :cpp:enum:`ModTypeId`
     @endrst
     *
     * @param config The pair's config -- the same one for every component
     * @param component Which of ``config.components`` this fixer is for, by its source name
     */
    IniFixBuilder::Factory makeZZMIFixer(ZZMIFixerConfig config, std::string component);
}

#endif
