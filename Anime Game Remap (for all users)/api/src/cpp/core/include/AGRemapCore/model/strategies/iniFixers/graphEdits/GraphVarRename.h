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

#ifndef AGRemapCore_GraphVarRename_H
#define AGRemapCore_GraphVarRename_H

#include <functional>
#include <map>
#include <optional>
#include <string>

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphEdit`

     Renames `3DMigoto`_ variables everywhere in the `sections`_ of some caller/callee graph of
     :cpp:class:`IniSectionGraph`: in each key (``global persist $x``, ``post $x``), each value and each ``if`` /
     ``else if`` condition. Which ``$name`` count is :cpp:func:`IniNamingTools::varsIn`'s answer -- a namespaced
     reference names another file's variable and is left alone :raw-html:`<br />` :raw-html:`<br />`

     For a `section`_ copied out of another ``.ini`` whose conditions read that file's own variables: a file with
     no ``namespace`` names its variables after its PATH, so another file cannot read them, and the copy reads a
     variable of its own under a name no other file uses
     @endrst
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphVarRename: public BaseIniGraphEdit<K, V, KeyHash, KeyEqual> {
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
             * @brief A `section`_ of the graph
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @brief A variable's lower-cased name -> its new name, both without the ``$``
             */
            std::map<std::string, std::string> renames;

            /**
             * @brief
             @rst
             The `Z3`_ context every condition of an edited `section`_ is parsed again in, so a renamed one reads its new
             names and a `section`_ copied from another ``.ini`` shares the context of the `sections`_ it now sits
             beside; ``nullptr`` to rename only the conditions' text. **Default**: ``nullptr``
             @endrst
             */
            Z3Context* z3 = nullptr;

            /**
             * @brief Constructs a new variable-renaming edit
             * @param renames A variable's lower-cased name -> its new name, both without the ``$``
             * @param z3 The `Z3`_ context a renamed condition is parsed again in. **Default**: ``nullptr``
             */
            explicit GraphVarRename(std::map<std::string, std::string> renames = {}, Z3Context* z3 = nullptr);

            /**
             * @brief
             @rst
             Renames the variables of one `section`_, as #edit does each `section`_ of a graph
             @endrst
             * @param section The `section`_, modified in place
             * @param renames A variable's lower-cased name -> its new name, both without the ``$``
             * @param z3 The `Z3`_ context every condition is parsed again in, or ``nullptr`` to rename only their text
             */
            static void renameIn(Section& section, const std::map<std::string, std::string>& renames, Z3Context* z3 = nullptr);

            /**
             * @brief Renames the variables of every `section`_ of 'graph' by #renames
             * @param graph The graph to edit, modified in place
             * @param modType The type of mod to fix. Unused by this edit
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             * @param partFilter The filter for valid order indices. Unused by this edit. **Default**: empty
             * @param trackKeys The caller's key-tracking default. Unused by this edit. **Default**: ``false``
             * @param keysToTrack The caller's key-tracking key set. Unused by this edit. **Default**: ``std::nullopt``
             * @return The same graph that was passed in
             */
            Graph& edit(Graph& graph, const ModType* modType,
                        const std::string& modName = "", const PartFilter& partFilter = {},
                        bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt) override;
    };
}

#include "GraphVarRename.tpp"

#endif
