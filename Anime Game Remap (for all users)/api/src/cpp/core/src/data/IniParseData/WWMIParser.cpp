#include "AGRemapCore/data/IniParseData/WWMIParser.h"

#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/data/IniParseData/TextureOverrides.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoleChooser.h"
#include "AGRemapCore/model/strategies/iniParsers/TextureRoles.h"
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
#include <optional>
#include <string>
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
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegDrawRangeRemap.h"
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
        class WWMIParser: public Parser {
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
                                     ContentPart* part, const Colouring* kvps) {
                            if (kvps == nullptr) {
                                return std::vector<ModObj>();
                            }

                            std::vector<ModObj> objs = classifier->classify(sectionName, section, *kvps);
                            if (objs.empty() && isUnknownTextureOverride(sectionName, section, part)) {
                                objs.emplace_back(IniGraphModObjKeywords::Texture, "");
                            }

                            // WHAT THE CLASSIFIER DECIDED, KEPT. The texture roles are read off the
                            // sections it placed, and re-deriving "which component is this" beside
                            // it would be a second answer to a question it has already answered.
                            if (!objs.empty()) {
                                classified_[sectionName] = objs;
                            }

                            return objs;
                        });

                    recordParseData();
                }

                void clear() override {
                    Parser::clear();
                    recordParseData();
                }

            private:
                /**
                 * The texture roles go into the parse data as a BUILDER, run on the first ask: sorting
                 * opens every candidate texture, and a parser is built for every `.ini` a run
                 * classifies -- most of which no WuWa fixer will ever ask about. None when the
                 * character's config carries no textures to sort by. Re-installed after a clear,
                 * which resets the parse data.
                 */
                void recordParseData() {
                    this->parseData().defer([this](IniParseData& data) {
                        readAnatomy(data);
                        if (config_.textures.roles.empty() && config_.textures.registerRoles.empty()) {
                            return;
                        }

                        TextureRoles roles;
                        buildRoles(roles);
                        data.setTextureRoles(std::move(roles));
                        chooseRoleFiles(data, *data.textureRoles());
                        readThisToggles(data);
                        readRegisterToggles(data);
                    });
                }

                // ---- the mod's anatomy: what its slot sections draw and bind, and its resource files ----

                // The parser's slot object for a component's sections, or nothing
                bool isSlotObj(const ModObj& obj) const {
                    return obj.first.empty() && StringTools::startsWith(obj.second, config_.slotPrefix);
                }

                void readAnatomy(IniParseData& data) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    const auto& templates = ini->getIfTemplates();
                    const auto contentParts = [](const Section& section) {
                        std::vector<const ContentPart*> parts;
                        for (const auto& part : section.parts()) {
                            if (const auto* content = dynamic_cast<const ContentPart*>(part.get())) {
                                parts.push_back(content);
                            }
                        }
                        return parts;
                    };

                    // every resource section that names a file
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        std::optional<std::string> file = ModBranches::firstVal(*entry.second, IniKeywords::Filename);
                        if (file.has_value()) {
                            data.setResourceFile(entry.first, *file);
                        }
                    }

                    // DOES THE MOD REPLACE TEXTURES BY HASH? A texture override (not a slot's) with a hash and
                    // a `this =`: its art then reaches every draw binding that texture, whatever the draw's
                    // own section binds.
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr || !StringTools::startsWith(entry.first, IniKeywords::TextureOverride)
                                || StringTools::startsWith(entry.first, IniKeywords::TextureOverride + config_.slotPrefix)) {
                            continue;
                        }

                        bool hasHash = false;
                        bool hasThis = false;
                        for (const ContentPart* content : contentParts(*entry.second)) {
                            hasHash = hasHash || content->count(IniKeywords::Hash) > 0;
                            hasThis = hasThis || content->count(IniKeywords::This) > 0;
                        }

                        if (hasHash && hasThis) {
                            data.setReplacesTexturesByHash(true);
                            break;
                        }
                    }

                    // A `run =` this file cannot resolve counts as drawing, unless it is WWMI's own library
                    std::function<bool(const std::string&, std::unordered_set<std::string>&)> mayDraw =
                        [&templates, &mayDraw, &contentParts](const std::string& section, std::unordered_set<std::string>& seen) -> bool {
                            if (!seen.insert(StringTools::toLower(section)).second) {
                                return false;
                            }

                            auto tpl = templates.find(section);
                            if (tpl == templates.end() || tpl->second == nullptr) {
                                return StringTools::toLower(section).find("\\wwmiv1\\") == std::string::npos;
                            }

                            for (const ContentPart* content : contentParts(*tpl->second)) {
                                for (const auto& item : content->items()) {
                                    const std::string key = StringTools::toLower(std::string(StringTools::strip(item.key)));
                                    if (StringTools::startsWith(key, "draw")) {
                                        return true;
                                    }

                                    if (key == StringTools::toLower(IniKeywords::Run)
                                        && mayDraw(std::string(StringTools::strip(item.value)), seen)) {
                                        return true;
                                    }
                                }
                            }

                            return false;
                        };

                    const auto skips = [&templates, &contentParts](const std::string& section) -> bool {
                        auto tpl = templates.find(section);
                        if (tpl == templates.end() || tpl->second == nullptr) {
                            return false;
                        }

                        for (const ContentPart* content : contentParts(*tpl->second)) {
                            for (const std::string& value : content->getVals(IniKeywords::Handling)) {
                                if (StringTools::equalsIgnoreCase(std::string(StringTools::strip(value)), IniKeywords::Skip)) {
                                    return true;
                                }
                            }
                        }

                        return false;
                    };

                    std::size_t slots = 0;
                    std::vector<ModObj> silent;
                    for (const auto& [obj, sections] : this->sectionTargets()) {
                        if (!isSlotObj(obj) || sections.empty()) {
                            continue;
                        }

                        ++slots;
                        bool drew = false;
                        bool draws = false;
                        bool skipped = false;
                        std::vector<std::string> late;
                        for (const std::string& section : sections) {
                            auto tpl = templates.find(section);
                            if (tpl == templates.end() || tpl->second == nullptr) {
                                continue;
                            }

                            // its draws, and which part each sits in (every part counted, `if` lines too)
                            std::size_t partIndex = 0;
                            for (const auto& part : tpl->second->parts()) {
                                ++partIndex;
                                const auto* content = dynamic_cast<const ContentPart*>(part.get());
                                if (content == nullptr) {
                                    continue;
                                }

                                for (const std::string& draw : content->getVals(IniKeywords::DrawIndexed)) {
                                    data.addDrawPart(obj, partIndex);
                                    if (std::count(draw.begin(), draw.end(), ',') < 2) {
                                        continue;
                                    }

                                    const std::optional<std::vector<long long>> numbers = RegDrawRangeRemap<>::parseRange(draw);
                                    if (numbers.has_value() && numbers->size() >= 3) {
                                        data.addDrawRange(obj, {(*numbers)[0], (*numbers)[1]});
                                    } else {
                                        // a dropped draw range is a piece of the mesh that stops being remapped
                                        ctx_.log("could not read the draw range `" + std::string(StringTools::strip(draw)) + "` in ["
                                                 + section + "], so it is not remapped");
                                    }
                                }
                            }

                            // the texture registers it binds only after it has drawn
                            for (const ContentPart* content : contentParts(*tpl->second)) {
                                for (const auto& item : content->items()) {
                                    const std::string key = StringTools::toLower(std::string(StringTools::strip(item.key)));
                                    if (key == StringTools::toLower(IniKeywords::DrawIndexed)) {
                                        drew = true;
                                    } else if (drew && StringTools::startsWith(key, StringTools::toLower(config_.texRegPrefix))) {
                                        ListTools::pushDistinct(late, key);
                                    }
                                }
                            }

                            std::unordered_set<std::string> seen;
                            draws = draws || mayDraw(section, seen);
                            skipped = skipped || skips(section);
                        }

                        if (!late.empty() && !data.replacesTexturesByHash()) {
                            data.setLateBindings(obj, std::move(late));
                        }

                        // A SLOT WHOSE SECTION DRAWS NOTHING AND SKIPS THE GAME'S DRAW IS UNDRAWN: the mod
                        // hides that component. One that draws nothing and leaves the game's geometry on
                        // screen is not hidden.
                        if (!draws && skipped) {
                            silent.push_back(obj);
                        }
                    }

                    // ...unless every slot is, which is a mod this reading does not understand
                    if (silent.size() < slots) {
                        for (const ModObj& obj : silent) {
                            data.addUndrawn(obj);
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
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    const auto& templates = ini->getIfTemplates();
                    std::unordered_map<std::string, std::string> ownerOf;            // resource -> the section binding it
                    std::unordered_map<std::string, std::vector<std::string>> variantsOf;
                    std::unordered_set<std::string> single;
                    for (const auto& entry : templates) {
                        // ANY texture override binding by hash, whatever its name: Lynae3 calls its hair
                        // overrides [TextureOverrideX] / [TextureOverrideXX]; a mesh section (one with
                        // match_first_index) is not a texture's override
                        if (entry.second == nullptr || !StringTools::startsWith(entry.first, IniKeywords::TextureOverride)
                                || isFixSection(entry.first)) {
                            continue;
                        }

                        std::vector<std::string> bound;
                        bool meshSection = false;
                        bool realToggle = false;
                        for (const auto& part : entry.second->parts()) {
                            if (const auto* pred = dynamic_cast<const IfPredPart*>(part.get())) {
                                if (pred->type == IfPredPartType::If || pred->type == IfPredPartType::Elif) {
                                    std::string test = StringTools::toLower(pred->getTestStr());
                                    test.erase(std::remove_if(test.begin(), test.end(),
                                                              [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }),
                                               test.end());
                                    if (test != IniKeywords::WWMIObjectDetected && test != IniKeywords::WWMIObjectDetected + "==1") {
                                        realToggle = true;
                                    }
                                }
                                continue;
                            }

                            const auto* content = dynamic_cast<const IfTemplate<std::string, std::string>::ContentPart*>(part.get());
                            if (content == nullptr) {
                                continue;
                            }
                            if (!content->getVals(IniKeywords::MatchFirstIndex).empty()) {
                                meshSection = true;
                            }
                            for (const std::string& val : content->getVals(IniKeywords::This)) {
                                bound.push_back(std::string(StringTools::strip(val)));
                            }
                        }

                        if (meshSection) {
                            continue;
                        }

                        if (bound.size() == 1 && realToggle) {
                            const std::string key = StringTools::toLower(bound.front());
                            if (ownerOf.count(key) == 0) {
                                ownerOf[key] = entry.first;
                                variantsOf[entry.first] = bound;
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
                        const auto resolves = [&templates](const std::vector<std::string>& vals) {
                            std::size_t n = 0;
                            for (const std::string& val : vals) {
                                if (templates.count(val) > 0) {
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
                            ownerOf[key] = entry.first;
                        }
                        variantsOf[entry.first] = bound;
                    }

                    // the texture graph the parser placed each section in
                    std::unordered_map<std::string, ModObj> graphOf;
                    for (const auto& [obj, sections] : this->sectionTargets()) {
                        if (obj.first != IniGraphModObjKeywords::Texture) {
                            continue;
                        }

                        for (const std::string& name : sections) {
                            graphOf.emplace(name, obj);
                        }
                    }

                    for (const auto& [resource, section] : ownerOf) {
                        const auto graph = graphOf.find(section);
                        data.setThisToggle(resource, IniParseData::TextureToggle{section, variantsOf[section], single.count(resource) > 0,
                                                                                 graph == graphOf.end() ? ModObj() : graph->second});
                    }
                }

                // THE SAME TOGGLE WRITTEN AS A REGISTER BINDING: a section binding one register to
                // several resources on its branches (`if $key2 == 1 / ps-t3 = A / else / ps-t3 = B`).
                // Every one reaches that slot, so each is a variant of the role resolved there. Per
                // (section, register): two registers of one section are two roles.
                void readRegisterToggles(IniParseData& data) {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    const std::string prefix = StringTools::toLower(config_.texRegPrefix);
                    for (const auto& entry : ini->getIfTemplates()) {
                        if (entry.second == nullptr || !StringTools::startsWith(entry.first, IniKeywords::TextureOverride)
                                || isFixSection(entry.first)) {
                            continue;
                        }

                        std::map<std::string, std::vector<std::string>> boundOfReg;
                        for (const auto& part : entry.second->parts()) {
                            const auto* content = dynamic_cast<const IfTemplate<std::string, std::string>::ContentPart*>(part.get());
                            if (content == nullptr) {
                                continue;
                            }

                            for (const auto& item : content->items()) {
                                const std::string key = StringTools::toLower(StringTools::strip(item.key));
                                if (!StringTools::startsWith(key, prefix)) {
                                    continue;
                                }

                                const std::string val = IniNamingTools::removeRefPrefix(std::string(StringTools::strip(item.value)));
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
                        }

                        for (const auto& [reg, vals] : boundOfReg) {
                            if (vals.size() < 2) {
                                continue;
                            }
                            for (const std::string& val : vals) {
                                data.setRegisterToggle(StringTools::toLower(val), IniParseData::TextureToggle{entry.first + " " + reg, vals, false});
                            }
                        }
                    }
                }

                // A vb0 hash of this character's, of ANY version: a mod carries whichever version's
                // hashes its author dumped, and the slot test beside it is what keeps this safe -- the
                // section must still carry a match_first_index one of the slots declares.
                bool isOwnVb0(const std::string& hash) const {
                    const auto* hashes = const_cast<IniFileParseContext&>(ctx_).modTypeHashes();
                    const std::string ownName = const_cast<IniFileParseContext&>(ctx_).modTypeName();
                    return hashes != nullptr && !ownName.empty()
                           && hashes->hasFrom(hash, std::nullopt, {std::optional<std::string>(ownName),
                                                                   std::optional<std::string>(config_.slotHashType)});
                }

                // The slot a match_first_index belongs to, in ANY version the library files this
                // character under: a mod is drawn at whichever generation its author dumped, and the
                // slots move between them -- Changli's components 4 to 7 sit 1020 indices later at 1.4
                // than at 1.1, so asking only the parser's own version left them undrawn.
                std::optional<int> slotOfIndex(const std::string& index) const {
                    const auto* indices = const_cast<IniFileParseContext&>(ctx_).modTypeIndices();
                    const std::string ownName = const_cast<IniFileParseContext&>(ctx_).modTypeName();
                    if (indices == nullptr || ownName.empty()) {
                        return std::nullopt;
                    }

                    const std::optional<std::vector<std::string>> key =
                        indices->getKey(std::string(StringTools::strip(index)), std::nullopt,
                                        {std::optional<std::string>(ownName), std::nullopt, std::nullopt}, false);
                    if (!key.has_value() || key->size() < 3 || key->back().rfind(config_.slotPrefix, 0) != 0) {
                        return std::nullopt;
                    }

                    try {
                        return std::stoi(key->back().substr(config_.slotPrefix.size()));
                    } catch (const std::exception&) {
                        return std::nullopt;
                    }
                }

                // slot -> the sections that draw it: a vb0 of this character's and one slot's offset
                std::map<int, std::vector<std::string>> slotSections(const ModBranches::Templates& templates) const {
                    std::map<int, std::vector<std::string>> result;
                    for (const auto& entry : templates) {
                        if (entry.second == nullptr) {
                            continue;
                        }

                        const std::optional<std::string> hash = ModBranches::firstVal(*entry.second, IniKeywords::Hash);
                        const std::optional<std::string> index = ModBranches::firstVal(*entry.second, IniKeywords::MatchFirstIndex);
                        if (!hash.has_value() || !index.has_value() || !isOwnVb0(StringTools::toLower(*hash))) {
                            continue;
                        }

                        if (const std::optional<int> slot = slotOfIndex(*index)) {
                            result[*slot].push_back(entry.first);
                        }
                    }

                    return result;
                }

                // The file a resource section names, resolved as the rest of this parser resolves one --
                // so a file found two ways is one path, and its two candidates one
                static std::string fileOfResource(const ModBranches::Templates& templates, const std::string& resource,
                                                  const std::string& folder) {
                    const auto found = templates.find(resource);
                    if (found == templates.end() || found->second == nullptr) {
                        return "";
                    }

                    const std::optional<std::string> file = ModBranches::firstVal(*found->second, IniKeywords::Filename);
                    if (!file.has_value() || file->empty()) {
                        return "";
                    }

                    return FileService::absPathOfRelPath(FileService::iniPathToRel(*file), folder);
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

                    const ModBranches::Templates& templates = ini->getIfTemplates();
                    const std::string iniPath = ini->getFile().value_or(std::string());
                    const std::map<int, std::vector<std::string>> drawn = slotSections(templates);

                    TextureRoleChooser chooser;
                    chooser.folder = ini->getFolder();
                    chooser.regionRoles = config_.textures.regionRoles;
                    for (const auto& entry : roles.resourcesOf(iniPath)) {
                        chooser.declaredFiles.insert(entry.second);
                    }

                    addOwnCandidates(chooser, roles, templates, iniPath, drawn);
                    if (!drawn.empty() && !iniPath.empty()) {
                        addSiblingRecolours(chooser, iniPath);
                        addSameDrawInis(chooser, iniPath, drawn);
                    }

                    std::vector<ModObj> objs;
                    for (const auto& entry : drawn) {
                        objs.push_back(slotObj(entry.first));
                    }
                    chooser.choose(data, objs);
                }

                // The roles above, and the ones the slots' own sections bind by register. A file the
                // .ini DECLARES but the folder does not hold is no candidate (2026-10-05): six of
                // thirteen Lynae mods name a texture they never shipped, from a dump's stale list, and on
                // the target that binds a missing resource where the role's fallback download is right.
                void addOwnCandidates(TextureRoleChooser& chooser, const TextureRoles& roles,
                                      const ModBranches::Templates& templates, const std::string& iniPath,
                                      const std::map<int, std::vector<std::string>>& drawn) {
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

                        for (const auto& [slot, sections] : drawn) {
                            const auto layout = config_.textures.registerRoles.find(slot);
                            if (layout == config_.textures.registerRoles.end()) {
                                continue;
                            }

                            for (const std::string& section : sections) {
                                const auto tpl = templates.find(section);
                                if (tpl == templates.end() || tpl->second == nullptr) {
                                    continue;
                                }

                                for (const auto& [reg, role] : layout->second) {
                                    const std::optional<std::string> bound = ModBranches::firstValLoose(*tpl->second, reg);
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
                    const auto& byHash = config_.textures.roles;
                    if (byHash.empty()) {
                        return;
                    }

                    std::size_t found = 0;
                    const std::filesystem::path own = FileService::strToPath(iniPath);
                    for (const std::string& path : TextureOverrides::loadedInis(own.parent_path(), own)) {
                        IniFile sibling(path);
                        const std::string folder = sibling.getFolder();
                        const ModBranches::Templates& templates = sibling.getIfTemplates();
                        for (const TextureOverrides::Override& override : TextureOverrides::collect(templates)) {
                            if (!StringTools::startsWith(override.section, IniKeywords::TextureOverride)) {
                                continue;
                            }

                            const auto role = byHash.find(override.hash);
                            if (role == byHash.end()) {
                                continue;   // not one of this character's textures
                            }

                            const std::string file = fileOfResource(templates, override.resource, folder);
                            std::error_code error;
                            if (file.empty() || !std::filesystem::is_regular_file(FileService::strToPath(file), error)) {
                                continue;
                            }

                            chooser.candidates[role->second].emplace_back(
                                file, "a sibling .ini overrides " + override.hash + ", the game texture it replaces");
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
                        for (const std::string& path : TextureOverrides::loadedInis(dir, own)) {
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

                            for (const std::string& path : TextureOverrides::loadedInis(it->path(), own)) {
                                inis.push_back(path);
                            }
                        }
                    }

                    std::size_t found = 0;
                    for (const std::string& path : inis) {
                        IniFile other(path);
                        const ModBranches::Templates& templates = other.getIfTemplates();
                        const std::string folder = other.getFolder();
                        for (const auto& entry : templates) {
                            if (entry.second == nullptr) {
                                continue;
                            }

                            const std::optional<std::string> hash = ModBranches::firstVal(*entry.second, IniKeywords::Hash);
                            const std::optional<std::string> index = ModBranches::firstVal(*entry.second, IniKeywords::MatchFirstIndex);
                            if (!hash.has_value() || !index.has_value()
                                    || !isOwnVb0(StringTools::toLower(std::string(StringTools::strip(*hash))))) {
                                continue;
                            }

                            const std::optional<int> indexSlot = slotOfIndex(*index);
                            for (const int slot : indexSlot.has_value() ? std::vector<int>{*indexSlot} : std::vector<int>{}) {
                                const auto layout = layouts.find(slot);
                                if (drawn.count(slot) == 0 || layout == layouts.end()) {
                                    continue;
                                }

                                for (const auto& [reg, role] : layout->second) {
                                    for (const std::string& bound : ModBranches::looseValsThroughRun(templates, entry.first, reg)) {
                                        const auto declared = std::find_if(templates.begin(), templates.end(), [&](const auto& t) {
                                            return StringTools::equalsIgnoreCase(t.first, IniNamingTools::removeRefPrefix(bound));
                                        });
                                        if (declared == templates.end() || declared->second == nullptr) {
                                            continue;
                                        }

                                        const std::string file = fileOfResource(templates, declared->first, folder);
                                        std::error_code error;
                                        if (file.empty() || !std::filesystem::is_regular_file(FileService::strToPath(file), error)) {
                                            continue;
                                        }

                                        chooser.candidates[role].emplace_back(
                                            file, "the " + reg + " " + FileService::pathToStr(FileService::strToPath(path).filename())
                                                  + " binds on the same draw");
                                        chooser.bindings[file][slotObj(slot)].insert(role);
                                        ++found;
                                    }
                                }
                            }
                        }
                    }

                    if (found > 0) {
                        ctx_.log("another .ini of the mod binds " + std::to_string(found)
                                 + " textures on this file's draws; carried over to the target, where"
                                 + " that .ini's overrides never fire");
                    }
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

                void buildRoles(TextureRoles& out) const {
                    IniFile* ini = const_cast<IniFileParseContext&>(ctx_).getIniFile();
                    if (ini == nullptr) {
                        return;
                    }

                    const auto& templates = ini->getIfTemplates();
                    const std::string folder = ini->getFolder();
                    const std::string iniPath = ini->getFile().value_or(std::string());

                    // NOTE: a section a PREVIOUS fix declared is reported here like any other, and
                    // deliberately. `[Resource<Role><Target>RemapRef]` names one of the MOD'S own
                    // textures, so dropping it here drops the texture: the role then has no file and
                    // falls back to downloading the GAME's, over the mod's own art. What must not
                    // happen is the FIXER reusing that section's NAME, since the undo deletes the
                    // section -- see WWMIFixer's assignRole, which declares a fresh one instead.
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
                                    out.add(file->second, *role, TextureRoles::DeclaredHash + value);
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

            private:
                IniFileParseContext ctx_;
                std::unique_ptr<Classifier> classifier_;
                WWMIParserConfig config_;
                std::optional<Version> version_;
                mutable std::unordered_map<std::string, std::vector<ModObj>> classified_;
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
