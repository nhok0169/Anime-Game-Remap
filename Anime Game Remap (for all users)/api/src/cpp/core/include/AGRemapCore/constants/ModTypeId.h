#ifndef AGRemapCore_ModTypeId_H
#define AGRemapCore_ModTypeId_H

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

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
        XingqiuBamboo
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

        private:
            static std::unordered_map<int, ModType> _modTypes;

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
