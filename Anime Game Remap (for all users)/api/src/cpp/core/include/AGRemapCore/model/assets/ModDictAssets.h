#ifndef AGRemapCore_ModDictAssets_H
#define AGRemapCore_ModDictAssets_H

#include <cstddef>
#include <functional>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class to handle assets of any type for a mod where retrieval is based on some keys where only
     one of the keys refers to some versioning -- the C++ counterpart to the pure-Python
     ``ModDictAssets`` (``model/assets/ModDictAssets.py``) :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Unlike the pure-Python original, this class never models the source data as a literal
        nested ``Dict[Hashable, T]`` -- the pure-Python ``makeIndices`` flattens that nested dict
        into a `pandas`_ `DataFrame`_ purely to get sorted/filterable version lookups, and
        ``_get`` separately re-walks the *original* nested dict for the actual leaf value. Here,
        the flattening happens once, up front (by whoever builds the :cpp:class:`Row` list --
        typically the pybind11 binding layer, walking a Python nested dict), and this class only
        ever operates on that flat form: a hash map from a row's non-version index values to a
        version-sorted list of ``(version, value)`` entries. There is no "partial query" support
        for retrieving an intermediate sub-structure at less than full index depth (something the
        pure-Python original's fully-generic ``T`` incidentally allows) -- \ref get always requires
        a value for every non-version index :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        No result caching (unlike the pure-Python original's ``@lru_cache`` on its
        version-resolution and lookup methods) -- that cache exists there to amortize repeated
        `pandas`_ boolean-mask filtering, a cost this class's hash-map-plus-per-group-binary-search
        design doesn't have in the first place. \ref get is already O(1) average for the group
        lookup plus O(log group size) for the version search

     .. note::
        \ref get's version resolution deliberately does **not** replicate a bug in the pure-Python
        original: querying for a version that has an *exact* match in the data incorrectly returns
        the *previous* version's data there (confirmed empirically against the live pure-Python
        class), except when that exact match happens to be the smallest available version. This
        class's version resolution is the corrected, inclusive-floor version -- the same algorithm
        :cpp:class:`VersionSet`'s own (already-correct) `findClosest` uses: an exact match returns
        itself; otherwise the largest available version below the target; otherwise (target smaller
        than every available version) the smallest available version
     @endrst
     *
     * @tparam K The type for an index value
     * @tparam T The type for the leaf content of an asset
     * @tparam KeyHash The hash function for index values
     * @tparam KeyEqual The equality function for index values
     */
    template <typename K, typename T, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class ModDictAssets {
        public:

            /**
             * @brief
             @rst
             A function that parses an index value into a :cpp:class:`Version`, used only for the
             raw value found at #getVersionIndexPos in each :cpp:class:`Row` :raw-html:`<br />` :raw-html:`<br />`

             Returning ``std::nullopt`` signals that a specific value could not be parsed as a
             version -- \ref addRows treats that as malformed input and throws
             ``std::invalid_argument`` rather than silently dropping or corrupting a group's sort
             order
             @endrst
             */
            using VersionParser = std::function<std::optional<Version>(const K&)>;

            /**
             * @brief Constructs a new asset lookup table
             *
             * @param totalIndices The total number of index columns (including the version index)
             * @param versionIndexPos The position (0-based) of the version index within a row's
             *      #Row::indexVals
             * @param parseVersion See #VersionParser
             * @param rows The initial rows to populate the table with -- see \ref addRows
             *
             * @throws std::invalid_argument If 'totalIndices' is ``0``, if 'versionIndexPos' is
             *      not a valid position within 'totalIndices', or if any row in 'rows' fails
             *      \ref addRows's own validation
             */
            ModDictAssets(std::size_t totalIndices, std::size_t versionIndexPos, VersionParser parseVersion, std::vector<Row<K, T>> rows = {});

            /**
             * @brief
             @rst
             Adds new rows to the table, overwriting the value of any row whose full key (every
             non-version index value, plus its parsed version) already exists -- matching the
             pure-Python original's ``addRepo``/``_updateAssetContent`` default of "new data
             replaces old data" for an identical full key path :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             *
             * @param newRows The rows to add
             *
             * @throws std::invalid_argument If any row's #Row::indexVals size doesn't match the
             *      table's #getTotalIndices, or if a row's version index value fails to parse
             *      (see #VersionParser)
             */
            void addRows(std::vector<Row<K, T>> newRows);

            /**
             * @brief Retrieves the corresponding asset
             *
             * @param nonVersionVals The values of every index column that does not refer to a
             *      version, in index order (with the version column's position skipped) -- must
             *      have exactly #getTotalIndices ``- 1`` elements
             * @param version
             @rst
             The specific version to query the asset :raw-html:`<br />` :raw-html:`<br />`

             If this value is ``std::nullopt``, the latest available version for the matching
             non-version values is used. Otherwise, the closest available version is used -- see
             the class-level note on this being the corrected, inclusive-floor version resolution
             @endrst
             * @param errorOnNotFound Whether to throw if no matching asset is found
             *
             * @throws std::invalid_argument If 'nonVersionVals' does not have exactly
             *      #getTotalIndices ``- 1`` elements
             * @throws std::out_of_range If no matching asset is found and 'errorOnNotFound' is
             *      ``true``
             *
             * @return The found asset, or ``std::nullopt`` if none is found and 'errorOnNotFound'
             *      is ``false``
             */
            std::optional<T> get(const std::vector<K>& nonVersionVals, const std::optional<Version>& version = std::nullopt, bool errorOnNotFound = true) const;

            /**
             * @brief The total number of index columns (including the version index)
             */
            std::size_t getTotalIndices() const;

            /**
             * @brief The position (0-based) of the version index within a row's #Row::indexVals
             */
            std::size_t getVersionIndexPos() const;

            /**
             * @brief The total number of rows currently in the table, across every non-version
             *      index group
             */
            std::size_t size() const;

            /**
             * @brief
             @rst
             Visits every row currently in the table -- the mechanism :cpp:class:`ModMappedAssets`
             uses to build its reverse (value -> key) index directly from this table's already-flat
             data, instead of re-walking a nested dict the way the pure-Python
             ``ModMappedAssets.updateKeys`` does (see :cpp:class:`ModMappedAssets`'s class-level
             note for why that matters)
             @endrst
             *
             * @tparam Visitor A callable of shape ``void(const std::vector<K>& nonVersionVals,
             *      const Version& version, const T& value)``
             *
             * @param visitor Called once per row, in unspecified order (rows are stored in an
             *      unordered map internally -- see #getTotalIndices's sibling accessors for what
             *      is and isn't order-preserving)
             */
            template <typename Visitor>
            void forEachEntry(Visitor&& visitor) const;

        private:
            struct VersionedEntry {
                Version version;
                T value;
            };

            struct KeyVecHash {
                std::size_t operator()(const std::vector<K>& keys) const;
            };

            struct KeyVecEqual {
                bool operator()(const std::vector<K>& a, const std::vector<K>& b) const;
            };

            std::size_t totalIndices_;
            std::size_t versionIndexPos_;
            VersionParser parseVersion_;

            /**
             * @brief
             @rst
             Maps a row's non-version index values to that group's entries, each sorted ascending
             by :cpp:class:`Version` -- the flat, dict-free representation described in the
             class-level note
             @endrst
             */
            std::unordered_map<std::vector<K>, std::vector<VersionedEntry>, KeyVecHash, KeyVecEqual> groups_;

            std::vector<K> extractNonVersionVals(const std::vector<K>& indexVals) const;
    };
}

#include "ModDictAssets.tpp"

#endif
