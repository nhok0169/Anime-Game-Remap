#ifndef AGRemapCore_GraphGroupPartEdits_H
#define AGRemapCore_GraphGroupPartEdits_H

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

#include <optional>
#include <string>

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Adapts a :cpp:class:`BaseIniGraphEdit` (the ``graphEdits/`` family) into the
     :cpp:class:`GraphGroupEdit::PartEdit` a :cpp:class:`GraphGroupEdit` holds
     :raw-html:`<br />` :raw-html:`<br />`

     :cpp:class:`GraphGroupEdit` deliberately stores a dispatch interface rather than
     :cpp:class:`BaseIniGraphEdit`/:cpp:class:`BaseRegEdit` pointers -- see that class's own note
     for the two reasons. The `pybind11`_ layer supplies its own implementation of that interface
     (``PyPartEdit``), which is what let it forward to a `Python`_ subclass; a **plain C++ caller
     had none at all**, so a :cpp:class:`GraphGroupEdit` built outside the binding layer could not
     be given a single edit. This class and \\ref RegPartEdit are that missing half
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphPartEdit: public GraphGroupEdit<K, V, KeyHash, KeyEqual>::PartEdit {
        public:

            /**
             * @brief The group edit whose dispatch interface this implements
             */
            using Owner = GraphGroupEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc GraphGroupEdit::PartEditKind
             */
            using PartEditKind = typename Owner::PartEditKind;

            /**
             * @copydoc GraphGroupEdit::Graph
             */
            using Graph = typename Owner::Graph;

            /**
             * @copydoc GraphGroupEdit::ContentPart
             */
            using ContentPart = typename Owner::ContentPart;

            /**
             * @copydoc GraphGroupEdit::PartFilter
             */
            using PartFilter = typename Owner::PartFilter;

            /**
             * @copydoc GraphGroupEdit::OrderRanges
             */
            using OrderRanges = typename Owner::OrderRanges;

            /**
             * @copydoc GraphGroupEdit::KeySet
             */
            using KeySet = typename Owner::KeySet;

            /**
             * @brief The kind of edit this adapts
             */
            using Edit = BaseIniGraphEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @brief Constructs an adapter around a graph edit
             *
             * @param edit The edit to adapt -- **borrowed**, and must outlive this adapter
             */
            explicit GraphPartEdit(Edit* edit): edit_(edit) {}

            /**
             * @brief The adapted edit -- borrowed, never owned
             */
            Edit* edit() const { return edit_; }

            PartEditKind kind() const override {
                // A null edit is skipped rather than crashed on, matching how the binding layer
                // reports an object it does not recognise.
                return edit_ == nullptr ? PartEditKind::None : PartEditKind::GraphEdit;
            }

            Graph* editGraph(Graph& graph, IniFile* ini, const ModType* modType, const std::string& modName,
                              const PartFilter* keyFilter, bool trackKeys,
                              const std::optional<KeySet>& keysToTrack) override {
                if (edit_ == nullptr) {
                    return &graph;
                }

                // editFromIni rather than edit: the .ini file is genuinely available here, and it
                // is what BaseIniGraphEdit's own contract says to call when it is.
                //
                // A null keyFilter is "no filter", which is an empty PartFilter rather than one
                // returning empty ranges -- the latter would filter everything away.
                return &edit_->editFromIni(graph, ini, modType, modName,
                                            keyFilter == nullptr ? PartFilter() : *keyFilter,
                                            trackKeys, keysToTrack);
            }

            void editPart(ContentPart&, const std::string&, IniFile*, const ModType*, const std::string&,
                           const OrderRanges&) override {
                // Never called -- kind() reports GraphEdit.
            }

        private:
            Edit* edit_;
    };


    /**
     * @brief
     @rst
     Adapts a :cpp:class:`BaseRegEdit` (the ``regEdits/`` family) into the
     :cpp:class:`GraphGroupEdit::PartEdit` a :cpp:class:`GraphGroupEdit` holds -- the register-level
     counterpart of \\ref GraphPartEdit, and see that class for why both exist
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegPartEdit: public GraphGroupEdit<K, V, KeyHash, KeyEqual>::PartEdit {
        public:

            /**
             * @copydoc GraphPartEdit::Owner
             */
            using Owner = GraphGroupEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc GraphGroupEdit::PartEditKind
             */
            using PartEditKind = typename Owner::PartEditKind;

            /**
             * @copydoc GraphGroupEdit::Graph
             */
            using Graph = typename Owner::Graph;

            /**
             * @copydoc GraphGroupEdit::ContentPart
             */
            using ContentPart = typename Owner::ContentPart;

            /**
             * @copydoc GraphGroupEdit::PartFilter
             */
            using PartFilter = typename Owner::PartFilter;

            /**
             * @copydoc GraphGroupEdit::OrderRanges
             */
            using OrderRanges = typename Owner::OrderRanges;

            /**
             * @copydoc GraphGroupEdit::KeySet
             */
            using KeySet = typename Owner::KeySet;

            /**
             * @brief The kind of edit this adapts
             */
            using Edit = BaseRegEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @brief Constructs an adapter around a register edit
             *
             * @param edit The edit to adapt -- **borrowed**, and must outlive this adapter
             */
            explicit RegPartEdit(Edit* edit): edit_(edit) {}

            /**
             * @brief The adapted edit -- borrowed, never owned
             */
            Edit* edit() const { return edit_; }

            PartEditKind kind() const override {
                return edit_ == nullptr ? PartEditKind::None : PartEditKind::RegEdit;
            }

            Graph* editGraph(Graph& graph, IniFile*, const ModType*, const std::string&, const PartFilter*, bool,
                              const std::optional<KeySet>&) override {
                // Never called -- kind() reports RegEdit.
                return &graph;
            }

            void editPart(ContentPart& part, const std::string& sectionName, IniFile* ini, const ModType* modType,
                           const std::string& modName, const OrderRanges& partRanges) override {
                if (edit_ == nullptr) {
                    return;
                }

                // By-pointer here, unlike the graph side's by-value filter: BaseRegEdit reads
                // 'partRanges' as nullable, where the enclosing GraphGroupEdit always has real
                // ranges to hand over.
                edit_->editFromIni(part, sectionName, ini, modType, modName, &partRanges);
            }

        private:
            Edit* edit_;
    };
}

#endif
