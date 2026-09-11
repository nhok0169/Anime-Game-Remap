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

#include "AGRemapCore/data/IniParseData/GIMICharParser.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"
#include "AGRemapCore/tools/DownloadTools.h"
#include "AGRemapCore/tools/TextTools.h"


namespace AGRemapCore {
    namespace {
        using Parser = GIMIParser<>;
        using Classifier = Parser::Classifier;
        using ModObj = Parser::ModObj;

        // Hash-data keys, not .ini register names -- spelled literally for the same reason
        // HashToModObjData.cpp spells them that way.
        const std::string IbHashKey = "ib";
        const std::string BlendHashKey = "blend_vb";
        const std::string PositionHashKey = "position_vb";
        const std::string TexcoordHashKey = "texcoord_vb";

        // The VertexLimitRaise section: a hash type belonging to no drawn object of its own, and a
        // plain hash swap with no geometry behind it. That is what ("", "other") is for.
        const std::string DrawHashKey = "draw_vb";

        // The face's own diffuse. It gets a mod object of its own -- rather than sharing
        // ("", "other") with the above, which is where it used to live -- because the fix reaches
        // into this graph to swap the diffuse and lightmap registers, which GI 6.x swapped under
        // everyone's feet. See GIMICharFixer.
        const std::string FaceDiffuseHashKey = "tex_face_diffuse";

        // The version VertexCountData is keyed under. Every row in that table is 4.0; it is the
        // count of the model itself, which no game version since has changed.
        const std::string VertexCountVersion = "4.0";


