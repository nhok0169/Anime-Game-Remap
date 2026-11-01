#ifndef AGRemapCore_GIMITemplateParser_H
#define AGRemapCore_GIMITemplateParser_H

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

#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/data/IniParseData/TemplateParser.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`TemplateParser`

     The base of the two GIMI template parsers -- :cpp:func:`makeGIMICharParser` and
     :cpp:func:`makeGIMIComponentParser` -- holding the steps both take: whether a ``.ini`` (or a sibling ``.ini`` of the
     mod's folder) draws the character's mesh or binds its position buffer, the texture bindings a texture override
     carries onto an object's entry section, the sibling textures declared as this file's own resources, and whether the
     file's own targeted sections bind anything :raw-html:`<br />` :raw-html:`<br />`

     A subclass names the hash types a hash is filed under for it (#hashKeysOf) and fills #textureBindings,
     #siblingRefs and #adoptedRefs as it reads the mod
     @endrst
     */
    class GIMITemplateParser: public TemplateParser {
        public:
            using TemplateParser::TemplateParser;

        protected:
            /**
             * @brief The hash type of a character's position buffer in ``HashData``
             */
            static inline const std::string PositionHashKey = "position_vb";

            /**
             * @brief The prefix of a texture's hash type in ``HashData`` (eg. ``tex_body_diffuse``)
             */
            static inline const std::string TexKeyPrefix = "tex_";

            /**
             * @brief
             @rst
             The hash types ``HashData`` files 'hash' under for this parser's character -- one for each mod type the
             character is drawn as (a skin of several components has one per component), each empty where that one
             does not file it
             @endrst
             *
             * @param hash The hash
             */
            virtual std::vector<std::string> hashKeysOf(const std::string& hash) = 0;

            /**
             * @brief
             @rst
             Adds to 'bound' the resources this parser binds besides #textureBindings, for #declareSiblingRefs.
             **Default**: none
             @endrst
             *
             * @param bound The resources
             */
            virtual void addBound(std::set<std::string>& bound) const;

            /**
             * @brief Whether #targetsBindSomething leaves a mod object's sections out. **Default**: none is left out
             *
             * @param modObj The mod object
             */
            virtual bool skipsBindCheck(const ModObj& modObj) const;

            /**
             * @brief The other ``.ini`` files of the mod's folder the game loads -- see :cpp:func:`TextureOverrides::siblingInis`
             */
            std::vector<std::string> siblingInis() const;

            /**
             * @brief
             @rst
             Whether 'templates' draws the character's mesh: a `section`_ matching one of its non-texture hashes that
             binds a resource at ``ib``, ``vb0`` or ``vb1``
             @endrst
             *
             * @param templates The `sections`_
             */
            bool drawsMesh(const ModBranches::Templates& templates);

            /**
             * @brief Whether a sibling ``.ini`` (#siblingInis) draws the character's mesh -- see #drawsMesh
             */
            bool siblingDrawsMesh();

            /**
             * @brief Whether 'templates' binds ``vb0`` on one of the character's position hashes, directly or through ``run =``
             *
             * @param templates The `sections`_
             */
            bool bindsPosition(const ModBranches::Templates& templates);

            /**
             * @brief Whether a sibling ``.ini`` (#siblingInis) binds the character's position buffer -- see #bindsPosition
             */
            bool siblingBindsPosition();

            /**
             * @brief
             @rst
             Declares each sibling texture a binding names as a resource of this file's own: those #textureBindings (or
             #addBound) name, and every one an adopted `section`_ needs (#adoptedRefs). A resource the file already
             declares -- a previous fix's, read before its undo -- is kept as it is
             @endrst
             */
            void declareSiblingRefs();

            /**
             * @brief
             @rst
             Puts each object's #textureBindings at the TOP of its entry `section`_, where a download is bound too: a
             GIMI ``TextureOverride`` binds for its whole draw, and a line the mod wrote itself binding the register as
             well keeps the last word
             @endrst
             */
            void bindTextureOverrides();

            /**
             * @brief
             @rst
             Whether any of the mod's own `sections`_ this parser targets -- or a command list one of them runs, when
             the file defines it -- binds something: a buffer, an index buffer, a texture register or a draw. Not a
             ``this``. A mod object #skipsBindCheck takes is left out
             @endrst
             */
            bool targetsBindSomething();

            /**
             * @brief The parse result, with the resources #declareSiblingRefs declared riding with the downloads
             */
            std::vector<GraphGroup> collectParseResult() const override;

            /**
             * @brief Each object's texture bindings, as ``(register, resource)``, put on its entry `section`_ by #bindTextureOverrides
             */
            std::map<ModObj, std::vector<std::pair<std::string, std::string>>> textureBindings_;

            /**
             * @brief Each sibling texture's resource name and file, for #declareSiblingRefs
             */
            std::map<std::string, std::string> siblingRefs_;

            /**
             * @brief Each resource an adopted `section`_ needs, with its keys, for #declareSiblingRefs
             */
            std::map<std::string, std::vector<std::pair<std::string, std::string>>> adoptedRefs_;

            /**
             * @brief The graphs #declareSiblingRefs built of the resources it declared
             */
            std::vector<std::pair<std::string, Graph*>> refGraphs_;

            /**
             * @brief Whether the file overrides one of the character's textures by hash itself
             */
            bool ownTextureOverrides_ = false;
    };
}

#endif
