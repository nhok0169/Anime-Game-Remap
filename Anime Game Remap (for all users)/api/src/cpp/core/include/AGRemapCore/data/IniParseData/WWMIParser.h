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

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/WWMITextureFacts.h"
#include "AGRemapCore/model/buffers/SkeletonNumberingProbe.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"


namespace AGRemapCore {
    class ModType;
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
         The config a parser factory from :cpp:func:`makeWWMIParser` was made with, or ``std::nullopt`` for any other
         factory. A shipped parser's factory is its mod type's builder row (:cpp:func:`IniParseBuilder::factoryOf`), so
         this reads a shipped parser -- or starts a variant of it (change a field, hand it to :cpp:func:`makeWWMIParser`)
         @endrst
         *
         * @param factory The factory
         *
         * @return The config
         */
        static std::optional<WWMIParserConfig> ofFactory(const IniParseBuilder::Factory& factory);

        /**
         * @brief
         @rst
         This config with the character's GAME DATA read off the versioned asset tables: her game textures'
         thumbprints (:cpp:member:`WWMITextureFacts::textureThumbprints`, :cpp:class:`TextureThumbprints`, at
         'version'), each :cpp:struct:`DrawSplit`'s :cpp:member:`DrawSplit::flags` named by :cpp:member:`DrawSplit::flag`
         (:cpp:class:`MaterialCodes`, at 'version') and, for a character whose skeleton was renumbered (her ``vb0`` rows in :cpp:class:`HashData` at two versions or more, each with a
         :cpp:class:`VGRemaps` row onto the newest), :cpp:member:`skeletonNumberings` -- one per version, oldest first,
         with the bones that row moves -- and :cpp:member:`referenceBoneCentroids` (:cpp:class:`BoneCentroids` at the
         newest). A field the tables hold nothing for keeps the config's value. The compiled parser calls this once per
         parse
         @endrst
         *
         * @param self The character's mod type
         * @param version The run's version of her (the mod's) -- ``std::nullopt`` reads at :cpp:member:`version`
         *
         * @return The filled config
         */
        WWMIParserConfig withGameData(const ModType& self, const std::optional<Version>& version = std::nullopt) const;

        /**
         * @brief
         @rst
         The game version the library files the character's rows under, used when the ``.ini``
         carries no ``fromVersion`` of its own. **Default**: ``"2.5"``
         @endrst
         */
        std::string version = "2.5";

        /**
         * @brief
         @rst
         The draw slot a slot's mod object names: ``("", <WWMINames::SlotPrefix><n>)`` is slot ``n``
         @endrst
         *
         * @param obj The mod object
         *
         * @return The slot, or ``std::nullopt`` for any other mod object
         */
        std::optional<int> slotOf(const std::pair<std::string, std::string>& obj) const;

        /**
         * @brief
         @rst
         The number a slot's mod object names under 'prefix': ``("", <prefix><n>)`` is ``n``, for digits only -- what
         :cpp:func:`slotOf` reads with :cpp:member:`WWMINames::SlotPrefix`
         @endrst
         *
         * @param obj The mod object
         * @param prefix The slot prefix, eg. ``component``
         *
         * @return The number, or ``std::nullopt`` for any other mod object
         */
        static std::optional<int> slotWithPrefix(const std::pair<std::string, std::string>& obj, const std::string& prefix);

        /**
         * @brief
         @rst
         The parser's graphs and the sections it placed in each, in the parser's order
         (:cpp:type:`IniParseData::SectionTargets`), as the parser's reading functions below take it
         @endrst
         */
        using SectionTargets = IniParseData::SectionTargets;

        /**
         * @brief A row a :cpp:class:`GraphCollect` of the parser filed (:cpp:type:`IniParseData::CollectedRow`)
         */
        using Row = IniParseData::CollectedRow;

