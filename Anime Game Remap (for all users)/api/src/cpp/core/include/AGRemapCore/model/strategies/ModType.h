#ifndef AGRemapCore_ModType_H
#define AGRemapCore_ModType_H

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
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/assets/Indices.h"
#include "AGRemapCore/model/assets/IndexCounts.h"
#include "AGRemapCore/model/assets/ShapeKeyChecksums.h"
#include "AGRemapCore/model/assets/DrawPasses.h"
#include "AGRemapCore/model/assets/TextureRegisters.h"
#include "AGRemapCore/model/assets/GameShaders.h"
#include "AGRemapCore/model/assets/ShapeKeyDispatchSizes.h"
#include "AGRemapCore/model/assets/BoneCentroids.h"
#include "AGRemapCore/model/assets/ShapeKeyRemaps.h"
#include "AGRemapCore/model/assets/MaterialCodes.h"
#include "AGRemapCore/model/assets/MaterialColours.h"
#include "AGRemapCore/model/assets/TextureDescs.h"
#include "AGRemapCore/model/assets/TextureThumbprints.h"
#include "AGRemapCore/model/textures/Colour.h"
#include "AGRemapCore/model/assets/VertexCounts.h"
#include "AGRemapCore/model/assets/VGCounts.h"
#include "AGRemapCore/model/assets/VGOffsets.h"
#include "AGRemapCore/model/assets/VGRemaps.h"
#include "AGRemapCore/model/iftemplate/IfContentPartColour.h"
#include "AGRemapCore/tools/Ranges.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"
#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"


namespace AGRemapCore {

    // Declared rather than included: IniFile.h includes this header, so the dependency can only
    // run the other way from ModType.cpp.
    class IniFile;


    /**
     * @brief
     @rst
     Heavy data for a type of mod :raw-html:`<br />` :raw-html:`<br />`

     Meant to carry the full C++-side representation of a mod type -- contrast with the cheap
     :cpp:class:`ModTypeIdData` an ini classifier (e.g. :cpp:class:`IniClassifier`) holds instead.
     Exposed to Python as ``ModType``.
     @endrst
     */
    class ModType {
        public:

