#include "AGRemapCore/data/IniParseData/WWMIParser.h"

#include "AGRemapCore/data/RabbitFX.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoles.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCreate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphVarApply.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/buffers/DrawUVCoverage.h"
#include "AGRemapCore/model/buffers/SkeletonNumberingProbe.h"
#include "AGRemapCore/model/buffers/WWMIBlend.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"
#include "AGRemapCore/data/IniParseData/TemplateParser.h"
#include "AGRemapCore/data/IniParseData/GIMITemplateParser.h"
#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/model/textures/TexThumbprint.h"
#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/tools/ListTools.h"

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

#include <algorithm>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/HashData.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"


namespace AGRemapCore {
    namespace {
        using Parser = TemplateParser;
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
        class WWMIParser: public Parser {
            public:
                /**
                 * 'onlyThisFile': the parser of ANOTHER .ini of the mod, which one of this character's parsers
                 * reads the rows of (see readOther) -- the same parse, with only the resource data read off its rows:
                 * no skeleton, no draw splits, no textures, and so none of its own linked files
                 */
                WWMIParser(IniFile* iniFile, std::optional<int> modTypeId, const WWMIParserConfig& config,
                               std::optional<Version> version, bool onlyThisFile = false):
                    Parser(iniFile, modTypeId), config_(config), version_(version), modTypeId_(modTypeId), onlyThisFile_(onlyThisFile) {
                    this->disjointModObjs = false;
                    this->keysToTrack = std::unordered_set<std::string>{IniKeywords::Hash, IniKeywords::MatchFirstIndex, MatchIndexCountKey};

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

                    // THE MOD'S TEXTURE OVERRIDES, each in the graph of the role its hash plays. HashData
                    // types every texture hash of the character by its role (`<role>`, `<role>Live`,
                    // `<role>Old`, `<role>Old2`, ...), so the classifier places one by its hash the way it
                    // places a slot section by its vb0. An override of a hash HashData does not know goes
                    // to the catch-all graph (see the target function below).
                    if (!ownName.empty()) {
                        std::unordered_set<std::string> roles;
                        for (const auto& entry : config.textures.roles) {
                            roles.insert(entry.second);
                        }

                        for (const auto& row : Data::getHashDataRows()) {
                            if (row.first.size() < 3 || row.first[1] != ownName || hashOnly.count(row.first[2]) > 0) {
                                continue;
                            }

                            const std::optional<std::string> role = roleOfHashType(row.first[2], roles);
                            if (!role.has_value()) {
                                continue;
                            }

                            const ModObj obj(IniGraphModObjKeywords::Texture, *role);
                            hashOnly[row.first[2]] = obj;
                            if (std::find(modObjs.begin(), modObjs.end(), obj) == modObjs.end()) {
                                modObjs.push_back(obj);
                            }
                        }

                        modObjs.emplace_back(IniGraphModObjKeywords::Texture, "");

                        // the sections that only watch her: her slot hash, at any version, and no index window
                        for (const auto& row : Data::getHashDataRows()) {
                            if (row.first.size() >= 3 && row.first[1] == ownName && row.first[2] == config.slotHashType) {
                                watchHashes_.insert(StringTools::toLower(row.second));
                            }
                        }
                        modObjs.emplace_back("", WWMIParseObjs::Watch);
                    }

                    this->setModObjs(std::move(modObjs));

                    // The version is passed explicitly: a reverse lookup with no version resolves
                    // through the NEWEST bucket holding the value, and `0` (component 0's index) is
                    // every GI head's index too, filed at 6.1 -- that bucket holds no WuWa row, so
                    // component 0 classified as nothing until the version was handed over.
                    std::unordered_map<std::string, Classifier::IndexModObjs> indexMap{{config.slotHashType, std::move(slotObjs)}};
                    classifier_ = &this->ownClassifier(std::make_unique<Classifier>(
                        std::move(hashOnly), ctx_.modTypeHashes(), std::move(indexMap), ctx_.modTypeIndices(), std::move(version)));

                    // Both lookups filtered to the character's own rows: a hash value is unique per
                    // character, and an index like `0` is not.
                    if (!ownName.empty()) {
                        classifier_->setHashNonVersionVals({ownName, std::nullopt});
                        classifier_->setIndexNonVersionVals({ownName, std::nullopt, std::nullopt});
                    }

                    Classifier* classifier = classifier_;
                    this->objTargetFuncs.emplace_back(
                        [this, classifier](GIMIParser<>&, const std::string& sectionName, Section* section, bool,
                                     ContentPart* part, const Colouring* kvps) {
                            if (kvps == nullptr) {
                                return std::vector<ModObj>();
                            }

                            if (watches(section, part)) {
                                return std::vector<ModObj>{ModObj("", WWMIParseObjs::Watch)};
                            }

                            std::vector<ModObj> objs = classifier->classify(sectionName, section, *kvps);
                            if (objs.empty() && isUnknownTextureOverride(sectionName, section, part)) {
                                objs.emplace_back(IniGraphModObjKeywords::Texture, "");
                            }

                            return objs;
                        });

                    buildSteps();
                }