        /**
         * @brief
         @rst
         Each draw slot the mod has sections for -> those sections: the roots of the slot graphs (the mod objects
         :cpp:func:`slotWithPrefix` reads with :cpp:member:`WWMINames::SlotPrefix`), in the order the ``.ini`` writes them,
         each once. A slot whose sections draw nothing (:cpp:func:`IniParseData::isUndrawn`) is listed too, and a slot graph
         with no sections maps to an empty list. A section 'sectionNames' does not name sorts after every one it does, in
         the parser's order
         @endrst
         *
         * @param targets The parser's graphs and their sections, in its order (:cpp:func:`GIMIParser::sectionTargets`)
         * @param sectionNames The ``.ini``'s sections, in the order it writes them; empty keeps the parser's order
         *
         * @return Draw slot -> its sections
         */
        static std::map<int, std::vector<std::string>> slotSections(const SectionTargets& targets, const std::vector<std::string>& sectionNames);

        /**
         * @brief
         @rst
         Whether the mod is written for WWMI's merged skeleton: a ``vg_offset`` row of 'objRows' sits in a section of a slot
         graph (:cpp:func:`slotSections`), the section's name matched ignoring case. A mod from before the merged skeleton
         carries no ``vg_offset`` there
         @endrst
         *
         * @param objRows The rows filed under :cpp:member:`IniParseRows::Objs`
         * @param targets The parser's graphs and their sections, in its order (:cpp:func:`GIMIParser::sectionTargets`)
         * @param slots How many draw slots count; a slot graph at or past it is not read. ``-1`` for no limit
         *
         * @return Whether it is a merged-skeleton mod
         */
        static bool mergedSkeleton(const std::vector<Row>& objRows, const SectionTargets& targets, int slots = -1);

        /**
         * @brief
         @rst
         The rules :cpp:func:`IniParseData::resourceDataOf` reads the mod's resource data by, as WWMI's parser hands them over:
         the rows the parser files under :cpp:member:`IniParseRows::Objs`, :cpp:member:`IniParseRows::Resources` and
         :cpp:member:`IniParseRows::Sections`; the slot graphs as the roots; ``ps-t`` registers
         (:cpp:member:`IniKeywords::TexRegPrefix`); no slot undrawn when every slot would be; RabbitFX as a library; a ``run =``
         drawing when it calls a section 'sectionNames' does not name outside WWMI's own namespace
         (:cpp:member:`IniKeywords::WWMINamespace`) :raw-html:`<br />` :raw-html:`<br />`

         Unless 'onlyThisFile', or this config has no textures (:cpp:member:`WWMITextureFacts::roles` and
         :cpp:member:`WWMITextureFacts::registerRoles` both empty), also the TOGGLES: the rows filed under
         :cpp:member:`WWMINames::TextureTogglesRows`, read in the ``TextureOverride`` sections that are not a fix's (a name
         ending :cpp:member:`IniKeywords::RemapFix`), the texture graphs as their roots, and a lone ``this`` under
         :cpp:member:`IniKeywords::WWMIObjectDetected` (or ``$object_detected==1``) not a toggle
         @endrst
         *
         * @param targets The parser's graphs and their sections, in its order (:cpp:func:`GIMIParser::sectionTargets`)
         * @param sectionNames The ``.ini``'s sections
         * @param onlyThisFile Whether the rules are for a linked ``.ini`` a fix reads only the resource data of
         * @param log Where the reading logs what it has to say. **Default**: nowhere
         *
         * @return The rules
         */
        IniParseData::ResourceDataRules resourceDataRules(const SectionTargets& targets, const std::vector<std::string>& sectionNames,
                                                          bool onlyThisFile, std::function<void(const std::string&)> log = {}) const;

        /**
         * @brief
         @rst
         Whether the mod replaces textures by HASH: a ``TextureOverride`` `section`_ that is no slot graph's root has a
         ``hash`` and a ``this`` among 'overrideRows'. Its art then reaches every draw binding that texture, whatever the
         draw's own section binds -- :cpp:member:`IniParseData::ResourceData::replacesTexturesByHash`
         @endrst
         *
         * @param overrideRows The rows filed under :cpp:member:`WWMINames::TextureOverridesRows`
         * @param targets The parser's graphs and their sections, in its order (:cpp:func:`GIMIParser::sectionTargets`)
         *
         * @return Whether it does
         */
        bool replacesTexturesByHash(const std::vector<Row>& overrideRows, const SectionTargets& targets) const;

