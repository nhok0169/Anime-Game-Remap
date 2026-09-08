#ifndef AGRemapCore_GameTypeId_H
#define AGRemapCore_GameTypeId_H

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief The names of the different supported games
     */
    enum class GameTypeId {
        /**
         * @brief Genshin Impact
         */
        GI,

        /**
         * @brief Wuthering Waves
         */
        WuWa
    };

    /**
     * @brief
     @rst
     A set of :cpp:enum:`GameTypeId`\\s to narrow something down to, where ``std::nullopt`` means
     "every game" :raw-html:`<br />` :raw-html:`<br />`

     The shape the whole classification path takes its game filter in --
     :cpp:member:`RemapService::gameTypeIds` down through :cpp:class:`IniFile` to
     :cpp:func:`BaseIniClassifier::classify` :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        An **empty** set is a filter that matches nothing, not "every game" -- the same reading
        :cpp:member:`RemapService::fromModTypeIds` has. ``std::nullopt`` is the only way to say
        "no filter". :cpp:class:`RemapServiceCLI` is where a user naming no games at all gets
        turned into ``std::nullopt``, since that is the ambiguity an argument parser creates
     @endrst
     */
    using GameTypeIdFilter = std::optional<std::unordered_set<GameTypeId>>;

    /**
     * @brief Tools for handling :cpp:enum:`GameTypeId`
     */
    class GameTypeIdTools {
        public:

            /**
             * @brief
             @rst
             Retrieves the corresponding :cpp:enum:`GameTypeId` for some integer value, checking
             that the value actually corresponds to one of :cpp:enum:`GameTypeId`'s declared values
             @endrst
             *
             * @param value The integer value to convert
             *
             * @return The corresponding :cpp:enum:`GameTypeId`, if 'value' is valid
             */
            static std::optional<GameTypeId> getEnum(int value);

            /**
             * @brief
             @rst
             Retrieves the corresponding name for a :cpp:enum:`GameTypeId` :raw-html:`<br />` :raw-html:`<br />`

             Mirrors the pure-Python ``GameTypeNames`` enum's values (``constants/GameTypeNames.py``)
             @endrst
             *
             * @param value The :cpp:enum:`GameTypeId` to retrieve the name for
             *
             * @return The name for 'value'
             */
            static std::string getName(GameTypeId value);

            /**
             * @brief
             @rst
             Retrieves the other names a :cpp:enum:`GameTypeId` also answers to :raw-html:`<br />`
             :raw-html:`<br />`

             Mirrors :cpp:member:`ModType::aliases` -- every one of these resolves through
             :cpp:func:`findByName` exactly as :cpp:func:`getName`'s answer does
             @endrst
             *
             * @param value The :cpp:enum:`GameTypeId` to retrieve the aliases for
             *
             * @return The aliases for 'value', empty if it has none
             */
            static const std::vector<std::string>& getAliases(GameTypeId value);

            /**
             * @brief
             @rst
             Every :cpp:enum:`GameTypeId`, in declaration order :raw-html:`<br />`
             :raw-html:`<br />`

             The order is stable across runs, so anything listing the supported games (the CLI's
             ``--help`` epilog) prints them the same way every time
             @endrst
             *
             * @return All the supported games
             */
            static const std::vector<GameTypeId>& getAll();

            /**
             * @brief
             @rst
             Finds the :cpp:enum:`GameTypeId` whose name or alias maximally matches some string --
             the :cpp:enum:`GameTypeId` counterpart of :cpp:func:`ModTypeIdTools::findByName`, and
             what turns what a user typed for the CLI's game option into the ids the model wants
             :raw-html:`<br />` :raw-html:`<br />`

             Case and surrounding whitespace are ignored, matching how a mod type's name resolves
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike :cpp:func:`ModTypeIdTools::findByName` there is no registry to consult and
                nothing to register: the games are the :cpp:enum:`GameTypeId` enumerators
                themselves, so every one of them is always findable
             @endrst
             *
             * @param name The string to search for a game's name/alias within
             *
             * @return The matched :cpp:enum:`GameTypeId`, if 'name' names one
             */
            static std::optional<GameTypeId> findByName(const std::string& name);

            /**
             * @brief
             @rst
             The `--help` text describing a single :cpp:enum:`GameTypeId` -- its name and its
             aliases :raw-html:`<br />` :raw-html:`<br />`

             Mirrors :cpp:func:`ModType::getHelpStr`, so the CLI's list of supported games reads
             exactly like its list of supported mods
             @endrst
             *
             * @param value The :cpp:enum:`GameTypeId` to describe
             *
             * @return The help text for 'value'
             */
            static std::string getHelpStr(GameTypeId value);
    };
}

#endif
