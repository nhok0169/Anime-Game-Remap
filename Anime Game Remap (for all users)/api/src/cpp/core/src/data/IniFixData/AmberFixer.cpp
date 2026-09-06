#include "AGRemapCore/data/IniFixData/AmberFixer.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIObjPartFilter.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegDelimitedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegFillMissing.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/VGRemapBlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {
    namespace {
        // The .ini register naming a resource's file, plus the identity conversions to and from a
        // file path -- see RaidenFixer's own copy for why this is spelled out per fixer.
        BaseResEdit<>::ResEditConfig makeAmberResEditConfig() {
            return BaseResEdit<>::ResEditConfig{
                IniKeywords::Filename,
                [](const std::string& value) { return value; },
                [](const std::string& file) { return file; }
            };
        }

        using Fixer = GIMIFixer<>;
        using ModObj = Fixer::ModObj;
        using ObjGroupEdit = GraphGroupEdit<>;
        using GraphId = BaseIniGraphGroupEdit<>::GraphId;
        using Collector = ResRegCollect<>;
        using ObjFilter = GIMIObjPartFilter<>;

        const std::string IbHashKey = "ib";
        const std::string BlendHashKey = "blend_vb";

        // The register a mod object's section points its Blend.buf through.
        const std::string BlendReg = "vb1";

        // What a re-issued draw call draws. 3dmigoto works the count out itself.
        const std::string DrawIndexedAuto = "auto";

        // The objects that actually draw. head/body share one 'ib' and are told apart by their
        // match_first_index, which is what makes GIMIObjPartFilter necessary for them.
        const std::vector<ModObj> AmberDrawnObjs = {{"", "head"}, {"", "body"}};

        // The shared draw call they run into -- the section carrying the 'ib' hash and no
        // match_first_index. See AmberParser for how the classifier separates it.
        const ModObj AmberIbObj{"", "ib"};
        const ModObj AmberBlendObj{"", "blend"};
        const ModObj AmberPositionObj{"", "position"};
        const ModObj AmberTexcoordObj{"", "texcoord"};

        // VertexLimitRaise and the face diffuse -- a hash swap and nothing else. See AmberParser
        // for why several hash types share this one mod object.
        const ModObj AmberOtherObj{"", "other"};

        /**
         * Amber's fix. The same skeleton as RaidenFixer -- read that one first -- with three
         * differences, all of them because Amber remaps onto a CN skin rather than onto a boss that
         * shares her geometry:
         *
         *  1. the hash AND the match_first_index are remapped, not just the blend's hash: the two
         *     models are genuinely different, so every asset value naming Amber has to become the
         *     one naming AmberCN
         *  2. the shared 'drawindexed' is taken off ("", "ib") and re-issued per drawn object,
         *     because each object now draws its own remapped geometry
         *  3. position and texcoord are remapped too
         */
        class AmberFixerImpl: public Fixer {
            public:
                AmberFixerImpl(BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId):
                    Fixer(parser, nullptr, {},
                           toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                             : std::optional<std::vector<std::string>>({toModName}),
                           nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName) {
                    this->setCtx(&ctx_);

                    buildBlendCollector();
                    buildEdits();

                    this->graphGroupEdits = {&blendCollect_, &objIndexEdits_, &objEdits_};

                    // NOTHING IS HIDDEN -- deliberately, and this is where Amber parts company
                    // with Raiden.
                    //
                    // Raiden's 6.1 fix hides her head/body/dress because the remap keeps the SOURCE
                    // model's hash and match_first_index: the boss draws the same geometry, only the
                    // blend weights differ, so the original and the remap trigger on exactly the same
                    // draw and would both fire. Hiding the original is what stops the double draw.
                    //
                    // Amber remaps onto a CN skin, which is a genuinely different model with
                    // different hashes. The original sections trigger on Amber and the remapped ones
                    // on AmberCN, so they can never both fire -- and hiding the originals would break
                    // the mod on the character it was actually built for. The pure-Python fix leaves
                    // them alone for the same reason.
                    //
                    // The rule: hide only when the remap keeps the source's hash and index.
                }

            protected:
                // GIMIFixer's own hands every group edit a nullptr .ini file, which stops
                // ResRegCollect ever building anything -- see RaidenFixer for the full story.
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

                void buildBlendCollector() {
                    const GraphId blendGraph(0, AmberBlendObj.first, AmberBlendObj.second);
                    const GraphId blendResGraph(0, AmberBlendObj.first, AmberBlendObj.second + "RemapBlend");

                    IniFile* iniFile = ctx_.getIniFile();
                    const std::optional<Version> toVersion = (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;

                    blendReplace_ = std::make_unique<VGRemapBlendReplace<>>(
                        blendResGraph, makeAmberResEditConfig(), ctx_.modType(), ctx_.version(), toVersion);

                    blendCollect_.srcRegs = {{blendGraph, BlendReg}};
                    blendCollect_.resEdits = {{IniKeywords::Blend, blendReplace_.get()}};
                    blendCollect_.partPredicates = {{blendGraph, blendHashParts()}};

                    blendCollect_.trackKeysIsGlobal = false;
                    blendCollect_.trackKeys = {{blendGraph, true}};
                    blendCollect_.keysToTrack = {{blendGraph, std::unordered_set<std::string>{IniKeywords::Hash}}};
                }

                // Whole-part accept/reject, not a sub-part window -- the qualifying hash and the
                // vb1 are in different sections. See RaidenFixer for why that matters.
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

                /**
                 * One match_first_index rewrite per drawn mod object, looked up FORWARD.
                 *
                 * An index's meaning is its mod object, and this fixer knows which object's graph
                 * each edit runs over -- so the target value is just Indices.get({target mod,
                 * component, object}), with no reference to the old value at all. That is both
                 * unambiguous (unlike reverse-looking-up "0") and per-object, which is why this is
                 * a map of edits rather than the single shared one the hash uses.
                 *
                 * RegNewVals' addNewKVPs stays at its false default: a section with no
                 * match_first_index of its own must not sprout one.
                 */
                void buildIndexEdits(const std::optional<Version>& toVersion) {
                    Indices* indices = ctx_.modTypeIndices();
                    if (indices == nullptr) {
                        return;
                    }

                    for (const ModObj& modObj : AmberDrawnObjs) {
                        std::optional<std::string> target =
                            indices->get({toModName_, modObj.first, modObj.second}, toVersion, false);

                        // No row for this object on the target is a real answer -- leave the index
                        // alone rather than writing a sentinel into a numeric field.
                        if (!target.has_value()) {
                            continue;
                        }

                        auto edit = std::make_unique<RegNewVals<>>(
                            std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>>{
                                {IniKeywords::MatchFirstIndex,
                                 RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(*target))}});

                        indexAdapters_[modObj] = std::make_unique<RegPartEdit<>>(edit.get());
                        indexEdits_.push_back(std::move(edit));
                    }

                    // A GROUP EDIT OF ITS OWN, and that is the whole point of this function.
                    //
                    // The index rewrite is the one edit here that must not stray outside its own
                    // mod object's KVP window: head and body share an 'ib' hash and are told apart
                    // only by the match_first_index this edit overwrites, so a stray write puts
                    // body's vertex range onto head. GIMIObjPartFilter builds that window.
                    //
                    // But GraphGroupEdit indexes keyFilters PER SAME-KIND RUN, restarting at 0 for
                    // each run, and hands one flat filter list to every run of a graph -- so within
                    // a mixed list a filter cannot be aimed at one edit without also landing on
                    // whichever edit sits at the same offset in the other runs. Giving the index
                    // edit a group edit to itself makes its run exactly one edit long, where
                    // index 0 unambiguously means "the index edit" and nothing else.
                    //
                    // It also has to run BEFORE the asset remap in objEdits_. The filter recognises
                    // a part by AMBER's own hash and index, the remap rewrites both to AmberCN's,
                    // and the register loop refreshes the KVP colouring after every edit -- so run
                    // the other way round the window comes back empty and this silently does
                    // nothing. That is exactly how the NNFix placement went missing.
                    ObjGroupEdit::IniEdits indexIniEdits;
                    for (const auto& indexEntry : indexAdapters_) {
                        const ModObj& modObj = indexEntry.first;

                        indexIniEdits.edits[modObj] = {indexEntry.second.get()};
                        indexIniEdits.keyFilters[modObj] = {objFilter_->filter(modObj)};

                        // Asked for rather than restated, so this can never drift out of step with
                        // what the filter actually reads.
                        indexIniEdits.keysToTrack[modObj] = objFilter_->keysToTrack();
                        indexIniEdits.trackKeys[modObj] = true;
                    }

                    objIndexEdits_ = ObjGroupEdit({indexIniEdits}, false);
                }

                // ---- every other graph and register edit ----
                void buildEdits() {
                    IniFile* iniFile = ctx_.getIniFile();
                    const std::optional<Version> toVersion =
                        (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;

                    // head and body are told apart by a match_first_index following the shared 'ib'
                    // hash, so 'ib' is the one index-qualified hash type here. Built up here because
                    // buildIndexEdits, called below, is the only thing that reads it.
                    objFilter_ = std::make_unique<ObjFilter>(ctx_.modTypeHashes(), ctx_.modTypeIndices(),
                                                              ObjFilter::KeySet{IbHashKey}, ctx_.version());

                    // 1. Drop the mod's own calls into the external ORFix library. Both halves go:
                    //    the fix re-issues NNFix itself below, in the one place it belongs, and
                    //    ORFix has no equivalent re-issue.
                    auto isFixCall = [](long long, const std::string& value) {
                        return value == IniKeywords::ORFixPath || value == IniKeywords::NNFixPath;
                    };

                    removeFixCalls_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::Run, RegRemove<>::RemoveKeyCheck(isFixCall)}});

                    // 2. Take the shared draw call off ("", "ib"). Each remapped object now draws
                    //    its own geometry, so the single drawindexed they all ran into is wrong.
                    removeDrawIndexed_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::DrawIndexed, std::nullopt}});

                    // 3. Re-issue that draw call per object. RegFillMissing only touches the paths
                    //    actually MISSING the register, so a path that already draws keeps the draw
                    //    call it shipped with.
                    fillDrawIndexed_ = std::make_unique<RegFillMissing<>>(
                        IniKeywords::DrawIndexed,
                        RegFillMissing<>::makeFillMissing(IniKeywords::DrawIndexed, DrawIndexedAuto));

                    // 4. Re-issue NNFix immediately before every drawindexed, and once at the end of
                    //    any path that draws nothing -- the same rule as Raiden's 6.1 fix, and the
                    //    reason RegDelimitedAdd exists.
                    addNNFix_ = std::make_unique<RegDelimitedAdd<>>(
                        std::pair<std::string, std::string>{IniKeywords::Run, IniKeywords::NNFixPath},
                        RegDelimitedAdd<>::RegMap{{IniKeywords::DrawIndexed, {}}});

                    // 0. Rename the copy GIMIFixer made -- that is what turns it into the remapped
                    //    mod, the originals being untouched and coming back via the appended source
                    //    text. See RaidenFixer for the full story, including why the blend needs a
                    //    rename of its own using the blend naming convention.
                    const std::string toModName = toModName_;
                    renameGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& sectionName) {
                            return IniNamingTools::getRemapFixName(sectionName, toModName);
                        });

                    // Each resource kind has a naming convention of its own, and using the
                    // generic RemapFix one for all of them produces names no other tool recognises
                    // (…AmberIBAmberCNRemapFix where the convention is …AmberAmberCNRemapIB).
                    // IniNamingTools has one function per kind; there is nothing to invent here.
                    renameBlendGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& sectionName) {
                            return IniNamingTools::getRemapBlendName(sectionName, toModName);
                        });

                    renamePositionGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& sectionName) {
                            return IniNamingTools::getRemapPositionName(sectionName, toModName);
                        });

                    renameTexcoordGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& sectionName) {
                            return IniNamingTools::getRemapTexcoordName(sectionName, toModName);
                        });

                    renameIbGraph_ = std::make_unique<GraphRename<>>(
                        [toModName](const std::string& sectionName) {
                            return IniNamingTools::getRemapIbName(sectionName, toModName);
                        });

                    // The HASH only. Amber and AmberCN are different models, so a section still
                    // naming Amber's own hash never triggers on the CN skin -- and the
                    // reverse-then-forward lookup RegAssetRemap does is exactly right here, because
                    // the old value is what says which KIND of hash it is.
                    //
                    // The index is deliberately NOT here; see buildIndexEdits and RegAssetRemap's
                    // own warning. Reverse-looking-up "0" is ambiguous (it is every character's head
                    // index) and it simply fails, writing "IndexNotFound" into a numeric field.
                    assetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash,
                             RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes(), IniKeywords::HashNotFound)}},
                        toModName_, ctx_.version(), toVersion);

                    renameAdapter_ = std::make_unique<GraphPartEdit<>>(renameGraph_.get());
                    renameBlendAdapter_ = std::make_unique<GraphPartEdit<>>(renameBlendGraph_.get());
                    renamePositionAdapter_ = std::make_unique<GraphPartEdit<>>(renamePositionGraph_.get());
                    renameTexcoordAdapter_ = std::make_unique<GraphPartEdit<>>(renameTexcoordGraph_.get());
                    renameIbAdapter_ = std::make_unique<GraphPartEdit<>>(renameIbGraph_.get());
                    assetAdapter_ = std::make_unique<RegPartEdit<>>(assetRemap_.get());
                    removeFixCallsAdapter_ = std::make_unique<RegPartEdit<>>(removeFixCalls_.get());
                    fillAdapter_ = std::make_unique<GraphPartEdit<>>(fillDrawIndexed_.get());
                    addNNFixAdapter_ = std::make_unique<GraphPartEdit<>>(addNNFix_.get());
                    removeDrawIndexedAdapter_ = std::make_unique<RegPartEdit<>>(removeDrawIndexed_.get());

                    buildIndexEdits(toVersion);

                    ObjGroupEdit::IniEdits iniEdits;

                    // ORDER IS LOAD-BEARING here, twice over:
                    //
                    //  * removeFixCalls has to precede addNNFix, or the removal strips the very
                    //    NNFix call the addition just made
                    //  * the drawindexed fill has to precede addNNFix, so NNFix lands in front of
                    //    the draw call the fill added and not only the ones already there
                    //
                    // No key filters on any of these, which is safe in a way it would NOT be for the
                    // index rewrite: none of them writes a value another object's window is read
                    // from, and every one of them is wanted everywhere in the graph it runs over.
                    // The one edit that does need a window has a group edit to itself -- see
                    // buildIndexEdits, and GIMIObjPartFilter for why filters are usually mandatory.
                    for (const ModObj& modObj : AmberDrawnObjs) {
                        iniEdits.edits[modObj] = {removeFixCallsAdapter_.get(), renameAdapter_.get(),
                                                   fillAdapter_.get(), addNNFixAdapter_.get(),
                                                   assetAdapter_.get()};
                        iniEdits.trackKeys[modObj] = false;
                    }

                    // No key window on any of these either: each holds one mod object's worth of
                    // registers, so there is nothing within them to tell apart. Every one gets the
                    // rename belonging to ITS OWN kind plus the shared hash remap.
                    iniEdits.edits[AmberIbObj] = {renameIbAdapter_.get(), assetAdapter_.get(),
                                                   removeDrawIndexedAdapter_.get()};
                    iniEdits.trackKeys[AmberIbObj] = false;

                    iniEdits.edits[AmberBlendObj] = {renameBlendAdapter_.get(), assetAdapter_.get()};
                    iniEdits.trackKeys[AmberBlendObj] = false;

                    iniEdits.edits[AmberPositionObj] = {renamePositionAdapter_.get(), assetAdapter_.get()};
                    iniEdits.trackKeys[AmberPositionObj] = false;

                    iniEdits.edits[AmberTexcoordObj] = {renameTexcoordAdapter_.get(), assetAdapter_.get()};
                    iniEdits.trackKeys[AmberTexcoordObj] = false;

                    // ("", "other") is the one that DOES use the generic RemapFix name: it holds the
                    // sections that are not a resource of any particular kind, and the fix has
                    // nothing to say about them beyond swapping the hash.
                    iniEdits.edits[AmberOtherObj] = {renameAdapter_.get(), assetAdapter_.get()};
                    iniEdits.trackKeys[AmberOtherObj] = false;

                    objEdits_ = ObjGroupEdit({iniEdits}, false);
                }

                IniFileFixContext ctx_;
                std::string toModName_;

                std::unique_ptr<VGRemapBlendReplace<>> blendReplace_;
                Collector blendCollect_;

                std::unique_ptr<GraphRename<>> renameGraph_;
                std::unique_ptr<GraphRename<>> renameBlendGraph_;
                std::unique_ptr<GraphRename<>> renamePositionGraph_;
                std::unique_ptr<GraphRename<>> renameTexcoordGraph_;
                std::unique_ptr<GraphRename<>> renameIbGraph_;
                std::unique_ptr<RegAssetRemap<>> assetRemap_;
                std::unique_ptr<RegRemove<>> removeFixCalls_;
                std::unique_ptr<RegFillMissing<>> fillDrawIndexed_;
                std::unique_ptr<RegDelimitedAdd<>> addNNFix_;
                std::unique_ptr<RegRemove<>> removeDrawIndexed_;

                std::unique_ptr<GraphPartEdit<>> renameAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameBlendAdapter_;
                std::unique_ptr<GraphPartEdit<>> renamePositionAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameTexcoordAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameIbAdapter_;
                std::unique_ptr<RegPartEdit<>> assetAdapter_;
                std::unique_ptr<RegPartEdit<>> removeFixCallsAdapter_;
                std::unique_ptr<GraphPartEdit<>> fillAdapter_;
                std::unique_ptr<GraphPartEdit<>> addNNFixAdapter_;
                std::unique_ptr<RegPartEdit<>> removeDrawIndexedAdapter_;

                std::unique_ptr<ObjFilter> objFilter_;

                std::vector<std::unique_ptr<RegNewVals<>>> indexEdits_;
                tsl::ordered_map<ModObj, std::unique_ptr<RegPartEdit<>>, Fixer::ModObjHash> indexAdapters_;

                ObjGroupEdit objIndexEdits_;
                ObjGroupEdit objEdits_;
        };
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::amber6_1() {
        return [](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<AmberFixerImpl>(parser, toModName, modTypeId);
        };
    }


    IniFixBuilder::Factory AmberFixer::v6_1() {
        return IniFixBuilderFuncs::amber6_1();
    }
}