            /**
             * @brief Constructs new data for a type of mod
             *
             * @param gameTypeId
             @rst
             The id for the game this type of mod belongs to -- stored as-is, with no validation
             that it corresponds to one of :cpp:enum:`GameTypeId`'s declared values (see
             :cpp:class:`GameTypeIdTools` if that's needed)
             @endrst
             * @param modTypeId
             @rst
             The id for this specific type of mod -- stored as-is, with no validation that it
             corresponds to one of :cpp:enum:`ModTypeId`'s declared values (see
             :cpp:class:`ModTypeIdTools` if that's needed), so a custom mod type using some id not
             registered in :cpp:enum:`ModTypeId` can still be represented
             @endrst
             * @param name The default name for the type of mod
             * @param aliases Other alternative names for the type of mod
             * @param hashes
             @rst
             The hashes related to the mod and its fix -- see #hashes :raw-html:`<br />`
             :raw-html:`<br />`

             If this is ``nullptr``, a new, fully-populated :cpp:class:`Hashes` is constructed
             instead :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param indices
             @rst
             The indices related to the mod and its fix -- see #indices :raw-html:`<br />`
             :raw-html:`<br />`

             If this is ``nullptr``, a new, fully-populated :cpp:class:`Indices` is constructed
             instead :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param vertexCounts
             @rst
             The vertex counts related to the mod -- see #vertexCounts :raw-html:`<br />`
             :raw-html:`<br />`

             If this is ``nullptr``, a new, fully-populated :cpp:class:`VertexCounts` is constructed
             instead :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param vgRemaps
             @rst
             The vertex group remaps for the mod -- see #vgRemaps :raw-html:`<br />`
             :raw-html:`<br />`

             If this is ``nullptr``, the **shared** :cpp:func:`ModDataAssets::vgRemaps` is used --
             *not* a fresh table, unlike #hashes/#indices/#vertexCounts :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param iniParseBuilder
             @rst
             The builder that builds the parser for a ``.ini`` file of this type of mod -- see
             #iniParseBuilder :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, a default-constructed :cpp:class:`IniParseBuilder` is used
             instead :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param iniFixBuilder
             @rst
             The builder that builds the fixer for a ``.ini`` file of this type of mod -- see
             #iniFixBuilder :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, a default-constructed :cpp:class:`IniFixBuilder` is used
             instead :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param iniRemoveBuilder
             @rst
             The builder that hands out the remover for a ``.ini`` file of this type of mod -- see
             #iniRemoveBuilder :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, :cpp:func:`GlobalIniRemoveBuilders::removeBuilder` is used
             instead -- note that this is a *shared* builder, not a fresh one per :cpp:class:`ModType`
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit ModType(int gameTypeId, int modTypeId, const std::string &name, const std::vector<std::string> &aliases = {},
                              std::shared_ptr<Hashes> hashes = nullptr,
                              std::shared_ptr<Indices> indices = nullptr,
                              std::shared_ptr<VertexCounts> vertexCounts = nullptr,
                              std::shared_ptr<VGRemaps> vgRemaps = nullptr,
                              std::shared_ptr<IniParseBuilder> iniParseBuilder = nullptr,
                              std::shared_ptr<IniFixBuilder> iniFixBuilder = nullptr,
                              std::shared_ptr<IniRemoveBuilder> iniRemoveBuilder = nullptr,
                              std::shared_ptr<IndexCounts> indexCounts = nullptr,
                              std::shared_ptr<VGOffsets> vgOffsets = nullptr,
                              std::shared_ptr<VGCounts> vgCounts = nullptr,
                              std::shared_ptr<ShapeKeyChecksums> shapeKeyChecksums = nullptr,
                              std::shared_ptr<DrawPasses> drawPasses = nullptr,
                              std::shared_ptr<TextureRegisters> textureRegisters = nullptr,
                              std::shared_ptr<GameShaders> gameShaders = nullptr,
                              std::shared_ptr<ShapeKeyDispatchSizes> shapeKeyDispatchSizes = nullptr,
                              std::shared_ptr<BoneCentroids> boneCentroids = nullptr,
                              std::shared_ptr<ShapeKeyRemaps> shapeKeyRemaps = nullptr,
                              std::shared_ptr<MaterialCodes> materialCodes = nullptr,
                              std::shared_ptr<MaterialColours> materialColours = nullptr,
                              std::shared_ptr<TextureDescs> textureDescs = nullptr,
                              std::shared_ptr<TextureThumbprints> textureThumbprints = nullptr);

            /**
             * @brief The id for the game this type of mod belongs to
             */
            int gameTypeId;

            /**
             * @brief The id for this specific type of mod
             */
            int modTypeId;

            /**
             * @brief The default name for the type of mod
             */
            std::string name;

            /**
             * @brief Other alternative names for the type of mod
             */
            std::vector<std::string> aliases;

            /**
             * @brief
             @rst
             The hashes related to the mod and its fix :raw-html:`<br />` :raw-html:`<br />`

             A :cpp:class:`ModType` constructed without one gets a **fully-populated**
             :cpp:class:`Hashes` (every hash the software ships with), not an empty table
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A ``shared_ptr`` rather than a by-value member, for the same two reasons as
                #iniParseBuilder: :cpp:class:`ModType` must stay cheap to copy (it is returned by
                value from :cpp:func:`ModTypeIdTools::getModType` and stored by value in
                :cpp:member:`IniFile::modTypes`), and copying one must *share* its hashes rather
                than clone them

             .. note::
                :cpp:class:`Hashes` is mutable (:cpp:func:`ModMappedAssets::addRepoRows`/
                :cpp:func:`ModMappedAssets::addMap`), so two :cpp:class:`ModType`\\s deliberately
                handed the *same* ``shared_ptr`` share those mutations. Each one built with the
                ``nullptr`` default instead gets its own table
             @endrst
             */
            std::shared_ptr<Hashes> hashes;

