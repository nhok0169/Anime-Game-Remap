#ifndef AGRemapCore_GraphRemove_H
#define AGRemapCore_GraphRemove_H

#include <functional>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Removes some graphs from a group of graphs :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        A graph id that names no existing graph (a missing ``(component, object)`` key, or an
        out-of-range ``.ini`` index) is skipped silently -- no exception. That's this class's
        documented contract, not an oversight; contrast :cpp:func:`BaseIniGraphGroupEdit::getGraph`,
        which raises by default
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphRemove: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniGraphGroupEdit::GraphGroups
             */
            using GraphGroups = typename Base::GraphGroups;

            /**
             * @copydoc BaseIniGraphGroupEdit::GraphId
             */
            using GraphId = typename Base::GraphId;

            /**
             * @brief The ids of the graphs to remove
             */
            std::vector<GraphId> graphIds;

            /**
             * @brief Constructs a new graph-removing edit
             *
             * @param graphIds The ids of the graphs to remove
             */
            explicit GraphRemove(std::vector<GraphId> graphIds = {});

            /**
             * @brief Removes every graph named by \ref graphIds from 'graphGroups'
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix. Unused by this edit
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;
    };
}

#include "GraphRemove.tpp"

#endif
