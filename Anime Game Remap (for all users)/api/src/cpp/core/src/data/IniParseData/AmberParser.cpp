#include "AGRemapCore/data/IniParseData/AmberParser.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/VertexCountData.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseDownloadData.h"
#include "AGRemapCore/tools/files/FileDownload.h"


namespace AGRemapCore {
    namespace {
        using Parser = GIMIParser<>;
        using Classifier = Parser::Classifier;
        using ModObj = Parser::ModObj;
        // DownloadData, the CONCRETE class -- IniParseDownloadData is the abstract base and has no
        // DownloadConfig of its own.
        using Download = DownloadData<>;

        // Hash-data keys, not .ini register names -- spelled literally for the same reason
        // HashToModObjData.cpp spells them that way.
        const std::string IbHashKey = "ib";
        const std::string BlendHashKey = "blend_vb";
        const std::string PositionHashKey = "position_vb";
        const std::string TexcoordHashKey = "texcoord_vb";

        // The two hash types that belong to no drawn object of their own: the VertexLimitRaise
        // section ('draw_vb') and the face's diffuse override ('tex_face_diffuse'). Both are plain
        // hash swaps with no geometry behind them, which is what ("", "other") is for.
        const std::string DrawHashKey = "draw_vb";
        const std::string FaceDiffuseHashKey = "tex_face_diffuse";

        // Where every downloadable default part is published. The pure-Python original's own
        // 'GithubDownloadFolder' (data/FileDownloadData.py).
        const std::string DownloadFolder =
            "https://github.com/nhok0169/Anime-Game-Remap/raw/nhok0169/Data/Mod%20Downloads";

        /**
         * One row out of VertexCountData, by its (version, mod name, component) key.
         *
         * VertexCountData.h exposes only the raw row list -- there is no lookup helper -- so this
         * is the small scan the pure-Python original gets for free from a nested dict. Returns 0
         * when the row is missing, which produces a visibly wrong "draw = 0,0" rather than a
         * silently plausible one.
         */
        long long vertexCountOf(const std::string& version, const std::string& modName, const std::string& component) {
            for (const auto& row : Data::getVertexCountDataRows()) {
                if (row.first.size() == 3 && row.first[0] == version && row.first[1] == modName
                        && row.first[2] == component) {
                    return row.second;
                }
            }

            return 0;
        }


        /**
         * The .ini-domain customization points every download this file builds shares.
         *
         * The plain-std::string counterpart of what the binding layer supplies for its own
         * py::object instantiation -- 'filename' is the register naming a resource's file, and both
         * conversions are the identity because K and V already ARE std::string here.
         */
        Download::DownloadConfig makeDownloadConfig() {
            Download::DownloadConfig config{};
            config.filenameKey = IniKeywords::Filename;
            config.valOfPath = [](const std::string& path) { return path; };
            config.runConfig = IfTemplateRunConfig<std::string, std::string>{
                IniKeywords::Run,
                [](const std::string& val) { return val; },
                [](const std::string& name) { return name; }
            };

            return config;
        }


        /**
         * One downloadable part.
         *
         * 'fixedName' is what the file is saved as locally -- always carrying IniKeywords::RemapDL,
         * which is how the remover later recognises a file this software downloaded rather than one
         * the modder shipped.
         */
        std::unique_ptr<Download> makeDownload(const std::string& name, const std::string& urlPath,
                                                const std::string& fixedName,
                                                std::vector<std::pair<std::string, std::string>> resourceKVPs = {},
                                                std::vector<std::pair<std::string, std::string>> downloadRefKVPs = {}) {
            return std::make_unique<Download>(
                name,
                std::make_unique<FileDownload>(DownloadFolder + "/" + urlPath, fixedName),
                makeDownloadConfig(),
                /*refToSection*/ false,
                std::move(downloadRefKVPs),
                std::move(resourceKVPs));
        }


