#ifndef AGRemapCore_BaseIniGraphEdit_H
#define AGRemapCore_BaseIniGraphEdit_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>

#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniGraphPartEdit.h"
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphPartEdit`

     Base class for a filter that edits some caller/callee graph of :cpp:class:`IniSectionGraph`
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class BaseIniGraphEdit: public BaseIniGraphPartEdit {
        public:

            /**
             * @brief The type of graph this edits
             */
            using Graph = IniSectionGraph<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The per-part iteration data a #PartFilter is handed
             */
            using IterData = SectionIterData<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The ranges of valid `KVP`_ order indices a #PartFilter returns -- ``long long`` because
             that's the type :cpp:class:`IfContentPart` keys its order indices by
             @endrst
             */
            using OrderRanges = Ranges<long long>;

            /**
             * @brief
             @rst
             The filter used to indicate the valid order indices to process for some
             :cpp:class:`IfContentPart` in the graph :raw-html:`<br />` :raw-html:`<br />`

             The mod type and ``.ini`` file arguments are **non-owning, nullable** pointers,
             matching the pure-Python original's ``Optional[IniFile]`` and this subsystem's own
             convention for still-pure-Python collaborators -- an empty ``std::function`` stands in
             for that original's ``partFilter = None``. Identical to
             :cpp:type:`GraphGroupEdit::PartFilter`, which is what actually hands one of these down
             @endrst
             */
            using PartFilter = std::function<OrderRanges(const IterData&, const ModType*, IniFile*)>;

            /**
             * @brief A set of the keys to keep track of for colouring
             */
            using KeySet = std::unordered_set<K, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             Edits the caller/callee graph with state info from 'ini' :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The base implementation forwards straight to #edit and **ignores 'ini' entirely**,
                exactly as the pure-Python original does (its ``BaseIniPartEdit.editFromIni`` calls
                ``self.edit(*args, **kwargs)`` without passing ``ini`` along). Subclasses that
                actually need the ``.ini`` file's state override this rather than #edit
             @endrst
             *
             * @param graph The graph to edit, modified in place
             * @param ini The associated .ini file. **Nullable** -- see this subsystem's own note on still-pure-Python collaborators
             * @param modType The type of mod to fix. **Nullable**, same reason
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param partFilter The filter for valid order indices -- empty for "no filter". **Default**: empty
             * @param trackKeys The caller's key-tracking default -- see #edit. **Default**: ``false``
             * @param keysToTrack The caller's key-tracking key set -- see #edit. **Default**: ``std::nullopt``
             *
             * @return The same graph that was passed in, after editing
             */
            virtual Graph& editFromIni(Graph& graph, IniFile* ini, const ModType* modType,
                                        const std::string& modName = "", const PartFilter& partFilter = {},
                                        bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt);

            /**
             * @brief
             @rst
             Edits the caller/callee graph. No-op by default (returns 'graph' untouched), matching
             the pure-Python original's ``pass``
             @endrst
             *
             * @param graph The graph to edit, modified in place
             * @param modType The type of mod to fix. **Nullable**
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param partFilter The filter for valid order indices -- empty for "no filter". **Default**: empty
             * @param trackKeys
             @rst
             The **caller's** key-tracking default, handed down by whatever is driving this edit
             (:cpp:class:`GraphGroupEdit` passes its own ``trackKeys`` here) :raw-html:`<br />`
             :raw-html:`<br />`

             A subclass that has its own key-tracking setting decides how to combine the two;
             :cpp:class:`RegFillMissing` treats this as a default it can turn *on* but not off, so
             either side asking for tracking is enough. A subclass with no such setting simply
             ignores it :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             * @param keysToTrack
             @rst
             The **caller's** key-tracking key set, handed down the same way -- ``std::nullopt``
             meaning "every key", matching
             :cpp:func:`IfContentPartColouring::updateColouring`'s own convention :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @return The same graph that was passed in, after editing
             */
            virtual Graph& edit(Graph& graph, const ModType* modType,
                                 const std::string& modName = "", const PartFilter& partFilter = {},
                                 bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt);
    };
}

#include "BaseIniGraphEdit.tpp"

#endif
