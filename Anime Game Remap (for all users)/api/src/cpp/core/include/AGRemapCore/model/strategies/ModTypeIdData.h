#ifndef AGRemapCore_ModTypeIdData_H
#define AGRemapCore_ModTypeIdData_H


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Cheap data for a type of mod, held by an ini classifier (e.g. :cpp:class:`IniClassifier`)
     :raw-html:`<br />` :raw-html:`<br />`

     Not meant to be a full representation of a mod type on its own -- the Python-side ``ModType``
     is meant to build its own richer representation from this data
     @endrst
     */
    class ModTypeIdData {
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
             */
            explicit ModTypeIdData(int gameTypeId, int modTypeId);

            /**
             * @brief The id for the game this type of mod belongs to
             */
            int gameTypeId;

            /**
             * @brief The id for this specific type of mod
             */
            int modTypeId;
    };
}

#endif
