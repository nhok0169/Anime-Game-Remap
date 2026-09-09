#include "AGRemapCore/constants/StrategyOverrides.h"

#include <unordered_map>
#include <utility>


namespace AGRemapCore {
    namespace {
        struct ParseEntry {
            std::optional<Version> versionKey;
            StrategyOverrides::ParseFactory factory;

            const std::optional<Version>& version() const { return versionKey; }
        };

        struct FixEntry {
            std::string toModName;
            std::optional<Version> fromVersion;
            StrategyOverrides::FixFactory factory;

            const std::optional<Version>& version() const { return fromVersion; }
        };

        // Function-local statics rather than namespace-level ones: the same lazy, exactly-once,
        // thread-safe initialization GlobalIniRemoveBuilders and GlobalModTypes use, and it keeps
        // these out of static-initialization order entirely.
        std::unordered_map<std::string, std::vector<ParseEntry>>& parseOverrides() {
            static std::unordered_map<std::string, std::vector<ParseEntry>> overrides;
            return overrides;
        }

        std::unordered_map<std::string, std::vector<FixEntry>>& fixOverrides() {
            static std::unordered_map<std::string, std::vector<FixEntry>> overrides;
            return overrides;
        }

        /**
         * Whether a versioned entry is a candidate for the version asked for.
         *
         * A request WITHOUT a version means "the newest one", so every entry is a candidate and the
         * highest wins. A request WITH one floor-matches: an entry at or below it applies, exactly
         * as ModDictAssets::get resolves the built-in tables.
         *
         * Exact matching was tried first and is wrong for what this class is for: a run normally
         * passes no version at all, so an override registered for "6.1" would never fire on an
         * ordinary run -- which is the case "override Raiden 6.1" means.
         */
        bool versionCandidate(const Version& entryVersion, const std::optional<Version>& asked) {
            return !asked.has_value() || entryVersion <= *asked;
        }

        /**
         * The best versioned entry, or nullptr. 'entries' is small (one prototyping session's worth),
         * so a linear scan for the maximum beats keeping it sorted.
         */
        template <typename Entry, typename Pred>
        const Entry* bestVersioned(const std::vector<Entry>& entries,
                                    const std::optional<Version>& asked, Pred matchesKey) {
            const Entry* best = nullptr;

            for (const Entry& entry : entries) {
                if (!matchesKey(entry) || !entry.version().has_value()) {
                    continue;
                }
                if (!versionCandidate(*entry.version(), asked)) {
                    continue;
                }
                if (best == nullptr || *best->version() < *entry.version()) {
                    best = &entry;
                }
            }

            return best;
        }
    }


    void StrategyOverrides::setParser(std::string modName, std::optional<Version> version,
                                       ParseFactory factory) {
        std::vector<ParseEntry>& entries = parseOverrides()[std::move(modName)];

        // Replacing rather than appending, so registering twice for the same version does not leave
        // the first one shadowing the second depending on scan order.
        for (ParseEntry& entry : entries) {
            if (entry.versionKey == version) {
                entry.factory = std::move(factory);
                return;
            }
        }

        entries.push_back(ParseEntry{std::move(version), std::move(factory)});
    }


    void StrategyOverrides::setFixer(std::string fromModName, std::string toModName,
                                      std::optional<Version> fromVersion, FixFactory factory) {
        std::vector<FixEntry>& entries = fixOverrides()[std::move(fromModName)];

        for (FixEntry& entry : entries) {
            if (entry.toModName == toModName && entry.fromVersion == fromVersion) {
                entry.factory = std::move(factory);
                return;
            }
        }

        entries.push_back(FixEntry{std::move(toModName), std::move(fromVersion), std::move(factory)});
    }


    std::optional<StrategyOverrides::ParseFactory> StrategyOverrides::findParser(
            const std::string& modName, const std::optional<Version>& version) {
        auto found = parseOverrides().find(modName);
        if (found == parseOverrides().end()) {
            return std::nullopt;
        }

        const ParseEntry* best = bestVersioned(found->second, version,
                                                [](const ParseEntry&) { return true; });
        if (best != nullptr) {
            return best->factory;
        }

        // A versionless override is the fallback, used only when no versioned one applied.
        for (const ParseEntry& entry : found->second) {
            if (!entry.versionKey.has_value()) {
                return entry.factory;
            }
        }

        return std::nullopt;
    }


    std::optional<StrategyOverrides::FixFactory> StrategyOverrides::findFixer(
            const std::string& fromModName, const std::string& toModName,
            const std::optional<Version>& fromVersion) {
        auto found = fixOverrides().find(fromModName);
        if (found == fixOverrides().end()) {
            return std::nullopt;
        }

        const FixEntry* best = bestVersioned(found->second, fromVersion,
                                              [&toModName](const FixEntry& entry) {
                                                  return entry.toModName == toModName;
                                              });
        if (best != nullptr) {
            return best->factory;
        }

        for (const FixEntry& entry : found->second) {
            if (entry.toModName == toModName && !entry.fromVersion.has_value()) {
                return entry.factory;
            }
        }

        return std::nullopt;
    }


    std::vector<std::string> StrategyOverrides::fixerTargets(const std::string& fromModName) {
        std::vector<std::string> result;

        auto found = fixOverrides().find(fromModName);
        if (found == fixOverrides().end()) {
            return result;
        }

        for (const FixEntry& entry : found->second) {
            bool seen = false;
            for (const std::string& name : result) {
                if (name == entry.toModName) {
                    seen = true;
                    break;
                }
            }

            if (!seen) {
                result.push_back(entry.toModName);
            }
        }

        return result;
    }


    void StrategyOverrides::clear() {
        parseOverrides().clear();
        fixOverrides().clear();
    }


    bool StrategyOverrides::empty() {
        return parseOverrides().empty() && fixOverrides().empty();
    }
}
