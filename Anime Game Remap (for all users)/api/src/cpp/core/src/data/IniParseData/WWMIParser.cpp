#include "AGRemapCore/data/IniParseData/WWMIParser.h"

#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/data/WWMITextureRoles.h"
#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/model/textures/TexThumbprint.h"
#include "AGRemapCore/tools/files/FileService.h"

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

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"


namespace AGRemapCore {
    namespace {
        using Parser = GIMIParser<>;
        using Classifier = Parser::Classifier;
        using ModObj = Parser::ModObj;

        // The keys the fixer reads back off a slot section: its identity, and the window it draws.
        const std::string MatchIndexCountKey = "match_index_count";


        /**
         * A GIMIParser that owns its context and its classifier -- the same ownership story as
         * GIMICharGIMIParser, minus the downloads a WuWa character does not register (yet).
         *
         * The draw slots are read off the character's OWN index rows here, at parse time, and not
         * when the builder table is assembled: the mod type registry the table's factories could ask
         * is still empty then, and a parser built with no slot objects classifies every slot section
         * as nothing while the hash-only ones still work -- which is exactly how it failed first.
         */
        class WWMIParser: public Parser, public WWMIParseFacts {
            // DECLARED FIRST: it owns the Z3Context every BranchVal it hands out belongs to, and
            // member destruction order is what keeps them alive long enough. ModBranches carries
            // the same note.
            mutable ModBranches branches_;

            public:
                WWMIParser(IniFile* iniFile, std::optional<int> modTypeId, const WWMIParserConfig& config,
                               std::optional<Version> version):
                    Parser(nullptr, {}, {}, {}, nullptr, true, false, true,
                           std::unordered_set<std::string>{IniKeywords::Hash, IniKeywords::MatchFirstIndex, MatchIndexCountKey}),
                    ctx_(iniFile, modTypeId), config_(config), version_(version) {
                    this->setCtx(&ctx_);
                    this->setIniFile(iniFile);

                    // The slots: every Indices row typed component0, component1, ... the character has.
                    const std::string ownName = ctx_.modTypeName();
                    std::vector<ModObj> modObjs;
                    Classifier::IndexModObjs slotObjs;
                    if (ctx_.modTypeIndices() != nullptr && !ownName.empty()) {
                        while (true) {
                            const std::string slot = config.slotPrefix + std::to_string(slotObjs.size());
                            if (!ctx_.modTypeIndices()->get({ownName, "", slot}, version, false).has_value()) {
                                break;
                            }

                            const ModObj obj("", slot);
                            modObjs.push_back(obj);
                            slotObjs[Classifier::IndexKey("", slot)] = obj;
                        }
                    }

                    std::unordered_map<std::string, ModObj> hashOnly;
                    for (const auto& entry : config.hashOnlyObjs) {
                        const ModObj obj("", entry.second);
                        hashOnly[entry.first] = obj;
                        modObjs.push_back(obj);
                    }

                    this->setModObjs(std::move(modObjs));

                    // The version is passed explicitly: a reverse lookup with no version resolves
                    // through the NEWEST bucket holding the value, and `0` (component 0's index) is
                    // every GI head's index too, filed at 6.1 -- that bucket holds no WuWa row, so
                    // component 0 classified as nothing until the version was handed over.
                    std::unordered_map<std::string, Classifier::IndexModObjs> indexMap{{config.slotHashType, std::move(slotObjs)}};
                    classifier_ = std::make_unique<Classifier>(
                        std::move(hashOnly), ctx_.modTypeHashes(), std::move(indexMap), ctx_.modTypeIndices(), std::move(version));

                    // Both lookups filtered to the character's own rows: a hash value is unique per
                    // character, and an index like `0` is not.
                    if (!ownName.empty()) {
                        classifier_->setHashNonVersionVals({ownName, std::nullopt});
                        classifier_->setIndexNonVersionVals({ownName, std::nullopt, std::nullopt});
                    }

                    Classifier* classifier = classifier_.get();
                    this->objTargetFuncs.emplace_back(
                        [this, classifier](Parser&, const std::string& sectionName, Section* section, bool,
                                     ContentPart*, const Colouring* kvps) {
                            if (kvps == nullptr) {
                                return std::vector<ModObj>();
                            }

                            std::vector<ModObj> objs = classifier->classify(sectionName, section, *kvps);

                            // WHAT THE CLASSIFIER DECIDED, KEPT. The texture roles are read off the
                            // sections it placed, and re-deriving "which component is this" beside
                            // it would be a second answer to a question it has already answered.
                            if (!objs.empty()) {
                                classified_[sectionName] = objs;
                            }

                            return objs;
                        });
                }

