#ifndef AGRemapCore_BaseTexFilter_H
#define AGRemapCore_BaseTexFilter_H

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
