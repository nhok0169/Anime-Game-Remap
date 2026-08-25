#ifndef AGRemapCore_FileStats_H
#define AGRemapCore_FileStats_H

#include <exception>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Keeps track of different types of files encountered by the program :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``FileStats`` class (``model/stats/FileStats.py``). A skipped file's
     exception is stored as ``std::exception_ptr`` (rather than a fixed exception type) so any
     exception type caught at the call site can be kept around for later, matching Python's own
     "any exception object" flexibility
     @endrst
     */
    class FileStats {
        public:

            /**
             * @brief The paths to the fixed files
             */
            std::unordered_set<std::string> fixed;

            /**
             * @brief The exceptions tied to file paths that were skipped due to errors
             */
            std::unordered_map<std::string, std::exception_ptr> skipped;

            /**
             * @brief
             @rst
             The exceptions tied to file paths that were skipped due to errors, grouped by mod
             folder path :raw-html:`<br />` :raw-html:`<br />`

             * The outer keys are the mod folder paths
             * The inner keys are the file paths
             * The inner values are the errors encountered :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Mirrors the Python original's ``DefaultDict``-based ``skippedByMods`` --
                ``skippedByMods[modFolder]`` auto-vivifies an empty inner map the same way a
                ``std::unordered_map``'s ``operator[]`` already does, so no separate default-dict
                wrapper is needed here
             @endrst
             */
            std::unordered_map<std::string, std::unordered_map<std::string, std::exception_ptr>> skippedByMods;

            /**
             * @brief The file paths for files that got removed
             */
            std::unordered_set<std::string> removed;

            /**
             * @brief The file paths for files that got undone to a previous state before the software was run
             */
            std::unordered_set<std::string> undoed;

            /**
             * @brief The file paths for files that got visited when attempting to remove those files
             */
            std::unordered_set<std::string> visitedAtRemoval;

            virtual ~FileStats() = default;

            /**
             * @brief Clears out all saved data about the files
             */
            virtual void clear();

            /**
             * @brief Updates the fixed file paths
             *
             * @param newFixed The newly added file paths that got fixed
             */
            void updateFixed(const std::unordered_set<std::string>& newFixed);

            /**
             * @brief Adds a file path to the paths of fixed files
             *
             * @param filePath The new file path to a fixed file
             */
            void addFixed(const std::string& filePath);

            /**
             * @brief
             @rst
             Updates the file paths that got skipped due to errors :raw-html:`<br />` :raw-html:`<br />`

             If 'modFolder' is ``std::nullopt``, the mod folder for each entry in 'newSkipped' is
             instead read from that entry's own file path (see #addSkipped)
             @endrst
             *
             * @param newSkipped The newly skipped file paths (and their errors), due to errors within a particular mod folder
             * @param modFolder The folder where the files got skipped
             */
            void updateSkipped(const std::unordered_map<std::string, std::exception_ptr>& newSkipped, std::optional<std::string> modFolder = std::nullopt);

            /**
             * @brief
             @rst
             Adds a new file path to the paths of skipped files :raw-html:`<br />` :raw-html:`<br />`

             If 'modFolder' is ``std::nullopt``, the mod folder is read from 'filePath''s own parent
             directory
             @endrst
             *
             * @param filePath The new file path that got skipped
             * @param error The exception that caused the file to be skipped
             * @param modFolder The mod folder that contains the file path
             */
            void addSkipped(const std::string& filePath, std::exception_ptr error, std::optional<std::string> modFolder = std::nullopt);

            /**
             * @brief Updates the file paths that got removed
             *
             * @param newRemoved The newly updated file paths that got removed
             */
            void updateRemoved(const std::unordered_set<std::string>& newRemoved);

            /**
             * @brief Adds a new file path that got removed
             *
             * @param filePath The file path that got removed
             */
            void addRemoved(const std::string& filePath);

            /**
             * @brief Updates the file paths whose contents got undone to a previous state before the software was run
             *
             * @param newUndoed The newly updated file paths that got their contents undone
             */
            void updateUndoed(const std::unordered_set<std::string>& newUndoed);

            /**
             * @brief Adds a new file path that got undone
             *
             * @param filePath The file path that got undone
             */
            void addUndoed(const std::string& filePath);

            /**
             * @brief Updates the file paths that got visited when the software attempts to remove those files
             *
             * @param newVisitedAtRemoval The newly updated file paths that got visited
             */
            void updateVisitedAtRemoval(const std::unordered_set<std::string>& newVisitedAtRemoval);

            /**
             * @brief Adds a new file path that got visited when the software attempts to remove the file
             *
             * @param filePath The file path that got visited
             */
            void addVisitedAtRemoval(const std::string& filePath);

            /**
             * @brief
             @rst
             Updates the overall file paths in this class -- see #updateFixed, #updateSkipped, and
             #updateRemoved for more details
             @endrst
             *
             * @param modFolder The folder where the files got skipped
             * @param newFixed The newly updated file paths that got fixed
             * @param newSkipped The newly skipped file paths (and their errors), within 'modFolder'
             * @param newRemoved The newly updated file paths that got removed
             * @param newUndoed The newly updated file paths that got their contents undone
             * @param newVisitedAtRemoval The newly updated file paths that got visited during removal
             */
            void update(std::optional<std::string> modFolder = std::nullopt,
                        std::optional<std::unordered_set<std::string>> newFixed = std::nullopt,
                        std::optional<std::unordered_map<std::string, std::exception_ptr>> newSkipped = std::nullopt,
                        std::optional<std::unordered_set<std::string>> newRemoved = std::nullopt,
                        std::optional<std::unordered_set<std::string>> newUndoed = std::nullopt,
                        std::optional<std::unordered_set<std::string>> newVisitedAtRemoval = std::nullopt);
    };
}

#endif
