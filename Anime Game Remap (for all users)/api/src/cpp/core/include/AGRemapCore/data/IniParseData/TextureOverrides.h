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

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"


namespace AGRemapCore {
    class IniFile;

    // BEING RETIRED (2026-11-01): it repeats the library. Its folder listing went to IniFile::loadedInis /
    // loadedSiblings; what is left is the two GI parsers' reading -- collect() is GraphCollect rows of `this` /
    // `hash` (a row's `guards` are Override::branches), isShared / keysOf are queries over the Hashes table, and
    // adopt() is sections of a sibling IniFile -- and moves onto those with the GI templates' refactor
    // (AI Agent Help/Overview, "USE THE API FIRST, EVERYWHERE"). Do not add to it.
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

                /**
                 * @brief
                 @rst
                 The ``if`` blocks the ``this =`` sits in, outermost first: per block, every branch
                 header from its opening ``if`` down to the branch holding the override (eg.
                 ``{{"if $color == 0"}}``, or ``{{"if $color == 1", "else"}}`` for an ``else``
                 branch), stripped. Empty for an unconditional override
                 @endrst
                 */
                std::vector<std::vector<std::string>> branches;
            };

            /**
             * @brief Every `section`_ of 'templates' that has both a ``hash`` and a ``this``
             *
             * @param templates The parsed `sections`_ of a ``.ini`` file
             *
             * @return The overrides, in the order of the `sections`_
             */
            static std::vector<Override> collect(const IfTemplates& templates);

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

            /**
             * @brief
             @rst
             A `section`_ of another ``.ini`` file, made ready to live in this one :raw-html:`<br />`
             :raw-html:`<br />`

             The `section`_, every `section`_ of that file it names in a value (a ``run =`` list,
             recursively), and the files of every resource they bind -- each resource renamed with
             ``RemapRef``, so a fix can declare it again in its own file and an undo removes only the
             declaration
             @endrst
             */
            struct Adoption {
                /**
                 * @brief The `sections`_ to add, in the order they were reached, resources renamed
                 */
                std::vector<std::pair<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>> sections;

                /**
                 * @brief Each renamed resource -> every key of its `section`_ (``filename``, and
                 *        ``type`` / ``format`` / ``stride`` for a buffer)
                 */
                std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> refs;
            };

            /**
             * @brief
             @rst
             'root' of 'sibling' as an :cpp:struct:`Adoption`, or ``std::nullopt`` when a `section`_
             it would bring is already named in 'own' :raw-html:`<br />` :raw-html:`<br />`

             A mod may keep its buffers and most objects in one ``.ini`` and one object's `section`_ in
             another file of the folder; the file that binds the buffers adopts that `section`_ so the
             object is remapped with the rest
             @endrst
             *
             * @param sibling The other ``.ini`` file
             * @param root The `section`_ of 'sibling' to adopt
             * @param own The `sections`_ of the file adopting it
             */
            static std::optional<Adoption> adopt(IniFile& sibling, const std::string& root, const IfTemplates& own);
    };
}

#endif
