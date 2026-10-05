#ifndef AGRemapCore_BaseTexFilter_H
#define AGRemapCore_BaseTexFilter_H

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

    class TextureFile;

    /**
     * @brief
     @rst
     Base class for transforming a texture file
     @endrst
     */
    class BaseTexFilter {
        public:
            virtual ~BaseTexFilter() = default;

            /**
             * @brief Calls #transform for this filter
             *
             * @param texFile The texture to be edited
             */
            void operator()(TextureFile &texFile);

            /**
             * @brief Applies a transformation to 'texFile'. No-op by default
             *
             * @param texFile The texture to be edited
             */
            virtual void transform(TextureFile &texFile);
    };
}

#endif
