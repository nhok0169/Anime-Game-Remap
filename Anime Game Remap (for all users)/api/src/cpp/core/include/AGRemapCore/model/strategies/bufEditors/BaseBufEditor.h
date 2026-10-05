#ifndef AGRemapCore_BaseBufEditor_H
#define AGRemapCore_BaseBufEditor_H

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