        /**
         * @brief
         @rst
         The rules :cpp:func:`IniParseData::roleFilesOf` reads which file each drawn slot takes for each texture role by, as
         WWMI's parser hands them over, or ``std::nullopt`` when this config has no textures (:cpp:member:`WWMITextureFacts::roles`
         and :cpp:member:`WWMITextureFacts::registerRoles` both empty):

         #. the rows the parser files (:cpp:member:`IniParseRows::Sections`, :cpp:member:`IniParseRows::Declared`,
            :cpp:member:`IniParseRows::Objs`, :cpp:member:`WWMINames::TextureOverridesRows`,
            :cpp:member:`WWMINames::TextureRolesRows`, :cpp:member:`WWMINames::CarriedRows`)
         #. the slots with sections (#slotSections) as the objects, each slot's :cpp:member:`WWMITextureFacts::registerRoles`, and
            :cpp:member:`WWMITextureFacts::regionRoles`
         #. a lone ``this`` under ``$object_detected`` (or ``$object_detected==1``) not a toggle
         #. a hash's role: the character's :cpp:class:`Hashes` row typed by a role this config knows, else
            :cpp:member:`WWMITextureFacts::roles`; a file's game texture by its thumbprint
            (:cpp:func:`TexThumbprint::identifyFile` over :cpp:member:`WWMITextureFacts::textureThumbprints`); a hash's age
            (:cpp:func:`Hashes::age` over the library's hashes)
         #. a section a previous fix wrote (:cpp:func:`IniNamingTools::looksRemapped` over the character's name and the mods
            she fixes) is not the mod's use of a resource

         :cpp:member:`IniParseData::RoleFileRules::linked` and :cpp:member:`IniParseData::RoleFileRules::log` are left for the
         caller. Every function in the rules holds its own copies of what it reads, so the rules may outlive this config
         and 'self'
         @endrst
         *
         * @param targets The parser's graphs and their sections, in its order (:cpp:func:`GIMIParser::sectionTargets`)
         * @param sectionNames The ``.ini``'s sections, in the order it writes them
         * @param self The character's mod type, or ``nullptr`` for none: no hash is then read off her :cpp:class:`Hashes`
         * @param iniPath The ``.ini``'s path
         * @param iniFolder The ``.ini``'s folder
         *
         * @return The rules
         */
        std::optional<IniParseData::RoleFileRules> roleFileRules(const SectionTargets& targets, const std::vector<std::string>& sectionNames,
                                                                 const ModType* self, const std::string& iniPath,
                                                                 const std::string& iniFolder) const;

        /**
         * @brief
         @rst
         Which numbering of :cpp:member:`skeletonNumberings` the mod's blend's bone ids are in, asked of its own geometry
         (:cpp:class:`SkeletonNumberingProbe`) -- the numbering's version, or ``std::nullopt``. Not asked with fewer than two
         numberings or no :cpp:member:`referenceBoneCentroids`, of a mod from before WWMI's merged skeleton (not
         #mergedSkeleton), or of one whose resources name no position or blend buffer
         @endrst
         *
         * @param objRows The rows filed under :cpp:member:`IniParseRows::Objs`
         * @param resourceRows The rows filed under :cpp:member:`IniParseRows::Resources`
         * @param targets The parser's graphs and their sections, in its order (:cpp:func:`GIMIParser::sectionTargets`)
         * @param iniFolder The ``.ini``'s folder, which the resources' files are relative to
         * @param log Where the probe's answer is logged. **Default**: nowhere
         *
         * @return The version of the numbering
         */
        std::optional<std::string> skeletonNumbering(const std::vector<Row>& objRows, const std::vector<Row>& resourceRows,
                                                     const SectionTargets& targets, const std::string& iniFolder,
                                                     std::function<void(const std::string&)> log = {}) const;

