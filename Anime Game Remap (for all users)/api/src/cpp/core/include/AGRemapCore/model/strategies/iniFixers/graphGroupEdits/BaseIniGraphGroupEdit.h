#ifndef AGRemapCore_BaseIniGraphGroupEdit_H
#define AGRemapCore_BaseIniGraphGroupEdit_H

#include <cstddef>
#include <string>
#include <vector>

#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniPartEdit.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     Base class for a filter that edits a group of caller/callee graphs across many ``.ini`` files
     :raw-html:`<br />` :raw-html:`<br />`

     Note that this derives from :cpp:class:`BaseIniPartEdit` **directly**, not from
     :cpp:class:`BaseIniGraphPartEdit` -- matching the pure-Python original, since this edits whole
     *groups* of graphs rather than one part within a single graph
     @endrst
     */
    class BaseIniGraphGroupEdit: public BaseIniPartEdit {
        public:

            /**
             * @brief The type of graph held in a group
             */
            using Graph = IniGraphGroup::Graph;

            /**
             * @brief
             @rst
             The group of graphs for each ``.ini`` file -- one :cpp:class:`IniGraphGroup` per file
             @endrst
             */
            using GraphGroups = std::vector<IniGraphGroup>;

            /**
             * @brief
             @rst
             Identifies one graph within a #GraphGroups :raw-html:`<br />` :raw-html:`<br />`

             The pure-Python original uses a bare ``Tuple[int, str, str]`` here. This is a named
             holder instead, following this subsystem's own "a named data class beats a positional
             tuple" convention, and reusing :cpp:type:`IniGraphGroup::ModObj` for the two name
             fields rather than restating them
             @endrst
             */
            struct GraphId {
                /**
                 * @brief The index of the .ini file -- an index into the #GraphGroups vector
                 */
                std::size_t iniIndex = 0;

                /**
                 * @brief The name of the component and the name of the mod object
                 */
                IniGraphGroup::ModObj modObj;
            };

            /**
             * @brief
             @rst
             Edits a group of caller/callee graphs with state info from 'ini' :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The base implementation forwards straight to #edit and **ignores 'ini' entirely**,
                exactly as the pure-Python original does -- see
                :cpp:func:`BaseIniGraphEdit::editFromIni`'s own note
             @endrst
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param ini The associated original .ini file
             * @param modType The type of mod to fix
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            virtual GraphGroups& editFromIni(GraphGroups& graphGroups, IniFile& ini, const ModType& modType,
                                              const std::string& modName = "");

            /**
             * @brief
             @rst
             Edits a group of caller/callee graphs. No-op by default (returns 'graphGroups'
             untouched), matching the pure-Python original's ``pass``
             @endrst
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            virtual GraphGroups& edit(GraphGroups& graphGroups, const ModType& modType, const std::string& modName = "");

            /**
             * @brief
             @rst
             Retrieves the corresponding graph from a group of graphs :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The pure-Python original also takes a ``default`` argument to return when nothing is
                found and ``errorOnNotFound`` is ``False``. That has no typed C++ equivalent, and
                isn't needed: ``nullptr`` already *is* the "not found" answer. This otherwise matches
                :cpp:func:`IniSectionGraph::getSection`'s existing
                found-or-``nullptr``-or-throw convention exactly
             @endrst
             *
             * @param graphGroups The group of graphs for each .ini file
             * @param id Which graph to retrieve
             * @param errorOnNotFound Whether to throw when no graph is found. **Default**: ``true``
             *
             * @throw std::out_of_range If no graph is found and 'errorOnNotFound' is ``true``
             *
             * @return
             @rst
             The found graph, or ``nullptr`` if there is none and 'errorOnNotFound' is ``false``.
             Non-owning -- the graph stays owned by its :cpp:class:`IniGraphGroup`
             @endrst
             */
            static Graph* getGraph(GraphGroups& graphGroups, const GraphId& id, bool errorOnNotFound = true);

            /**
             * @brief Adds a graph to the group of graphs
             *
             * @param graphGroups The group of graphs for each .ini file
             * @param id Where to add the graph
             * @param graph
             @rst
             The graph to add -- taken **by value and moved from**, since
             :cpp:class:`IniSectionGraph` is move-only
             @endrst
             *
             * @return
             @rst
             Whether the graph was added -- ``false`` if ``id.iniIndex`` is out of range for
             'graphGroups', matching the pure-Python original
             @endrst
             */
            static bool addGraph(GraphGroups& graphGroups, const GraphId& id, Graph graph);
    };
}

#endif
