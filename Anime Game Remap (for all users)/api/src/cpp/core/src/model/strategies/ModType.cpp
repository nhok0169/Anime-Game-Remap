#include "AGRemapCore/model/strategies/ModType.h"

#include <set>
#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/tools/Heading.h"
#include "AGRemapCore/tools/StringTools.h"

#include "AGRemapCore/data/ModDataAssets.h"

#include "AGRemapCore/constants/GlobalIniRemoveBuilders.h"


namespace AGRemapCore {
    ModType::ModType(int gameTypeId, int modTypeId, const std::string &name, const std::vector<std::string> &aliases,
                      std::shared_ptr<Hashes> hashes, std::shared_ptr<Indices> indices,
                      std::shared_ptr<VertexCounts> vertexCounts, std::shared_ptr<VGRemaps> vgRemaps,
                      std::shared_ptr<IniParseBuilder> iniParseBuilder, std::shared_ptr<IniFixBuilder> iniFixBuilder,
                      std::shared_ptr<IniRemoveBuilder> iniRemoveBuilder):
        gameTypeId(gameTypeId), modTypeId(modTypeId), name(name), aliases(aliases),
        hashes(std::move(hashes)), indices(std::move(indices)), vertexCounts(std::move(vertexCounts)),
        vgRemaps(std::move(vgRemaps)),
        iniParseBuilder(std::move(iniParseBuilder)), iniFixBuilder(std::move(iniFixBuilder)), iniRemoveBuilder(std::move(iniRemoveBuilder)) {
        // Mirrors the pure-Python original's own "if (hashes is None): hashes = Hashes()". Note a
        // default-constructed Hashes is fully populated, so this is a real table, not an empty one.
        // A fresh instance per ModType, exactly as the original does -- see ModType::hashes on why
        // that (rather than one shared table) is the right default.
        if (this->hashes == nullptr) {
            this->hashes = std::make_shared<Hashes>();
        }

        // The same, for the original's "if (indices is None): indices = Indices()".
        if (this->indices == nullptr) {
            this->indices = std::make_shared<Indices>();
        }

        // ...and for "if (vertexCounts is None): vertexCounts = VertexCounts()".
        if (this->vertexCounts == nullptr) {
            this->vertexCounts = std::make_shared<VertexCounts>();
        }

        // vgRemaps is the odd one out: the original falls back to the SHARED
        // "ModDataAssets.VGRemaps.value", not to a fresh VGRemaps(). Mirrored deliberately -- see
        // ModType::vgRemaps' warning about the mutation consequences.
        if (this->vgRemaps == nullptr) {
            this->vgRemaps = ModDataAssets::vgRemaps();
        }

        // Mirrors the pure-Python original's own "if (iniParseBuilder is None): iniParseBuilder =
        // IniParseBuilder(GIMIParser)" fallback -- a default-constructed IniParseBuilder builds a
        // plain BaseIniParser, since no concrete GIMIParser equivalent exists in C++ yet (see
        // IniParseBuilder::defaultFactory, the single place to change when one lands).
        if (this->iniParseBuilder == nullptr) {
            this->iniParseBuilder = std::make_shared<IniParseBuilder>();
        }

        // Same fallback on the fixer side, mirroring the original's own "iniFixBuilder" default.
        if (this->iniFixBuilder == nullptr) {
            this->iniFixBuilder = std::make_shared<IniFixBuilder>();
        }

        // Mirrors the pure-Python original's own
        // "iniRemoveBuilder = GlobalIniRemoveBuilders.RemoveBuilder.value" default. Note this is
        // the *shared* global builder rather than a fresh one, so every ModType that falls back
        // here shares one builder instance -- deliberate, and what the original does too.
        if (this->iniRemoveBuilder == nullptr) {
            this->iniRemoveBuilder = GlobalIniRemoveBuilders::removeBuilder();
        }
    }


