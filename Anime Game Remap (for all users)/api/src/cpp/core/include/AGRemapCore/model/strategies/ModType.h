#ifndef AGRemapCore_ModType_H
#define AGRemapCore_ModType_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/assets/Indices.h"
#include "AGRemapCore/model/assets/VertexCounts.h"
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
     The Python-side ``ModType`` is meant to build itself using this data.
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
             instead, mirroring the pure-Python original's own
             ``if (hashes is None): hashes = Hashes()`` fallback :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param indices
             @rst
             The indices related to the mod and its fix -- see #indices :raw-html:`<br />`
             :raw-html:`<br />`

             If this is ``nullptr``, a new, fully-populated :cpp:class:`Indices` is constructed
             instead, mirroring the pure-Python original's own
             ``if (indices is None): indices = Indices()`` fallback :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param vertexCounts
             @rst
             The vertex counts related to the mod -- see #vertexCounts :raw-html:`<br />`
             :raw-html:`<br />`

             If this is ``nullptr``, a new, fully-populated :cpp:class:`VertexCounts` is constructed
             instead, mirroring the pure-Python original's own
             ``if (vertexCounts is None): vertexCounts = VertexCounts()`` fallback
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param vgRemaps
             @rst
             The vertex group remaps for the mod -- see #vgRemaps :raw-html:`<br />`
             :raw-html:`<br />`

             If this is ``nullptr``, the **shared** :cpp:func:`ModDataAssets::vgRemaps` is used --
             *not* a fresh table, unlike #hashes/#indices/#vertexCounts. That mirrors the pure-Python
             original's own ``if (vgRemaps is None): vgRemaps = ModDataAssets.VGRemaps.value``
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param iniParseBuilder
             @rst
             The builder that builds the parser for a ``.ini`` file of this type of mod -- see
             #iniParseBuilder :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, a default-constructed :cpp:class:`IniParseBuilder` is used
             instead, mirroring the pure-Python original's own
             ``if (iniParseBuilder is None): iniParseBuilder = IniParseBuilder(GIMIParser)``
             fallback :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param iniFixBuilder
             @rst
             The builder that builds the fixer for a ``.ini`` file of this type of mod -- see
             #iniFixBuilder :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, a default-constructed :cpp:class:`IniFixBuilder` is used
             instead, mirroring the pure-Python original's own ``iniFixBuilder`` fallback
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param iniRemoveBuilder
             @rst
             The builder that hands out the remover for a ``.ini`` file of this type of mod -- see
             #iniRemoveBuilder :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, :cpp:func:`GlobalIniRemoveBuilders::removeBuilder` is used
             instead, mirroring the pure-Python original's own
             ``iniRemoveBuilder = GlobalIniRemoveBuilders.RemoveBuilder.value`` fallback -- note
             that this is a *shared* builder, not a fresh one per :cpp:class:`ModType`
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
                              std::shared_ptr<IniRemoveBuilder> iniRemoveBuilder = nullptr);

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

             Mirrors the pure-Python ``ModType.hashes``, including its default: a
             :cpp:class:`ModType` constructed without one gets a **fully-populated**
             :cpp:class:`Hashes` (every hash the software ships with), not an empty table
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A ``shared_ptr`` rather than a by-value member, for the same two reasons as
                #iniParseBuilder: :cpp:class:`ModType` must stay cheap to copy (it is returned by
                value from :cpp:func:`ModTypeIdTools::getModType` and stored by value in
                :cpp:member:`IniFile::modTypes`), and copying one must *share* its hashes rather
                than clone them -- which is also what the pure-Python original does, since its
                ``self.hashes`` is an ordinary object reference

             .. note::
                :cpp:class:`Hashes` is mutable (:cpp:func:`ModMappedAssets::addRepoRows`/
                :cpp:func:`ModMappedAssets::addMap`), so two :cpp:class:`ModType`\\s deliberately
                handed the *same* ``shared_ptr`` share those mutations. Each one built with the
                ``nullptr`` default instead gets its own table, matching the original's
                per-``ModType`` ``Hashes()``
             @endrst
             */
            std::shared_ptr<Hashes> hashes;

            /**
             * @brief
             @rst
             The indices related to the mod and its fix :raw-html:`<br />` :raw-html:`<br />`

             Mirrors the pure-Python ``ModType.indices``, which likewise lives on the base mod type
             rather than in a game-specific subclass. Same default as #hashes: a
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

             Mirrors the pure-Python ``ModType.vertexCounts``, including its default: a
             :cpp:class:`ModType` constructed without one gets a **fully-populated**
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

             Mirrors the pure-Python ``ModType.vgRemaps`` :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                **Its default differs from the other three asset tables.** #hashes, #indices and
                #vertexCounts each get a *fresh* table when not supplied; this one falls back to the
                single **shared** :cpp:func:`ModDataAssets::vgRemaps`, matching the pure-Python
                original. So mutating a defaulted ``vgRemaps`` affects every other
                :cpp:class:`ModType` that also defaulted, whereas mutating a defaulted ``hashes``
                does not. That asymmetry is deliberate and upstream, not an oversight -- and it
                matters here because this is much the largest of the tables

             .. note::
                Otherwise the ownership rules are the same as #hashes: a ``shared_ptr``, shared on
                copy
             @endrst
             */
            std::shared_ptr<VGRemaps> vgRemaps;

            /**
             * @brief
             @rst
             The builder that builds the parser for a ``.ini`` file of this type of mod
             :raw-html:`<br />` :raw-html:`<br />`

             A *factory*, not a parser, matching the pure-Python original's own
             ``iniParseBuilder`` attribute: :cpp:func:`IniFile::parse` builds a **fresh** parser per
             ``.ini`` file from it, passing that file's :cpp:member:`IniFile::version` along, so a
             mod type can use a different parser for a 4.0-era ``.ini`` file than for a 5.7-era one
             -- see :cpp:class:`IniParseBuilder` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This used to hold one shared :cpp:class:`BaseIniParser` instance that every
                ``.ini`` file of this mod type rebound to itself via
                :cpp:func:`BaseIniParser::setIniFile`. Holding a builder instead removes both
                problems that caused: the built parser now arrives already bound to its own file,
                and two :cpp:class:`IniFile`\\s of the same mod type no longer stomp each other's
                binding

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

             A *factory*, not a fixer, matching the pure-Python original's own ``iniFixBuilder``
             attribute: :cpp:func:`IniFile::fix` builds a **fresh** fixer per ``.ini`` file from it,
             passing that file's :cpp:member:`IniFile::version` along, so a mod type can use a
             different fixer for a 4.0-era ``.ini`` file than for a 5.7-era one -- see
             :cpp:class:`IniFixBuilder` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The built fixer arrives already bound to that file's own built parser (the original
                builds it as ``iniFixBuilder.build(self._iniParser, ...)``), so nothing rebinds it
                afterwards and two :cpp:class:`IniFile`\\s of the same mod type no longer stomp each
                other -- the same problem the #iniParseBuilder conversion removed on the parser side

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

             A *factory*, matching the pure-Python original's own ``iniRemoveBuilder`` attribute:
             :cpp:func:`IniFile::removeFix` asks it for a remover per ``.ini`` file, and it hands
             one back already bound to that file :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This behaves like #iniParseBuilder and #iniFixBuilder -- a fresh remover per
                :cpp:func:`IniRemoveBuilder::build`, bound to the caller's file and nobody else's.
                Its pure-Python original is a ``FlyweightBuilder`` that shares one instance instead;
                see :cpp:class:`IniRemoveBuilder`'s own warning for why that was not mirrored

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
             Compared case-insensitively against #name and every entry in #aliases, matching the
             pure-Python original's own ``isName``
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

             .. warning::
                **Deliberately not bug-compatible with the pure-Python original.** That one unions
                ``hashes.fixTo`` and ``indices.fixTo`` -- two sets it declares and then never
                populates anywhere, so it returns an empty set for every mod type, always. This
                reads the remap targets that actually exist
                (:cpp:func:`ModMappedAssets::getMap`), which is what the name promises
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
             The component being fixed from. ``std::nullopt`` leaves the column unconstrained,
             matching how the pure-Python original simply omits the key
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
             * @brief The help text describing this mod type, as the CLI prints it
             *
             * @return The help text
             */
            std::string getHelpStr() const;

            /**
             * @brief
             @rst
             Fixes a ``.ini`` file, but **only if that file was classified as this mod type** --
             a no-op otherwise, exactly as the pure-Python original is :raw-html:`<br />`
             :raw-html:`<br />`

             Returns nothing, also matching the original: the fix it produces is written out by
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
