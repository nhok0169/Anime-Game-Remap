#ifndef AGRemapCore_GraphGroupRemove_H
#define AGRemapCore_GraphGroupRemove_H

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
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Removes whole groups of graphs -- every graph one ``.ini`` file of the fix would be rendered
     from -- where :cpp:class:`GraphRemove` removes single graphs out of a group
     :raw-html:`<br />` :raw-html:`<br />`

     Removing EVERY group is how a fixer that has given up writes nothing. A
     :cpp:class:`GIMIFixer` renders every graph the parser handed it whether or not an edit touched
     it, so a fixer that simply stops building its edits writes the mod's own sections out again after
     the remap header, under the SOURCE's names -- which the remover cannot tell from the author's,
     so every later run appends another copy. With no groups there is nothing to render, and a fixer
     sharing the file with others contributes nothing to it

     .. note::
        A group index past the end is skipped silently, as :cpp:class:`GraphRemove` skips an
        out-of-range graph id. Groups are removed from the highest index down, so the indices given
        all mean the groups as they were before this edit ran
     @endrst
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphGroupRemove: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
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
             * @brief The ``.ini`` indices of the groups to remove, or ``std::nullopt`` for every group
             */
            std::optional<std::vector<std::size_t>> iniIndices;

            /**
             * @brief Constructs a new group-removing edit
             * @param iniIndices The ``.ini`` indices of the groups to remove, or ``std::nullopt`` for every group. **Default**: ``std::nullopt``
             */
            explicit GraphGroupRemove(std::optional<std::vector<std::size_t>> iniIndices = std::nullopt);

            /**
             * @brief Removes every group named by \ref iniIndices from 'graphGroups'
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix. Unused by this edit
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;
    };
}

#include "GraphGroupRemove.tpp"

#endif
