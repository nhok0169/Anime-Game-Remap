#include "AGRemapCore/data/IniParseData/WWMIParser.h"

#include "AGRemapCore/data/RabbitFX.h"
#include "AGRemapCore/model/assets/Hashes.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCreate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphVarApply.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/buffers/SkeletonNumberingProbe.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/data/IniParseData/TemplateParser.h"
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

#include <algorithm>
#include <stdexcept>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniGraphModObjKeywords.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/WWMINames.h"
#include "AGRemapCore/data/HashData.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"


namespace AGRemapCore {
    namespace {
        using Parser = TemplateParser;
        using Classifier = Parser::Classifier;
        using ModObj = Parser::ModObj;


        /**
         * A GIMIParser that owns its context and its classifier -- the same ownership story as
         * GIMICharGIMIParser, minus the downloads a WuWa character does not register (yet).
         *
         * The draw slots are counted off the character's OWN mod type here (ModType::getComponentCount), at parse time, and not
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
                    // the character's game data off the versioned asset tables (WWMIParserConfig::withGameData) -- a
                    // linked file's parser (readOther) is handed its parent's config, already filled at the same version
                    const std::optional<ModType> own = modTypeId_.has_value() ? ModTypeIdTools::getModType(*modTypeId_) : std::nullopt;
                    if (own.has_value() && !onlyThisFile_) {
                        config_ = config_.withGameData(*own, version_);
                    }
                    this->keysToTrack = std::unordered_set<std::string>{IniKeywords::Hash, IniKeywords::MatchFirstIndex, IniKeywords::MatchIndexCount};

                    // The slots: component0, component1, ... as many as the .ini's own mod type has (ModType::getComponentCount).
                    const std::string ownName = ctx_.modTypeName();
                    std::vector<ModObj> modObjs;
                    Classifier::IndexModObjs slotObjs;
                    const ModType* modType = nullptr;
                    if (IniFile* ini = ctx_.getIniFile(); ini != nullptr && modTypeId_.has_value()) {
                        const auto found = ini->getModTypes().find(*modTypeId_);
                        modType = found != ini->getModTypes().end() ? &found->second : nullptr;
                    }
                    const std::size_t slots = ctx_.modTypeIndices() != nullptr && modType != nullptr && !ownName.empty()
                        ? modType->getComponentCount(WWMINames::SlotPrefix, version) : 0;
                    for (std::size_t i = 0; i < slots; ++i) {
                        const std::string slot = WWMINames::SlotPrefix + std::to_string(i);
                        const ModObj obj("", slot);
                        modObjs.push_back(obj);
                        slotObjs[Classifier::IndexKey("", slot)] = obj;
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

                            const std::optional<std::string> role = WWMITextureFacts::roleOfHashType(row.first[2], roles);
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
                            if (row.first.size() >= 3 && row.first[1] == ownName && row.first[2] == IniKeywords::Vb0) {
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
                    std::unordered_map<std::string, Classifier::IndexModObjs> indexMap{{IniKeywords::Vb0, std::move(slotObjs)}};
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
                inline static const std::string TextureOverridesCollect = WWMINames::TextureOverridesRows;
                inline static const std::string DeclaredCollect = IniParseRows::Declared;
                inline static const std::string TextureRolesCollect = WWMINames::TextureRolesRows;
                inline static const std::string TextureTogglesCollect = WWMINames::TextureTogglesRows;
                inline static const std::string CarriedCollect = WWMINames::CarriedRows;

                using Row = IniParseData::CollectedRow;


                // The parser's graphs and the sections it placed in each, in its order (IniParseData::SectionTargets)
                IniParseData::SectionTargets targetList() const {
                    return IniParseData::sectionTargetsOf(this->sectionTargets());
                }

                // Where the parser's reading logs
                std::function<void(const std::string&)> logger() {
                    return [this](const std::string& message) { ctx_.log(message); };
                }

                /**
                 * The edits that read the mod, all deferred: they run on the first ask of the parse data, since
                 * sorting the textures opens every candidate, and a parser is built for every `.ini` a run
                 * classifies -- most of which no WuWa fixer will ever ask about.
                 *
                 * (1) the standard rows (IniParseRows), off the parser's global graph: the files of the resources
                 * the mod binds, every section's declaration and every key of every section, and every key of every
                 * slot graph;
                 * (2) the hash and the `this` of every texture override; (3) the textures' roles and toggles;
                 * then the variables made from those rows (GraphVarApply): (4) the skeleton numbering; (5) the
                 * resource data (IniParseData::resourceDataOf, by WWMI's rules); (6) the textures' role files
                 * (IniParseData::roleFilesOf); (7) the draw splits, added to the resource data.
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
                    const std::string regPrefix = StringTools::toLower(IniKeywords::TexRegPrefix);
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

                        const std::optional<int> slot = config_.slotOf(obj);
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

                    // THE STANDARD ROWS (IniParseRows), off the parser's global graph: a copy of it put in the
                    // groups; the resource sections a line binds anywhere built as a graph (ResIdentity: nothing renamed,
                    // no models) and their `filename`; every section's declarations, with [Constants]' mesh vertex
                    // count; every key of every section; both graphs taken out again; and every key of each slot
                    const GraphId globalGraph{0, "global", ""};
                    const GraphId resourceGraph{0, "resource", ""};
                    const auto anyKey = [](const std::string&) { return true; };

                    auto global = std::make_unique<GraphCreate<>>(globalGraph, nullptr);
                    global->graphOf = [this]() { return this->globalGraph(); };

                    BaseResEdit<>::ResEditConfig resourceNames{IniKeywords::Filename,
                        [this](const std::string& value) { return this->resourceNamed(value); },
                        [](const std::string& file) { return file; }};
                    auto resourceSections = std::make_unique<ResRegCollect<>>();
                    resourceSections->srcRegMatches.insert_or_assign(globalGraph, anyKey);
                    resourceSections->resPredicates.insert_or_assign(globalGraph,
                        [this](const std::string&, const std::string& value, const ResRegCollect<>::IterData&) {
                            return !this->resourceNamed(value).empty();
                        });
                    resourceSections->resEdits.insert_or_assign("", &this->ownResEdit(
                        std::make_unique<ResIdentity<>>(resourceGraph, std::move(resourceNames), false)));

                    auto files = std::make_unique<Collect>(Collect::ByGraph<std::vector<std::string>>{
                        {resourceGraph, {IniKeywords::Filename}}});
                    files->name = IniParseRows::Resources;
                    auto declared = std::make_unique<Collect>(Collect::ByGraph<std::vector<std::string>>{
                        {globalGraph, {IniKeywords::Filename, IniKeywords::Type, IniKeywords::Format, IniKeywords::Stride,
                                       IniKeywords::Global + " " + IniKeywords::WWMIMeshVertexCount}}});
                    declared->name = IniParseRows::Declared;
                    auto sections = std::make_unique<Collect>();
                    sections->name = IniParseRows::Sections;
                    sections->keyMatches.insert_or_assign(globalGraph, anyKey);
                    auto removeGraphs = std::make_unique<GraphRemove<>>(std::vector<GraphId>{resourceGraph, globalGraph});
                    auto slots = std::make_unique<Collect>();
                    slots->name = SlotsCollect;
                    for (const ModObj& obj : this->modObjs()) {
                        if (config_.slotOf(obj).has_value()) {
                            slots->keyMatches.insert_or_assign(GraphId(0, obj.first, obj.second), anyKey);
                        }
                    }

                    // the skeleton numbering, made from the slot rows and the resource rows
                    auto skeleton = std::make_unique<GraphVarApply>(
                        std::vector<std::string>{SlotsCollect, IniParseRows::Resources},
                        [this](const std::vector<const GraphEditVariables::Value*>& inputs) { return skeletonNumbering(inputs); },
                        IniParseData::BlendVersion);

                    // the resource data, made from the rows
                    const std::vector<std::string> resourceInputs{SlotsCollect, IniParseRows::Resources, IniParseRows::Sections,
                                                                  TextureTogglesCollect, TextureOverridesCollect};
                    auto resources = std::make_unique<GraphVarApply>(resourceInputs,
                        [this, resourceInputs](const std::vector<const GraphEditVariables::Value*>& inputs) {
                            return resourceData(GraphEditVariables::of(resourceInputs, inputs));
                        }, IniParseData::ResourceDataVar);

                    // the file each slot takes for each texture role, made from the rows and the resource data
                    const std::vector<std::string> roleInputs{IniParseRows::Sections, DeclaredCollect, TextureOverridesCollect,
                                                              TextureRolesCollect, SlotsCollect, IniParseData::ResourceDataVar};
                    auto roleFiles = std::make_unique<GraphVarApply>(roleInputs,
                        [this, roleInputs](const std::vector<const GraphEditVariables::Value*>& inputs) {
                            return textureRoleFiles(GraphEditVariables::of(roleInputs, inputs));
                        }, IniParseData::RoleFilesVar);

                    // the draw splits, added to the resource data
                    auto splits = std::make_unique<GraphVarApply>(
                        std::vector<std::string>{IniParseData::ResourceDataVar, IniParseData::RoleFilesVar},
                        [this](const std::vector<const GraphEditVariables::Value*>& inputs) { return drawSplits(inputs); },
                        IniParseData::ResourceDataVar);

                    // WHAT A LINKED FILE SETS THAT A FIX CARRIES AS IT IS (IniParseData::LinkedLine): RabbitFX's glow
                    // map on a slot's draw, with its condition -- read from a file parsed in its only-this-file mode
                    auto carried = std::make_unique<Collect>();
                    carried->name = CarriedCollect;
                    carried->withConditions = true;
                    for (const ModObj& obj : onlyThisFile_ && readsTextures ? this->modObjs() : std::vector<ModObj>{}) {
                        if (config_.slotOf(obj).has_value()) {
                            carried->keyMatches.insert_or_assign(GraphId(0, obj.first, obj.second), [](const std::string& key) {
                                return IniNamingTools::sameKey(key, RabbitFX::GlowMapKey);
                            });
                        }
                    }

                    // THE EDITS: the ones that read the mod, then the variables made from their rows -- run on the
                    // first ask of the parse data
                    this->deferCommandEdits = true;
                    this->addCommandEdit(std::move(global));
                    this->addCommandEdit(std::move(resourceSections));
                    this->addCommandEdit(std::move(files));
                    this->addCommandEdit(std::move(declared));
                    this->addCommandEdit(std::move(sections));
                    this->addCommandEdit(std::move(removeGraphs));
                    this->addCommandEdit(std::move(slots));
                    this->addCommandEdit(std::move(overrides));
                    this->addCommandEdit(std::move(roles));
                    this->addCommandEdit(std::move(toggles));
                    this->addCommandEdit(std::move(carried));
                    if (!onlyThisFile_) {
                        this->addCommandEdit(std::move(skeleton));
                    }
                    this->addCommandEdit(std::move(resources));
                    if (!onlyThisFile_) {
                        this->addCommandEdit(std::move(roleFiles));
                        this->addCommandEdit(std::move(splits));
                    }
                }

                // ---- WWMI's reading of the rows: the mod's resource data, as WWMIFixer asks for it ----
                // (WWMIParserConfig::resourceDataRules, WWMIParserConfig::replacesTexturesByHash)
                std::optional<GraphEditVariables::Value> resourceData(const GraphEditVariables& rows) {
                    const WWMIParserConfig::SectionTargets targets = targetList();
                    const IniParseData::ResourceDataRules rules = config_.resourceDataRules(
                        targets, ctx_.sectionNames(), onlyThisFile_, logger());

                    IniParseData::ResourceData start;
                    const std::vector<Row>* overrideRows = rows.find<std::vector<Row>>(TextureOverridesCollect);
                    start.replacesTexturesByHash = overrideRows != nullptr && config_.replacesTexturesByHash(*overrideRows, targets);
                    return GraphEditVariables::Value::of(IniParseData::resourceDataOf(rules, rows, std::move(start)));
                }

                // THE TEXTURE ROLES (IniParseData::roleFilesOf) by WWMIParserConfig::roleFileRules, and the linked .ini
                // files, each read by this parser in its only-this-file mode
                std::optional<GraphEditVariables::Value> textureRoleFiles(const GraphEditVariables& rows) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return std::nullopt;
                    }

                    // her mod type: her hashes, her name and the mods she fixes
                    const std::optional<ModType> own = modTypeId_.has_value() ? ModTypeIdTools::getModType(*modTypeId_) : std::nullopt;
                    std::optional<IniParseData::RoleFileRules> made = config_.roleFileRules(
                        targetList(), ctx_.sectionNames(), own.has_value() ? &*own : nullptr,
                        ini->getFile().value_or(std::string()), ini->getFolder());
                    if (!made.has_value()) {
                        return std::nullopt;
                    }
                    IniParseData::RoleFileRules& rules = *made;
                    rules.log = logger();

                    // the files LINKED to this one (IniFile::linkedInis: a namespace one references, a resource file both
                    // name), wherever they are, each parsed by this character's parser in its only-this-file mode
                    std::vector<std::unique_ptr<IniFile>> others;
                    std::vector<std::unique_ptr<WWMIParser>> readers;
                    for (IniFile* linked : rules.objs.empty() || rules.iniPath.empty() ? std::vector<IniFile*>{} : ini->linkedInis()) {
                        others.push_back(otherIni(*linked));
                        readers.push_back(readOther(*others.back()));
                        rules.linked.push_back({&readers.back()->parseData(), others.back()->getFolder(),
                                                FileService::pathToStr(FileService::strToPath(linked->getFile().value_or("")).filename()),
                                                linked->getFile().value_or("")});
                    }

                    return GraphEditVariables::Value::of(IniParseData::roleFilesOf(rules, rows));
                }

                // Which numbering the blend's bone ids are in (WWMIParserConfig::skeletonNumbering), from the slot rows
                // and the resource rows -- the version, or nothing
                std::optional<GraphEditVariables::Value> skeletonNumbering(const std::vector<const GraphEditVariables::Value*>& inputs) {
                    static const std::vector<Row> none;
                    const auto rowsOf = [](const GraphEditVariables::Value* value) -> const std::vector<Row>& {
                        const std::vector<Row>* rows = value == nullptr ? nullptr : value->as<std::vector<Row>>();
                        return rows == nullptr ? none : *rows;
                    };

                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return std::nullopt;
                    }

                    const std::optional<std::string> numbering = config_.skeletonNumbering(
                        rowsOf(inputs[0]), rowsOf(inputs[1]), targetList(), ini->getFolder(), logger());
                    if (!numbering.has_value()) {
                        return std::nullopt;
                    }
                    return GraphEditVariables::Value::of<std::string>(*numbering);
                }

                // The resource data (inputs[0]) with the draw splits' draws added (WWMIParserConfig::splitDraws), read
                // with the role files (inputs[1])
                std::optional<GraphEditVariables::Value> drawSplits(const std::vector<const GraphEditVariables::Value*>& inputs) {
                    const IniParseData::ResourceData* data = inputs[0] == nullptr ? nullptr : inputs[0]->as<IniParseData::ResourceData>();
                    const IniParseData::RoleFileData* roleData = inputs[1] == nullptr ? nullptr : inputs[1]->as<IniParseData::RoleFileData>();
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return std::nullopt;
                    }

                    std::optional<IniParseData::ResourceData> split = config_.splitDraws(
                        data, roleData, targetList(), ini->getFile().value_or(std::string()), ini->getFolder(), logger());
                    if (!split.has_value()) {
                        return std::nullopt;
                    }
                    return GraphEditVariables::Value::of(std::move(*split));
                }

                // A section a previous fix wrote: its name ends `<target>RemapFix`
                static bool isFixSection(const std::string& name) {
                    return StringTools::endsWithIgnoreCase(name, IniKeywords::RemapFix);
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

                // ---- another .ini of the mod, read through a parser of its own ----

                // Another .ini of the mod, as this character's: the same mod types, forced, and version
                std::unique_ptr<IniFile> otherIni(const IniFile& linked) const {
                    const IniFile* ini = const_cast<IniFileParseContext&>(ctx_).getIniFile();
                    return ini->sibling(linked, modTypeId_.has_value()
                        ? std::optional<std::unordered_set<int>>(std::unordered_set<int>{*modTypeId_}) : std::nullopt);
                }

                // Parses 'other' with this character's parser in its only-this-file mode; its rows are read
                // through IniParseData::roleFilesOf (RoleFileRules::linked), while 'other' lives
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
        };
    }


    std::shared_ptr<BaseIniParser<>> WWMIParseFactory::operator()(IniFile* iniFile, std::optional<int> modTypeId) const {
        std::optional<Version> version = Version::parse(config.version);
        if (iniFile != nullptr && iniFile->fromVersion.has_value()) {
            version = iniFile->fromVersion;
        }

        return std::make_shared<WWMIParser>(iniFile, modTypeId, config, version);
    }

    IniParseBuilder::Factory makeWWMIParser(WWMIParserConfig config) {
        return WWMIParseFactory{std::move(config)};
    }

    WWMIParserConfig WWMIParserConfig::withGameData(const ModType& self, const std::optional<Version>& version) const {
        WWMIParserConfig config = *this;
        const std::optional<Version> at = version.has_value() ? version : Version::parse(config.version);

        // her game textures' thumbprints, at her version
        std::unordered_map<std::string, std::vector<std::uint8_t>> thumbprints = self.getTextureThumbprints(at);
        if (!thumbprints.empty()) {
            config.textures.textureThumbprints = std::move(thumbprints);
        }

        // each draw split's flags, by name off her code map's MaterialCodes rows
        for (DrawSplit& split : config.drawSplits) {
            if (split.flag.empty()) {
                continue;
            }
            const std::string component = WWMINames::SlotPrefix + std::to_string(split.component);
            const std::string type = split.codeRole + ".flags." + split.flag;
            const std::optional<long long> code = self.getMaterialCode(component, type, at);
            if (!code.has_value()) {
                throw std::invalid_argument("WWMIParserConfig: no MaterialCodes row for " + self.name + " " + component + " " + type);
            }
            split.flags = static_cast<int>(*code);
        }

        std::vector<Version> versions;
        for (const auto& [hash, version] : WWMIFixerConfig::vb0VersionsOf(self)) {
            (void)hash;
            const std::optional<Version> parsed = Version::parse(version);
            if (parsed.has_value() && std::find(versions.begin(), versions.end(), *parsed) == versions.end()) {
                versions.push_back(*parsed);
            }
        }
        if (versions.size() < 2) {
            return config;
        }
        std::sort(versions.begin(), versions.end());

        const Version& reference = versions.back();
        std::vector<SkeletonNumberingProbe::Numbering> numberings;
        for (const Version& version : versions) {
            const std::optional<VGRemap> row = self.getVGRemap(self.name, version, reference, std::string(""), std::string(""));
            if (!row.has_value()) {
                return config;            // not a renumbered skeleton: no row from this model onto the newest
            }
            SkeletonNumberingProbe::Numbering numbering;
            numbering.version = version.toString();
            for (const auto& [from, to] : row->getRemap()) {
                if (from != to) {
                    numbering.toReference.emplace(from, to);
                }
            }
            numberings.push_back(std::move(numbering));
        }
        config.skeletonNumberings = std::move(numberings);

        const std::map<long long, std::array<double, 3>> centroids = self.getBoneCentroids(reference);
        if (!centroids.empty()) {
            config.referenceBoneCentroids = {centroids.begin(), centroids.end()};
        }
        return config;
    }

    std::optional<WWMIParserConfig> WWMIParserConfig::ofFactory(const IniParseBuilder::Factory& factory) {
        const WWMIParseFactory* made = factory.target<WWMIParseFactory>();
        return made != nullptr ? std::optional<WWMIParserConfig>(made->config) : std::nullopt;
    }
}
