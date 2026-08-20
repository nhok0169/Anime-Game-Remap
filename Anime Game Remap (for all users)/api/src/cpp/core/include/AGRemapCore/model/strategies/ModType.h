#ifndef AGRemapCore_ModType_H
#define AGRemapCore_ModType_H

#include <string>


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
             * @param modTypeId See :cpp:func:`setModTypeId`
             */
            explicit ModType(int gameTypeId, int modTypeId);

            /**
             * @brief The id for the game this type of mod belongs to
             */
            int gameTypeId;

            /**
             * @brief Getter for #modTypeId
             *
             * @return The id for this specific type of mod
             */
            int getModTypeId() const;

            /**
             * @brief
             @rst
             Setter for #modTypeId -- stored as-is, with no validation that it corresponds to one
             of :cpp:enum:`ModTypeId`'s declared values, so a custom mod type using some id not
             registered in :cpp:enum:`ModTypeId` can still be represented :raw-html:`<br />` :raw-html:`<br />`

             Also updates #name: if the new value corresponds to one of :cpp:enum:`ModTypeId`'s
             declared values, #name is set to that value's name (see :cpp:class:`ModTypeIdTools`);
             otherwise, #name is set to the empty string
             @endrst
             *
             * @param modTypeId The new id for this specific type of mod
             */
            void setModTypeId(int modTypeId);

            /**
             * @brief
             @rst
             The name for this type of mod :raw-html:`<br />` :raw-html:`<br />`

             Kept in sync with #modTypeId by :cpp:func:`setModTypeId`: if #modTypeId corresponds to
             one of :cpp:enum:`ModTypeId`'s declared values, this is that value's name (see
             :cpp:class:`ModTypeIdTools`); otherwise, this is the empty string
             @endrst
             */
            std::string name;

        protected:

            /**
             * @brief The id for this specific type of mod
             */
            int modTypeId;
    };
}

#endif
