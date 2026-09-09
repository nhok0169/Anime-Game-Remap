#include "AGRemapCore/data/IniFixData/Raiden/RaidenFixer.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIObjPartFilter.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegDelimitedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/VGRemapBlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {
    namespace {
        // The last index column of the Hashes row holding a Blend.buf's hash -- a key *of the hash
        // data table*, not a .ini register name, so it is spelled literally for the same reason
        // HashToModObjData.cpp spells it that way.
        const std::string BlendHashKey = "blend_vb";

        // The 'ib' half of the same split -- head/body/dress are all reached through it, and
        // telling them apart needs a match_first_index as well. Duplicated from RaidenParser.cpp
        // rather than shared: both are internal-linkage constants of one .ini-data translation
        // unit, and the parser and the fixer are separately readable that way.
        const std::string IbHashKey = "ib";

        // The .ini register naming a resource's file, plus the identity conversions to and from a
        // file path -- the plain-std::string counterpart of the binding layer's own
        // makeResEditConfig (py/src/.../resEdits/PyResEdit.cpp).
        BaseResEdit<>::ResEditConfig makeResEditConfig() {
            return BaseResEdit<>::ResEditConfig{
                IniKeywords::Filename,
                [](const std::string& value) { return value; },
                [](const std::string& file) { return file; }
            };
        }

        using Fixer = GIMIFixer<>;
        using ModObj = Fixer::ModObj;
        // NOT named 'GroupEdit': GIMIFixer already inherits an alias of that name for
        // BaseIniGraphGroupEdit, and inside RaidenFixer the inherited one wins.
        using ObjGroupEdit = GraphGroupEdit<>;
        using GraphId = BaseIniGraphGroupEdit<>::GraphId;
        using Collector = ResRegCollect<>;
        using ObjFilter = GIMIObjPartFilter<>;

        // The mod objects Raiden's 6.1 parser classifies, split by what the fix does with them.
        // Kept in sync with IniParseBuilderFuncs::raiden6_1 by hand -- they are two halves of one
        // arrangement, and the parser is what decides these names exist at all.
        const std::vector<ModObj> RaidenDrawnObjs = {{"", "head"}, {"", "body"}, {"", "dress"}};
        const ModObj RaidenBlendObj{"", "blend"};

        // The register a mod object's section points its Blend.buf through.
        const std::string BlendReg = "vb1";

        // The face's own diffuse, given a mod object of its own so the fix can reach the registers
        // its textures hang off. See the swap below for why that is worth a mod object.
        const ModObj FaceObj{"", "face"};

        // ---- the face diffuse / lightmap register swap ----
        //
        // WHAT IT IS FOR: several months ago character faces started showing white shiny spots on
        // the cheeks. The cause is NOT the texture. GI 6.x swapped which register the shader reads
        // the face diffuse and the face lightmap out of, so a section still binding its diffuse to
        // ps-t0 is handing it to the slot the shader now treats as the LIGHTMAP -- and the blush
        // mask living in that texture's alpha channel comes back as the shiny spots.
        //
        // Swapping the two registers back is the whole fix. It is also one of the things the
        // external NNFix library does under the hood -- "an overglorified RegEdit", in the
        // maintainer's words.
        //
        // A mod binding only ps-t0 (the common case -- see any of the CN mods, whose face section
        // is three lines long) simply ends up binding only ps-t1, which is right: the game supplies
        // the slot the mod says nothing about.
        //
        // Read the register numbers off the mod's own .ini rather than assuming them -- which ps-tN
        // a character's face sits on is not derivable in general (see CreatingRemaps' note on the
        // download assets), though every character so far uses ps-t0/ps-t1.
        const std::string FaceDiffuseReg = "ps-t0";
        const std::string FaceLightMapReg = "ps-t1";

        // BOTH DIRECTIONS IN ONE RegRemap, which is what makes this a swap rather than two renames
        // that collapse into one. IfContentPart::remapKeys rebuilds the whole part in a single pass,
        // consulting the rules once per ORIGINAL key, so the ps-t0 -> ps-t1 result can never be
        // re-read as an input to the ps-t1 -> ps-t0 rule.
        std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>> makeFaceRegSwap(
                const std::string& diffuseReg, const std::string& lightMapReg) {
            using RemapTo = RemapList<std::string, std::string>;
            return {{diffuseReg, RegRemap<>::KeyRemapValue(RemapTo{lightMapReg})},
                    {lightMapReg, RegRemap<>::KeyRemapValue(RemapTo{diffuseReg})}};
        }

        /**
         * Everything AGRemapCore::GIMIFixer borrows rather than owns, kept alive for exactly as
         * long as the fixer: its context (as OwnedContextGIMIFixer already does), plus the resource
         * edit, the two register/graph edits, and the two group edits it is handed by pointer.
         *
         * Declaration order matters twice over: ctx_ before the constructor body so setCtx can take
         * its address, and every edit before the group edits that point at them.
         */
        class RaidenFixerImpl: public Fixer {
            public:
                RaidenFixerImpl(BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId):
                    Fixer(parser, nullptr, {},
                           toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                             : std::optional<std::vector<std::string>>({toModName}),
                           nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName) {
                    this->setCtx(&ctx_);

                    buildBlendCollector();
                    buildObjEdits();

                    this->graphGroupEdits = {&blendCollect_, &objEdits_};

                    // The fix rewrites head/body/dress in place rather than adding beside them, so
                    // the originals have to go from the source text it appends -- see
                    // GIMIFixer::hiddenModObjs. 'blend' is NOT hidden: its section is what the
                    // remapped Blend.buf is pointed at from, and commenting it out would break the
                    // very fix this is protecting.
                    this->hiddenModObjs.insert(RaidenDrawnObjs.begin(), RaidenDrawnObjs.end());

                    // The face IS hidden, for the same reason head/body/dress are: this remap keeps
                    // the source's hash, so the original and the remapped face section trigger on
                    // exactly the same draw.
                    //
                    // Left in, the original would bind the diffuse to ps-t0 while the remapped one
                    // binds it to ps-t1, and the face would end up with a diffuse in BOTH slots --
                    // the lightmap slot included. That is worse than the bug being fixed, and not
                    // something to leave to whichever section the game happens to apply last.
                    // Hiding the original makes the swapped binding the only answer, and costs
                    // nothing on Raiden herself, since the remapped section carries her own hash.
                    this->hiddenModObjs.insert(FaceObj);
                }

            protected:
                /**
                 * GIMIFixer's own applyGraphGroupEdits hands every group edit a nullptr .ini file,
                 * because the base has nothing castable to give -- see its own note. That is fine
                 * for an edit that only rewrites graphs, and fatal for one that has to BUILD
                 * something: ResRegCollect collects its references either way, but only ever
                 * creates the resource (and repoints the register at it) when it is given a real
                 * .ini file to build into.
                 *
                 * This fixer owns its context, so it has the .ini file to hand over.
                 */
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

            private:
                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                // ---- Blend.buf collection ----
                void buildBlendCollector() {
                    const GraphId blendGraph(0, RaidenBlendObj.first, RaidenBlendObj.second);

                    IniFile* iniFile = ctx_.getIniFile();
                    const std::optional<Version> toVersion = (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;

                    // VGRemapBlendReplace, NOT the plain RemapBlendReplace: the base copies the
                    // Blend.buf without remapping its vertex groups, which is the exploded-mesh bug
                    // rather than a milder version of it. See that class's own note.
                    // A DISTINCT mod object from the source graph: this is where the resource's
                    // own newly-created graph goes, and pointing it at blendGraph would have the
                    // resource overwrite the very graph it was collected from. The 'remap' keyword
                    // in the name is what marks it as something this software wrote rather than
                    // something the mod shipped -- see GIMIFixer::touchedSectionNames.
                    const GraphId blendResGraph(0, RaidenBlendObj.first, RaidenBlendObj.second + "RemapBlend");

                    blendReplace_ = std::make_unique<VGRemapBlendReplace<>>(
                        blendResGraph, makeResEditConfig(), ctx_.modType(), ctx_.version(), toVersion);

                    blendCollect_.srcRegs = {{blendGraph, BlendReg}};
                    blendCollect_.resEdits = {{IniKeywords::Blend, blendReplace_.get()}};

                    // The remapped blend section is a copy, so its 'hash' still names Raiden's own
                    // Blend.buf. Left alone it would never trigger on the boss at all -- see
                    // RegAssetRemap.
                    //
                    // ONLY the hash, and only on the blend graph. Raiden's drawn objects keep the
                    // source model's hash and match_first_index, because the boss draws the same
                    // geometry and only the blend weights differ. Other characters generally need
                    // the index remapped too -- that is a second entry here, not a different class.
                    blendAssetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)}},
                        toModName_, ctx_.modTypeName().value_or(""), ctx_.version(), toVersion);

                    blendAssetAdapter_ = std::make_unique<RegPartEdit<>>(blendAssetRemap_.get());

                    // 'remaps' is deliberately left EMPTY. It exists to build a remapped *copy* of
                    // the graph, and this fix does not want one -- the copy GIMIFixer already made
                    // is the thing being turned into the remap, so the vb1 references are repointed
                    // in place and the graph is renamed by the edit list below.
                    //
                    // It also avoids a documented gap: on the remapped-copy path ResRegCollect
                    // looks parts up by id in the copy, and IniSectionGraph::deepcopy draws fresh
                    // ids, so the reference is silently never rewritten (see
                    // ResRegCollect::collectResourceNames' own comment).
                    blendCollect_.partPredicates = {{blendGraph, blendHashParts()}};

                    // The predicate reads the part's KVP colouring, which only exists when the
                    // collector is told to track keys. Only 'hash' is ever consulted.
                    blendCollect_.trackKeysIsGlobal = false;
                    blendCollect_.trackKeys = {{blendGraph, true}};
                    blendCollect_.keysToTrack = {{blendGraph, std::unordered_set<std::string>{IniKeywords::Hash}}};
                }

                // Accepts or rejects a part WHOLE, by whether its KVP colouring carries a 'hash'
                // naming Raiden's own Blend.buf.
                //
                // Deliberately not a sub-part window, unlike GIMIObjPartFilter's. The register this
                // collects (vb1) and the hash that qualifies it are in DIFFERENT sections -- the
                // hash sits on the TextureOverride root, the vb1 down in the CommandList that root
                // runs -- so there is no span within one part to narrow to, and windowing by the
                // hash's own order index selected nothing at all. The real question is "does this
                // part belong to the blend chain", which is a yes/no about the whole part.
                Collector::PartPredicate blendHashParts() {
                    return [this](const Collector::IterData& iterData) {
                        const Collector::OrderRanges nothing(std::vector<Collector::OrderRanges::Range>{});
                        if (iterData.colouring == nullptr) {
                            return nothing;
                        }

                        Hashes* hashes = ctx_.modTypeHashes();
                        if (hashes == nullptr) {
                            return nothing;
                        }

                        std::optional<Version> version = ctx_.version();

                        for (const auto& hashVal : iterData.colouring->getIndVals(IniKeywords::Hash)) {
                            std::optional<std::vector<std::string>> hashKeyRow =
                                hashes->getKey(hashVal.second, version,
                                                std::vector<std::optional<std::string>>{}, false);

                            if (hashKeyRow.has_value() && !hashKeyRow->empty() && hashKeyRow->back() == BlendHashKey) {
                                return Collector::OrderRanges::createFull();
                            }
                        }

                        return nothing;
                    };
                }

                // ---- head/body/dress register edits ----
                void buildObjEdits() {
                    // 1. Drop the mod's own calls into the external ORFix library. Both halves go:
                    //    the fix re-issues NNFix itself below, in the one place it actually belongs,
                    //    and ORFix has no equivalent re-issue.
                    auto isFixCall = [](long long, const std::string& value) {
                        return value == IniKeywords::ORFixPath || value == IniKeywords::NNFixPath;
                    };

                    removeFixCalls_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::Run, RegRemove<>::RemoveKeyCheck(isFixCall)}});

                    // 2. Re-issue NNFix immediately before every drawindexed, and once at the end of
                    //    any path that draws nothing. RegDelimitedAdd is built for exactly this rule
                    //    -- see its own note on why RegSurroundedAdd cannot express it.
                    addNNFix_ = std::make_unique<RegDelimitedAdd<>>(
                        RegDelimitedAdd<>::Additions{{IniKeywords::Run, IniKeywords::NNFixPath}},
                        RegDelimitedAdd<>::RegMap{{IniKeywords::DrawIndexed, {}}});

                    // 0. Rename the graph. GIMIFixer edits a deep COPY of what the parser found, so
                    //    renaming that copy is what turns it into the remapped mod -- the original
                    //    sections are untouched and come back via the appended source text. The
                    //    rename also rewrites every 'run =' pointing at a renamed section, so the
                    //    copy's internal wiring follows it (IniSectionGraph::rename).
                    //
                    //    buildAll fans one fixer out per target, so this fixer's own target is the
                    //    only name it ever has to produce -- which is why a rename func taking just
                    //    the old section name is enough.
                    const std::string toModName = toModName_;
                    renameGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& sectionName) {
                            return IniNamingTools::getRemapFixName(sectionName, toModName);
                        });

                    renameAdapter_ = std::make_unique<GraphPartEdit<>>(renameGraph_.get());

                    // Blend uses the BLEND naming convention (…RaidenBossRemapBlend), not the
                    // RemapFix one the drawn objects use. Without a rename of its own, the copied
                    // blend graph renders under the original's exact section names -- a verbatim
                    // duplicate of what the appended source text already contains.
                    renameBlendGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& sectionName) {
                            return IniNamingTools::getRemapBlendName(sectionName, toModName);
                        });

                    // The face's register swap -- a plain register edit, applied to every part of
                    // the face graph. That reach is the point for Raiden, whose face graph is a
                    // TextureOverride running a CommandList that binds a DIFFERENT texture per
                    // $swapvar branch: each branch is a part of its own, and each one needs the
                    // swap.
                    faceRegSwap_ = std::make_unique<RegRemap<>>(makeFaceRegSwap(FaceDiffuseReg, FaceLightMapReg));

                    renameBlendAdapter_ = std::make_unique<GraphPartEdit<>>(renameBlendGraph_.get());
                    faceSwapAdapter_ = std::make_unique<RegPartEdit<>>(faceRegSwap_.get());
                    removeAdapter_ = std::make_unique<RegPartEdit<>>(removeFixCalls_.get());
                    addAdapter_ = std::make_unique<GraphPartEdit<>>(addNNFix_.get());

                    // head/body/dress are told apart by a match_first_index following the shared
                    // 'ib' hash, so 'ib' is the one index-qualified hash type here.
                    objFilter_ = std::make_unique<ObjFilter>(ctx_.modTypeHashes(), ctx_.modTypeIndices(),
                                                              ObjFilter::KeySet{IbHashKey}, ctx_.version());

                    ObjGroupEdit::IniEdits iniEdits;
                    for (const ModObj& modObj : RaidenDrawnObjs) {
                        iniEdits.edits[modObj] = {renameAdapter_.get(), removeAdapter_.get(), addAdapter_.get()};

                        // ONE FILTER PER EDIT, in the same order as 'edits'. The rename is
                        // deliberately UNFILTERED (an empty PartFilter means "the whole part"): it
                        // renames whole sections, so restricting it to one mod object's KVP window
                        // would be meaningless -- and, since the window is read off the hash/index
                        // the rename does not touch, harmless either way. The two register edits
                        // are the ones that must stay inside their own object's window; see
                        // GIMIObjPartFilter.
                        iniEdits.keyFilters[modObj] = {ObjGroupEdit::PartFilter{},
                                                       objFilter_->filter(modObj),
                                                       objFilter_->filter(modObj)};

                        // Asked for rather than restated, so this can never drift out of step with
                        // what the filter actually reads.
                        iniEdits.keysToTrack[modObj] = objFilter_->keysToTrack();
                        iniEdits.trackKeys[modObj] = true;
                    }

                    // The blend graph gets its own entry: a different edit (the asset remap) and
                    // no key window, since a blend section holds exactly one mod object's worth of
                    // registers -- there is nothing to tell apart within it.
                    iniEdits.edits[RaidenBlendObj] = {renameBlendAdapter_.get(), blendAssetAdapter_.get()};
                    iniEdits.keyFilters[RaidenBlendObj] = {ObjGroupEdit::PartFilter{}, ObjGroupEdit::PartFilter{}};
                    iniEdits.trackKeys[RaidenBlendObj] = false;

                    // The face gets the RENAME and the register swap. Its hash is deliberately not
                    // remapped: RaidenBoss has no tex_face_diffuse row of its own and does not need
                    // one, the face being the same.
                    //
                    // Without the rename the copied face graph renders under the original's exact
                    // section names and is a verbatim duplicate of the source text -- the same trap
                    // the blend fell into. Neither edit wants a key window: the rename works on
                    // whole sections, and the swap is wanted on every register it finds.
                    iniEdits.edits[FaceObj] = {renameAdapter_.get(), faceSwapAdapter_.get()};
                    iniEdits.keyFilters[FaceObj] = {ObjGroupEdit::PartFilter{}, ObjGroupEdit::PartFilter{}};
                    iniEdits.trackKeys[FaceObj] = false;

                    objEdits_ = ObjGroupEdit({iniEdits}, false);
                }

                IniFileFixContext ctx_;

                std::unique_ptr<RegRemap<>> faceRegSwap_;
                std::unique_ptr<RegPartEdit<>> faceSwapAdapter_;

                std::unique_ptr<VGRemapBlendReplace<>> blendReplace_;
                std::unique_ptr<RegAssetRemap<>> blendAssetRemap_;
                std::unique_ptr<RegPartEdit<>> blendAssetAdapter_;
                Collector blendCollect_;

                std::unique_ptr<RegRemove<>> removeFixCalls_;
                std::unique_ptr<RegDelimitedAdd<>> addNNFix_;
                std::string toModName_;

                std::unique_ptr<GraphRename<>> renameGraph_;
                std::unique_ptr<GraphPartEdit<>> renameAdapter_;
                std::unique_ptr<GraphRename<>> renameBlendGraph_;
                std::unique_ptr<GraphPartEdit<>> renameBlendAdapter_;
                std::unique_ptr<ObjFilter> objFilter_;
                std::unique_ptr<RegPartEdit<>> removeAdapter_;
                std::unique_ptr<GraphPartEdit<>> addAdapter_;
                ObjGroupEdit objEdits_;
        };
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::raiden6_1() {
        return [](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<RaidenFixerImpl>(parser, toModName, modTypeId);
        };
    }


    IniFixBuilder::Factory RaidenFixer::v6_1() {
        return IniFixBuilderFuncs::raiden6_1();
    }
}
