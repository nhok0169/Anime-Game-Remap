#ifndef AGRemapCore_WWMIParser_H
#define AGRemapCore_WWMIParser_H

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
#include <utility>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/WWMITextureFacts.h"
#include "AGRemapCore/data/WWMITextureRoles.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     What :cpp:func:`makeWWMIParser` needs to read a mod of one Wuthering Waves character -- the
     parser counterpart of :cpp:struct:`WWMIFixerConfig` :raw-html:`<br />` :raw-html:`<br />`

     A WWMI mod is ONE mesh drawn in several ranges: every ``[TextureOverrideComponentN]`` matches
     the character's ``vb0`` hash and one draw slot's ``match_first_index``, and the buffers, the
     bone data and the shape keys are matched by hashes of their own. Nothing is named after the
     character, so a section is classified by hash and index alone -- the same
     :cpp:class:`GIMISectionClassifier` a GIMI parser uses, over the rows :cpp:class:`WWMIBuilder`
     files the character under. Every draw slot the library knows for the character (its
     :cpp:class:`Indices` rows typed ``component0``, ``component1``, ...) becomes the mod object
     ``("", "componentN")``, and each entry of :cpp:member:`hashOnlyObjs` becomes ``("", <obj>)``
     :raw-html:`<br />` :raw-html:`<br />`

     The version matters more than it does for a GIMI parser: a reverse lookup with no version
     resolves through the NEWEST bucket holding the value, and ``0`` -- component 0's index -- is
     every GI head's index too, filed at 6.1. That bucket holds no WuWa row, so without
     :cpp:member:`version` component 0 would classify as nothing
     @endrst
     */
    struct WWMIParserConfig {
        /**
         * @brief
         @rst
         The game version the library files the character's rows under, used when the ``.ini``
         carries no ``fromVersion`` of its own. **Default**: ``"2.5"``
         @endrst
         */
        std::string version = "2.5";

        /**
         * @brief The hash type every draw slot section matches. **Default**: ``"vb0"``
         */
        std::string slotHashType = "vb0";

        /**
         * @brief The ``type`` the library files a draw slot's index rows under, followed by the slot number. **Default**: ``"component"``
         */
        std::string slotPrefix = "component";

        /**
         * @brief
         @rst
         The sections identified by a hash alone, as ``(hash type in HashData, mod object name)``
         :raw-html:`<br />` :raw-html:`<br />`
         **Default**: the bone-data override (``cb4`` -> ``boneData``) and the two shape-key
         overrides (``shapekey_offsets`` -> ``shapekeyOffsets``, ``shapekey_scale`` ->
         ``shapekeyScale``). The shape-key ones are classified so the fixer can decide what to do
         with them -- hide them, as the shipped fix does, or retarget them -- rather than because
         the fix copies them
         @endrst
         */
        std::vector<std::pair<std::string, std::string>> hashOnlyObjs = {
            {"cb4", "boneData"}, {"shapekey_offsets", "shapekeyOffsets"}, {"shapekey_scale", "shapekeyScale"}};

        /**
         * @brief
         @rst
         What this character's own textures look like, so the mod's texture files can be sorted into
         ROLES :raw-html:`<br />` :raw-html:`<br />`

         Left empty, the parser builds no index and :cpp:class:`WWMIParseFacts` answers ``nullptr``;
         the fixer then builds its own.

         .. note::
            The intended end state for :cpp:member:`WWMITextureFacts::roles` is that it is EMPTY and
            :cpp:class:`GIMISectionClassifier` places every texture from the library's own
            :cpp:class:`Hashes` rows, which is where a character's hashes belong. Of the WuWa
            characters only Chisa's texture hashes are filed there so far; for the others the roles
            listed here are the only record, and many of their roles have SEVERAL hashes, which
            :cpp:class:`Hashes` can only tell apart by version
         @endrst
         */
        WWMITextureFacts textures;
    };


    /**
     * @brief
     @rst
     What a parser from :cpp:func:`makeWWMIParser` learned about a ``.ini`` file that its fixer needs
     :raw-html:`<br />` :raw-html:`<br />`

     The fixer reaches it by ``dynamic_cast`` from the parser it is handed. Absent (another parser,
     or a config with no :cpp:member:`WWMIParserConfig::textures`), the fixer falls back to building
     its own index :raw-html:`<br />` :raw-html:`<br />`

     The same shape as :cpp:class:`GIMIComponentParseFacts`, for the same reason: deciding what a
     mod's files ARE is the parser's job, and the fixer only needs the answer
     @endrst
     */
    class WWMIParseFacts {
        public:
            virtual ~WWMIParseFacts() = default;

            /**
             * @brief
             @rst
             Every texture the ``.ini`` names with the role it plays, or ``nullptr`` when the parser
             was given no :cpp:member:`WWMIParserConfig::textures` to sort them by
             @endrst
             */
            virtual const WWMITextureRoles* textureRoles() const = 0;
    };


    /**
     * @brief
     @rst
     Builds the parser for a mod of one Wuthering Waves character -- see :cpp:struct:`WWMIParserConfig`
     @endrst
     * @param config The character's config
     */
    IniParseBuilder::Factory makeWWMIParser(WWMIParserConfig config);
}

#endif