                /**
                 * Built on the first ask, not in the constructor: the index walks the mod's folder
                 * and opens every candidate texture, and a parser is built for every `.ini` a run
                 * classifies -- most of which no WuWa fixer will ever ask about.
                 *
                 * `nullptr` when the character's config carries no textures to sort by, which is
                 * what makes the fixer fall back to building its own.
                 */
                const WWMITextureRoles* textureRoles() const override {
                    if (config_.textures.roles.empty() && config_.textures.registerRoles.empty()) {
                        return nullptr;         // nothing to sort by: the fixer does what it did before
                    }

                    if (!roles_) {
                        roles_ = std::make_unique<WWMITextureRoles>();
                        buildRoles(*roles_);
                    }

                    return roles_.get();
                }

            private:

                /**
                 * Every texture the .ini names, with the role it plays. Nothing here looks at the
                 * folder: a file the .ini does not name is not the mod's texture.
                 */
                // A FILE A PREVIOUS RUN OF THE FIX WROTE, WHICH PIXEL IDENTITY CANNOT TELL FROM
                // THE MOD'S OWN (2026-09-29).
                //
                // A `RemapDL` download is a byte copy of the game's texture, so it correlates
                // perfectly with the thumbprint OF that texture -- because it is one. Fixing Sanhua2
                // twice, the second pass identified nine of the first pass's downloads as the mod's
                // textures, the fix bound them instead of declaring fresh downloads, and the same mod
                // produced two different `.ini`. Neither the hash nor the register route can do this:
                // they read what the mod's own sections SAY, and a mod declares nothing about a file
                // that did not exist when it was written.
                //
                // `RemapDL` / `RemapTex` immediately before the extension is the fix's own naming --
                // the same marker `RemapIniRemover` uses to decide what an undo deletes. An undo
                // normally takes these files with it; this mod is one of the corpus's incomplete
                // undos, and a fix may not assume the undo was complete over a folder someone has
                // been editing by hand.
                static bool isOursAlready(const std::string& file) {
                    const std::string stem = StringTools::toLower(FileService::stem(file));

                    return StringTools::endsWith(stem, StringTools::toLower(IniKeywords::RemapDL))
                           || StringTools::endsWith(stem, StringTools::toLower(IniKeywords::RemapTex));
                }

                void buildRoles(WWMITextureRoles& out) const {
                    IniFile* ini = const_cast<IniFileParseContext&>(ctx_).getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    const auto& templates = ini->getIfTemplates();
                    const std::string folder = ini->getFolder();
                    const std::string iniPath = ini->getFile().value_or(std::string());

                    // resource -> the file it names
                    std::unordered_map<std::string, std::string> fileOf;
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        const std::optional<std::string> name =
                            ModBranches::firstVal(*entry.second, IniKeywords::Filename);
                        if (!name.has_value()) {
                            continue;
                        }

                        const std::string file = FileService::absPathOfRelPath(
                            FileService::iniPathToRel(*name), folder);
                        fileOf[StringTools::toLower(entry.first)] = file;
                        out.addResource(iniPath, entry.first, file);
                    }

                    const auto* hashes = const_cast<IniFileParseContext&>(ctx_).modTypeHashes();
                    const std::string ownName = const_cast<IniFileParseContext&>(ctx_).modTypeName();

                    std::unordered_set<std::string> knownRoles;
                    for (const auto& entry : config_.textures.roles) {
                        knownRoles.insert(entry.second);
                    }
                    for (const auto& comp : config_.textures.registerRoles) {
                        for (const auto& reg : comp.second) {
                            knownRoles.insert(reg.second);
                        }
                    }

                    // THE LIBRARY FIRST. A mod carries whatever hash its author exported, and
                    // HashData holds the character's history typed by role; the config's table is
                    // one generation and is the fallback until the rest are filed.
                    auto roleOfHash = [&](const std::string& hash) -> std::optional<std::string> {
                        if (hashes != nullptr && !ownName.empty()) {
                            const std::optional<std::vector<std::string>> key =
                                hashes->getKey(hash, std::nullopt, {ownName, std::nullopt}, false);
                            if (key.has_value() && key->size() > 1 && knownRoles.count((*key)[1]) > 0) {
                                return (*key)[1];
                            }
                        }

                        auto it = config_.textures.roles.find(hash);
                        return it == config_.textures.roles.end() ? std::optional<std::string>()
                                                                  : std::optional<std::string>(it->second);
                    };

                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        const std::optional<std::string> hash =
                            ModBranches::firstVal(*entry.second, IniKeywords::Hash);
                        if (!hash.has_value()) {
                            continue;
                        }

