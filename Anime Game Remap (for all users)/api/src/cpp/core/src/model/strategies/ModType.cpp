// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "AGRemapCore/model/strategies/ModType.h"

#include <set>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/constants/GameTypeId.h"
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
                      std::shared_ptr<IniRemoveBuilder> iniRemoveBuilder,
                      std::shared_ptr<IndexCounts> indexCounts, std::shared_ptr<VGOffsets> vgOffsets,
                      std::shared_ptr<VGCounts> vgCounts, std::shared_ptr<ShapeKeyChecksums> shapeKeyChecksums,
                      std::shared_ptr<DrawPasses> drawPasses, std::shared_ptr<TextureRegisters> textureRegisters,
                      std::shared_ptr<GameShaders> gameShaders, std::shared_ptr<ShapeKeyDispatchSizes> shapeKeyDispatchSizes,
                      std::shared_ptr<BoneCentroids> boneCentroids, std::shared_ptr<ShapeKeyRemaps> shapeKeyRemaps,
                      std::shared_ptr<MaterialCodes> materialCodes, std::shared_ptr<MaterialColours> materialColours,
                      std::shared_ptr<TextureDescs> textureDescs, std::shared_ptr<TextureThumbprints> textureThumbprints):
        gameTypeId(gameTypeId), modTypeId(modTypeId), name(name), aliases(aliases),
        hashes(std::move(hashes)), indices(std::move(indices)), vertexCounts(std::move(vertexCounts)),
        vgRemaps(std::move(vgRemaps)),
        indexCounts(std::move(indexCounts)), vgOffsets(std::move(vgOffsets)), vgCounts(std::move(vgCounts)), shapeKeyChecksums(std::move(shapeKeyChecksums)),
        drawPasses(std::move(drawPasses)), textureRegisters(std::move(textureRegisters)), gameShaders(std::move(gameShaders)),
        shapeKeyDispatchSizes(std::move(shapeKeyDispatchSizes)), boneCentroids(std::move(boneCentroids)), shapeKeyRemaps(std::move(shapeKeyRemaps)),
        materialCodes(std::move(materialCodes)), materialColours(std::move(materialColours)), textureDescs(std::move(textureDescs)),
        textureThumbprints(std::move(textureThumbprints)),
        iniParseBuilder(std::move(iniParseBuilder)), iniFixBuilder(std::move(iniFixBuilder)), iniRemoveBuilder(std::move(iniRemoveBuilder)) {
        // The four WWMI-only tables default as hashes does: a fresh, fully-populated table each.
        if (this->indexCounts == nullptr) {
            this->indexCounts = std::make_shared<IndexCounts>();
        }
        if (this->vgOffsets == nullptr) {
            this->vgOffsets = std::make_shared<VGOffsets>();
        }
        if (this->vgCounts == nullptr) {
            this->vgCounts = std::make_shared<VGCounts>();
        }
        if (this->shapeKeyChecksums == nullptr) {
            this->shapeKeyChecksums = std::make_shared<ShapeKeyChecksums>();
        }

        // ...and so do the six game-data tables
        if (this->drawPasses == nullptr) {
            this->drawPasses = std::make_shared<DrawPasses>();
        }
        if (this->textureRegisters == nullptr) {
            this->textureRegisters = std::make_shared<TextureRegisters>();
        }
        if (this->gameShaders == nullptr) {
            this->gameShaders = std::make_shared<GameShaders>();
        }
        if (this->shapeKeyDispatchSizes == nullptr) {
            this->shapeKeyDispatchSizes = std::make_shared<ShapeKeyDispatchSizes>();
        }
        if (this->boneCentroids == nullptr) {
            this->boneCentroids = std::make_shared<BoneCentroids>();
        }
        if (this->shapeKeyRemaps == nullptr) {
            this->shapeKeyRemaps = std::make_shared<ShapeKeyRemaps>();
        }
        if (this->materialCodes == nullptr) {
            this->materialCodes = std::make_shared<MaterialCodes>();
        }
        if (this->materialColours == nullptr) {
            this->materialColours = std::make_shared<MaterialColours>();
        }
        if (this->textureDescs == nullptr) {
            this->textureDescs = std::make_shared<TextureDescs>();
        }
        if (this->textureThumbprints == nullptr) {
            this->textureThumbprints = std::make_shared<TextureThumbprints>();
        }

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

    // The four WWMI lookups: three non-version columns (name, component, type), as Indices --
    // errorOnNotFound = false so a mod type with no row answers nullopt rather than throwing.
    std::optional<std::string> ModType::getIndexCount(const std::string& type, const std::string& component, const std::optional<Version>& version) const {
        return indexCounts == nullptr ? std::nullopt : indexCounts->get({name, component, type}, version, false);
    }

    std::optional<std::string> ModType::getVGOffset(const std::string& type, const std::string& component, const std::optional<Version>& version) const {
        return vgOffsets == nullptr ? std::nullopt : vgOffsets->get({name, component, type}, version, false);
    }

    std::optional<std::string> ModType::getVGCount(const std::string& type, const std::string& component, const std::optional<Version>& version) const {
        return vgCounts == nullptr ? std::nullopt : vgCounts->get({name, component, type}, version, false);
    }

    std::optional<std::string> ModType::getShapeKeyChecksum(const std::string& type, const std::string& component, const std::optional<Version>& version) const {
        return shapeKeyChecksums == nullptr ? std::nullopt : shapeKeyChecksums->get({name, component, type}, version, false);
    }

    std::optional<std::string> ModType::getDrawPass(const std::string& component, const std::string& type, const std::optional<Version>& version) const {
        return drawPasses == nullptr ? std::nullopt : drawPasses->get({name, component, type}, version, false);
    }

    std::optional<std::string> ModType::getTextureRegister(const std::string& component, const std::string& role, const std::optional<Version>& version) const {
        if (textureRegisters == nullptr) {
            return std::nullopt;
        }
        std::optional<std::string> own = textureRegisters->get({name, component, role}, version, false);
        return own.has_value() ? own : textureRegisters->get({GameTypeIdTools::getName(static_cast<GameTypeId>(gameTypeId)), "", role}, version, false);
    }

    std::optional<std::string> ModType::getTextureRole(const std::string& component, const std::string& reg, const std::optional<Version>& version,
                                                       const std::string& drawPass) const {
        if (textureRegisters == nullptr) {
            return std::nullopt;
        }
        // a type is '<role>' or '<role>@rabbitfx' on the slot's main pass, '<role>@<pass>' on another: the role, if the
        // type is on the pass asked for
        const auto roleOn = [&drawPass](const std::string& type) -> std::optional<std::string> {
            const std::size_t at = type.rfind('@');
            const std::string suffix = at == std::string::npos ? "" : type.substr(at + 1);
            const bool onPass = drawPass.empty() ? (suffix.empty() || suffix == "rabbitfx") : suffix == drawPass;
            if (!onPass) {
                return std::nullopt;
            }
            return at == std::string::npos ? type : type.substr(0, at);
        };

        // the mod's own row whose register this is -- still its register at 'version' (a newer row of the same role
        // may have moved it)
        for (const std::vector<std::string>& key : textureRegisters->getKeys(reg, version, {name, component, std::nullopt})) {
            const std::optional<std::string> role = roleOn(key[2]);
            if (role.has_value() && getTextureRegister(component, key[2], version) == reg) {
                return role;
            }
        }
        // ...else the game's default, for a role the mod has no row of
        const std::string game = GameTypeIdTools::getName(static_cast<GameTypeId>(gameTypeId));
        for (const std::vector<std::string>& key : textureRegisters->getKeys(reg, version, {game, std::string(""), std::nullopt})) {
            const std::optional<std::string> role = roleOn(key[2]);
            if (role.has_value() && !textureRegisters->get({name, component, key[2]}, version, false).has_value()
                && getTextureRegister(component, key[2], version) == reg) {
                return role;
            }
        }
        return std::nullopt;
    }

    std::optional<std::string> ModType::getGameShader(const std::string& type, const std::optional<Version>& version) const {
        return gameShaders == nullptr ? std::nullopt : gameShaders->get({name, "", type}, version, false);
    }

    std::optional<std::string> ModType::getShapeKeyDispatchSize(const std::optional<Version>& version) const {
        return shapeKeyDispatchSizes == nullptr ? std::nullopt : shapeKeyDispatchSizes->get({name, "", "dispatchY"}, version, false);
    }

    std::map<long long, std::array<double, 3>> ModType::getBoneCentroids(const std::optional<Version>& version) const {
        std::map<long long, std::array<double, 3>> result;
        if (boneCentroids == nullptr) {
            return result;
        }

        std::set<std::string> bones;
        boneCentroids->getRepo().forEachEntry([this, &bones](const std::vector<std::string>& key, const Version&, const std::string&) {
            if (key.size() == 3 && key[0] == name && key[1].empty()) {
                bones.insert(key[2]);
            }
        });
        for (const std::string& bone : bones) {
            const std::optional<std::string> value = boneCentroids->get({name, "", bone}, version, false);
            if (!value.has_value()) {
                continue;
            }
            std::array<double, 3> centroid{};
            std::size_t at = 0;
            std::size_t start = 0;
            for (std::size_t i = 0; i <= value->size() && at < 3; ++i) {
                if (i == value->size() || (*value)[i] == ',') {
                    centroid[at++] = std::stod(value->substr(start, i - start));
                    start = i + 1;
                }
            }
            result[std::stoll(bone)] = centroid;
        }
        return result;
    }

    std::optional<long long> ModType::getMaterialCode(const std::string& component, const std::string& type, const std::optional<Version>& version) const {
        if (materialCodes == nullptr) {
            return std::nullopt;
        }
        const std::optional<std::string> value = materialCodes->get({name, component, type}, version, false);
        if (!value.has_value()) {
            return std::nullopt;
        }
        return std::stoll(*value, nullptr, 0);         // decimal, or 0x hex
    }

    std::optional<Colour> ModType::getMaterialColour(const std::string& component, const std::string& type, const std::optional<Version>& version) const {
        if (materialColours == nullptr) {
            return std::nullopt;
        }
        const std::optional<std::string> value = materialColours->get({name, component, type}, version, false);
        if (!value.has_value()) {
            return std::nullopt;
        }
        return MaterialColours::colourOf(*value);
    }

    std::optional<std::tuple<std::string, std::uint32_t, std::uint32_t, std::uint32_t>> ModType::getTextureDesc(
        const std::string& textureHash, const std::optional<Version>& version) const {
        if (textureDescs == nullptr) {
            return std::nullopt;
        }
        const std::optional<std::string> value = textureDescs->get({name, std::string(""), textureHash}, version, false);
        if (!value.has_value()) {
            return std::nullopt;
        }
        std::vector<std::string> parts;
        std::size_t start = 0;
        for (std::size_t i = 0; i <= value->size(); ++i) {
            if (i == value->size() || (*value)[i] == ',') {
                parts.push_back(value->substr(start, i - start));
                start = i + 1;
            }
        }
        if (parts.size() != 4) {
            throw std::invalid_argument("a texture description is format,width,height,mips: '" + *value + "'");
        }
        return std::make_tuple(parts[0], static_cast<std::uint32_t>(std::stoul(parts[1])), static_cast<std::uint32_t>(std::stoul(parts[2])),
                               static_cast<std::uint32_t>(std::stoul(parts[3])));
    }

    std::unordered_map<std::string, std::vector<std::uint8_t>> ModType::getTextureThumbprints(const std::optional<Version>& version) const {
        std::unordered_map<std::string, std::vector<std::uint8_t>> out;
        if (textureThumbprints == nullptr) {
            return out;
        }
        std::set<std::string> hashes;
        textureThumbprints->getRepo().forEachEntry([this, &hashes](const std::vector<std::string>& key, const Version&, const std::string&) {
            if (key.size() >= 3 && key[0] == name) {
                hashes.insert(key[2]);
            }
        });
        for (const std::string& hash : hashes) {
            const std::optional<std::string> value = textureThumbprints->get({name, std::string(""), hash}, version, false);
            if (!value.has_value() || value->size() % 2 != 0) {
                continue;
            }
            std::vector<std::uint8_t>& bytes = out[hash];
            for (std::size_t i = 0; i < value->size(); i += 2) {
                bytes.push_back(static_cast<std::uint8_t>(std::stoul(value->substr(i, 2), nullptr, 16)));
            }
        }
        return out;
    }

    std::optional<std::vector<long long>> ModType::getShapeKeyRemap(const std::string& modName, const std::optional<Version>& fromVersion,
                                                                     const std::optional<Version>& toVersion) const {
        if (shapeKeyRemaps == nullptr) {
            return std::nullopt;
        }
        const std::optional<std::string> value = shapeKeyRemaps->get({name, std::string(""), modName, std::string("")}, {fromVersion, toVersion}, false);
        if (!value.has_value()) {
            return std::nullopt;
        }
        std::vector<long long> order;
        std::size_t start = 0;
        for (std::size_t i = 0; i <= value->size(); ++i) {
            if (i == value->size() || (*value)[i] == ',') {
                if (i > start) {
                    order.push_back(std::stoll(value->substr(start, i - start)));
                }
                start = i + 1;
            }
        }
        return order;
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
        // "Is this .ini file one of mine?" -- asked across ALL the mod types it classified as, not
        // just whichever happened to come first. An IniFile can hold several, so a single-type
        // check here would silently skip a .ini file this mod type genuinely owns.
        bool isMine = false;
        for (const auto& entry : iniFile.getModTypes()) {
            if (entry.second.name == name) {
                isMine = true;
                break;
            }
        }

        if (!isMine) {
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