        /**
         * A GIMIParser that owns its context, its classifier AND every download it was given.
         *
         * GIMIParser's 'downloads' map holds BORROWED pointers, so the DownloadData objects have to
         * outlive the parser -- the same ownership story as the context and the classifier, and the
         * same reason a Factory subclass exists at all rather than a bare lambda.
         */
        class GIMICharGIMIParser: public Parser {
            public:
                GIMICharGIMIParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs,
                                    std::unordered_map<std::string, ModObj> hashKeyOnlyToModObj,
                                    std::unordered_map<std::string, Classifier::IndexModObjs> indexKeyToModObj,
                                    const GIMICharParserConfig& config, long long vertexCount):
                    Parser(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId) {
                    this->setCtx(&ctx_);
                    this->setIniFile(iniFile);

                    // One section may name several mod objects -- see RaidenParser for the 3dmigoto
                    // grammar bug this works around.
                    this->disjointModObjs = false;

                    // Copied before the classifier takes them: the parser needs the same two maps
                    // to answer objIdentityKVPs below, and the classifier's constructor moves from
                    // these.
                    const auto hashOnlyMap = hashKeyOnlyToModObj;
                    const auto indexMap = indexKeyToModObj;

                    classifier_ = std::make_unique<Classifier>(
                        std::move(hashKeyOnlyToModObj), ctx_.modTypeHashes(),
                        std::move(indexKeyToModObj), ctx_.modTypeIndices(), ctx_.version());

                    // WHAT A SECTION FOR THIS OBJECT WOULD HAVE CARRIED, for the one case where the
                    // parser has to invent one -- see GIMIParser::objIdentityKVPs. Built from the
                    // very maps the classifier uses to recognise a real section, so the two cannot
                    // drift: whatever identifies an object on the way in is what an invented section
                    // says about itself on the way out.
                    this->objIdentityKVPs =
                        [this, hashOnlyMap, indexMap](const ModObj& modObj) {
                            std::vector<std::pair<std::string, std::string>> kvps;

                            // Asked of the context rather than captured: it is the same name the
                            // classifier's own lookups are filtered by, and asking keeps the two
                            // from drifting if the .ini is reclassified.
                            const std::string srcModName = ctx_.modTypeName();

                            auto* hashes = ctx_.modTypeHashes();
                            auto* indices = ctx_.modTypeIndices();
                            const std::optional<Version> version = ctx_.version();

                            // A DRAWN object first: identified by the shared 'ib' hash AND its own
                            // match_first_index. Checked before the hash-only map because ('', 'ib')
                            // lives in both -- see makeGIMICharParser's note on that overlap.
                            for (const auto& indexEntry : indexMap) {
                                for (const auto& objEntry : indexEntry.second) {
                                    if (objEntry.second != modObj) {
                                        continue;
                                    }

                                    if (hashes != nullptr) {
                                        std::optional<std::string> hash =
                                            hashes->get({srcModName, indexEntry.first}, version, false);
                                        if (hash.has_value()) {
                                            kvps.emplace_back(IniKeywords::Hash, *hash);
                                        }
                                    }

                                    if (indices != nullptr) {
                                        std::optional<std::string> index =
                                            indices->get({srcModName, objEntry.first.first, objEntry.first.second},
                                                          version, false);
                                        if (index.has_value()) {
                                            kvps.emplace_back(IniKeywords::MatchFirstIndex, *index);
                                        }
                                    }

                                    return kvps;
                                }
                            }

                            // Everything else -- the face, the buffers -- is named by a hash of its
                            // own and has no index at all.
                            for (const auto& hashEntry : hashOnlyMap) {
                                if (hashEntry.second != modObj || hashes == nullptr) {
                                    continue;
                                }

                                std::optional<std::string> hash =
                                    hashes->get({srcModName, hashEntry.first}, version, false);
                                if (hash.has_value()) {
                                    kvps.emplace_back(IniKeywords::Hash, *hash);
                                }

                                return kvps;
                            }

                            return kvps;
                        };

                    Classifier* classifier = classifier_.get();
                    this->objTargetFuncs.emplace_back(
                        [classifier](Parser&, const std::string& sectionName, Section* section, bool,
                                      ContentPart*, const Colouring* kvps) {
                            if (kvps == nullptr) {
                                return std::vector<ModObj>();
                            }

                            return classifier->classify(sectionName, section, *kvps);
                        });

                    buildDownloads(config, vertexCount);
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
                 *
                 * The shape is the same for every character this builds: two textures and an index
                 * buffer per drawn object, the three buffers, and the face. Only the names differ.
                 */
                void buildDownloads(const GIMICharParserConfig& config, long long vertexCount) {
                    for (const std::string& obj : config.drawnObjs) {
                        // "head" is the mod object; "Head" is the middle of the file name AND of the
                        // .ini resource name.
                        //
                        // THE OBJECT HAS TO BE IN THE .INI NAME, not just the file name.
                        // createDownloadResource builds the resource section as
                        // "<modType><download name>" and dedupes on it, so calling these "Diffuse"
                        // gave every drawn object the same section: head's and body's diffuse
                        // downloads collapsed into one [Resource<Mod>DiffuseRemapDL], one of them
                        // silently winning. Naming them "HeadDiffuse"/"BodyDiffuse" separates them
                        // and matches the old script, which emits [Resource<Mod>BodyDiffuseRemapDL].
                        const std::string file = TextTools::capitalize(obj);

                        // refToSection: referenced ONCE from the top of each root section the
                        // register does not fully cover, which is what the pure-Python original
                        // does -- its output carries all three on
                        // [TextureOverride<Mod><Obj><Target>RemapFix].
                        //
                        // Per-part placement (the default) only agrees with that while every
                        // branch of a CommandList binds every register, as Ningguang's single
                        // if-block does. GanyuTwilight's body is two sequential if-blocks -- six
                        // branches binding ib, then two binding ps-t0/ps-t1 -- so per-part gave
                        // the $Tight branches an ib they should not have and the ib branches a
                        // ps-t0 they should not have. The draw call is derived from ib, so the two
                        // stray ib lines became two stray drawindexed: 8 draws instead of 6, and
                        // ORFix/NNFix run 8 times instead of 2. ORFix swaps the diffuse and
                        // lightmap registers per call, so the surplus swaps turned the model green
                        // and yellow.
                        // ps-t0/ps-t1 unless this object says otherwise -- see
                        // GIMICharParserConfig::objDownloadRegs for why a 4.0-era character does.
                        GIMICharParserConfig::ObjDownloadRegs regs;
                        for (const auto& override_ : config.objDownloadRegs) {
                            if (override_.obj == obj) {
                                regs = override_;
                                break;
                            }
                        }

                        // Before the diffuse, matching the order the pure-Python table lists them
                        // in -- the .ini file's sections come out in the order they were added.
                        if (!regs.normalMapReg.empty()) {
                            add(config, {"", obj}, regs.normalMapReg, file + "NormalMap", file + "NormalMap", ".dds",
                                 {}, {}, true);
                        }

                        add(config, {"", obj}, regs.diffuseReg, file + "Diffuse", file + "Diffuse", ".dds",
                             {}, {}, true);
                        // ...unless there is no lightmap upstream to fetch -- see
                        // GIMICharParserConfig::objsWithoutLightMap.
                        if (std::find(config.objsWithoutLightMap.begin(), config.objsWithoutLightMap.end(), obj)
                                == config.objsWithoutLightMap.end()) {
                            add(config, {"", obj}, regs.lightMapReg, file + "LightMap", file + "LightMap", ".dds",
                                 {}, {}, true);
                        }
                        add(config, {"", obj}, IniKeywords::Ib, file + "Ib", file, ".ib",
                             DownloadTools::ibResourceKVPs(), {}, true);
                    }

                    // The face diffuse. Named for the character rather than for an object, so
                    // 'FaceDiffuse' serves as both the .ini name and the middle of the file name.
                    //
                    // Filled on the register the mod WOULD have used, ps-t0, and left for the
                    // fixer's swap to move to ps-t1 along with everything else in the graph. The
                    // parser adds a download's KVP straight into the part it is missing from
                    // (GIMIParser::addDownloads), so by the time the fix runs a downloaded diffuse
                    // is indistinguishable from one the mod shipped -- which is exactly what makes
                    // the ordering work.
                    // ...from wherever this character's face diffuse actually lives, which for three
                    // of them is not where the rest of their assets do -- see
                    // GIMICharParserConfig::faceDownloadVersionFolder.
                    GIMICharParserConfig faceConfig = config;
                    if (!config.faceDownloadVersionFolder.empty()) {
                        faceConfig.downloadVersionFolder = config.faceDownloadVersionFolder;
                    }

                    if (!config.faceDownloadPrefix.empty()) {
                        faceConfig.downloadPrefix = config.faceDownloadPrefix;
                    }

                    add(faceConfig, {"", "face"}, "ps-t0", "FaceDiffuse", "FaceDiffuse", ".dds");

                    // Per buffer kind. The blend is the one that needs downloadRefKVPs -- see
                    // DownloadTools::blendRefKVPs for why a downloaded Blend.buf has to be drawn
                    // by hand.
                    add(config, {"", "blend"}, IniKeywords::Vb1, IniKeywords::Blend, "Blend", ".buf",
                         DownloadTools::bufResourceKVPs(config.blendStride),
                         DownloadTools::blendRefKVPs(vertexCount));
                    add(config, {"", "position"}, IniKeywords::Vb0, IniKeywords::Position, "Position", ".buf",
                         DownloadTools::bufResourceKVPs(config.positionStride));
                    add(config, {"", "texcoord"}, IniKeywords::Vb1, IniKeywords::Texcoord, "Texcoord", ".buf",
                         DownloadTools::bufResourceKVPs(config.texcoordStride));
                }

                /**
                 * One download, named the same way on both sides.
                 *
                 * 'kind' is what the download is called in the .ini file it creates; 'file' is the
                 * middle of the file name on GitHub and on disk. They differ where the file name
                 * carries the object ("HeadDiffuse") but the .ini name does not ("Diffuse").
                 */
                void add(const GIMICharParserConfig& config, const ModObj& modObj, const std::string& reg,
                          const std::string& kind, const std::string& file, const std::string& ext,
                          DownloadTools::KVPs resourceKVPs = {}, DownloadTools::KVPs downloadRefKVPs = {},
                          bool refToSection = false) {
                    downloadStore_.add(
                        this->downloads, modObj, reg,
                        DownloadTools::make(kind,
                                             DownloadTools::urlPath(config.downloadCharFolder,
                                                                     config.downloadVersionFolder,
                                                                     config.downloadPrefix, file, ext),
                                             DownloadTools::fixedFileName(config.downloadPrefix, file, ext),
                                             std::move(resourceKVPs), std::move(downloadRefKVPs),
                                             refToSection));
                }

                IniFileParseContext ctx_;
                std::unique_ptr<Classifier> classifier_;
                DownloadStore downloadStore_;
        };
    }


