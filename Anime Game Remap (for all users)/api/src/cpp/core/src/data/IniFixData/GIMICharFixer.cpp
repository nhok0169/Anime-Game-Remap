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
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegFillMissing.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/PositionEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexCreatorEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEditorEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/VGRemapBlendEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/tools/StringTools.h"
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

                    // buildObjMap FIRST -- it is what works out groupCount_, and both collectors
                    // below build one instance per group.
                    buildObjMap();
                    buildBlendCollector();
                    buildPositionCollector();
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

                    for (auto& collect : blendCollects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }

                    // Empty unless this character's config asks for a position edit, which
                    // almost none do -- see GIMICharFixerConfig::positionEdit.
                    for (auto& collect : positionCollects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }

                    for (auto& collect : texCollects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }

                    this->graphGroupEdits.push_back(&objIndexEdits_);
                    this->graphGroupEdits.push_back(&objEdits_);

                    // THE TEXTURE ADDS GO LAST, unlike the edits above, and the difference is not
                    // cosmetic.
                    //
                    // A texture EDIT names the register its texture already hangs off, which is
                    // the register BEFORE any shift -- so it has to collect before the shift moves
                    // it. A texture ADD names the register it is filling, which only becomes free
                    // AFTER the shift has vacated it.
                    //
                    // Run both before, and they collide: Ganyu's DarkDiffuse edit and her NormalMap
                    // add both key on ps-t0, the second overwrote the first, and the shift then
                    // duplicated the normal map into ps-t1 as well. The head rendered with a flat
                    // yellow normal map in the diffuse slot and the edited diffuse referenced by
                    // nothing at all -- with every log line reporting success, both textures
                    // written, and the .ini file looking entirely reasonable.
                    for (auto& collect : texAddCollects_) {
                        this->graphGroupEdits.push_back(collect.get());
                    }

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

                    this->copyPreamble = config_.copyPreamble;
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
                            pairGroups_.push_back(0);
                        }

                        return;
                    }

                    ObjGroupRemap::RemapList remap;

                    // WHICH GROUP EACH PAIR LANDS IN, worked out the same way GraphGroupRemap does
                    // it: a target's first claimant keeps the main group, the second goes to an
                    // additional one, and so on. Mirroring that here rather than asking afterwards
                    // is what lets the edits below be built before the groups exist.
                    std::unordered_map<ModObj, std::size_t, Fixer::ModObjHash> claims;

                    for (const auto& split : config_.objSplits) {
                        const ModObj srcObj("", split.first);
                        std::vector<ObjGroupRemap::RemapTarget> targets;

                        for (const std::string& toObj : split.second) {
                            const ModObj tgtObj("", toObj);
                            const std::size_t group = claims[tgtObj]++;

                            objMap_.emplace_back(srcObj, tgtObj);
                            pairGroups_.push_back(group);
                            groupCount_ = std::max(groupCount_, group + 1);

                            if (std::find(targetObjs_.begin(), targetObjs_.end(), tgtObj) == targetObjs_.end()) {
                                targetObjs_.push_back(tgtObj);
                            }

                            targets.emplace_back(GraphId(0, tgtObj.first, tgtObj.second));
                        }

                        remap.emplace_back(GraphId(0, srcObj.first, srcObj.second), std::move(targets));
                    }

                    // EVERY OTHER GRAPH HAS TO BE IN EVERY GROUP, and this is the half of a merge
                    // that is easy to miss. A second .ini file holding only the merged object would
                    // name a Blend.buf, a position and a texcoord that are not in it. The old
                    // script's own output settles it -- JeanSeaRemapFix1.ini carries the full
                    // blend/position/texcoord/ib/VertexLimitRaise/face set exactly as the first file
                    // does, and differs only in which drawn object it holds.
                    //
                    // Listing the same target twice is how that is asked for: the first lands in
                    // group 0, the second collides and lands in group 1 -- the same mechanism that
                    // creates the extra group in the first place.
                    if (groupCount_ > 1) {
                        // AND THEY MUST COME OUT UNRENAMED. copyGraph renames as it copies, falling
                        // back to IniNamingTools::getObjRemapFixName -- right for a drawn object,
                        // which is being renamed from one object to another, and wrong for these,
                        // which already have a rename of their own further down (the blend uses the
                        // blend convention, the ib the ib one, and so on). Letting both run gives
                        // ...JeanRemapBlendJeanRemapFix, a name no other tool recognises.
                        //
                        // An IDENTITY function rather than an empty one: empty is what asks for the
                        // default. This is the one place in this file that wants copyGraph to do
                        // nothing but copy.
                        const ObjGroupRemap::RenameFunc keepName = [](const std::string& name) { return name; };

                        for (const ModObj& modObj : {IbObj, BlendObj, PositionObj, TexcoordObj, OtherObj, FaceObj}) {
                            std::vector<ObjGroupRemap::RemapTarget> targets;
                            for (std::size_t i = 0; i < groupCount_; ++i) {
                                targets.emplace_back(GraphId(0, modObj.first, modObj.second), keepName);
                            }

                            remap.emplace_back(GraphId(0, modObj.first, modObj.second), std::move(targets));
                        }
                    }

                    // An empty renameFunc on every target, which is what asks copyGraph for the
                    // default -- IniNamingTools::getObjRemapFixName, the object swap plus the same
                    // getRemapFixName the one-to-one path uses. Writing one by hand here would only
                    // be a way to get it subtly wrong.
                    objSplitRemap_ = std::make_unique<ObjGroupRemap>(std::move(remap));
                }

                /**
                 * Whether 'group' holds the copy of 'targetObj' that came from 'srcObj'.
                 *
                 * A one-to-one fix has no objSplits and therefore one group per object, so the
                 * answer is just "is this the object itself".
                 */
                bool groupHasSrc(std::size_t group, const std::string& targetObj, const std::string& srcObj) const {
                    for (std::size_t i = 0; i < objMap_.size(); ++i) {
                        if (pairGroups_[i] == group && objMap_[i].second.second == targetObj
                                && objMap_[i].first.second == srcObj) {
                            return true;
                        }
                    }

                    return false;
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
                    // Per group, for the same reason as the blend collector above.
                    for (std::size_t group = 0; group < groupCount_; ++group) {
                        for (const GIMICharFixerConfig::TexEdit& texEdit : config_.texEdits) {
                            // A texture edit belonging to ONE source object runs only in the group
                            // that source landed in -- the merge case, see buildEdits' note. An
                            // empty srcObj is every group, which is what a split always wants.
                            if (!texEdit.srcObj.empty() && !groupHasSrc(group, texEdit.obj, texEdit.srcObj)) {
                                continue;
                            }

                            const GraphId srcGraph(group, "", texEdit.obj);

                            // A DISTINCT mod object from the source graph, exactly as the blend
                            // needs: pointing it at srcGraph would have the resource overwrite the
                            // graph it was collected from.
                            // THE EDIT'S NAME IS PART OF THE GRAPH, not just the object's.
                            // Named for the object alone, two edits on the SAME object share
                            // one resource graph and the second loses: CherryHuTao edits her
                            // body's diffuse AND its lightmap, and only the diffuse survived --
                            // the .ini referenced the lightmap's resource and never defined it,
                            // so the body drew with no lightmap at all and looked flat.
                            //
                            // Nothing about the OUTPUT naming changes: a resource section is
                            // named from the source resource plus the target and the edit name
                            // (getFixResourceName), never from this graph's mod object.
                            const GraphId resGraph(group, "", texEdit.obj + "RemapTex" + texEdit.name);

                            auto replace = std::make_unique<TexEditorReplace<>>(
                                resGraph, TexEditor({texEdit.filter}, texEdit.compress), makeCharResEditConfig(),
                                "resourceRemapTexEdit", texEdit.name);

                            auto collect = std::make_unique<Collector>();
                            collect->srcRegs = {{srcGraph, texEdit.reg}};
                            collect->resEdits = {{texEdit.obj, replace.get()}};

                            texReplaces_.push_back(std::move(replace));
                            texCollects_.push_back(std::move(collect));
                        }

                        // The CREATED textures, same shape. TexCreatorCreate rather than the
                        // plain TexCreate for the same reason the edit above uses
                        // TexEditorReplace: the core class names things correctly and builds
                        // no file, and is in fact abstract, since a created resource also has
                        // to build the section naming it.
                        for (const GIMICharFixerConfig::TexAdd& texAdd : config_.texAdds) {
                            const GraphId srcGraph(group, "", texAdd.obj);
                            const GraphId resGraph(group, "", texAdd.obj + "RemapTexAdd");

                            auto create = std::make_unique<TexCreatorCreate<>>(
                                resGraph, texAdd.name, texAdd.texCreator, makeCharResEditConfig());

                            auto collect = std::make_unique<Collector>();
                            collect->srcRegs = {{srcGraph, texAdd.reg}};
                            collect->resEdits = {{texAdd.obj, create.get()}};

                            texCreates_.push_back(std::move(create));
                            texAddCollects_.push_back(std::move(collect));
                        }
                    }
                }

                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                // ONE COLLECTOR PER GROUP. A Collector is addressed by GraphId, and a GraphId's
                // iniIndex is the GROUP index -- so a collector built for group 0 simply does not
                // see the graphs a merge put in group 1.
                //
                // Left that way the second .ini file keeps `vb1 = Resource<Mod>Blend`, the mod's
                // ORIGINAL blend, and never gets a [Resource<Mod><Target>RemapBlend] section at all.
                // Every text-level check passes -- the reference resolves, because the original
                // blend really is there -- and in game that half of the model draws with unremapped
                // weights. Only reading the section's content catches it.
                void buildBlendCollector() {
                    IniFile* iniFile = ctx_.getIniFile();
                    const std::optional<Version> toVersion = (iniFile == nullptr) ? std::nullopt : iniFile->toVersion;

                    for (std::size_t group = 0; group < groupCount_; ++group) {
                        const GraphId blendGraph(group, BlendObj.first, BlendObj.second);
                        const GraphId blendResGraph(group, BlendObj.first, BlendObj.second + "RemapBlend");

                        auto replace = std::make_unique<VGRemapBlendReplace<>>(
                            blendResGraph, makeCharResEditConfig(), ctx_.modType(), ctx_.version(), toVersion);

                        auto collect = std::make_unique<Collector>();
                        collect->srcRegs = {{blendGraph, BlendReg}};
                        collect->resEdits = {{IniKeywords::Blend, replace.get()}};
                        collect->partPredicates = {{blendGraph, blendHashParts()}};

                        collect->trackKeysIsGlobal = false;
                        collect->trackKeys = {{blendGraph, true}};
                        collect->keysToTrack = {{blendGraph, std::unordered_set<std::string>{IniKeywords::Hash}}};

                        blendReplaces_.push_back(std::move(replace));
                        blendCollects_.push_back(std::move(collect));
                    }
                }

                /**
                 * The position buffer, for the one pair that needs its vertices moved.
                 *
                 * Deliberately much simpler than the blend's collector next door. The blend has
                 * to find which parts carry the qualifying hash, because its vb1 and that hash
                 * live in different sections; the position graph IS the position mod object, so
                 * every part of it is wanted and there is nothing to window.
                 *
                 * Nothing is built at all when the config asks for no edit -- not even a renamed
                 * copy of the original, which would go stale silently the moment the mod's own
                 * position buffer changed.
                 */
                void buildPositionCollector() {
                    if (!config_.positionEdit) {
                        return;
                    }

                    for (std::size_t group = 0; group < groupCount_; ++group) {
                        const GraphId positionGraph(group, PositionObj.first, PositionObj.second);
                        const GraphId positionResGraph(group, PositionObj.first,
                                                        PositionObj.second + "RemapPosition");

                        auto replace = std::make_unique<PositionEditReplace<>>(
                            positionResGraph, makeCharResEditConfig(), config_.positionEdit);

                        auto collect = std::make_unique<Collector>();
                        collect->srcRegs = {{positionGraph, IniKeywords::Vb0}};
                        collect->resEdits = {{IniKeywords::Position, replace.get()}};

                        positionReplaces_.push_back(std::move(replace));
                        positionCollects_.push_back(std::move(collect));
                    }
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
                        const ModObj& modObj = entry.second;

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
                    // a part by the SOURCE's own hash and index, the remap rewrites both to the
                    // target's, and the register loop refreshes the KVP colouring after every edit --
                    // so run
                    // the other way round the window comes back empty and this silently does
                    // nothing. That is exactly how the NNFix placement went missing.
                    // ONE IniEdits PER GROUP, because the WINDOW differs per group even though the
                    // edit does not. GraphGroupEdit indexes this vector by group and gives a group
                    // past its end nothing at all, so a merge whose second group is missing here
                    // comes out as an unrenamed verbatim copy.
                    //
                    // The value written is the same in every group -- the target's index for that
                    // object -- but the filter that finds the part to write it into reads the
                    // SOURCE's hash and index, and group 1's "body" was copied from the source's
                    // "dress". Ask about the wrong one and the window comes back empty and the
                    // rewrite silently does nothing.
                    std::vector<ObjGroupEdit::IniEdits> indexIniEdits(groupCount_);

                    for (std::size_t i = 0; i < objMap_.size(); ++i) {
                        const ModObj& srcObj = objMap_[i].first;
                        const ModObj& modObj = objMap_[i].second;
                        const std::size_t group = pairGroups_[i];

                        auto adapter = indexAdapters_.find(modObj);
                        if (adapter == indexAdapters_.end() || group >= indexIniEdits.size()) {
                            continue;
                        }

                        indexIniEdits[group].edits[modObj] = {adapter->second.get()};
                        indexIniEdits[group].keyFilters[modObj] = {objFilter_->filter(srcObj)};

                        // Asked for rather than restated, so this can never drift out of step with
                        // what the filter actually reads.
                        indexIniEdits[group].keysToTrack[modObj] = objFilter_->keysToTrack();
                        indexIniEdits[group].trackKeys[modObj] = true;
                    }

                    objIndexEdits_ = ObjGroupEdit(std::move(indexIniEdits), false);
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
                    //
                    // ORFix and NNFix go unconditionally: they are MANDATORY, re-issued for every
                    // drawn object, so a survivor of the mod's own would be a duplicate call.
                    //
                    // TexFx does NOT, and matching it by folder was deleting modder content. TexFx
                    // is OPTIONAL -- re-issued only where this character's objFixCalls row names a
                    // sub-command -- so a mod's call to any other one duplicates nothing and is
                    // simply a feature of their mod. GanyuTwilight's dress ships
                    // 'CommandList\TexFx\Transparency.0', which the pure-Python original keeps in
                    // the remapped copy and the folder match silently removed.
                    //
                    // So: strip only the sub-commands this fixer will itself re-issue, which keeps
                    // the no-duplicates guarantee for those and leaves the rest alone.
                    std::unordered_set<std::string> reissuedTexFx;
                    for (const auto& fixCallEntry : config_.objFixCalls) {
                        for (const std::string& fixCallPath : fixCallEntry.second) {
                            if (StringTools::startsWith(fixCallPath, IniKeywords::TexFxFolder)) {
                                reissuedTexFx.insert(fixCallPath);
                            }
                        }
                    }

                    auto isFixCall = [reissuedTexFx](long long, const std::string& value) {
                        return value == IniKeywords::ORFixPath || value == IniKeywords::NNFixPath ||
                               reissuedTexFx.find(value) != reissuedTexFx.end();
                    };

                    removeFixCalls_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::Run, RegRemove<>::RemoveKeyCheck(isFixCall)}});

                    // 2. Take the shared draw call off ("", "ib"). Each remapped object now draws
                    //    its own geometry, so the single drawindexed they all ran into is wrong.
                    removeDrawIndexed_ = std::make_unique<RegRemove<>>(
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>>{
                            {IniKeywords::DrawIndexed, std::nullopt}});

                    // 3. Re-issue that draw call per object -- with RegFillMissing, which puts it
                    //    AFTER everything the object's CommandList sets up.
                    //
                    //    getKeyMissingPartsNode reports a section whose branches ALL lack the
                    //    register as allBranchesMissing, bubbling the fill up to the parent
                    //    TextureOverride; a section where only SOME branches lack it gets just
                    //    those branches. For a GIMI character nothing in the object's CommandList
                    //    draws, so the draw lands on the TextureOverride, once, after the whole
                    //    CommandList has returned.
                    //
                    //    That ordering is the point, and it is why this is NOT derived from 'ib'
                    //    the way the pure-Python original derives it. Binding the draw to 'ib' puts
                    //    it inside each branch that binds geometry -- ahead of anything the section
                    //    does afterwards:
                    //
                    //        if $top...        ib, ps-t0, ps-t1, drawindexed
                    //        endif
                    //        if $DressTransparency==1
                    //            ps-t69 = ResourceDressTransparency
                    //            run = CommandList\TexFx\Transparency.0
                    //        endif
                    //
                    //    The unfixed mod draws in the shared [CommandList<Mod>IB] instead, which
                    //    runs once this section has RETURNED, so ps-t69 and TexFx are in place
                    //    before any geometry is rendered. Drawing in the ib branches renders the
                    //    dress before its transparency is configured, and TexFx then acts on a
                    //    surface that is already drawn. The original has that bug; matching it
                    //    reproduced it, which is why this deliberately diverges.
                    fillDrawIndexed_ = std::make_unique<RegFillMissing<>>(
                        IniKeywords::DrawIndexed,
                        RegFillMissing<>::makeFillMissing(IniKeywords::DrawIndexed, DrawIndexedAuto));

                    // 4. Re-issuing the external libraries is per object now -- see the
                    //    objFixCalls loop below, which builds one RegDelimitedAdd per call. The rule
                    //    is the same for all three of them (NNFix, ORFix, TexFx): immediately before
                    //    every drawindexed, and once at the end of any path that draws nothing,
                    //    which is the reason RegDelimitedAdd exists.

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

                    // THE FACE GETS ITS OWN, LENIENT ASSET REMAP -- notFoundVal left as
                    // std::nullopt, which means "leave the value alone" rather than writing the
                    // HashNotFound sentinel the drawn objects use.
                    //
                    // A missing tex_face_diffuse row on the TARGET is not the same kind of fact as a
                    // missing ib or blend row. The latter is a real error and should be loud. The
                    // former is a hole in the collected data: 3dmigoto's frame dump does not always
                    // capture the face, and GanyuTwilight is one of the characters it missed -- her
                    // source assets carry no face component at all, though she plainly has a face.
                    //
                    // Leaving the source's hash in place is also the RIGHT answer whenever the pair
                    // shares one, which is the strong norm: of the eleven remap pairs where both
                    // sides have a row, nine share it (the exceptions are Kaeya/KaeyaSailwind and
                    // Keqing/KeqingOpulent). So this quietly does the correct thing for a shared
                    // hash without anyone having to assert the missing row's value, and degrades to
                    // a section that simply never fires rather than a corrupt one if it differs.
                    //
                    // The same reasoning RaidenFixer applies by giving its face a rename and no
                    // asset remap at all -- RaidenBoss has no tex_face_diffuse row either.
                    faceAssetRemap_ = std::make_unique<RegAssetRemap<>>(
                        std::vector<std::pair<std::string, RegAssetRemap<>::AssetSpec>>{
                            {IniKeywords::Hash, RegAssetRemap<>::AssetSpec(ctx_.modTypeHashes())}},
                        toModName_, ctx_.modTypeName().value_or(""), ctx_.version(), toVersion);

                    renameAdapter_ = std::make_unique<GraphPartEdit<>>(renameGraph_.get());
                    renameBlendAdapter_ = std::make_unique<GraphPartEdit<>>(renameBlendGraph_.get());
                    renamePositionAdapter_ = std::make_unique<GraphPartEdit<>>(renamePositionGraph_.get());
                    renameTexcoordAdapter_ = std::make_unique<GraphPartEdit<>>(renameTexcoordGraph_.get());
                    renameIbAdapter_ = std::make_unique<GraphPartEdit<>>(renameIbGraph_.get());
                    assetAdapter_ = std::make_unique<RegPartEdit<>>(assetRemap_.get());
                    faceSwapAdapter_ = std::make_unique<RegPartEdit<>>(faceRegSwap_.get());
                    faceAssetAdapter_ = std::make_unique<RegPartEdit<>>(faceAssetRemap_.get());
                    removeFixCallsAdapter_ = std::make_unique<RegPartEdit<>>(removeFixCalls_.get());
                    fillAdapter_ = std::make_unique<GraphPartEdit<>>(fillDrawIndexed_.get());
                    removeDrawIndexedAdapter_ = std::make_unique<RegPartEdit<>>(removeDrawIndexed_.get());

                    // ---- register shifts ----
                    //
                    // One RegRemap per object holding every rename that object wants, which is what
                    // makes a shift safe: remapKeys rebuilds the part in a single pass, consulting
                    // the rules once per ORIGINAL key, so ps-t1 -> ps-t0 and ps-t2 -> ps-t1 cannot
                    // read each other's output. Two edits in sequence would collapse them.
                    for (const auto& entry : config_.objRegRemaps) {
                        const ModObj modObj("", entry.first);

                        std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>> remaps;
                        for (const auto& rename : entry.second) {
                            RemapList<std::string, std::string> targets;
                            for (const std::string& toReg : rename.second) {
                                targets.push_back(toReg);
                            }

                            remaps.emplace_back(rename.first, RegRemap<>::KeyRemapValue(std::move(targets)));
                        }

                        auto edit = std::make_unique<RegRemap<>>(std::move(remaps));
                        regRemapAdapters_[modObj] = std::make_unique<RegPartEdit<>>(edit.get());
                        regRemaps_[modObj] = std::move(edit);
                    }

                    // ---- which external libraries each object re-issues ----
                    //
                    // One RegDelimitedAdd per call, all sharing the same placement rule -- see
                    // GIMICharFixerConfig::objFixCalls -- but NOT all unconditional.
                    //
                    // NNFix and ORFix are MANDATORY: a remapped object re-issues them whether or not
                    // the mod mentions them. TexFx is OPTIONAL. It is an addon a modder opts into by
                    // binding its two dedicated registers, ps-t69 and ps-t70, so its sub-command
                    // belongs only where one of those is actually bound -- which is what requiredRegs
                    // below expresses.
                    //
                    // Without that gate TexFx was placed like NNFix, before every draw. On
                    // GanyuTwilight's head that is six TN.0 calls where the pure-Python original
                    // emits exactly one, on the trailing 'ps-t69 = null' part.
                    std::unordered_map<ModObj, std::vector<std::string>, Fixer::ModObjHash> fixCalls;
                    for (const ModObj& modObj : targetObjs_) {
                        fixCalls[modObj] = {IniKeywords::NNFixPath};
                    }

                    for (const auto& entry : config_.objFixCalls) {
                        fixCalls[ModObj("", entry.first)] = entry.second;
                    }

                    for (const auto& entry : fixCalls) {
                        for (const std::string& path : entry.second) {
                            // TexFx reads ps-t69/ps-t70, so its sub-command only has to come AFTER
                            // whichever of them the modder bound -- not at the end of the path, and
                            // not at all when neither is bound. That is an any-of "must precede"
                            // group, which is exactly RegSurroundedAdd::optBeforeRegs.
                            if (StringTools::startsWith(path, IniKeywords::TexFxFolder)) {
                                auto texFxAdd = std::make_unique<RegSurroundedAdd<>>(
                                    RegSurroundedAdd<>::Additions{{IniKeywords::Run, path}},
                                    RegSurroundedAdd<>::RegMap{},
                                    RegSurroundedAdd<>::RegMap{},
                                    false,
                                    RegSurroundedAdd<>::RegMap{
                                        {IniKeywords::PsT69, {}}, {IniKeywords::PsT70, {}}});

                                fixCallAdapters_[entry.first].push_back(
                                    std::make_unique<GraphPartEdit<>>(texFxAdd.get()));
                                texFxAdds_.push_back(std::move(texFxAdd));
                                continue;
                            }

                            // NNFix and ORFix are mandatory and keyed on the draw call instead.
                            //
                            // pathEndOnlyWhenUndelimited: one call immediately before every draw,
                            // and a single one at the end only for a graph that never draws (the
                            // shape a character whose fix does not move the draw call has). Without
                            // it a path that already drew picks up one more call after its last
                            // draw, and ORFix swaps the diffuse and lightmap registers on every
                            // call, so that surplus one leaves them swapped.
                            auto add = std::make_unique<RegDelimitedAdd<>>(
                                RegDelimitedAdd<>::Additions{{IniKeywords::Run, path}},
                                RegDelimitedAdd<>::RegMap{{IniKeywords::DrawIndexed, {}}},
                                /*pathEndOnlyWhenUndelimited*/ true);

                            fixCallAdapters_[entry.first].push_back(
                                std::make_unique<GraphPartEdit<>>(add.get()));
                            fixCallAdds_.push_back(std::move(add));
                        }
                    }

                    // Registers this target has no use for, stripped before anything else looks
                    // at the part. std::nullopt as the check means "every occurrence, whatever its
                    // value" -- the same shape removeDrawIndexed_ uses.
                    for (const auto& entry : config_.objRegRemovals) {
                        const ModObj modObj("", entry.first);

                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> keys;
                        for (const std::string& reg : entry.second) {
                            keys.emplace_back(reg, std::nullopt);
                        }

                        auto edit = std::make_unique<RegRemove<>>(std::move(keys));
                        regRemovalAdapters_[modObj] = std::make_unique<RegPartEdit<>>(edit.get());
                        regRemovals_[modObj] = std::move(edit);
                    }

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

                    // ---- the SOURCE-KEYED edits ----
                    //
                    // Everything above names a TARGET object, which is right for a split -- each
                    // target has exactly one source, so "the copy that came from X" and "the
                    // target called Y" are the same thing. A MERGE breaks that: several sources
                    // land on one target, in different groups, and the pure-Python rows key their
                    // pre-split edits by SOURCE for exactly that reason.
                    //
                    // Xiangling is the worked example. Her DarkDiffuse edit is declared on her
                    // head, and all three of her objects merge onto XianglingCheer's head -- so a
                    // target-keyed edit darkens the body and dress copies too, which the old
                    // script's own output shows it does not (RemapFix1 and RemapFix2 point at the
                    // untouched ...BodyDiffuse.0 and ...DressDiffuse.0).
                    //
                    // Resolved here rather than at the call site because objMap_ and pairGroups_
                    // already say which (target, group) each source became.
                    // Built at size rather than resized: MSVC will not grow a vector whose element
                    // is an unordered_map of unique_ptr, because that map has no noexcept move and
                    // vector then falls back to copying.
                    srcRegRemovalAdapters_ = SrcAdapters(groupCount_);
                    srcRegRemapAdapters_ = SrcAdapters(groupCount_);

                    for (const auto& entry : config_.srcObjRegRemovals) {
                        std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> keys;
                        for (const std::string& reg : entry.second) {
                            keys.emplace_back(reg, std::nullopt);
                        }

                        for (std::size_t i = 0; i < objMap_.size(); ++i) {
                            if (objMap_[i].first.second != entry.first) {
                                continue;
                            }

                            auto edit = std::make_unique<RegRemove<>>(keys);
                            srcRegRemovalAdapters_[pairGroups_[i]][objMap_[i].second] =
                                std::make_unique<RegPartEdit<>>(edit.get());
                            srcRegRemovals_.push_back(std::move(edit));
                        }
                    }

                    for (const auto& entry : config_.srcObjRegRemaps) {
                        for (std::size_t i = 0; i < objMap_.size(); ++i) {
                            if (objMap_[i].first.second != entry.first) {
                                continue;
                            }

                            // Rebuilt per pair rather than shared: RegRemap owns its rules and the
                            // adapters hand out raw pointers, so one instance per place it runs.
                            std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>> remaps;
                            for (const auto& rename : entry.second) {
                                RemapList<std::string, std::string> targets;
                                for (const std::string& toReg : rename.second) {
                                    targets.push_back(toReg);
                                }

                                remaps.emplace_back(rename.first, RegRemap<>::KeyRemapValue(std::move(targets)));
                            }

                            auto edit = std::make_unique<RegRemap<>>(std::move(remaps));
                            srcRegRemapAdapters_[pairGroups_[i]][objMap_[i].second] =
                                std::make_unique<RegPartEdit<>>(edit.get());
                            srcRegRemaps_.push_back(std::move(edit));
                        }
                    }

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
                    std::vector<ObjGroupEdit::IniEdits> perGroupEdits;

                    for (std::size_t group = 0; group < groupCount_; ++group) {
                        ObjGroupEdit::IniEdits iniEdits;

                        for (const ModObj& modObj : targetObjs_) {
                            std::vector<ObjGroupEdit::PartEdit*> edits;

                            // FIRST, so nothing downstream has to reason about a register that is on
                            // its way out -- notably the NNFix placement, which counts what it walks
                            // past. The source-keyed one goes alongside for the same reason.
                            auto removal = regRemovalAdapters_.find(modObj);
                            if (removal != regRemovalAdapters_.end()) {
                                edits.push_back(removal->second.get());
                            }

                            auto srcRemoval = srcRegRemovalAdapters_[group].find(modObj);
                            if (srcRemoval != srcRegRemovalAdapters_[group].end()) {
                                edits.push_back(srcRemoval->second.get());
                            }

                            edits.push_back(removeFixCallsAdapter_.get());

                            // After the removal and before anything that reads a register by name.
                            auto remap = regRemapAdapters_.find(modObj);
                            if (remap != regRemapAdapters_.end()) {
                                edits.push_back(remap->second.get());
                            }

                            auto srcRemap = srcRegRemapAdapters_[group].find(modObj);
                            if (srcRemap != srcRegRemapAdapters_[group].end()) {
                                edits.push_back(srcRemap->second.get());
                            }

                            // The rename belongs to whichever of the two actually did it. With a
                            // split, GraphGroupRemap::copyGraph already renamed every section as it
                            // copied -- through IniNamingTools::getObjRemapFixName, which is the same
                            // getRemapFixName underneath plus the object swap. Renaming again here
                            // would append a second RemapFix suffix.
                            if (objSplitRemap_ == nullptr) {
                                edits.push_back(renameAdapter_.get());
                            }

                            // Only when this character's fix moves the draw call onto its drawn
                            // objects. Without it there is no drawindexed in these parts at all,
                            // RegDelimitedAdd finds no delimiter, and NNFix lands once at the end of
                            // the path -- which is exactly what the pure-Python Mona/Rosaria output
                            // shows.
                            if (config_.moveDrawIndexed) {
                                edits.push_back(fillAdapter_.get());
                            }

                            auto calls = fixCallAdapters_.find(modObj);
                            if (calls != fixCallAdapters_.end()) {
                                for (const auto& adapter : calls->second) {
                                    edits.push_back(adapter.get());
                                }
                            }

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

                        // ("", "other") is the one that DOES use the generic RemapFix name: it holds
                        // the sections that are not a resource of any particular kind, and the fix has
                        // nothing to say about them beyond swapping the hash.
                        iniEdits.edits[OtherObj] = {renameAdapter_.get(), assetAdapter_.get()};
                        iniEdits.trackKeys[OtherObj] = false;

                        // The face keeps the two edits it would have had inside ("", "other") -- the
                        // RemapFix name is right for it, and the hash remap is a no-op wherever the
                        // pair shares a face hash -- and adds the register swap, which is the whole
                        // reason it has a mod object of its own. The rename is what stops the copied
                        // graph rendering as a verbatim duplicate of the source text, the same trap
                        // the blend fell into.
                        iniEdits.edits[FaceObj] = {renameAdapter_.get(), faceAssetAdapter_.get(),
                                                    faceSwapAdapter_.get()};
                        iniEdits.trackKeys[FaceObj] = false;

                        perGroupEdits.push_back(std::move(iniEdits));
                    }

                    // One IniEdits per group. They are identical unless srcObjRegRemovals or
                    // srcObjRegRemaps put something in one of them -- an entry for an object a given
                    // group does not hold costs nothing either way, since GraphGroupEdit walks the
                    // GROUP's mod objects and looks each one up here rather than the other way round.
                    objEdits_ = ObjGroupEdit(std::move(perGroupEdits), false);
                }

                IniFileFixContext ctx_;
                std::string toModName_;
                GIMICharFixerConfig config_;

                // The source's drawn objects, the target's, and the (source -> target) pairs
                // between them -- one entry per remap, so a split source appears more than once.
                std::vector<ModObj> drawnObjs_;
                std::vector<ModObj> targetObjs_;
                std::vector<std::pair<ModObj, ModObj>> objMap_;

                // Which group each objMap_ pair lands in, and how many groups that makes. Both are
                // 0/1 for every shape but a merge.
                std::vector<std::size_t> pairGroups_;
                std::size_t groupCount_ = 1;

                std::unique_ptr<ObjGroupRemap> objSplitRemap_;
                std::unordered_map<ModObj, std::unique_ptr<RegRemap<>>, Fixer::ModObjHash> regRemaps_;
                std::unordered_map<ModObj, std::unique_ptr<RegPartEdit<>>, Fixer::ModObjHash> regRemapAdapters_;

                std::vector<std::unique_ptr<RegDelimitedAdd<>>> fixCallAdds_;
                std::vector<std::unique_ptr<RegSurroundedAdd<>>> texFxAdds_;
                std::unordered_map<ModObj, std::vector<std::unique_ptr<GraphPartEdit<>>>, Fixer::ModObjHash> fixCallAdapters_;

                // The source-keyed edits, indexed by GROUP then target object -- see the note in
                // buildEdits. The owning vectors are flat because nothing looks these up by key;
                // the adapters do, and they hold raw pointers into these.
                using SrcAdapters = std::vector<std::unordered_map<ModObj, std::unique_ptr<RegPartEdit<>>, Fixer::ModObjHash>>;

                std::vector<std::unique_ptr<PositionEditReplace<>>> positionReplaces_;
                std::vector<std::unique_ptr<Collector>> positionCollects_;

                std::vector<std::unique_ptr<RegRemove<>>> srcRegRemovals_;
                SrcAdapters srcRegRemovalAdapters_;
                std::vector<std::unique_ptr<RegRemap<>>> srcRegRemaps_;
                SrcAdapters srcRegRemapAdapters_;

                std::unordered_map<ModObj, std::unique_ptr<RegRemove<>>, Fixer::ModObjHash> regRemovals_;
                std::unordered_map<ModObj, std::unique_ptr<RegPartEdit<>>, Fixer::ModObjHash> regRemovalAdapters_;

                std::unordered_map<ModObj, std::unique_ptr<RegNewVals<>>, Fixer::ModObjHash> newRegVals_;
                std::unordered_map<ModObj, std::unique_ptr<RegPartEdit<>>, Fixer::ModObjHash> newRegValAdapters_;

                std::vector<std::unique_ptr<TexEditorReplace<>>> texReplaces_;
                std::vector<std::unique_ptr<TexCreatorCreate<>>> texCreates_;
                std::vector<std::unique_ptr<Collector>> texAddCollects_;
                std::vector<std::unique_ptr<Collector>> texCollects_;

                std::unique_ptr<RegRemap<>> faceRegSwap_;
                std::unique_ptr<RegPartEdit<>> faceSwapAdapter_;
                std::unique_ptr<RegPartEdit<>> faceAssetAdapter_;

                std::vector<std::unique_ptr<VGRemapBlendReplace<>>> blendReplaces_;
                std::vector<std::unique_ptr<Collector>> blendCollects_;

                std::unique_ptr<GraphRename<>> renameGraph_;
                std::unique_ptr<GraphRename<>> renameBlendGraph_;
                std::unique_ptr<GraphRename<>> renamePositionGraph_;
                std::unique_ptr<GraphRename<>> renameTexcoordGraph_;
                std::unique_ptr<GraphRename<>> renameIbGraph_;
                std::unique_ptr<RegAssetRemap<>> assetRemap_;
                std::unique_ptr<RegAssetRemap<>> faceAssetRemap_;
                std::unique_ptr<RegRemove<>> removeFixCalls_;
                std::unique_ptr<RegFillMissing<>> fillDrawIndexed_;
                std::unique_ptr<RegRemove<>> removeDrawIndexed_;

                std::unique_ptr<GraphPartEdit<>> renameAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameBlendAdapter_;
                std::unique_ptr<GraphPartEdit<>> renamePositionAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameTexcoordAdapter_;
                std::unique_ptr<GraphPartEdit<>> renameIbAdapter_;
                std::unique_ptr<RegPartEdit<>> assetAdapter_;
                std::unique_ptr<RegPartEdit<>> removeFixCallsAdapter_;
                std::unique_ptr<GraphPartEdit<>> fillAdapter_;
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
