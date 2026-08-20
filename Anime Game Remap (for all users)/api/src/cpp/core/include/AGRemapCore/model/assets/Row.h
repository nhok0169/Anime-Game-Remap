#ifndef AGRemapCore_Row_H
#define AGRemapCore_Row_H

#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     One flattened entry for :cpp:class:`ModDictAssets`/:cpp:class:`ModMappedAssets` -- the C++
     side never models the pure-Python originals' nested ``Dict[Hashable, T]`` directly (see
     :cpp:class:`ModDictAssets`'s class-level note for why); a ``Row`` is what a leaf of that
     nested dict becomes once flattened
     @endrst
     *
     * @tparam K The type for an index value
     * @tparam T The type for the leaf content of an asset
     */
    template <typename K, typename T>
    struct Row {
        /**
         * @brief The value of every index column for this row, in index order (the raw,
         *      not-yet-parsed value is stored at the version index's position too -- see
         *      :cpp:class:`ModDictAssets`'s constructor for where that gets parsed)
         */
        std::vector<K> indexVals;

        /**
         * @brief The leaf content of the asset at this row
         */
        T value;
    };
}

#endif
