#ifndef AGRemapCore_IncIdGenerator_H
#define AGRemapCore_IncIdGenerator_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

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