                        const std::string value = StringTools::toLower(StringTools::strip(*hash).data());

                        // (1) the section IS a texture -- its hash names the role, `this` the file
                        const std::optional<std::string> role = roleOfHash(value);
                        if (role.has_value()) {
                            for (const BranchVal& bound :
                                     branches_.valsThroughRun(templates, entry.first, IniKeywords::This)) {
                                auto file = fileOf.find(StringTools::toLower(
                                    std::string(StringTools::strip(bound.val))));
                                if (file != fileOf.end()) {
                                    out.add(file->second, *role, WWMITextureRoles::DeclaredHash + value);
                                }
                            }
                        }

                        // (2) the section is a DRAW the classifier placed -- its registers say what
                        // they bind. Read through `run =`: a section may carry only a hash and a call.
                        auto placed = classified_.find(entry.first);
                        if (placed == classified_.end()) {
                            continue;
                        }

                        for (const ModObj& obj : placed->second) {
                            if (obj.second.rfind(config_.slotPrefix, 0) != 0) {
                                continue;
                            }

                            int component = -1;
                            try {
                                component = std::stoi(obj.second.substr(config_.slotPrefix.size()));
                            } catch (const std::exception&) {
                                continue;
                            }

                            auto table = config_.textures.registerRoles.find(component);
                            if (table == config_.textures.registerRoles.end()) {
                                continue;
                            }

                            for (const auto& reg : table->second) {
                                for (const BranchVal& bound :
                                         branches_.valsThroughRun(templates, entry.first, reg.first)) {
                                    const std::string val =
                                        IniNamingTools::removeRefPrefix(std::string(bound.val));

                                    auto file = fileOf.find(StringTools::toLower(val));
                                    if (file != fileOf.end()) {
                                        out.add(file->second, reg.second,
                                                "bound at " + reg.first + " of " + obj.second);
                                    }
                                }
                            }
                        }
                    }

                    // (3) A FILE NO HASH AND NO REGISTER NAMED MAY STILL BE THE GAME'S OWN TEXTURE,
                    // AND ITS PIXELS SAY SO. A mod that ships a texture dumped straight out of the
                    // game carries neither -- the author never declared a hash for it and binds it
                    // through a slot this character's table does not list -- so ChisaParfait1's
                    // panelMask and panelNormal resolved to nothing and were DOWNLOADED over
                    // (13 of 220 placements in the corpus are this and nothing else).
                    //
                    // The candidates are the files this .ini names, which is the set every other
                    // part of the fix works from. What this replaced walked the mod folder and up to
                    // three parents, so a file the mod never binds was a candidate too: Chisa7 keeps
                    // spare colourways beside the installed one and a spare won `upperDiffuse`,
                    // which is what turned a kimono red (2026-09-29).
                    for (const auto& declared : fileOf) {
                        const std::string& file = declared.second;
                        if (!StringTools::endsWithIgnoreCase(file, FileExt::DDS)
                                || out.rolesOf().count(file) > 0 || isOursAlready(file)) {
                            continue;
                        }

                        std::optional<std::string> hash;
                        if (config_.textures.identifyTexture) {
                            hash = config_.textures.identifyTexture(file);
                        }

                        if (!hash.has_value()) {
                            hash = TexThumbprint::identifyFile(
                                file, config_.textures.textureThumbprints, config_.textures.thumbprintSize,
                                config_.textures.identityMin, config_.textures.identityGap);
                        }

                        if (!hash.has_value()) {
                            continue;
                        }

                        const std::optional<std::string> byPixels = roleOfHash(StringTools::toLower(*hash));
                        if (byPixels.has_value()) {
                            out.add(file, *byPixels, "the game's own " + *hash + " by its pixels");
                        }
                    }
                }

            public:

            private:
                IniFileParseContext ctx_;
                std::unique_ptr<Classifier> classifier_;
                WWMIParserConfig config_;
                std::optional<Version> version_;
                mutable std::unordered_map<std::string, std::vector<ModObj>> classified_;
                mutable std::unique_ptr<WWMITextureRoles> roles_;
        };
    }


    IniParseBuilder::Factory makeWWMIParser(WWMIParserConfig config) {
        const std::optional<Version> defaultVersion = Version::parse(config.version);

        return [config, defaultVersion](IniFile* iniFile, std::optional<int> modTypeId) {
            std::optional<Version> version = defaultVersion;
            if (iniFile != nullptr && iniFile->fromVersion.has_value()) {
                version = iniFile->fromVersion;
            }

            return std::make_shared<WWMIParser>(iniFile, modTypeId, config, version);
        };
    }
}