            /**
             * @brief
             @rst
             The indices related to the mod and its fix :raw-html:`<br />` :raw-html:`<br />`

             Same default as #hashes: a
             :cpp:class:`ModType` constructed without one gets a **fully-populated**
             :cpp:class:`Indices` (every index the software ships with), not an empty table
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A ``shared_ptr``, per-:cpp:class:`ModType` by default and shared on copy, for
                exactly the reasons spelled out on #hashes -- see that member's two notes
             @endrst
             */
            std::shared_ptr<Indices> indices;

            /**
             * @brief
             @rst
             The vertex counts related to the mod :raw-html:`<br />` :raw-html:`<br />`

             A :cpp:class:`ModType` constructed without one gets a **fully-populated**
             :cpp:class:`VertexCounts`, not an empty table :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike #hashes and #indices this is a :cpp:class:`ModDictAssets` rather than a
                :cpp:class:`ModMappedAssets`, so it has no ``getMap``/``hasFrom`` -- see
                :cpp:class:`VertexCounts` for why. Ownership works identically though: a
                ``shared_ptr``, per-:cpp:class:`ModType` by default and shared on copy
             @endrst
             */
            std::shared_ptr<VertexCounts> vertexCounts;

            /**
             * @brief
             @rst
             The vertex group remaps for the mod -- maps the blend indices of this mod's vertex
             groups onto another mod's :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                **Its default differs from the other three asset tables.** #hashes, #indices and
                #vertexCounts each get a *fresh* table when not supplied; this one falls back to the
                single **shared** :cpp:func:`ModDataAssets::vgRemaps`. So mutating a defaulted
                ``vgRemaps`` affects every other :cpp:class:`ModType` that also defaulted, whereas
                mutating a defaulted ``hashes`` does not. That asymmetry is deliberate -- this is
                much the largest of the tables

             .. note::
                Otherwise the ownership rules are the same as #hashes: a ``shared_ptr``, shared on
                copy
             @endrst
             */
            std::shared_ptr<VGRemaps> vgRemaps;

            /**
             * @brief
             @rst
             The ``match_index_count`` of each of the mod's draw slots (WuWa) :raw-html:`<br />` :raw-html:`<br />`

             The first of the four WWMI-only asset tables, each an :cpp:class:`Indices`
             sibling keyed ``(version, name, component, type)`` with the draw slot as ``type``:
             this one, #vgOffsets, #vgCounts and #shapeKeyChecksums. They default the way #hashes
             does -- a fresh, fully-populated table with an empty remap map,
             which for a GI mod type holds no row of its own and remaps onto nothing
             @endrst
             */
            std::shared_ptr<IndexCounts> indexCounts;

            /**
             * @brief The ``vg_offset`` of each of the mod's draw slots (WuWa) -- see #indexCounts
             */
            std::shared_ptr<VGOffsets> vgOffsets;

            /**
             * @brief The ``vg_count`` of each of the mod's draw slots (WuWa) -- see #indexCounts
             */
            std::shared_ptr<VGCounts> vgCounts;

            /**
             * @brief The shape-key ``checksum`` of the mod (WuWa) -- see #indexCounts
             */
            std::shared_ptr<ShapeKeyChecksums> shapeKeyChecksums;

            /**
             * @brief
             @rst
             The draw passes of each of the mod's draw slots (WuWa) -- see :cpp:class:`DrawPasses` and #getDrawPass
             :raw-html:`<br />` :raw-html:`<br />`

             The first of six versioned tables of a character's GAME data that a fix reads at the run's version rather
             than carrying it: this, #textureRegisters, #gameShaders, #shapeKeyDispatchSizes, #boneCentroids and
             #shapeKeyRemaps. They default as #hashes does: a fresh, fully-populated table each
             @endrst
             */
            std::shared_ptr<DrawPasses> drawPasses;

            /**
             * @brief Where the mod's own draws bind each texture role -- see :cpp:class:`TextureRegisters` and #getTextureRegister
             */
            std::shared_ptr<TextureRegisters> textureRegisters;

