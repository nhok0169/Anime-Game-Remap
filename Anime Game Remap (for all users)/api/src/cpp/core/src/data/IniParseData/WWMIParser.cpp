#include "AGRemapCore/data/IniParseData/WWMIParser.h"

#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/data/RabbitFX.h"
#include "AGRemapCore/data/IniParseData/TextureRoleChooser.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoles.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCreate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufFloat.h"
#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/buffers/DrawUVCoverage.h"
#include "AGRemapCore/model/buffers/SkeletonNumberingProbe.h"
#include "AGRemapCore/model/buffers/WWMIBlend.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"
#include "AGRemapCore/data/IniParseData/TemplateParser.h"
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
                 * reads the rows of (see readOther) -- it classifies and collects as usual, and reads nothing
                 * further: no resources, no anatomy, no skeleton, no textures, and none of its own siblings
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

                // Where each section sits in the file, and each part in its section (every part counted, `if`
                // lines too): the order the .ini is written in, for rows found in another order
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

                std::size_t partAt(const std::string& section, std::size_t partId) const {
                    const auto key = std::make_pair(section, partId);
                    const auto found = partAt_.find(key);
                    if (found != partAt_.end()) {
                        return found->second;
                    }

                    if (const Section* tpl = const_cast<IniFileParseContext&>(ctx_).getSection(section)) {
                        std::size_t index = 0;
                        for (const auto& part : tpl->parts()) {
                            partAt_.emplace(std::make_pair(section, part->id()), ++index);
                        }
                    }

                    const auto filed = partAt_.find(key);
                    return filed == partAt_.end() ? 0 : filed->second;
                }

                // A section's rows, in the order they are written
                void sortAsWritten(std::vector<const Row*>& rows) const {
                    std::stable_sort(rows.begin(), rows.end(), [this](const Row* a, const Row* b) {
                        return std::make_tuple(partAt(a->section, a->partId), a->line)
                               < std::make_tuple(partAt(b->section, b->partId), b->line);
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
                 * (1) the standard rows (TemplateParser::addRowSteps): the files of the resources the mod binds,
                 * every section's declaration and every key of every section, and every key of every slot graph;
                 * (2) the hash and the `this` of every texture override; (3) the textures' roles and toggles;
                 * (4) the anatomy (IniParseData::fileAnatomy, by WWMI's rules); (5) the skeleton numbering;
                 * (6) the textures; (7) the draw splits.
                 */
                void buildSteps() {
                    std::vector<std::unique_ptr<GroupEdit>> steps;

                    auto overrides = std::make_unique<Collect>();
                    overrides->name = TextureOverridesCollect;
                    for (const ModObj& obj : this->modObjs()) {
                        overrides->keys.insert_or_assign(GraphId(0, obj.first, obj.second),
                                                         std::vector<std::string>{IniKeywords::Hash, IniKeywords::This});
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
                            if (onlyThisFile_) {
                                continue;
                            }
                            toggles->keys.insert_or_assign(id, std::vector<std::string>{IniKeywords::This});
                            toggles->keyMatches.insert_or_assign(id, [regPrefix](const std::string& key) {
                                return StringTools::startsWith(StringTools::toLower(StringTools::strip(key)), regPrefix);
                            });
                            continue;
                        }

                        const std::optional<int> slot = slotOf(obj);
                        const auto layout = slot.has_value() ? config_.textures.registerRoles.find(*slot) : config_.textures.registerRoles.end();
                        if (layout != config_.textures.registerRoles.end() && onlyThisFile_) {
                            std::unordered_set<std::string> loose;
                            for (const auto& [reg, role] : layout->second) {
                                (void)role;
                                loose.insert(ModBranches::looseKey(reg));
                            }
                            roles->keyMatches.insert_or_assign(id, [loose](const std::string& key) {
                                return loose.count(ModBranches::looseKey(key)) > 0;
                            });
                        } else if (layout != config_.textures.registerRoles.end()) {
                            std::vector<std::string> regs;
                            for (const auto& [reg, role] : layout->second) {
                                (void)role;
                                regs.push_back(reg);
                            }
                            roles->keys.insert_or_assign(id, std::move(regs));
                        }
                    }

                    steps.push_back(std::move(overrides));
                    steps.push_back(std::move(roles));
                    steps.push_back(std::move(toggles));
                    if (!onlyThisFile_) {
                        this->deferredParseReaders = {
                            [this](IniParseData& data) { applyAnatomy(data); },
                            [this](IniParseData& data) { readSkeletonNumbering(data); },
                            [this](IniParseData& data) { readTextures(data); },
                            [this](IniParseData& data) { readDrawSplits(data); }};
                    }

                    // the standard rows first -- with [Constants]' mesh vertex count among the declarations -- then
                    // the steps in the order built
                    RowSteps rows;
                    rows.resources = !onlyThisFile_;
                    rows.sections = !onlyThisFile_;
                    rows.declaredKeys = {IniKeywords::Global + " " + IniKeywords::WWMIMeshVertexCount};
                    rows.objs = [this](const ModObj& obj) { return isSlotObj(obj); };
                    this->addRowSteps(rows);
                    for (auto& step : steps) {
                        this->addDeferredParseStep(std::move(step));
                    }
                }

                // ---- WWMI's reading of the rows: the mod's anatomy, as WWMIFixer asks for it ----
                //
                // The policies here are WWMI's: a slot is undrawn when its sections draw nothing and skip the
                // game's draw (unless every slot is, a mod this reading does not understand); a texture register
                // bound after a draw is late, unless the mod replaces textures by hash (its art reaches those
                // draws anyway); a `run =` this file cannot resolve draws, unless it is WWMI's own library.
                void applyAnatomy(IniParseData& data) {
                    std::unordered_set<std::string> slotRoots;
                    IniParseData::AnatomyRules rules;
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

                    data.fileAnatomy(rules);
                }

                // The texture roles, when the character's config carries textures to sort by
                void readTextures(IniParseData& data) {
                    if (config_.textures.roles.empty() && config_.textures.registerRoles.empty()) {
                        return;
                    }

                    TextureRoles roles;
                    buildRoles(roles, data);
                    data.setTextureRoles(std::move(roles));
                    readThisToggles(data);
                    readRegisterToggles(data);
                    chooseRoleFiles(data, *data.textureRoles());
                }

                // the name the texcoord buffer's one element is read under
                inline static const std::string UVElement = "Texcoord";

                // ---- WWMIParserConfig::skeletonNumberings ----
                // Which numbering the blend's bone ids are in, asked of the mod's own geometry
                // (SkeletonNumberingProbe). Not asked of a mod with no vg_offset on any slot section: one from
                // before WWMI's merged skeleton, whose ids are each component's own.
                void readSkeletonNumbering(IniParseData& data) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr || config_.skeletonNumberings.size() < 2 || config_.referenceBoneCentroids.empty()) {
                        return;
                    }

                    bool merged = false;
                    for (const auto& entry : ownRows(data.collected(SlotsCollect), rootsOf([this](const ModObj& obj) { return isSlotObj(obj); }))) {
                        for (const Row* row : entry.second) {
                            merged = merged || row->key == IniKeywords::WWMIVgOffset;
                        }
                    }

                    const std::optional<std::string> position = data.resourceFile(IniKeywords::WWMIPositionBuffer);
                    const std::optional<std::string> blend = data.resourceFile(IniKeywords::WWMIBlendBuffer);
                    if (!merged || !position.has_value() || !blend.has_value()) {
                        return;
                    }

                    // the remapped skeleton's own 16-bit ids: the first resource, by name, naming them
                    std::optional<std::string> vertexVG;
                    for (const auto& [section, file] : data.resourceFiles()) {
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
                        return;
                    }

                    std::ostringstream how;
                    for (std::size_t n = 0; n < found->medians.size(); ++n) {
                        how << (n == 0 ? "" : ", ") << config_.skeletonNumberings[n].version << " "
                            << static_cast<long long>(std::lround(found->medians[n] * 100)) / 100.0;
                    }

                    const std::string& version = config_.skeletonNumberings[found->best].version;
                    data.setBlendVersion(version);
                    ctx_.log("its bone ids are " + version + "'s numbering (" + std::to_string(found->vertices)
                             + " vertices on renumbered bones; median distance to the bone by version: " + how.str() + ")");
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

                            const std::string uvPath = FileService::absPathOfRelPath(*texcoord, folder);
                            const std::size_t uvBytes = static_cast<std::size_t>(FileService::fileSize(uvPath).value_or(0));
                            const std::size_t vertices = static_cast<std::size_t>(std::filesystem::file_size(
                                FileService::strToPath(FileService::absPathOfRelPath(*position, folder))) / WWMIBlend::PositionStride);
                            if (vertices == 0 || uvBytes % vertices != 0 || uvBytes / vertices < 4) {
                                continue;
                            }

                            // a texcoord line is the U and V of the first channel, then any more, each a half float
                            std::vector<std::unique_ptr<BufElementType>> elements;
                            elements.push_back(BufElementType::repeated(UVElement, uvBytes / vertices / 2,
                                                                        [] { return std::make_unique<BufFloat16>(); }));
                            BufFile uvFile{uvPath, std::move(elements)};
                            const DrawUVCoverage coverage(IbFile{FileService::absPathOfRelPath(*index, folder)}.getIndices(),
                                                          uvFile.column(UVElement, 0), uvFile.column(UVElement, 1), vertices);

                            const DrawUVCoverage::TexelTest marked = [&](long long x, long long y) {
                                const auto at = static_cast<std::size_t>(y * w + x);
                                if (useMask) {
                                    return ((mask.bits[at / 8] >> (at % 8)) & 1) != 0;
                                }
                                return at < codes.size() && (codes[at] & split.flags) != 0;
                            };

                            for (const IniParseData::DrawRange& range : ranges) {
                                const DrawUVCoverage::Count counted = coverage.count(range.second, range.first, w, h, marked);
                                if (counted.vertices > 0
                                        && static_cast<double>(counted.matched) > split.share * static_cast<double>(counted.vertices)) {
                                    taken.push_back(range);
                                }
                            }
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

                // WHICH of the mod's own resources it binds in SEVERAL VARIANTS. A toggle mod writes
                // `if $hair == 0 / this = X / else / this = XA` in a texture override, and a single
                // `<reg> = <resource>` line in a fix can carry only one of them. A section binding ONE
                // resource under `$object_detected` alone is not a toggle (every Sanhua mod wraps its one
                // `this` that way); one `this` under any other condition is a toggle of one variant
                // (Lynae3's `if $Hairs == 1`), whose original draws with the game's texture while off.
                void readThisToggles(IniParseData& data) {
                    // the texture graph the parser placed each section in, and each section's own `this` rows
                    const auto roots = rootsOf([](const ModObj& obj) { return obj.first == IniGraphModObjKeywords::Texture; });
                    auto own = ownRows(data.collected(TextureTogglesCollect), roots);

                    std::unordered_map<std::string, std::string> ownerOf;            // resource -> the section binding it
                    std::unordered_map<std::string, std::vector<std::string>> variantsOf;
                    std::unordered_set<std::string> single;
                    std::vector<std::string> sections;
                    for (const auto& entry : own) {
                        sections.push_back(entry.first);
                    }
                    std::sort(sections.begin(), sections.end(),
                              [this](const std::string& a, const std::string& b) { return sectionAt(a) < sectionAt(b); });

                    for (const std::string& name : sections) {
                        // ANY texture override binding by hash, whatever its name: Lynae3 calls its hair
                        // overrides [TextureOverrideX] / [TextureOverrideXX]
                        if (!StringTools::startsWith(name, IniKeywords::TextureOverride) || isFixSection(name)) {
                            continue;
                        }

                        std::vector<const Row*>& rows = own[name];
                        sortAsWritten(rows);
                        std::vector<std::string> bound;
                        bool realToggle = false;
                        for (const Row* row : rows) {
                            for (const std::string& guard : row->guards) {
                                std::string test = StringTools::toLower(guard);
                                test.erase(std::remove_if(test.begin(), test.end(),
                                                          [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }),
                                           test.end());
                                // an `else` is a toggle's other side (Chisa18 binds its normal map under `else` of `if $Gloss == 1`)
                                if (test != IniKeywords::WWMIObjectDetected && test != IniKeywords::WWMIObjectDetected + "==1") {
                                    realToggle = true;
                                }
                            }

                            if (row->key == IniKeywords::This) {
                                bound.push_back(std::string(StringTools::strip(row->value)));
                            }
                        }

                        if (bound.size() == 1 && realToggle) {
                            const std::string key = StringTools::toLower(bound.front());
                            if (ownerOf.count(key) == 0) {
                                ownerOf[key] = name;
                                variantsOf[name] = bound;
                                single.insert(key);
                            }
                            continue;
                        }

                        if (bound.size() < 2) {
                            continue;
                        }

                        // TWO SECTIONS MAY BIND ONE RESOURCE BEHIND A TOGGLE, AND ONLY ONE OF THEM MAY BE
                        // WHOLE: SanhuaExorcist4 selects `7 / 7a / 7b` in one and `7 / 7.1 / 7.2` -- names
                        // it never declares -- in another. The section that answers for most of its own
                        // variants keeps the resource.
                        const auto resolves = [this](const std::vector<std::string>& vals) {
                            std::size_t n = 0;
                            for (const std::string& val : vals) {
                                if (const_cast<IniFileParseContext&>(ctx_).getSection(val) != nullptr) {
                                    ++n;
                                }
                            }
                            return n;
                        };

                        const std::size_t score = resolves(bound);
                        for (const std::string& resource : bound) {
                            const std::string key = StringTools::toLower(resource);
                            const auto owner = ownerOf.find(key);
                            if (owner != ownerOf.end()) {
                                const auto held = variantsOf.find(owner->second);
                                if (held != variantsOf.end() && resolves(held->second) >= score) {
                                    continue;
                                }
                            }
                            ownerOf[key] = name;
                        }
                        variantsOf[name] = bound;
                    }

                    for (const auto& [resource, section] : ownerOf) {
                        const auto graph = roots.find(section);
                        data.setThisToggle(resource, IniParseData::TextureToggle{section, variantsOf[section], single.count(resource) > 0,
                                                                                 graph == roots.end() ? ModObj() : graph->second.front()});
                    }
                }

                // THE SAME TOGGLE WRITTEN AS A REGISTER BINDING: a section binding one register to
                // several resources on its branches (`if $key2 == 1 / ps-t3 = A / else / ps-t3 = B`).
                // Every one reaches that slot, so each is a variant of the role resolved there. Per
                // (section, register): two registers of one section are two roles.
                void readRegisterToggles(IniParseData& data) {
                    // each texture override's own register rows, a slot's and a texture's
                    const auto roots = rootsOf([this](const ModObj& obj) {
                        return obj.first == IniGraphModObjKeywords::Texture || isSlotObj(obj);
                    });
                    auto own = ownRows(data.collected(TextureTogglesCollect), roots);
                    for (auto& [name, rows] : ownRows(data.collected(SlotsCollect), roots)) {
                        auto& into = own[name];
                        into.insert(into.end(), rows.begin(), rows.end());
                    }

                    std::vector<std::string> sections;
                    for (const auto& entry : own) {
                        sections.push_back(entry.first);
                    }
                    std::sort(sections.begin(), sections.end(),
                              [this](const std::string& a, const std::string& b) { return sectionAt(a) < sectionAt(b); });

                    const std::string prefix = StringTools::toLower(config_.texRegPrefix);
                    for (const std::string& name : sections) {
                        if (!StringTools::startsWith(name, IniKeywords::TextureOverride) || isFixSection(name)) {
                            continue;
                        }

                        std::vector<const Row*>& rows = own[name];
                        sortAsWritten(rows);
                        std::map<std::string, std::vector<std::string>> boundOfReg;
                        for (const Row* row : rows) {
                            const std::string key = StringTools::toLower(StringTools::strip(row->key));
                            if (!StringTools::startsWith(key, prefix)) {
                                continue;
                            }

                            const std::string val = IniNamingTools::removeRefPrefix(std::string(StringTools::strip(row->value)));
                            if (val.empty() || StringTools::equalsIgnoreCase(val, IniKeywords::Null)) {
                                continue;
                            }

                            std::vector<std::string>& vals = boundOfReg[key];
                            const bool seen = std::any_of(vals.begin(), vals.end(),
                                                          [&val](const std::string& had) { return StringTools::equalsIgnoreCase(had, val); });
                            if (!seen) {
                                vals.push_back(val);
                            }
                        }

                        for (const auto& [reg, vals] : boundOfReg) {
                            if (vals.size() < 2) {
                                continue;
                            }
                            for (const std::string& val : vals) {
                                data.setRegisterToggle(StringTools::toLower(val), IniParseData::TextureToggle{name + " " + reg, vals, false});
                            }
                        }
                    }
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

                /**
                 * The file each drawn slot takes for each role: every candidate the .ini offers, then
                 * TextureRoleChooser. The candidates are the roles above, the registers each slot's own
                 * sections bind (WWMITextureFacts::registerRoles), a sibling .ini's recolour of one of
                 * this character's textures, and another .ini of the mod binding textures on one of
                 * this file's draws.
                 */
                void chooseRoleFiles(IniParseData& data, const TextureRoles& roles) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    const std::string iniPath = ini->getFile().value_or(std::string());
                    const std::map<int, std::vector<std::string>> drawn = drawnSlots();

                    TextureRoleChooser chooser;
                    chooser.folder = ini->getFolder();
                    chooser.regionRoles = config_.textures.regionRoles;
                    // the toggles' variants, as files: a resource's toggle (IniParseData::thisToggleOf / registerToggleOf)
                    // names resources, each declared for one file
                    std::unordered_map<std::string, std::string> fileOfResource;
                    std::unordered_map<std::string, std::vector<std::string>> resourcesOfFile;
                    for (const auto& entry : roles.resourcesOf(iniPath)) {
                        chooser.declaredFiles.insert(entry.second);
                        fileOfResource.emplace(StringTools::toLower(entry.first), entry.second);
                        resourcesOfFile[entry.second].push_back(StringTools::toLower(entry.first));
                    }
                    chooser.variantsOf = [&data, fileOfResource, resourcesOfFile](const std::string& file) {
                        std::vector<std::string> files;
                        const auto resources = resourcesOfFile.find(file);
                        for (const std::string& resource : resources != resourcesOfFile.end() ? resources->second : std::vector<std::string>{}) {
                            for (const IniParseData::TextureToggle* toggle : {data.thisToggleOf(resource), data.registerToggleOf(resource)}) {
                                if (toggle == nullptr || toggle->variants.size() < 2) {
                                    continue;
                                }
                                for (const std::string& variant : toggle->variants) {
                                    const auto variantFile = fileOfResource.find(StringTools::toLower(variant));
                                    if (variantFile != fileOfResource.end()
                                            && std::find(files.begin(), files.end(), variantFile->second) == files.end()) {
                                        files.push_back(variantFile->second);
                                    }
                                }
                            }
                        }
                        return files.size() >= 2 ? files : std::vector<std::string>{};
                    };

                    addOwnCandidates(chooser, roles, data, iniPath, drawn);
                    if (!drawn.empty() && !iniPath.empty()) {
                        addSiblingRecolours(chooser, iniPath);
                        addSameDrawInis(chooser, iniPath, drawn);
                    }

                    std::vector<ModObj> objs;
                    for (const auto& entry : drawn) {
                        objs.push_back(slotObj(entry.first));
                    }
                    chooser.choose(data, objs);

                    // which file each texture resource names, so a line binding the resource can be asked
                    // its roles (IniParseData::rolesOfResource)
                    for (const auto& entry : roles.resourcesOf(iniPath)) {
                        data.setTextureResource(entry.first, entry.second);
                    }
                }

                // The roles above, and the ones the slots' own sections bind by register. A file the
                // .ini DECLARES but the folder does not hold is no candidate (2026-10-05): six of
                // thirteen Lynae mods name a texture they never shipped, from a dump's stale list, and on
                // the target that binds a missing resource where the role's fallback download is right.
                void addOwnCandidates(TextureRoleChooser& chooser, const TextureRoles& roles, const IniParseData& data,
                                      const std::string& iniPath, const std::map<int, std::vector<std::string>>& drawn) {
                    std::set<std::string> missing;
                    auto isMissing = [&missing](const std::string& file) {
                        std::error_code error;
                        if (std::filesystem::is_regular_file(FileService::strToPath(file), error)) {
                            return false;
                        }

                        missing.insert(file);
                        return true;
                    };

                    for (const auto& entry : roles.rolesOf()) {
                        if (isMissing(entry.first)) {
                            continue;
                        }

                        for (const TextureRoles::Role& role : entry.second) {
                            chooser.candidates[role.role].emplace_back(entry.first, role.how);
                            if (StringTools::startsWith(role.how, TextureRoles::DeclaredHash)) {
                                chooser.declared[entry.first].insert(role.role);
                            }
                        }
                    }

                    if (!config_.textures.registerRoles.empty()) {
                        std::unordered_map<std::string, std::string> fileOfResource;
                        for (const auto& entry : roles.resourcesOf(iniPath)) {
                            fileOfResource.emplace(StringTools::toLower(entry.first), entry.second);
                        }

                        // each slot section's own rows, in the order they are written
                        auto own = ownRows(data.collected(SlotsCollect), rootsOf([this](const ModObj& obj) { return isSlotObj(obj); }));
                        for (auto& entry : own) {
                            sortAsWritten(entry.second);
                        }

                        for (const auto& [slot, sections] : drawn) {
                            const auto layout = config_.textures.registerRoles.find(slot);
                            if (layout == config_.textures.registerRoles.end()) {
                                continue;
                            }

                            for (const std::string& section : sections) {
                                const auto rows = own.find(section);
                                if (rows == own.end()) {
                                    continue;
                                }

                                for (const auto& [reg, role] : layout->second) {
                                    // the first binding of the register in the section, its key matched loosely
                                    const std::string want = ModBranches::looseKey(reg);
                                    std::optional<std::string> bound;
                                    for (const Row* row : rows->second) {
                                        if (ModBranches::looseKey(row->key) == want) {
                                            bound = std::string(StringTools::strip(row->value));
                                            break;
                                        }
                                    }
                                    if (!bound.has_value()) {
                                        continue;
                                    }

                                    // `ps-t1 = ref ResourceFoo` names the same resource as `ps-t1 = ResourceFoo`
                                    const auto file = fileOfResource.find(StringTools::toLower(IniNamingTools::removeRefPrefix(*bound)));
                                    if (file == fileOfResource.end() || isMissing(file->second)) {
                                        continue;
                                    }

                                    chooser.candidates[role].emplace_back(file->second, "the " + reg + " its own section binds it at");
                                    chooser.bindings[file->second][slotObj(slot)].insert(role);
                                }
                            }
                        }
                    }

                    for (const std::string& file : missing) {
                        ctx_.log(FileService::pathToStr(FileService::strToPath(file).filename())
                                 + " is declared but not in the mod's folder; its role is treated as one the"
                                 + " mod has no file for");
                    }
                }

                // A TEXTURE-ONLY RECOLOUR IN A SIBLING .ini, of the form `hash = <the GAME texture it
                // replaces>` / `this = ResourceFoo`. On the mod's own character the game binds that
                // texture and the recolour shows; remapped, the slots take the TARGET's textures and the
                // hashes never appear. The role is STATED rather than guessed -- the hash is this
                // character's texture of that role -- and the mod's own candidates come first.
                void addSiblingRecolours(TextureRoleChooser& chooser, const std::string& iniPath) {
                    if (config_.textures.roles.empty()) {
                        return;
                    }

                    std::size_t found = 0;
                    const std::filesystem::path own = FileService::strToPath(iniPath);
                    for (const std::string& path : IniFile::loadedInis(own.parent_path(), own)) {
                        const std::unique_ptr<IniFile> sibling = otherIni(path);
                        const std::unique_ptr<WWMIParser> reader = readOther(*sibling);
                        for (const auto& [section, role, hash, file] : reader->recolours()) {
                            (void)section;
                            std::error_code error;
                            if (!std::filesystem::is_regular_file(FileService::strToPath(file), error)) {
                                continue;
                            }

                            chooser.candidates[role].emplace_back(
                                file, "a sibling .ini overrides " + hash + ", the game texture it replaces");
                            ++found;
                        }
                    }

                    if (found > 0) {
                        ctx_.log("a sibling .ini recolours " + std::to_string(found)
                                 + " of the game's textures; carried over to the target, which binds"
                                 + " its own and would never see those hashes");
                    }
                }

                // TEXTURES ANOTHER .ini OF THE MOD BINDS ON ONE OF THIS FILE'S DRAWS -- the same vb0 and
                // match_first_index. On the mod's own character both fire; remapped, the other never does,
                // so every role it serves would be downloaded from the GAME over the mod's art. Only the
                // registers WWMITextureFacts::registerRoles names for that slot are read, so this is the
                // mod stating a role. Looked for beside this .ini and in the two folders above it -- and
                // with WWMITextureFacts::readInisBelow, in every folder below it.
                void addSameDrawInis(TextureRoleChooser& chooser, const std::string& iniPath,
                                     const std::map<int, std::vector<std::string>>& drawn) {
                    const auto& layouts = config_.textures.registerRoles;
                    if (layouts.empty()) {
                        return;
                    }

                    const std::filesystem::path own = FileService::strToPath(iniPath);
                    std::vector<std::string> inis;
                    std::filesystem::path dir = own.parent_path();
                    for (int depth = 0; depth < 3 && !dir.empty(); ++depth) {
                        for (const std::string& path : IniFile::loadedInis(dir, own)) {
                            inis.push_back(path);
                        }

                        const std::filesystem::path parent = dir.parent_path();
                        if (parent == dir) {
                            break;
                        }
                        dir = parent;
                    }

                    if (config_.textures.readInisBelow) {
                        std::error_code error;
                        std::filesystem::recursive_directory_iterator it(own.parent_path(), error);
                        for (; !error && it != std::filesystem::recursive_directory_iterator(); it.increment(error)) {
                            if (!it->is_directory(error)) {
                                continue;
                            }

                            if (StringTools::startsWith(StringTools::toLower(FileService::pathToStr(it->path().filename())), "disabled")) {
                                it.disable_recursion_pending();
                                continue;
                            }

                            for (const std::string& path : IniFile::loadedInis(it->path(), own)) {
                                inis.push_back(path);
                            }
                        }
                    }

                    std::size_t found = 0;
                    for (const std::string& path : inis) {
                        const std::unique_ptr<IniFile> other = otherIni(path);
                        const std::unique_ptr<WWMIParser> reader = readOther(*other);
                        for (const auto& [slot, reg, role, file] : reader->drawBindings()) {
                            if (drawn.count(slot) == 0) {
                                continue;
                            }

                            std::error_code error;
                            if (!std::filesystem::is_regular_file(FileService::strToPath(file), error)) {
                                continue;
                            }

                            chooser.candidates[role].emplace_back(
                                file, "the " + reg + " " + FileService::pathToStr(FileService::strToPath(path).filename())
                                      + " binds on the same draw");
                            chooser.bindings[file][slotObj(slot)].insert(role);
                            ++found;
                        }
                    }

                    if (found > 0) {
                        ctx_.log("another .ini of the mod binds " + std::to_string(found)
                                 + " textures on this file's draws; carried over to the target, where"
                                 + " that .ini's overrides never fire");
                    }
                }

                // ---- another .ini of the mod, read through a parser of its own ----

                // Another .ini of the mod, as this character's: the same mod types, forced, and version
                std::unique_ptr<IniFile> otherIni(const std::string& path) const {
                    const IniFile* ini = const_cast<IniFileParseContext&>(ctx_).getIniFile();
                    return ini->sibling(path, modTypeId_.has_value()
                        ? std::optional<std::unordered_set<int>>(std::unordered_set<int>{*modTypeId_}) : std::nullopt);
                }

                // Parses 'other' with this character's parser in its only-this-file mode; its rows are read
                // through #recolours and #drawBindings, while 'other' lives
                std::unique_ptr<WWMIParser> readOther(IniFile& other) const {
                    auto reader = std::make_unique<WWMIParser>(&other, modTypeId_, config_, version_, true);
                    reader->parse();
                    return reader;
                }

                // resource (lowercased) -> the file it names: each section's first `filename`, from the
                // folder of this parser's .ini
                std::unordered_map<std::string, std::string> declaredFiles(const IniParseData& data) const {
                    const std::string folder = const_cast<IniFileParseContext&>(ctx_).getIniFile()->getFolder();
                    std::unordered_map<std::string, std::string> fileOf;
                    std::unordered_set<std::string> seen;
                    for (const Row& row : data.collected(DeclaredCollect)) {
                        if (row.key == IniKeywords::Filename && seen.insert(row.section).second) {
                            fileOf[StringTools::toLower(row.section)] = FileService::absPathOfRelPath(
                                FileService::iniPathToRel(std::string(StringTools::strip(row.value))), folder);
                        }
                    }
                    return fileOf;
                }

                // THIS FILE'S RECOLOURS: each texture override placed in a role's graph, its first own
                // `this`, as (section, role, hash, file), in the order the sections are written
                std::vector<std::tuple<std::string, std::string, std::string, std::string>> recolours() {
                    const IniParseData& data = this->parseData();
                    const auto fileOf = declaredFiles(data);
                    std::unordered_map<std::string, std::string> hashOf;
                    for (const Row& row : data.collected(TextureOverridesCollect)) {
                        if (row.key == IniKeywords::Hash) {
                            hashOf.emplace(row.section, StringTools::toLower(StringTools::strip(row.value).data()));
                        }
                    }

                    std::unordered_map<std::string, std::vector<const Row*>> own;
                    for (const Row& row : data.collected(TextureRolesCollect)) {
                        if (row.graphId.modObj.first == IniGraphModObjKeywords::Texture && row.key == IniKeywords::This
                                && row.section == row.rootSection) {
                            own[row.section].push_back(&row);
                        }
                    }

                    std::vector<std::string> sections;
                    for (const auto& entry : own) {
                        sections.push_back(entry.first);
                    }
                    std::sort(sections.begin(), sections.end(),
                              [this](const std::string& a, const std::string& b) { return sectionAt(a) < sectionAt(b); });

                    std::vector<std::tuple<std::string, std::string, std::string, std::string>> result;
                    for (const std::string& name : sections) {
                        if (!StringTools::startsWith(name, IniKeywords::TextureOverride)) {
                            continue;
                        }

                        std::vector<const Row*>& rows = own[name];
                        sortAsWritten(rows);
                        const auto file = fileOf.find(StringTools::toLower(std::string(StringTools::strip(rows.front()->value))));
                        const auto hash = hashOf.find(name);
                        if (file != fileOf.end() && hash != hashOf.end()) {
                            result.emplace_back(name, rows.front()->graphId.modObj.second, hash->second, file->second);
                        }
                    }
                    return result;
                }

                // THIS FILE'S BINDINGS ON THE SLOTS' DRAWS: every value a slot section binds, through `run =`,
                // at a register WWMITextureFacts::registerRoles names for the slot, as (slot, register, role,
                // file) -- the sections in the order they are written, each register in the table's order,
                // each part counted once
                std::vector<std::tuple<int, std::string, std::string, std::string>> drawBindings() {
                    const IniParseData& data = this->parseData();
                    const auto fileOf = declaredFiles(data);

                    std::unordered_map<std::string, std::vector<const Row*>> byRoot;
                    std::set<std::tuple<std::string, std::string, std::size_t, long long, std::string>> seen;
                    for (const Row& row : data.collected(TextureRolesCollect)) {
                        if (isSlotObj(row.graphId.modObj)
                                && seen.emplace(row.rootSection, row.section, row.partId, row.line, row.key).second) {
                            byRoot[row.rootSection].push_back(&row);
                        }
                    }

                    std::vector<std::string> sections;
                    for (const auto& entry : byRoot) {
                        sections.push_back(entry.first);
                    }
                    std::sort(sections.begin(), sections.end(),
                              [this](const std::string& a, const std::string& b) { return sectionAt(a) < sectionAt(b); });

                    std::vector<std::tuple<int, std::string, std::string, std::string>> result;
                    for (const std::string& name : sections) {
                        const std::vector<const Row*>& rows = byRoot[name];
                        std::set<int> slots;
                        for (const Row* row : rows) {
                            if (const std::optional<int> slot = slotOf(row->graphId.modObj)) {
                                slots.insert(*slot);
                            }
                        }

                        for (const int slot : slots) {
                            const auto layout = config_.textures.registerRoles.find(slot);
                            if (layout == config_.textures.registerRoles.end()) {
                                continue;
                            }

                            for (const auto& [reg, role] : layout->second) {
                                const std::string want = ModBranches::looseKey(reg);
                                for (const Row* row : rows) {
                                    if (slotOf(row->graphId.modObj) != slot || ModBranches::looseKey(row->key) != want) {
                                        continue;
                                    }

                                    const std::string val = IniNamingTools::removeRefPrefix(std::string(StringTools::strip(row->value)));
                                    const auto file = fileOf.find(StringTools::toLower(val));
                                    if (file != fileOf.end()) {
                                        result.emplace_back(slot, reg, role, file->second);
                                    }
                                }
                            }
                        }
                    }
                    return result;
                }

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

                void buildRoles(TextureRoles& out, const IniParseData& data) const {
                    IniFile* ini = const_cast<IniFileParseContext&>(ctx_).getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    const std::string folder = ini->getFolder();
                    const std::string iniPath = ini->getFile().value_or(std::string());

                    // NOTE: a section a PREVIOUS fix declared is reported here like any other, and
                    // deliberately. `[Resource<Role><Target>RemapRef]` names one of the MOD'S own
                    // textures, so dropping it here drops the texture: the role then has no file and
                    // falls back to downloading the GAME's, over the mod's own art. What must not
                    // happen is the FIXER reusing that section's NAME, since the undo deletes the
                    // section -- see WWMIFixer's assignRole, which declares a fresh one instead.
                    // resource -> the file it names: each section's first `filename`
                    std::unordered_map<std::string, std::string> fileOf;
                    std::unordered_set<std::string> declared;
                    for (const Row& row : data.collected(DeclaredCollect)) {
                        if (row.key != IniKeywords::Filename || !declared.insert(row.section).second) {
                            continue;
                        }

                        const std::string file = FileService::absPathOfRelPath(
                            FileService::iniPathToRel(std::string(StringTools::strip(row.value))), folder);
                        fileOf[StringTools::toLower(row.section)] = file;
                        out.addResource(iniPath, row.section, file);
                    }

                    // each section's first `hash`, which a role stated by hash is reported under
                    std::unordered_map<std::string, std::string> hashOf;
                    for (const Row& row : data.collected(TextureOverridesCollect)) {
                        if (row.key == IniKeywords::Hash) {
                            hashOf.emplace(row.section, StringTools::toLower(StringTools::strip(row.value).data()));
                        }
                    }

                    // the rows each root section states its roles by, the sections in the order they are written
                    std::unordered_map<std::string, std::vector<const Row*>> byRoot;
                    for (const Row& row : data.collected(TextureRolesCollect)) {
                        byRoot[row.rootSection].push_back(&row);
                    }
                    std::vector<std::string> sections;
                    for (const auto& entry : byRoot) {
                        sections.push_back(entry.first);
                    }
                    std::sort(sections.begin(), sections.end(),
                              [this](const std::string& a, const std::string& b) { return sectionAt(a) < sectionAt(b); });

                    for (const std::string& name : sections) {
                        const std::vector<const Row*>& rows = byRoot[name];

                        // (1) the section IS a texture -- the parser placed it in its role's graph by its hash,
                        // and `this` names the file
                        for (const Row* row : rows) {
                            if (row->graphId.modObj.first != IniGraphModObjKeywords::Texture || row->key != IniKeywords::This) {
                                continue;
                            }

                            const auto file = fileOf.find(StringTools::toLower(std::string(StringTools::strip(row->value))));
                            const auto hash = hashOf.find(name);
                            if (file != fileOf.end()) {
                                out.add(file->second, row->graphId.modObj.second,
                                        TextureRoles::DeclaredHash + (hash == hashOf.end() ? std::string() : hash->second));
                            }
                        }

                        // (2) the section is a slot's DRAW -- its registers say what they bind, read through
                        // `run =`: a section may carry only a hash and a call
                        for (const Row* row : rows) {
                            const std::optional<int> slot = slotOf(row->graphId.modObj);
                            const auto table = slot.has_value() ? config_.textures.registerRoles.find(*slot)
                                                                : config_.textures.registerRoles.end();
                            if (table == config_.textures.registerRoles.end()) {
                                continue;
                            }

                            const auto role = table->second.find(row->key);
                            if (role == table->second.end()) {
                                continue;
                            }

                            const std::string val = IniNamingTools::removeRefPrefix(std::string(StringTools::strip(row->value)));
                            const auto file = fileOf.find(StringTools::toLower(val));
                            if (file != fileOf.end()) {
                                out.add(file->second, role->second, "bound at " + row->key + " of " + row->graphId.modObj.second);
                            }
                        }
                    }

                    // the role a hash of the character's plays: HashData first, the config's table after it
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
                    for (const auto& entry : fileOf) {
                        const std::string& file = entry.second;
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

            private:
                Classifier* classifier_ = nullptr;     // owned by the base (ownClassifier)
                std::unordered_set<std::string> watchHashes_;     // her slot hashes at every version, lowercased
                WWMIParserConfig config_;
                std::optional<Version> version_;
                std::optional<int> modTypeId_;
                bool onlyThisFile_ = false;
                mutable std::unordered_map<std::string, std::size_t> sectionAt_;
                mutable std::map<std::pair<std::string, std::size_t>, std::size_t> partAt_;
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
