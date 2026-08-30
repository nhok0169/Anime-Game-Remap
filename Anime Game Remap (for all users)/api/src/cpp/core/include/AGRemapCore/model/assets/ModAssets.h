#ifndef AGRemapCore_ModAssets_H
#define AGRemapCore_ModAssets_H

#include <cstddef>
#include <functional>
#include <optional>
#include <stdexcept>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class to handle assets of any type for a mod where retrieval is based on some keys where one
     *or more* of the keys refer to some versioning -- the C++ counterpart to the pure-Python
     ``ModAssets`` (``model/assets/ModAssets.py``) :raw-html:`<br />` :raw-html:`<br />`

     .. tip::
        If an asset has only one version column, :cpp:class:`ModDictAssets` is the better fit (a
        real hash-map lookup instead of this class's linear scan). This class exists specifically
        for the multi-version-column case (e.g. this project's real ``VGRemaps``, which resolves
        a ``fromVersion`` and a ``toVersion`` independently)

     .. note::
        Unlike :cpp:class:`ModDictAssets`/:cpp:class:`ModMappedAssets`, this class does a plain
        linear scan per query rather than building any index -- deliberately, not as an
        unfinished optimization. Every real table this class backs (this project's ``VGRemaps``,
        ``VertexCounts``, ``PositionEditors``, ``IniFixBuilderArgs``, ``IniParseBuilderArgs``) is
        on the order of dozens to a few hundred rows, and the query shape itself (progressively
        narrow a candidate set by a growing number of independent, sequentially-resolved version
        columns) doesn't map cleanly onto a single flat hash index the way
        :cpp:class:`ModDictAssets`'s single-version-column case does. A linear scan over this
        table's real size is already far cheaper than the `pandas`_-based original it replaces
     @endrst
     *
     * @tparam K The type for an index value
     * @tparam T The type for the leaf content of an asset
     * @tparam KeyEqual The equality function for index values
     */
    template <typename K, typename T, typename KeyEqual = std::equal_to<K>>
    class ModAssets {
        public:

            /**
             * @brief See :cpp:class:`ModDictAssets::VersionParser` -- same role here, applied to
             *      each version column's raw value independently
             */
            using VersionParser = std::function<std::optional<Version>(const K&)>;

            /**
             * @brief Constructs a new asset lookup table
             *
             * @param isVersionColumn
             @rst
             One entry per index column, in index order -- ``true`` marks that column as a
             version column. At least one entry must be given (an empty 'isVersionColumn' is
             invalid -- there must be at least one index column, version or not)
             @endrst
             * @param parseVersion See #VersionParser
             * @param rows The initial rows to populate the table with -- see \ref addRows
             *
             * @throws std::invalid_argument If 'isVersionColumn' is empty, or if any row in
             *      'rows' fails \ref addRows's own validation
             */
            ModAssets(std::vector<bool> isVersionColumn, VersionParser parseVersion, std::vector<Row<K, T>> rows = {});

            /**
             * @brief Adds new rows to the table (an addition beyond the pure-Python original,
             *      which has no incremental-add capability at all -- only whole-table
             *      replacement via reassigning its ``repo`` property) -- overwrites the value of
             *      any row whose full key (every column's value) already exists
             *
             * @param newRows The rows to add
             *
             * @throws std::invalid_argument If any row's \ref Row::indexVals size doesn't match
             *      #getTotalIndices, or if a version column's raw value fails to parse
             */
            void addRows(std::vector<Row<K, T>> newRows);

            /**
             * @brief Retrieves the corresponding asset
             *
             * @param nonVersionVals
             @rst
             One entry per non-version column, in their relative index order (version columns
             skipped) -- ``std::nullopt`` at a position means "match any value there". Must have
             exactly #getNonVersionColumnCount elements
             @endrst
             * @param versionVals
             @rst
             One entry per version column, in their relative index order -- ``std::nullopt`` at a
             position means "use the latest available value for this column, among rows still
             matching everything resolved so far". Must have exactly #getVersionColumnCount
             elements :raw-html:`<br />` :raw-html:`<br />`

             Version columns are resolved **sequentially, in index order** -- each one's
             floor-match (largest available value :math:`\leq` the target; the smallest available
             value if none qualifies) narrows the candidate set before the next version column is
             resolved against it. This matches the pure-Python original's own progressive
             DataFrame-filtering order exactly
             @endrst
             * @param errorOnNotFound Whether to throw if no matching asset is found
             *
             * @throws std::invalid_argument If 'nonVersionVals'/'versionVals' don't have exactly
             *      #getNonVersionColumnCount/#getVersionColumnCount elements respectively
             * @throws std::out_of_range If no matching asset is found and 'errorOnNotFound' is
             *      ``true``
             *
             * @return The found asset, or ``std::nullopt`` if none is found and 'errorOnNotFound'
             *      is ``false``
             */
            std::optional<T> get(const std::vector<std::optional<K>>& nonVersionVals, const std::vector<std::optional<Version>>& versionVals, bool errorOnNotFound = true) const;

            /**
             * @brief
             @rst
             Retrieves **every** matching asset, one per distinct combination of the non-version
             columns left unresolved -- the multi-result counterpart to #get :raw-html:`<br />`
             :raw-html:`<br />`

             Where #get answers "the asset for this key", this answers "every asset reachable by
             leaving some columns free". A ``std::nullopt`` in 'nonVersionVals' is that free axis:
             rows are grouped by their values in those positions, and each group independently
             resolves its version columns exactly the way #get does :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                **Version resolution happens per group, not once globally.** That is the whole point
                -- resolving versions across the merged candidate set first would pick one winning
                version and silently drop every group that has no row at it. Doing it per group
                means each free-column combination contributes its own best version match, so a
                caller fanning out over (say) target mod names gets one result per target mod even
                when those targets are listed at different versions

             .. note::
                Groups come back in no particular order. Sort by the returned key if a stable order
                matters

             .. note::
                There is no ``errorOnNotFound`` here: "nothing matched" is a legitimate, expected
                answer for a fan-out query, so this returns an empty vector rather than throwing
             @endrst
             *
             * @param nonVersionVals One entry per non-version column, in their relative index order
             *      -- ``std::nullopt`` marks a column to fan out over. Must have exactly
             *      #getNonVersionColumnCount elements
             * @param versionVals One entry per version column, in their relative index order --
             *      resolved per group, see #get. Must have exactly #getVersionColumnCount elements
             *
             * @throws std::invalid_argument If 'nonVersionVals'/'versionVals' don't have exactly
             *      #getNonVersionColumnCount/#getVersionColumnCount elements respectively
             *
             * @return One ``(nonVersionVals, asset)`` pair per matching group, with the key given in
             *      full (including the columns that were fixed), so the caller can tell the groups
             *      apart
             */
            std::vector<std::pair<std::vector<K>, T>> getAll(const std::vector<std::optional<K>>& nonVersionVals, const std::vector<std::optional<Version>>& versionVals) const;

            /**
             * @brief The total number of index columns
             */
            std::size_t getTotalIndices() const;

            /**
             * @brief The number of version columns
             */
            std::size_t getVersionColumnCount() const;

            /**
             * @brief The number of non-version columns
             */
            std::size_t getNonVersionColumnCount() const;

            /**
             * @brief The total number of rows currently in the table
             */
            std::size_t size() const;

        private:
            struct StoredRow {
                std::vector<K> nonVersionVals;      // one per non-version column, in relative order
                std::vector<Version> versionVals;   // one per version column, in relative order
                T value;
            };

            std::size_t totalIndices_;
            std::vector<bool> isVersionColumn_;
            std::vector<std::size_t> versionColumnPositions_;
            std::vector<std::size_t> nonVersionColumnPositions_;
            VersionParser parseVersion_;
            std::vector<StoredRow> rows_;

            void addRow(const Row<K, T>& row);

            // Narrows 'candidates' in place by resolving each version column in turn, exactly as
            // get() documents: nullopt means "latest among what's left", otherwise floor-match
            // (largest <= target, else the smallest available). Shared by get() and getAll() so the
            // two can never drift apart on version semantics.
            void resolveVersionColumns(std::vector<const StoredRow*>& candidates, const std::vector<std::optional<Version>>& versionVals) const;
    };
}

#include "ModAssets.tpp"

#endif
