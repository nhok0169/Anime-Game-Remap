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

#ifndef AGRemapCore_TextureOverrides_H
#define AGRemapCore_TextureOverrides_H

#include <string>
#include <vector>

#include "AGRemapCore/data/IniFixData/ModBranches.h"

namespace AGRemapCore {
    class IniFile;

    /**
     * @brief
     @rst
     Reads a mod's texture overrides: `sections`_ of the shape ``hash = <a game texture>`` /
     ``this = Resource...``, which replace a texture wherever the GAME binds it :raw-html:`<br />`
     :raw-html:`<br />`

     Such an override never reaches a texture the fix binds itself (a download, or any other
     resource of the mod's), so a parser that fills a slot from downloads has to bind the mod's
     resource there in its place. The parsers built by :cpp:func:`makeGIMICharParser` and
     :cpp:func:`makeGIMIComponentParser` both read overrides through this class
     @endrst
     */
    class TextureOverrides {
        public:
            /**
             * @brief One texture override
             */
            struct Override {
                /**
                 * @brief The name of the `section`_ that holds the override
                 */
                std::string section;

                /**
                 * @brief The texture hash the override matches, stripped and lowercased
                 */
                std::string hash;

                /**
                 * @brief The resource the override binds in its place, stripped
                 */
                std::string resource;
            };

            /**
             * @brief Every `section`_ of 'templates' that has both a ``hash`` and a ``this``
             *
             * @param templates The parsed `sections`_ of a ``.ini`` file
             *
             * @return The overrides, in the order of the `sections`_
             */
            static std::vector<Override> collect(const ModBranches::Templates& templates);

            /**
             * @brief
             @rst
             The other ``.ini`` files of the folder 'iniFile' is in that the game loads: not
             'iniFile' itself, not a ``DISABLED`` one, and not a copy a fix wrote
             (``<name>RemapFix<N>.ini``)
             @endrst
             *
             * @param iniFile The ``.ini`` file
             *
             * @return The full paths of the other files, sorted
             */
            static std::vector<std::string> siblingInis(IniFile* iniFile);

            /**
             * @brief
             @rst
             Whether ``HashData`` files 'hash' under more than one character :raw-html:`<br />`
             :raw-html:`<br />`

             The components of a skin of several components count as that skin. A texture two
             characters share is drawn by both, so an override of it already applies to either of
             them without any fix -- eg. Amber and AmberCN draw one face diffuse, ``1d064079``
             @endrst
             *
             * @param hash The texture hash
             *
             * @return Whether more than one character draws the texture
             */
            static bool isShared(const std::string& hash);

            /**
             * @brief
             @rst
             Every texture key (``tex_<object>_<role>``) ``HashData`` files 'hash' under for the
             character 'name', at any version :raw-html:`<br />` :raw-html:`<br />`

             More than one when the character draws several objects with one texture -- Yelan's body
             and dress share ``df127976`` -- and an override of that texture recolours all of them
             @endrst
             *
             * @param name The character's mod type name, eg. ``Yelan``
             * @param hash The texture hash
             *
             * @return The keys, each once, in the table's order
             */
            static std::vector<std::string> keysOf(const std::string& name, const std::string& hash);
    };

    /**
     * @brief
     @rst
     What a parser from :cpp:func:`makeGIMICharParser` learned about a ``.ini`` file's texture
     overrides that a fixer needs :raw-html:`<br />` :raw-html:`<br />`

     The fixer reaches it by ``dynamic_cast`` from the parser it is handed. Absent (another parser),
     the ``.ini`` file is not a recolour
     @endrst
     */
    class TextureOverrideFacts {
        public:
            virtual ~TextureOverrideFacts() = default;

            /**
             * @brief
             @rst
             Whether the ``.ini`` file is a RECOLOUR and nothing else: it draws none of the
             character's mesh, no other ``.ini`` file beside it does, and it overrides a texture of
             the character that the target does not draw :raw-html:`<br />` :raw-html:`<br />`

             Its whole model then comes from downloads, with the mod's textures bound in place of the
             downloaded ones -- so a fixer that reads the mod's buffers while it is being built has
             to fetch those downloads first
             @endrst
             */
            virtual bool isRecolourOnly() const = 0;
    };
}

#endif