            /**
             * @brief The game pixel shaders a fix draws the mod's parts with -- see :cpp:class:`GameShaders` and #getGameShader
             */
            std::shared_ptr<GameShaders> gameShaders;

            /**
             * @brief The height WWMI dispatches the mod's shape keys with -- see :cpp:class:`ShapeKeyDispatchSizes`
             */
            std::shared_ptr<ShapeKeyDispatchSizes> shapeKeyDispatchSizes;

            /**
             * @brief Each bone's rest-pose centroid on the mod's own model -- see :cpp:class:`BoneCentroids` and #getBoneCentroids
             */
            std::shared_ptr<BoneCentroids> boneCentroids;

            /**
             * @brief The order the mod's shape keys are put in on another character -- see :cpp:class:`ShapeKeyRemaps` and #getShapeKeyRemap
             */
            std::shared_ptr<ShapeKeyRemaps> shapeKeyRemaps;

            /**
             * @brief What the codes of the mod's single-channel code maps mean -- see :cpp:class:`MaterialCodes` and #getMaterialCode
             */
            std::shared_ptr<MaterialCodes> materialCodes;

            /**
             * @brief The colours the mod's material masks mark each material with -- see :cpp:class:`MaterialColours` and #getMaterialColour
             */
            std::shared_ptr<MaterialColours> materialColours;

            /**
             * @brief How the game creates the mod's game textures -- see :cpp:class:`TextureDescs` and #getTextureDesc
             */
            std::shared_ptr<TextureDescs> textureDescs;

            /**
             * @brief What the mod's game textures look like, as thumbprints -- see :cpp:class:`TextureThumbprints` and
             *     #getTextureThumbprints
             */
            std::shared_ptr<TextureThumbprints> textureThumbprints;

            /**
             * @brief
             @rst
             The builder that builds the parser for a ``.ini`` file of this type of mod
             :raw-html:`<br />` :raw-html:`<br />`

             A *factory*, not a parser: :cpp:func:`IniFile::parse` builds a **fresh** parser per
             ``.ini`` file from it, passing that file's :cpp:member:`IniFile::version` along, so a
             mod type can use a different parser for a 4.0-era ``.ini`` file than for a 5.7-era one
             -- see :cpp:class:`IniParseBuilder` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The built parser arrives already bound to its own file, so two
                :cpp:class:`IniFile`\\s of the same mod type never share a parser's binding

             .. note::
                A ``shared_ptr`` rather than a ``unique_ptr`` because :cpp:class:`ModType` must stay
                **copyable** -- :cpp:func:`ModTypeIdTools::getModType` returns one by value, and
                :cpp:member:`IniFile::modTypes` stores them by value. It also lets every
                :cpp:class:`ModType` of a game share one builder
             @endrst
             */
            std::shared_ptr<IniParseBuilder> iniParseBuilder;

            /**
             * @brief
             @rst
             The builder that builds the fixer for a ``.ini`` file of this type of mod
             :raw-html:`<br />` :raw-html:`<br />`

             A *factory*, not a fixer: :cpp:func:`IniFile::fix` builds a **fresh** fixer per ``.ini`` file from it,
             passing that file's :cpp:member:`IniFile::version` along, so a mod type can use a
             different fixer for a 4.0-era ``.ini`` file than for a 5.7-era one -- see
             :cpp:class:`IniFixBuilder` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The built fixer arrives already bound to that file's own built parser, so nothing
                rebinds it afterwards and two :cpp:class:`IniFile`\\s of the same mod type never
                share a fixer -- as with #iniParseBuilder on the parser side

             .. note::
                A ``shared_ptr`` for the same reasons as #iniParseBuilder -- :cpp:class:`ModType`
                must stay copyable, and every :cpp:class:`ModType` of a game shares one builder
             @endrst
             */
            std::shared_ptr<IniFixBuilder> iniFixBuilder;

