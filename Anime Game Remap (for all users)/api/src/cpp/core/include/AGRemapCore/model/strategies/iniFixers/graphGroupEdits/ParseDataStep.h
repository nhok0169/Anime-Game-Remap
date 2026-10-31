#ifndef AGRemapCore_ParseDataStep_H
#define AGRemapCore_ParseDataStep_H
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

#include <functional>
#include <string>
#include <utility>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit` and :cpp:class:`IParseResult`

     A parse step that edits nothing and, when the parser files it, runs a function on the parser's
     :cpp:class:`IniParseData` :raw-html:`<br />` :raw-html:`<br />`

     Put it in :cpp:member:`GIMIParser::parseSteps` or :cpp:member:`GIMIParser::deferredParseSteps` to read what the
     steps before it filed -- the rows of a :cpp:class:`GraphCollect`, say -- and file a conclusion of your own,
     in the order the steps run. A later step sees what this one filed
     @endrst
     */
    class ParseDataStep: public BaseIniGraphGroupEdit<>, public IParseResult {
        public:
            /**
             * @brief The function run on the parser's data when the step is filed
             */
            using File = std::function<void(IniParseData&)>;

            /**
             * @brief Constructs a new step
             *
             * @param file See \ref file
             */
            explicit ParseDataStep(File file): file(std::move(file)) {}

            /**
             * @brief The function run on the parser's data when the step is filed; nothing runs when it is empty
             */
            File file;

            /**
             * @brief Edits nothing
             *
             * @param graphGroups The groups of graphs
             *
             * @return 'graphGroups', unchanged
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType*, const std::string& = "") override {
                return graphGroups;
            }

            /**
             * @brief Runs \ref file on 'data'
             *
             * @param data The parser's data
             */
            void fileInto(IniParseData& data) const override {
                if (file) {
                    file(data);
                }
            }
    };
}

#endif