        /**
         * A GIMIParser that owns its context, its classifier AND every download it was given.
         *
         * GIMIParser's 'downloads' map holds BORROWED pointers, so the DownloadData objects have to
         * outlive the parser -- the same ownership story as the context and the classifier, and the
         * same reason a Factory subclass exists at all rather than a bare lambda.
         */
        class AmberGIMIParser: public Parser {
            public:
                AmberGIMIParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs,
                                 std::unordered_map<std::string, ModObj> hashKeyOnlyToModObj,
                                 std::unordered_map<std::string, Classifier::IndexModObjs> indexKeyToModObj,
                                 long long vertexCount, int texcoordStride):
                    Parser(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId) {
                    this->setCtx(&ctx_);
                    this->setIniFile(iniFile);

                    // One section may name several mod objects -- see RaidenParser for the 3dmigoto
                    // grammar bug this works around.
                    this->disjointModObjs = false;

                    classifier_ = std::make_unique<Classifier>(
                        std::move(hashKeyOnlyToModObj), ctx_.modTypeHashes(),
                        std::move(indexKeyToModObj), ctx_.modTypeIndices(), ctx_.version());

                    Classifier* classifier = classifier_.get();
                    this->objTargetFuncs.emplace_back(
                        [classifier](Parser&, const std::string& sectionName, Section* section, bool,
                                      ContentPart*, const Colouring* kvps) {
                            if (kvps == nullptr) {
                                return std::vector<ModObj>();
                            }

                            return classifier->classify(sectionName, section, *kvps);
                        });

                    buildDownloads(vertexCount, texcoordStride);
                }

            private:
                /**
                 * The default parts a modder may have left out, so a fix can still reference them.
                 *
                 * The pure-Python original splits these across two arguments -- 'objFileDownloads'
                 * (per mod object) and 'bufDownloads' (per .buf kind). The C++ 'downloads' map is
                 * keyed by mod object throughout, and the .buf kinds ARE mod objects here (the
                 * parser classifies blend/position/texcoord in their own right), so the two
                 * collapse into one map with no information lost.
                 */
                void buildDownloads(long long vertexCount, int texcoordStride) {
                    const std::vector<std::pair<std::string, std::string>> bufKVPs32 = {{"type", "Buffer"}, {"stride", "32"}};
                    const std::vector<std::pair<std::string, std::string>> bufKVPs40 = {{"type", "Buffer"}, {"stride", "40"}};
                    const std::vector<std::pair<std::string, std::string>> texKVPs = {
                        {"type", "Buffer"}, {"stride", std::to_string(texcoordStride)}};
                    const std::vector<std::pair<std::string, std::string>> ibKVPs = {
                        {"type", "Buffer"}, {"format", "DXGI_FORMAT_R32_UINT"}};

                    // A downloaded Blend.buf is drawn by hand: the .ini file it is dropped into has
                    // no drawindexed of its own for it, so the reference is followed by
                    // 'handling = skip' and a non-indexed 'draw = <vertex count>,0'. That is what
                    // the pure-Python BlendDownloadData adds in its own addToPart override, and
                    // what downloadRefKVPs ("KVPs to add after the download reference") is for.
                    const std::vector<std::pair<std::string, std::string>> blendRefKVPs = {
                        {IniKeywords::Handling, "skip"},
                        {IniKeywords::Draw, std::to_string(vertexCount) + ",0"}};

                    for (const std::string& obj : {std::string("Head"), std::string("Body")}) {
                        const std::string lower = (obj == "Head") ? "head" : "body";

                        addDownload({"", lower}, "ps-t0",
                                     makeDownload("Diffuse", "GI/Amber/4_0/Amber" + obj + "Diffuse.dds",
                                                   "Amber" + obj + "Diffuse" + IniKeywords::RemapDL + ".dds"));
                        addDownload({"", lower}, "ps-t1",
                                     makeDownload("LightMap", "GI/Amber/4_0/Amber" + obj + "LightMap.dds",
                                                   "Amber" + obj + "LightMap" + IniKeywords::RemapDL + ".dds"));
                        addDownload({"", lower}, IniKeywords::Ib,
                                     makeDownload("Ib", "GI/Amber/4_0/Amber" + obj + ".ib",
                                                   "Amber" + obj + IniKeywords::RemapDL + ".ib", ibKVPs));
                    }

                    addDownload({"", "blend"}, IniKeywords::Vb1,
                                 makeDownload(IniKeywords::Blend, "GI/Amber/4_0/AmberBlend.buf",
                                               "AmberBlend" + IniKeywords::RemapDL + ".buf", bufKVPs32, blendRefKVPs));
                    addDownload({"", "position"}, IniKeywords::Vb0,
                                 makeDownload(IniKeywords::Position, "GI/Amber/4_0/AmberPosition.buf",
                                               "AmberPosition" + IniKeywords::RemapDL + ".buf", bufKVPs40));
                    addDownload({"", "texcoord"}, IniKeywords::Vb1,
                                 makeDownload(IniKeywords::Texcoord, "GI/Amber/4_0/AmberTexcoord.buf",
                                               "AmberTexcoord" + IniKeywords::RemapDL + ".buf", texKVPs));
                }

