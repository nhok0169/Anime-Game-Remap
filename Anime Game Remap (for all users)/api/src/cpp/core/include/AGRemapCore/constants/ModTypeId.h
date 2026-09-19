#ifndef AGRemapCore_ModTypeId_H
#define AGRemapCore_ModTypeId_H

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

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The names of the different types of mods this fix will fix from or fix to :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the keys of the pure-Python ``ModTypeNames`` enum (``constants/ModTypeNames.py``)
     @endrst
     */
    enum class ModTypeId {
        /**
         * @brief Amber from GI
         */
        Amber,

        /**
         * @brief Amber Chinese version from GI
         */
        AmberCN,

        /**
         * @brief Ayaka from GI
         */
        Ayaka,

        /**
         * @brief Ayaka Fontaine skin from GI
         */
        AyakaSpringbloom,

        /**
         * @brief Arlecchino from GI
         */
        Arlecchino,

        /**
         * @brief The first phase of the Arlecchino boss from GI
         */
        ArlecchinoBoss,

        /**
         * @brief Barbara from GI
         */
        Barbara,

        /**
         * @brief Barbara summer skin from GI
         */
        BarbaraSummertime,

        /**
         * @brief Bennett from GI
         */
        Bennett,

        /**
         * @brief Bennett outfit skin (Adventure) from GI -- THREE components (Body, Bang, Eye)
         */
        BennettAdventure,

        /**
         * @brief
         @rst
         BennettAdventure's ``Body`` component, as a fix TARGET :raw-html:`<br />` :raw-html:`<br />`

         The same arrangement as :cpp:enumerator:`YelanTranquilBody`, whose comment has the reasoning:
         a skin of several components is fixed by one fixer per component, and the tables those
         fixers read -- :cpp:class:`IniFixBuilderData`, :cpp:class:`HashData`,
         :cpp:class:`IndexData` -- are keyed by a mod type NAME, so each component is a mod type of
         its own for their purposes. Nothing classifies a ``.ini`` file AS one of these; the skin
         itself is :cpp:enumerator:`BennettAdventure`, whose vertex-group rows are keyed by component
         @endrst
         */
        BennettAdventureBody,

        /**
         * @brief BennettAdventure's ``Bang`` component, as a fix target -- see :cpp:enumerator:`BennettAdventureBody`
         */
        BennettAdventureBang,

        /**
         * @brief BennettAdventure's ``Eye`` component, as a fix target -- see :cpp:enumerator:`BennettAdventureBody`
         */
        BennettAdventureEye,

        /**
         * @brief Hu Tao Lantern Rite skin from GI
         */
        CherryHuTao,

        /**
         * @brief Diluc from GI
         */
        Diluc,

        /**
         * @brief Diluc Red Dead of the Night skin from GI
         */
        DilucFlamme,

        /**
         * @brief Fischl from GI
         */
        Fischl,

        /**
         * @brief Fischl summer skin from GI
         */
        FischlHighness,

        /**
         * @brief Ganyu from GI
         */
        Ganyu,

        /**
         * @brief Ganyu Lantern Rite skin from GI
         */
        GanyuTwilight,

        /**
         * @brief HuTao from GI
         */
        HuTao,

        /**
         * @brief Jean from GI
         */
        Jean,

        /**
         * @brief Jean Chinese version from GI
         */
        JeanCN,

        /**
         * @brief Jean summer skin from GI
         */
        JeanSea,

        /**
         * @brief Kaeya from GI
         */
        Kaeya,

        /**
         * @brief KaeyaSailwind from GI
         */
        KaeyaSailwind,

        /**
         * @brief Keqing from GI
         */
        Keqing,

        /**
         * @brief Keqing Lantern Rite skin from GI
         */
        KeqingOpulent,

        /**
         * @brief Kirara from GI
         */
        Kirara,

        /**
         * @brief Kirara summer skin from GI
         */
        KiraraBoots,

        /**
         * @brief Klee from GI
         */
        Klee,

        /**
         * @brief Klee summer skin from GI
         */
        KleeBlossomingStarlight,

        /**
         * @brief Lisa from GI
         */
        Lisa,

        /**
         * @brief Lisa Sumeru skin from GI
         */
        LisaStudent,

        /**
         * @brief Mona from GI
         */
        Mona,

        /**
         * @brief Mona Chinese version from GI
         */
        MonaCN,

        /**
         * @brief Nilou from GI
         */
        Nilou,

        /**
         * @brief Nilou summer skin from GI
         */
        NilouBreeze,

        /**
         * @brief Ningguang from GI
         */
        Ningguang,

        /**
         * @brief Ningguang Lantern Rite from GI
         */
        NingguangOrchid,

        /**
         * @brief Ei from GI
         */
        Raiden,

        /**
         * @brief The first phase of the Raiden Shogun boss from GI
         */
        RaidenBoss,

        /**
         * @brief Rosaria from GI
         */
        Rosaria,

        /**
         * @brief Rosaria Chinese version from GI
         */
        RosariaCN,

        /**
         * @brief Shenhe from GI
         */
        Shenhe,

        /**
         * @brief Shenhe Lantern Rite skin from GI
         */
        ShenheFrostFlower,

        /**
         * @brief Xiangling from GI
         */
        Xiangling,

        /**
         * @brief Xiangling Lantern Rite skin from GI
         */
        XianglingCheer,

        /**
         * @brief Xingqiu from GI
         */
        Xingqiu,

        /**
         * @brief Xingqiu Lantern Rite skin from GI
         */
        XingqiuBamboo,

        /**
         * @brief Yelan from GI
         */
        Yelan,

        /**
         * @brief Yelan summer skin (Tranquil Banquet) from GI -- THREE components (Body, Bang, Eye)
         */
        YelanTranquil,

        /**
         * @brief
         @rst
         YelanTranquil's ``Body`` component, as a fix TARGET :raw-html:`<br />` :raw-html:`<br />`

         A skin of several components is fixed by one fixer per component (see
         :cpp:func:`makeGIMIComponentFixer`), and the tables the fixer reads --
         :cpp:class:`IniFixBuilderData`, :cpp:class:`HashData`, :cpp:class:`IndexData` -- are keyed
         by a mod type NAME. So each component is a mod type of its own for those tables' purposes:
         ``YelanTranquilBody`` carries the Body's hashes and slot indices, and Yelan remaps onto it.
         Like the boss ids, nothing classifies a ``.ini`` file AS one of these; the skin itself is
         :cpp:enumerator:`YelanTranquil`, whose vertex-group rows the fixer reads by component
         @endrst
         */
        YelanTranquilBody,

        /**
         * @brief YelanTranquil's ``Bang`` component, as a fix target -- see :cpp:enumerator:`YelanTranquilBody`
         */
        YelanTranquilBang,

        /**
         * @brief YelanTranquil's ``Eye`` component, as a fix target -- see :cpp:enumerator:`YelanTranquilBody`
         */
        YelanTranquilEye,

        /**
         * @brief
         @rst
         Sanhua from WuWa (Wuthering Waves) -- the first WWMI mod type (2026-09-19) :raw-html:`<br />` :raw-html:`<br />`

         A WWMI character is drawn as several ``Component N`` slots of ONE vertex buffer sharing ONE
         merged skeleton (each slot's ``vg_map`` in WWMI-Assets' ``Metadata.json`` maps its bones into
         it), so unlike :cpp:enumerator:`YelanTranquil` it is a single-component mod type whose
         slots are the ``type`` column of :cpp:class:`Indices`, :cpp:class:`IndexCounts`,
         :cpp:class:`VGOffsets` and :cpp:class:`VGCounts`. Built by :cpp:class:`WWMIBuilder`
         @endrst
         */
        Sanhua,

        /**
         * @brief Sanhua's Exorcist skin from WuWa (WWMI-Assets' ``SanhuaSkin1``) -- see :cpp:enumerator:`Sanhua`
         */
        SanhuaExorcist
    };

    /**
     * @brief Tools for handling :cpp:enum:`ModTypeId`
     */
    class ModTypeIdTools {
        public:

            /**
             * @brief
             @rst
             Retrieves the corresponding :cpp:enum:`ModTypeId` for some integer value, checking
             that the value actually corresponds to one of :cpp:enum:`ModTypeId`'s declared values
             @endrst
             *
             * @param value The integer value to convert
             *
             * @return The corresponding :cpp:enum:`ModTypeId`, if 'value' is valid
             */
            static std::optional<ModTypeId> getEnum(int value);

            /**
             * @brief
             @rst
             Retrieves the corresponding name for a :cpp:enum:`ModTypeId` :raw-html:`<br />` :raw-html:`<br />`

             Mirrors the pure-Python ``ModTypeNames`` enum's values (``constants/ModTypeNames.py``)
             @endrst
             *
             * @param value The :cpp:enum:`ModTypeId` to retrieve the name for
             *
             * @return The name for 'value'
             */
            static std::string getName(ModTypeId value);

            /**
             * @brief
             @rst
             Retrieves the :cpp:class:`ModType` registered for a :cpp:enum:`ModTypeId`, if one has
             been registered (via :cpp:func:`registerModType`) :raw-html:`<br />` :raw-html:`<br />`

             This is a plain lookup into a global registry shared by every caller of
             :cpp:class:`ModTypeIdTools` -- it never builds a :cpp:class:`ModType` itself. If a
             :cpp:enum:`ModTypeId` is never registered, nothing is ever built for it, since building
             one can be expensive; only a :cpp:enum:`ModTypeId` that's actually been registered
             (typically by whichever builder -- e.g. ``GIBuilder`` -- actually owns it) can be
             retrieved here
             @endrst
             *
             * @param modTypeId
             @rst
             The integer id for the :cpp:enum:`ModTypeId` to retrieve the registered
             :cpp:class:`ModType` for -- stored/looked-up as-is, with no validation that it
             corresponds to one of :cpp:enum:`ModTypeId`'s declared values, so a custom mod type
             using some id not registered in :cpp:enum:`ModTypeId` can still be looked up here
             @endrst
             *
             * @return The registered :cpp:class:`ModType`, if one exists for 'modTypeId'
             */
            static std::optional<ModType> getModType(int modTypeId);

            /**
             * @brief
             @rst
             Registers a :cpp:class:`ModType` into the global registry, under the
             :cpp:enum:`ModTypeId` it owns (``modType.modTypeId``) :raw-html:`<br />` :raw-html:`<br />`

             If a :cpp:class:`ModType` is already registered for that :cpp:enum:`ModTypeId`, it gets
             overwritten with the new one
             @endrst
             *
             * @param modType The :cpp:class:`ModType` to register
             */
            static void registerModType(const ModType &modType);

            /**
             * @brief
             @rst
             Finds the :cpp:enum:`ModTypeId` whose registered :cpp:class:`ModType` name or alias
             maximally matches some string, similar to how :cpp:func:`IniClassifier::readSectionName`
             searches ``sectionKeywordsDFA`` :raw-html:`<br />` :raw-html:`<br />`

             Only searches names/aliases of :cpp:class:`ModType` s that have actually been registered
             via :cpp:func:`registerModType` -- an unregistered :cpp:enum:`ModTypeId` can never be
             found this way, even if 'name' textually matches what :cpp:func:`getName` would return
             for it :raw-html:`<br />` :raw-html:`<br />`

             If more than one registered :cpp:enum:`ModTypeId` shares the maximally-matched name (or
             alias) -- after filtering by 'gameTypeId', when given -- the match is ambiguous and
             ``std::nullopt`` is returned rather than guessing
             @endrst
             *
             * @param name The string to search for a registered :cpp:class:`ModType` name/alias within
             * @param gameTypeId
             @rst
             If provided, only considers a :cpp:class:`ModType` registered under this
             :cpp:enum:`GameTypeId` (via ``modType.gameTypeId``) a candidate match
             @endrst
             *
             * @return The matched :cpp:enum:`ModTypeId`, if exactly one unambiguous match was found
             */
            static std::optional<ModTypeId> findByName(const std::string &name, std::optional<GameTypeId> gameTypeId = std::nullopt);

            /**
             * @brief
             @rst
             Clears the global registry -- every :cpp:class:`ModType` registered via
             :cpp:func:`registerModType` is forgotten, and :cpp:func:`getModType`/
             :cpp:func:`findByName` behave as if nothing was ever registered :raw-html:`<br />` :raw-html:`<br />`

             Mirrors ``HashTools``/``CppHashTools``'s own ``clear()`` -- meant for resetting shared
             global state between independent uses (e.g. between unit tests)
             @endrst
             */
            static void clear();

            /**
             * @brief
             @rst
             How many times the registry has been emptied by :cpp:func:`clear` :raw-html:`<br />`
             :raw-html:`<br />`

             Starts at ``1`` and is bumped by every :cpp:func:`clear`, so a caller that populates
             the registry can tell whether the registry it populated is still the one being read.
             That is what :cpp:func:`GlobalIniClassifiers::classifier` uses to know it has to
             re-file the shipped mod types: its own population used to be welded to a one-shot
             lazy initializer, so a :cpp:func:`clear` afterwards left it naming mod type ids that
             nothing could resolve, permanently, for the rest of the process :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Deliberately **not** bumped by :cpp:func:`registerModType` -- this counts
                *invalidations*, not writes. A caller registering a mod type is adding to the
                registry that already exists, not replacing it
             @endrst
             *
             * @return The current generation
             */
            static unsigned long long generation();

            /**
             * @brief
             @rst
             The mod types a given mod type's **hashes** can be remapped onto :raw-html:`<br />`
             :raw-html:`<br />`

             This is the remap graph itself -- what the software is for. It mirrors the ``map``
             argument the pure-Python ``GIBuilder`` passes to each mod type's ``Hashes``
             (``constants/GIBuilder.py``), lifted out of the 43 individual factories and into one
             table so a target can be named by :cpp:enum:`ModTypeId` rather than by a bare string

             .. note::
                Two :cpp:enum:`ModTypeId`\s -- ``RaidenBoss`` and ``ArlecchinoBoss`` -- appear only
                ever as *targets* here and are never a source, which is why
                :cpp:class:`GIBuilder` has no factory for them
             @endrst
             *
             * @param value The mod type to look up the remap targets of
             *
             * @return The mod types 'value' remaps onto, or an empty list if it remaps onto none
             */
            static std::vector<ModTypeId> getHashRemapTargets(ModTypeId value);

            /**
             * @brief
             @rst
             The mod types a given mod type's **indices** can be remapped onto :raw-html:`<br />`
             :raw-html:`<br />`

             Identical to :cpp:func:`getHashRemapTargets` for every mod type but one -- see that
             function for the shape, and this one's implementation for the exception
             @endrst
             *
             * @param value The mod type to look up the remap targets of
             *
             * @return The mod types 'value' remaps onto, or an empty list if it remaps onto none
             */
            static std::vector<ModTypeId> getIndexRemapTargets(ModTypeId value);

            /**
             * @brief
             @rst
             The **component** mod types a skin of several components is made of, or an empty list
             for the ordinary one-mesh mod type :raw-html:`<br />` :raw-html:`<br />`

             A skin like :cpp:enumerator:`YelanTranquil` draws out of three separate buffer sets,
             and the asset tables file its hashes under the COMPONENT names
             (``YelanTranquilBody`` / ``...Bang`` / ``...Eye``) rather than the skin's, because a
             fixer of the forward direction needs one row per component. Those component ids are a
             fix *target* in :cpp:func:`getHashRemapTargets`; this function is the other half of the
             same fact, and is what lets the component names be remap **sources** too

             .. note::
                This is not cosmetic. :cpp:func:`ModMappedAssets::replace` is reverse-then-forward,
                so remapping a several-component skin ONTO something resolves the source hash back
                to a component name and then asks the remap graph what that name maps to. Without
                these edges the forward half finds nothing and every ``hash`` in the output is
                written as ``HashNotFound`` -- a well-formed ``.ini`` file that triggers on nothing
                at all
             @endrst
             *
             * @param value The mod type to look up the components of
             *
             * @return The mod type's component ids, or an empty list if it is a single mesh
             */
            static std::vector<ModTypeId> getComponentIds(ModTypeId value);

            /**
             * @brief
             @rst
             The `section`_-name keywords that identify a mod type when classifying a ``.ini`` file
             :raw-html:`<br />` :raw-html:`<br />`

             Lowercased, and matched **maximally** (longest wins) by
             :cpp:func:`IniClassifier::readSectionName`. That is what disambiguates an overlapping
             pair without any extra machinery: a `section`_ named ``TextureOverrideAmberCNBody``
             matches ``ambercn`` rather than ``amber``, because the longer keyword wins

             .. note::
                The pure-Python ``IniClassifierBuilderOld`` reaches the same result a different
                way -- one compiled regex per keyword carrying a negative lookahead
                (``(amber)((?!cn).)*``). Only the keywords carry over; the regexes do not, because
                maximal matching already encodes what they were disambiguating

             Most mod types have exactly one keyword. Three carry a second alias-like spelling
             (``CherryHuTao``, ``Raiden``, ``XianglingCheer``), and the two target-only ids have
             none at all
             @endrst
             *
             * @param value The mod type to look up the `section`_-name keywords of
             *
             * @return The keywords identifying 'value', or an empty list if it has none
             */
            static std::vector<std::string> getSectionKeywords(ModTypeId value);

        private:
            static std::unordered_map<int, ModType> _modTypes;

            // Bumped by clear(). See generation()'s own note for what reads it and why.
            static unsigned long long _generation;

            // name/alias -> the ModTypeIds of every registered ModType sharing that exact name/alias
            // (game-agnostic; narrowed down to a specific GameTypeId, when requested, by
            // cross-referencing each candidate's own entry in '_modTypes')
            static BaseAhoCorasickDFA<std::unordered_set<int>> _nameDFA;

            // name/alias -> the GameTypeIds of every registered ModType sharing that exact
            // name/alias -- mirrors IniClassifier's 'keywordGameTypeIds', used the same way to build
            // a KeywordPredicate for '_nameDFA's maximal-match search
            static std::unordered_map<std::string, std::unordered_set<int>> _nameGameTypeIds;

            // Sets up '_nameDFA's duplicate-merging behavior exactly once (mirrors what
            // IniClassifier's constructor does for its own 'sectionKeywordsDFA') -- mutates
            // '_nameDFA' in place via 'setHandleDuplicate' rather than constructing-and-returning a
            // whole BaseAhoCorasickDFA by value, since that type holds a unique_ptr member and so
            // has no copy/move constructor to return through.
            static bool _setupNameDFA();
            static bool _nameDFAInitialized;
    };
}

#endif
