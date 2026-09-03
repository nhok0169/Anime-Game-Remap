#ifndef AGRemapCore_BaseBufEditor_H
#define AGRemapCore_BaseBufEditor_H

#include <optional>
#include <string>

#include "AGRemapCore/model/files/BufFile.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class to edit some ``.buf`` file
     @endrst
     */
    class BaseBufEditor {
        public:
            virtual ~BaseBufEditor() = default;

            /**
             * @brief Edits the ``.buf`` file. No-op by default
             *
             * @param bufFile The binary ``.buf`` file to be modified
             * @param fixedBufFile The name of the fixed ``.buf`` file. If this is ``std::nullopt``,
             *      the fixed bytes are returned directly instead of being written to a file
             *
             * @return If 'fixedBufFile' is ``std::nullopt``, the fixed bytes. Otherwise, 'fixedBufFile' itself
             */
            virtual BufFile::FixResult fix(BufFile &bufFile, const std::optional<std::string> &fixedBufFile = std::nullopt);
    };
}

#endif