            private:
                using GroupEdit = BaseIniGraphGroupEdit<>;
                using Collect = GraphCollect<>;
                using GraphId = GroupEdit::GraphId;

                // The names the collects file their rows under (IniParseData::collected)
                inline static const std::string SlotsCollect = IniParseRows::Objs;
                inline static const std::string TextureOverridesCollect = "WWMIParser.textureOverrides";
                inline static const std::string DeclaredCollect = IniParseRows::Declared;
                inline static const std::string TextureRolesCollect = "WWMIParser.textureRoles";
                inline static const std::string TextureTogglesCollect = "WWMIParser.textureToggles";

                using Row = IniParseData::CollectedRow;


                bool isSlotObj(const ModObj& obj) const {
                    return obj.first.empty() && StringTools::startsWith(obj.second, config_.slotPrefix);
                }

                // the slot a slot's mod object names
                std::optional<int> slotOf(const ModObj& obj) const {
                    if (!isSlotObj(obj)) {
                        return std::nullopt;
                    }

                    try {
                        return std::stoi(obj.second.substr(config_.slotPrefix.size()));
                    } catch (const std::exception&) {
                        return std::nullopt;
                    }
                }

                // Where each section sits in the file: the order the .ini is written in, for rows found in another order
                // (a row's own `position` places it within its section)
                std::size_t sectionAt(const std::string& name) const {
                    if (sectionAt_.empty()) {
                        std::size_t at = 0;
                        for (const std::string& each : const_cast<IniFileParseContext&>(ctx_).sectionNames()) {
                            sectionAt_.emplace(each, at++);
                        }
                    }

                    const auto found = sectionAt_.find(name);
                    return found == sectionAt_.end() ? sectionAt_.size() : found->second;
                }

                // A section's rows, in the order they are written
                void sortAsWritten(std::vector<const Row*>& rows) const {
                    std::stable_sort(rows.begin(), rows.end(), [this](const Row* a, const Row* b) {
                        return std::make_tuple(a->position, a->line) < std::make_tuple(b->position, b->line);
                    });
                }

                // The sections each graph is rooted at (the parser's classification), for the graphs 'keep' takes
                std::unordered_map<std::string, std::vector<ModObj>> rootsOf(const std::function<bool(const ModObj&)>& keep) const {
                    std::unordered_map<std::string, std::vector<ModObj>> result;
                    for (const auto& [obj, sections] : this->sectionTargets()) {
                        if (!keep(obj)) {
                            continue;
                        }

                        for (const std::string& name : sections) {
                            result[name].push_back(obj);
                        }
                    }
                    return result;
                }

                // A collect's rows found in a section's OWN parts, when the section is a root of the row's graph
                static std::unordered_map<std::string, std::vector<const Row*>> ownRows(
                        const std::vector<Row>& rows, const std::unordered_map<std::string, std::vector<ModObj>>& roots) {
                    std::unordered_map<std::string, std::vector<const Row*>> result;
                    for (const Row& row : rows) {
                        const auto root = roots.find(row.section);
                        if (root != roots.end()
                                && std::find(root->second.begin(), root->second.end(), row.graphId.modObj) != root->second.end()) {
                            result[row.section].push_back(&row);
                        }
                    }
                    return result;
                }

