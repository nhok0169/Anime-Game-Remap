#ifndef AGRemapCore_GraphVarTake_H
#define AGRemapCore_GraphVarTake_H
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
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/GraphEditVariables.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Copies named `sections`_ of a graph in the first group into a sections variable of a
     :cpp:class:`GraphEditVariables`, and removes the graph :raw-html:`<br />` :raw-html:`<br />`

     The usual use: copy some of the mod's own sections into a graph of their own (:cpp:class:`GraphCopy`), rename and
     edit that graph, then take the result into the variable a :cpp:member:`GraphCreate::sectionsOf` writes out. The sections
     taken go after every section the variable holds when this edit runs -- or, with 'inPlace', where this edit was
     made: the place is held when the edit is constructed, so sections added to the variable after that come after
     them
     @endrst
     */
    class GraphVarTake: public BaseIniGraphGroupEdit<> {
        public:
            /**
             * @brief A mod object: ``(component, object)``
             */
            using ModObj = std::pair<std::string, std::string>;

            /**
             * @brief Constructs the edit
             *
             * @param variables The variables -- borrowed, they must outlive the edit
             * @param variable The name of the sections variable to take into
             * @param graph The graph, in the first group
             * @param sections The `sections`_ to take, in order. A name the graph does not hold is skipped
             * @param comment A comment written above each section taken. **Default**: none
             * @param inPlace Whether the sections keep the place held when the edit is made. **Default**: ``false``
             */
            GraphVarTake(GraphEditVariables& variables, std::string variable, ModObj graph, std::vector<std::string> sections,
                            std::string comment = "", bool inPlace = false);

            /**
             * @brief Takes the sections -- see this class's own description
             *
             * @param graphGroups The groups of graphs
             * @param modType Unused
             * @param modName Unused
             *
             * @return 'graphGroups'
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

        private:
            GraphEditVariables& variables_;
            std::string variable_;
            ModObj graph_;
            std::vector<std::string> sections_;
            std::string comment_;
            std::optional<GraphEditVariables::Sections::Place> place_;
    };
}

#endif