            /**
             * @brief
             @rst
             The builder that hands out the remover for a ``.ini`` file of this type of mod
             :raw-html:`<br />` :raw-html:`<br />`

             A *factory*: :cpp:func:`IniFile::removeFix` asks it for a remover per ``.ini`` file, and it hands
             one back already bound to that file :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This behaves like #iniParseBuilder and #iniFixBuilder -- a fresh remover per
                :cpp:func:`IniRemoveBuilder::build`, bound to the caller's file and nobody else's

             .. note::
                A ``shared_ptr`` for the same reasons as #iniParseBuilder: several
                :cpp:class:`ModType`\\s share one builder (notably every one falling back to
                :cpp:func:`GlobalIniRemoveBuilders::removeBuilder`), and a builder is immutable once
                constructed, so sharing it is free
             @endrst
             */
            std::shared_ptr<IniRemoveBuilder> iniRemoveBuilder;

            /**
             * @brief Determines whether this mod type goes by some name
             @rst
             Compared case-insensitively against #name and every entry in #aliases
             @endrst
             *
             * @param name The name to check
             *
             * @return Whether this mod type goes by 'name'
             */
            bool isName(const std::string& name) const;

            /**
             * @brief
             @rst
             The names of the mods this mod type can be fixed onto :raw-html:`<br />`
             :raw-html:`<br />`

             Read from the remap targets that actually exist in the asset tables
             (:cpp:func:`ModMappedAssets::getMap`)
             @endrst
             *
             * @return The names of the mods to fix to
             */
            std::unordered_set<std::string> getModsToFix() const;

