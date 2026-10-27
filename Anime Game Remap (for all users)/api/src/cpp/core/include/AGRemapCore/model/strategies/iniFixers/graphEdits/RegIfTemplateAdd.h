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

#ifndef AGRemapCore_RegIfTemplateAdd_H
#define AGRemapCore_RegIfTemplateAdd_H

#include <functional>
#include <optional>
#include <string>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphEdit`

     Adds the parts of an :cpp:class:`IfTemplate` -- `KVPs`_ and whole ``if`` / ``else`` blocks --
     into every root `section`_ of a graph, at the \ref position given :raw-html:`<br />`
     :raw-html:`<br />`

     Where :cpp:class:`RegAdd` adds `KVPs`_ to parts that already exist and :cpp:class:`RegBottomAdd`
     appends one block at the bottom, this one inserts a structure the caller wrote, wherever the
     fix needs it to run -- eg. a constant buffer bound per pass BEFORE the `section`_'s draws:

     .. code-block:: ini

        [TextureOverrideCharacterLegs]
        hash = a913e9a9
        match_first_index = 0
        ; position = AfterKey, key = match_first_index: the template's parts go HERE
        if ps == 51840.1
            ps-cb3 = ResourceLegsMainCb3
        else if ps == 51840.2
            ps-cb3 = ResourceLegsOutlineCb3
        endif
        ib = ResourceLegsIB
        drawindexed = auto

     The template's parts are copied in, so one template serves every root and every graph. Its
     ``if`` lines are re-parsed under the graph's own :cpp:class:`Z3Context`, and its `KVPs`_ keep
     their nesting, shifted to the depth they are inserted at. A template is most easily written
     as text and parsed: the one `section`_ of an :cpp:class:`IniFile` made from it.

     .. note::
        At the bottom, a namespace-merged `section`_ (one guarded as a whole by
        ``if $\<Char>\Master\swapvar == n``) takes the parts inside its guard, before the guard's
        ``endif``, as :cpp:class:`RegBottomAdd` does. With \ref Position::AfterKey, a root that has no
        \ref key is left alone
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegIfTemplateAdd: public BaseIniGraphEdit<K, V, KeyHash, KeyEqual> {
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
             * @brief The `section`_ type this edit adds to, and the template's type
             */
            using Section = typename Graph::Section;

            /**
             * @brief The parts holding `KVPs`_
             */
            using ContentPart = typename Graph::ContentPart;

            /**
             * @brief Where in each root the template's parts go
             */
            enum class Position {
                /**
                 * @brief Before everything the `section`_ has
                 */
                Top,

                /**
                 * @brief After everything the `section`_ has, at its own depth (inside a namespace-merge guard)
                 */
                Bottom,

                /**
                 * @brief Right after the first `KVP`_ whose key is \ref key, at that `KVP`_'s depth
                 */
                AfterKey
            };

            /**
             * @brief
             @rst
             The template whose parts are added. **Borrowed**: the caller keeps it alive while the
             edit runs. ``nullptr`` makes the edit a no-op
             @endrst
             */
            const Section* ifTemplate;

            /**
             * @brief Where in each root the parts go
             */
            Position position;

            /**
             * @brief The key \ref Position::AfterKey inserts after
             */
            K key;

            /**
             * @brief Constructs a new template-adding edit
             *
             * @param ifTemplate The template whose parts are added -- see \ref ifTemplate
             * @param position Where in each root the parts go. **Default**: \ref Position::Bottom
             * @param key The key \ref Position::AfterKey inserts after. **Default**: empty
             */
            explicit RegIfTemplateAdd(const Section* ifTemplate = nullptr, Position position = Position::Bottom, K key = K());

            /**
             @rst
             Adds the parts of \ref ifTemplate into every root `section`_ of 'graph'. A
             \ref BaseIniGraphEdit::PartFilter given is asked about the root's first
             :cpp:class:`IfContentPart`, and a root it rejects is left alone
             @endrst
             *
             * @copydoc BaseIniGraphEdit::edit
             */
            Graph& edit(Graph& graph, const ModType* modType, const std::string& modName = "", const PartFilter& partFilter = {},
                        bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt) override;
    };
}

#include "RegIfTemplateAdd.tpp"

#endif
