#ifndef AGRemapCore_GraphInherit_H
#define AGRemapCore_GraphInherit_H

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
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"
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

     Each `KVP`_ is ``<reg> = <a root of dst>``, so besides the ``run =`` call the name suggests,
     this can also compose a graph of resources into a graph that uses them. For example, with
     'dst' as the graph of ``[ResourceHeadDiffuse]`` and \ref reg as ``ps-t0``, a
     ``[TextureOverrideComponent0]`` in 'src' that binds no textures of its own gains
     ``ps-t0 = ResourceHeadDiffuse`` :raw-html:`<br />` :raw-html:`<br />`

     Where the `KVPs`_ go is decided by \ref adder when one is given, and otherwise by
     \ref latest and \ref partFilter (see \ref edit) :raw-html:`<br />` :raw-html:`<br />`

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
             The reference `KVPs`_ to insert, one ``(reg, root)`` pair per root `section`_ of the
             graph at \ref dst, in the order of that graph's roots
             @endrst
             */
            using KVPs = std::vector<std::pair<K, V>>;

            /**
             * @brief A register edit an #Adder can hand back
             */
            using RegEdit = BaseRegEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @brief A graph edit an #Adder can hand back
             */
            using GraphEdit = BaseIniGraphEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             What an #Adder hands back :raw-html:`<br />` :raw-html:`<br />`

             * a :cpp:class:`BaseRegEdit` or :cpp:class:`BaseIniGraphEdit` built from the `KVPs`_,
               which this class then runs over the graph at \ref src, the same way a
               :cpp:class:`GraphGroupEdit` would run it, with \ref partFilter as its key filter
             * ``std::monostate``, when the adder already inserted the `KVPs`_ itself
             @endrst
             */
            using AddEdit = std::variant<std::monostate, std::shared_ptr<RegEdit>, std::shared_ptr<GraphEdit>>;

            /**
             * @brief
             @rst
             Decides how the reference `KVPs`_ are added to the graph at \ref src
             :raw-html:`<br />` :raw-html:`<br />`

             Called once per edit, with the graph at \ref src, the `KVPs`_ to add, the .ini file
             being fixed (``nullptr`` when there is none), the type of mod and the name of the
             mod. An empty ``std::function`` means "no adder" -- see \ref edit
             @endrst
             */
            using Adder = std::function<AddEdit(Graph&, const KVPs&, IniFile*, const ModType*, const std::string&)>;

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
             :raw-html:`<br />` :raw-html:`<br />`

             With an \ref adder, this is instead the key filter the adder's edit runs with
             @endrst
             */
            PartFilter partFilter;

            /**
             * @brief
             @rst
             How the reference `KVPs`_ are added to the graph at \ref src -- empty to use
             \ref latest and \ref partFilter instead
             @endrst
             */
            Adder adder;

            /**
             * @brief Constructs a new graph-inheriting edit
             *
             * @param src The id of the source graph to insert the reference KVPs into
             * @param dst The id of the graph to merge into 'src'
             * @param reg The name of the register used to reference the root `sections`_ of the graph at 'dst'
             * @param latest Whether to insert the KVPs at the back instead of the front. **Default**: ``true``
             * @param partFilter Which areas of a part are valid to insert into -- empty for "no filter". **Default**: empty
             * @param adder How the KVPs are added -- empty to use 'latest' and 'partFilter' instead. **Default**: empty
             */
            GraphInherit(GraphId src, GraphId dst, K reg, bool latest = true, PartFilter partFilter = {}, Adder adder = {});

            /**
             * @brief
             @rst
             Inserts the reference `KVPs`_ from the graph at \ref dst into the graph at \ref src
             :raw-html:`<br />` :raw-html:`<br />`

             With an \ref adder, the adder decides: the edit it hands back is run over the graph
             at \ref src with \ref partFilter as its key filter, or, if it hands back nothing, it
             is taken to have inserted the `KVPs`_ itself. :raw-html:`<br />` :raw-html:`<br />`

             Without one, and with no \ref partFilter, the `KVPs`_ go straight to the very
             front/back (based on \ref latest) of every root `section`_ of the graph at \ref src.
             With a \ref partFilter, they instead go at the earliest/latest valid index of every
             :cpp:class:`IfContentPart` the filter accepts :raw-html:`<br />` :raw-html:`<br />`

             Nothing is added, and \ref adder is not called, when the graph at \ref dst has no roots
             @endrst
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix -- only ever handed to \ref partFilter and \ref adder. **Nullable**
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

            /**
             * @brief
             @rst
             The same as \ref edit, except that 'ini' is also handed to \ref partFilter,
             \ref adder, and the edit the adder hands back
             @endrst
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param ini The .ini file being fixed. **Nullable**
             * @param modType The type of mod to fix. **Nullable**
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& editFromIni(GraphGroups& graphGroups, IniFile* ini, const ModType* modType,
                                     const std::string& modName = "") override;

        private:
            /**
             * @brief The shared body of \ref edit and \ref editFromIni -- 'ini' is ``nullptr`` for \ref edit
             */
            GraphGroups& editImpl(GraphGroups& graphGroups, IniFile* ini, const ModType* modType, const std::string& modName);

            /**
             * @brief
             @rst
             Calls \ref adder, and runs the edit it hands back over 'srcGraph' through
             :cpp:func:`GraphGroupEdit::editSectionGraph`, putting any different graph that edit
             returns in the place of \ref src
             @endrst
             */
            void addWithEdit(GraphGroups& graphGroups, Graph& srcGraph, const KVPs& kvps, IniFile* ini, const ModType* modType,
                             const std::string& modName);
    };
}

#include "GraphInherit.tpp"

#endif
