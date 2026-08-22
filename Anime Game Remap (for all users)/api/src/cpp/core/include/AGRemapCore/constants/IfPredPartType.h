#ifndef AGRemapCore_IfPredPartType_H
#define AGRemapCore_IfPredPartType_H

#include <optional>
#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The possible types for an :cpp:class:`IfPredPart` :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``IfPredPartType`` enum (``constants/IfPredPartType.py``)
     @endrst
     */
    enum class IfPredPartType {
        /**
         * @brief The part contains the starting keyword 'if'
         */
        If,

        /**
         * @brief The part contains the starting keyword 'else'
         */
        Else,

        /**
         * @brief The part contains the starting keyword 'elif'
         */
        Elif,

        /**
         * @brief The part contains the starting keyword 'endif'
         */
        EndIf
    };

    /**
     * @brief Tools for handling :cpp:enum:`IfPredPartType`
     */
    class IfPredPartTypeTools {
        public:

            /**
             * @brief The keyword text for a :cpp:enum:`IfPredPartType` (eg. ``IfPredPartType::EndIf``
             *      -> ``"endif"``) -- the C++ counterpart to the pure-Python enum member's own
             *      ``.value``
             *
             * @param value The :cpp:enum:`IfPredPartType` to retrieve the keyword for
             *
             * @return The keyword text for 'value'
             */
            static std::string getName(IfPredPartType value);

            /**
             * @brief
             @rst
             Retrieves the :cpp:enum:`IfPredPartType` for an :cpp:class:`IfPredPart` :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Matches by a case-insensitive *prefix* check only, with no word-boundary
                requirement after the keyword (eg. ``"iffy ..."`` still classifies as
                :cpp:enumerator:`IfPredPartType::If`) -- this is a deliberate (if permissive) match
                to the pure-Python original's own equally permissive ``str.startswith`` checks, not
                a bug to tighten up here
             @endrst
             *
             * @param rawPredPart The predicate string for the :cpp:class:`IfPredPart`
             *
             * @return The type found based off 'rawPredPart', if any
             */
            static std::optional<IfPredPartType> getType(const std::string& rawPredPart);
    };
}

#endif
