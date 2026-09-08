#ifndef AGRemapCore_GameType_H
#define AGRemapCore_GameType_H

#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Name lookups for :cpp:enum:`GameTypeId`, in both directions :raw-html:`<br />`
     :raw-html:`<br />`

     Every name/alias a :cpp:enum:`GameTypeId` answers to lives in this class's constructor, and
     :cpp:class:`GameTypeIdTools` is the public way in -- see :cpp:func:`GameTypeIdTools::getName`,
     :cpp:func:`GameTypeIdTools::getAliases` and :cpp:func:`GameTypeIdTools::findByName`, all of
     which read a single process-wide instance of this class
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
             The other names each :cpp:enum:`GameTypeId` also answers to :raw-html:`<br />`
             :raw-html:`<br />`

             Mirrors :cpp:member:`ModType::aliases` -- a game a user could reasonably name more than
             one way (``"GI"``/``"Genshin"``/``"GenshinImpact"``) is reachable by all of them.
             Every entry here is also in #dfa
             @endrst
             */
            std::unordered_map<GameTypeId, std::vector<std::string>> aliases;

            /**
             * @brief
             @rst
             Every :cpp:enum:`GameTypeId`, in declaration order :raw-html:`<br />`
             :raw-html:`<br />`

             Declaration order rather than #names's (unordered) iteration order, so anything listing
             the supported games -- the CLI's ``--help`` epilog above all -- prints them the same way
             every run
             @endrst
             */
            std::vector<GameTypeId> order;

            /**
             * @brief
             @rst
             The `DFA`_ used to search for a :cpp:enum:`GameTypeId` by name using `Aho-Corasick`_
             :raw-html:`<br />` :raw-html:`<br />`

             Holds both #names and #aliases, each **lowercased**, matching how
             ``ModTypeIdTools::registerModType`` files a :cpp:class:`ModType`'s own names -- see
             :cpp:func:`GameTypeIdTools::findByName`, which lowercases what it is asked
             @endrst
             */
            BaseAhoCorasickDFA<GameTypeId> dfa;
    };
}

#endif
