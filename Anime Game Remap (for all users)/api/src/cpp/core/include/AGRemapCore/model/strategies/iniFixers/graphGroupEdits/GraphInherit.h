#ifndef AGRemapCore_GraphInherit_H
#define AGRemapCore_GraphInherit_H

#include <functional>
#include <string>

#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Merges the graph at 'dst' into the graph at 'src', by inserting consecutive `KVPs`_ into
     'src' that reference every root `section`_ of the graph at 'dst' :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        This only inserts the reference `KVPs`_ into 'src' -- the `sections`_ of 'dst' themselves
        are left untouched (and still need to be reachable/present elsewhere for the reference to
        resolve, the same way a plain ``run =`` reference to another `section`_ works)

     .. note::
        If either the graph at 'src' or the graph at 'dst' cannot be found, nothing is inserted and
        the original ``graphGroups`` is returned as-is -- no exception is raised
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphInherit: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
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
             * @copydoc BaseIniGraphGroupEdit::Graph
             */
            using Graph = typename Base::Graph;

            /**
             * @brief The per-part iteration data a #PartFilter is handed
             */
            using IterData = SectionIterData<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The ranges of valid `KVP`_ order indices a #PartFilter returns -- ``long long``
             because that's the type :cpp:class:`IfContentPart` keys its order indices by
             @endrst
             */
            using OrderRanges = Ranges<long long>;

            /**
             * @brief
             @rst
             The filter used to indicate which areas of some :cpp:class:`IfContentPart` within the
             graph at \ref src are valid to insert the `KVPs`_ :raw-html:`<br />`
             :raw-html:`<br />`

             An empty ``std::function`` stands in for the pure-Python original's
             ``partFilter = None``, which selects the very front/back of every root `section`_
             instead (see \ref edit)
             @endrst
             */
            using PartFilter = std::function<OrderRanges(const IterData&, const ModType*, IniFile*)>;

            /**
             * @brief
             @rst
             The id of the source graph to insert the reference `KVPs`_ into
             @endrst
             */
            GraphId src;

            /**
             * @brief The id of the graph to merge into \ref src
             */
            GraphId dst;

            /**
             * @brief
             @rst
             The name of the register used to reference the root `sections`_ of the graph at
             \ref dst
             @endrst
             */
            K reg;

            /**
             * @brief
             @rst
             Whether to insert the `KVPs`_ at the back of the areas to insert, instead of at the
             front
             @endrst
             */
            bool latest;

            /**
             * @brief
             @rst
             The filter used to indicate which areas of some :cpp:class:`IfContentPart` within the
             graph at \ref src are valid to insert the `KVPs`_ -- empty for "no filter"
             @endrst
             */
            PartFilter partFilter;

            /**
             * @brief Constructs a new graph-inheriting edit
             *
             * @param src The id of the source graph to insert the reference KVPs into
             * @param dst The id of the graph to merge into 'src'
             * @param reg The name of the register used to reference the root `sections`_ of the graph at 'dst'
             * @param latest Whether to insert the KVPs at the back instead of the front. **Default**: ``true``
             * @param partFilter Which areas of a part are valid to insert into -- empty for "no filter". **Default**: empty
             */
            GraphInherit(GraphId src, GraphId dst, K reg, bool latest = true, PartFilter partFilter = {});

            /**
             * @brief
             @rst
             Inserts the reference `KVPs`_ from the graph at \ref dst into the graph at \ref src
             :raw-html:`<br />` :raw-html:`<br />`

             With no \ref partFilter, the `KVPs`_ go straight to the very front/back (based on
             \ref latest) of every root `section`_ of the graph at \ref src. With one, they instead
             go at the earliest/latest valid index of every :cpp:class:`IfContentPart` the filter
             accepts
             @endrst
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix -- only ever handed to \ref partFilter. **Nullable**
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;
    };
}

#include "GraphInherit.tpp"

#endif
