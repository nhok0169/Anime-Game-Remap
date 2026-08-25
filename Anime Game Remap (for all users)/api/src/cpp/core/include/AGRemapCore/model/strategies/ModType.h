#ifndef AGRemapCore_ModType_H
#define AGRemapCore_ModType_H

#include <string>
#include <vector>


namespace AGRemapCore {

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
             */
            explicit ModType(int gameTypeId, int modTypeId, const std::string &name, const std::vector<std::string> &aliases = {});

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
    };
}

#endif