                /**
                 * The edits that read the mod, all deferred: they run on the first ask of the parse data, since
                 * sorting the textures opens every candidate, and a parser is built for every `.ini` a run
                 * classifies -- most of which no WuWa fixer will ever ask about.
                 *
                 * (1) the standard rows (GIMIParser::rowSteps): the files of the resources the mod binds,
                 * every section's declaration and every key of every section, and every key of every slot graph;
                 * (2) the hash and the `this` of every texture override; (3) the textures' roles and toggles;
                 * (4) the skeleton numbering, a variable made from the slot and resource rows (GraphVarApply); then
                 * the functions reading the rows: (5) the resource data (IniParseData::resourceData, by WWMI's
                 * rules); (6) the textures; (7) the draw splits.
                 */
                void buildSteps() {
                    auto overrides = std::make_unique<Collect>();
                    overrides->name = TextureOverridesCollect;
                    for (const ModObj& obj : this->modObjs()) {
                        overrides->keys.insert_or_assign(GraphId(0, obj.first, obj.second),
                                                         std::vector<std::string>{IniKeywords::Hash, IniKeywords::This,
                                                                                  IniKeywords::MatchPriority});
                    }


                    // THE TEXTURE ROLES THE .ini STATES, per root section and through `run =`: a texture
                    // override's `this` (the role is its graph's), and the registers a slot's sections bind
                    // that WWMITextureFacts::registerRoles names. And the toggles: each texture override's own
                    // `this` and texture registers, with the branches they sit in.
                    auto roles = std::make_unique<Collect>();
                    roles->name = TextureRolesCollect;
                    roles->withConditions = true;
                    auto toggles = std::make_unique<Collect>();
                    toggles->name = TextureTogglesCollect;
                    const std::string regPrefix = StringTools::toLower(config_.texRegPrefix);
                    const bool readsTextures = !config_.textures.roles.empty() || !config_.textures.registerRoles.empty();
                    for (const ModObj& obj : readsTextures ? this->modObjs() : std::vector<ModObj>{}) {
                        const GraphId id(0, obj.first, obj.second);
                        if (obj.first == IniGraphModObjKeywords::Texture) {
                            if (!obj.second.empty()) {
                                roles->keys.insert_or_assign(id, std::vector<std::string>{IniKeywords::This});
                            }
                            toggles->keys.insert_or_assign(id, std::vector<std::string>{IniKeywords::This});
                            toggles->keyMatches.insert_or_assign(id, [regPrefix](const std::string& key) {
                                return StringTools::startsWith(StringTools::toLower(StringTools::strip(key)), regPrefix);
                            });
                            continue;
                        }

                        const std::optional<int> slot = slotOf(obj);
                        const auto layout = slot.has_value() ? config_.textures.registerRoles.find(*slot) : config_.textures.registerRoles.end();
                        if (layout != config_.textures.registerRoles.end()) {
                            std::vector<std::string> regs;
                            for (const auto& [reg, role] : layout->second) {
                                (void)role;
                                regs.push_back(reg);
                            }
                            roles->keyMatches.insert_or_assign(id, [regs](const std::string& key) {
                                return std::any_of(regs.begin(), regs.end(),
                                                   [&key](const std::string& reg) { return IniNamingTools::sameKey(key, reg); });
                            });
                        }
                    }

                    // the standard rows -- with [Constants]' mesh vertex count among the declarations
                    RowSteps rows;
                    rows.declaredKeys = {IniKeywords::Global + " " + IniKeywords::WWMIMeshVertexCount};
                    rows.objs = [this](const ModObj& obj) { return isSlotObj(obj); };

                    // the skeleton numbering, made from the slot rows and the resource rows
                    auto skeleton = std::make_unique<GraphVarApply>(
                        std::vector<std::string>{SlotsCollect, IniParseRows::Resources},
                        [this](const std::vector<const GraphEditVariables::Value*>& inputs) { return skeletonNumbering(inputs); },
                        IniParseData::BlendVersion);

                    // THE STEPS: the edits that read the mod, then the functions that read their rows
                    this->deferredParseSteps = this->rowSteps(rows);
                    for (GroupEdit* step : {&this->ownStep(std::move(overrides)), &this->ownStep(std::move(roles)),
                                            &this->ownStep(std::move(toggles))}) {
                        this->deferredParseSteps.push_back(step);
                    }
                    if (!onlyThisFile_) {
                        this->deferredParseSteps.push_back(&this->ownStep(std::move(skeleton)));
                    }

                    this->deferredParseReaders = {[this](IniParseData& data) { applyResourceData(data); }};
                    if (!onlyThisFile_) {
                        this->deferredParseReaders.push_back([this](IniParseData& data) { readTextures(data); });
                        this->deferredParseReaders.push_back([this](IniParseData& data) { readDrawSplits(data); });
                    }
                }

