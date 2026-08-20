#ifndef AGRemapCore_VersionSet_H
#define AGRemapCore_VersionSet_H

#include <cstddef>
#include <optional>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/tools/caches/LruCache.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class for tracking a set of available :cpp:class:`Version` s and finding the closest
     available version to some queried version :raw-html:`<br />` :raw-html:`<br />`

     This is the C++ counterpart to the pure-Python ``Version`` class (``model/Version.py``) --
     renamed here since that name is used on the Python side for both "one version value" (this
     library's :cpp:class:`Version`) and "a searchable collection of them" (this class); splitting
     the two concepts avoids that overload on the C++ side
     @endrst
     */
    class VersionSet {
        public:

            /**
             * @brief Constructs a new, empty set of versions
             *
             * @param cacheCapacity The capacity of the internal closest-version cache used by
             *      \ref findClosest
             */
            explicit VersionSet(std::size_t cacheCapacity = LruCache<Version, Version>::DefaultCapacity);

            /**
             * @brief
             @rst
             Adds a new available version, keeping the internal version list sorted and
             deduplicated :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                 Matching the pure-Python original: this does **not** invalidate the
                 closest-version cache used by :cpp:func:`findClosest`. A version added after a
                 query has already been cached for some target can leave that cached result
                 stale (no longer the true closest match) until :cpp:func:`clear` is called. This
                 is an inherited quirk, not a deliberate design choice -- call :cpp:func:`clear`
                 first if a caller needs strict re-computation after adding versions
             @endrst
             *
             * @param newVersion The new version to add
             */
            void add(const Version& newVersion);

            /**
             * @brief Clears all version data, including the closest-version cache
             */
            void clear();

            /**
             * @brief Getter for the latest (largest) version available
             *
             * @return The latest version, or ``std::nullopt`` if no versions have been added
             */
            const std::optional<Version>& getLatestVersion() const;

            /**
             * @brief Retrieves all the available versions, in sorted ascending order
             */
            const std::vector<Version>& getVersions() const;

            /**
             * @brief
             @rst
             Finds the closest available version to 'version' :raw-html:`<br />` :raw-html:`<br />`

             If 'version' is ``std::nullopt``, returns the latest available version. Otherwise,
             returns the largest available version :math:`\leq` 'version' -- or, if every
             available version is greater than 'version', the smallest available version
             (matching the pure-Python original's fallback behaviour rather than returning
             ``std::nullopt`` in that case)
             @endrst
             *
             * @param version The version to search for, or ``std::nullopt`` for the latest version
             * @param fromCache Whether to use (and populate) the internal closest-version cache
             *
             * @return The closest available version, or ``std::nullopt`` if no versions have been added
             */
            std::optional<Version> findClosest(const std::optional<Version>& version, bool fromCache = true);

            /**
             * @brief Finds the closest version to 'version' from an already-sorted list of versions
             *
             * @param versions The sorted (ascending), non-empty list of versions to search
             * @param version The version to search for
             *
             * @return The closest version in 'versions' to 'version'
             */
            static Version findClosestFromSorted(const std::vector<Version>& versions, const Version& version);

        private:
            std::vector<Version> versions_;
            std::optional<Version> latestVersion_;
            LruCache<Version, Version> cache_;

            void updateLatestVersion(const Version& newVersion);
    };
}

#endif
