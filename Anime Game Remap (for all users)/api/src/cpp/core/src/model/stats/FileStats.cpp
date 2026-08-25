#include "AGRemapCore/model/stats/FileStats.h"

#include <filesystem>


namespace AGRemapCore {
    void FileStats::clear() {
        fixed.clear();
        skipped.clear();
        skippedByMods.clear();
        removed.clear();
        undoed.clear();
        visitedAtRemoval.clear();
    }

    void FileStats::updateFixed(const std::unordered_set<std::string>& newFixed) {
        fixed.insert(newFixed.begin(), newFixed.end());
    }

    void FileStats::addFixed(const std::string& filePath) {
        fixed.insert(filePath);
    }

    void FileStats::updateSkipped(const std::unordered_map<std::string, std::exception_ptr>& newSkipped, std::optional<std::string> modFolder) {
        if (modFolder.has_value()) {
            for (const auto& entry : newSkipped) {
                skipped[entry.first] = entry.second;
            }

            if (!newSkipped.empty()) {
                auto& modSkipped = skippedByMods[*modFolder];
                for (const auto& entry : newSkipped) {
                    modSkipped[entry.first] = entry.second;
                }
            }

            return;
        }

        for (const auto& entry : newSkipped) {
            addSkipped(entry.first, entry.second, std::nullopt);
        }
    }

    void FileStats::addSkipped(const std::string& filePath, std::exception_ptr error, std::optional<std::string> modFolder) {
        std::string resolvedModFolder = modFolder.has_value() ? *modFolder : std::filesystem::path(filePath).parent_path().string();

        skipped[filePath] = error;
        skippedByMods[resolvedModFolder][filePath] = error;
    }

    void FileStats::updateRemoved(const std::unordered_set<std::string>& newRemoved) {
        removed.insert(newRemoved.begin(), newRemoved.end());
    }

    void FileStats::addRemoved(const std::string& filePath) {
        removed.insert(filePath);
    }

    void FileStats::updateUndoed(const std::unordered_set<std::string>& newUndoed) {
        undoed.insert(newUndoed.begin(), newUndoed.end());
    }

    void FileStats::addUndoed(const std::string& filePath) {
        undoed.insert(filePath);
    }

    void FileStats::updateVisitedAtRemoval(const std::unordered_set<std::string>& newVisitedAtRemoval) {
        visitedAtRemoval.insert(newVisitedAtRemoval.begin(), newVisitedAtRemoval.end());
    }

    void FileStats::addVisitedAtRemoval(const std::string& filePath) {
        visitedAtRemoval.insert(filePath);
    }

    void FileStats::update(std::optional<std::string> modFolder, std::optional<std::unordered_set<std::string>> newFixed,
                            std::optional<std::unordered_map<std::string, std::exception_ptr>> newSkipped,
                            std::optional<std::unordered_set<std::string>> newRemoved,
                            std::optional<std::unordered_set<std::string>> newUndoed,
                            std::optional<std::unordered_set<std::string>> newVisitedAtRemoval) {
        if (newFixed.has_value()) {
            updateFixed(*newFixed);
        }

        if (newSkipped.has_value()) {
            updateSkipped(*newSkipped, modFolder);
        }

        if (newRemoved.has_value()) {
            updateRemoved(*newRemoved);
        }

        if (newUndoed.has_value()) {
            updateUndoed(*newUndoed);
        }

        if (newVisitedAtRemoval.has_value()) {
            updateVisitedAtRemoval(*newVisitedAtRemoval);
        }
    }
}
