#ifndef AGRemapCore_IterGraphGroupEdit_H
#define AGRemapCore_IterGraphGroupEdit_H

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
#include <memory>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Runs the edits a factory builds for each group of graphs, on every group there is when the edit
     runs :raw-html:`<br />` :raw-html:`<br />`

     What it is for: edits on graphs that only exist at fix time. :cpp:class:`GraphGroupEdit` is
     handed its edits one ``.ini`` file at a time when it is built, but an edit such as
     :cpp:class:`GraphGroupRemap` decides at fix time how many ``.ini`` files there are, and an edit
     that names a graph -- a :cpp:class:`ResRegCollect`'s keys, a resource edit's graph id -- names it
     by its ``.ini`` file's index. The factory is handed that index and the groups, and builds the
     group's edits from them :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The factory is called for **every** group, so the edits it builds have to be right on every
        group: an edit that would do something wrong on some group must return nothing for it, or
        carry its own filter (eg. :cpp:member:`RegIfTemplateAdd::sectionFilter`, or a
        :cpp:class:`ResRegCollect` predicate) that makes it a no-op there. To edit chosen groups
        only, use :cpp:class:`GraphGroupEdit`
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IterGraphGroupEdit: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
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
             * @brief The kind of resource edit a #GroupEdits keeps alive
             */
            using ResEdit = BaseResEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The edits the factory builds for one group
             */
            struct GroupEdits {
                /**
                 * @brief The edits to run on the groups, in order
                 */
                std::vector<std::unique_ptr<Base>> edits;

                /**
                 * @brief
                 @rst
                 The resource edits #edits borrow -- eg. those a :cpp:class:`ResRegCollect` names in
                 :cpp:member:`ResRegCollect::resEdits` -- owned here so they live until #edits have run
                 @endrst
                 */
                std::vector<std::unique_ptr<ResEdit>> resEdits;

                /**
                 * @brief
                 @rst
                 Edits owned elsewhere, run after #edits, in order -- eg. a fixer's stage, built when the list first
                 reaches it and kept by the fixer because what it registers (a resource collect's edits) is read after
                 the edits have run. Each must outlive the run
                 @endrst
                 */
                std::vector<Base*> borrowed;
            };

            /**
             * @brief
             @rst
             Builds the edits for one group, from the group's ``.ini`` file index and the groups.
             Returns no edits to leave the group alone
             @endrst
             */
            using EditFactory = std::function<GroupEdits(std::size_t iniIndex, GraphGroups& graphGroups)>;

            /**
             * @brief The factory that builds each group's edits. An empty ``std::function`` edits nothing
             */
            EditFactory makeEdits;

            /**
             * @brief
             @rst
             Builds the edits to run ONCE, over all the groups, from what the edit is run on: the groups, the ``.ini``
             file, the mod type and the mod being fixed to. Returns no edits to do nothing
             @endrst
             */
            using RunFactory = std::function<GroupEdits(GraphGroups& graphGroups, IniFile* ini, const ModType* modType,
                                                        const std::string& modName)>;

            /**
             * @brief
             @rst
             The factory that builds the edits run once over all the groups, before #makeEdits' per-group ones -- for a
             step of a list that is the same for every mod, whose edits come from the mod it runs on. An empty
             ``std::function`` (the default) builds nothing
             @endrst
             */
            RunFactory makeRunEdits;

            /**
             * @brief Constructs a new edit run on every group
             *
             * @param makeEdits The factory that builds each group's edits
             */
            explicit IterGraphGroupEdit(EditFactory makeEdits = nullptr);

            /**
             * @brief
             @rst
             Runs the edits #makeRunEdits builds, then calls #makeEdits for each group, in order of the ``.ini`` file
             index, and runs the edits it builds with ``editFromIni`` :raw-html:`<br />` :raw-html:`<br />`

             The groups are counted once, before the first one is edited, so a group an edit adds is
             not visited
             @endrst
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param ini The associated original .ini file. **Nullable**
             * @param modType The type of mod to fix. **Nullable**
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& editFromIni(GraphGroups& graphGroups, IniFile* ini, const ModType* modType,
                                     const std::string& modName = "") override;

            /**
             * @brief The same as #editFromIni, with no ``.ini`` file
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix. **Nullable**
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;
    };
}

#include "IterGraphGroupEdit.tpp"

namespace AGRemapCore {
    // note: every translation unit that uses IterGraphGroupEdit<std::string, std::string> would otherwise
    //   instantiate all of its member bodies itself; they are compiled once, in
    //   src/model/strategies/iniFixers/graphGroupEdits/IterGraphGroupEditInstantiation.cpp, instead.
    extern template class IterGraphGroupEdit<std::string, std::string>;
}

#endif
