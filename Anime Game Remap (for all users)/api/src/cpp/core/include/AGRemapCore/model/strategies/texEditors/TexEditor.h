#ifndef AGRemapCore_TexEditor_H
#define AGRemapCore_TexEditor_H

#include <functional>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"

namespace AGRemapCore {

    class TextureFile;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexEditor`

     Class for editing a texture file by running a fixed sequence of filters over it
     @endrst
     */
    class TexEditor: public BaseTexEditor {
        public:

            /**
             * @brief A single edit applied to a :cpp:class:`TextureFile` by #fix
             */
            using Filter = std::function<void(TextureFile&)>;

            /**
             * @brief Constructs a new texture editor
             *
             * @param filters The filters for editing the image, applied in order
             */
            explicit TexEditor(std::vector<Filter> filters = {});

            /**
             * @brief The filters for editing the image
             */
            const std::vector<Filter>& getFilters() const;

            /**
             * @brief Sets #getFilters
             */
            void setFilters(std::vector<Filter> filters);

            /**
             * @brief
             @rst
             Edits the texture file :raw-html:`<br />` :raw-html:`<br />`

             No-op if #getFilters is empty, or if 'texFile' does not exist on disk. Otherwise, opens
             'texFile', runs every filter in #getFilters over it in order, then saves it to
             'fixedTexFile'
             @endrst
             *
             * @param texFile The texture ``.dds`` file to be modified
             * @param fixedTexFile The name of the fixed texture file
             */
            void fix(TextureFile &texFile, const std::string &fixedTexFile) override;

        private:
            std::vector<Filter> filters_;
    };
}

#endif