                // ---- WWMI's reading of the rows: the mod's resource data, as WWMIFixer asks for it ----
                //
                // The policies here are WWMI's: a slot is undrawn when its sections draw nothing and skip the
                // game's draw (unless every slot is, a mod this reading does not understand); a texture register
                // bound after a draw is late, unless the mod replaces textures by hash (its art reaches those
                // draws anyway); a `run =` this file cannot resolve draws, unless it is WWMI's own library.
                void applyResourceData(IniParseData& data) {
                    std::unordered_set<std::string> slotRoots;
                    IniParseData::ResourceDataRules rules;
                    rules.rows = SlotsCollect;
                    rules.resources = IniParseRows::Resources;
                    rules.sections = IniParseRows::Sections;
                    rules.texRegPrefix = config_.texRegPrefix;
                    rules.noneUndrawnWhenAll = true;
                    rules.libraries = {{RabbitFX::Name, [](const std::string& text) { return RabbitFX::names(text); }}};
                    rules.runDraws = [this](const std::string& called) {
                        return ctx_.getSection(called) == nullptr && StringTools::toLower(called).find("\\wwmiv1\\") == std::string::npos;
                    };
                    rules.log = [this](const std::string& message) { ctx_.log(message); };
                    for (const auto& [obj, sections] : this->sectionTargets()) {
                        if (isSlotObj(obj)) {
                            slotRoots.insert(sections.begin(), sections.end());
                            rules.roots.emplace_back(obj, sections);
                        }
                    }

                    // THE TOGGLES: which of the mod's own resources a texture override binds in SEVERAL VARIANTS. A
                    // toggle mod writes `if $hair == 0 / this = X / else / this = XA`, and a single `<reg> = <resource>`
                    // line in a fix can carry only one of them. One `this` under `$object_detected` alone is not a
                    // toggle (every Sanhua mod wraps its one `this` that way); under any other condition it is a toggle
                    // of one variant (Lynae3's `if $Hairs == 1`). Only for the file a fix reads its textures from
                    if (!onlyThisFile_ && !(config_.textures.roles.empty() && config_.textures.registerRoles.empty())) {
                        rules.toggles = TextureTogglesCollect;
                        rules.togglesIn = [](const std::string& section) {
                            return StringTools::startsWith(section, IniKeywords::TextureOverride) && !isFixSection(section);
                        };
                        rules.notToggles = {IniKeywords::WWMIObjectDetected, IniKeywords::WWMIObjectDetected + "==1"};
                        for (const auto& [obj, sections] : this->sectionTargets()) {
                            if (obj.first == IniGraphModObjKeywords::Texture) {
                                rules.textureRoots.emplace_back(obj, sections);
                            }
                        }
                    }

                    // DOES THE MOD REPLACE TEXTURES BY HASH? A texture override (not a slot's) with a hash and a
                    // `this =`: its art then reaches every draw binding that texture, whatever the draw's own
                    // section binds.
                    std::map<std::string, std::pair<bool, bool>> hashAndThis;
                    for (const auto& row : data.collected(TextureOverridesCollect)) {
                        if (!StringTools::startsWith(row.section, IniKeywords::TextureOverride) || slotRoots.count(row.section) > 0) {
                            continue;
                        }

                        auto& has = hashAndThis[row.section];
                        has.first = has.first || row.key == IniKeywords::Hash;
                        has.second = has.second || row.key == IniKeywords::This;
                        if (has.first && has.second) {
                            data.setReplacesTexturesByHash(true);
                        }
                    }

                    data.resourceData(rules);
                }