                void addDownload(const ModObj& modObj, const std::string& reg, std::unique_ptr<Download> download) {
                    this->downloads[modObj][reg] = download.get();
                    ownedDownloads_.push_back(std::move(download));
                }

                IniFileParseContext ctx_;
                std::unique_ptr<Classifier> classifier_;
                std::vector<std::unique_ptr<Download>> ownedDownloads_;
        };
    }

    IniParseBuilder::Factory IniParseBuilderFuncs::amber4_0() {
        // THREE kinds of mod object here, not Raiden's two, and the third falls out of the
        // classifier's own logic rather than needing anything new:
        //
        //  * head/body share one 'ib' and are told apart by the match_first_index that follows it
        //  * blend/position/texcoord are each named outright by their own hash
        //  * ("", "ib") is the section that carries the 'ib' hash and NO match_first_index at all --
        //    the shared draw call the objects run into. GIMISectionClassifier already does this:
        //    with "ib" in BOTH maps, a part whose index matches resolves to head or body, and one
        //    with no index falls through to the hash-only entry. See its classify(), where
        //    'inHashOnly' is what catches the no-index case
        //
        // ("", "other") is the fourth kind, and the one Raiden deliberately does without: SEVERAL
        // hash types mapping to ONE mod object. VertexLimitRaise and the face diffuse each own a
        // hash and nothing else -- no geometry, no index, no resource to rebuild -- so the fix has
        // the same thing to say about both of them (swap the hash, rename the section), and there
        // is no reason to give them separate graphs. Raiden skipped it because her 6.1 fix changes
        // nothing about those sections and an empty remap is wasted space in the .ini file; Amber
        // needs it because the CN skin genuinely has different hashes for them.
        const std::vector<ModObj> ibModObjs = {{"", "head"}, {"", "body"}};
        const ModObj otherObj{"", "other"};
        const std::vector<std::pair<std::string, ModObj>> hashOnlyObjs = {
            {IbHashKey, {"", "ib"}},
            {BlendHashKey, {"", "blend"}},
            {PositionHashKey, {"", "position"}},
            {TexcoordHashKey, {"", "texcoord"}},
            {DrawHashKey, otherObj},
            {FaceDiffuseHashKey, otherObj}};

        // Deduplicated, because the map above is many-hash-types-to-one-object: ("", "other")
        // appears twice there and must appear once here, or the parser is handed a duplicate graph.
        std::vector<ModObj> modObjs = ibModObjs;
        for (const auto& entry : hashOnlyObjs) {
            if (std::find(modObjs.begin(), modObjs.end(), entry.second) == modObjs.end()) {
                modObjs.push_back(entry.second);
            }
        }

        Classifier::IndexModObjs indexModObjs;
        for (const ModObj& modObj : ibModObjs) {
            indexModObjs.emplace(Classifier::IndexKey(modObj.first, modObj.second), modObj);
        }

        const std::unordered_map<std::string, ModObj> hashKeyOnlyToModObj(hashOnlyObjs.begin(), hashOnlyObjs.end());
        const std::unordered_map<std::string, Classifier::IndexModObjs> indexKeyToModObj = {{IbHashKey, std::move(indexModObjs)}};

        // Baked in once, exactly as the pure-Python original bakes 'VertexCountData[4.0][Amber]'
        // and 'TexcoordByteSizeData[4.0][Amber]' into its own table.
        //
        // The texcoord stride is a literal because there is no C++ TexcoordByteSizeData yet -- only
        // data/TexcoordByteSizeData.py, which this layer cannot read. Worth porting once a second
        // character needs it; until then a literal with this comment beats a table with one row.
        const long long vertexCount = vertexCountOf("4.0", ModTypeIdTools::getName(ModTypeId::Amber), "");
        const int texcoordStride = 12;

        return [modObjs, hashKeyOnlyToModObj, indexKeyToModObj, vertexCount, texcoordStride](
                   IniFile* iniFile, std::optional<int> modTypeId) {
            return std::make_shared<AmberGIMIParser>(iniFile, modTypeId, modObjs, hashKeyOnlyToModObj,
                                                      indexKeyToModObj, vertexCount, texcoordStride);
        };
    }


    IniParseBuilder::Factory AmberParser::v4_0() {
        return IniParseBuilderFuncs::amber4_0();
    }
}
