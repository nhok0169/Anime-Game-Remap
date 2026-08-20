#ifndef AGRemapCore_ModMappedAssets_H
#define AGRemapCore_ModMappedAssets_H

#include <cstddef>
#include <functional>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModDictAssets.h"
#include "AGRemapCore/model/assets/Row.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class to handle assets of any type where asset retrieval is based on a mapping -- the C++
     counterpart to the pure-Python ``ModMappedAssets`` (``model/assets/ModMappedAssets.py``) --
     this is a `bipartite graph`_ that maps assets to fix from to assets to fix to :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The reverse (value -> originating key) index this class builds is derived directly from
        #getRepo's already-flat :cpp:func:`ModDictAssets::forEachEntry` data, **not** by re-walking
        a nested dict the way the pure-Python ``updateKeys`` does. This isn't just a style
        preference: a live side-by-side check against the real pure-Python ``ModMappedAssets``
        during development (two names sharing one hash value at the same version) found that its
        stack-based nested-dict traversal actually **corrupts** a sibling candidate into
        ``NaN``/``NaN`` and silently drops it from the reverse index whenever a value is shared
        across more than one name at the same version -- a real, confirmed bug, not a hypothetical
        one. Building the reverse index from the flat, already-tested :cpp:class:`ModDictAssets`
        data instead avoids that whole bug class structurally, not just incidentally
     @endrst
     *
     * @tparam K The type for an index value
     * @tparam T The type for the leaf content of an asset
     * @tparam KeyHash The hash function for index values
     * @tparam KeyEqual The equality function for index values
     * @tparam ValueHash The hash function for asset values (the leaf content, ``T``)
     * @tparam ValueEqual The equality function for asset values
     */
    template <typename K, typename T, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>, typename ValueHash = std::hash<T>, typename ValueEqual = std::equal_to<T>>
    class ModMappedAssets {
        public:

            /**
             * @brief Constructs a new mapped asset table
             *
             * @param repo The underlying asset data -- see #getRepo
             * @param map The initial `adjacency list`_ mapping assets to fix from to assets to
             *      fix to -- see #getMap
             */
            explicit ModMappedAssets(ModDictAssets<K, T, KeyHash, KeyEqual> repo, std::unordered_map<K, std::vector<K>, KeyHash, KeyEqual> map = {});

            /**
             * @brief
             @rst
             Adds new rows to #getRepo, then rebuilds the reverse index (see the class-level note)
             to reflect them :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             *
             * @param newRows The rows to add -- see :cpp:func:`ModDictAssets::addRows`
             */
            void addRepoRows(std::vector<Row<K, T>> newRows);

            /**
             * @brief
             @rst
             Merges new entries into the existing `adjacency list`_ (see #getMap) -- for any
             ``fromAsset`` already present, new ``toAsset`` values are appended after the existing
             ones, skipping any that are already present (a set-union that preserves insertion
             order, matching the pure-Python original's `OrderedSet`_-based ``addMap``)
             @endrst
             *
             * @param assetMap The new adjacency entries to merge in
             * @param newRows Any new rows needed to support 'assetMap' -- if non-empty, added to
             *      #getRepo first (see \ref addRepoRows)
             */
            void addMap(const std::unordered_map<K, std::vector<K>, KeyHash, KeyEqual>& assetMap, std::vector<Row<K, T>> newRows = {});

            /**
             * @brief Retrieves the corresponding asset -- forwards directly to
             *      :cpp:func:`ModDictAssets::get` on #getRepo
             */
            std::optional<T> get(const std::vector<K>& nonVersionVals, const std::optional<Version>& version = std::nullopt, bool errorOnNotFound = true) const;

            /**
             * @brief
             @rst
             Determines whether 'asset' exists in the assets to map from -- a cheap wrapper over
             \ref getKey (``errorOnNotFound = false``)
             @endrst
             *
             * @param asset The asset to search for
             * @param version The version to search from -- see \ref getKey
             * @param nonVersionVals The non-version value filter -- see \ref getKey
             */
            bool hasFrom(const T& asset, const std::optional<Version>& version = std::nullopt, const std::vector<std::optional<K>>& nonVersionVals = {}) const;

            /**
             * @brief
             @rst
             Retrieves the key that produced 'asset', disambiguating between multiple candidates
             (a single asset value can originate from more than one row -- e.g. a texture hash
             shared by several characters at the same version) via 'fromNonVersionVals' -- the
             first remaining candidate wins if more than one still matches after filtering
             @endrst
             *
             * @param asset The asset value to search for
             * @param fromVersion
             @rst
             The version to search from :raw-html:`<br />` :raw-html:`<br />`

             If ``std::nullopt``, the latest version 'asset' is available at is used. Otherwise,
             the closest available version is used (the same corrected, inclusive-floor
             resolution as :cpp:class:`ModDictAssets`'s own version search -- an exact match
             returns itself)
             @endrst
             * @param fromNonVersionVals
             @rst
             A per-position filter over the candidate keys' non-version index values --
             ``std::nullopt`` at a position means "match any value there" :raw-html:`<br />` :raw-html:`<br />`

             An empty vector means "no filtering at all" (every position wildcarded); otherwise
             this must have exactly #getRepo's ``getTotalIndices() - 1`` elements
             @endrst
             * @param errorOnNotFound Whether to throw if no matching key is found
             *
             * @throws std::invalid_argument If 'fromNonVersionVals' is non-empty and doesn't have
             *      exactly #getRepo's ``getTotalIndices() - 1`` elements
             * @throws std::out_of_range If no matching key is found and 'errorOnNotFound' is
             *      ``true``
             *
             * @return
             @rst
             The non-version index values identifying the originating row (the first element is
             always the "name" value -- see the class-level note on index ordering), or
             ``std::nullopt`` if none is found and 'errorOnNotFound' is ``false`` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Earlier drafts of this method also returned the specific version the key was
                found at, alongside the key itself. Dropped deliberately: the pure-Python
                original this replaces returns just the bare key, and at least one real caller
                (``GIMIParser.py``'s hash/index resolution) destructures the result positionally
                (``key[-1]``) assuming exactly that shape -- returning anything richer would
                silently corrupt that caller rather than erroring. \ref replace/\ref replaceAll
                still resolve the version internally (via a private overload) since they need it;
                it just isn't part of this method's own public return value
             @endrst
             */
            std::optional<std::vector<K>> getKey(const T& asset, const std::optional<Version>& fromVersion, const std::vector<std::optional<K>>& fromNonVersionVals, bool errorOnNotFound = true) const;

            /**
             * @brief Retrieves the single corresponding asset to replace 'asset' with, for one
             *      specific target asset name
             *
             * @param asset The asset to be replaced
             * @param fromVersion The version to replace from -- see \ref getKey
             * @param fromNonVersionVals The non-version value filter -- see \ref getKey
             * @param toVersion The version to replace to -- ``std::nullopt`` for the latest
             * @param toAssetName The specific name of the asset to map to
             * @param errorOnNotFound
             @rst
             Whether to throw if 'asset' (or a mapping for it) isn't found at all :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This governs only the initial lookup of 'asset' itself and of its mapping --
                unlike the pure-Python original (where the equivalent "asset's fromAsset isn't in
                the map at all" case always raises regardless of the ``errorOnNotFound`` argument,
                seemingly inconsistently with every other failure path in the same method), this
                deliberately makes every failure path respect 'errorOnNotFound' uniformly. Once
                past that point, "toAssetName isn't actually mapped from asset's name" or "no data
                exists for it at the queried version" always just returns ``std::nullopt``,
                'errorOnNotFound' notwithstanding -- those aren't failures to find 'asset', they're
                just "there is nothing there"
             @endrst
             *
             * @return The replacement asset, or ``std::nullopt`` if none is found
             */
            std::optional<T> replace(const T& asset, const std::optional<Version>& fromVersion, const std::vector<std::optional<K>>& fromNonVersionVals, const std::optional<Version>& toVersion, const K& toAssetName, bool errorOnNotFound = true) const;

            /**
             * @brief Retrieves every corresponding asset to replace 'asset' with
             *
             * @param asset The asset to be replaced
             * @param fromVersion The version to replace from -- see \ref getKey
             * @param fromNonVersionVals The non-version value filter -- see \ref getKey
             * @param toVersion The version to replace to -- ``std::nullopt`` for the latest
             * @param toAssetNames
             @rst
             The specific names of the assets to map to -- an empty vector means "every asset
             name 'asset' maps to" :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             * @param errorOnNotFound See \ref replace's note on this parameter
             *
             * @return
             @rst
             The corresponding assets for the fix to replace, keyed by asset name :raw-html:`<br />` :raw-html:`<br />`

             Empty if 'asset' (or its mapping) isn't found and 'errorOnNotFound' is ``false``, if
             nothing in 'toAssetNames' is actually mapped from 'asset', or if none of the mapped
             names have data at the queried version
             @endrst
             */
            std::unordered_map<K, T, KeyHash, KeyEqual> replaceAll(const T& asset, const std::optional<Version>& fromVersion, const std::vector<std::optional<K>>& fromNonVersionVals, const std::optional<Version>& toVersion, const std::vector<K>& toAssetNames = {}, bool errorOnNotFound = true) const;

            /**
             * @brief Every asset value that has at least one known originating key -- i.e. every
             *      leaf value present in #getRepo
             */
            std::vector<T> getFromAssets() const;

            /**
             * @brief The underlying asset data
             */
            const ModDictAssets<K, T, KeyHash, KeyEqual>& getRepo() const;

            /**
             * @brief The `adjacency list`_ mapping assets to fix from to assets to fix to
             */
            const std::unordered_map<K, std::vector<K>, KeyHash, KeyEqual>& getMap() const;

        private:
            struct VersionBucket {
                Version version;
                std::vector<std::vector<K>> candidates;
            };

            /**
             * @brief The result of a successful internal key lookup -- see #getKeyInternal
             */
            struct InternalKeyResult {
                Version version;
                std::vector<K> nonVersionVals;
            };

            ModDictAssets<K, T, KeyHash, KeyEqual> repo_;
            std::unordered_map<K, std::vector<K>, KeyHash, KeyEqual> map_;

            /**
             * @brief
             @rst
             Maps an asset value to its version-sorted buckets of originating candidate keys --
             see the class-level note on how (and why) this is built
             @endrst
             */
            std::unordered_map<T, std::vector<VersionBucket>, ValueHash, ValueEqual> keys_;

            void rebuildKeys();
            std::optional<std::vector<K>> resolveToAssetNames(const K& fromAsset, const std::vector<K>& toAssetsFilter) const;

            /**
             * @brief The real implementation behind #getKey -- also used internally by
             *      \ref replace/\ref replaceAll, which need the resolved version too (to avoid
             *      re-resolving it a second time) even though #getKey itself doesn't expose it
             */
            std::optional<InternalKeyResult> getKeyInternal(const T& asset, const std::optional<Version>& fromVersion, const std::vector<std::optional<K>>& fromNonVersionVals, bool errorOnNotFound) const;
    };
}

#include "ModMappedAssets.tpp"

#endif