                // THE TEXTURE ROLES (IniParseData::roleFiles), when the character's config carries textures to sort
                // by. WWMI's policy is what the rules hold: the drawn slots and the registers each binds a role at
                // (WWMITextureFacts::registerRoles), a hash's role and age from HashData, a file's game texture by
                // its thumbprint, and the linked .ini files, each read by this parser in its only-this-file mode
                void readTextures(IniParseData& data) {
                    if (config_.textures.roles.empty() && config_.textures.registerRoles.empty()) {
                        return;
                    }

                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    IniParseData::RoleFileRules rules;
                    rules.iniPath = ini->getFile().value_or(std::string());
                    rules.folder = ini->getFolder();
                    rules.sections = IniParseRows::Sections;
                    rules.declared = DeclaredCollect;
                    rules.overrides = TextureOverridesCollect;
                    rules.roleRows = TextureRolesCollect;
                    rules.objRows = SlotsCollect;
                    for (const auto& [slot, sections] : drawnSlots()) {
                        rules.objs.emplace_back(slotObj(slot), sections);
                    }
                    for (const auto& [slot, layout] : config_.textures.registerRoles) {
                        rules.registerRoles[slotObj(slot)] = layout;
                    }
                    rules.regionRoles = config_.textures.regionRoles;
                    rules.notToggles = {StringTools::toLower(IniKeywords::WWMIObjectDetected),
                                        StringTools::toLower(IniKeywords::WWMIObjectDetected) + "==1"};

                    // the role a hash of the character plays: HashData first, the config's table after it
                    const auto* hashes = ctx_.modTypeHashes();
                    const std::string ownName = ctx_.modTypeName();
                    std::unordered_set<std::string> knownRoles;
                    for (const auto& entry : config_.textures.roles) {
                        knownRoles.insert(entry.second);
                    }
                    for (const auto& comp : config_.textures.registerRoles) {
                        for (const auto& reg : comp.second) {
                            knownRoles.insert(reg.second);
                        }
                    }
                    rules.roleOfHash = [this, hashes, ownName, knownRoles](const std::string& hash) -> std::optional<std::string> {
                        if (hashes != nullptr && !ownName.empty()) {
                            const std::optional<std::vector<std::string>> key = hashes->getKey(hash, std::nullopt, {ownName, std::nullopt}, false);
                            if (key.has_value() && key->size() > 1 && knownRoles.count((*key)[1]) > 0) {
                                return (*key)[1];
                            }
                        }
                        const auto it = config_.textures.roles.find(hash);
                        return it == config_.textures.roles.end() ? std::optional<std::string>() : std::optional<std::string>(it->second);
                    };

                    // A FILE NO HASH AND NO REGISTER NAMED MAY STILL BE THE GAME'S OWN TEXTURE, AND ITS PIXELS SAY SO:
                    // a mod that ships a texture dumped straight out of the game declares neither (ChisaParfait1's
                    // panelMask and panelNormal were downloaded over before this)
                    rules.identify = [this](const std::string& file) -> std::optional<std::string> {
                        std::optional<std::string> hash;
                        if (config_.textures.identifyTexture) {
                            hash = config_.textures.identifyTexture(file);
                        }
                        if (!hash.has_value()) {
                            hash = TexThumbprint::identifyFile(file, config_.textures.textureThumbprints, config_.textures.thumbprintSize,
                                                               config_.textures.identityMin, config_.textures.identityGap);
                        }
                        return hash;
                    };

                    // HOW OLD A HASH IS: the versions HashData files it at, newest first, and within one version the
                    // current hash before its `Live` and `Old`, `Old2`, ... generations
                    rules.hashAge = [ownName](const std::string& hash) {
                        std::vector<Version> versions;
                        for (const auto& row : Data::getHashDataRows()) {
                            if (row.first.size() >= 3 && row.first[1] == ownName) {
                                if (const std::optional<Version> version = Version::parse(row.first[0])) {
                                    if (std::find(versions.begin(), versions.end(), *version) == versions.end()) {
                                        versions.push_back(*version);
                                    }
                                }
                            }
                        }
                        int age = 1 << 20;
                        for (const auto& row : Data::getHashDataRows()) {
                            if (row.first.size() < 3 || row.first[1] != ownName || !StringTools::equalsIgnoreCase(row.second, hash)) {
                                continue;
                            }
                            const std::optional<Version> version = Version::parse(row.first[0]);
                            const int newer = version.has_value()
                                ? static_cast<int>(std::count_if(versions.begin(), versions.end(), [&version](const Version& other) { return *version < other; }))
                                : static_cast<int>(versions.size());
                            const std::string& type = row.first[2];
                            int generation = 0;
                            for (const auto& [suffix, base] : {std::make_pair(std::string("Live"), 1), std::make_pair(std::string("Old"), 2)}) {
                                const std::size_t at = type.rfind(suffix);
                                if (at != std::string::npos && at > 0) {
                                    const std::string rest = type.substr(at + suffix.size());
                                    if (std::all_of(rest.begin(), rest.end(), [](char c) { return c >= '0' && c <= '9'; })) {
                                        generation = base + (rest.empty() ? 0 : std::stoi(rest) - 1);
                                    }
                                }
                            }
                            age = std::min(age, newer * 100 + generation);
                        }
                        return age;
                    };

                    // a section a previous fix wrote is not the mod's use of a resource
                    std::vector<std::string> fixNames{ownName};
                    if (const std::optional<ModType> own = modTypeId_.has_value() ? ModTypeIdTools::getModType(*modTypeId_) : std::nullopt) {
                        for (const std::string& name : own->getModsToFix()) {
                            fixNames.push_back(name);
                        }
                    }
                    rules.fixSection = [fixNames](const std::string& section) { return IniNamingTools::looksRemapped(section, fixNames); };
                    rules.log = [this](const std::string& message) { ctx_.log(message); };

                    // the files LINKED to this one (IniFile::linkedInis: a namespace one references, a resource file both
                    // name), wherever they are, each parsed by this character's parser in its only-this-file mode
                    std::vector<std::unique_ptr<IniFile>> others;
                    std::vector<std::unique_ptr<WWMIParser>> readers;
                    for (IniFile* linked : rules.objs.empty() || rules.iniPath.empty() ? std::vector<IniFile*>{} : ini->linkedInis()) {
                        others.push_back(otherIni(*linked));
                        readers.push_back(readOther(*others.back()));
                        rules.linked.push_back({&readers.back()->parseData(), others.back()->getFolder(),
                                                FileService::pathToStr(FileService::strToPath(linked->getFile().value_or("")).filename())});
                    }

                    data.roleFiles(rules);
                }

