#ifndef AGRemapCore_RegRemap_H
#define AGRemapCore_RegRemap_H

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

     Bulk-renames the register keys for some :cpp:class:`IfContentPart`
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegRemap: public BaseRegEdit<K, V, KeyHash, KeyEqual> {
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
             The remap rules for one old key -- either a plain list of replacement keys/
             :cpp:class:`RemappedKeyData`, or a whole :cpp:class:`KeyRemapData`, exactly as
             :cpp:func:`IfContentPart::remapKeys` takes
             @endrst
             */
            using KeyRemapValue = typename ContentPart::KeyRemapValue;

            /**
             * @brief
             @rst
             The old key -> remap rules mapping to apply :raw-html:`<br />` :raw-html:`<br />`

             See :cpp:func:`IfContentPart::remapKeys` for the full semantics of how a rule set is
             evaluated for a given key's occurrences
             @endrst
             */
            std::vector<std::pair<K, KeyRemapValue>> keyRemap;

            /**
             * @brief Constructs a new bulk key-renaming register edit
             *
             * @param keyRemap The old key -> remap rules mapping to apply
             */
            explicit RegRemap(std::vector<std::pair<K, KeyRemapValue>> keyRemap = {});

            /**
             * @brief
             @rst
             Applies \ref keyRemap to 'part', by forwarding straight to
             :cpp:func:`IfContentPart::remapKeys`
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

#include "RegRemap.tpp"

#endif
