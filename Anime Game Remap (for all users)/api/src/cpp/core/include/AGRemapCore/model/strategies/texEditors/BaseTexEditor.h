#ifndef AGRemapCore_BaseTexEditor_H
#define AGRemapCore_BaseTexEditor_H

#include <string>

namespace AGRemapCore {

    class TextureFile;

    /**
     * @brief
     @rst
     Base class to edit some ``.dds`` file
     @endrst
     */
    class BaseTexEditor {
        public:
            virtual ~BaseTexEditor() = default;

            /**
             * @brief Edits the texture file. No-op by default
             *
             * @param texFile The texture ``.dds`` file to be modified
             * @param fixedTexFile The name of the fixed texture file
             */
            virtual void fix(TextureFile &texFile, const std::string &fixedTexFile);
    };
}

#endif