                // ---- WWMIParserConfig::skeletonNumberings ----
                // Which numbering the blend's bone ids are in, asked of the mod's own geometry
                // (SkeletonNumberingProbe), from the slot rows and the resource rows -- the version, or nothing.
                // Not asked of a mod with no vg_offset on any slot section: one from before WWMI's merged skeleton,
                // whose ids are each component's own.
                std::optional<GraphEditVariables::Value> skeletonNumbering(const std::vector<const GraphEditVariables::Value*>& inputs) {
                    static const std::vector<Row> none;
                    const auto rowsOf = [](const GraphEditVariables::Value* value) -> const std::vector<Row>& {
                        const std::vector<Row>* rows = value == nullptr ? nullptr : value->as<std::vector<Row>>();
                        return rows == nullptr ? none : *rows;
                    };

                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr || config_.skeletonNumberings.size() < 2 || config_.referenceBoneCentroids.empty()) {
                        return std::nullopt;
                    }

                    bool merged = false;
                    for (const auto& entry : ownRows(rowsOf(inputs[0]), rootsOf([this](const ModObj& obj) { return isSlotObj(obj); }))) {
                        for (const Row* row : entry.second) {
                            merged = merged || row->key == IniKeywords::WWMIVgOffset;
                        }
                    }

                    const std::map<std::string, std::string> files = IniParseData::resourceFilesOf(rowsOf(inputs[1]));
                    const auto fileOf = [&files](const std::string& section) -> std::optional<std::string> {
                        const auto found = files.find(StringTools::toLower(section));
                        return found == files.end() ? std::nullopt : std::optional<std::string>(found->second);
                    };
                    const std::optional<std::string> position = fileOf(IniKeywords::WWMIPositionBuffer);
                    const std::optional<std::string> blend = fileOf(IniKeywords::WWMIBlendBuffer);
                    if (!merged || !position.has_value() || !blend.has_value()) {
                        return std::nullopt;
                    }

                    // the remapped skeleton's own 16-bit ids: the first resource, by name, naming them
                    std::optional<std::string> vertexVG;
                    for (const auto& [section, file] : files) {
                        std::string name = section;
                        name.erase(std::remove(name.begin(), name.end(), '_'), name.end());
                        if (name.find("blendremapvertexvg") != std::string::npos) {
                            vertexVG = file;
                            break;
                        }
                    }

                    const std::string folder = ini->getFolder();
                    const auto pathOf = [&folder](const std::string& rel) {
                        return FileService::absPathOfRelPath(FileService::iniPathToRel(rel), folder);
                    };
                    const std::optional<SkeletonNumberingProbe::Result> found = SkeletonNumberingProbe::probe(
                        pathOf(*position), pathOf(*blend), vertexVG.has_value() ? std::optional<std::string>(pathOf(*vertexVG)) : std::nullopt,
                        config_.skeletonNumberings, config_.referenceBoneCentroids);
                    if (!found.has_value()) {
                        return std::nullopt;
                    }

                    std::ostringstream how;
                    for (std::size_t n = 0; n < found->medians.size(); ++n) {
                        how << (n == 0 ? "" : ", ") << config_.skeletonNumberings[n].version << " "
                            << static_cast<long long>(std::lround(found->medians[n] * 100)) / 100.0;
                    }

                    const std::string& version = config_.skeletonNumberings[found->best].version;
                    ctx_.log("its bone ids are " + version + "'s numbering (" + std::to_string(found->vertices)
                             + " vertices on renumbered bones; median distance to the bone by version: " + how.str() + ")");
                    return GraphEditVariables::Value::of<std::string>(version);
                }

