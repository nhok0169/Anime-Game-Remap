#ifndef AGRemapCore_IncIdGenerator_H
#define AGRemapCore_IncIdGenerator_H

#include "AGRemapCore/tools/idGenerator/BaseIdGenerator.h"


namespace AGRemapCore {

    /**
     * @brief 
     * This class inherits from \ref BaseIdGenerator
     * 
     * A simple id generator that generates ids based
     * incrementing the current id
     * 
     * @tparam Id
     *      The type for the generated id
     * 
     * @note The type for the generated id must support the post-increment operator
     */
    template <typename Id>
    class IncIdGenerator: public BaseIdGenerator<Id> {
        public:

            /**
             * @brief Constructs a new generator
             * 
             * @param defaultId The default id the generator will start off with
             */
            IncIdGenerator(const Id &defaultId);

            void reset() override;
            bool getId(Id &result) override;

        protected:
            /**
             * @brief The internal id for the current id to generate
             */
            Id currentId;

            /**
             * @brief The default id the generator starts off with
             */
            Id defaultId;
    };
}

#include "IncIdGenerator.tpp"

#endif