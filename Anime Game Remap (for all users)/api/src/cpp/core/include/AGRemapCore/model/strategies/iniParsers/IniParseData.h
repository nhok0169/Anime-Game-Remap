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


#ifndef AGRemapCore_IniParseData_H
#define AGRemapCore_IniParseData_H

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollectRow.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoles.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     What a parser learned about a ``.ini`` file that a fixer needs and cannot read off the sections
     it is handed -- the classification data a parser keeps, whatever the game
     :raw-html:`<br />` :raw-html:`<br />`

     Every :cpp:class:`BaseIniParser` owns one (:cpp:func:`BaseIniParser::parseData`): the parser
     fills it while it parses, and a fixer reads it through the parser it is given. A parser that
     fills nothing leaves the defaults, which say "everything is there": the file has something to
     remap, no mod object is undrawn, and no texture has a known role :raw-html:`<br />`
     :raw-html:`<br />`

     A mod object is ``(component, object)``, as the parser's graphs are keyed -- eg. ``("Body", "C")``
     for a slot of a GI component, ``("", "component3")`` for a Wuthering Waves draw slot
     :raw-html:`<br />` :raw-html:`<br />`

     What the mod's textures ARE comes in three layers, each read off the one before: which roles each
     file plays (#textureRoles), the candidates left for each role once the files that cannot serve it
     are refused (#roleCandidates, #rejections), and the one file each mod object takes for a role
     (#roleFile). The expensive ones may be filled on the first ask -- see #defer
     @endrst
     */
    class IniParseData {
        public:
            /**
             * @brief A mod object: ``(component, object)``
             */
            using ModObj = std::pair<std::string, std::string>;

            /**
             * @brief Fills the deferred part of the data -- see #defer
             */
            using Filler = std::function<void(IniParseData& data)>;

            /**
             * @brief The file a mod object takes for a role -- see #roleFile
             */
            struct RoleFile {
                /**
                 * @brief The file
                 */
                std::string file;

                /**
                 * @brief How it was found, for the log
                 */
                std::string how;

                /**
                 * @brief
                 @rst
                 Another file ranked EQUALLY good for the role, or empty: only a measurement can say
                 which is right, so a fixer should say so
                 @endrst
                 */
                std::string tiedFile;

                /**
                 * @brief How #tiedFile was found
                 */
                std::string tiedHow;
            };

            /**
             * @brief
             @rst
             A toggle that chooses among several of the mod's texture resources -- see #thisToggleOf and
             #registerToggleOf
             @endrst
             */
            struct TextureToggle {
                /**
                 * @brief The `section`_ that chooses (for a register toggle, ``<section> <register>``)
                 */
                std::string section;

                /**
                 * @brief Every resource it chooses among, in the order the `section`_ names them
                 */
                std::vector<std::string> variants;

                /**
                 * @brief
                 @rst
                 Whether it binds ONE resource under a real condition -- off, the original draws with the
                 game's own texture
                 @endrst
                 */
                bool single = false;

                /**
                 * @brief
                 @rst
                 The parser's graph holding the `section`_, as ``(component, mod object)`` -- see
                 :cpp:member:`IniGraphModObjKeywords::Texture`. Empty for a register toggle
                 @endrst
                 */
                std::pair<std::string, std::string> graph;
            };

            /**
             * @brief A file refused for a role -- see #rejections
             */
            struct Rejection {
                /**
                 * @brief The file
                 */
                std::string file;

                /**
                 * @brief The role it was refused FOR -- it may still serve another
                 */
                std::string role;

                /**
                 * @brief Why, as a sentence about the file, eg. ``is a flat hairMask, which marks no regions``
                 */
                std::string why;
            };

            /**
             * @brief Resets everything to the defaults
             */
            void clear();

            /**
             * @brief
             @rst
             Whether the ``.ini`` file carries anything to remap. ``false`` for a file that only
             WATCHES the character -- eg. a help overlay whose one section matches the position hash
             to know she is on screen, which remapped like a mod would draw a second copy of her
             :raw-html:`<br />` :raw-html:`<br />`
             **Default**: ``true``
             @endrst
             */
            bool hasRemappableContent() const;

            /**
             * @brief Sets #hasRemappableContent
             *
             * @param remappable Whether the file carries anything to remap
             */
            void setRemappableContent(bool remappable);

            /**
             * @brief
             @rst
             Whether the ``.ini`` file is a RECOLOUR and nothing else: it draws none of the
             character's mesh, no other ``.ini`` file beside it does, and it overrides a texture of the
             character that the target does not draw. Its whole model then comes from downloads, with
             the mod's textures bound in place of the downloaded ones -- so a fixer that reads the
             mod's buffers while it is being built has to fetch those downloads first
             :raw-html:`<br />` :raw-html:`<br />`
             **Default**: ``false``
             @endrst
             */
            bool isRecolourOnly() const;

            /**
             * @brief Sets #isRecolourOnly
             *
             * @param recolourOnly Whether the file is a recolour and nothing else
             */
            void setRecolourOnly(bool recolourOnly);

            /**
             * @brief
             @rst
             `Sections`_ a fix writes into every ``.ini`` file it generates, so a SIBLING's toggle keeps
             working there: a ``[Constants]`` declaring a copy of each variable a carried recolour's
             condition reads, and a copy of each sibling ``[Key...]`` section that cycles one, both
             under the copy's name :raw-html:`<br />` :raw-html:`<br />`

             A file with no ``namespace =`` is named after its PATH, so a variable of one ``.ini``
             cannot be read from another. The copy is driven by the same key under the same
             condition, so the two keep in step :raw-html:`<br />` :raw-html:`<br />`
             **Default**: empty -- nothing to carry
             @endrst
             */
            const std::string& carriedSections() const;

            /**
             * @brief Sets #carriedSections
             *
             * @param sections The `sections`_, as ``.ini`` text
             */
            void setCarriedSections(std::string sections);

            /**
             * @brief
             @rst
             Whether the mod leaves a mod object UNDRAWN -- on its own character the game never draws
             it, so a fix must not draw a download of it over the mod's own buffers
             @endrst
             *
             * @param modObj The mod object
             */
            bool isUndrawn(const ModObj& modObj) const;

            /**
             * @brief Records that the mod leaves 'modObj' undrawn
             *
             * @param modObj The mod object
             */
            void addUndrawn(const ModObj& modObj);

            /**
             * @brief Forgets every mod object #addUndrawn recorded
             */
            void clearUndrawn();

            /**
             * @brief Every mod object the mod leaves undrawn
             */
            const std::set<ModObj>& undrawnObjs() const;

            /**
             * @brief A draw's ``(index count, first index)``, as ``drawindexed = <count>, <first>, ...`` writes it
             */
            using DrawRange = std::pair<long long, long long>;

            /**
             * @brief
             @rst
             The draws the mod's own `sections`_ of a mod object make with numbers, in the order they
             are written -- not ``drawindexed = auto``, which draws the whole of whatever is bound
             @endrst
             *
             * @param modObj The mod object
             */
            const std::vector<DrawRange>& drawRanges(const ModObj& modObj) const;

            /**
             * @brief Records one of #drawRanges
             *
             * @param modObj The mod object
             * @param range The draw
             */
            void addDrawRange(const ModObj& modObj, DrawRange range);

            /**
             * @brief
             @rst
             Which parts of the mod object's `sections`_ hold a ``drawindexed``, counted from 1 over
             every part of a `section`_ (an ``if`` line is a part too). Draws all in one part are draws
             under one condition
             @endrst
             *
             * @param modObj The mod object
             */
            const std::set<std::size_t>& drawParts(const ModObj& modObj) const;

            /**
             * @brief Records one of #drawParts
             *
             * @param modObj The mod object
             * @param part The part
             */
            void addDrawPart(const ModObj& modObj, std::size_t part);

            /**
             * @brief
             @rst
             The external libraries a mod object's `sections`_ name on any of their lines (eg.
             ``RabbitFX`` -- see :cpp:member:`RabbitFX::Name`), by the name the parser gives each
             @endrst
             *
             * @param modObj The mod object
             */
            const std::set<std::string>& libraries(const ModObj& modObj) const;

            /**
             * @brief Records one of #libraries
             *
             * @param modObj The mod object
             * @param library The library's name
             */
            void addLibrary(const ModObj& modObj, std::string library);

            /**
             * @brief
             @rst
             The texture registers a mod object's `sections`_ bind only AFTER they have drawn, in the
             order first bound. A draw before them renders with whatever the game had bound, so a mod
             whose section draws, then binds, then draws again has two binding generations
             @endrst
             *
             * @param modObj The mod object
             */
            const std::vector<std::string>& lateBindings(const ModObj& modObj) const;

            /**
             * @brief Sets #lateBindings
             *
             * @param modObj The mod object
             * @param regs The registers
             */
            void setLateBindings(const ModObj& modObj, std::vector<std::string> regs);

            /**
             * @brief
             @rst
             Whether the mod replaces a game texture BY HASH anywhere -- a texture override with a
             ``hash`` and a ``this =`` -- so its art reaches every draw that binds that texture,
             whatever the draw's own `section`_ binds
             @endrst
             */
            bool replacesTexturesByHash() const;

            /**
             * @brief Sets #replacesTexturesByHash
             *
             * @param replaces Whether it does
             */
            void setReplacesTexturesByHash(bool replaces);

            /**
             * @brief
             @rst
             The ``filename =`` of one of the mod's resource `sections`_, found without regard to the
             case of its name, or ``std::nullopt``. The value is the ``.ini``'s own (a Windows path,
             relative to the ``.ini``)
             @endrst
             *
             * @param section The resource `section`_
             */
            std::optional<std::string> resourceFile(const std::string& section) const;

            /**
             * @brief Every resource `section`_ of the mod that names a file, as ``lowercased name -> filename``
             */
            const std::map<std::string, std::string>& resourceFiles() const;

            /**
             * @brief Records one of #resourceFiles
             *
             * @param section The resource `section`_
             * @param file Its ``filename =``
             */
            void setResourceFile(const std::string& section, std::string file);

            /**
             * @brief
             @rst
             What role each texture of the ``.ini`` plays, or ``nullptr`` when the parser records
             none -- every role the ``.ini`` binds a file for, since one atlas can serve two objects
             @endrst
             */
            const TextureRoles* textureRoles() const;

            /**
             * @brief Sets the texture roles outright
             *
             * @param roles The roles
             */
            void setTextureRoles(TextureRoles roles);

            /**
             * @brief
             @rst
             Fills the texture data -- #textureRoles, #roleCandidates, #roleFile, #rejections -- on the
             first ask of any of them, rather than now :raw-html:`<br />` :raw-html:`<br />`

             Sorting a mod's textures may open every one of them, and a parser is built for every
             ``.ini`` a run classifies, most of which no fixer asks about
             @endrst
             *
             * @param fill What fills them, or empty for nothing
             */
            void defer(Filler fill);

            /**
             * @brief
             @rst
             The files left for a role once the ones that cannot serve it are refused, as
             ``(file, how it was found)`` in the order they were found; empty when nothing serves it
             @endrst
             *
             * @param role The role
             */
            const std::vector<std::pair<std::string, std::string>>& roleCandidates(const std::string& role) const;

            /**
             * @brief Sets #roleCandidates for a role
             *
             * @param role The role
             * @param candidates ``(file, how)`` for each
             */
            void setRoleCandidates(const std::string& role, std::vector<std::pair<std::string, std::string>> candidates);

            /**
             * @brief
             @rst
             The file a mod object takes for a role, or ``nullptr`` when no file of the mod serves the
             role -- the choice among #roleCandidates is per mod object, since a file a mod binds on
             one object is the better answer for that object
             @endrst
             *
             * @param role The role
             * @param modObj The mod object
             */
            const RoleFile* roleFile(const std::string& role, const ModObj& modObj) const;

            /**
             * @brief Sets #roleFile
             *
             * @param role The role
             * @param modObj The mod object
             * @param file The file it takes
             */
            void setRoleFile(const std::string& role, const ModObj& modObj, RoleFile file);

            /**
             * @brief
             @rst
             The ``this =`` toggle that chooses a resource, or ``nullptr`` -- a texture override
             ``hash = <a texture> / if ... this = A / else this = B`` the mod writes to swap one texture
             for another
             @endrst
             *
             * @param resource The resource, lowercased
             */
            const TextureToggle* thisToggleOf(const std::string& resource) const;

            /**
             * @brief Records the ``this =`` toggle that chooses 'resource'
             *
             * @param resource The resource, lowercased
             * @param toggle The toggle
             */
            void setThisToggle(const std::string& resource, TextureToggle toggle);

            /**
             * @brief
             @rst
             The register toggle that chooses a resource, or ``nullptr`` -- one `section`_ binding one
             register to several resources on its branches (``if $x == 1 / ps-t3 = A / else / ps-t3 = B``)
             @endrst
             *
             * @param resource The resource, lowercased
             */
            const TextureToggle* registerToggleOf(const std::string& resource) const;

            /**
             * @brief Records the register toggle that chooses 'resource'
             *
             * @param resource The resource, lowercased
             * @param toggle The toggle
             */
            void setRegisterToggle(const std::string& resource, TextureToggle toggle);

            /**
             * @brief Every file refused for a role, in the order they were refused
             */
            const std::vector<Rejection>& rejections() const;

            /**
             * @brief
             @rst
             The game version whose skeleton numbering the mod's blend gives its bone ids in, when the
             parser could tell from the mod's geometry -- a mod's hashes may name one version while its
             bone ids are another's. ``std::nullopt`` when the parser did not ask or could not tell
             @endrst
             */
            const std::optional<std::string>& blendVersion() const;

            /**
             * @brief Sets #blendVersion
             *
             * @param version The version
             */
            void setBlendVersion(std::string version);

            /**
             * @brief One role the parser chose a file for, on one mod object -- see #rolesOfResource
             */
            struct ResourceRole {
                /**
                 * @brief The role
                 */
                std::string role;

                /**
                 * @brief The mod object it was chosen for
                 */
                ModObj modObj;
            };

            /**
             * @brief Records that the ``.ini``'s resource `section`_ 'resource' names the texture 'file'
             *
             * @param resource The resource `section`_
             * @param file The file, as #roleFile and #rejections name it
             */
            void setTextureResource(const std::string& resource, std::string file);

            /**
             * @brief
             @rst
             What a texture line binding 'resource' binds: every ``(role, mod object)`` the parser chose
             the resource's file for (#roleFile), by role and then by mod object. Empty for a resource
             naming no texture, or one chosen for nothing
             @endrst
             *
             * @param resource The resource `section`_, in any case
             */
            std::vector<ResourceRole> rolesOfResource(const std::string& resource) const;

            /**
             * @brief The role the parser REFUSED the resource's file for (the first of #rejections), or ``std::nullopt``
             *
             * @param resource The resource `section`_, in any case
             */
            std::optional<std::string> refusedRoleOf(const std::string& resource) const;

            /**
             * @brief Records that 'file' was refused for 'role'
             *
             * @param file The file
             * @param role The role
             * @param why Why, as a sentence about the file
             */
            void reject(const std::string& file, const std::string& role, const std::string& why);

            /**
             * @brief One row a :cpp:class:`GraphCollect` collected
             */
            using CollectedRow = GraphCollectRow<>;

            /**
             * @brief
             @rst
             The rows of the :cpp:class:`GraphCollect` named 'name' among the parser's
             :cpp:member:`GIMIParser::parseSteps`, in the order it collected them. Empty for a name nothing filed
             @endrst
             *
             * @param name The collect's :cpp:member:`GraphCollect::name`
             */
            const std::vector<CollectedRow>& collected(const std::string& name) const;

            /**
             * @brief Files 'rows' under 'name', replacing what was filed there -- see #collected
             *
             * @param name The name
             * @param rows The rows
             */
            void setCollected(const std::string& name, std::vector<CollectedRow> rows);

        private:
            void fill() const;

            bool remappable_ = true;
            bool recolourOnly_ = false;
            std::string carriedSections_;
            std::set<ModObj> undrawn_;
            std::map<ModObj, std::vector<DrawRange>> drawRanges_;
            std::map<ModObj, std::set<std::size_t>> drawParts_;
            std::map<ModObj, std::set<std::string>> libraries_;
            std::map<ModObj, std::vector<std::string>> lateBindings_;
            bool replacesByHash_ = false;
            std::map<std::string, std::string> resourceFiles_;
            std::map<std::string, std::string> textureResources_;
            std::optional<std::string> blendVersion_;
            std::unique_ptr<TextureRoles> roles_;
            std::map<std::string, std::vector<std::pair<std::string, std::string>>> candidates_;
            std::map<std::pair<std::string, ModObj>, RoleFile> roleFiles_;
            std::vector<Rejection> rejections_;
            std::map<std::string, TextureToggle> thisToggles_;
            std::map<std::string, TextureToggle> registerToggles_;
            std::map<std::string, std::vector<CollectedRow>> collected_;
            mutable Filler fill_;
    };
}

#endif
