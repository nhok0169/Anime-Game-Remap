#include "AGRemapCore/data/IniParseData/Raiden/RaidenParser.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"


namespace AGRemapCore {
    namespace {
        using Parser = GIMIParser<>;
        using Classifier = Parser::Classifier;
        using ModObj = Parser::ModObj;

        // The last index column of a Hashes row for an index buffer / a blend vertex buffer.
        // Spelled as literals for the same reason HashToModObjData.cpp spells them that way: these
        // are keys *of the hash data table*, not .ini register names, so IniKeywords::Ib would be
        // the wrong constant even though it happens to hold the same characters.
        const std::string IbHashKey = "ib";
        const std::string BlendHashKey = "blend_vb";

        // The face's own diffuse, classified by a hash of its own. It draws nothing and has no
        // index; it exists as a mod object purely so the fix can reach the TEXTURES its graph
        // points at -- see the fixer, which collects that graph's ps-t0 and makes the blush mask
        // in the alpha channel transparent.
        const std::string FaceDiffuseHashKey = "tex_face_diffuse";

        /**
         * A GIMIParser that owns both the context it reads through and the classifier it
         * classifies with. GIMIParser holds a bare Context* and stores its ObjTargetFuncs as
         * std::functions, so something has to keep both alive for exactly as long as the parser --
         * and a Factory hands back only the parser. Same shape (and same member-ordering reason)
         * as IniParseBuilder.cpp's own OwnedContextGIMIParser, plus the classifier.
         */
        class ObjClassifiedGIMIParser: public Parser {
            public:
                ObjClassifiedGIMIParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs,
                                         std::unordered_map<std::string, ModObj> hashKeyOnlyToModObj,
                                         std::unordered_map<std::string, Classifier::IndexModObjs> indexKeyToModObj):
                    Parser(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId) {
                    this->setCtx(&ctx_);

                    // See OwnedContextGIMIParser's own note: a fixer takes its .ini file from
                    // parser->getIniFile(), which GIMIParser's constructor never sets.
                    this->setIniFile(iniFile);

                    // One section may belong to several mod objects. 3dmigoto reads only the FIRST
                    // hash/match_first_index pair in a section and applies it to every branch,
                    // ignoring its own if/else grammar -- so a section that spells out a different
                    // pair per branch does not actually cycle mod objects the way its author
                    // intended. Attributing such a section to every object it names is what a
                    // reader expects, and costs nothing in practice: nobody writes that structure,
                    // because anyone who tries hits the same 3dmigoto bug and stops.
                    //
                    // Set here rather than passed positionally so the arguments this factory does
                    // NOT care about (downloads, commandEdits, makeGlobalGraph) do not have to be
                    // spelled out just to reach this one -- same reason objTargetFuncs is assigned
                    // below rather than handed to the constructor.
                    this->disjointModObjs = false;

                    // The asset tables are borrowed from the ModType the .ini file was classified
                    // as, which outlives every parser built over it.
                    classifier_ = std::make_unique<Classifier>(
                        std::move(hashKeyOnlyToModObj), ctx_.modTypeHashes(),
                        std::move(indexKeyToModObj), ctx_.modTypeIndices(), ctx_.version());

                    Classifier* classifier = classifier_.get();
                    this->objTargetFuncs.emplace_back(
                        [classifier](Parser&, const std::string& sectionName, Section* section, bool,
                                      ContentPart*, const Colouring* kvps) {
                            // kvps is null whenever the parser is not tracking KVPs, which is the
                            // one case this strategy cannot answer -- there is nothing to match a
                            // hash or a first index against.
                            if (kvps == nullptr) {
                                return std::vector<ModObj>();
                            }

                            return classifier->classify(sectionName, section, *kvps);
                        });
                }

            private:
                IniFileParseContext ctx_;
                std::unique_ptr<Classifier> classifier_;
        };
    }

    IniParseBuilder::Factory IniParseBuilderFuncs::raiden6_1() {
        // Built once and captured, not rebuilt per .ini file: the mapping is pure data, and every
        // parser this factory makes wants the identical copy.
        //
        // Two kinds of mod object, told apart two different ways:
        //
        //  * head/body/dress all draw from the same 'ib', so a hash alone cannot separate them --
        //    each is keyed by the last two index columns of its own Indices row, (component,
        //    object), which for Raiden is exactly the mod object itself
        //  * blend is the Blend.buf the fix remaps, and its 'blend_vb' hash names it outright --
        //    no index needed, so it goes in the hash-only mapping instead
        //  * face is the same shape as blend -- named outright by 'tex_face_diffuse' -- and is
        //    tracked for its textures rather than its geometry
        const std::vector<ModObj> ibModObjs = {{"", "head"}, {"", "body"}, {"", "dress"}};
        const ModObj blendModObj{"", "blend"};
        const ModObj faceModObj{"", "face"};

        std::vector<ModObj> modObjs = ibModObjs;
        modObjs.push_back(blendModObj);
        modObjs.push_back(faceModObj);

        Classifier::IndexModObjs indexModObjs;
        for (const ModObj& modObj : ibModObjs) {
            indexModObjs.emplace(Classifier::IndexKey(modObj.first, modObj.second), modObj);
        }

        const std::unordered_map<std::string, ModObj> hashKeyOnlyToModObj = {{BlendHashKey, blendModObj},
                                                                              {FaceDiffuseHashKey, faceModObj}};
        const std::unordered_map<std::string, Classifier::IndexModObjs> indexKeyToModObj = {{IbHashKey, std::move(indexModObjs)}};

        return [modObjs, hashKeyOnlyToModObj, indexKeyToModObj](IniFile* iniFile, std::optional<int> modTypeId) {
            return std::make_shared<ObjClassifiedGIMIParser>(iniFile, modTypeId, modObjs, hashKeyOnlyToModObj,
                                                              indexKeyToModObj);
        };
    }


    IniParseBuilder::Factory RaidenParser::v6_1() {
        return IniParseBuilderFuncs::raiden6_1();
    }
}
