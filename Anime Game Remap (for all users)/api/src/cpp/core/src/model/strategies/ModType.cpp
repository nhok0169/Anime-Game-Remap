#include "AGRemapCore/model/strategies/ModType.h"

#include <utility>

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
        // here also shares its flyweight cache -- deliberate, and what the original does too.
        if (this->iniRemoveBuilder == nullptr) {
            this->iniRemoveBuilder = GlobalIniRemoveBuilders::removeBuilder();
        }
    }
}