                // ---- WWMIParserConfig::drawSplits ----
                // Which draws of a slot carry a split's flags, read off the code map the mod draws the slot with
                // at the draws' vertices (DrawUVCoverage), filed as the draw ranges of `(split name, slot)`
                void readDrawSplits(IniParseData& data) {
                    IniFile* ini = ctx_.getIniFile();
                    const std::optional<std::string> index = data.resourceFile(IniKeywords::WWMIIndexBuffer);
                    const std::optional<std::string> texcoord = data.resourceFile(IniKeywords::WWMITexcoordBuffer);
                    const std::optional<std::string> position = data.resourceFile(IniKeywords::WWMIPositionBuffer);
                    if (config_.drawSplits.empty() || ini == nullptr || !index.has_value() || !texcoord.has_value() || !position.has_value()) {
                        return;
                    }

                    const std::string folder = ini->getFolder();
                    const std::string iniPath = ini->getFile().value_or(std::string());
                    const std::map<int, std::vector<std::string>> drawn = drawnSlots();
                    for (const WWMIParserConfig::DrawSplit& split : config_.drawSplits) {
                        const ModObj obj = slotObj(split.component);
                        const std::vector<IniParseData::DrawRange>& ranges = data.drawRanges(obj);
                        const auto sections = drawn.find(split.component);
                        if (ranges.empty() || data.isUndrawn(obj) || sections == drawn.end() || sections->second.empty()) {
                            continue;
                        }

                        // the MOD's own map, when a resource of this .ini names the file chosen for the role (one a
                        // previous fix declared is no evidence about the mod) -- else the character's own game map,
                        // which DrawSplit::sourceFlags stands in for
                        std::optional<std::string> codesPath;
                        const IniParseData::RoleFile* chosen = data.roleFile(split.codeRole, obj);
                        const TextureRoles* roles = data.textureRoles();
                        if (chosen != nullptr && roles != nullptr) {
                            for (const auto& [resource, file] : roles->resourcesOf(iniPath)) {
                                if (file == chosen->file) {
                                    if (!IniNamingTools::looksRemapped(resource, {})) {
                                        codesPath = FileService::absPathOfRelPath(file, folder);
                                    }
                                    break;
                                }
                            }
                        }

                        const WWMIParserConfig::DrawSplit::FlagMask& mask = split.sourceFlags;
                        const bool useMask = !codesPath.has_value();
                        if (useMask && (mask.width <= 0 || mask.height <= 0
                                        || mask.bits.size() * 8 < static_cast<std::size_t>(mask.width) * static_cast<std::size_t>(mask.height))) {
                            continue;
                        }

                        std::vector<IniParseData::DrawRange> taken;
                        try {
                            std::vector<std::uint8_t> codes;
                            long long w = mask.width;
                            long long h = mask.height;
                            if (!useMask) {
                                TextureFile map{*codesPath};
                                map.open();
                                if (!map.hasImage()) {
                                    continue;
                                }
                                codes = map.shaderChannel(0);
                                w = map.getWidth();
                                h = map.getHeight();
                            }

                            // the mesh's vertices are its Position.buf's lines
                            const std::size_t vertices = static_cast<std::size_t>(std::filesystem::file_size(
                                FileService::strToPath(FileService::absPathOfRelPath(*position, folder))) / WWMIBlend::PositionStride);
                            const DrawUVCoverage coverage = DrawUVCoverage::ofMesh(FileService::absPathOfRelPath(*index, folder),
                                                                                   FileService::absPathOfRelPath(*texcoord, folder), vertices);

                            const DrawUVCoverage::TexelTest marked = [&](long long x, long long y) {
                                const auto at = static_cast<std::size_t>(y * w + x);
                                if (useMask) {
                                    return ((mask.bits[at / 8] >> (at % 8)) & 1) != 0;
                                }
                                return at < codes.size() && (codes[at] & split.flags) != 0;
                            };

                            taken = coverage.coveredRanges(ranges, w, h, marked, split.share);
                        } catch (const std::exception& e) {
                            ctx_.log("could not read which draws of " + obj.second + " carry the " + split.codeRole + " flags: " + e.what());
                            continue;
                        }

                        for (const IniParseData::DrawRange& range : taken) {
                            data.addDrawRange(ModObj(split.name, obj.second), range);
                        }
                    }
                }

                // A section a previous fix wrote: its name ends `<target>RemapFix`
                static bool isFixSection(const std::string& name) {
                    return StringTools::endsWithIgnoreCase(name, IniKeywords::RemapFix);
                }

