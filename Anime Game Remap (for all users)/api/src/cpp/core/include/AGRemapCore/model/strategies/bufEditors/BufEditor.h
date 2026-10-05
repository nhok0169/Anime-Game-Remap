#ifndef AGRemapCore_BufEditor_H
#define AGRemapCore_BufEditor_H

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
#include <vector>

#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/strategies/bufEditors/BaseBufEditor.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseBufEditor`

     Class to edit some ``.buf`` file by running a fixed sequence of :cpp:type:`BufFile::Filter`\\s
     over it
     @endrst
     */
    class BufEditor: public BaseBufEditor {
        public:

            /**
             * @brief The filters used to edit the data for each line in the ``.buf`` file, applied in order
             */
            std::vector<BufFile::Filter> filters;

            /**
             * @brief Constructs a new ``.buf`` file editor
             *
             * @param filters The filters used to edit the data for each line in the ``.buf`` file
             */
            explicit BufEditor(std::vector<BufFile::Filter> filters = {});

            /**
             * @brief
             @rst
             Edits the ``.buf`` file, by calling :cpp:func:`BufFile::fix` with #filters
             @endrst
             *
             * @param bufFile The binary ``.buf`` file to be modified
             * @param fixedBufFile The name of the fixed ``.buf`` file. If this is ``std::nullopt``,
             *      the fixed bytes are returned directly instead of being written to a file
             *
             * @return If 'fixedBufFile' is ``std::nullopt``, the fixed bytes. Otherwise, 'fixedBufFile' itself
             */
            BufFile::FixResult fix(BufFile &bufFile, const std::optional<std::string> &fixedBufFile = std::nullopt) override;
    };
}

#endif