    IniParseBuilder::Factory makeGIMICharParser(GIMICharParserConfig config) {
        // FOUR kinds of mod object, and only the first needs anything the classifier does not
        // already do on its own:
        //
        //  * the drawn objects all share one 'ib' and are told apart by the match_first_index that
        //    follows it -- each keyed by the last two index columns of its own Indices row,
        //    (component, object)
        //  * blend/position/texcoord are each named outright by their own hash
        //  * ("", "ib") is the section carrying the 'ib' hash and NO match_first_index at all --
        //    the shared draw call the objects run into. GIMISectionClassifier already does this:
        //    with "ib" in BOTH maps, a part whose index matches resolves to a drawn object, and one
        //    with no index falls through to the hash-only entry. See its classify(), where
        //    'inHashOnly' is what catches the no-index case
        //  * ("", "other") and ("", "face") are each named by a hash of their own, like the buffers
        std::vector<ModObj> ibModObjs;
        for (const std::string& obj : config.drawnObjs) {
            ibModObjs.emplace_back("", obj);
        }

        const ModObj otherObj{"", "other"};
        const ModObj faceObj{"", "face"};
        const std::vector<std::pair<std::string, ModObj>> hashOnlyObjs = {
            {IbHashKey, {"", "ib"}},
            {BlendHashKey, {"", "blend"}},
            {PositionHashKey, {"", "position"}},
            {TexcoordHashKey, {"", "texcoord"}},
            {DrawHashKey, otherObj},
            {FaceDiffuseHashKey, faceObj}};

        // Deduplicated, because the map above may be many-hash-types-to-one-object: an object
        // appearing twice there must appear once here, or the parser is handed a duplicate graph.
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
        const std::unordered_map<std::string, Classifier::IndexModObjs> indexKeyToModObj = {
            {IbHashKey, std::move(indexModObjs)}};

        // Baked in once, exactly as the pure-Python original bakes 'VertexCountData[4.0][<char>]'
        // into its own table.
        const long long vertexCount = DownloadTools::vertexCountOf(
            VertexCountVersion, ModTypeIdTools::getName(config.modTypeId), "");

        return [modObjs, hashKeyOnlyToModObj, indexKeyToModObj, config, vertexCount](
                   IniFile* iniFile, std::optional<int> modTypeId) {
            return std::make_shared<GIMICharGIMIParser>(iniFile, modTypeId, modObjs, hashKeyOnlyToModObj,
                                                         indexKeyToModObj, config, vertexCount);
        };
    }
}