                // The role a HashData hash type of the character names: the role itself, or the role
                // followed by `Live` or `Old`, `Old2`, ... (see HashData's WuWa texture rows)
                static std::optional<std::string> roleOfHashType(const std::string& type, const std::unordered_set<std::string>& roles) {
                    if (roles.count(type) > 0) {
                        return type;
                    }

                    for (const std::string suffix : {std::string("Live"), std::string("Old")}) {
                        const std::size_t at = type.rfind(suffix);
                        if (at == std::string::npos || at == 0) {
                            continue;
                        }

                        const std::string rest = type.substr(at + suffix.size());
                        const bool counted = std::all_of(rest.begin(), rest.end(), [](char c) { return c >= '0' && c <= '9'; });
                        const std::string role = type.substr(0, at);
                        if (counted && (suffix == "Old" || rest.empty()) && roles.count(role) > 0) {
                            return role;
                        }
                    }

                    return std::nullopt;
                }

                // The part of a section that only watches her (WWMIParseObjs::Watch) naming her hash: it has
                // one of her slot hashes and, anywhere in the section, no `match_first_index`
                bool watches(Section* section, ContentPart* part) const {
                    if (section == nullptr || part == nullptr || watchHashes_.empty()) {
                        return false;
                    }

                    const std::vector<std::string> hashes = part->getVals(IniKeywords::Hash);
                    if (hashes.empty() || watchHashes_.count(StringTools::toLower(std::string(StringTools::strip(hashes.front())))) == 0) {
                        return false;
                    }

                    for (const auto& other : section->parts()) {
                        const auto* content = dynamic_cast<const ContentPart*>(other.get());
                        if (content != nullptr && !content->getVals(IniKeywords::MatchFirstIndex).empty()) {
                            return false;
                        }
                    }

                    return true;
                }

                // The part of a texture override that names its hash, when the hash is not one the
                // classifier knows: a `hash` and, anywhere in the section, no `match_first_index`
                static bool isUnknownTextureOverride(const std::string& name, Section* section, ContentPart* part) {
                    if (section == nullptr || part == nullptr || isFixSection(name)
                            || !StringTools::startsWith(name, IniKeywords::TextureOverride) || part->getVals(IniKeywords::Hash).empty()) {
                        return false;
                    }

                    for (const auto& other : section->parts()) {
                        const auto* content = dynamic_cast<const ContentPart*>(other.get());
                        if (content != nullptr && !content->getVals(IniKeywords::MatchFirstIndex).empty()) {
                            return false;
                        }
                    }

                    return true;
                }

                // slot -> the sections that draw it, in the order they are written: the slot graphs' roots
                std::map<int, std::vector<std::string>> drawnSlots() const {
                    std::map<int, std::vector<std::string>> result;
                    for (const auto& [obj, sections] : this->sectionTargets()) {
                        if (const std::optional<int> slot = slotOf(obj)) {
                            std::vector<std::string>& into = result[*slot];
                            into.insert(into.end(), sections.begin(), sections.end());
                        }
                    }

                    for (auto& [slot, sections] : result) {
                        (void)slot;
                        std::stable_sort(sections.begin(), sections.end(),
                                         [this](const std::string& a, const std::string& b) { return sectionAt(a) < sectionAt(b); });
                        sections.erase(std::unique(sections.begin(), sections.end()), sections.end());
                    }
                    return result;
                }

                ModObj slotObj(int slot) const {
                    return ModObj("", config_.slotPrefix + std::to_string(slot));
                }

                // ---- another .ini of the mod, read through a parser of its own ----

                // Another .ini of the mod, as this character's: the same mod types, forced, and version
                std::unique_ptr<IniFile> otherIni(const IniFile& linked) const {
                    const IniFile* ini = const_cast<IniFileParseContext&>(ctx_).getIniFile();
                    return ini->sibling(linked, modTypeId_.has_value()
                        ? std::optional<std::unordered_set<int>>(std::unordered_set<int>{*modTypeId_}) : std::nullopt);
                }

                // Parses 'other' with this character's parser in its only-this-file mode; its rows are read
                // through IniParseData::roleFiles (RoleFileRules::linked), while 'other' lives
                std::unique_ptr<WWMIParser> readOther(IniFile& other) const {
                    auto reader = std::make_unique<WWMIParser>(&other, modTypeId_, config_, version_, true);
                    reader->parse();
                    return reader;
                }

            private:
                Classifier* classifier_ = nullptr;     // owned by the base (ownClassifier)
                std::unordered_set<std::string> watchHashes_;     // her slot hashes at every version, lowercased
                WWMIParserConfig config_;
                std::optional<Version> version_;
                std::optional<int> modTypeId_;
                bool onlyThisFile_ = false;
                mutable std::unordered_map<std::string, std::size_t> sectionAt_;
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
