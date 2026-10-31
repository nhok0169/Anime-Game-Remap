#ifndef AGRemapCore_VarSectionsCreate_H
#define AGRemapCore_VarSectionsCreate_H
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

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/GraphEditVariables.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Writes sections variables of a :cpp:class:`GraphEditVariables` out as a graph in each group :raw-html:`<br />`
     :raw-html:`<br />`

     For each group, :cpp:member:`variablesOf` names the variables whose `sections`_ make up that group's graph, in
     order; a section of a name already in the graph is left out, so the first one written wins. The graph is added
     under :cpp:member:`graph` in that group, and a group whose variables hold nothing gets no graph. When there are no
     groups at all, the first group's variables are written into a group of its own
     @endrst
     */
    class VarSectionsCreate: public BaseIniGraphGroupEdit<> {
        public:
            /**
             * @brief A mod object: ``(component, object)``
             */
            using ModObj = std::pair<std::string, std::string>;

            /**
             * @brief The names of the variables that make up a group's graph, in order, given the group's index
             */
            using VariablesOf = std::function<std::vector<std::string>(std::size_t group)>;

            /**
             * @brief How a graph made of a fix's own `sections`_ follows ``run =``: each value names the section it calls
             */
            static IfTemplateRunConfig<std::string, std::string> runCalls();

            /**
             * @brief Constructs the edit
             *
             * @param variables The variables -- borrowed, they must outlive the edit
             * @param graph See \ref graph
             * @param variablesOf See \ref variablesOf
             * @param runConfig See \ref runConfig. **Default**: :cpp:func:`runCalls`
             */
            VarSectionsCreate(GraphEditVariables& variables, ModObj graph, VariablesOf variablesOf,
                              IfTemplateRunConfig<std::string, std::string> runConfig = runCalls());

            /**
             * @brief The mod object the graph is added under in each group
             */
            ModObj graph;

            /**
             * @brief The names of the variables that make up a group's graph, in order, given the group's index
             */
            VariablesOf variablesOf;

            /**
             * @brief How the graph follows ``run =``
             */
            IfTemplateRunConfig<std::string, std::string> runConfig;

            /**
             * @brief The context the graph builds its conditions in, or ``nullptr`` for its own. **Default**: ``nullptr``
             */
            Z3Context* z3 = nullptr;

            /**
             * @brief Writes the graphs -- see this class's own description
             *
             * @param graphGroups The groups of graphs
             * @param modType Unused
             * @param modName The name of the mod, handed to the graph's creation
             *
             * @return 'graphGroups'
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

        private:
            GraphEditVariables& variables_;
    };
}

#endif
