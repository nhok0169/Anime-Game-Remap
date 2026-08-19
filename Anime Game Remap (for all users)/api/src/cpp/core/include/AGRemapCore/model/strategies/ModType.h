#ifndef AGRemapCore_ModType_H
#define AGRemapCore_ModType_H


namespace AGRemapCore {

    /**
     * @brief Class for defining a type of mod
     */
    class ModType {
        public:

            /**
             * @brief Constructs a new type of mod
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
            explicit ModType(int gameTypeId, int modTypeId);

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