        /**
         * @brief
         @rst
         The resource data with each of :cpp:member:`drawSplits`' draws added, or ``std::nullopt`` when there are no splits,
         no data, or the resources name no index, texcoord or position buffer. A split reads the draws of its drawn slot (not
         undrawn, with draws and sections) off the material-code map the slot takes for :cpp:member:`DrawSplit::codeRole` --
         the mod's own when a resource of the ``.ini`` whose name is not a fix's names that file, else
         :cpp:member:`DrawSplit::sourceFlags` -- at the draws' vertices (:cpp:class:`DrawUVCoverage`), and files the draws
         it picks as the draw ranges of ``(split name, slot)``
         @endrst
         *
         * @param data The resource data (:cpp:member:`IniParseData::ResourceDataVar`), or ``nullptr``
         * @param roles The role files (:cpp:member:`IniParseData::RoleFilesVar`), or ``nullptr``
         * @param targets The parser's graphs and their sections, in its order (:cpp:func:`GIMIParser::sectionTargets`)
         * @param iniPath The ``.ini``'s path
         * @param iniFolder The ``.ini``'s folder, which the resources' files are relative to
         * @param log Where a map that cannot be read is logged. **Default**: nowhere
         *
         * @return The resource data with the splits' draws
         */
        std::optional<IniParseData::ResourceData> splitDraws(const IniParseData::ResourceData* data, const IniParseData::RoleFileData* roles,
                                                             const SectionTargets& targets, const std::string& iniPath,
                                                             const std::string& iniFolder,
                                                             std::function<void(const std::string&)> log = {}) const;

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

         Left empty, the parser records no roles and :cpp:func:`IniParseData::textureRoles` answers
         ``nullptr``; the fixer then gives up, since it cannot tell what any texture is.

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

        /**
         * @brief
         @rst
         Every numbering this character's merged skeleton has had, when a game update RENUMBERED it --
         the parser asks the mod's own geometry which one its bone ids are in
         (:cpp:class:`SkeletonNumberingProbe`) and records that numbering's version as the mod's
         :cpp:func:`IniParseData::blendVersion` :raw-html:`<br />` :raw-html:`<br />`

         A mod's ``vb0`` cannot say: community hash-update tools rewrite a mod's hashes to the new
         version and leave its bone ids alone, so one of Lynae's 3.7-``vb0`` mods carries her 3.6
         numbering. Not asked of a mod from before WWMI's merged skeleton (no ``vg_offset`` in any slot
         section), whose ids are each component's own. Fewer than two numberings (the default) switches
         the probe off
         @endrst
         */
        std::vector<SkeletonNumberingProbe::Numbering> skeletonNumberings;

        /**
         * @brief
         @rst
         The rest-pose centroid of each of this character's bones, keyed by its id in the numbering
         :cpp:member:`skeletonNumberings` map onto -- the mean position of her own model's vertices whose
         heaviest weight (over half) is that bone
         @endrst
         */
        std::unordered_map<long long, std::array<double, 3>> referenceBoneCentroids;

        /**
         * @brief Some draws of one slot, picked out by the material-code map the mod draws them with
         */
        struct DrawSplit {
            /**
             * @brief
             @rst
             The split's name: the parser files the draws it picks as the
             :cpp:func:`IniParseData::drawRanges` of the mod object ``(name, <the slot's own name>)``
             @endrst
             */
            std::string name;

            /**
             * @brief The source slot whose draws are read
             */
            int component = -1;

            /**
             * @brief The role of the material-code map, among the slot's textures, that marks the draws
             */
            std::string codeRole;

            /**
             * @brief
             @rst
             The flag bits that mark a texel, in the code a shader reads off the map's red channel
             (:cpp:func:`TextureFile::shaderChannel`): a draw is picked when more than :cpp:member:`share`
             of the texels under its vertices carry any of them
             @endrst
             */
            int flags = 0;

