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
#include "AGRemapCore/tools/z3/Z3Context.h"


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

                /**
                 * @brief
                 @rst
                 The files of the toggle #file is a variant of, in the order the toggle names them (#file among
                 them), or empty when it is no toggle's. Each plays the same role on the same object
                 @endrst
                 */
                std::vector<std::string> variants;
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
             The texture resources a mod object's own `sections`_ bind ONLY under a toggle, lowercased: every line binding
             one sits in a branch that no draw of the object sits in, at a register no line reaching a draw binds. The
             game's own texture is what those draws render with whenever the toggle is off
             @endrst
             *
             * @param modObj The mod object
             */
            const std::set<std::string>& toggledBindings(const ModObj& modObj) const;

            /**
             * @brief Sets #toggledBindings
             *
             * @param modObj The mod object
             * @param resources The resources, lowercased
             */
            void setToggledBindings(const ModObj& modObj, std::set<std::string> resources);

            /**
             * @brief
             @rst
             The resource a ``TextureOverride`` `section`_ with ``hash`` 'hash' puts in the texture's place with
             ``this =``, as written -- the first such section's -- or ``std::nullopt``
             @endrst
             *
             * @param hash The texture's hash, in any case
             */
            std::optional<std::string> thisOverrideOf(const std::string& hash) const;

            /**
             * @brief Sets #thisOverrideOf, unless a resource is already recorded for 'hash'
             *
             * @param hash The texture's hash, in any case
             * @param resource The resource
             */
            void setThisOverride(const std::string& hash, std::string resource);

            /**
             * @brief Whether any `section`_ binds 'resource' with ``this =``
             *
             * @param resource The resource, in any case
             */
            bool isThisBound(const std::string& resource) const;

            /**
             * @brief Records that a `section`_ binds 'resource' with ``this =`` -- see #isThisBound
             *
             * @param resource The resource, in any case
             */
            void addThisBound(const std::string& resource);

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
             The resource `section`_ that names 'file' as the mod USES it, or ``std::nullopt`` -- one file may be
             declared by several resources, and the one a section of the mod binds is the one that reaches the
             draws. Recorded by the parser with #setFileResource
             @endrst
             *
             * @param file The file, as #roleFile names it
             */
            std::optional<std::string> resourceOfFile(const std::string& file) const;

            /**
             * @brief Records that the mod uses the resource `section`_ 'resource' for 'file' -- see #resourceOfFile
             *
             * @param file The file
             * @param resource The resource `section`_
             */
            void setFileResource(const std::string& file, std::string resource);

            /**
             * @brief
             @rst
             What a texture line binding 'resource' binds: every ``(role, mod object)`` the parser chose
             the resource's file for (#roleFile) -- or a file of the same toggle (:cpp:member:`RoleFile::variants`) --
             by role and then by mod object. Empty for a resource
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

             .. warning::
                The rows describe the graphs as they were when the collect ran (see :cpp:class:`GraphCollect`).
                A collect among :cpp:member:`GIMIParser::deferredParseSteps` runs on the FIRST ask of this data, so
                that first ask has to come before any edit to the graphs it reads: a fixer asks in its constructor,
                before its edits run. Asked first after an edit, the rows are read from the edited graphs
             @endrst
             *
             * @param name The collect's :cpp:member:`GraphCollect::name`
             */
            const std::vector<CollectedRow>& collected(const std::string& name) const;

            /**
             * @brief
             @rst
             The first row of the :cpp:class:`GraphCollect` named 'name' that holds 'key' in 'section' itself -- not in
             a section it ``run =``\s -- in the order the collect found them. 'section' is matched regardless of case,
             as 3DMigoto matches a section's name; 'key' exactly. ``nullptr`` when there is none
             @endrst
             *
             * @param name The collect's :cpp:member:`GraphCollect::name`
             * @param section The `section`_
             * @param key The key
             */
            const CollectedRow* firstCollected(const std::string& name, const std::string& section, const std::string& key) const;

            /**
             * @brief
             @rst
             The `sections`_ the :cpp:class:`GraphCollect` named 'name' has rows in, in the order they are written in the
             ``.ini`` file
             @endrst
             *
             * @param name The collect's :cpp:member:`GraphCollect::name`
             */
            const std::vector<std::string>& collectedSections(const std::string& name) const;

            /**
             * @brief
             @rst
             The rows of the :cpp:class:`GraphCollect` named 'name' in 'section', in the order they are written: each line
             once however many graphs reached it, and a key's rows among the section's others rather than grouped by key
             as a collect files them. Empty for a section with none
             @endrst
             *
             * @param name The collect's :cpp:member:`GraphCollect::name`
             * @param section The `section`_, as the file names it
             */
            const std::vector<const CollectedRow*>& sectionRows(const std::string& name, const std::string& section) const;

            /**
             * @brief Files 'rows' under 'name', replacing what was filed there -- see #collected
             *
             * @param name The name
             * @param rows The rows
             */
            void setCollected(const std::string& name, std::vector<CollectedRow> rows);

            /**
             * @brief What #resourceData reads, and the rules a game reads it by
             */
            struct ResourceDataRules {
                /**
                 * @brief The collect whose rows it reads: every key each object's `sections`_ reach -- see :cpp:member:`IniParseRows::Objs`
                 */
                std::string rows;

                /**
                 * @brief The collect whose rows name each bound resource's ``filename`` -- see :cpp:member:`IniParseRows::Resources` -- or empty for none
                 */
                std::string resources;

                /**
                 * @brief The objects to read, each with its root `sections`_ in the order they are written
                 */
                std::vector<std::pair<ModObj, std::vector<std::string>>> roots;

                /**
                 * @brief The register family a mod binds its textures with. **Default**: ``"ps-t"``
                 */
                std::string texRegPrefix = "ps-t";

                /**
                 * @brief
                 @rst
                 Whether a ``run =`` of a `section`_ counts as a draw -- eg. one of a library this file does not hold.
                 **Default**: none does
                 @endrst
                 */
                std::function<bool(const std::string& called)> runDraws;

                /**
                 * @brief The libraries to record (#libraries), each by whether a key or value names it
                 */
                std::vector<std::pair<std::string, std::function<bool(const std::string& text)>>> libraries;

                /**
                 * @brief
                 @rst
                 When EVERY object read looks hidden (draws nothing and skips the game's draw), record none of them as
                 #isUndrawn: a mod the reading does not understand. **Default**: ``false``
                 @endrst
                 */
                bool noneUndrawnWhenAll = false;

                /**
                 * @brief
                 @rst
                 The collect whose rows hold every key of every `section`_ -- see :cpp:member:`IniParseRows::Sections` --
                 for #thisOverrideOf and #isThisBound, or empty for none
                 @endrst
                 */
                std::string sections;

                /**
                 * @brief
                 @rst
                 The collect whose rows hold each texture override's ``this =`` and texture register lines, with the
                 conditions they sit under, for #thisToggleOf and #registerToggleOf -- or empty for none
                 @endrst
                 */
                std::string toggles;

                /**
                 * @brief
                 @rst
                 The texture objects whose `sections`_ may choose among the mod's textures, each with its root
                 `sections`_: a ``this =`` toggle is read in these, and a register toggle in these and in #roots
                 @endrst
                 */
                std::vector<std::pair<ModObj, std::vector<std::string>>> textureRoots;

                /**
                 * @brief
                 @rst
                 Which `sections`_ may hold a toggle. **Default**: every `section`_ named ``TextureOverride...``
                 @endrst
                 */
                std::function<bool(const std::string& section)> togglesIn;

                /**
                 * @brief
                 @rst
                 The conditions that do not make a toggle, lowercased and without spaces -- eg. a condition every
                 section of a game's mods is wrapped in. **Default**: none
                 @endrst
                 */
                std::vector<std::string> notToggles;

                /**
                 * @brief Where a draw range that cannot be read is reported, or empty
                 */
                std::function<void(const std::string& message)> log;
            };

            /**
             * @brief
             @rst
             Reads the mod's resource data off collected rows and records it: each resource's file (#resourceFiles), and for
             each object its numbered draws (#drawRanges) and the parts they sit in (#drawParts), the texture registers it
             binds only after it has drawn (#lateBindings, unless #replacesTexturesByHash), the libraries it names
             (#libraries), the texture resources it binds only under a toggle (#toggledBindings), and whether it is
             hidden -- its `sections`_ draw nothing and skip the game's draw (#isUndrawn). Given
             :cpp:member:`ResourceDataRules::sections`, also every ``this =`` override (#thisOverrideOf, #isThisBound). Only the rows in an object's own root `sections`_ count, except that a draw or a ``run =``
             anywhere it reaches counts as drawing
             :raw-html:`<br />` :raw-html:`<br />`

             Given :cpp:member:`ResourceDataRules::toggles`, also the toggles that choose among the mod's textures:

             * **a ``this =`` toggle** (#thisToggleOf): a texture override binding several resources on its branches, or
               ONE under a condition that is not in :cpp:member:`ResourceDataRules::notToggles` (off, the original
               draws with the game's texture). When two `sections`_ bind one resource, the one that declares more of its
               own variants as `sections`_ keeps it
             * **a register toggle** (#registerToggleOf): a texture override binding one texture register to several
               resources on its branches, per `section`_ and register
             @endrst
             *
             * @param rules What to read, and how
             */
            void resourceData(const ResourceDataRules& rules);

            /**
             * @brief How #roleFiles reads which file each mod object takes for each role -- see #roleFiles
             */
            struct RoleFileRules {
                /**
                 * @brief Another ``.ini`` of the mod, linked to this one, whose parse is read too -- see #linked
                 */
                struct Linked {
                    /**
                     * @brief Its parse data, read by the same collects in its parser's only-this-file mode
                     */
                    const IniParseData* data = nullptr;

                    /**
                     * @brief Its folder, which its resources' files are relative to
                     */
                    std::string folder;

                    /**
                     * @brief Its file name, for the log
                     */
                    std::string name;
                };

                /**
                 * @brief The ``.ini``'s path, which #textureRoles files its resources under
                 */
                std::string iniPath;

                /**
                 * @brief The ``.ini``'s folder, which its resources' files are relative to
                 */
                std::string folder;

                /**
                 * @brief The collect of every key of every `section`_ (:cpp:member:`IniParseRows::Sections`), which orders the `sections`_ as written
                 */
                std::string sections;

                /**
                 * @brief The collect of every `section`_'s declaration (:cpp:member:`IniParseRows::Declared`), whose ``filename`` names a resource's file
                 */
                std::string declared;

                /**
                 * @brief The collect of each texture override's ``hash``, ``this`` and ``match_priority``
                 */
                std::string overrides;

                /**
                 * @brief
                 @rst
                 The collect, with conditions, of the rows that STATE a role: a ``this`` in a role's texture graph (mod object
                 ``(IniGraphModObjKeywords::Texture, <role>)``), and a register of :cpp:member:`registerRoles` in an object's
                 graph, through ``run =``
                 @endrst
                 */
                std::string roleRows;

                /**
                 * @brief The collect of every key of each object's graph, whose OWN `sections`_ say what the object binds
                 */
                std::string objRows;

                /**
                 * @brief The objects a file is chosen for, each with its root `sections`_
                 */
                std::vector<std::pair<ModObj, std::vector<std::string>>> objs;

                /**
                 * @brief Which role each register an object binds plays, as ``object -> {register -> role}``
                 */
                std::map<ModObj, std::map<std::string, std::string>> registerRoles;

                /**
                 * @brief The role a game texture hash of the character plays, or ``std::nullopt``
                 */
                std::function<std::optional<std::string>(const std::string& hash)> roleOfHash;

                /**
                 * @brief
                 @rst
                 Which game texture a file IS, by its pixels, or ``std::nullopt`` -- asked only of a ``.dds`` no hash and no
                 register places. Empty: no file is identified that way
                 @endrst
                 */
                std::function<std::optional<std::string>(const std::string& file)> identify;

                /**
                 * @brief
                 @rst
                 How old a hash of the character is, lower being newer -- when the mod declares a role by two
                 different hashes, the newest is the one the game binds now. Empty: every hash is as new as any other
                 @endrst
                 */
                std::function<int(const std::string& hash)> hashAge;

                /**
                 * @brief The roles whose texture marks regions, refused when flat or bound as a second role of one object
                 */
                std::set<std::string> regionRoles;

                /**
                 * @brief Whether a file is flat -- see :cpp:member:`regionRoles`. Empty: its channel 0 is constant
                 */
                std::function<bool(const std::string& file)> isFlat;

                /**
                 * @brief A condition that is not a toggle, lowercased with no spaces -- see :cpp:member:`ResourceDataRules::notToggles`
                 */
                std::vector<std::string> notToggles;

                /**
                 * @brief Whether a `section`_ was written by a previous fix, so what it binds is not the mod's use
                 */
                std::function<bool(const std::string& section)> fixSection;

                /**
                 * @brief The other ``.ini`` files of the mod linked to this one -- see :cpp:func:`IniFile::linkedInis`
                 */
                std::vector<Linked> linked;

                /**
                 * @brief Where what was found is reported, or empty
                 */
                std::function<void(const std::string& message)> log;
            };

            /**
             * @brief
             @rst
             Reads which file each mod object takes for each texture role, and records it: #textureRoles, #roleCandidates,
             #rejections, #roleFile, and each texture resource's file (#rolesOfResource, #resourceOfFile)
             :raw-html:`<br />` :raw-html:`<br />`

             A file is a candidate for a role when the ``.ini`` states it: a texture override's own ``hash =`` names a
             game texture of the role, an object's `section`_ binds it at a register of the role, a linked ``.ini``
             recolours that texture or binds it on the same draw, or -- for a ``.dds`` nothing states -- its pixels are
             the game texture's (:cpp:member:`RoleFileRules::identify`). A file the folder does not hold, or one a
             previous fix wrote (``RemapDL`` / ``RemapTex``), is none. A flat file, or one an object binds for a second
             role too, is refused for a region role (:cpp:member:`RoleFileRules::regionRoles`)
             :raw-html:`<br />` :raw-html:`<br />`

             Each object then takes, in this order:

             #. a file not declared by its hash as ANOTHER role
             #. the file bound only on this object, then on it and others, then on none, then only on others
             #. a file declared by its hash, then one bound at a register, then one known by its pixels
             #. of two hashes of one role, the newest (:cpp:member:`RoleFileRules::hashAge`)
             #. of two `sections`_ of one hash, the one 3DMigoto applies last: the higher ``match_priority``, then
                the `section`_ name later in order
             #. the file whose resource is declared first -- reported as a tie (:cpp:member:`RoleFile::tiedFile`)

             The variants of one toggle are not ranked against each other: the toggle's first-named variant among the
             candidates is chosen, and all of them recorded (:cpp:member:`RoleFile::variants`). A toggle is a ``this =``
             or register toggle (#thisToggleOf, #registerToggleOf), or one register an object's graph -- through
             ``run =``, or in a linked ``.ini`` -- binds to several resources under conditions
             @endrst
             *
             * @param rules What to read, and how
             */
            void roleFiles(const RoleFileRules& rules);

            /**
             * @brief
             @rst
             The values the ``.ini``'s variables start at, as one condition: ``$x == <value>`` for every variable its
             ``[Constants]`` declares (``global``, ``local``, ``persist`` alike; one declared with no value starts at
             ``0``) -- or ``std::nullopt`` when it declares none :raw-html:`<br />` :raw-html:`<br />`

             Which of several values a toggle shows before anyone presses a key is the one whose condition can hold
             together with this (:cpp:func:`GraphCollect::valueUnder`). A variable another file declares is left free
             @endrst
             *
             * @param sections The collect of every key of every `section`_ (:cpp:member:`IniParseRows::Sections`)
             * @param z3Ctx The context the condition is built in
             *
             * @return The condition
             */
            std::optional<Z3Predicate> startingValues(const std::string& sections, Z3Context& z3Ctx) const;

            /**
             * @brief
             @rst
             The mesh buffer registers a `section`_ binds -- ``ib`` and ``vb<n>``, lowercased, each once, as written. A
             line that binds a RESOURCE to one of them (``ResourceBypassVB0 = ref vb0``) binds no register and is not one
             @endrst
             *
             * @param sections The collect of every key of every `section`_ (:cpp:member:`IniParseRows::Sections`)
             * @param section The `section`_
             *
             * @return The registers
             */
            std::vector<std::string> buffersBoundBy(const std::string& sections, const std::string& section) const;

        private:
            void fill() const;
            void fileToggles(const ResourceDataRules& rules);

            bool remappable_ = true;
            bool recolourOnly_ = false;
            std::string carriedSections_;
            std::set<ModObj> undrawn_;
            std::map<ModObj, std::vector<DrawRange>> drawRanges_;
            std::map<ModObj, std::set<std::size_t>> drawParts_;
            std::map<ModObj, std::set<std::string>> libraries_;
            std::map<ModObj, std::vector<std::string>> lateBindings_;
            std::map<ModObj, std::set<std::string>> toggledBindings_;
            std::map<std::string, std::string> thisOverrides_;
            std::set<std::string> thisBound_;
            bool replacesByHash_ = false;
            std::map<std::string, std::string> resourceFiles_;
            std::map<std::string, std::string> textureResources_;
            std::map<std::string, std::string> fileResources_;     // file -> the resource the mod uses for it
            std::optional<std::string> blendVersion_;
            std::unique_ptr<TextureRoles> roles_;
            std::map<std::string, std::vector<std::pair<std::string, std::string>>> candidates_;
            std::map<std::pair<std::string, ModObj>, RoleFile> roleFiles_;
            std::vector<Rejection> rejections_;
            std::map<std::string, TextureToggle> thisToggles_;
            std::map<std::string, TextureToggle> registerToggles_;
            std::map<std::string, std::vector<CollectedRow>> collected_;

            // A collect's rows by section, in written order -- built on the first ask, dropped when the rows change
            struct SectionIndex {
                std::vector<std::string> order;
                std::map<std::string, std::vector<const CollectedRow*>> rows;
            };
            const SectionIndex& sectionIndex(const std::string& name) const;
            mutable std::map<std::string, SectionIndex> sectionIndexes_;
            mutable Filler fill_;
    };
}

#endif
