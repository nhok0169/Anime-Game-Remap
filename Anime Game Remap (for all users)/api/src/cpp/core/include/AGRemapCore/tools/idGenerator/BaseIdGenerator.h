#ifndef AGRemapCore_BaseIdGenerator_H
#define AGRemapCore_BaseIdGenerator_H

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