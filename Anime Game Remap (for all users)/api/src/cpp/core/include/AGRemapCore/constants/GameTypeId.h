#ifndef AGRemapCore_GameTypeId_H
#define AGRemapCore_GameTypeId_H

#include <optional>
#include <string>


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
    };
}

#endif
