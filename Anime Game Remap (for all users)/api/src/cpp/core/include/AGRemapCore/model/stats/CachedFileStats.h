#ifndef AGRemapCore_CachedFileStats_H
#define AGRemapCore_CachedFileStats_H

#include <exception>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "AGRemapCore/model/stats/FileStats.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`FileStats`

     Adds tracking for a file retrieved via a cache hit, on top of what :cpp:class:`FileStats`
     already tracks -- mirrors the pure-Python ``CachedFileStats`` class (``model/stats/CachedFileStats.py``)
     @endrst
     */
    class CachedFileStats: public FileStats {
        public:

            /**
             * @brief The paths to the files retrieved during a cache hit
             */
            std::unordered_set<std::string> hit;

            void clear() override;

            /**
             * @brief Updates the file paths that have a cache hit
             *
             * @param newHit The new file paths that got a hit
             */
            void updateHit(const std::unordered_set<std::string>& newHit);

            /**
             * @brief Adds a new file path to the paths of cache hit files
             *
             * @param filePath The new file path that was hit
             */
            void addHit(const std::string& filePath);

            /**
             * @brief
             @rst
             Same as :cpp:func:`FileStats::update`, with an additional 'newHit' argument -- see
             :cpp:func:`FileStats::update` for the other parameters
             @endrst
             *
             * @param newHit The new file paths that got a cache hit
             */
            void update(std::optional<std::string> modFolder = std::nullopt,
                        std::optional<std::unordered_set<std::string>> newFixed = std::nullopt,
                        std::optional<std::unordered_map<std::string, std::exception_ptr>> newSkipped = std::nullopt,
                        std::optional<std::unordered_set<std::string>> newRemoved = std::nullopt,
                        std::optional<std::unordered_set<std::string>> newUndoed = std::nullopt,
                        std::optional<std::unordered_set<std::string>> newVisitedAtRemoval = std::nullopt,
                        std::optional<std::unordered_set<std::string>> newHit = std::nullopt);
    };
}

#endif
