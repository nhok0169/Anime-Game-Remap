#ifndef AGRemapCore_BaseIdGenerator_H
#define AGRemapCore_BaseIdGenerator_H


namespace AGRemapCore {

    /**
     * @brief Base class for a generator that generates some ids
     * 
     * @tparam Id
     *      The type for the generated id
     */
    template <typename Id>
    class BaseIdGenerator {
        public:

            /**
             * @brief Destroys the generator
             */
            virtual ~BaseIdGenerator() = default;

            /**
             * @brief Resets the state of the generator
             */
            virtual void reset() = 0;

            /**
             * @brief Generates an id
             * 
             * @param result The reference to store the generated id
             * 
             * @return Whether the id generated successfully
             */
            virtual bool getId(Id &result) = 0;
    };
}

#endif