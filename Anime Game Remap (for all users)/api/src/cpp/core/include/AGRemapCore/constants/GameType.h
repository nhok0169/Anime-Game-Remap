#ifndef AGRemapCore_GameType_H
#define AGRemapCore_GameType_H

#include <string>
#include <unordered_map>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Name lookups for :cpp:enum:`GameTypeId`, in both directions
     @endrst
     */
    class GameType {
        public:

            /**
             * @brief Constructs the name lookups for :cpp:enum:`GameTypeId`
             */
            GameType();

            /**
             * @brief The name for each :cpp:enum:`GameTypeId`
             */
            std::unordered_map<GameTypeId, std::string> names;

            /**
             * @brief
             @rst
             The `DFA`_ used to search for a :cpp:enum:`GameTypeId` by name using `Aho-Corasick`_
             @endrst
             */
            BaseAhoCorasickDFA<GameTypeId> dfa;
    };
}

#endif