            /**
             * @brief The number of vertices for this mod
             *
             * @param version
             @rst
             The game version wanted, or ``std::nullopt`` for the latest
             @endrst
             *
             * @return
             @rst
             The vertex count for this mod as a whole -- the ``component`` index column is queried
             as ``""``, which is what every shipped row carries -- or ``std::nullopt`` if this mod
             type has no row for it
             @endrst
             */
            std::optional<int> getVertexCount(const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief The vertex group remap for fixing this mod type onto another
             *
             * @param modName The name of the mod being fixed onto
             * @param fromVersion The version being fixed from, or ``std::nullopt`` for the latest
             * @param toVersion The version being fixed to, or ``std::nullopt`` for the latest
             * @param fromComp
             @rst
             The component being fixed from. ``std::nullopt`` leaves the column unconstrained
             @endrst
             * @param toComp The component being fixed onto, with the same ``std::nullopt`` meaning
             *
             * @return The remap, or ``std::nullopt`` if the table has no matching row
             */
            std::optional<VGRemap> getVGRemap(const std::string& modName,
                                               const std::optional<Version>& fromVersion = std::nullopt,
                                               const std::optional<Version>& toVersion = std::nullopt,
                                               const std::optional<std::string>& fromComp = std::nullopt,
                                               const std::optional<std::string>& toComp = std::nullopt) const;

            /**
             * @brief
             @rst
             The ``match_index_count`` of one of this mod's draw slots (WuWa) :raw-html:`<br />` :raw-html:`<br />`

             The four lookups below are the same shape: the slot is the ``type`` column
             (``component0``, ``component1``, ...), the ``component`` column is ``""`` for every
             WWMI row, and a mod type with no row answers ``std::nullopt``. The value is the string
             the ``.ini`` carries, as :cpp:class:`Indices` values are
             @endrst
             *
             * @param type The draw slot, eg. ``component3``
             * @param component The component column (``""`` for every shipped row)
             * @param version The version, or ``std::nullopt`` for the latest
             */
            std::optional<std::string> getIndexCount(const std::string& type, const std::string& component = "",
                                                     const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief The ``vg_offset`` of one of this mod's draw slots (WuWa) -- see #getIndexCount
             */
            std::optional<std::string> getVGOffset(const std::string& type, const std::string& component = "",
                                                   const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief The ``vg_count`` of one of this mod's draw slots (WuWa) -- see #getIndexCount
             */
            std::optional<std::string> getVGCount(const std::string& type, const std::string& component = "",
                                                  const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief The shape-key ``checksum`` of this mod (WuWa) -- see #getIndexCount; the ``type`` is ``shapekeys`` for every shipped row
             */
            std::optional<std::string> getShapeKeyChecksum(const std::string& type = "shapekeys", const std::string& component = "",
                                                           const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief
             @rst
             One of #drawPasses' values for a draw slot: ``<pass>`` (its vertex shaders, comma-separated),
             ``<pass>.ps`` (its pixel shader) or ``<pass>.tags`` (each vertex shader's ``filter_index``)
             @endrst
             *
             * @param component The draw slot, eg. ``component3``
             * @param type The pass and what is read of it, eg. ``main``, ``other.ps``
             * @param version The game version wanted -- the newest row at or below it; ``std::nullopt`` for the latest
             *
             * @return The value, or ``std::nullopt`` when the mod has no such row
             */
            std::optional<std::string> getDrawPass(const std::string& component, const std::string& type,
                                                   const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief
             @rst
             The register the mod's own draw of a slot binds a texture role at (#textureRegisters) -- else the game's
             default for the role, the row named after the mod's game (:cpp:func:`GameTypeIdTools::getName`)
             @endrst
             *
             * @param component The draw slot, eg. ``component3``
             * @param role The role, eg. ``upperDiffuse``; ``<role>@rabbitfx`` for the key it is handed to RabbitFX under;
             *     ``<role>@<pass>``, eg. ``upperDiffuse@other``, for where another pass of the slot (#getDrawPass's
             *     ``other``) binds it
             * @param version The game version wanted -- see #getDrawPass
             *
             * @return The register, or ``std::nullopt`` when neither the mod nor its game has a row for the role
             */
            std::optional<std::string> getTextureRegister(const std::string& component, const std::string& role,
                                                          const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief
             @rst
             The texture role the mod's own draw of a slot binds at a register -- #getTextureRegister read backwards,
             with the game's default where the mod has no row of the role. A RabbitFX key answers with the role
             it carries (no ``@rabbitfx``)
             @endrst
             *
             * @param component The draw slot, eg. ``component3``
             * @param reg The register, eg. ``ps-t3``, or a RabbitFX key
             * @param version The game version wanted -- see #getDrawPass
             * @param drawPass The pass of the slot, as #getDrawPass names it (``other``, ``other2``, ...): its
             *     ``<role>@<pass>`` rows are read, and the role answered without the suffix. Empty for the slot's main
             *     pass (its plain rows and RabbitFX keys)
             *
             * @return The role, or ``std::nullopt`` when no role is bound there
             */
            std::optional<std::string> getTextureRole(const std::string& component, const std::string& reg,
                                                      const std::optional<Version>& version = std::nullopt,
                                                      const std::string& drawPass = "") const;

            /**
             * @brief One of #gameShaders' values: a shader's HLSL (under its name, eg. ``LynaeUpper``) or a constant buffer it read (under the name, a dot and the register, eg. ``LynaeUpper.ps-cb4``)
             *
             * @param type The shader, eg. ``LynaeUpper``, or one of its constant buffers, eg. ``LynaeUpper.ps-cb4``
             * @param version The game version wanted -- see #getDrawPass
             *
             * @return The value, or ``std::nullopt`` when the mod has no such row
             */
            std::optional<std::string> getGameShader(const std::string& type, const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief The height WWMI dispatches the mod's shape keys with (#shapeKeyDispatchSizes)
             *
             * @param version The game version wanted -- see #getDrawPass
             *
             * @return The height, or ``std::nullopt`` when the mod has no row
             */
            std::optional<std::string> getShapeKeyDispatchSize(const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief Each bone's rest-pose centroid on the mod's own model (#boneCentroids), at a version
             *
             * @param version The game version wanted -- for each bone, the newest row at or below it
             *
             * @return Bone id -> centroid; empty when the mod has none
             */
            std::map<long long, std::array<double, 3>> getBoneCentroids(const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief The source shape key each of another mod's shape-key slots takes, from this mod (#shapeKeyRemaps)
             *
             * @param modName The mod remapped to
             * @param fromVersion This mod's version -- the newest row at or below it
             * @param toVersion The other mod's version -- likewise
             *
             * @return The order (``-1`` for a slot that takes none), or ``std::nullopt`` when the table has none for the pair
             */
            std::optional<std::vector<long long>> getShapeKeyRemap(const std::string& modName, const std::optional<Version>& fromVersion = std::nullopt,
                                                                   const std::optional<Version>& toVersion = std::nullopt) const;

            /**
             * @brief
             @rst
             A field or a code of one of the mod's code maps (#materialCodes): ``<role>.<field>`` the bits the field
             occupies (``propsDetail.flags.film``), ``<role>.<field>.<name>`` one code of it (``pouchDetail.material.plain``)
             @endrst
             *
             * @param component The draw slot, eg. ``component6``
             * @param type The row's type
             * @param version The game version wanted -- see #getDrawPass
             *
             * @return The number, or ``std::nullopt`` when the mod has no such row
             */
            std::optional<long long> getMaterialCode(const std::string& component, const std::string& type,
                                                     const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief The colour one of the mod's material masks marks a material with (#materialColours), eg. ``torsoMask.skin``
             *
             * @param component The draw slot, eg. ``component3``
             * @param type The row's type, ``<role>.<material>``
             * @param version The game version wanted -- see #getDrawPass
             *
             * @return The colour, or ``std::nullopt`` when the mod has no such row
             */
            std::optional<Colour> getMaterialColour(const std::string& component, const std::string& type,
                                                    const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief How the game creates one of the mod's game textures (#textureDescs)
             *
             * @param textureHash The texture's hash
             * @param version The game version wanted -- see #getDrawPass
             *
             * @return ``(format, width, height, mips)`` -- the format by its name, ``BC7_TYPELESS`` -- or ``std::nullopt``
             *     when the mod has no such row
             */
            std::optional<std::tuple<std::string, std::uint32_t, std::uint32_t, std::uint32_t>> getTextureDesc(
                const std::string& textureHash, const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief The thumbprint of each of the mod's game textures (#textureThumbprints), by the texture's hash
             *
             * @param version The game version wanted -- see #getDrawPass
             *
             * @return Hash -> its thumbprint's bytes (16 x 16, row by row); empty when the mod has no rows
             */
            std::unordered_map<std::string, std::vector<std::uint8_t>> getTextureThumbprints(
                const std::optional<Version>& version = std::nullopt) const;

            /**
             * @brief The help text describing this mod type, as the CLI prints it
             *
             * @return The help text
             */
            std::string getHelpStr() const;

            /**
             * @brief
             @rst
             Fixes a ``.ini`` file, but **only if that file was classified as this mod type** --
             a no-op otherwise :raw-html:`<br />` :raw-html:`<br />`

             Returns nothing: the fix it produces is written out by
             :cpp:func:`IniFile::fix` rather than handed back. Call that directly to see it
             @endrst
             *
             * @param iniFile The ``.ini`` file to fix
             * @param keepBackup Whether to keep a backup copy of the original ``.ini`` file
             * @param fixOnly Whether to only fix without removing any previous fix
             */
            void fixIni(IniFile& iniFile, bool keepBackup = true, bool fixOnly = false) const;

            /**
             * @brief
             @rst
             The valid ranges of order indices within an :cpp:class:`IfContentPart` whose ``hash``
             values belong to this mod type
             @endrst
             *
             * @param partColours The current states of the ``IfContentPart``
             * @param version The version the hashes should come from, or ``std::nullopt`` for any
             * @param nonVersionVals
             @rst
             Values for the non-version index columns, used to narrow which instance of a hash is
             wanted -- see :cpp:func:`ModMappedAssets::hasFrom`
             @endrst
             *
             * @return The valid ranges of indices
             */
            Ranges<long long> getHashRanges(const IfContentPartColouring<std::string, std::string>& partColours,
                                             const std::optional<Version>& version = std::nullopt,
                                             const std::vector<std::optional<std::string>>& nonVersionVals = {}) const;
    };
}

#endif
