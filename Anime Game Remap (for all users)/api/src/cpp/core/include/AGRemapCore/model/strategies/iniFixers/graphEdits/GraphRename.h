#ifndef AGRemapCore_GraphRename_H
#define AGRemapCore_GraphRename_H

#include <functional>
#include <optional>
#include <string>

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphEdit`

     Renames the `sections`_ of some caller/callee graph of :cpp:class:`IniSectionGraph`
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This is a thin wrapper over :cpp:func:`IniSectionGraph::rename`, which also rewrites every
        ``run =`` reference to a renamed `section`_ and rebuilds the graph -- so a rename never
        leaves a dangling caller/callee edge behind
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphRename: public BaseIniGraphEdit<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseIniGraphEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniGraphEdit::Graph
             */
            using Graph = typename Base::Graph;

            /**
             * @copydoc BaseIniGraphEdit::PartFilter
             */
            using PartFilter = typename Base::PartFilter;

            /**
             * @copydoc BaseIniGraphEdit::KeySet
             */
            using KeySet = typename Base::KeySet;

            /**
             * @brief
             @rst
             The function used to rename a `section`_ -- it takes in the name of the old `section`_
             and returns the new name for the `section`_
             @endrst
             */
            using RenameFunc = std::function<std::string(const std::string&)>;

            /**
             * @brief
             @rst
             The function used to rename a `section`_ -- it takes in the name of the old `section`_
             and returns the new name for the `section`_
             @endrst
             */
            RenameFunc renameFunc;

            /**
             * @brief Constructs a new `section`_-renaming edit
             *
             * @param renameFunc The function used to rename a `section`_ -- an empty function leaves every name untouched
             */
            explicit GraphRename(RenameFunc renameFunc = {});

            /**
             * @brief
             @rst
             Renames every `section`_ of 'graph' by ``renameFunc`` :raw-html:`<br />`
             :raw-html:`<br />`

             An empty ``renameFunc`` is a no-op -- 'graph' is handed straight back
             @endrst
             *
             * @param graph The graph to edit, modified in place
             * @param modType The type of mod to fix. Unused by this edit
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             * @param partFilter The filter for valid order indices. Unused by this edit. **Default**: empty
             * @param trackKeys The caller's key-tracking default. Unused by this edit. **Default**: ``false``
             * @param keysToTrack The caller's key-tracking key set. Unused by this edit. **Default**: ``std::nullopt``
             *
             * @return The same graph that was passed in, after every `section`_ was renamed
             */
            Graph& edit(Graph& graph, const ModType* modType,
                         const std::string& modName = "", const PartFilter& partFilter = {},
                         bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt) override;
    };
}

#include "GraphRename.tpp"

#endif
