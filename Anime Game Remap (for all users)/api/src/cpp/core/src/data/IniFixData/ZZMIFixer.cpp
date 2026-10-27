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

#include "AGRemapCore/data/IniFixData/ZZMIFixer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemove.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEditorEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/VGRemapBlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileDownload.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {
        using Parser = GIMIParser<>;
        using Classifier = Parser::Classifier;
        using Fixer = GIMIFixer<>;
        using ModObj = Fixer::ModObj;
        using GraphId = BaseIniGraphGroupEdit<>::GraphId;
        using Collector = ResRegCollect<>;
        using ObjGroupEdit = GraphGroupEdit<>;

        // NOT 'GroupRemap': GIMIFixer inherits an alias of that name -- see GIMICharFixer
        using ObjGroupRemap = GraphGroupRemap<>;

        // The hash types that name a component's graphs, and the graph each sorts into
        const std::vector<std::pair<std::string, std::string>> HashKinds = {
            {"ib", "ib"}, {"blend_vb", "blend"}, {"texcoord_vb", "texcoord"}, {"draw_vb", "vertexlimit"},
            {"position_vb", "position"}};

        const std::string Vb2 = "vb2";
        const std::string ZZMIResource = "Resource\\ZZMI\\";
        const std::string LightMapRole = "lightmap";
        const std::string DiffuseRole = "diffuse";

        // ---- names ----

        // A component's mod object: the ZZZ component goes into the OBJECT name and the mod object's own
        // component stays "" -- a non-empty component means "the same object drawn more than once" to
        // the graph groups
        ModObj objOf(const std::string& comp, const std::string& name) {
            return {"", comp + "_" + name};
        }

        std::vector<std::string> compObjs(const ZZMIFixerConfig::Component& c) {
            std::vector<std::string> objs;
            for (const ZZMIFixerConfig::Slot& slot : c.slots) {
                objs.push_back(slot.name);
            }
            for (const auto& kind : HashKinds) {
                objs.push_back(kind.second);
            }
            for (const ZZMIFixerConfig::Texture& tex : c.textures) {
                if (std::find(objs.begin(), objs.end(), "tex_" + tex.role) == objs.end()) {
                    objs.push_back("tex_" + tex.role);
                }
            }
            return objs;
        }

        std::string capitalised(const std::string& role) {
            if (role == "lightmap") return "LightMap";
            if (role == "materialmap") return "MaterialMap";
            if (role == "normalmap") return "NormalMap";
            std::string out = role;
            if (!out.empty()) {
                out[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[0])));
            }
            return out;
        }

        // `ref X` -> `X`: a register naming a resource may say `ref`
        std::string removeRef(const std::string& value) {
            return IniNamingTools::removeRefPrefix(value);
        }

        // The library's names append the tag (<name><tag>), which reorders sections of one hash and
        // first index -- and 3DMigoto runs those in NAME order. A mod may split one draw over two
        // (LegsA draws the legs, LegsA2 rebinds vb0 / vb1 to the Body and draws it): "LegsA<tag>"
        // sorts after "LegsA2<tag>" ('2' < 'M'), and LegsA then drew the legs' indices over the
        // Body's vertices. The tag goes after "TextureOverride" instead.
        std::string orderedName(const std::string& name, const std::string& fixed) {
            const std::string prefix = IniKeywords::TextureOverride;
            if (fixed.size() < name.size() || fixed.compare(0, name.size(), name) != 0
                    || name.compare(0, prefix.size(), prefix) != 0) {
                return fixed;
            }
            return prefix + fixed.substr(name.size()) + name.substr(prefix.size());
        }

        BaseResEdit<>::ResEditConfig resEditConfig() {
            return BaseResEdit<>::ResEditConfig{
                IniKeywords::Filename,
                [](const std::string& value) { return value; },
                [](const std::string& file) { return file; }};
        }

        // ---- the mod's own text ----

        struct RawSection {
            std::string name;
            std::string body;
        };

        // The mod's own text: what comes before any fix block of this source
        std::string ownText(const IniFile* ini, const std::string& source) {
            if (ini == nullptr) {
                return {};
            }
            const std::string& txt = ini->getFileTxt();
            const std::size_t at = txt.find("; --------------- " + source + " Remap");
            return at == std::string::npos ? txt : txt.substr(0, at);
        }

        std::vector<RawSection> sectionsOf(const std::string& text) {
            std::vector<RawSection> out;
            std::istringstream stream(text);
            std::string line;
            RawSection* cur = nullptr;
            while (std::getline(stream, line)) {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                const std::string stripped = std::string(StringTools::strip(line));
                if (stripped.size() > 2 && stripped.front() == '[' && stripped.back() == ']') {
                    out.push_back({stripped.substr(1, stripped.size() - 2), ""});
                    cur = &out.back();
                    continue;
                }
                if (cur != nullptr) {
                    cur->body += line + "\n";
                }
            }
            return out;
        }

        // The value of the first `key = value` line of a section body, or nothing
        std::optional<std::string> firstValue(const std::string& body, const std::string& key) {
            std::istringstream stream(body);
            std::string line;
            while (std::getline(stream, line)) {
                const std::size_t eq = line.find('=');
                if (eq == std::string::npos) {
                    continue;
                }
                if (StringTools::equalsIgnoreCase(std::string(StringTools::strip(line.substr(0, eq))), key)) {
                    return std::string(StringTools::strip(line.substr(eq + 1)));
                }
            }
            return std::nullopt;
        }

        std::vector<std::pair<std::string, std::string>> keyValues(const std::string& body) {
            std::vector<std::pair<std::string, std::string>> out;
            std::istringstream stream(body);
            std::string line;
            while (std::getline(stream, line)) {
                const std::size_t eq = line.find('=');
                if (eq == std::string::npos) {
                    continue;
                }
                out.emplace_back(std::string(StringTools::strip(line.substr(0, eq))),
                                 std::string(StringTools::strip(line.substr(eq + 1))));
            }
            return out;
        }

        std::string lowered(std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
            return s;
        }

        // {resource section: (file name, declared stride)} -- a borrowed texcoord is told by its stride
        std::unordered_map<std::string, std::pair<std::string, int>> declaredStrides(const std::string& text) {
            std::unordered_map<std::string, std::pair<std::string, int>> out;
            for (const RawSection& s : sectionsOf(text)) {
                if (s.name.compare(0, 8, IniKeywords::Resource) != 0) {
                    continue;
                }
                const std::optional<std::string> stride = firstValue(s.body, "stride");
                const std::optional<std::string> file = firstValue(s.body, IniKeywords::Filename);
                if (stride.has_value() && file.has_value()) {
                    std::string name = *file;
                    std::replace(name.begin(), name.end(), '\\', '/');
                    name = lowered(name.substr(name.find_last_of('/') == std::string::npos ? 0 : name.find_last_of('/') + 1));
                    out[s.name] = {name, std::atoi(stride->c_str())};
                }
            }
            return out;
        }

        // ---- the texcoord rewrite ----

        // A Texcoord.buf rewritten at another stride: ZZZ's 20-byte texcoord is COLOR, TEXCOORD0 (half2),
        // TEXCOORD1 (float2), TEXCOORD2 (half2); the 24-byte one appends a TEXCOORD3 (half2) that equals
        // TEXCOORD0 on most vertices -- so narrowing drops it and widening copies TEXCOORD0 into it
        bool cutTexcoord(const std::string& src, const std::string& dst, int fromStride, int toStride) {
            std::ifstream in(FileService::strToPath(src), std::ios::binary);
            if (!in || fromStride <= 0 || toStride <= 0) {
                return false;
            }
            std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            if (data.size() % static_cast<std::size_t>(fromStride) != 0) {
                return false;
            }
            std::vector<char> out;
            out.reserve(data.size() / fromStride * toStride);
            for (std::size_t i = 0; i < data.size(); i += fromStride) {
                if (toStride <= fromStride) {
                    out.insert(out.end(), data.begin() + i, data.begin() + i + toStride);
                } else {
                    out.insert(out.end(), data.begin() + i, data.begin() + i + fromStride);
                    for (int extra = fromStride; extra < toStride; extra += 4) {
                        out.insert(out.end(), data.begin() + i + 4, data.begin() + i + 8);
                    }
                }
            }
            FileService::makeFolderFor(dst);
            std::ofstream outFile(FileService::strToPath(dst), std::ios::binary);
            outFile.write(out.data(), static_cast<std::streamsize>(out.size()));
            return static_cast<bool>(outFile);
        }

        // A Texcoord.buf rewritten at the target's stride. The source stride is the one the resource
        // section of the file DECLARES (a texcoord another component's section borrows has its owner's),
        // or 'fromStride' when the file declares none
        class TexcoordCutReplace: public RemapBlendReplace<> {
            public:
                TexcoordCutReplace(GraphId resModObj, int fromStride, int toStride,
                                    std::unordered_map<std::string, int> strideOfFile):
                    RemapBlendReplace<>(std::move(resModObj), resEditConfig(), "texcoord"),
                    fromStride_(fromStride), toStride_(toStride), strideOfFile_(std::move(strideOfFile)) {}

                void buildResModel(const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
                                    const std::string& modName, const std::string& fileKey, Context& ctx) override {
                    (void)resType;
                    (void)modName;
                    int from = fromStride_;
                    std::string file = srcPath;
                    std::replace(file.begin(), file.end(), '\\', '/');
                    const auto found = strideOfFile_.find(lowered(file.substr(file.find_last_of('/') + 1)));
                    if (found != strideOfFile_.end()) {
                        from = found->second;
                    }
                    const int to = toStride_;
                    auto resource = std::make_unique<RemapBlendResource>(
                        ctx.iniFolder(), srcPath, fixedPath, VGRemap{}, "texcoord",
                        std::function<bool(RemapBlendResource&)>([from, to](RemapBlendResource& res) {
                            return cutTexcoord(res.srcPath, res.fixedPath, from, to);
                        }),
                        std::vector<std::unique_ptr<BufElementType>>{});
                    resource->logger = ctx.logger();
                    ctx.storeResource(fileKey, std::move(resource));
                }

            private:
                int fromStride_;
                int toStride_;
                std::unordered_map<std::string, int> strideOfFile_;
        };

        // ---- the band edit ----

        // A light map's RED channel is a material band: each texel ON a band (within the tolerance) moves
        // per the component's band moves; a mod's in-between reds (a gradient, anti-aliasing, a level of
        // its own) keep their value. Every decision from the ORIGINAL red: the moves may be a permutation
        TexEditor::Filter bandEdit(const ZZMIFixerConfig& config, const std::map<int, int>& moves) {
            const std::array<int, 5> levels = config.bandLevels;
            const int tolerance = config.bandTolerance;
            return [levels, tolerance, moves](TextureFile& texFile) {
                std::vector<std::uint8_t> pixels = texFile.getPixels();
                for (std::size_t i = 0; i + 3 < pixels.size(); i += 4) {
                    const int red = pixels[i];
                    int best = 0;
                    for (std::size_t k = 1; k < levels.size(); ++k) {
                        if (std::abs(red - levels[k]) < std::abs(red - levels[best])) {
                            best = static_cast<int>(k);
                        }
                    }
                    if (std::abs(red - levels[best]) > tolerance) {
                        continue;
                    }
                    const auto move = moves.find(levels[best]);
                    if (move != moves.end() && move->second != levels[best]) {
                        pixels[i] = static_cast<std::uint8_t>(move->second);
                    }
                }
                texFile.setPixels(std::move(pixels), texFile.getWidth(), texFile.getHeight());
            };
        }

        // ---- the draw ranges two components' sections draw (a doubled mesh) ----

        struct DrawRange {
            bool all = true;
            long long start = 0;
            long long end = 0;
        };

        DrawRange drawRange(const std::string& value) {
            std::vector<std::string> parts;
            std::stringstream ss(value);
            std::string part;
            while (std::getline(ss, part, ',')) {
                parts.push_back(std::string(StringTools::strip(part)));
            }
            if (parts.size() < 2 || StringTools::equalsIgnoreCase(parts[0], "auto")) {
                return {};
            }
            const long long count = std::atoll(parts[0].c_str());
            const long long start = std::atoll(parts[1].c_str());
            return {false, start, start + count};
        }

        bool overlaps(const DrawRange& r, const std::vector<DrawRange>& ranges) {
            for (const DrawRange& o : ranges) {
                if (o.all || r.all || (r.start < o.end && o.start < r.end)) {
                    return true;
                }
            }
            return false;
        }

        // {ib resource: ranges} the mod's own sections on 'ibHash' draw, following each section's `ib =`
        // lines in order; a section with no `ib =` of its own draws whatever ib the hash's other sections
        // bound, credited to it when there is exactly one
        std::unordered_map<std::string, std::vector<DrawRange>> rangesDrawnBy(const std::string& text, const std::string& ibHash) {
            std::vector<const RawSection*> sections;
            const std::vector<RawSection> all = sectionsOf(text);
            std::set<std::string> bound;
            for (const RawSection& s : all) {
                const std::optional<std::string> hash = firstValue(s.body, IniKeywords::Hash);
                if (!hash.has_value() || !StringTools::equalsIgnoreCase(*hash, ibHash)) {
                    continue;
                }
                sections.push_back(&s);
                for (const auto& kv : keyValues(s.body)) {
                    if (StringTools::equalsIgnoreCase(kv.first, "ib") && !StringTools::equalsIgnoreCase(removeRef(kv.second), "null")) {
                        bound.insert(removeRef(kv.second));
                    }
                }
            }
            const std::optional<std::string> inherited = bound.size() == 1 ? std::optional<std::string>(*bound.begin()) : std::nullopt;
            std::unordered_map<std::string, std::vector<DrawRange>> out;
            for (const RawSection* s : sections) {
                std::optional<std::string> cur = inherited;
                for (const auto& kv : keyValues(s->body)) {
                    if (StringTools::equalsIgnoreCase(kv.first, "ib")) {
                        cur = removeRef(kv.second);
                    } else if (StringTools::equalsIgnoreCase(kv.first, IniKeywords::DrawIndexed) && cur.has_value()
                               && !StringTools::equalsIgnoreCase(*cur, "null")) {
                        out[*cur].push_back(drawRange(kv.second));
                    }
                }
            }
            return out;
        }

        std::string newline(const std::string& text) {
            return text.find("\r\n") != std::string::npos ? "\r\n" : "\n";
        }

        bool startsWithWord(const std::string& stripped, const char* word) {
            const std::size_t n = std::strlen(word);
            return stripped.size() >= n && StringTools::equalsIgnoreCase(stripped.substr(0, n), word)
                && (stripped.size() == n || !std::isalnum(static_cast<unsigned char>(stripped[n])));
        }

        bool isStructure(const std::string& stripped) {
            return startsWithWord(stripped, "if") || startsWithWord(stripped, "else") || startsWithWord(stripped, "elif")
                || startsWithWord(stripped, "endif");
        }

        // `key = value` of one stripped line, or nothing
        std::optional<std::pair<std::string, std::string>> kvOf(const std::string& stripped) {
            const std::size_t eq = stripped.find('=');
            if (eq == std::string::npos) {
                return std::nullopt;
            }
            return std::make_pair(std::string(StringTools::strip(stripped.substr(0, eq))),
                                  std::string(StringTools::strip(stripped.substr(eq + 1))));
        }

        bool isTextureLine(const std::string& key) {
            return (key.size() > ZZMIResource.size() && StringTools::equalsIgnoreCase(key.substr(0, ZZMIResource.size()), ZZMIResource))
                || (key.size() > 4 && StringTools::equalsIgnoreCase(key.substr(0, 4), "ps-t"));
        }

        bool isSetTextures(const std::pair<std::string, std::string>& kv) {
            return StringTools::equalsIgnoreCase(kv.first, IniKeywords::Run)
                && StringTools::equalsIgnoreCase(kv.second, "CommandList\\ZZMI\\SetTextures");
        }


        /**
         * The parser of a ZZMI character's mods: one classifier per component, filtered to that
         * component's own hash rows (every component carries the same hash TYPES, so a type alone
         * cannot say which one a section is), and a drawn object resolved from the component's slots
         */
        class ZZMIGIMIParser: public Parser {
            public:
                ZZMIGIMIParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs,
                                const ZZMIFixerConfig& config):
                    Parser(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId), config_(config) {
                    this->setCtx(&ctx_);
                    this->setIniFile(iniFile);
                    this->disjointModObjs = false;
                    this->trackKeys = true;
                    this->keysToTrack = std::unordered_set<std::string>{IniKeywords::Hash, IniKeywords::MatchFirstIndex};

                    for (const ZZMIFixerConfig::Component& c : config_.components) {
                        std::unordered_map<std::string, ModObj> hashOnly;
                        for (const auto& kind : HashKinds) {
                            hashOnly[kind.first] = objOf(c.name, kind.second);
                        }
                        for (const ZZMIFixerConfig::Texture& tex : c.textures) {
                            hashOnly[tex.hashType] = objOf(c.name, "tex_" + tex.role);
                        }

                        auto classifier = std::make_unique<Classifier>(
                            std::move(hashOnly), ctx_.modTypeHashes(),
                            std::unordered_map<std::string, Classifier::IndexModObjs>{}, ctx_.modTypeIndices(),
                            ctx_.version());
                        classifier->setHashNonVersionVals({config_.source + c.name, std::nullopt});
                        classifiers_.push_back(std::move(classifier));
                    }

                    const ZZMIFixerConfig* cfg = &config_;
                    std::vector<Classifier*> classifiers;
                    for (const auto& classifier : classifiers_) {
                        classifiers.push_back(classifier.get());
                    }

                    this->objTargetFuncs.emplace_back(
                        [cfg, classifiers](Parser&, const std::string& sectionName, Section* section, bool,
                                            ContentPart*, const Colouring* kvps) {
                            std::vector<ModObj> result;
                            if (kvps == nullptr) {
                                return result;
                            }

                            for (std::size_t i = 0; i < classifiers.size() && i < cfg->components.size(); ++i) {
                                std::vector<ModObj> found = classifiers[i]->classify(sectionName, section, *kvps);
                                if (found.empty()) {
                                    continue;
                                }

                                // A drawn object: the component's ib hash plus a match_first_index it knows
                                const ZZMIFixerConfig::Component& c = cfg->components[i];
                                std::vector<std::string> indexVals = kvps->getVals(IniKeywords::MatchFirstIndex);
                                for (ModObj& modObj : found) {
                                    if (modObj != objOf(c.name, "ib") || indexVals.empty()) {
                                        continue;
                                    }
                                    for (const ZZMIFixerConfig::Slot& slot : c.slots) {
                                        if (slot.srcIndex == std::string(StringTools::strip(indexVals.front()))) {
                                            modObj = objOf(c.name, slot.name);
                                            break;
                                        }
                                    }
                                }
                                result.insert(result.end(), found.begin(), found.end());
                            }
                            return result;
                        });
                }

            private:
                IniFileParseContext ctx_;
                ZZMIFixerConfig config_;
                std::vector<std::unique_ptr<Classifier>> classifiers_;
        };


        /**
         * The fixer of one source component onto its target component -- see ZZMIFixerConfig
         */
        class ZZMIFixerImpl: public Fixer {
            public:
                ZZMIFixerImpl(BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId,
                               ZZMIFixerConfig config, const std::string& component):
                    Fixer(parser, nullptr, {},
                           toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                             : std::optional<std::vector<std::string>>({toModName}),
                           nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName),
                    config_(std::move(config)) {
                    this->setCtx(&ctx_);

                    for (const ZZMIFixerConfig::Component& c : config_.components) {
                        if (c.name == component) {
                            comp_ = c;
                        }
                    }

                    own_ = ownText(ctx_.getIniFile(), config_.source);
                    buildRemap();
                    buildBlendCollect();
                    buildTexcoordCollects();
                    buildBandEdits();
                    buildObjEdits();
                    buildFallbacks();

                    this->graphGroupEdits.clear();
                    this->graphGroupEdits.push_back(remap_.get());
                    for (auto& collect : collects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }
                    for (auto& edit : groupEdits_) {
                        this->graphGroupEdits.push_back(edit.get());
                    }

                    this->copyPreamble = config_.copyPreamble;
                }

            protected:
                // GIMIFixer's own hands every group edit a nullptr .ini file, which stops a collect ever
                // building anything -- see GIMICharFixerImpl
                void applyGraphGroupEdits(const std::string& modName) override {
                    if (this->graphGroups() == nullptr) {
                        return;
                    }
                    for (Fixer::GroupEdit* edit : this->graphGroupEdits) {
                        if (edit != nullptr) {
                            edit->editFromIni(*this->graphGroups(), ctx_.getIniFile(), nullptr, modName);
                        }
                    }
                }

                // The two passes over this fixer's own rendered block: the recolour defaults and the
                // cross-slot draws. Text, because each moves lines between sections of TWO fixers' copies
                std::string groupToStr(std::size_t groupInd) const override {
                    std::string content = Fixer::groupToStr(groupInd);
                    if (groupInd != 0 || content.empty()) {
                        return content;
                    }
                    content = injectDefaults(content);
                    content = zeroCrossSlotDraws(content);
                    content = insertCrossSlotDraws(content);
                    return content;
                }

            private:
                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                std::string srcName() const { return config_.source + comp_.name; }
                std::string targetName() const { return config_.target + comp_.target; }

                std::optional<Version> toVersion() const {
                    IniFile* ini = ctx_.getIniFile();
                    return ini == nullptr ? std::nullopt : ini->toVersion;
                }

                std::optional<std::string> hashOf(const std::string& name, const std::string& type) const {
                    Hashes* hashes = ctx_.modTypeHashes();
                    if (hashes == nullptr) {
                        return std::nullopt;
                    }
                    return hashes->get({name, type}, ctx_.version(), false);
                }

                // ---- 1. this pair's graphs only: every other component's are some other fixer's ----
                void buildRemap() {
                    ObjGroupRemap::RemapList remap;
                    for (const ZZMIFixerConfig::Component& other : config_.components) {
                        for (const std::string& obj : compObjs(other)) {
                            std::vector<ObjGroupRemap::RemapTarget> targets;
                            if (other.name == comp_.name) {
                                targets.emplace_back(GraphId(0, "", other.name + "_" + obj),
                                                     [](const std::string& name) { return name; });
                            }
                            remap.emplace_back(GraphId(0, "", other.name + "_" + obj), std::move(targets));
                        }
                    }
                    remap_ = std::make_unique<ObjGroupRemap>(std::move(remap));
                }

                // ---- 2. the blend, wherever the component's sections bind it ----
                //
                // A mod may run its pre-skin pass (`vb2 = ...` + `draw = <n>,0`) from the BLEND section or
                // from the POSITION section; a blend left unremapped there skins with the source's bone
                // indices on the target's palette, which shatters the model
                void buildBlendCollect() {
                    auto replace = std::make_unique<VGRemapBlendReplace<>>(
                        GraphId(0, "", comp_.name + "_RemapBlend"), resEditConfig(), ctx_.modType(), ctx_.version(),
                        toVersion(), "blend", std::nullopt, comp_.name, comp_.target);
                    replace->rowModName = config_.target;

                    auto collect = std::make_unique<Collector>();
                    collect->srcRegs = {{GraphId(0, "", comp_.name + "_blend"), Vb2},
                                        {GraphId(0, "", comp_.name + "_position"), Vb2}};
                    collect->resEdits = {{IniKeywords::Blend, replace.get()}};
                    resEdits_.push_back(std::move(replace));
                    collects_.push_back(std::move(collect));
                }

                // ---- 3. the texcoord: its own where the strides differ, and one an ib section BORROWS ----
                void buildTexcoordCollects() {
                    const int fromStride = comp_.srcTexcoordStride;
                    const int toStride = comp_.targetTexcoordStride;
                    std::unordered_map<std::string, int> strideOfFile;
                    const auto declared = declaredStrides(own_);
                    for (const auto& entry : declared) {
                        strideOfFile[entry.second.first] = entry.second.second;
                    }

                    if (fromStride != toStride) {
                        addTexcoordCollect({GraphId(0, "", comp_.name + "_texcoord")}, "RemapTexcoord", fromStride, toStride,
                                           strideOfFile, false);
                    }

                    // A texcoord an ib section borrows from another component (the Body's, drawn from the
                    // Legs section), or that the Blend / Position section binds for the draw passes: cut
                    // when its DECLARED stride is not the target's
                    std::vector<GraphId> holders;
                    for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                        holders.emplace_back(0, "", comp_.name + "_" + slot.name);
                    }
                    holders.emplace_back(0, "", comp_.name + "_blend");
                    holders.emplace_back(0, "", comp_.name + "_position");
                    addTexcoordCollect(holders, "RemapBorrowedTexcoord", fromStride, toStride, strideOfFile, true);
                }

                void addTexcoordCollect(const std::vector<GraphId>& graphs, const std::string& resName, int fromStride,
                                         int toStride, std::unordered_map<std::string, int> strideOfFile, bool onlyOtherStrides) {
                    const GraphId resGraph(0, "", comp_.name + "_" + resName);
                    auto replace = std::make_unique<TexcoordCutReplace>(resGraph, fromStride, toStride, strideOfFile);
                    auto collect = std::make_unique<Collector>();
                    for (const GraphId& graph : graphs) {
                        collect->srcRegs[graph] = IniKeywords::Vb1;
                    }
                    collect->resEdits = {{"texcoord", replace.get()}};

                    if (onlyOtherStrides) {
                        const auto declared = declaredStrides(own_);
                        for (const GraphId& graph : graphs) {
                            collect->resPredicates[graph] = [declared, toStride](const std::string&, const std::string& val,
                                                                                const Collector::IterData&) {
                                const auto found = declared.find(removeRef(val));
                                return found != declared.end() && found->second.second != toStride;
                            };
                        }
                    }

                    // ...and the section the collect writes still DECLARES the source's stride
                    auto stride = std::make_unique<RegNewVals<>>(
                        std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{{"stride", RegNewVals<>::NewVal(std::to_string(toStride))}});
                    auto adapter = std::make_unique<RegPartEdit<>>(stride.get());
                    ObjGroupEdit::IniEdits iniEdits;
                    iniEdits.edits[resGraph.modObj] = {adapter.get()};
                    auto strideEdit = std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{iniEdits});

                    resEdits_.push_back(std::move(replace));
                    collects_.push_back(std::move(collect));
                    regEdits_.push_back(std::move(stride));
                    partEdits_.push_back(std::move(adapter));
                    groupEdits_.push_back(std::move(strideEdit));
                }

                // ---- 3b. THE BAND EDIT on every light map the component binds ----
                void buildBandEdits() {
                    if (comp_.bandMoves.empty()) {
                        return;
                    }

                    const std::string zzmiLightMap = ZZMIResource + "LightMap";
                    struct Ref { std::string graph; std::string reg; std::string res; };
                    std::vector<Ref> refs = {{comp_.name + "_tex_" + LightMapRole, IniKeywords::This, "RemapTexLightMap"}};
                    for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                        refs.push_back({comp_.name + "_" + slot.name, zzmiLightMap, slot.name + "RemapTexLightMap"});
                        refs.push_back({comp_.name + "_" + slot.name, config_.roleRegs.at(LightMapRole), slot.name + "RemapTexLightMapT"});
                    }

                    // The `ref` comes off for the collect, which looks the value up as a section name, and goes
                    // back on after it (without it 3DMigoto COPIES the resource rather than referencing it)
                    auto stripRef = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                        {zzmiLightMap, RegNewVals<>::NewVal(RegNewVals<>::OldValProducer(
                            [](const std::string& old, const ModType*) { return removeRef(old); }))}});
                    auto addRef = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                        {zzmiLightMap, RegNewVals<>::NewVal(RegNewVals<>::OldValProducer(
                            [](const std::string& old, const ModType*) {
                                const std::string bare = removeRef(old);
                                return StringTools::equalsIgnoreCase(bare, "null") ? old : "ref " + bare;
                            }))}});
                    auto stripAdapter = std::make_unique<RegPartEdit<>>(stripRef.get());
                    auto addAdapter = std::make_unique<RegPartEdit<>>(addRef.get());

                    ObjGroupEdit::IniEdits strips, adds;
                    for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                        strips.edits[objOf(comp_.name, slot.name)] = {stripAdapter.get()};
                        adds.edits[objOf(comp_.name, slot.name)] = {addAdapter.get()};
                    }
                    groupEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{strips}));

                    // a `ps-t5 = ref <resource VARIABLE>` (one command list every component shares) names no
                    // file: left unedited
                    const Collector::ResPredicate direct = [](const std::string&, const std::string& val, const Collector::IterData&) {
                        const std::string s = lowered(std::string(StringTools::strip(val)));
                        return s.compare(0, 4, "ref ") != 0;
                    };

                    for (const Ref& ref : refs) {
                        const GraphId srcGraph(0, "", ref.graph);
                        auto replace = std::make_unique<TexEditorReplace<>>(
                            GraphId(0, "", comp_.name + "_" + ref.res),
                            TexEditor({bandEdit(config_, comp_.bandMoves)}, false, true), resEditConfig(),
                            "resourceRemapTexEdit", comp_.target + "LightMap");
                        auto collect = std::make_unique<Collector>();
                        collect->srcRegs = {{srcGraph, ref.reg}};
                        collect->resEdits = {{"lightMap", replace.get()}};
                        collect->resPredicates = {{srcGraph, direct}};
                        resEdits_.push_back(std::move(replace));
                        groupCollectsAfterStrip_.push_back(std::move(collect));
                    }
                    for (auto& collect : groupCollectsAfterStrip_) {
                        collectsBox_.push_back(std::move(collect));
                    }
                    groupCollectsAfterStrip_.clear();

                    bandEdits_ = std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{adds});
                    regEdits_.push_back(std::move(stripRef));
                    regEdits_.push_back(std::move(addRef));
                    partEdits_.push_back(std::move(stripAdapter));
                    partEdits_.push_back(std::move(addAdapter));
                }

                // ---- 4. names, hashes, slot indices, strides, and a doubled mesh ----
                void buildObjEdits() {
                    const std::string toModName = toModName_;
                    renames_.push_back(std::make_unique<GraphRename<>>([toModName](const std::string& n) {
                        return orderedName(n, IniNamingTools::getRemapFixName(n, toModName)); }));
                    renames_.push_back(std::make_unique<GraphRename<>>([toModName](const std::string& n) {
                        return orderedName(n, IniNamingTools::getRemapIbName(n, toModName)); }));
                    renames_.push_back(std::make_unique<GraphRename<>>([toModName](const std::string& n) {
                        return orderedName(n, IniNamingTools::getRemapBlendName(n, toModName)); }));
                    for (auto& rename : renames_) {
                        partEdits_.push_back(std::make_unique<GraphPartEdit<>>(rename.get()));
                    }
                    GroupPartEdit* rename = partEdits_[partEdits_.size() - 3].get();
                    GroupPartEdit* renameIb = partEdits_[partEdits_.size() - 2].get();
                    GroupPartEdit* renameBlend = partEdits_[partEdits_.size() - 1].get();

                    auto hashRemap = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)}},
                        toModName_, srcName(), ctx_.version(), toVersion());
                    partEdits_.push_back(std::make_unique<RegPartEdit<>>(hashRemap.get()));
                    GroupPartEdit* hash = partEdits_.back().get();
                    regEdits_.push_back(std::move(hashRemap));

                    // a VertexLimitRaise's override_byte_stride is the SOURCE component's draw stride
                    // (Position 40 + Texcoord + Blend 32), and the target's is its own
                    auto drawStride = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                        {"override_byte_stride", RegNewVals<>::NewVal(std::to_string(40 + comp_.targetTexcoordStride + 32))}});
                    partEdits_.push_back(std::make_unique<RegPartEdit<>>(drawStride.get()));
                    GroupPartEdit* stride = partEdits_.back().get();
                    regEdits_.push_back(std::move(drawStride));

                    ObjGroupEdit::IniEdits edits;
                    edits.edits[objOf(comp_.name, "ib")] = {renameIb, hash};
                    edits.edits[objOf(comp_.name, "blend")] = {renameBlend, hash};
                    edits.edits[objOf(comp_.name, "texcoord")] = {rename, hash};
                    edits.edits[objOf(comp_.name, "vertexlimit")] = {rename, hash, stride};
                    edits.edits[objOf(comp_.name, "position")] = {rename, hash};

                    // A mesh two components' sections BOTH draw shows whichever slot the game draws LATER,
                    // at equal depth -- and the two characters order their slots differently. So a draw
                    // whose index RANGE another component's sections also draw from the same ib is dropped
                    std::vector<DrawRange> shared;
                    for (const auto& pair : config_.crossSlotDraws) {
                        if (pair.first != comp_.name) {
                            continue;
                        }
                        const std::optional<std::string> mineHash = hashOf(srcName(), "ib");
                        const std::optional<std::string> laterHash = hashOf(config_.source + pair.second, "ib");
                        if (!mineHash.has_value() || !laterHash.has_value()) {
                            continue;
                        }
                        const auto mine = rangesDrawnBy(own_, *mineHash);
                        const auto later = rangesDrawnBy(own_, *laterHash);
                        for (const auto& entry : mine) {
                            const auto found = later.find(entry.first);
                            if (found != later.end()) {
                                shared.insert(shared.end(), found->second.begin(), found->second.end());
                            }
                        }
                    }
                    GroupPartEdit* noDouble = nullptr;
                    if (!shared.empty()) {
                        auto drop = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                            {IniKeywords::DrawIndexed, RegNewVals<>::NewVal(RegNewVals<>::OldValProducer(
                                [shared](const std::string& old, const ModType*) {
                                    return overlaps(drawRange(old), shared) ? std::string("0, 0, 0") : old;
                                }))}});
                        partEdits_.push_back(std::make_unique<RegPartEdit<>>(drop.get()));
                        noDouble = partEdits_.back().get();
                        regEdits_.push_back(std::move(drop));
                        edits.edits[objOf(comp_.name, "ib")].push_back(noDouble);
                    }

                    for (const ZZMIFixerConfig::Slot& slot : comp_.slots) {
                        auto vals = std::make_unique<RegNewVals<>>(std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                            {IniKeywords::MatchFirstIndex, RegNewVals<>::NewVal(slot.targetIndex)},
                            {"match_index_count", RegNewVals<>::NewVal(std::to_string(slot.targetIndexCount))}});
                        partEdits_.push_back(std::make_unique<RegPartEdit<>>(vals.get()));
                        std::vector<GroupPartEdit*> list = {rename, hash, partEdits_.back().get()};
                        if (noDouble != nullptr) {
                            list.push_back(noDouble);
                        }
                        edits.edits[objOf(comp_.name, slot.name)] = list;
                        regEdits_.push_back(std::move(vals));
                    }
                    for (const ZZMIFixerConfig::Texture& tex : comp_.textures) {
                        edits.edits[objOf(comp_.name, "tex_" + tex.role)] = {rename, hash};
                    }

                    // the band edit's collects, then the `ref` going back on, then the names
                    for (auto& collect : collectsBox_) {
                        groupEdits_.push_back(std::move(collect));
                    }
                    collectsBox_.clear();
                    if (bandEdits_ != nullptr) {
                        groupEdits_.push_back(std::move(bandEdits_));
                    }
                    groupEdits_.push_back(std::make_unique<ObjGroupEdit>(std::vector<ObjGroupEdit::IniEdits>{edits}));
                }

                // ---- the fallbacks: the source's game texture of a role the mod recolours by no hash ----
                std::string fallbackName(const std::string& role) const {
                    std::string stem;
                    const std::optional<std::string>& file = ctx_.getIniFile()->getFile();
                    if (file.has_value()) {
                        const std::filesystem::path path = FileService::strToPath(*file);
                        for (char ch : FileService::pathToStr(path.stem())) {
                            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
                                stem += ch;
                            }
                        }
                    }
                    return toModName_ + IniKeywords::RemapDL + capitalised(role) + stem;
                }

                bool drawsComponent() const {
                    const std::optional<std::string> ib = hashOf(srcName(), "ib");
                    if (!ib.has_value()) {
                        return false;
                    }
                    for (const RawSection& s : sectionsOf(own_)) {
                        const std::optional<std::string> hash = firstValue(s.body, IniKeywords::Hash);
                        if (hash.has_value() && StringTools::equalsIgnoreCase(*hash, *ib)) {
                            return true;
                        }
                    }
                    return false;
                }

                std::set<std::string> recolouredHashes() const {
                    std::set<std::string> found;
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr) {
                        return found;
                    }
                    // the folder of the .ini and below, as readAllInis reads them
                    std::error_code ec;
                    for (const auto& entry : std::filesystem::recursive_directory_iterator(FileService::strToPath(ini->getFolder()), ec)) {
                        const std::string name = FileService::pathToStr(entry.path().filename());
                        if (!entry.is_regular_file() || !StringTools::endsWithIgnoreCase(name, ".ini")
                                || StringTools::startsWith(lowered(name), "disabled")) {
                            continue;
                        }
                        std::ifstream in(entry.path(), std::ios::binary);
                        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                        const std::size_t at = text.find("; --------------- " + config_.source + " Remap");
                        for (const RawSection& s : sectionsOf(at == std::string::npos ? text : text.substr(0, at))) {
                            const std::optional<std::string> hash = firstValue(s.body, IniKeywords::Hash);
                            if (hash.has_value() && s.name.find(IniKeywords::Remap) == std::string::npos
                                    && firstValue(s.body, IniKeywords::This).has_value()) {
                                found.insert(lowered(*hash));
                            }
                        }
                    }
                    return found;
                }

                void buildFallbacks() {
                    IniFile* ini = ctx_.getIniFile();
                    if (ini == nullptr || comp_.textures.empty() || !drawsComponent()) {
                        return;
                    }

                    const std::set<std::string> recoloured = recolouredHashes();
                    std::string text;
                    for (const ZZMIFixerConfig::Texture& tex : comp_.textures) {
                        if (!tex.fallback) {
                            continue;
                        }
                        const std::optional<std::string> srcHash = hashOf(srcName(), tex.hashType);
                        const std::optional<std::string> dstHash = hashOf(targetName(), tex.hashType);
                        if (!srcHash.has_value() || !dstHash.has_value()) {
                            continue;
                        }

                        const std::string name = fallbackName(tex.role);
                        const bool banded = tex.role == LightMapRole && !comp_.bandMoves.empty();
                        const std::string fileName = name + (banded ? "Raw" : "") + FileExt::DDS;
                        const std::string url = DownloadTools::downloadFolder() + "/"
                            + DownloadTools::urlPath(config_.downloadGameFolder, config_.downloadCharFolder,
                                                     config_.downloadVersionFolder, srcName(), "A" + capitalised(tex.role), FileExt::DDS);
                        const std::string path = FileService::absPathOfRelPath(fileName, ini->getFolder());
                        ini->getFileDownloads().push_back(std::make_unique<RemapIniDownload>(
                            ini->getFolder(), fileName, std::make_unique<FileDownload>(url, fileName)));

                        if (banded) {
                            // the band edit over the download, which is fetched first (downloads before resources)
                            ini->getResources().push_back(std::make_unique<RemapTexEditResource>(
                                ini->getFolder(), path, FileService::absPathOfRelPath(name + FileExt::DDS, ini->getFolder()),
                                TexEditor({bandEdit(config_, comp_.bandMoves)}, false, true)));
                            text += "[" + IniKeywords::Resource + name + "Raw]\nfilename = " + name + "Raw" + FileExt::DDS + "\n\n";
                        }

                        text += "[" + IniKeywords::Resource + name + "]\nfilename = " + name + FileExt::DDS + "\n\n";
                        if (recoloured.count(lowered(*srcHash)) > 0) {
                            // the mod recolours it, maybe only under a toggle: its carried copy gets this as its
                            // DEFAULT, so a toggle that is off does not leave the target's texture at the source's UVs
                            defaults_[lowered(*dstHash)] = IniKeywords::Resource + name;
                            continue;
                        }
                        text += "[" + IniKeywords::TextureOverride + name + "]\nhash = " + *dstHash + "\nthis = "
                            + IniKeywords::Resource + name + "\n\n";
                    }
                    this->appendedSections = text;
                }

                // ---- the text passes over this fixer's block ----

                // Each carried recolour copy first binds the fallback, so the mod's own `this` lines -- under a
                // toggle or not -- override a default
                std::string injectDefaults(const std::string& content) const {
                    if (defaults_.empty()) {
                        return content;
                    }
                    const std::string nl = newline(content);
                    std::istringstream stream(content);
                    std::string line, out, section;
                    bool pending = false;
                    std::string pendingRes;
                    while (std::getline(stream, line)) {
                        if (!line.empty() && line.back() == '\r') {
                            line.pop_back();
                        }
                        const std::string stripped = std::string(StringTools::strip(line));
                        if (stripped.size() > 2 && stripped.front() == '[' && stripped.back() == ']') {
                            section = stripped;
                            pending = false;
                        }
                        out += line + nl;
                        const auto kv = kvOf(stripped);
                        if (kv.has_value() && StringTools::equalsIgnoreCase(kv->first, IniKeywords::Hash)
                                && section.find(IniKeywords::RemapFix) != std::string::npos) {
                            const auto found = defaults_.find(lowered(kv->second));
                            if (found != defaults_.end()) {
                                out += IniKeywords::This + " = " + found->second + nl;
                            }
                        }
                    }
                    (void)pending;
                    (void)pendingRes;
                    return out;
                }

                // The buffers a section of 'comp' pre-skins into (`ResourceX = copy so0` / `= so0`)
                std::set<std::string> preSkinBuffers(const std::string& comp) const {
                    std::set<std::string> written;
                    std::set<std::string> hashes;
                    for (const char* type : {"blend_vb", "position_vb", "ib"}) {
                        const std::optional<std::string> h = hashOf(config_.source + comp, type);
                        if (h.has_value()) {
                            hashes.insert(lowered(*h));
                        }
                    }
                    for (const RawSection& s : sectionsOf(own_)) {
                        const std::optional<std::string> hash = firstValue(s.body, IniKeywords::Hash);
                        if (!hash.has_value() || hashes.count(lowered(*hash)) == 0) {
                            continue;
                        }
                        for (const auto& kv : keyValues(s.body)) {
                            const std::string v = lowered(kv.second);
                            if (v == "so0" || v == "copy so0") {
                                written.insert(kv.first);
                            }
                        }
                    }
                    return written;
                }

                // A drawn section's lines: the cross draws (made while vb0 is a pre-skin buffer of the other
                // component) zeroed, or only the lines that move -- structure, ib, texture lines, the
                // pre-skin vertex buffers and the cross draws, in place
                static std::string crossLines(const std::string& body, const std::set<std::string>& written, bool zero,
                                              bool& hasCross, const std::string& nl) {
                    std::istringstream stream(body);
                    std::string line, out;
                    std::optional<std::string> ib, vb0;
                    while (std::getline(stream, line)) {
                        if (!line.empty() && line.back() == '\r') {
                            line.pop_back();
                        }
                        const std::string stripped = std::string(StringTools::strip(line));
                        const auto kv = kvOf(stripped);
                        bool cross = false;
                        bool keep = isStructure(stripped);
                        if (kv.has_value()) {
                            if (StringTools::equalsIgnoreCase(kv->first, "ib")) {
                                ib = removeRef(kv->second);
                                keep = true;
                            } else if (StringTools::equalsIgnoreCase(kv->first, IniKeywords::Vb0)) {
                                vb0 = removeRef(kv->second);
                            }
                            const bool preSkin = (StringTools::equalsIgnoreCase(kv->first, IniKeywords::Vb0)
                                                  || StringTools::equalsIgnoreCase(kv->first, "vb3"))
                                && written.count(removeRef(kv->second)) > 0;
                            cross = StringTools::equalsIgnoreCase(kv->first, IniKeywords::DrawIndexed) && vb0.has_value()
                                && written.count(*vb0) > 0 && ib.has_value();
                            keep = keep || cross || preSkin || isTextureLine(kv->first) || isSetTextures(*kv);
                        }
                        hasCross = hasCross || cross;
                        if (zero) {
                            const std::size_t indent = line.find_first_not_of(" \t");
                            out += (cross ? line.substr(0, indent == std::string::npos ? 0 : indent) + IniKeywords::DrawIndexed + " = 0, 0, 0" : line) + nl;
                        } else if (keep) {
                            out += line + nl;
                        }
                    }
                    return out;
                }

                // In the fixer of a pair's SECOND component: zero the draws its copies make over the first's mesh
                std::string zeroCrossSlotDraws(const std::string& content) const {
                    for (const auto& pair : config_.crossSlotDraws) {
                        if (pair.second != comp_.name) {
                            continue;
                        }
                        const std::set<std::string> written = preSkinBuffers(pair.first);
                        if (written.empty()) {
                            continue;
                        }
                        const std::string nl = newline(content);
                        std::string out;
                        for (const RawSection& s : sectionsOf(content)) {
                            bool hasCross = false;
                            const bool drawn = firstValue(s.body, IniKeywords::MatchFirstIndex).has_value();
                            out += "[" + s.name + "]" + nl + (drawn ? crossLines(s.body, written, true, hasCross, nl) : s.body);
                        }
                        // keep what precedes the first section (comments)
                        const std::size_t first = content.find('[');
                        return (first == std::string::npos ? std::string() : content.substr(0, first)) + out;
                    }
                    return content;
                }

                // In the fixer of a pair's FIRST component: the moved draws, at the end of its object section
                std::string insertCrossSlotDraws(const std::string& content) const {
                    for (const auto& pair : config_.crossSlotDraws) {
                        if (pair.first != comp_.name) {
                            continue;
                        }
                        const std::set<std::string> written = preSkinBuffers(pair.first);
                        const std::optional<std::string> legsIb = hashOf(config_.source + pair.second, "ib");
                        if (written.empty() || !legsIb.has_value()) {
                            continue;
                        }
                        const std::string nl = newline(content);

                        // the mod's own texture overrides of the second component's textures (every role the main
                        // shader reads, the normal map included), bound by register: the moved draws read them
                        std::string texLines;
                        for (const ZZMIFixerConfig::Component& c : config_.components) {
                            if (c.name != pair.second) {
                                continue;
                            }
                            for (const auto& roleReg : config_.roleRegs) {
                                const std::optional<std::string> h = hashOf(config_.source + c.name, "tex_a_" + roleReg.first);
                                const auto reg = config_.roleRegs.find(roleReg.first);
                                if (!h.has_value()) {
                                    continue;
                                }
                                for (const RawSection& s : sectionsOf(own_)) {
                                    const std::optional<std::string> hash = firstValue(s.body, IniKeywords::Hash);
                                    if (!hash.has_value() || !StringTools::equalsIgnoreCase(*hash, *h)
                                            || !firstValue(s.body, IniKeywords::This).has_value()) {
                                        continue;
                                    }
                                    std::istringstream stream(s.body);
                                    std::string line;
                                    while (std::getline(stream, line)) {
                                        if (!line.empty() && line.back() == '\r') {
                                            line.pop_back();
                                        }
                                        const std::string stripped = std::string(StringTools::strip(line));
                                        const auto kv = kvOf(stripped);
                                        if (isStructure(stripped)) {
                                            texLines += line + nl;
                                        } else if (kv.has_value() && StringTools::equalsIgnoreCase(kv->first, IniKeywords::This)) {
                                            texLines += line.substr(0, line.find_first_not_of(" \t")) + reg->second + " = " + kv->second + nl;
                                        }
                                    }
                                }
                            }
                        }

                        // the second component's drawn sections, in name order (3DMigoto's), with their cross draws
                        std::vector<RawSection> legs;
                        for (const RawSection& s : sectionsOf(own_)) {
                            const std::optional<std::string> hash = firstValue(s.body, IniKeywords::Hash);
                            if (hash.has_value() && StringTools::equalsIgnoreCase(*hash, *legsIb)
                                    && firstValue(s.body, IniKeywords::MatchFirstIndex).has_value()) {
                                legs.push_back(s);
                            }
                        }
                        std::sort(legs.begin(), legs.end(), [](const RawSection& a, const RawSection& b) {
                            return lowered(a.name) < lowered(b.name); });

                        std::string moved;
                        bool anyCross = false;
                        for (const RawSection& s : legs) {
                            bool hasCross = false;
                            const std::string lines = crossLines(s.body, written, false, hasCross, nl);
                            anyCross = anyCross || hasCross;
                            moved += lines;
                        }
                        if (!anyCross) {
                            continue;
                        }

                        // the copy of this component's first drawn section
                        const std::vector<RawSection> sections = sectionsOf(content);
                        std::string out;
                        const std::size_t first = content.find('[');
                        out = first == std::string::npos ? std::string() : content.substr(0, first);
                        bool inserted = false;
                        for (const RawSection& s : sections) {
                            std::string body = s.body;
                            if (!inserted && firstValue(s.body, IniKeywords::MatchFirstIndex).has_value()) {
                                std::string trimmed = std::string(StringTools::rstrip(body));
                                body = trimmed + nl + "; the draws this mod makes of its " + pair.first + " from its "
                                    + pair.second + " slot" + nl + texLines + moved + nl;
                                inserted = true;
                            }
                            out += "[" + s.name + "]" + nl + body;
                        }
                        if (!inserted) {
                            ctx_.log("the " + pair.first + " draws this mod makes from its " + pair.second
                                     + " slot have no " + pair.first + " section to move into");
                            return content;
                        }
                        return out;
                    }
                    return content;
                }

                using GroupPartEdit = ObjGroupEdit::PartEdit;

                mutable IniFileFixContext ctx_;
                std::string toModName_;
                ZZMIFixerConfig config_;
                ZZMIFixerConfig::Component comp_;
                std::string own_;

                std::unique_ptr<ObjGroupRemap> remap_;
                std::vector<std::unique_ptr<Collector>> collects_;
                std::vector<std::unique_ptr<Collector>> collectsBox_;
                std::vector<std::unique_ptr<Collector>> groupCollectsAfterStrip_;
                std::vector<std::unique_ptr<Fixer::GroupEdit>> groupEdits_;
                std::unique_ptr<ObjGroupEdit> bandEdits_;
                std::vector<std::unique_ptr<BaseResEdit<>>> resEdits_;
                std::vector<std::unique_ptr<BaseRegEdit<>>> regEdits_;
                std::vector<std::unique_ptr<GraphRename<>>> renames_;
                std::vector<std::unique_ptr<GroupPartEdit>> partEdits_;
                std::unordered_map<std::string, std::string> defaults_;
        };
    }


    IniParseBuilder::Factory makeZZMIParser(ZZMIFixerConfig config) {
        return [config](IniFile* iniFile, std::optional<int> modTypeId) {
            std::vector<ModObj> modObjs;
            for (const ZZMIFixerConfig::Component& c : config.components) {
                for (const std::string& obj : compObjs(c)) {
                    modObjs.push_back(objOf(c.name, obj));
                }
            }
            return std::make_shared<ZZMIGIMIParser>(iniFile, modTypeId, std::move(modObjs), config);
        };
    }


    IniFixBuilder::Factory makeZZMIFixer(ZZMIFixerConfig config, std::string component) {
        return [config, component](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<ZZMIFixerImpl>(parser, toModName, modTypeId, config, component);
        };
    }
}
