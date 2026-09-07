#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
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
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEditorEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/VGRemapBlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {
    namespace {
        // The .ini register naming a resource's file, plus the identity conversions to and from a
        // file path -- see RaidenFixer's own copy for why this is spelled out per fixer family.
        BaseResEdit<>::ResEditConfig makeCharResEditConfig() {
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

        // NOT named 'GroupRemap': GIMIFixer already inherits an alias of that name, and inside this
        // class the inherited one would win -- the same trap RaidenFixer's ObjGroupEdit documents.
        using ObjGroupRemap = GraphGroupRemap<>;

        const std::string IbHashKey = "ib";
        const std::string BlendHashKey = "blend_vb";

        // The register a mod object's section points its Blend.buf through.
        const std::string BlendReg = "vb1";

        // What a re-issued draw call draws. 3dmigoto works the count out itself.
        const std::string DrawIndexedAuto = "auto";

        // The mod objects every character of this shape has, whatever its drawn objects are.
        // The DRAWN ones are per character and arrive through GIMICharFixerConfig::drawnObjs.
        //
        // ("", "ib") is the shared draw call the drawn objects run into -- the section carrying the
        // 'ib' hash and no match_first_index. See GIMICharParser for how the classifier separates
        // it from the drawn objects, which carry the same hash WITH an index.
        const ModObj IbObj{"", "ib"};
        const ModObj BlendObj{"", "blend"};
        const ModObj PositionObj{"", "position"};
        const ModObj TexcoordObj{"", "texcoord"};

        // VertexLimitRaise -- a hash swap and nothing else.
        const ModObj OtherObj{"", "other"};

        // The face's own diffuse, which used to sit in OtherObj alongside the above. It has a mod
        // object of its own so the fix can reach the registers its textures hang off -- see the
        // swap below.
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
        //
        // The register names themselves come from GIMICharFixerConfig, since they are per
        // character in principle even though no character has differed yet.

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
         * The fix for a character remapped onto a genuinely DIFFERENT model -- a CN skin, or a
         * skin's base character. The same skeleton as RaidenFixer -- read that one first -- with
         * three differences, all of them because the target does not share the source's geometry:
         *
         *  1. the hash AND the match_first_index are remapped, not just the blend's hash: the two
         *     models are genuinely different, so every asset value naming the source has to become
         *     the one naming the target
         *  2. optionally (GIMICharFixerConfig::moveDrawIndexed) the shared 'drawindexed' is taken
         *     off ("", "ib") and re-issued per drawn object. Amber needs that; Mona and Rosaria
         *     keep the draw call exactly where it is
         *  3. position and texcoord are remapped too
         */
        class GIMICharFixerImpl: public Fixer {
            public:
                GIMICharFixerImpl(BaseIniParser<>* parser, const std::string& toModName,
                                   std::optional<int> modTypeId, GIMICharFixerConfig config):
                    Fixer(parser, nullptr, {},
                           toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                             : std::optional<std::vector<std::string>>({toModName}),
                           nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr, modTypeId), toModName_(toModName),
                    config_(std::move(config)) {
                    this->setCtx(&ctx_);

                    buildObjMap();
                    buildBlendCollector();
                    buildTexEdits();
                    buildEdits();

                    // THE SPLIT GOES FIRST, and everything after it is keyed by the TARGET's object
                    // names -- which is the whole reason this ordering is not arbitrary. Once
                    // GraphGroupRemap has run, a graph that arrived as the source's "body" is
                    // sitting under the target's "dress", so the index rewrite, the asset remap and
                    // the register edits can all be written against the target without knowing a
                    // split happened at all.
                    this->graphGroupEdits.clear();
                    if (objSplitRemap_ != nullptr) {
                        this->graphGroupEdits.push_back(objSplitRemap_.get());
                    }

                    this->graphGroupEdits.push_back(&blendCollect_);
                    for (auto& collect : texCollects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }

                    this->graphGroupEdits.push_back(&objIndexEdits_);
                    this->graphGroupEdits.push_back(&objEdits_);

                    // ALMOST NOTHING IS HIDDEN -- deliberately, and this is where this shape
                    // parts company with Raiden's.
                    //
                    // Raiden's 6.1 fix hides her head/body/dress because the remap keeps the SOURCE
                    // model's hash and match_first_index: the boss draws the same geometry, only the
                    // blend weights differ, so the original and the remap trigger on exactly the same
                    // draw and would both fire. Hiding the original is what stops the double draw.
                    //
                    // These characters remap onto a genuinely different model with different
                    // hashes. The original sections trigger on the source and the remapped ones on
                    // the target, so they can never both fire -- and hiding the originals would
                    // break the mod on the character it was actually built for. The pure-Python fix
                    // leaves them alone for the same reason.
                    //
                    // The rule: hide only when the remap keeps the source's hash and index.
                    //
                    // The FACE is the one object here that meets it, so it is the one thing hidden.
                    // Every CN pair so far shares its tex_face_diffuse hash outright (Amber/AmberCN
                    // 1d064079, Mona/MonaCN 8e116301, Rosaria/RosariaCN 2abd61ee -- the skin reuses
                    // the face), so the original face section and the remapped one trigger on
                    // exactly the same draw.
                    //
                    // Left in, the original would bind the diffuse to ps-t0 while the remapped one
                    // binds it to ps-t1, and the face would end up with a diffuse in BOTH slots --
                    // the lightmap slot included. That is worse than the bug being fixed, and not
                    // something to leave to whichever section the game happens to apply last.
                    // Hiding the original makes the swapped binding the only answer, and costs
                    // nothing on the source character, since the remapped section carries her own
                    // hash.
                    this->hiddenModObjs.insert(FaceObj);
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
                // ---- which of the source's objects becomes which of the target's ----
                //
                // One entry in objMap_ per REMAP, not per object: a split source appears once per
                // target it becomes, and a merge puts several sources against one target. That is
                // the shape every later step reads, so this is the only place the difference
                // between "same objects on both sides" and a genuine split is handled.
                void buildObjMap() {
                    for (const std::string& obj : config_.drawnObjs) {
                        drawnObjs_.emplace_back("", obj);
                    }

                    if (config_.objSplits.empty()) {
                        // One-to-one -- every object keeps its own name, and no graph is copied.
                        targetObjs_ = drawnObjs_;
                        for (const ModObj& modObj : drawnObjs_) {
                            objMap_.emplace_back(modObj, modObj);
                        }

                        return;
                    }

                    ObjGroupRemap::RemapList remap;

                    for (const auto& split : config_.objSplits) {
                        const ModObj srcObj("", split.first);
                        std::vector<ObjGroupRemap::RemapTarget> targets;

                        for (const std::string& toObj : split.second) {
                            const ModObj tgtObj("", toObj);
                            objMap_.emplace_back(srcObj, tgtObj);

                            if (std::find(targetObjs_.begin(), targetObjs_.end(), tgtObj) == targetObjs_.end()) {
                                targetObjs_.push_back(tgtObj);
                            }

                            targets.emplace_back(GraphId(0, tgtObj.first, tgtObj.second));
                        }

                        remap.emplace_back(GraphId(0, srcObj.first, srcObj.second), std::move(targets));
                    }

                    // An empty renameFunc on every target, which is what asks copyGraph for the
                    // default -- IniNamingTools::getObjRemapFixName, the object swap plus the same
                    // getRemapFixName the one-to-one path uses. Writing one by hand here would only
                    // be a way to get it subtly wrong.
                    objSplitRemap_ = std::make_unique<ObjGroupRemap>(std::move(remap));
                }

                // ---- textures this fix rewrites ----
                //
                // The same three-part shape as the blend collector -- read buildBlendCollector
                // first, and TexEditorReplace's own note on why the plain TexReplace silently
                // writes no file at all when driven from C++.
                //
                // One collector per edit rather than one shared: each names a resource graph of its
                // own, and a shared one would have the second edit overwrite the first's resources.
                void buildTexEdits() {
                    for (const GIMICharFixerConfig::TexEdit& texEdit : config_.texEdits) {
                        const GraphId srcGraph(0, "", texEdit.obj);

                        // A DISTINCT mod object from the source graph, exactly as the blend needs:
                        // pointing it at srcGraph would have the resource overwrite the graph it was
                        // collected from.
                        const GraphId resGraph(0, "", texEdit.obj + "RemapTex");

                        auto replace = std::make_unique<TexEditorReplace<>>(
                            resGraph, TexEditor({texEdit.filter}, texEdit.compress), makeCharResEditConfig(),
                            "resourceRemapTexEdit", texEdit.name);

                        auto collect = std::make_unique<Collector>();
                        collect->srcRegs = {{srcGraph, texEdit.reg}};
                        collect->resEdits = {{texEdit.obj, replace.get()}};

                        texReplaces_.push_back(std::move(replace));
                        texCollects_.push_back(std::move(collect));
                    }
                }

                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                void buildBlendCollector() {
                    const GraphId blendGraph(0, BlendObj.first, BlendObj.second);
                    const GraphId blendResGraph(0, BlendObj.first, BlendObj.second + "RemapBlend");

                    IniFile* iniFile = ctx_.getIniFile();
                    const std::optional<Version> toVersion = (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;

                    blendReplace_ = std::make_unique<VGRemapBlendReplace<>>(
                        blendResGraph, makeCharResEditConfig(), ctx_.modType(), ctx_.version(), toVersion);

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

                    for (const auto& entry : objMap_) {
                        const ModObj& srcObj = entry.first;
                        const ModObj& modObj = entry.second;

                        std::optional<std::string> target =
                            indices->get({toModName_, modObj.first, modObj.second}, toVersion, false);

                        // No row for this object on the target is a real answer -- leave the index
                        // alone rather than writing a sentinel into a numeric field.
                        if (!target.has_value()) {
                            continue;
                        }

                        // The window is read off the SOURCE's own hash and index, so a split copy
                        // asks about the object it was copied FROM. Jean's body becomes JeanSea's
                        // dress; the copy still carries Jean's body index, and asking about a
                        // "dress" Jean never had would come back empty.
                        indexSrcObjs_[modObj] = srcObj;

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
                    // a part by the SOURCE's own hash and index, the remap rewrites both to the
                    // target's, and the register loop refreshes the KVP colouring after every edit --
                    // so run
                    // the other way round the window comes back empty and this silently does
                    // nothing. That is exactly how the NNFix placement went missing.
                    ObjGroupEdit::IniEdits indexIniEdits;
                    for (const auto& indexEntry : indexAdapters_) {
                        const ModObj& modObj = indexEntry.first;

                        auto srcEntry = indexSrcObjs_.find(modObj);
                        const ModObj& srcObj = (srcEntry == indexSrcObjs_.end()) ? modObj : srcEntry->second;

                        indexIniEdits.edits[modObj] = {indexEntry.second.get()};
                        indexIniEdits.keyFilters[modObj] = {objFilter_->filter(srcObj)};

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

                    // The HASH only. Source and target are different models here, so a section
                    // still naming the source's own hash never triggers on the target -- and the
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
                        toModName_, ctx_.modTypeName().value_or(""), ctx_.version(), toVersion);

                    // The face's register swap -- a plain register edit, applied to every part of
                    // the face graph.
                    faceRegSwap_ = std::make_unique<RegRemap<>>(
                        makeFaceRegSwap(config_.faceDiffuseReg, config_.faceLightMapReg));

                    renameAdapter_ = std::make_unique<GraphPartEdit<>>(renameGraph_.get());
                    renameBlendAdapter_ = std::make_unique<GraphPartEdit<>>(renameBlendGraph_.get());
                    renamePositionAdapter_ = std::make_unique<GraphPartEdit<>>(renamePositionGraph_.get());
                    renameTexcoordAdapter_ = std::make_unique<GraphPartEdit<>>(renameTexcoordGraph_.get());
                    renameIbAdapter_ = std::make_unique<GraphPartEdit<>>(renameIbGraph_.get());
                    assetAdapter_ = std::make_unique<RegPartEdit<>>(assetRemap_.get());
                    faceSwapAdapter_ = std::make_unique<RegPartEdit<>>(faceRegSwap_.get());
                    removeFixCallsAdapter_ = std::make_unique<RegPartEdit<>>(removeFixCalls_.get());
                    fillAdapter_ = std::make_unique<GraphPartEdit<>>(fillDrawIndexed_.get());
                    addNNFixAdapter_ = std::make_unique<GraphPartEdit<>>(addNNFix_.get());
                    removeDrawIndexedAdapter_ = std::make_unique<RegPartEdit<>>(removeDrawIndexed_.get());

                    buildIndexEdits(toVersion);

                    // Registers forced onto one target object. addNewKVPs stays at its false
                    // default, so a part with no such register does not grow one -- this replaces a
                    // value the split copied in, it does not invent one.
                    for (const auto& entry : config_.objNewRegVals) {
                        const ModObj modObj("", entry.first);

                        std::vector<std::pair<std::string, RegNewVals<>::NewValSpec>> vals;
                        for (const auto& kvp : entry.second) {
                            vals.emplace_back(kvp.first, RegNewVals<>::NewValSpec(RegNewVals<>::NewVal(kvp.second)));
                        }

                        auto edit = std::make_unique<RegNewVals<>>(std::move(vals));
                        newRegValAdapters_[modObj] = std::make_unique<RegPartEdit<>>(edit.get());
                        newRegVals_[modObj] = std::move(edit);
                    }

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
                    for (const ModObj& modObj : targetObjs_) {
                        std::vector<ObjGroupEdit::PartEdit*> edits = {removeFixCallsAdapter_.get()};

                        // The rename belongs to whichever of the two actually did it. With a split,
                        // GraphGroupRemap::copyGraph already renamed every section as it copied --
                        // through IniNamingTools::getObjRemapFixName, which is the same
                        // getRemapFixName underneath plus the object swap. Renaming again here
                        // would append a second RemapFix suffix.
                        if (objSplitRemap_ == nullptr) {
                            edits.push_back(renameAdapter_.get());
                        }

                        // Only when this character's fix moves the draw call onto its drawn objects.
                        // Without it there is no drawindexed in these parts at all, RegDelimitedAdd
                        // finds no delimiter, and NNFix lands once at the end of the path -- which
                        // is exactly what the pure-Python Mona/Rosaria output shows.
                        if (config_.moveDrawIndexed) {
                            edits.push_back(fillAdapter_.get());
                        }

                        edits.push_back(addNNFixAdapter_.get());
                        edits.push_back(assetAdapter_.get());

                        // Forced register values come last, so nothing above can overwrite them.
                        auto forced = newRegValAdapters_.find(modObj);
                        if (forced != newRegValAdapters_.end()) {
                            edits.push_back(forced->second.get());
                        }

                        iniEdits.edits[modObj] = std::move(edits);
                        iniEdits.trackKeys[modObj] = false;
                    }

                    // No key window on any of these either: each holds one mod object's worth of
                    // registers, so there is nothing within them to tell apart. Every one gets the
                    // rename belonging to ITS OWN kind plus the shared hash remap.
                    std::vector<ObjGroupEdit::PartEdit*> ibEdits = {renameIbAdapter_.get(), assetAdapter_.get()};
                    if (config_.moveDrawIndexed) {
                        ibEdits.push_back(removeDrawIndexedAdapter_.get());
                    }

                    iniEdits.edits[IbObj] = std::move(ibEdits);
                    iniEdits.trackKeys[IbObj] = false;

                    iniEdits.edits[BlendObj] = {renameBlendAdapter_.get(), assetAdapter_.get()};
                    iniEdits.trackKeys[BlendObj] = false;

                    iniEdits.edits[PositionObj] = {renamePositionAdapter_.get(), assetAdapter_.get()};
                    iniEdits.trackKeys[PositionObj] = false;

                    iniEdits.edits[TexcoordObj] = {renameTexcoordAdapter_.get(), assetAdapter_.get()};
                    iniEdits.trackKeys[TexcoordObj] = false;

                    // ("", "other") is the one that DOES use the generic RemapFix name: it holds the
                    // sections that are not a resource of any particular kind, and the fix has
                    // nothing to say about them beyond swapping the hash.
                    iniEdits.edits[OtherObj] = {renameAdapter_.get(), assetAdapter_.get()};
                    iniEdits.trackKeys[OtherObj] = false;

                    // The face keeps the two edits it would have had inside ("", "other") -- the
                    // RemapFix name is right for it, and the hash remap is a no-op wherever the pair
                    // shares a face hash -- and adds the register swap, which is the whole reason it
                    // has a mod object of its own. The rename is what stops the copied graph
                    // rendering as a verbatim duplicate of the source text, the same trap the blend
                    // fell into.
                    iniEdits.edits[FaceObj] = {renameAdapter_.get(), assetAdapter_.get(),
                                                faceSwapAdapter_.get()};
                    iniEdits.trackKeys[FaceObj] = false;

                    objEdits_ = ObjGroupEdit({iniEdits}, false);
                }

                IniFileFixContext ctx_;
                std::string toModName_;
                GIMICharFixerConfig config_;

                // The source's drawn objects, the target's, and the (source -> target) pairs
                // between them -- one entry per remap, so a split source appears more than once.
                std::vector<ModObj> drawnObjs_;
                std::vector<ModObj> targetObjs_;
                std::vector<std::pair<ModObj, ModObj>> objMap_;
                std::unordered_map<ModObj, ModObj, Fixer::ModObjHash> indexSrcObjs_;

                std::unique_ptr<ObjGroupRemap> objSplitRemap_;
                std::unordered_map<ModObj, std::unique_ptr<RegNewVals<>>, Fixer::ModObjHash> newRegVals_;
                std::unordered_map<ModObj, std::unique_ptr<RegPartEdit<>>, Fixer::ModObjHash> newRegValAdapters_;

                std::vector<std::unique_ptr<TexEditorReplace<>>> texReplaces_;
                std::vector<std::unique_ptr<Collector>> texCollects_;

                std::unique_ptr<RegRemap<>> faceRegSwap_;
                std::unique_ptr<RegPartEdit<>> faceSwapAdapter_;

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

    IniFixBuilder::Factory makeGIMICharFixer(GIMICharFixerConfig config) {
        return [config](BaseIniParser<>* parser, const std::string& toModName, std::optional<int> modTypeId) {
            return std::make_shared<GIMICharFixerImpl>(parser, toModName, modTypeId, config);
        };
    }
}
