#ifndef AGRemapCore_GraphVarApply_H
#define AGRemapCore_GraphVarApply_H
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

#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/GraphEditVariables.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit` and :cpp:class:`IParseResult`

     Makes a variable of the parse from others: when the parser files it, the variables named #inputs of the parser's
     :cpp:func:`IniParseData::variables` are handed to #fn, and what it returns replaces the variable #into -- see
     :cpp:func:`GraphEditVariables::apply` :raw-html:`<br />` :raw-html:`<br />`

     Put it in :cpp:member:`GIMIParser::commandEdits` after the edits
     that file its inputs (a :cpp:class:`GraphCollect` files its rows in the variable named after it). It changes no
     graph; it reads only variables, so anything a graph says is collected into one first.

     In a FIXER's list, give it the fixer's own :cpp:class:`GraphEditVariables` (#variables): the function then runs
     when the edit does, over those variables, so a decision of the fix is a step of its list that the edits after it
     read
     @endrst
     */
    class GraphVarApply: public BaseIniGraphGroupEdit<>, public IParseResult {
        public:
            /**
             * @brief Constructs the edit
             *
             * @param inputs See #inputs
             * @param fn See #fn
             * @param into See #into
             */
            GraphVarApply(std::vector<std::string> inputs, GraphEditVariables::Apply fn, std::string into);

            /**
             * @brief Constructs the edit for a fixer's list, over 'variables'
             * @param variables See #variables
             * @param inputs See #inputs
             * @param fn See #fn
             * @param into See #into
             */
            GraphVarApply(GraphEditVariables& variables, std::vector<std::string> inputs, GraphEditVariables::Apply fn,
                          std::string into);

            /**
             * @brief
             @rst
             The variables a fixer's list keeps, which the edit applies #fn over when it runs; ``nullptr`` (the
             default) for a parser's, where the parser files it (:cpp:func:`fileInto`)
             @endrst
             */
            GraphEditVariables* variables = nullptr;

            /**
             * @brief The names of the variables read, in the order #fn is given them
             */
            std::vector<std::string> inputs;

            /**
             * @brief The function making the new value of #into -- see :cpp:type:`GraphEditVariables::Apply`
             */
            GraphEditVariables::Apply fn;

            /**
             * @brief The name of the variable written
             */
            std::string into;

            /**
             * @brief Changes no graph: applies #fn over #variables, when there are any
             *
             * @param graphGroups The groups of graphs
             * @param modType Unused
             * @param modName Unused
             *
             * @return 'graphGroups', unchanged
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

            /**
             * @brief Applies #fn to the variables of 'data' -- see this class's own description. Nothing when the edit has
             *    #variables of its own, where #edit has applied it
             *
             * @param data The parser's data
             */
            void fileInto(IniParseData& data) const override;
    };
}

#endif
