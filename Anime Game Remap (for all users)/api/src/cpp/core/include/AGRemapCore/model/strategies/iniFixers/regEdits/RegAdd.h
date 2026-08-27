#ifndef AGRemapCore_RegAdd_H
#define AGRemapCore_RegAdd_H

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseRegEdit`

     Bulk adds some `KVPs`_ into some :cpp:class:`IfContentPart`
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegAdd: public BaseRegEdit<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseRegEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseRegEdit::ContentPart
             */
            using ContentPart = typename Base::ContentPart;

            /**
             * @copydoc BaseRegEdit::OrderRanges
             */
            using OrderRanges = typename Base::OrderRanges;

            /**
             * @brief
             @rst
             The `KVPs`_ to add, in the order given
             @endrst
             */
            std::vector<std::pair<K, V>> vals;

            /**
             * @brief
             @rst
             Whether to add \ref vals at the end of the :cpp:class:`IfContentPart` (or, if
             ``partRanges`` is provided to \ref edit, at the end of that window), instead of at
             the beginning
             @endrst
             */
            bool latest;

            /**
             * @brief Constructs a new bulk-add register edit
             *
             * @param vals
             @rst
             The `KVPs`_ to add, in the order given
             @endrst
             * @param latest
             @rst
             Whether to add ``vals`` at the end of the :cpp:class:`IfContentPart` instead of at
             the beginning :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             */
            explicit RegAdd(std::vector<std::pair<K, V>> vals = {}, bool latest = true);

            /**
             * @brief
             @rst
             Adds every `KVP`_ in \ref vals into 'part' :raw-html:`<br />` :raw-html:`<br />`

             With no ``partRanges``, the `KVPs`_ go straight to the true beginning/end of 'part'
             (based on \ref latest). With a ``partRanges`` window, they instead go right after
             the last valid index of that window (or right before its first valid index, when
             \ref latest is ``false``) -- an unbounded window edge falls back to the true
             end/beginning of 'part'. An empty \ref vals or an empty ``partRanges`` leaves 'part'
             untouched
             @endrst
             *
             * @param part The part of the `IfTemplate` being edited, modified in place
             * @param sectionName The name of the `section`_ being edited. Unused by this edit
             * @param modType The type of mod to fix, or ``nullptr``. Unused by this edit. **Default**: ``nullptr``
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             * @param partRanges The valid order indices to process for 'part', or ``nullptr`` for all of them. **Default**: ``nullptr``
             *
             * @return The same part that was passed in, after editing
             */
            ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                              const std::string& modName = "", const OrderRanges* partRanges = nullptr) override;
    };
}

#include "RegAdd.tpp"

#endif
