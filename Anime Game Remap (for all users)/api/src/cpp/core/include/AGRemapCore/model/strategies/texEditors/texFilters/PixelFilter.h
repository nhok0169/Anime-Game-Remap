#ifndef AGRemapCore_PixelFilter_H
#define AGRemapCore_PixelFilter_H

#include <functional>
#include <vector>

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Manipulates each pixel within an image, by running a fixed sequence of per-pixel transforms
     over every pixel of the texture :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Every whole-image :cpp:class:`BaseTexFilter` in this codebase (eg.
        :cpp:class:`ColourReplaceFilter`) is, at this level, also just a C++ loop over every pixel
        -- `Compressonator`_ has no vectorized whole-image pixel-remap API the way `Pillow`_ did for
        the pure-Python original, so there's no longer a real "whole image at once" fast path to
        prefer instead. A #Filter is called directly, in C++, for every pixel -- no different in
        cost from a dedicated filter's own inlined loop body
     @endrst
     */
    class PixelFilter: public BaseTexFilter {
        public:

            /**
             * @brief A single per-pixel transform applied by #transform
             */
            using Filter = std::function<void(Colour&, int, int)>;

            /**
             * @brief Constructs a new pixel filter
             *
             * @param transforms The functions to edit a single pixel in the texture file, applied
             *      in order to every pixel
             */
            explicit PixelFilter(std::vector<Filter> transforms = {});

            /**
             * @brief The functions to edit a single pixel in the texture file
             */
            const std::vector<Filter>& getTransforms() const;

            /**
             * @brief Sets #getTransforms
             */
            void setTransforms(std::vector<Filter> transforms);

            void transform(TextureFile &texFile) override;

        private:
            std::vector<Filter> transforms_;
    };
}

#endif