    bool ModType::isName(const std::string& name) const {
        // Case-insensitive by grapheme through StringTools (the pure-Python original's own
        // '.lower()' comparison on a Unicode str), so a non-ASCII name or alias compares correctly.
        if (StringTools::equalsIgnoreCase(this->name, name)) {
            return true;
        }

        for (const std::string& alias : aliases) {
            if (StringTools::equalsIgnoreCase(alias, name)) {
                return true;
            }
        }

        return false;
    }

    std::unordered_set<std::string> ModType::getModsToFix() const {
        // See the header's warning: the pure-Python original reads two never-populated 'fixTo'
        // sets and so always answers "none". These are the targets the remap tables actually hold,
        // keyed by this mod type's own name.
        std::unordered_set<std::string> result;

        auto addFrom = [this, &result](const auto& assets) {
            if (assets == nullptr) {
                return;
            }

            const auto& map = assets->getMap();
            auto it = map.find(name);
            if (it == map.end()) {
                return;
            }

            result.insert(it->second.begin(), it->second.end());
        };

        addFrom(hashes);
        addFrom(indices);

        return result;
    }

    std::optional<int> ModType::getVertexCount(const std::optional<Version>& version) const {
        if (vertexCounts == nullptr) {
            return std::nullopt;
        }

        // Two non-version columns, name then component -- VertexCounts' own class note. The
        // pure-Python original passes only the name because its ModAssets.get pads; ModDictAssets
        // requires both, and "" is the component every shipped row carries (a real key value, not
        // a "missing" marker), so it is what a caller wanting a mod's overall count passes.
        //
        // errorOnNotFound = false: the pure-Python original leaves its own default in place and so
        // raises for a mod type with no row, which is not a useful way to say "unknown" in C++.
        return vertexCounts->get({name, ""}, version, false);
    }

    std::optional<VGRemap> ModType::getVGRemap(const std::string& modName, const std::optional<Version>& fromVersion,
                                                const std::optional<Version>& toVersion,
                                                const std::optional<std::string>& fromComp,
                                                const std::optional<std::string>& toComp) const {
        if (vgRemaps == nullptr) {
            return std::nullopt;
        }

        // VGRemaps' four non-version columns are, in order: fromChar, fromComp, toChar, toComp
        // (see its own class note). A std::nullopt leaves that column unconstrained, which is what
        // the pure-Python original expresses by simply leaving the key out of its dict.
        return vgRemaps->get({name, fromComp, modName, toComp}, {fromVersion, toVersion}, false);
    }

    std::string ModType::getHelpStr() const {
        Heading modTypeHeading(name, 8, "-");

        std::string result = modTypeHeading.open();
        result += "\n\nname: " + name;

        if (!aliases.empty()) {
            // Sorted, matching the original -- the help text is user-facing, so a stable order
            // matters more than the order the aliases happen to be declared in.
            std::set<std::string> sortedAliases(aliases.begin(), aliases.end());

            std::string aliasStr;
            for (const std::string& alias : sortedAliases) {
                if (!aliasStr.empty()) {
                    aliasStr += ", ";
                }
                aliasStr += alias;
            }

            result += "\naliases: " + aliasStr;
        }

        result += "\n\n" + modTypeHeading.close();
        return result;
    }

    void ModType::fixIni(IniFile& iniFile, bool keepBackup, bool fixOnly) const {
        const ModType* iniModType = iniFile.getAvailableType();
        if (iniModType == nullptr || iniModType->name != name) {
            return;
        }

        iniFile.fix(keepBackup, fixOnly);
    }

    Ranges<long long> ModType::getHashRanges(const IfContentPartColouring<std::string, std::string>& partColours,
                                              const std::optional<Version>& version,
                                              const std::vector<std::optional<std::string>>& nonVersionVals) const {
        std::unordered_map<std::string, bool> keysExists;
        keysExists.emplace(IniKeywords::Hash, true);

        std::unordered_map<std::string, IfContentPartColouring<std::string, std::string>::Filter> keyFilters;
        keyFilters.emplace(IniKeywords::Hash,
                           [this, &version, &nonVersionVals](std::optional<long long>, const std::string& value) {
            return hashes != nullptr && hashes->hasFrom(value, version, nonVersionVals);
        });

        return partColours.getRanges(keysExists, keyFilters);
    }
}