            /**
             * @brief
             @rst
             Where a material-code map carries the split's :cpp:member:`flags`, cut into ``width x height``
             blocks: a block's bit is set when more than half of its texels carry any of them. Bits are
             row-major, the lowest bit of each byte first
             @endrst
             */
            struct FlagMask {
                /**
                 * @brief The number of blocks across
                 */
                int width = 0;

                /**
                 * @brief The number of blocks down
                 */
                int height = 0;

                /**
                 * @brief The blocks, one bit each
                 */
                std::vector<std::uint8_t> bits;
            };

            /**
             * @brief
             @rst
             The character's own game code map of :cpp:member:`codeRole`, as a :cpp:struct:`FlagMask`, for a
             mod that binds no code map of its own for that role: such a mod draws with the game's map on its
             own character, so that map decides which of its draws carry the flags. **Default**: empty, and
             such a mod's slot is not split
             @endrst
             */
            FlagMask sourceFlags;

            /**
             * @brief
             @rst
             The share of a draw's texels that must carry :cpp:member:`flags` for the draw to be picked, eg.
             ``0.01`` for a draw that holds any real amount of the flagged material among others.
             **Default**: ``0.5``, more than half
             @endrst
             */
            double share = 0.5;

            /**
             * @brief
             @rst
             The :cpp:class:`MaterialCodes` row naming :cpp:member:`flags`: ``<codeRole>.flags.<flag>`` on the slot,
             eg. ``film`` for ``jacketDetail.flags.film``. :cpp:func:`WWMIParserConfig::withGameData` reads it into
             :cpp:member:`flags` (and refuses a name with no row). **Default**: ``""``, :cpp:member:`flags` as given
             @endrst
             */
            std::string flag;
        };

        /**
         * @brief
         @rst
         Draws of a slot to pick out by the mod's own material-code map -- never by its colours -- for a fixer
         to treat apart (:cpp:member:`WWMIFixerConfig::drawReroutes`). Lynae paints a see-through film with
         flag ``0x10``. A slot whose map is not the mod's own is read through
         :cpp:member:`DrawSplit::sourceFlags`, or not split without one; a split no draw carries is not filed.
         **Default**: none
         @endrst
         */
        std::vector<DrawSplit> drawSplits;
    };


    /**
     * @brief
     @rst
     Builds the parser for a mod of one Wuthering Waves character -- see :cpp:struct:`WWMIParserConfig`
     @endrst
     * @param config The character's config
     */
    IniParseBuilder::Factory makeWWMIParser(WWMIParserConfig config);

    /**
     * @brief
     @rst
     What :cpp:func:`makeWWMIParser` returns, as an :cpp:type:`IniParseBuilder::Factory`: the character's config and the
     call that builds a parser from it -- so the config can be read back (:cpp:func:`WWMIParserConfig::ofFactory`)
     @endrst
     */
    struct WWMIParseFactory {
        /**
         * @brief The character's config
         */
        WWMIParserConfig config;

        /**
         * @brief Builds the parser, at the ``.ini`` file's own version, else the config's -- see
         * :cpp:type:`IniParseBuilder::Factory`
         */
        std::shared_ptr<BaseIniParser<>> operator()(IniFile* iniFile, std::optional<int> modTypeId) const;
    };

    /**
     * @brief
     @rst
     The mod objects the parser :cpp:func:`makeWWMIParser` builds places sections in besides the draw slots, the
     :cpp:member:`WWMIParserConfig::hashOnlyObjs` and the texture roles
     @endrst
     */
    struct WWMIParseObjs {
        /**
         * @brief
         @rst
         ``("", "watch")``: a `section`_ that only WATCHES the character -- it matches her
         :cpp:member:`IniKeywords::Vb0` hash at any version, and has no ``match_first_index``, so it
         draws nothing and is there to set a variable while she is on screen (a help overlay, a key menu)
         @endrst
         */
        static inline const std::string Watch = "watch";
    };
}

#endif
